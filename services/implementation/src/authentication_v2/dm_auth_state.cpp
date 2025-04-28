/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "access_control_profile.h"
#include "hichain_auth_connector.h"
#include "multiple_user_connector.h"
#include "dm_crypto.h"
#include "dm_auth_state.h"
#include "dm_auth_context.h"
#include "dm_auth_manager_base.h"
#include "dm_auth_state_machine.h"
#include "dm_crypto.h"
#include "dm_softbus_cache.h"
#if defined(SUPPORT_SCREENLOCK)
#include "screenlock_manager.h"
#endif
#include "dm_log.h"
#include <sys/time.h>

namespace OHOS {
namespace DistributedHardware {
// clone task timeout map
const std::map<std::string, int32_t> TASK_TIME_OUT_MAP = {
    { std::string(AUTHENTICATE_TIMEOUT_TASK), CLONE_AUTHENTICATE_TIMEOUT },
    { std::string(NEGOTIATE_TIMEOUT_TASK), CLONE_NEGOTIATE_TIMEOUT },
    { std::string(CONFIRM_TIMEOUT_TASK), CLONE_CONFIRM_TIMEOUT },
    { std::string(ADD_TIMEOUT_TASK), CLONE_ADD_TIMEOUT },
    { std::string(WAIT_NEGOTIATE_TIMEOUT_TASK), CLONE_WAIT_NEGOTIATE_TIMEOUT },
    { std::string(WAIT_REQUEST_TIMEOUT_TASK), CLONE_WAIT_REQUEST_TIMEOUT },
    { std::string(WAIT_PIN_AUTH_TIMEOUT_TASK), CLONE_PIN_AUTH_TIMEOUT },
    { std::string(SESSION_HEARTBEAT_TIMEOUT_TASK), CLONE_SESSION_HEARTBEAT_TIMEOUT }
};

constexpr int32_t ONBINDRESULT_MAPPING_NUM = 2;
constexpr int32_t MS_PER_SECOND = 1000;
constexpr int32_t US_PER_MSECOND = 1000;
constexpr const static char* ONBINDRESULT_MAPPING_LIST[ONBINDRESULT_MAPPING_NUM] = {
    "CollaborationFwk",
    "cast_engine_service",
};

const std::map<DmAuthStateType, DmAuthStatus> NEW_AND_OLD_STATE_MAPPING = {
    { DmAuthStateType::AUTH_SRC_FINISH_STATE, DmAuthStatus::STATUS_DM_AUTH_FINISH },
    { DmAuthStateType::AUTH_SINK_FINISH_STATE, DmAuthStatus::STATUS_DM_SINK_AUTH_FINISH }
};

const std::map<int32_t, int32_t> NEW_AND_OLD_REPLAY_MAPPING = {
    { DM_ALREADY_AUTHED, SOFTBUS_OK },
    { SOFTBUS_OK, SOFTBUS_OK }
};

constexpr int32_t OPEN_PROCESS_NAME_WHITE_LIST_NUM = 1;
constexpr int32_t CLOSE_PROCESS_NAME_WHITE_LIST_NUM = 2;
constexpr const static char* OPEN_PROCESS_NAME_WHITE_LIST[OPEN_PROCESS_NAME_WHITE_LIST_NUM] = {
    "com.example.myapplication"
};
constexpr const static char* CLOSE_PROCESS_NAME_WHITE_LIST[CLOSE_PROCESS_NAME_WHITE_LIST_NUM] = {
    "CollaborationFwk",
    "gameservice_server"
};


int32_t DmAuthState::GetTaskTimeout(std::shared_ptr<DmAuthContext> context, const char* taskName, int32_t taskTimeOut)
{
    LOGI("GetTaskTimeout, taskName: %{public}s, authType_: %{public}d", taskName, context->authType);
    if (DmAuthState::IsImportAuthCodeCompatibility(context->authType)) {
        auto timeout = TASK_TIME_OUT_MAP.find(std::string(taskName));
        if (timeout != TASK_TIME_OUT_MAP.end()) {
            return timeout->second;
        }
    }
    return taskTimeOut;
}

void DmAuthState::HandleAuthenticateTimeout(std::shared_ptr<DmAuthContext> context, std::string name)
{
    LOGI("DmAuthContext::HandleAuthenticateTimeout start timer name %{public}s", name.c_str());
    context->timer->DeleteTimer(name);
    context->reason = ERR_DM_TIME_OUT;
    context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    LOGI("DmAuthContext::HandleAuthenticateTimeout complete");
}

bool DmAuthState::IsScreenLocked()
{
    bool isLocked = false;
#if defined(SUPPORT_SCREENLOCK)
    isLocked = OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked();
#endif
    LOGI("IsScreenLocked isLocked: %{public}d.", isLocked);
    return isLocked;
}

void DmAuthState::SourceFinish(std::shared_ptr<DmAuthContext> context)
{
    LOGI("SourceFinish reason:%{public}d", context->reason);
    context->listener->OnAuthResult(context->processInfo, context->peerTargetId.deviceId, context->accessee.tokenIdHash,
        GetOutputState(context->accesser.bundleName, context->state), context->reason);
    context->listener->OnBindResult(context->processInfo, context->peerTargetId,
        GetOutputReplay(context->accesser.bundleName, context->reason),
        GetOutputState(context->accesser.bundleName, context->state), GenerateBindResultContent(context));
    context->successFinished = true;

    if (context->reason != DM_OK && context->reason != DM_ALREADY_AUTHED) {
        // 根据凭据id 删除sink端多余的凭据
        context->hiChainAuthConnector->DeleteCredential(context->accesser.userId,
            context->accesser.lnnCredentialId);
        context->hiChainAuthConnector->DeleteCredential(context->accesser.userId,
            context->accesser.transmitCredentialId);
        // 根据skid删除sk，删除skid
        DeviceProfileConnector::GetInstance().DeleteSessionKey(context->accesser.userId,
            context->accesser.lnnSessionKeyId);
        DeviceProfileConnector::GetInstance().DeleteSessionKey(context->accesser.userId,
            context->accesser.transmitSessionKeyId);
    }
    LOGI("SourceFinish notify online");
    char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    Crypto::GetUdidHash(context->accessee.deviceId, reinterpret_cast<uint8_t *>(deviceIdHash));
    if (SoftbusCache::GetInstance().CheckIsOnline(std::string(deviceIdHash))) {
        SetProcessInfo(context);
        int32_t authForm = context->accesser.transmitBindType == DM_POINT_TO_POINT_TYPE ?
            DmAuthForm::PEER_TO_PEER : context->accesser.transmitBindType;
        context->softbusConnector->HandleDeviceOnline(context->accessee.deviceId, authForm);
    }

    context->authUiStateMgr->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
    context->timer->DeleteAll();
}

void DmAuthState::SinkFinish(std::shared_ptr<DmAuthContext> context)
{
    LOGI("SinkFinish reason:%{public}d", context->reason);
    context->listener->OnSinkBindResult(context->processInfo, context->peerTargetId,
        GetOutputReplay(context->accessee.bundleName, context->reason),
        GetOutputState(context->accessee.bundleName, context->state), GenerateBindResultContent(context));
    context->successFinished = true;
    if (context->reason != DM_OK) {
        // 根据凭据id 删除sink端多余的凭据
        context->hiChainAuthConnector->DeleteCredential(context->accessee.userId,
            context->accessee.lnnCredentialId);
        context->hiChainAuthConnector->DeleteCredential(context->accessee.userId,
            context->accessee.transmitCredentialId);
        // 根据skid删除sk，删除skid
        DeviceProfileConnector::GetInstance().DeleteSessionKey(context->accessee.userId,
            context->accessee.lnnSessionKeyId);
        DeviceProfileConnector::GetInstance().DeleteSessionKey(context->accessee.userId,
            context->accessee.transmitSessionKeyId);
    } else {
        SetAclInfo(context);
        if (NeedAgreeAcl(context)) {
            context->authMessageProcessor->PutAccessControlList(context,
                context->accessee, context->accesser.deviceId);
        }
        LOGI("SinkFinish notify online");
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        Crypto::GetUdidHash(context->accesser.deviceId, reinterpret_cast<uint8_t *>(deviceIdHash));
        if (SoftbusCache::GetInstance().CheckIsOnline(std::string(deviceIdHash))) {
            SetProcessInfo(context);
            int32_t authForm = context->accessee.transmitBindType == DM_POINT_TO_POINT_TYPE ?
                DmAuthForm::PEER_TO_PEER : context->accessee.transmitBindType;
            context->softbusConnector->HandleDeviceOnline(context->accesser.deviceId, authForm);
        }
    }

    context->authUiStateMgr->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
    context->timer->DeleteAll();
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_AUTH_RESP_FINISH, context); // 发送201给source侧
}

std::string DmAuthState::GenerateBindResultContent(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, "");
    DmAccess access = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        context->accessee : context->accesser;
    JsonObject jsonObj;
    jsonObj[DM_BIND_RESULT_NETWORK_ID] = access.networkId;
    if (access.deviceId.empty()) {
        jsonObj[TAG_DEVICE_ID] = "";
    } else {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        Crypto::GetUdidHash(access.deviceId, reinterpret_cast<uint8_t *>(deviceIdHash));
        jsonObj[TAG_DEVICE_ID] = deviceIdHash;
    }
    jsonObj[TAG_CONFIRM_OPERATION_V2] = context->confirmOperation;
    std::string content = jsonObj.Dump();
    return content;
}

bool DmAuthState::NeedReqUserConfirm(std::shared_ptr<DmAuthContext> context)
{
    // 不管是否有可信关系，都需要走pin码认证，主要指鸿蒙环PIN码导入场景
    if (DmAuthState::IsImportAuthCodeCompatibility(context->authType)) {
        return true;
    }

    // 有ACL，跳转到结束状态，发200报文，直接组网
    DmAccess access = context->direction == DM_AUTH_SOURCE ? context->accesser : context->accessee;
    if (access.isAuthed) {
        return false;
    }

    return true;
}

bool DmAuthState::NeedAgreeCredential(std::shared_ptr<DmAuthContext> context)
{
    return context->needAgreeCredential;
}

bool DmAuthState::NeedAgreeAcl(std::shared_ptr<DmAuthContext> context)
{
    return (context->direction == DM_AUTH_SOURCE) ? !context->accesser.isAuthed : !context->accessee.isAuthed;
}

bool DmAuthState::IsImportAuthCodeCompatibility(DmAuthType authType)
{
    if (authType == DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE ||
        authType == DmAuthType::AUTH_TYPE_NFC) {
        return true;
    }
    return false;
}

void DmAuthState::SetAclExtraInfo(std::shared_ptr<DmAuthContext> context)
{
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    JsonObject jsonObj;
    jsonObj[TAG_DMVERSION] = access.dmVersion;
    access.extraInfo = jsonObj.Dump();
    remoteAccess.extraInfo = jsonObj.Dump();
}

void DmAuthState::SetAclInfo(std::shared_ptr<DmAuthContext> context)
{
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    SetAclExtraInfo(context);
    access.lnnBindType = GetAclBindType(context, access.lnnCredentialId);
    remoteAccess.lnnBindType = GetAclBindType(context, remoteAccess.lnnCredentialId);

    access.transmitBindType = GetAclBindType(context, access.transmitCredentialId);
    remoteAccess.transmitBindType = GetAclBindType(context, remoteAccess.transmitCredentialId);
}

int32_t DmAuthState::GetAclBindType(std::shared_ptr<DmAuthContext> context, std::string credId)
{
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    JsonObject result;
    int32_t ret = context->hiChainAuthConnector->QueryCredInfoByCredId(access.userId, credId, result);
    if (ret != DM_OK) {
        LOGE("GetAclBindType QueryCredInfoByCredId failed, ret: %{public}d.", ret);
        return DM_UNKNOWN_TYPE;
    }
    if (!result.Contains(credId)) {
        LOGE("GetAclBindType result not contains credId.");
        return DM_UNKNOWN_TYPE;
    }
    int32_t credType = result[credId][FILED_CRED_TYPE].Get<int32_t>();
    if (credType == DM_AUTH_CREDENTIAL_ACCOUNT_RELATED) {
        return DM_SAME_ACCOUNT_TYPE;
    }
    if (credType == DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED) {
        return DM_POINT_TO_POINT_TYPE;
    }
    if (credType == DM_AUTH_CREDENTIAL_ACCOUNT_ACROSS) {
        return DM_SHARE_TYPE;
    }
    return DM_UNKNOWN_TYPE;
}

uint32_t DmAuthState::GetCredType(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credInfo)
{
    int32_t credType = credInfo[FILED_CRED_TYPE].Get<int32_t>();
    int32_t authorizedScope = credInfo[FILED_AUTHORIZED_SCOPE].Get<int32_t>();
    int32_t subject = credInfo[FILED_SUBJECT].Get<int32_t>();
    std::vector<std::string> appList;
    credInfo[FILED_AUTHORIZED_APP_LIST].Get(appList);
    if (credType == ACCOUNT_RELATED && authorizedScope == SCOPE_USER) {
        return DM_IDENTICAL_ACCOUNT;
    }
    if (credType == ACCOUNT_ACROSS && authorizedScope == SCOPE_USER &&
        context->direction == DM_AUTH_SOURCE && subject == SUBJECT_PRIMARY) {
        return DM_SHARE;
    }
    if (credType == ACCOUNT_ACROSS && authorizedScope == SCOPE_USER &&
        context->direction == DM_AUTH_SINK && subject == SUBJECT_SECONDARY) {
        return DM_SHARE;
    }
    if (credType == ACCOUNT_UNRELATED && authorizedScope == SCOPE_APP && HaveSameTokenId(context, appList)) {
        return DM_POINT_TO_POINT;
    }
    if (credType == ACCOUNT_UNRELATED && authorizedScope == SCOPE_USER && appList.empty()) {
        return DM_LNN;
    }
    return DM_INVALIED_TYPE;
}

uint32_t DmAuthState::GetCredentialType(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credInfo)
{
    CHECK_NULL_RETURN(context, DM_INVALIED_TYPE);
    if (!credInfo.Contains(FILED_CRED_TYPE) || !credInfo[FILED_CRED_TYPE].IsNumberInteger() ||
        !credInfo.Contains(FILED_AUTHORIZED_SCOPE) || !credInfo[FILED_AUTHORIZED_SCOPE].IsNumberInteger() ||
        !credInfo.Contains(FILED_SUBJECT) || !credInfo[FILED_SUBJECT].IsNumberInteger()) {
        LOGE("credType or authorizedScope invalid.");
        return DM_INVALIED_TYPE;
    }
    return GetCredType(context, credInfo);
}

bool DmAuthState::HaveSameTokenId(std::shared_ptr<DmAuthContext> context, const std::vector<std::string> &tokenList)
{
    // Store the token of src and sink. The size must be 2.
    if (tokenList.size() != 2) {
        LOGE("HaveSameTokenId invalid tokenList size.");
        return false;
    }

    // tokenIdList = [srcTokenId, sinkTokenId]
    std::string srcTokenIdHash = Crypto::GetTokenIdHash(tokenList[0]);
    std::string sinkTokenIdHash = Crypto::GetTokenIdHash(tokenList[1]);

    return ((srcTokenIdHash == context->accesser.tokenIdHash) &&
        (sinkTokenIdHash == context->accessee.tokenIdHash)) ||
        ((sinkTokenIdHash == context->accesser.tokenIdHash) &&
        (srcTokenIdHash == context->accessee.tokenIdHash));
}

bool DmAuthState::CheckProcessNameInWhiteList(const std::string &processName)
{
    LOGI("DmAuthState::CheckProcessNameInWhiteList start");
    if (processName.empty()) {
        LOGE("processName is empty");
        return false;
    }
    uint16_t index = 0;
#ifdef DEVICE_MANAGER_COMMON_FLAG
    for (; index < OPEN_PROCESS_NAME_WHITE_LIST_NUM; ++index) {
        std::string whitePkgName(OPEN_PROCESS_NAME_WHITE_LIST[index]);
        if (processName == whitePkgName) {
            LOGI("processName = %{public}s in whiteList.", processName.c_str());
            return true;
        }
    }
#else
    for (; index < CLOSE_PROCESS_NAME_WHITE_LIST_NUM; ++index) {
        std::string whitePkgName(CLOSE_PROCESS_NAME_WHITE_LIST[index]);
        if (processName == whitePkgName) {
            LOGI("processName = %{public}s in whiteList.", processName.c_str());
            return true;
        }
    }
#endif
    LOGI("CheckProcessNameInWhiteList: %{public}s invalid.", processName.c_str());
    return false;
}

int32_t DmAuthState::GetOutputState(const std::string &processName, int32_t state)
{
    LOGI("state %{public}d.", state);
    bool needMapFlag = false;
    for (uint16_t index = 0; index < ONBINDRESULT_MAPPING_NUM; ++index) {
        if (std::string(ONBINDRESULT_MAPPING_LIST[index]) == processName) {
            LOGI("processName %{public}s new protocol param convert to old protocol param.", processName.c_str());
            needMapFlag = true;
            break;
        }
    }
    if (needMapFlag) {
        auto it = NEW_AND_OLD_STATE_MAPPING.find(static_cast<DmAuthStateType>(state));
        if (it != NEW_AND_OLD_STATE_MAPPING.end()) {
            return static_cast<int32_t>(it->second);
        }
    }
    return state;
}

int32_t DmAuthState::GetOutputReplay(const std::string &processName, int32_t replay)
{
    LOGI("replay %{public}d.", replay);
    bool needMapFlag = false;
    for (uint16_t index = 0; index < ONBINDRESULT_MAPPING_NUM; ++index) {
        if (std::string(ONBINDRESULT_MAPPING_LIST[index]) == processName) {
            LOGI("processName %{public}s new protocol param convert to old protocol param.", processName.c_str());
            needMapFlag = true;
            break;
        }
    }
    if (needMapFlag) {
        auto it = NEW_AND_OLD_REPLAY_MAPPING.find(replay);
        if (it != NEW_AND_OLD_REPLAY_MAPPING.end()) {
            return static_cast<int32_t>(it->second);
        }
    }
    return replay;
}

uint64_t DmAuthState::GetSysTimeMs()
{
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;
    if (gettimeofday(&time, nullptr) != 0) {
        LOGE("GetSysTimeMs failed.");
        return 0;
    }
    return (uint64_t) time.tv_sec * MS_PER_SECOND + (uint64_t)time.tv_usec / US_PER_MSECOND;
}

void DmAuthState::DeleteAcl(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile)
{
    CHECK_NULL_VOID(context);
    LOGI("direction %{public}d.", static_cast<int32_t>(context->direction));
    CHECK_NULL_VOID(context->authMessageProcessor);
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    int32_t userId = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        profile.GetAccesser().GetAccesserUserId() : profile.GetAccessee().GetAccesseeUserId();
    int32_t sessionKeyId = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        profile.GetAccesser().GetAccesserSessionKeyId() : profile.GetAccessee().GetAccesseeSessionKeyId();
    std::string credId = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        profile.GetAccesser().GetAccesserCredentialIdStr() : profile.GetAccessee().GetAccesseeCredentialIdStr();

    context->authMessageProcessor->DeleteSessionKeyToDP(userId, sessionKeyId);
    context->hiChainAuthConnector->DeleteCredential(userId, credId);
    DeviceProfileConnector::GetInstance().DeleteAccessControlById(profile.GetAccessControlId());
}

void DmAuthState::SetProcessInfo(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    DmAccess localAccess = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        context->accesser : context->accessee;
    ProcessInfo processInfo;
    processInfo.userId = localAccess.userId;
    uint32_t bindLevel = static_cast<uint32_t>(localAccess.bindLevel);
    if (bindLevel == APP || bindLevel == SERVICE) {
        processInfo.pkgName = localAccess.pkgName;
    } else if (bindLevel == USER) {
        processInfo.pkgName = std::string(DM_PKG_NAME);
    } else {
        LOGE("bindlevel error %{public}d.", bindLevel);
        return;
    }
    context->softbusConnector->SetProcessInfo(processInfo);
}
} // namespace DistributedHardware
} // namespace OHOS
