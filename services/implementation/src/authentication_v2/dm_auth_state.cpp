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

constexpr uint16_t ONBINDRESULT_MAPPING_NUM = 2;
constexpr int32_t MS_PER_SECOND = 1000;
constexpr int32_t US_PER_MSECOND = 1000;
constexpr int32_t GET_SYSTEMTIME_MAX_NUM = 3;
constexpr const static char* ONBINDRESULT_MAPPING_LIST[ONBINDRESULT_MAPPING_NUM] = {
    "CollaborationFwk",
    "cast_engine_service",
};

const std::map<DmAuthStateType, DmAuthStatus> NEW_AND_OLD_STATE_MAPPING = {
    { DmAuthStateType::AUTH_SRC_FINISH_STATE, DmAuthStatus::STATUS_DM_AUTH_FINISH },
    { DmAuthStateType::AUTH_SINK_FINISH_STATE, DmAuthStatus::STATUS_DM_SINK_AUTH_FINISH },
    { DmAuthStateType::AUTH_IDLE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_START_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_NEGOTIATE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_CONFIRM_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_PIN_INPUT_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_START_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_DONE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_CREDENTIAL_EXCHANGE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_START_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_DONE_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT }
};

const std::map<int32_t, int32_t> NEW_AND_OLD_REPLAY_MAPPING = {
    { DM_ALREADY_AUTHED, SOFTBUS_OK },
    { SOFTBUS_OK, SOFTBUS_OK },
    { DM_BIND_TRUST_TARGET, DM_OK },
    { ERR_DM_SKIP_AUTHENTICATE, DM_OK },
};

int32_t DmAuthState::GetTaskTimeout(std::shared_ptr<DmAuthContext> context, const char* taskName, int32_t taskTimeOut)
{
    LOGI("GetTaskTimeout, taskName: %{public}s, authType_: %{public}d", taskName, context->authType);
    if (AUTH_TYPE_IMPORT_AUTH_CODE == context->authType) {
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
    CHECK_NULL_RETURN(OHOS::ScreenLock::ScreenLockManager::GetInstance(), isLocked);
    isLocked = OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked();
#endif
    LOGI("IsScreenLocked isLocked: %{public}d.", isLocked);
    return isLocked;
}

DmAuthScope DmAuthState::GetAuthorizedScope(int32_t bindLevel)
{
    DmAuthScope authorizedScope = DM_AUTH_SCOPE_INVALID;
    if (bindLevel == static_cast<int32_t>(APP) || bindLevel == static_cast<int32_t>(SERVICE)) {
        authorizedScope = DM_AUTH_SCOPE_APP;
    } else if (bindLevel == static_cast<int32_t>(USER)) {
        authorizedScope = DM_AUTH_SCOPE_USER;
    }
    return authorizedScope;
}

void DmAuthState::SourceFinish(std::shared_ptr<DmAuthContext> context)
{
    LOGI("SourceFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    context->listener->OnAuthResult(context->processInfo, context->peerTargetId.deviceId, context->accessee.tokenIdHash,
        GetOutputState(context->state), context->reason);
    context->listener->OnBindResult(context->processInfo, context->peerTargetId,
        GetOutputReplay(context->accesser.bundleName, context->reason),
        GetOutputState(context->state), GenerateBindResultContent(context));
    context->successFinished = true;

    if (context->reason != DM_OK && context->reason != DM_ALREADY_AUTHED && context->reason != DM_BIND_TRUST_TARGET) {
        BindFail(context);
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
    LOGI("SinkFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    context->processInfo.pkgName = context->accessee.pkgName;
    context->listener->OnSinkBindResult(context->processInfo, context->peerTargetId,
        GetOutputReplay(context->accessee.bundleName, context->reason),
        GetOutputState(context->state), GenerateBindResultContent(context));
    context->successFinished = true;
    if (context->reason != DM_OK) {
        BindFail(context);
    } else {
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
    context->authUiStateMgr->UpdateUiState(DmUiStateMsg::MSG_CANCEL_CONFIRM_SHOW);
    context->timer->DeleteAll();
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_AUTH_RESP_FINISH, context); // 发送201给source侧
}

std::string DmAuthState::GenerateBindResultContent(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, "");
    DmAccess access = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        context->accessee : context->accesser;
    JsonObject jsonObj;
    LOGE("networkId: %{public}s", GetAnonyString(access.networkId).c_str());
    jsonObj[DM_BIND_RESULT_NETWORK_ID] = access.networkId;
    if (access.deviceId.empty()) {
        jsonObj[TAG_DEVICE_ID] = "";
    } else {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        Crypto::GetUdidHash(access.deviceId, reinterpret_cast<uint8_t *>(deviceIdHash));
        jsonObj[TAG_DEVICE_ID] = deviceIdHash;
    }
    jsonObj[TAG_CONFIRM_OPERATION_V2] = context->confirmOperation;
    if (context->remainingFrozenTime != 0) {
        jsonObj[TAG_REMAINING_FROZEN_TIME] = context->remainingFrozenTime;
    }
    LOGI("remainingFrozenTime: %{public}" PRId64, context->remainingFrozenTime);
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

bool DmAuthState::NeedAgreeAcl(std::shared_ptr<DmAuthContext> context)
{
    if (context == nullptr) {
        return true;
    }
    if (context->direction == DM_AUTH_SOURCE) {
        if (context->accesser.isUserLevelAuthed) {
            LOGI("accesser user level authed");
            return false;
        }
        if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
            LOGI("accesser is authed");
            return !context->accesser.isAuthed;
        }
        if (context->IsCallingProxyAsSubject && !context->accesser.isAuthed) {
            LOGI("accesser is not authed");
            return true;
        }
        return ProxyNeedAgreeAcl(context);
    }
    if (context->accessee.isUserLevelAuthed) {
        LOGI("accessee user level authed");
        return false;
    }
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        LOGI("accessee is authed");
        return !context->accessee.isAuthed;
    }
    if (context->IsCallingProxyAsSubject && !context->accessee.isAuthed) {
        LOGI("accessee is not authed");
        return true;
    }
    return ProxyNeedAgreeAcl(context);
}

bool DmAuthState::ProxyNeedAgreeAcl(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return false;
    }
    if (context->direction == DM_AUTH_SOURCE) {
        for (const auto &app : context->subjectProxyOnes) {
            if (!app.proxyAccesser.isAuthed || app.IsNeedSetProxyRelationShip) {
                return true;
            }
        }
        return false;
    }
    for (const auto &app : context->subjectProxyOnes) {
        if (!app.proxyAccessee.isAuthed || app.IsNeedSetProxyRelationShip) {
            return true;
        }
    }
    return false;
}

bool DmAuthState::GetReuseSkId(std::shared_ptr<DmAuthContext> context, int32_t &skId)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    DistributedDeviceProfile::AccessControlProfile profile;
    GetReuseACL(context, profile);
    if (!profile.GetAccesser().GetAccesserCredentialIdStr().empty() &&
        !profile.GetAccessee().GetAccesseeCredentialIdStr().empty()) {
        if (context->direction == DM_AUTH_SOURCE) {
            if (context->accesser.deviceId == profile.GetAccesser().GetAccesserDeviceId()) {
                skId = profile.GetAccesser().GetAccesserSessionKeyId();
                context->reUseCreId = profile.GetAccesser().GetAccesserCredentialIdStr();
                context->accessee.deviceId = profile.GetAccessee().GetAccesseeDeviceId();
                return true;
            }
            skId = profile.GetAccessee().GetAccesseeSessionKeyId();
            context->reUseCreId = profile.GetAccessee().GetAccesseeCredentialIdStr();
            context->accessee.deviceId = profile.GetAccesser().GetAccesserDeviceId();
            return true;
        }
        if (context->accessee.deviceId == profile.GetAccessee().GetAccesseeDeviceId()) {
            skId = profile.GetAccessee().GetAccesseeSessionKeyId();
            context->reUseCreId = profile.GetAccessee().GetAccesseeCredentialIdStr();
            context->accesser.deviceId = profile.GetAccesser().GetAccesserDeviceId();
            return true;
        }
        skId = profile.GetAccesser().GetAccesserSessionKeyId();
        context->reUseCreId = profile.GetAccesser().GetAccesserCredentialIdStr();
        context->accesser.deviceId = profile.GetAccessee().GetAccesseeDeviceId();
        return true;
    }
    return false;
}

void DmAuthState::GetReuseACL(std::shared_ptr<DmAuthContext> context,
    DistributedDeviceProfile::AccessControlProfile &profile)
{
    if (context == nullptr) {
        return;
    }
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    if (!access.aclTypeList.empty()) {
        JsonObject aclTypeListJson;
        aclTypeListJson.Parse(access.aclTypeList);
        if (!aclTypeListJson.IsDiscarded() && aclTypeListJson.Contains("pointTopointAcl") &&
            access.aclProfiles.find(DM_POINT_TO_POINT) != access.aclProfiles.end() &&
            !access.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeCredentialIdStr().empty() &&
            !access.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserCredentialIdStr().empty()) {
            profile = access.aclProfiles[DM_POINT_TO_POINT];
            return;
        }
    }
    for (auto &app : context->subjectProxyOnes) {
        DmProxyAccess &proxyAccess = context->direction == DM_AUTH_SOURCE ? app.proxyAccesser : app.proxyAccessee;
        JsonObject aclList;
        if (!proxyAccess.aclTypeList.empty()) {
            aclList.Parse(proxyAccess.aclTypeList);
        }
        if (!aclList.IsDiscarded() && aclList.Contains("pointTopointAcl") &&
            proxyAccess.aclProfiles.find(DM_POINT_TO_POINT) != proxyAccess.aclProfiles.end() &&
            !proxyAccess.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeCredentialIdStr().empty() &&
            !proxyAccess.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserCredentialIdStr().empty()) {
            profile = proxyAccess.aclProfiles[DM_POINT_TO_POINT];
            return;
        }
    }
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
    if (!result[credId].Contains(FILED_CRED_TYPE) ||
        !result[credId][FILED_CRED_TYPE].IsNumberInteger()) {
        LOGE("credType is invalid.");
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
        return DM_SHARE;
    }
    return DM_UNKNOWN_TYPE;
}

bool DmAuthState::ValidateCredInfoStructure(const JsonItemObject &credInfo)
{
    if (!credInfo.Contains(FILED_CRED_TYPE) || !credInfo[FILED_CRED_TYPE].IsNumberInteger() ||
        !credInfo.Contains(FILED_AUTHORIZED_SCOPE) || !credInfo[FILED_AUTHORIZED_SCOPE].IsNumberInteger() ||
        !credInfo.Contains(FILED_SUBJECT) || !credInfo[FILED_SUBJECT].IsNumberInteger()) {
        LOGE("credType or authorizedScope or subject invalid.");
        return false;
    }
    return true;
}

uint32_t DmAuthState::GetCredType(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credInfo)
{
    CHECK_NULL_RETURN(context, DM_INVALIED_TYPE);
    if (!ValidateCredInfoStructure(credInfo)) {
        return DM_INVALIED_TYPE;
    }
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
    if (credType == ACCOUNT_UNRELATED && (authorizedScope == SCOPE_APP || authorizedScope == SCOPE_USER)) {
        std::vector<std::string> tokenIdHashList;
        for (std::string tokenId : appList) {
            tokenIdHashList.push_back(Crypto::GetTokenIdHash(tokenId));
        }
        GetProxyCredInfo(context, credInfo, tokenIdHashList);
        if (HaveSameTokenId(context, tokenIdHashList)) {
            return DM_POINT_TO_POINT;
        }
    }
    if (credType == ACCOUNT_UNRELATED && authorizedScope == SCOPE_USER && appList.empty()) {
        return DM_LNN;
    }
    return DM_INVALIED_TYPE;
}

int32_t DmAuthState::GetProxyCredInfo(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credInfo,
    const std::vector<std::string> &tokenIdHashList)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (context->subjectProxyOnes.empty()) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!credInfo.Contains(FILED_CRED_ID) || !credInfo[FILED_CRED_ID].IsString()) {
        return static_cast<int32_t>(DM_INVALIED_TYPE);
    }
    for (auto &app : context->subjectProxyOnes) {
        if (std::find(tokenIdHashList.begin(), tokenIdHashList.end(), app.proxyAccesser.tokenIdHash)
            != tokenIdHashList.end() &&
            std::find(tokenIdHashList.begin(), tokenIdHashList.end(), app.proxyAccessee.tokenIdHash)
            != tokenIdHashList.end()) {
            JsonObject appCredInfo(credInfo.Dump());
            appCredInfo[FILED_CRED_TYPE] = DM_POINT_TO_POINT;
            JsonObject credInfoJson;
            std::string credInfoJsonStr = context->direction == DM_AUTH_SOURCE ? app.proxyAccesser.credInfoJson :
                app.proxyAccessee.credInfoJson;
            if (!credInfoJsonStr.empty()) {
                credInfoJson.Parse(credInfoJsonStr);
            }
            credInfoJson.Insert(credInfo[FILED_CRED_ID].Get<std::string>(), appCredInfo);
            if (context->direction == DM_AUTH_SOURCE) {
                app.proxyAccesser.credInfoJson = credInfoJson.Dump();
            } else {
                app.proxyAccessee.credInfoJson = credInfoJson.Dump();
            }
        }
    }
    return DM_OK;
}

uint32_t DmAuthState::GetCredentialType(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credInfo)
{
    CHECK_NULL_RETURN(context, DM_INVALIED_TYPE);
    if (!ValidateCredInfoStructure(credInfo)) {
        return DM_INVALIED_TYPE;
    }
    return GetCredType(context, credInfo);
}

bool DmAuthState::HaveSameTokenId(std::shared_ptr<DmAuthContext> context,
    const std::vector<std::string> &tokenIdHashList)
{
    // Store the token of src and sink. The size must be greater than or equal to 2.
    if (tokenIdHashList.size() < 2) {
        LOGE("HaveSameTokenId invalid tokenList size.");
        return false;
    }
    return std::find(tokenIdHashList.begin(), tokenIdHashList.end(),
        context->accesser.tokenIdHash) != tokenIdHashList.end() &&
        std::find(tokenIdHashList.begin(), tokenIdHashList.end(),
        context->accessee.tokenIdHash) != tokenIdHashList.end();
}

int32_t DmAuthState::GetOutputState(int32_t state)
{
    LOGI("state %{public}d.", state);
    auto it = NEW_AND_OLD_STATE_MAPPING.find(static_cast<DmAuthStateType>(state));
    if (it != NEW_AND_OLD_STATE_MAPPING.end()) {
        return static_cast<int32_t>(it->second);
    }
    return static_cast<int32_t>(STATUS_DM_AUTH_DEFAULT);
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
    int32_t retryNum = 0;
    while (retryNum < GET_SYSTEMTIME_MAX_NUM) {
        if (gettimeofday(&time, nullptr) != 0) {
            retryNum++;
            LOGE("GetSysTimeMs failed. retryNum: %{public}d", retryNum);
            continue;
        }
        return (uint64_t) time.tv_sec * MS_PER_SECOND + (uint64_t)time.tv_usec / US_PER_MSECOND;
    }
    return 0;
}

void DmAuthState::DeleteAcl(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile)
{
    CHECK_NULL_VOID(context);
    LOGI("direction %{public}d.", static_cast<int32_t>(context->direction));
    CHECK_NULL_VOID(context->authMessageProcessor);
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    DmAccess &access = context->direction == DmAuthDirection::DM_AUTH_SOURCE ? context->accesser : context->accessee;
    int32_t userId = access.userId;
    int32_t sessionKeyId = access.deviceId == profile.GetAccesser().GetAccesserDeviceId() ?
        profile.GetAccesser().GetAccesserSessionKeyId() : profile.GetAccessee().GetAccesseeSessionKeyId();
    std::string credId = access.deviceId == profile.GetAccesser().GetAccesserDeviceId() ?
        profile.GetAccesser().GetAccesserCredentialIdStr() : profile.GetAccessee().GetAccesseeCredentialIdStr();
    context->authMessageProcessor->DeleteSessionKeyToDP(userId, sessionKeyId);
    JsonObject credJson;
    context->hiChainAuthConnector->QueryCredInfoByCredId(userId, credId, credJson);
    if (credJson.Contains(credId)) {
        DeleteCredential(context, userId, credJson[credId], profile);
    }
    DeviceProfileConnector::GetInstance().DeleteAccessControlById(profile.GetAccessControlId());
}

void DmAuthState::SetProcessInfo(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    DmAccess localAccess = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        context->accesser : context->accessee;
    std::vector<ProcessInfo> processInfoVec;
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
    processInfoVec.push_back(processInfo);
    if (context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        for (const auto &app : context->subjectProxyOnes) {
            ProcessInfo processInfo;
            processInfo.userId = localAccess.userId;
            processInfo.pkgName = context->direction == DmAuthDirection::DM_AUTH_SOURCE ? app.proxyAccesser.bundleName :
                app.proxyAccessee.bundleName;
            processInfoVec.push_back(processInfo);
        }
    }
    context->softbusConnector->SetProcessInfoVec(processInfoVec);
}

void DmAuthState::FilterProfilesByContext(
    std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles, std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    std::vector<DistributedDeviceProfile::AccessControlProfile> aclProfilesVec;
    for (auto &item : profiles) {
        std::string accesserDeviceIdHash = Crypto::GetUdidHash(item.GetAccesser().GetAccesserDeviceId());
        std::string accesseeDeviceIdHash = Crypto::GetUdidHash(item.GetAccessee().GetAccesseeDeviceId());
        if ((context->accesser.deviceIdHash == accesserDeviceIdHash &&
            context->accessee.deviceIdHash == accesseeDeviceIdHash &&
            context->accesser.userId == item.GetAccesser().GetAccesserUserId() &&
            context->accessee.userId == item.GetAccessee().GetAccesseeUserId()) ||
            (context->accessee.deviceIdHash == accesserDeviceIdHash &&
            context->accesser.deviceIdHash == accesseeDeviceIdHash &&
            context->accessee.userId == item.GetAccesser().GetAccesserUserId() &&
            context->accesser.userId == item.GetAccessee().GetAccesseeUserId())) {
            if (item.GetStatus() == INACTIVE) {
                item.SetStatus(ACTIVE);
                DeviceProfileConnector::GetInstance().UpdateAclStatus(item);
            }
            aclProfilesVec.push_back(item);
        }
    }
    profiles.clear();
    profiles.assign(aclProfilesVec.begin(), aclProfilesVec.end());
}

bool DmAuthState::GetSessionKey(std::shared_ptr<DmAuthContext> context)
{
    int32_t skId = 0;
    if (!GetReuseSkId(context, skId)) {
        return false;
    }
    return context->authMessageProcessor->GetSessionKey(context->accesser.userId, skId) == DM_OK;
}

bool DmAuthState::IsAclHasCredential(const DistributedDeviceProfile::AccessControlProfile &profile,
    const std::string &credInfoJson, std::string &credId)
{
    JsonObject credInfoJsonObj;
    if (!credInfoJson.empty()) {
        credInfoJsonObj.Parse(credInfoJson);
    }
    credId = profile.GetAccesser().GetAccesserCredentialIdStr();
    if (credInfoJsonObj.Contains(credId)) {
        return true;
    }
    credId = profile.GetAccessee().GetAccesseeCredentialIdStr();
    if (credInfoJsonObj.Contains(credId)) {
        return true;
    }
    return false;
}

void DmAuthState::UpdateCredInfo(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty() || context->reUseCreId.empty()) {
        return;
    }
    std::vector<std::string> tokenIds;
    bool isAuthed = context->direction == DM_AUTH_SOURCE ? context->accesser.isAuthed : context->accessee.isAuthed;
    if (context->IsCallingProxyAsSubject && !isAuthed) {
        tokenIds.push_back(std::to_string(context->accesser.tokenId));
        tokenIds.push_back(std::to_string(context->accessee.tokenId));
    }
    for (auto &app : context->subjectProxyOnes) {
        if (context->direction == DM_AUTH_SOURCE ? app.proxyAccesser.isAuthed : app.proxyAccessee.isAuthed) {
            continue;
        }
        tokenIds.push_back(std::to_string(app.proxyAccesser.tokenId));
        tokenIds.push_back(std::to_string(app.proxyAccessee.tokenId));
    }
    if (tokenIds.empty()) {
        return;
    }
    context->hiChainAuthConnector->AddTokensToCredential(context->reUseCreId, context->direction == DM_AUTH_SOURCE ?
        context->accesser.userId : context->accessee.userId, tokenIds);
}

bool DmAuthState::IsNeedBind(std::shared_ptr<DmAuthContext> context)
{
    if (context == nullptr) {
        return true;
    }
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        LOGI("no proxy");
        return context->needBind;
    }
    if (context->authType == DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE) {
        LOGI("authType is import pin code");
        return true;
    }
    if (context->needBind && context->needAgreeCredential && context->IsCallingProxyAsSubject) {
        LOGI("subject need bind");
        return context->needBind;
    }
    for (const auto &app : context->subjectProxyOnes) {
        if (app.needBind || app.IsNeedSetProxyRelationShip) {
            LOGI("proxy need bind");
            return true;
        }
    }
    return false;
}

bool DmAuthState::IsNeedAgreeCredential(std::shared_ptr<DmAuthContext> context)
{
    if (context == nullptr) {
        return true;
    }
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        LOGI("no proxy");
        return context->needAgreeCredential;
    }
    if (!context->needAgreeCredential) {
        LOGI("subject not need agree credential");
        return context->needAgreeCredential;
    }
    for (const auto &app : context->subjectProxyOnes) {
        if (!app.needAgreeCredential) {
            LOGI("proxy not need agree credential");
            return app.needAgreeCredential;
        }
    }
    return true;
}

bool DmAuthState::IsNeedAuth(std::shared_ptr<DmAuthContext> context)
{
    if (context == nullptr) {
        return true;
    }
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        LOGI("no proxy");
        return context->needAuth;
    }
    if (!context->needAuth) {
        LOGI("subject not need auth");
        return context->needAuth;
    }
    for (const auto &app : context->subjectProxyOnes) {
        if (!app.needAuth) {
            LOGI("proxy not need auth");
            return app.needAuth;
        }
    }
    return true;
}

void DmAuthState::DeleteCredential(std::shared_ptr<DmAuthContext> context, int32_t userId,
    const JsonItemObject &credInfo, const DistributedDeviceProfile::AccessControlProfile &profile)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (!credInfo.Contains(FILED_CRED_ID) || !credInfo[FILED_CRED_ID].IsString()) {
        return;
    }
    if (!credInfo.Contains(FILED_AUTHORIZED_APP_LIST)) {
        context->hiChainAuthConnector->DeleteCredential(userId, credInfo[FILED_CRED_ID].Get<std::string>());
        return;
    }
    std::vector<std::string> appList;
    credInfo[FILED_AUTHORIZED_APP_LIST].Get(appList);
    auto erIt = std::find(appList.begin(), appList.end(), std::to_string(profile.GetAccesser().GetAccesserTokenId()));
    if (erIt != appList.end()) {
        appList.erase(erIt);
    }
    auto eeIt = std::find(appList.begin(), appList.end(), std::to_string(profile.GetAccessee().GetAccesseeTokenId()));
    if (eeIt != appList.end()) {
        appList.erase(eeIt);
    }
    if (appList.size() == 0) {
        context->hiChainAuthConnector->DeleteCredential(userId, credInfo[FILED_CRED_ID].Get<std::string>());
        return;
    }
    context->hiChainAuthConnector->UpdateCredential(credInfo[FILED_CRED_ID].Get<std::string>(), userId, appList);
}

void DmAuthState::DirectlyDeleteCredential(std::shared_ptr<DmAuthContext> context, int32_t userId,
    const JsonItemObject &credInfo)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (!credInfo.Contains(FILED_CRED_ID) || !credInfo[FILED_CRED_ID].IsString()) {
        return;
    }
    context->hiChainAuthConnector->DeleteCredential(userId, credInfo[FILED_CRED_ID].Get<std::string>());
}

void DmAuthState::DeleteAclAndSk(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->authMessageProcessor);
    DmAccess &access = context->direction == DmAuthDirection::DM_AUTH_SOURCE ? context->accesser : context->accessee;
    int32_t userId = access.userId;
    int32_t sessionKeyId = access.deviceId == profile.GetAccesser().GetAccesserDeviceId() ?
        profile.GetAccesser().GetAccesserSessionKeyId() : profile.GetAccessee().GetAccesseeSessionKeyId();
    context->authMessageProcessor->DeleteSessionKeyToDP(userId, sessionKeyId);
    DeviceProfileConnector::GetInstance().DeleteAccessControlById(profile.GetAccessControlId());
}

void DmAuthState::GetPeerDeviceId(std::shared_ptr<DmAuthContext> context, std::string &peerDeviceId)
{
    CHECK_NULL_VOID(context);
    if (context->accesser.aclProfiles.find(DM_IDENTICAL_ACCOUNT) != context->accesser.aclProfiles.end()) {
        peerDeviceId = context->accesser.aclProfiles[DM_IDENTICAL_ACCOUNT].GetAccessee().GetAccesseeDeviceId();
        if (!peerDeviceId.empty()) {
            return;
        }
    }
    if (context->accesser.aclProfiles.find(DM_SHARE) != context->accesser.aclProfiles.end()) {
        peerDeviceId = context->accesser.aclProfiles[DM_SHARE].GetAccessee().GetAccesseeDeviceId();
        if (peerDeviceId == context->accesser.deviceId) {
            peerDeviceId = context->accesser.aclProfiles[DM_SHARE].GetAccesser().GetAccesserDeviceId();
        }
        if (!peerDeviceId.empty()) {
            return;
        }
    }
    if (context->accesser.aclProfiles.find(DM_POINT_TO_POINT) != context->accesser.aclProfiles.end()) {
        peerDeviceId = context->accesser.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeDeviceId();
        if (peerDeviceId == context->accesser.deviceId) {
            peerDeviceId = context->accesser.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserDeviceId();
        }
        if (!peerDeviceId.empty()) {
            return;
        }
    }
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto &app : context->subjectProxyOnes) {
        if (app.proxyAccesser.aclProfiles.find(DM_POINT_TO_POINT) != app.proxyAccesser.aclProfiles.end()) {
            peerDeviceId = app.proxyAccesser.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeDeviceId();
            if (peerDeviceId == context->accesser.deviceId) {
                peerDeviceId = app.proxyAccesser.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserDeviceId();
            }
            if (!peerDeviceId.empty()) {
                return;
            }
        }
    }
    LOGE("failed");
}

bool DmAuthState::IsMatchCredentialAndP2pACL(JsonObject &credInfo, std::string &credId,
    const DistributedDeviceProfile::AccessControlProfile &profile)
{
    if (!credInfo.Contains(credId) || !credInfo[credId].Contains(FILED_AUTHORIZED_SCOPE) ||
        !credInfo[credId][FILED_AUTHORIZED_SCOPE].IsNumberInteger()) {
        return false;
    }
    int32_t authorizedScope = credInfo[credId][FILED_AUTHORIZED_SCOPE].Get<int32_t>();
    if (authorizedScope == static_cast<int32_t>(DM_AUTH_SCOPE_USER) && profile.GetBindLevel() == USER) {
        return true;
    }
    if (authorizedScope == static_cast<int32_t>(DM_AUTH_SCOPE_APP) &&
        (profile.GetBindLevel() == SERVICE || profile.GetBindLevel() == APP)) {
        return true;
    }
    return false;
}

void DmAuthState::BindFail(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (context->reason == DM_BIND_TRUST_TARGET) {
        return;
    }
    bool isDelLnnAcl = false;
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    if (access.isGeneratedLnnCredThisBind) {
        if (!access.lnnCredentialId.empty()) {
            context->hiChainAuthConnector->DeleteCredential(access.userId, access.lnnCredentialId);
        }
        if (access.lnnSessionKeyId != 0) {
            DeviceProfileConnector::GetInstance().DeleteSessionKey(access.userId, access.lnnSessionKeyId);
        }
        isDelLnnAcl = true;
    }
    std::vector<std::pair<int64_t, int64_t>> tokenIds;
    if (!access.isAuthed && access.transmitSessionKeyId != 0) {
        DeviceProfileConnector::GetInstance().DeleteSessionKey(access.userId, access.transmitSessionKeyId);
        tokenIds.push_back(std::make_pair(context->accesser.tokenId, context->accessee.tokenId));
    }
    if (context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        for (auto &app : context->subjectProxyOnes) {
            DmProxyAccess &proxyAccess = context->direction == DM_AUTH_SOURCE ? app.proxyAccesser : app.proxyAccessee;
            if (proxyAccess.isAuthed || proxyAccess.transmitSessionKeyId == 0) {
                continue;
            }
            DeviceProfileConnector::GetInstance().DeleteSessionKey(access.userId, proxyAccess.transmitSessionKeyId);
            tokenIds.push_back(std::make_pair(app.proxyAccesser.tokenId, app.proxyAccessee.tokenId));
        }
    }
    if (access.isGeneratedTransmitThisBind && !access.transmitCredentialId.empty()) {
        context->hiChainAuthConnector->DeleteCredential(access.userId, access.transmitCredentialId);
    } else if (!context->reUseCreId.empty()) {
        RemoveTokenIdsFromCredential(context, context->reUseCreId, tokenIds);
    } else {
        LOGE("no credential");
    }
    DeleteAcl(context, isDelLnnAcl, tokenIds);
}

void DmAuthState::DeleteAcl(std::shared_ptr<DmAuthContext> context, bool isDelLnnAcl,
    std::vector<std::pair<int64_t, int64_t>> &tokenIds)
{
    CHECK_NULL_VOID(context);
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    if (remoteAccess.deviceId.empty()) {
        return;
    }
    if (!isDelLnnAcl && tokenIds.empty()) {
        return;
    }
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls =
        DeviceProfileConnector::GetInstance().GetAclList(access.deviceId, access.userId,
        remoteAccess.deviceId, remoteAccess.userId);
    for (DistributedDeviceProfile::AccessControlProfile acl : acls) {
        if (isDelLnnAcl && DeviceProfileConnector::GetInstance().IsLnnAcl(acl)) {
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(acl.GetAccessControlId());
            continue;
        }
        auto it = std::find(tokenIds.begin(), tokenIds.end(),
            std::make_pair(acl.GetAccesser().GetAccesserTokenId(), acl.GetAccessee().GetAccesseeTokenId()));
        if (it != tokenIds.end()) {
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(acl.GetAccessControlId());
            continue;
        }
    }
}

void DmAuthState::RemoveTokenIdsFromCredential(std::shared_ptr<DmAuthContext> context, const std::string &credId,
    std::vector<std::pair<int64_t, int64_t>> &tokenIds)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    JsonObject credJson;
    context->hiChainAuthConnector->QueryCredInfoByCredId(access.userId, credId, credJson);
    if (!credJson.Contains(credId)) {
        LOGE("query cred failed");
        return;
    }
    if (!credJson[credId].Contains(FILED_AUTHORIZED_APP_LIST)) {
        LOGE("applist is empty");
        context->hiChainAuthConnector->DeleteCredential(access.userId, credId);
        return;
    }
    std::vector<std::string> appList;
    credJson[credId][FILED_AUTHORIZED_APP_LIST].Get(appList);
    for (const auto& it : tokenIds) {
        auto erIt = std::find(appList.begin(), appList.end(), std::to_string(it.first));
        if (erIt != appList.end()) {
            appList.erase(erIt);
        }
        auto eeIt = std::find(appList.begin(), appList.end(), std::to_string(it.second));
        if (eeIt != appList.end()) {
            appList.erase(eeIt);
        }
    }
    if (appList.size() == 0) {
        LOGE("applist is empty, delete credential");
        context->hiChainAuthConnector->DeleteCredential(access.userId, credId);
        return;
    }
    context->hiChainAuthConnector->UpdateCredential(credId, access.userId, appList);
}

void DmAuthState::JoinLnn(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->softbusConnector);
    bool isForceJoin = false;
    if (!context->accesser.isOnline) {
        LOGI("The remote device is offline.");
        isForceJoin = true;
    }
    if (context->connSessionType == CONN_SESSION_TYPE_HML || context->isServiceBind) {
        context->softbusConnector->JoinLnnByHml(context->sessionId, context->accesser.transmitSessionKeyId,
            context->accessee.transmitSessionKeyId, isForceJoin);
    } else {
        char udidHashTmp[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetUdidHash(context->accessee.deviceId, reinterpret_cast<uint8_t*>(udidHashTmp)) == DM_OK) {
            std::string peerUdidHash = std::string(udidHashTmp);
            context->softbusConnector->JoinLNNBySkId(context->sessionId, context->accesser.transmitSessionKeyId,
                context->accessee.transmitSessionKeyId, context->accessee.addr, peerUdidHash, isForceJoin);
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
