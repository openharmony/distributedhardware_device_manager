/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "dm_auth_manager.h"

#include <algorithm>
#include <mutex>
#include <string>
#include <unistd.h>

#include "iservice_registry.h"
#if defined(SUPPORT_SCREENLOCK)
#include "screenlock_manager.h"
#endif

#include "app_manager.h"
#include "auth_message_processor.h"
#include "common_event_support.h"
#include "dm_ability_manager.h"
#include "dm_anonymous.h"
#include "dm_config_manager.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_dialog_manager.h"
#include "dm_language_manager.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "dm_random.h"
#include "multiple_user_connector.h"
#include "json_object.h"
#include "openssl/sha.h"
#include "parameter.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "multiple_user_connector.h"
#endif

namespace OHOS {
namespace DistributedHardware {
const int32_t CANCEL_PIN_CODE_DISPLAY = 1;
const int32_t DEVICE_ID_HALF = 2;
const int32_t MAX_AUTH_TIMES = 3;
const int32_t MIN_PIN_TOKEN = 10000000;
const int32_t MAX_PIN_TOKEN = 90000000;
const int32_t MIN_PIN_CODE = 100000;
const int32_t MAX_PIN_CODE = 999999;
const int32_t DM_AUTH_TYPE_MAX = 6;
const int32_t DM_AUTH_TYPE_MIN = 0;
const int32_t AUTH_SESSION_SIDE_SERVER = 0;
const int32_t USLEEP_TIME_US_500000 = 500000; // 500ms
const int32_t AUTH_DEVICE_TIMEOUT = 10;
const int32_t ALREADY_BIND = 1;
const int32_t STRTOLL_BASE_10 = 10;
const int32_t MAX_PUT_SESSIONKEY_TIMEOUT = 100; //ms
const int32_t SESSION_CLOSE_TIMEOUT = 2;
const char* IS_NEED_JOIN_LNN = "IsNeedJoinLnn";
constexpr const char* NEED_JOIN_LNN = "0";
constexpr const char* NO_NEED_JOIN_LNN = "1";

// clone task timeout map
const std::map<std::string, int32_t> TASK_TIME_OUT_MAP = {
    { std::string(AUTHENTICATE_TIMEOUT_TASK), CLONE_AUTHENTICATE_TIMEOUT },
    { std::string(NEGOTIATE_TIMEOUT_TASK), CLONE_NEGOTIATE_TIMEOUT },
    { std::string(CONFIRM_TIMEOUT_TASK), CLONE_CONFIRM_TIMEOUT },
    { std::string(ADD_TIMEOUT_TASK), CLONE_ADD_TIMEOUT },
    { std::string(WAIT_NEGOTIATE_TIMEOUT_TASK), CLONE_WAIT_NEGOTIATE_TIMEOUT },
    { std::string(WAIT_REQUEST_TIMEOUT_TASK), CLONE_WAIT_REQUEST_TIMEOUT },
    { std::string(SESSION_HEARTBEAT_TIMEOUT_TASK), CLONE_SESSION_HEARTBEAT_TIMEOUT }
};
const std::map<AuthState, DmAuthStatus> OLD_STATE_MAPPING = {
    { AuthState::AUTH_REQUEST_INIT, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_NEGOTIATE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_NEGOTIATE_DONE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_REPLY, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_JOIN, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_NETWORK, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_FINISH, DmAuthStatus::STATUS_DM_AUTH_FINISH },
    { AuthState::AUTH_REQUEST_CREDENTIAL, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_CREDENTIAL_DONE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_AUTH_FINISH, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_RECHECK_MSG, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_REQUEST_RECHECK_MSG_DONE, DmAuthStatus::STATUS_DM_AUTH_DEFAULT },
    { AuthState::AUTH_RESPONSE_FINISH, DmAuthStatus::STATUS_DM_SINK_AUTH_FINISH }
};
constexpr int32_t PROCESS_NAME_WHITE_LIST_NUM = 1;
constexpr const static char* PROCESS_NAME_WHITE_LIST[PROCESS_NAME_WHITE_LIST_NUM] = {
    "com.example.myapplication"
};

constexpr const char* DM_VERSION_4_1_5_1 = "4.1.5.1";
std::mutex g_authFinishLock;

DmAuthManager::DmAuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                             std::shared_ptr<HiChainConnector> hiChainConnector,
                             std::shared_ptr<IDeviceManagerServiceListener> listener,
                             std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
    : softbusConnector_(softbusConnector), hiChainConnector_(hiChainConnector), listener_(listener),
      hiChainAuthConnector_(hiChainAuthConnector)
{
    LOGI("DmAuthManager constructor");
    DmConfigManager &dmConfigManager = DmConfigManager::GetInstance();
    dmConfigManager.GetAuthAdapter(authenticationMap_);
    authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener_);
    authenticationMap_[AUTH_TYPE_IMPORT_AUTH_CODE] = nullptr;
    authenticationMap_[AUTH_TYPE_CRE] = nullptr;
    authenticationMap_[AUTH_TYPE_NFC] = nullptr;
    dmVersion_ = DM_VERSION_5_0_5;
}

DmAuthManager::~DmAuthManager()
{
    LOGI("DmAuthManager destructor");
}

bool DmAuthManager::IsHmlSessionType()
{
    CHECK_NULL_RETURN(authRequestContext_, false);
    return authRequestContext_->connSessionType == CONN_SESSION_TYPE_HML;
}

int32_t DmAuthManager::CheckAuthParamVaild(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("start.");
    if (authType < DM_AUTH_TYPE_MIN || authType > DM_AUTH_TYPE_MAX) {
        LOGE("CheckAuthParamVaild failed, authType is illegal.");
        return ERR_DM_AUTH_FAILED;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DmAuthManager::CheckAuthParamVaild failed, pkgName is %{public}s, deviceId is %{public}s, extra is"
            "%{public}s.", pkgName.c_str(), GetAnonyString(deviceId).c_str(), extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (listener_ == nullptr || authUiStateMgr_ == nullptr) {
        LOGE("DmAuthManager::CheckAuthParamVaild listener or authUiStateMgr is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (!IsAuthTypeSupported(authType)) {
        LOGE("DmAuthManager::CheckAuthParamVaild authType %{public}d not support.", authType);
        listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT,
            ERR_DM_UNSUPPORTED_AUTH_TYPE);
        listener_->OnBindResult(processInfo_, peerTargetId_, ERR_DM_UNSUPPORTED_AUTH_TYPE, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_UNSUPPORTED_AUTH_TYPE;
    }

    if (authRequestState_ != nullptr || authResponseState_ != nullptr) {
        LOGE("DmAuthManager::CheckAuthParamVaild %{public}s is request authentication.", pkgName.c_str());
        return ERR_DM_AUTH_BUSINESS_BUSY;
    }

    if ((authType == AUTH_TYPE_IMPORT_AUTH_CODE || authType == AUTH_TYPE_NFC) && (!IsAuthCodeReady(pkgName))) {
        LOGE("Auth code not exist.");
        listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT,
            ERR_DM_INPUT_PARA_INVALID);
        listener_->OnBindResult(processInfo_, peerTargetId_, ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}

int32_t DmAuthManager::CheckAuthParamVaildExtra(const std::string &extra, const std::string &deviceId)
{
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_INPUT_PARA_INVALID);
    JsonObject jsonObject(extra);
    if ((jsonObject.IsDiscarded() || !IsString(jsonObject, PARAM_KEY_CONN_SESSIONTYPE) ||
        jsonObject[PARAM_KEY_CONN_SESSIONTYPE].Get<std::string>() != CONN_SESSION_TYPE_HML) &&
        !softbusConnector_->HaveDeviceInMap(deviceId)) {
        LOGE("CheckAuthParamVaild failed, the discoveryDeviceInfoMap_ not have this device.");
        CHECK_NULL_RETURN(listener_, ERR_DM_INPUT_PARA_INVALID);
        listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT,
            ERR_DM_INPUT_PARA_INVALID);
        listener_->OnBindResult(processInfo_, peerTargetId_, ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (jsonObject.IsDiscarded()) {
        return DM_OK;
    }
    std::string connSessionType;
    if (IsString(jsonObject, PARAM_KEY_CONN_SESSIONTYPE)) {
        connSessionType = jsonObject[PARAM_KEY_CONN_SESSIONTYPE].Get<std::string>();
    }
    if (connSessionType == CONN_SESSION_TYPE_HML && !CheckHmlParamValid(jsonObject)) {
        LOGE("CONN_SESSION_TYPE_HML, CheckHmlParamValid failed");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (jsonObject.IsDiscarded() || !jsonObject.Contains(TAG_BIND_LEVEL)) {
        return DM_OK;
    }
    return DM_OK;
}

bool DmAuthManager::CheckHmlParamValid(JsonObject &jsonObject)
{
    if (!IsString(jsonObject, PARAM_KEY_HML_ACTIONID)) {
        LOGE("PARAM_KEY_HML_ACTIONID is not string");
        return false;
    }
    std::string actionIdStr = jsonObject[PARAM_KEY_HML_ACTIONID].Get<std::string>();
    if (!IsNumberString(actionIdStr)) {
        LOGE("PARAM_KEY_HML_ACTIONID is not number");
        return false;
    }
    int32_t actionId = std::atoi(actionIdStr.c_str());
    if (actionId <= 0) {
        LOGE("PARAM_KEY_HML_ACTIONID is <= 0");
        return false;
    }
    return true;
}

void DmAuthManager::GetAuthParam(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("Get auth param.");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(softbusConnector_);
    authRequestContext_->hostPkgName = pkgName;
    authRequestContext_->authType = authType;
    authRequestContext_->localDeviceName = softbusConnector_->GetLocalDeviceName();
    authRequestContext_->localDeviceTypeId = softbusConnector_->GetLocalDeviceTypeId();
    authRequestContext_->localDeviceId = localUdid;
    authRequestContext_->deviceId = deviceId;
    authRequestContext_->addr = deviceId;
    authRequestContext_->dmVersion = DM_VERSION_5_0_5;
    authRequestContext_->localUserId = MultipleUserConnector::GetFirstForegroundUserId();
    authRequestContext_->localAccountId =
        MultipleUserConnector::GetOhosAccountIdByUserId(authRequestContext_->localUserId);
    authRequestContext_->isOnline = false;
    authRequestContext_->authed = !authRequestContext_->bindType.empty();
    authRequestContext_->bindLevel = INVALIED_TYPE;
    JsonObject jsonObject(extra);
    if (jsonObject.IsDiscarded()) {
        LOGE("extra string not a json type.");
        return;
    }
    ParseJsonObject(jsonObject);
    authRequestContext_->token = std::to_string(GenRandInt(MIN_PIN_TOKEN, MAX_PIN_TOKEN));
}

void DmAuthManager::ParseJsonObject(JsonObject &jsonObject)
{
    CHECK_NULL_VOID(authRequestContext_);
    if (!jsonObject.IsDiscarded()) {
        if (IsString(jsonObject, TARGET_PKG_NAME_KEY)) {
            authRequestContext_->targetPkgName = jsonObject[TARGET_PKG_NAME_KEY].Get<std::string>();
        }
        if (IsString(jsonObject, APP_OPERATION_KEY)) {
            authRequestContext_->appOperation = jsonObject[APP_OPERATION_KEY].Get<std::string>();
        }
        if (IsString(jsonObject, CUSTOM_DESCRIPTION_KEY)) {
            authRequestContext_->customDesc = DmLanguageManager::GetInstance().
                GetTextBySystemLanguage(jsonObject[CUSTOM_DESCRIPTION_KEY].Get<std::string>());
        }
        if (IsString(jsonObject, TAG_APP_THUMBNAIL2)) {
            authRequestContext_->appThumbnail = jsonObject[TAG_APP_THUMBNAIL2].Get<std::string>();
        }
        authRequestContext_->closeSessionDelaySeconds = 0;
        if (IsString(jsonObject, PARAM_CLOSE_SESSION_DELAY_SECONDS)) {
            std::string delaySecondsStr = jsonObject[PARAM_CLOSE_SESSION_DELAY_SECONDS].Get<std::string>();
            authRequestContext_->closeSessionDelaySeconds = GetCloseSessionDelaySeconds(delaySecondsStr);
        }
        if (IsString(jsonObject, TAG_PEER_BUNDLE_NAME)) {
            authRequestContext_->peerBundleName = jsonObject[TAG_PEER_BUNDLE_NAME].Get<std::string>();
            if (authRequestContext_->peerBundleName == "") {
                authRequestContext_->peerBundleName = authRequestContext_->hostPkgName;
            }
            LOGI("ParseJsonObject peerBundleName = %{public}s", authRequestContext_->peerBundleName.c_str());
        } else {
            authRequestContext_->peerBundleName = authRequestContext_->hostPkgName;
        }
        ParseHmlInfoInJsonObject(jsonObject);
    }
}

void DmAuthManager::ParseHmlInfoInJsonObject(JsonObject &jsonObject)
{
    CHECK_NULL_VOID(authRequestContext_);
    if (IsString(jsonObject, PARAM_KEY_CONN_SESSIONTYPE)) {
        authRequestContext_->connSessionType = jsonObject[PARAM_KEY_CONN_SESSIONTYPE].Get<std::string>();
        LOGI("connSessionType %{public}s", authRequestContext_->connSessionType.c_str());
    }
    if (!IsHmlSessionType()) {
        return;
    }
    if (IsString(jsonObject, PARAM_KEY_HML_ACTIONID)) {
        std::string actionIdStr = jsonObject[PARAM_KEY_HML_ACTIONID].Get<std::string>();
        if (IsNumberString(actionIdStr)) {
            authRequestContext_->hmlActionId = std::atoi(actionIdStr.c_str());
        }
        if (authRequestContext_->hmlActionId <= 0) {
            authRequestContext_->hmlActionId = 0;
        }
        LOGI("hmlActionId %{public}d", authRequestContext_->hmlActionId);
    }
}

int32_t DmAuthManager::GetCloseSessionDelaySeconds(std::string &delaySecondsStr)
{
    if (!IsNumberString(delaySecondsStr)) {
        LOGE("Invalid parameter, param is not number.");
        return 0;
    }
    const int32_t CLOSE_SESSION_DELAY_SECONDS_MAX = 10;
    int32_t delaySeconds = std::atoi(delaySecondsStr.c_str());
    if (delaySeconds < 0 || delaySeconds > CLOSE_SESSION_DELAY_SECONDS_MAX) {
        LOGE("Invalid parameter, param out of range.");
        return 0;
    }
    return delaySeconds;
}

void DmAuthManager::InitAuthState(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    auto iter = authenticationMap_.find(authType);
    if (iter != authenticationMap_.end()) {
        authPtr_ = iter->second;
    }

    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK),
        GetTaskTimeout(AUTHENTICATE_TIMEOUT_TASK, AUTHENTICATE_TIMEOUT), [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
    authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(shared_from_this());
    authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    JsonObject jsonObject(extra);
    if (jsonObject.IsDiscarded()) {
        LOGE("extra string not a json type.");
        return;
    }
    
    GetAuthParam(pkgName, authType, deviceId, extra);
    authMessageProcessor_->SetRequestContext(authRequestContext_);
    authRequestState_ = std::make_shared<AuthRequestInitState>();
    authRequestState_->SetAuthManager(shared_from_this());
    authRequestState_->SetAuthContext(authRequestContext_);
    if (!DmRadarHelper::GetInstance().ReportAuthStart(peerTargetId_.deviceId, pkgName)) {
        LOGE("ReportAuthStart failed");
    }
    GetBindCallerInfo();
    authRequestState_->Enter();
    LOGI("DmAuthManager::AuthenticateDevice complete");
}

int32_t DmAuthManager::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("start auth type %{public}d.", authType);
    processInfo_.pkgName = pkgName;
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        if (bindParam_.find("bindCallerUserId") != bindParam_.end()) {
            processInfo_.userId = std::atoi(bindParam_["bindCallerUserId"].c_str());
        }
    }
    SetAuthType(authType);
    int32_t ret = CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    if (ret != DM_OK) {
        LOGE("DmAuthManager::AuthenticateDevice failed, param is invaild.");
        listener_->OnBindResult(processInfo_, peerTargetId_, ret, STATUS_DM_AUTH_DEFAULT, "");
        return ret;
    }
    ret = CheckAuthParamVaildExtra(extra, deviceId);
    if (ret != DM_OK) {
        LOGE("CheckAuthParamVaildExtra failed, param is invaild.");
        listener_->OnBindResult(processInfo_, peerTargetId_, ret, STATUS_DM_AUTH_DEFAULT, "");
        return ret;
    }
    isAuthenticateDevice_ = true;
    if (authType == AUTH_TYPE_CRE) {
        LOGI("DmAuthManager::AuthenticateDevice for credential type, joinLNN directly.");
        softbusConnector_->JoinLnn(deviceId, true);
        listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT, DM_OK);
        listener_->OnBindResult(processInfo_, peerTargetId_, DM_OK, STATUS_DM_AUTH_DEFAULT, "");
    }
    InitAuthState(pkgName, authType, deviceId, extra);
    return DM_OK;
}

int32_t DmAuthManager::UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    struct RadarInfo info = {
        .funcName = "UnAuthenticateDevice",
        .toCallPkg = HICHAINNAME,
        .hostName = pkgName,
        .peerUdid = udid,
    };
    if (!DmRadarHelper::GetInstance().ReportDeleteTrustRelation(info)) {
        LOGE("ReportDeleteTrustRelation failed");
    }
    remoteDeviceId_ = udid;
    if (static_cast<uint32_t>(bindLevel) == USER) {
        DeleteGroup(pkgName, udid);
    }
    std::string extra = "";
    return DeleteAcl(pkgName, std::string(localDeviceId), udid, bindLevel, extra);
}

int32_t DmAuthManager::StopAuthenticateDevice(const std::string &pkgName)
{
    if (pkgName.empty() || authRequestContext_ == nullptr || authResponseContext_ == nullptr) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (((authRequestState_!= nullptr && authRequestContext_->hostPkgName == pkgName) ||
        (authResponseContext_ != nullptr && authResponseContext_->hostPkgName == pkgName)) &&
        isAuthenticateDevice_) {
        LOGI("Stop previous AuthenticateDevice.");
        authRequestContext_->reason = STOP_BIND;
        authResponseContext_->state = authRequestState_->GetStateType();
        authResponseContext_->reply = STOP_BIND;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    }
    return DM_OK;
}

int32_t DmAuthManager::DeleteAcl(const std::string &pkgName, const std::string &localUdid,
    const std::string &remoteUdid, int32_t bindLevel, const std::string &extra)
{
    LOGI("DeleteAcl pkgName %{public}s, localUdid %{public}s, remoteUdid %{public}s, bindLevel %{public}d.",
        pkgName.c_str(), GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), bindLevel);
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localUdid, remoteUdid, bindLevel, extra);
    if (offlineParam.bindType == INVALIED_TYPE) {
        LOGE("Acl not contain the pkgname bind data.");
        return ERR_DM_FAILED;
    }
    if (static_cast<uint32_t>(bindLevel) == APP) {
        ProcessInfo processInfo;
        processInfo.pkgName = pkgName;
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        CHECK_NULL_RETURN(softbusConnector_, ERR_DM_FAILED);
        if (offlineParam.leftAclNumber != 0) {
            LOGI("The pkgName unbind app-level type leftAclNumber not zero.");
            softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
            softbusConnector_->HandleDeviceOffline(remoteUdid);
            return DM_OK;
        }
        if (offlineParam.leftAclNumber == 0) {
            LOGI("The pkgName unbind app-level type leftAclNumber is zero.");
            softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
            CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_FAILED);
            hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
                offlineParam.peerUserId);
            return DM_OK;
        }
    }
    if (static_cast<uint32_t>(bindLevel) == USER && offlineParam.leftAclNumber != 0) {
        LOGI("Unbind deivce-level, retain identical account bind type.");
        return DM_OK;
    }
    if (static_cast<uint32_t>(bindLevel) == USER && offlineParam.leftAclNumber == 0) {
        LOGI("Unbind deivce-level, retain null.");
        CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_FAILED);
        hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
            offlineParam.peerUserId);
        return DM_OK;
    }
    return ERR_DM_FAILED;
}

int32_t DmAuthManager::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    if (static_cast<uint32_t>(bindLevel) == USER) {
        DeleteGroup(pkgName, udid);
    }
    return DeleteAcl(pkgName, std::string(localDeviceId), udid, bindLevel, extra);
}

void DmAuthManager::GetPeerUdidHash(int32_t sessionId, std::string &peerUdidHash)
{
    CHECK_NULL_VOID(softbusConnector_);
    std::string peerUdid = "";
    int32_t ret = softbusConnector_->GetSoftbusSession()->GetPeerDeviceId(sessionId, peerUdid);
    if (ret != DM_OK) {
        LOGE("DmAuthManager::GetPeerUdidHash failed.");
        peerUdidHash = "";
        return;
    }
    char udidHashTmp[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(peerUdid, reinterpret_cast<uint8_t *>(udidHashTmp)) != DM_OK) {
        LOGE("get udidhash by udid: %{public}s failed.", GetAnonyString(peerUdid).c_str());
        peerUdidHash = "";
        return;
    }
    peerUdidHash = std::string(udidHashTmp);
}

void DmAuthManager::DeleteOffLineTimer(int32_t sessionId)
{
    GetPeerUdidHash(sessionId, remoteUdidHash_);
    if (remoteUdidHash_.empty()) {
        LOGE("DeleteOffLineTimer remoteUdidHash is empty.");
        return;
    }
    if (softbusConnector_ != nullptr) {
        softbusConnector_->DeleteOffLineTimer(remoteUdidHash_);
    }
}

void DmAuthManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
    DeleteOffLineTimer(sessionId);
    if (sessionSide == AUTH_SESSION_SIDE_SERVER) {
        if (authResponseState_ == nullptr && authRequestState_ == nullptr) {
            authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(shared_from_this());
            authResponseState_ = std::make_shared<AuthResponseInitState>();
            authResponseState_->SetAuthManager(shared_from_this());
            authResponseState_->Enter();
            authResponseContext_ = std::make_shared<DmAuthResponseContext>();
            authResponseContext_->sessionId = sessionId;
            if (timer_ == nullptr) {
                timer_ = std::make_shared<DmTimer>();
            }
            timer_->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK),
                GetTaskTimeout(AUTHENTICATE_TIMEOUT_TASK, AUTHENTICATE_TIMEOUT), [this] (std::string name) {
                    DmAuthManager::HandleAuthenticateTimeout(name);
                });
            timer_->StartTimer(std::string(WAIT_NEGOTIATE_TIMEOUT_TASK),
                GetTaskTimeout(WAIT_NEGOTIATE_TIMEOUT_TASK, WAIT_NEGOTIATE_TIMEOUT), [this] (std::string name) {
                    DmAuthManager::HandleAuthenticateTimeout(name);
                });
        } else {
            std::shared_ptr<AuthMessageProcessor> authMessageProcessor =
                std::make_shared<AuthMessageProcessor>(shared_from_this());
            std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
            authResponseContext->reply = ERR_DM_AUTH_BUSINESS_BUSY;
            authMessageProcessor->SetResponseContext(authResponseContext);
            std::string message = authMessageProcessor->CreateSimpleMessage(MSG_TYPE_REQ_AUTH_TERMINATE);
            softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
            softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
        }
    } else {
        if (authResponseState_ == nullptr && authRequestState_ != nullptr &&
            authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_INIT) {
            CHECK_NULL_VOID(authRequestContext_);
            authRequestContext_->sessionId = sessionId;
            authResponseContext_->sessionId = sessionId;
            authMessageProcessor_->SetRequestContext(authRequestContext_);
            authRequestState_->SetAuthContext(authRequestContext_);
            authRequestState_->TransitionTo(std::make_shared<AuthRequestNegotiateState>());
            struct RadarInfo info = { .funcName = "OnSessionOpened" };
            info.channelId = sessionId;
            DmRadarHelper::GetInstance().ReportAuthSendRequest(info);
        } else {
            softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
            LOGE("DmAuthManager::OnSessionOpened but request state is wrong");
        }
    }
}

void DmAuthManager::OnSessionClosed(const int32_t sessionId)
{
    LOGI("DmAuthManager::OnSessionClosed sessionId = %{public}d", sessionId);
    if (authResponseState_ != nullptr && authResponseContext_ != nullptr) {
        {
            std::lock_guard<std::mutex> lock(groupMutex_);
            if (authResponseState_->GetStateType() == AUTH_RESPONSE_SHOW &&
                authResponseContext_->reply == DM_OK && isCreateGroup_ && !isAddMember_) {
                LOGI("wait addmemer callback");
                return;
            }
        }
        isFinishOfLocal_ = false;
        authResponseContext_->state = authResponseState_->GetStateType();
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
    }
}

void DmAuthManager::ClearSoftbusSessionCallback()
{
    LOGI("DmAuthManager ClearSoftbusSessionCallback");
    if (softbusConnector_ != nullptr && softbusConnector_->GetSoftbusSession() != nullptr) {
        softbusConnector_->GetSoftbusSession()->UnRegisterSessionCallback();
    }
}

void DmAuthManager::PrepareSoftbusSessionCallback()
{
    LOGI("DmAuthManager PrepareSoftbusSessionCallback");
    if (softbusConnector_ != nullptr && softbusConnector_->GetSoftbusSession() != nullptr) {
        softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(shared_from_this());
    }
}

void DmAuthManager::ProcessSourceMsg()
{
    CHECK_NULL_VOID(authMessageProcessor_);
    CHECK_NULL_VOID(authRequestState_);
    CHECK_NULL_VOID(authResponseContext_);
    authRequestContext_ = authMessageProcessor_->GetRequestContext();
    authRequestState_->SetAuthContext(authRequestContext_);
    LOGI("OnDataReceived for source device, authResponseContext msgType = %{public}d, authRequestState stateType ="
        "%{public}d", authResponseContext_->msgType, authRequestState_->GetStateType());

    switch (authResponseContext_->msgType) {
        case MSG_TYPE_RESP_AUTH:
        case MSG_TYPE_RESP_AUTH_EXT:
            if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_NEGOTIATE_DONE) {
                authRequestState_->TransitionTo(std::make_shared<AuthRequestReplyState>());
            }
            break;
        case MSG_TYPE_RESP_NEGOTIATE:
            if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_NEGOTIATE) {
                authRequestState_->TransitionTo(std::make_shared<AuthRequestNegotiateDoneState>());
            }
            break;
        case MSG_TYPE_REQ_AUTH_TERMINATE:
            if (authRequestState_->GetStateType() != AuthState::AUTH_REQUEST_FINISH) {
                isFinishOfLocal_ = false;
                authResponseContext_->state = authRequestState_->GetStateType();
                authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            }
            break;
        case MSG_TYPE_RESP_PUBLICKEY:
            if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_CREDENTIAL) {
                authRequestState_->TransitionTo(std::make_shared<AuthRequestCredentialDone>());
            }
            break;
        case MSG_TYPE_RESP_RECHECK_MSG:
            if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_RECHECK_MSG) {
                authRequestState_->TransitionTo(std::make_shared<AuthRequestReCheckMsgDone>());
            }
            break;
        default:
            break;
    }
}

void DmAuthManager::ProcessSinkMsg()
{
    CHECK_NULL_VOID(authResponseState_);
    CHECK_NULL_VOID(authResponseContext_);
    authResponseState_->SetAuthContext(authResponseContext_);
    LOGI("OnDataReceived for sink device, authResponseContext msgType = %{public}d, authResponseState stateType ="
        "%{public}d", authResponseContext_->msgType, authResponseState_->GetStateType());

    switch (authResponseContext_->msgType) {
        case MSG_TYPE_NEGOTIATE:
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_INIT) {
                if (timer_ != nullptr) {
                    timer_->DeleteTimer(std::string(WAIT_NEGOTIATE_TIMEOUT_TASK));
                }
                authResponseState_->TransitionTo(std::make_shared<AuthResponseNegotiateState>());
            }
            break;
        case MSG_TYPE_REQ_AUTH:
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_NEGOTIATE) {
                if (timer_ != nullptr) {
                    timer_->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));
                }
                authResponseState_->TransitionTo(std::make_shared<AuthResponseConfirmState>());
            }
            break;
        case MSG_TYPE_REQ_AUTH_TERMINATE:
            ProcessReqAuthTerminate();
            break;
        case MSG_TYPE_REQ_PUBLICKEY:
            ProcessReqPublicKey();
            break;
        case MSG_TYPE_REQ_RECHECK_MSG:
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_AUTH_FINISH) {
                authResponseState_->TransitionTo(std::make_shared<AuthResponseReCheckMsg>());
                break;
            }
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_SHOW) {
                std::lock_guard<std::mutex> lock(srcReqMsgLock_);
                isNeedProcCachedSrcReqMsg_ = true;
            }
            break;
        default:
            break;
    }
}

void DmAuthManager::ProcessReqAuthTerminate()
{
    {
        std::lock_guard<std::mutex> lock(groupMutex_);
        if (authResponseState_->GetStateType() == AUTH_RESPONSE_SHOW &&
            authResponseContext_->reply == DM_OK && isCreateGroup_ && !isAddMember_) {
            LOGI("wait addmemer callback");
            transitToFinishState_ = true;
            return;
        }
    }
    if (authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_FINISH) {
        isFinishOfLocal_ = false;
        authResponseContext_->state = authResponseState_->GetStateType();
        if (authResponseContext_->reply == DM_OK) {
            authResponseContext_->state = AuthState::AUTH_RESPONSE_FINISH;
        }
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
    }
}

void DmAuthManager::OnDataReceived(const int32_t sessionId, const std::string message)
{
    if (authResponseContext_ == nullptr || authMessageProcessor_ == nullptr ||
        sessionId != authResponseContext_->sessionId) {
        LOGE("OnDataReceived failed, authResponseContext or authMessageProcessor_ is nullptr.");
        return;
    }

    authResponseContext_->sessionId = sessionId;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    int32_t ret = authMessageProcessor_->ParseMessage(message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        return;
    }

    if ((authRequestState_ != nullptr) && (authResponseState_ == nullptr)) {
        // source device auth process
        ProcessSourceMsg();
    } else if ((authResponseState_ != nullptr) && (authRequestState_ == nullptr)) {
        // sink device auth process
        {
            std::lock_guard<std::mutex> lock(srcReqMsgLock_);
            srcReqMsg_ = message;
        }
        ProcessSinkMsg();
    } else {
        LOGE("DmAuthManager::OnDataReceived failed, authRequestState_ or authResponseState_ is invalid.");
    }
}

void DmAuthManager::OnGroupCreated(int64_t requestId, const std::string &groupId)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to OnGroupCreated because authResponseContext_ is nullptr");
        return;
    }
    if (authResponseState_ == nullptr) {
        LOGE("DmAuthManager::AuthenticateDevice end");
        return;
    }
    LOGI("DmAuthManager::OnGroupCreated start group id %{public}s", GetAnonyString(groupId).c_str());
    CHECK_NULL_VOID(authMessageProcessor_);
    CHECK_NULL_VOID(softbusConnector_);
    if (groupId == "{}") {
        authResponseContext_->reply = ERR_DM_CREATE_GROUP_FAILED;
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        return;
    }
    std::string pinCode = "";
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE && !importAuthCode_.empty()) {
        GetAuthCode(authResponseContext_->hostPkgName, pinCode);
    } else if (authResponseContext_->authType != AUTH_TYPE_IMPORT_AUTH_CODE) {
        pinCode = GeneratePincode();
    } else {
        LOGE("authType invalied.");
    }
    JsonObject jsonObj;
    jsonObj[PIN_TOKEN] = authResponseContext_->token;
    jsonObj[QR_CODE_KEY] = GenerateGroupName();
    jsonObj[NFC_CODE_KEY] = GenerateGroupName();
    authResponseContext_->authToken = jsonObj.Dump();
    LOGI("DmAuthManager::OnGroupCreated start group id %{public}s", GetAnonyString(groupId).c_str());
    authResponseContext_->groupId = groupId;
    authResponseContext_->code = pinCode;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    authResponseContext_->isFinish = true;
    authResponseState_->TransitionTo(std::make_shared<AuthResponseShowState>());
}

void DmAuthManager::OnMemberJoin(int64_t requestId, int32_t status, int32_t operationCode)
{
    isAddingMember_ = false;
    if (authResponseContext_ == nullptr || authUiStateMgr_ == nullptr) {
        LOGE("failed to OnMemberJoin because authResponseContext_ or authUiStateMgr is nullptr");
        return;
    }
    LOGI("DmAuthManager OnMemberJoin start authTimes %{public}d", authTimes_);
    if (status == DM_OK && operationCode == GroupOperationCode::MEMBER_JOIN) {
        LOGI("join group success.");
        CompatiblePutAcl();
        {
            std::lock_guard<std::mutex> lock(groupMutex_);
            isAddMember_ = true;
            if (transitToFinishState_) {
                LOGI("Have received src finish state.");
                authResponseContext_->state = AuthState::AUTH_RESPONSE_FINISH;
                CHECK_NULL_VOID(authResponseState_);
                authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
            }
        }
    }
    if (status == DM_OK && operationCode == GroupOperationCode::GROUP_CREATE) {
        {
            std::lock_guard<std::mutex> lock(groupMutex_);
            isCreateGroup_ = true;
        }
    }
    if ((authRequestState_ != nullptr) && (authResponseState_ == nullptr)) {
        MemberJoinAuthRequest(requestId, status);
    } else if ((authResponseState_ != nullptr) && (authRequestState_ == nullptr)) {
        if (status == DM_OK && authResponseContext_->requestId == requestId &&
            authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_SHOW) {
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
        } else {
            if (++authTimes_ >= MAX_AUTH_TIMES) {
                authResponseContext_->isFinish = false;
                authResponseContext_->reply = ERR_DM_BIND_PIN_CODE_ERROR;
                authResponseContext_->state = AuthState::AUTH_RESPONSE_SHOW;
                isFinishOfLocal_ = false;
                authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
            }
        }
    } else {
        LOGE("DmAuthManager::OnMemberJoin failed, authRequestState_ or authResponseState_ is invalid.");
    }
}

void DmAuthManager::MemberJoinAuthRequest(int64_t requestId, int32_t status)
{
    authTimes_++;
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(ADD_TIMEOUT_TASK));
    }
    CHECK_NULL_VOID(authResponseContext_);
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        HandleMemberJoinImportAuthCode(requestId, status);
        return;
    }
    if (status != DM_OK || authResponseContext_->requestId != requestId) {
        if (authRequestState_ != nullptr && authTimes_ >= MAX_AUTH_TIMES) {
            authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
            authResponseContext_->reply = ERR_DM_BIND_PIN_CODE_ERROR;
            CHECK_NULL_VOID(authRequestContext_);
            authRequestContext_->reason = ERR_DM_BIND_PIN_CODE_ERROR;
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return;
        }
        if (timer_ != nullptr) {
            timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK),
                GetTaskTimeout(INPUT_TIMEOUT_TASK, INPUT_TIMEOUT), [this] (std::string name) {
                    DmAuthManager::HandleAuthenticateTimeout(name);
                });
        }
        CHECK_NULL_VOID(authUiStateMgr_);
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_PIN_CODE_ERROR);
    } else {
        CHECK_NULL_VOID(authRequestState_);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestNetworkState>());
        if (timer_ != nullptr) {
            timer_->DeleteTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK));
        }
    }
}

void DmAuthManager::HandleMemberJoinImportAuthCode(const int64_t requestId, const int32_t status)
{
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authRequestState_);
    if (status != DM_OK || authResponseContext_->requestId != requestId) {
        authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
        authResponseContext_->reply = ERR_DM_AUTH_CODE_INCORRECT;
        CHECK_NULL_VOID(authRequestContext_);
        authRequestContext_->reason = ERR_DM_AUTH_CODE_INCORRECT;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    } else {
        authRequestState_->TransitionTo(std::make_shared<AuthRequestNetworkState>());
    }
}

void DmAuthManager::HandleAuthenticateTimeout(std::string name)
{
    LOGI("DmAuthManager::HandleAuthenticateTimeout start timer name %{public}s", name.c_str());
    if (authRequestState_ != nullptr && authRequestState_->GetStateType() != AuthState::AUTH_REQUEST_FINISH) {
        if (authResponseContext_ == nullptr) {
            authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        }
        authResponseContext_->state = authRequestState_->GetStateType();
        CHECK_NULL_VOID(authRequestContext_);
        authRequestContext_->reason = ERR_DM_TIME_OUT;
        authResponseContext_->reply = ERR_DM_TIME_OUT;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    }

    if (authResponseState_ != nullptr && authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_FINISH) {
        CHECK_NULL_VOID(authResponseContext_);
        authResponseContext_->state = authResponseState_->GetStateType();
        authResponseContext_->reply = ERR_DM_TIME_OUT;
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
    }
    LOGI("DmAuthManager::HandleAuthenticateTimeout start complete");
}

int32_t DmAuthManager::EstablishAuthChannel(const std::string &deviceId)
{
    if (NeedInsensibleSwitching()) {
        return DM_OK;
    }
    int32_t sessionId = 0;
    if (IsHmlSessionType()) {
        CHECK_NULL_RETURN(authRequestContext_, ERR_DM_FAILED);
        CHECK_NULL_RETURN(listener_, ERR_DM_FAILED);
        LOGI("hmlActionId %{public}d, hmlReleaseTime %{public}d, hmlEnable160M %{public}d",
            authRequestContext_->hmlActionId, authRequestContext_->closeSessionDelaySeconds,
            authRequestContext_->hmlEnable160M);
        sessionId = listener_->OpenAuthSessionWithPara(deviceId,
            authRequestContext_->hmlActionId, authRequestContext_->hmlEnable160M);
    } else {
        sessionId = softbusConnector_->GetSoftbusSession()->OpenAuthSession(deviceId);
    }
    struct RadarInfo info = {
        .funcName = "EstablishAuthChannel",
        .stageRes = (sessionId > 0) ?
            static_cast<int32_t>(StageRes::STAGE_IDLE) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .bizState = (sessionId > 0) ?
            static_cast<int32_t>(BizState::BIZ_STATE_START) : static_cast<int32_t>(BizState::BIZ_STATE_END),
        .localSessName = DM_SESSION_NAME,
        .peerSessName = DM_SESSION_NAME,
        .isTrust = static_cast<int32_t>(TrustStatus::NOT_TRUST),
        .commServ = static_cast<int32_t>(CommServ::USE_SOFTBUS),
        .peerUdid = peerTargetId_.deviceId,
        .channelId = sessionId,
        .errCode = sessionId,
    };
    if (!DmRadarHelper::GetInstance().ReportAuthOpenSession(info)) {
        LOGE("ReportAuthOpenSession failed");
    }
    if (sessionId < 0) {
        LOGE("OpenAuthSession failed, stop the authentication");
        if (authResponseContext_ == nullptr) {
            authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        }
        authResponseContext_->state = AuthState::AUTH_REQUEST_NEGOTIATE;
        authResponseContext_->reply = sessionId;
        if (authRequestContext_ == nullptr) {
            authRequestContext_ = std::make_shared<DmAuthRequestContext>();
        }
        authRequestContext_->reason = sessionId;
        if (authRequestState_ != nullptr) {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        }
    }
    return DM_OK;
}

void DmAuthManager::StartNegotiate(const int32_t &sessionId)
{
    if (authResponseContext_ == nullptr) {
        LOGE("DmAuthManager::StartNegotiate error, authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::StartNegotiate sessionId %{public}d.", sessionId);
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(authMessageProcessor_);
    authResponseContext_->localDeviceId = authRequestContext_->localDeviceId;
    authResponseContext_->reply = ERR_DM_AUTH_REJECT;
    authResponseContext_->authType = authRequestContext_->authType;
    authResponseContext_->deviceId = authRequestContext_->deviceId;
    authResponseContext_->accountGroupIdHash = GetAccountGroupIdHash();
    authResponseContext_->hostPkgName = authRequestContext_->hostPkgName;
    authResponseContext_->bundleName = authRequestContext_->bundleName;
    authResponseContext_->peerBundleName = authRequestContext_->peerBundleName;
    authResponseContext_->hostPkgLabel = authRequestContext_->hostPkgLabel;
    authResponseContext_->tokenId = authRequestContext_->tokenId;
    authResponseContext_->bindLevel = authRequestContext_->bindLevel;
    authResponseContext_->bindType = authRequestContext_->bindType;
    authResponseContext_->isOnline = authRequestContext_->isOnline;
    authResponseContext_->authed = authRequestContext_->authed;
    authResponseContext_->dmVersion = "";
    authResponseContext_->localAccountId = authRequestContext_->localAccountId;
    authResponseContext_->localUserId = authRequestContext_->localUserId;
    authResponseContext_->isIdenticalAccount = false;
    authResponseContext_->edition = DM_VERSION_5_0_5;
    authResponseContext_->remoteDeviceName = authRequestContext_->localDeviceName;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_NEGOTIATE);
    if (!NeedInsensibleSwitching()) {
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
    }
    if (timer_ != nullptr) {
        timer_->StartTimer(std::string(NEGOTIATE_TIMEOUT_TASK),
            GetTaskTimeout(NEGOTIATE_TIMEOUT_TASK, NEGOTIATE_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    }
}

void DmAuthManager::AbilityNegotiate()
{
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(hiChainConnector_);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    authResponseContext_->remoteAccountId = authResponseContext_->localAccountId;
    authResponseContext_->remoteUserId = authResponseContext_->localUserId;
    GetBinderInfo();
    bool ret = hiChainConnector_->IsDevicesInP2PGroup(authResponseContext_->localDeviceId, localDeviceId);
    if (ret) {
        LOGE("DmAuthManager::EstablishAuthChannel device is in group");
        if (!DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(authResponseContext_->hostPkgName,
            authResponseContext_->localDeviceId)) {
            CompatiblePutAcl();
        }
        authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
    } else {
        authResponseContext_->reply = ERR_DM_AUTH_REJECT;
    }
    authResponseContext_->localDeviceId = localDeviceId;

    if (!IsAuthTypeSupported(authResponseContext_->authType)) {
        LOGE("DmAuthManager::AuthenticateDevice authType %{public}d not support.", authResponseContext_->authType);
        authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    } else {
        authPtr_ = authenticationMap_[authResponseContext_->authType];
    }

    if (IsAuthCodeReady(authResponseContext_->hostPkgName)) {
        authResponseContext_->isAuthCodeReady = true;
    } else {
        authResponseContext_->isAuthCodeReady = false;
    }
}

void DmAuthManager::RespNegotiate(const int32_t &sessionId)
{
    if (authResponseContext_ == nullptr || authRequestState_ != nullptr) {
        LOGE("failed to RespNegotiate because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::RespNegotiate sessionid %{public}d", sessionId);
    CHECK_NULL_VOID(softbusConnector_);
    remoteDeviceId_ = authResponseContext_->localDeviceId;
    authResponseContext_->networkId = softbusConnector_->GetLocalDeviceNetworkId();
    authResponseContext_->targetDeviceName = softbusConnector_->GetLocalDeviceName();
    remoteVersion_ = AuthManagerBase::ConvertSrcVersion(authResponseContext_->dmVersion, authResponseContext_->edition);
    NegotiateRespMsg(remoteVersion_);
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= USER &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        ProcRespNegotiateExt(sessionId);
        CHECK_NULL_VOID(timer_);
        timer_->StartTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK),
            GetTaskTimeout(WAIT_REQUEST_TIMEOUT_TASK, WAIT_REQUEST_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        static_cast<uint32_t>(authResponseContext_->bindLevel) == INVALIED_TYPE) {
        ProcRespNegotiate(sessionId);
        CHECK_NULL_VOID(timer_);
        timer_->StartTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK),
            GetTaskTimeout(WAIT_REQUEST_TIMEOUT_TASK, WAIT_REQUEST_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    } else {
        ProcIncompatible(sessionId);
    }
}

void DmAuthManager::NegotiateRespMsg(const std::string &version)
{
    CHECK_NULL_VOID(authResponseContext_);
    if (version == DM_VERSION_5_0_1) {
        authResponseContext_->dmVersion = DM_VERSION_5_0_1;
    } else if (version < DM_VERSION_5_0_1) {
        authResponseContext_->dmVersion = "";
        authResponseContext_->bindLevel = INVALIED_TYPE;
    } else if (version > DM_VERSION_5_0_1) {
        authResponseContext_->dmVersion = dmVersion_;
    }
}

void DmAuthManager::SendAuthRequest(const int32_t &sessionId)
{
    LOGI("DmAuthManager::SendAuthRequest sessionId %{public}d.", sessionId);
    if (authResponseContext_ == nullptr) {
        LOGE("failed to SendAuthRequest because authResponseContext_ is nullptr");
        return;
    }
    CHECK_NULL_VOID(authRequestContext_);
    if (authResponseContext_->reply == ERR_DM_VERSION_INCOMPATIBLE) {
        LOGE("The peer device version is not supported");
        authRequestContext_->reason = authResponseContext_->reply;
        CHECK_NULL_VOID(authRequestState_);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return;
    }
    remoteDeviceId_ = authResponseContext_->localDeviceId;
    authRequestContext_->remoteDeviceName = authResponseContext_->targetDeviceName;
    remoteVersion_ = ConvertSinkVersion(authResponseContext_->dmVersion);
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(NEGOTIATE_TIMEOUT_TASK));
    }
    if (authResponseContext_->cryptoSupport) {
        isCryptoSupport_ = true;
    }
    LOGI("SendAuthRequest dmversion %{public}s, level %{public}d",
        authResponseContext_->dmVersion.c_str(), authResponseContext_->bindLevel);
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= USER &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        ProcessAuthRequestExt(sessionId);
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        static_cast<uint32_t>(authResponseContext_->bindLevel) == INVALIED_TYPE) {
        ProcessAuthRequest(sessionId);
    } else {
        LOGE("Invalied bind mode.");
    }
}

void DmAuthManager::ProcessAuthRequest(const int32_t &sessionId)
{
    LOGI("ProcessAuthRequest start.");
    CHECK_NULL_VOID(authResponseContext_);
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        !authResponseContext_->importAuthCode.empty() && !importAuthCode_.empty()) {
        if (authResponseContext_->importAuthCode != Crypto::Sha256(importAuthCode_)) {
            SetReasonAndFinish(ERR_DM_AUTH_CODE_INCORRECT, AuthState::AUTH_REQUEST_FINISH);
            return;
        }
    }
    CHECK_NULL_VOID(softbusConnector_);
    if (authResponseContext_->isOnline && softbusConnector_->CheckIsOnline(remoteDeviceId_)) {
        authResponseContext_->isOnline = true;
    } else {
        authResponseContext_->isOnline = false;
    }
    if (CheckTrustState() != DM_OK) {
        LOGI("CheckTrustState end.");
        return;
    }
    CHECK_NULL_VOID(authMessageProcessor_);
    std::vector<std::string> messageList = authMessageProcessor_->CreateAuthRequestMessage();
    for (auto msg : messageList) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, msg);
    }
    CHECK_NULL_VOID(listener_);
    CHECK_NULL_VOID(authRequestContext_);
    listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId, authRequestContext_->token,
        STATUS_DM_SHOW_AUTHORIZE_UI, DM_OK);
    listener_->OnBindResult(processInfo_, peerTargetId_, DM_OK, STATUS_DM_SHOW_AUTHORIZE_UI, "");
    CHECK_NULL_VOID(timer_);
    timer_->StartTimer(std::string(CONFIRM_TIMEOUT_TASK),
        GetTaskTimeout(CONFIRM_TIMEOUT_TASK, CONFIRM_TIMEOUT), [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
}

void DmAuthManager::GetAuthRequestContext()
{
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(softbusConnector_);
    char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    Crypto::GetUdidHash(authResponseContext_->localDeviceId, reinterpret_cast<uint8_t *>(deviceIdHash));
    authRequestContext_->deviceId = static_cast<std::string>(deviceIdHash);
    authResponseContext_->deviceId = authResponseContext_->localDeviceId;
    authResponseContext_->localDeviceId = authRequestContext_->localDeviceId;
    authRequestContext_->remoteAccountId = authResponseContext_->localAccountId;
    authResponseContext_->remoteAccountId = authRequestContext_->remoteAccountId;
    authResponseContext_->localAccountId = authRequestContext_->localAccountId;
    authRequestContext_->remoteUserId = authResponseContext_->localUserId;
    if (authResponseContext_->isOnline && softbusConnector_->CheckIsOnline(remoteDeviceId_)) {
        authResponseContext_->isOnline = true;
    } else {
        authResponseContext_->isOnline = false;
    }
    authResponseContext_->haveCredential = false;
}

void DmAuthManager::ProcessAuthRequestExt(const int32_t &sessionId)
{
    LOGI("ProcessAuthRequestExt start.");
    CHECK_NULL_VOID(authResponseContext_);
    if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_5_0_3)) &&
        authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        !authResponseContext_->importAuthCode.empty() && !importAuthCode_.empty()) {
        if (authResponseContext_->importAuthCode != Crypto::Sha256(importAuthCode_)) {
            SetReasonAndFinish(ERR_DM_AUTH_CODE_INCORRECT, AuthState::AUTH_REQUEST_FINISH);
            return;
        }
    }

    GetAuthRequestContext();
    std::vector<int32_t> bindType =
        DeviceProfileConnector::GetInstance().SyncAclByBindType(authResponseContext_->hostPkgName,
        authResponseContext_->bindType, authResponseContext_->localDeviceId, authResponseContext_->deviceId);
    authResponseContext_->authed = !bindType.empty();
    if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_5_0_3)) &&
        authResponseContext_->isOnline && authResponseContext_->authed &&
        authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        (authResponseContext_->importAuthCode.empty() || importAuthCode_.empty())) {
        SetReasonAndFinish(ERR_DM_AUTH_CODE_INCORRECT, AuthState::AUTH_REQUEST_FINISH);
        return;
    }
    authResponseContext_->bindType = bindType;
    if (IsAuthFinish()) {
        return;
    }

    std::vector<std::string> messageList = authMessageProcessor_->CreateAuthRequestMessage();
    for (auto msg : messageList) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, msg);
    }
    listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId, authRequestContext_->token,
        STATUS_DM_SHOW_AUTHORIZE_UI, DM_OK);
    listener_->OnBindResult(processInfo_, peerTargetId_, DM_OK, STATUS_DM_SHOW_AUTHORIZE_UI, "");
    timer_->StartTimer(std::string(CONFIRM_TIMEOUT_TASK),
        GetTaskTimeout(CONFIRM_TIMEOUT_TASK, CONFIRM_TIMEOUT), [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
}

bool DmAuthManager::IsAuthFinish()
{
    CHECK_NULL_RETURN(authResponseContext_, false);
    if (authResponseContext_->reply == ERR_DM_UNSUPPORTED_AUTH_TYPE) {
        CHECK_NULL_RETURN(listener_, false);
        CHECK_NULL_RETURN(authRequestContext_, false);
        CHECK_NULL_RETURN(authRequestState_, false);
        listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId,
            authRequestContext_->token, AuthState::AUTH_REQUEST_NEGOTIATE_DONE, ERR_DM_UNSUPPORTED_AUTH_TYPE);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return true;
    }

    if ((authResponseContext_->isIdenticalAccount && !authResponseContext_->authed) ||
        (authResponseContext_->authed && !authResponseContext_->isOnline)) {
        CHECK_NULL_RETURN(authRequestContext_, false);
        JoinLnn(authRequestContext_->addr);
        authRequestContext_->reason = DM_OK;
        authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
        authResponseContext_->reply = DM_OK;
        CHECK_NULL_RETURN(authRequestState_, false);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return true;
    }

    if (authResponseContext_->reply == ERR_DM_UNSUPPORTED_AUTH_TYPE ||
        (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        authResponseContext_->isAuthCodeReady == false)) {
        CHECK_NULL_RETURN(authRequestState_, false);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return true;
    }
    return false;
}

int32_t DmAuthManager::ConfirmProcess(const int32_t &action)
{
    LOGI("ConfirmProcess start.");
    CHECK_NULL_RETURN(authResponseContext_, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(authResponseState_, ERR_DM_POINT_NULL);
    authResponseContext_->confirmOperation = action;
    if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH || action_ == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        authResponseContext_->reply = USER_OPERATION_TYPE_ALLOW_AUTH;
    } else {
        authResponseContext_->reply = action_;
    }

    if (authResponseContext_->reply == USER_OPERATION_TYPE_ALLOW_AUTH &&
        authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_CONFIRM) {
        authResponseState_->TransitionTo(std::make_shared<AuthResponseGroupState>());
    } else {
        CHECK_NULL_RETURN(authMessageProcessor_, ERR_DM_POINT_NULL);
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH);
        CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    }
    return DM_OK;
}

int32_t DmAuthManager::ConfirmProcessExt(const int32_t &action)
{
    LOGI("ConfirmProcessExt start.");
    CHECK_NULL_RETURN(authResponseContext_, ERR_DM_POINT_NULL);
    authResponseContext_->confirmOperation = action;
    if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH || action_ == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        authResponseContext_->reply = USER_OPERATION_TYPE_ALLOW_AUTH;
    } else {
        authResponseContext_->reply = USER_OPERATION_TYPE_CANCEL_AUTH;
    }
    CHECK_NULL_RETURN(authMessageProcessor_, ERR_DM_POINT_NULL);
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    CHECK_NULL_RETURN(authResponseState_, ERR_DM_POINT_NULL);
    if (authResponseContext_->reply == USER_OPERATION_TYPE_ALLOW_AUTH &&
        authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_CONFIRM) {
        if (CanUsePincodeFromDp()) {
            authResponseContext_->code = serviceInfoProfile_.GetPinCode();
            LOGI("import pincode from dp");
        } else if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE && !importAuthCode_.empty()) {
            GetAuthCode(authResponseContext_->hostPkgName, authResponseContext_->code);
        } else {
            authResponseContext_->code = GeneratePincode();
        }
        authResponseContext_->requestId = GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
        authResponseState_->TransitionTo(std::make_shared<AuthResponseShowState>());
    }
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH_EXT);
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    return DM_OK;
}

int32_t DmAuthManager::StartAuthProcess(const int32_t &action)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to StartAuthProcess because authResponseContext_ is nullptr");
        return ERR_DM_AUTH_NOT_START;
    }
    LOGI("DmAuthManager::StartAuthProcess");
    action_ = action;
    struct RadarInfo info = {
        .funcName = "StartAuthProcess",
        .stageRes = (action_ == USER_OPERATION_TYPE_CANCEL_AUTH) ?
            static_cast<int32_t>(StageRes::STAGE_CANCEL) : static_cast<int32_t>(StageRes::STAGE_SUCC),
        .bizState = (action_ == USER_OPERATION_TYPE_CANCEL_AUTH) ?
            static_cast<int32_t>(BizState::BIZ_STATE_END) : static_cast<int32_t>(BizState::BIZ_STATE_START),
        .errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_AUTH_REJECT),
    };
    if (!DmRadarHelper::GetInstance().ReportAuthConfirmBox(info)) {
        LOGE("ReportAuthConfirmBox failed");
    }
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= USER &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        return ConfirmProcessExt(action);
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        static_cast<uint32_t>(authResponseContext_->bindLevel) == INVALIED_TYPE) {
        return ConfirmProcess(action);
    } else {
        LOGE("Invalied bind mode.");
    }
    return DM_OK;
}

//LCOV_EXCL_START
void DmAuthManager::StartRespAuthProcess()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to StartRespAuthProcess because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::StartRespAuthProcess sessionId = %{public}d", authResponseContext_->sessionId);
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(CONFIRM_TIMEOUT_TASK));
    }
    if (authResponseContext_->groupName[CHECK_AUTH_ALWAYS_POS] == AUTH_ALWAYS) {
        action_ = USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    } else if (authResponseContext_->groupName[CHECK_AUTH_ALWAYS_POS] == AUTH_ONCE) {
        action_ = USER_OPERATION_TYPE_ALLOW_AUTH;
    }
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(authRequestState_);
    if (authResponseContext_->reply == USER_OPERATION_TYPE_ALLOW_AUTH) {
        if (timer_ != nullptr) {
            timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK),
                GetTaskTimeout(INPUT_TIMEOUT_TASK, INPUT_TIMEOUT), [this] (std::string name) {
                    DmAuthManager::HandleAuthenticateTimeout(name);
                });
            timer_->StartTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK),
                GetTaskTimeout(SESSION_HEARTBEAT_TIMEOUT_TASK, SESSION_HEARTBEAT_TIMEOUT), [this] (std::string name) {
                    DmAuthManager::HandleSessionHeartbeat(name);
                });
        }

        CHECK_NULL_VOID(listener_);
        listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId,
            authRequestContext_->token, STATUS_DM_SHOW_PIN_INPUT_UI, DM_OK);
        listener_->OnBindResult(processInfo_, peerTargetId_, DM_OK, STATUS_DM_SHOW_PIN_INPUT_UI, "");
        authRequestState_->TransitionTo(std::make_shared<AuthRequestJoinState>());
    } else {
        LOGE("do not accept");
        authResponseContext_->state = AuthState::AUTH_REQUEST_REPLY;
        authRequestContext_->reason = ERR_DM_AUTH_PEER_REJECT;
        authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    }
}

int32_t DmAuthManager::CreateGroup()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to CreateGroup because authResponseContext_ is nullptr");
        return ERR_DM_FAILED;
    }
    LOGI("DmAuthManager::CreateGroup start");
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_FAILED);
    authResponseContext_->groupName = GenerateGroupName();
    authResponseContext_->requestId = GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
    hiChainConnector_->CreateGroup(authResponseContext_->requestId, authResponseContext_->groupName);
    return DM_OK;
}
//LCOV_EXCL_STOP

int32_t DmAuthManager::AddMember(const std::string &pinCode)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to AddMember because authResponseContext_ is nullptr");
        return ERR_DM_FAILED;
    }
    LOGI("DmAuthManager::AddMember start group id %{public}s", GetAnonyString(authResponseContext_->groupId).c_str());
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(INPUT_TIMEOUT_TASK));
    }
    JsonObject jsonObject;
    jsonObject[TAG_GROUP_ID] = authResponseContext_->groupId;
    jsonObject[TAG_GROUP_NAME] = authResponseContext_->groupName;
    jsonObject[PIN_CODE_KEY] = pinCode;
    jsonObject[TAG_REQUEST_ID] = authResponseContext_->requestId;
    jsonObject[TAG_DEVICE_ID] = authResponseContext_->deviceId;
    std::string connectInfo = jsonObject.Dump();
    if (timer_ != nullptr) {
        timer_->StartTimer(std::string(ADD_TIMEOUT_TASK),
            GetTaskTimeout(ADD_TIMEOUT_TASK, ADD_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    }
    if (authUiStateMgr_ == nullptr || hiChainConnector_ == nullptr || authRequestContext_ == nullptr) {
        LOGE("DmAuthManager::AddMember authUiStateMgr_ or hiChainConnector_ or authRequestContext_ is null.");
        return ERR_DM_FAILED;
    }
    if (isAddingMember_) {
        LOGE("DmAuthManager::AddMember doing add member.");
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_DOING_AUTH);
        return ERR_DM_FAILED;
    }
    isAddingMember_ = true;
    int32_t ret = hiChainConnector_->AddMember(authRequestContext_->addr, connectInfo);
    struct RadarInfo info = {
        .funcName = "AddMember",
        .stageRes = (ret == 0) ?
            static_cast<int32_t>(StageRes::STAGE_IDLE) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .peerUdid = authResponseContext_ == nullptr ? "" : authResponseContext_->deviceId,
        .errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_ADD_GROUP_FAILED),
    };
    if (!DmRadarHelper::GetInstance().ReportAuthAddGroup(info)) {
        LOGE("ReportAuthAddGroup failed");
    }
    if (ret != 0) {
        LOGE("DmAuthManager::AddMember failed, ret: %{public}d", ret);
        isAddingMember_ = false;
        return ERR_DM_ADD_GROUP_FAILED;
    }
    return DM_OK;
}

std::string DmAuthManager::GetConnectAddr(std::string deviceId)
{
    CHECK_NULL_RETURN(softbusConnector_, "");
    std::string connectAddr;
    if (softbusConnector_->GetConnectAddr(deviceId, connectAddr) == nullptr) {
        LOGE("DmAuthManager::GetConnectAddr error");
    }
    return connectAddr;
}

int32_t DmAuthManager::JoinNetwork()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to JoinNeWork because authResponseContext_ is nullptr");
        return ERR_DM_FAILED;
    }
    LOGI("DmAuthManager JoinNetwork start");
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
    }
    CHECK_NULL_RETURN(authRequestContext_, ERR_DM_FAILED);
    CHECK_NULL_RETURN(authRequestState_, ERR_DM_FAILED);
    authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
    authResponseContext_->isFinish = true;
    authRequestContext_->reason = DM_OK;
    authResponseContext_->reply = DM_OK;
    authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    return DM_OK;
}

void DmAuthManager::SinkAuthenticateFinish()
{
    LOGI("DmAuthManager::SinkAuthenticateFinish, isFinishOfLocal: %{public}d", isFinishOfLocal_);
    processInfo_.pkgName = authResponseContext_->peerBundleName;
    listener_->OnSinkBindResult(processInfo_, peerTargetId_, authResponseContext_->reply,
        authResponseContext_->state, GenerateBindResultContent());
    if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_FINISH &&
        (authResponseContext_->authType == AUTH_TYPE_NFC || authPtr_ != nullptr)) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_CONFIRM_SHOW);
    }
    if (isFinishOfLocal_) {
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_AUTH_TERMINATE);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    }
    authResponseState_ = nullptr;
    authTimes_ = 0;
}

int32_t DmAuthManager::GetOutputState(int32_t state)
{
    LOGI("state %{public}d.", state);
    auto it = OLD_STATE_MAPPING.find(static_cast<AuthState>(state));
    if (it != OLD_STATE_MAPPING.end()) {
        return static_cast<int32_t>(it->second);
    }
    return static_cast<int32_t>(STATUS_DM_AUTH_DEFAULT);
}

void DmAuthManager::SrcAuthenticateFinish()
{
    LOGI("DmAuthManager::SrcAuthenticateFinish, isFinishOfLocal: %{public}d", isFinishOfLocal_);
    CHECK_NULL_VOID(authResponseContext_);
    if (isFinishOfLocal_) {
        CHECK_NULL_VOID(authMessageProcessor_);
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_AUTH_TERMINATE);
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    } else {
        CHECK_NULL_VOID(authRequestContext_);
        authRequestContext_->reason = authResponseContext_->reply;
    }
    if ((authResponseContext_->state == AuthState::AUTH_REQUEST_JOIN ||
        authResponseContext_->state == AuthState::AUTH_REQUEST_FINISH) &&
        (authResponseContext_->authType == AUTH_TYPE_NFC || authPtr_ != nullptr)) {
        CHECK_NULL_VOID(authUiStateMgr_);
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
    }
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    CHECK_NULL_VOID(authRequestContext_);
    int32_t closeSessionDelaySeconds = authRequestContext_->closeSessionDelaySeconds;
    if (IsHmlSessionType() && closeSessionDelaySeconds == 0 && isWaitingJoinLnnCallback_) {
        closeSessionDelaySeconds = SESSION_CLOSE_TIMEOUT;
    }
    timer_->StartTimer(std::string(WAIT_SESSION_CLOSE_TIMEOUT_TASK) + std::string(CLOSE_SESSION_TASK_SEPARATOR) +
        std::to_string(authRequestContext_->sessionId),
        closeSessionDelaySeconds, [this] (std::string name) {
            int32_t sessionIdIndex = 1;
            std::string sessionStr = GetSubStr(name, std::string(CLOSE_SESSION_TASK_SEPARATOR), sessionIdIndex);
            if (!sessionStr.empty()) {
                int32_t sessionId = std::atoi(sessionStr.c_str());
                DmAuthManager::CloseAuthSession(sessionId);
            }
        });
    int32_t status = GetOutputState(authResponseContext_->state);
    CHECK_NULL_VOID(listener_);
    listener_->OnAuthResult(processInfo_, peerTargetId_.deviceId, authRequestContext_->token, status,
        authRequestContext_->reason);
    listener_->OnBindResult(processInfo_, peerTargetId_, authRequestContext_->reason, status,
        GenerateBindResultContent());

    authRequestContext_ = nullptr;
    authRequestState_ = nullptr;
    authTimes_ = 0;
    isWaitingJoinLnnCallback_ = false;
}

void DmAuthManager::AuthenticateFinish()
{
    {
        std::lock_guard<std::mutex> lock(srcReqMsgLock_);
        srcReqMsg_ = "";
        isNeedProcCachedSrcReqMsg_ = false;
        std::lock_guard<std::mutex> guard(sessionKeyIdMutex_);
        sessionKeyIdAsyncResult_.clear();
    }
    pincodeDialogEverShown_ = false;
    serviceInfoProfile_ = {};
    authType_ = AUTH_TYPE_UNKNOW;
    std::lock_guard<std::mutex> autoLock(g_authFinishLock);
    if (authResponseContext_ == nullptr || authUiStateMgr_ == nullptr) {
        LOGE("failed to AuthenticateFinish because authResponseContext_ or authUiStateMgr is nullptr");
        return;
    }
    LOGI("DmAuthManager::AuthenticateFinish start");
    isAddingMember_ = false;
    isAuthenticateDevice_ = false;
    isAuthDevice_ = false;
    isNeedJoinLnn_ = true;
    CHECK_NULL_VOID(softbusConnector_);
    if (DeviceProfileConnector::GetInstance().GetTrustNumber(remoteDeviceId_) >= 1 &&
        CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        softbusConnector_->CheckIsOnline(remoteDeviceId_) && authResponseContext_->isFinish) {
        softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
    }

    DeleteAuthCode();
    if (timer_ != nullptr) {
        timer_->DeleteAll();
    }
    if (authResponseState_ != nullptr) {
        SinkAuthenticateFinish();
    } else if (authRequestState_ != nullptr) {
        SrcAuthenticateFinish();
    }
    ResetParams();
    LOGI("DmAuthManager::AuthenticateFinish complete");
}

void DmAuthManager::ResetParams()
{
    isFinishOfLocal_ = true;
    authResponseContext_ = nullptr;
    authMessageProcessor_ = nullptr;
    authPtr_ = nullptr;
    authRequestStateTemp_ = nullptr;
    authenticationType_ = USER_OPERATION_TYPE_ALLOW_AUTH;
    bundleName_ = "";
    isAddMember_ = false;
    isCreateGroup_ = false;
    transitToFinishState_ = false;
    if (cleanNotifyCallback_ != nullptr) {
        cleanNotifyCallback_(0);
    }
}

int32_t DmAuthManager::RegisterUiStateCallback(const std::string pkgName)
{
    LOGI("DmAuthManager::RegisterUiStateCallback start");
    if (authUiStateMgr_ == nullptr) {
        LOGE("DmAuthManager::RegisterUiStateCallback authUiStateMgr_ is null.");
        return ERR_DM_FAILED;
    }
    authUiStateMgr_->RegisterUiStateCallback(pkgName);
    return DM_OK;
}

int32_t DmAuthManager::UnRegisterUiStateCallback(const std::string pkgName)
{
    LOGI("DmAuthManager::UnRegisterUiStateCallback start");
    if (authUiStateMgr_ == nullptr) {
        LOGE("DmAuthManager::UnRegisterUiStateCallback authUiStateMgr_ is null.");
        return ERR_DM_FAILED;
    }
    authUiStateMgr_->UnRegisterUiStateCallback(pkgName);
    return DM_OK;
}

std::string DmAuthManager::GeneratePincode()
{
    int32_t pinCode = GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
    return std::to_string(pinCode);
}

bool DmAuthManager::IsPinCodeValid(const std::string strpin)
{
    if (strpin.empty()) {
        return false;
    }
    for (size_t i = 0; i < strpin.length(); i++) {
        if (!isdigit(strpin[i])) {
            return false;
        }
    }
    int32_t pinnum = std::atoi(strpin.c_str());
    return IsPinCodeValid(pinnum);
}

bool DmAuthManager::IsPinCodeValid(int32_t numpin)
{
    if (numpin < MIN_PIN_CODE || numpin > MAX_PIN_CODE) {
        return false;
    }
    return true;
}

bool DmAuthManager::CanUsePincodeFromDp()
{
    CHECK_NULL_RETURN(authResponseContext_, false);
    return (IsPinCodeValid(serviceInfoProfile_.GetPinCode()) &&
        serviceInfoProfile_.GetPinExchangeType() == (int32_t)DMLocalServiceInfoPinExchangeType::FROMDP);
}

bool DmAuthManager::IsServiceInfoAuthTypeValid(int32_t authType)
{
    if (authType != (int32_t)DMLocalServiceInfoAuthType::TRUST_ONETIME &&
        authType != (int32_t)DMLocalServiceInfoAuthType::TRUST_ALWAYS &&
        authType != (int32_t)DMLocalServiceInfoAuthType::CANCEL) {
        return false;
    }
    return true;
}

bool DmAuthManager::IsServiceInfoAuthBoxTypeValid(int32_t authBoxType)
{
    if (authBoxType != (int32_t)DMLocalServiceInfoAuthBoxType::STATE3 &&
        authBoxType != (int32_t)DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM) {
        return false;
    }
    return true;
}

bool DmAuthManager::IsServiceInfoPinExchangeTypeValid(int32_t pinExchangeType)
{
    if (pinExchangeType != (int32_t)DMLocalServiceInfoPinExchangeType::PINBOX &&
        pinExchangeType != (int32_t)DMLocalServiceInfoPinExchangeType::FROMDP &&
        pinExchangeType != (int32_t)DMLocalServiceInfoPinExchangeType::ULTRASOUND &&
        pinExchangeType != (int32_t)DMLocalServiceInfoPinExchangeType::IMPORT_AUTH_CODE) {
        return false;
    }
    return true;
}

bool DmAuthManager::IsLocalServiceInfoValid(const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo)
{
    if (!IsServiceInfoAuthTypeValid(localServiceInfo.GetAuthType())) {
        LOGE("AuthType not valid, %{public}d", localServiceInfo.GetAuthType());
        return false;
    }
    if (!IsServiceInfoAuthBoxTypeValid(localServiceInfo.GetAuthBoxType())) {
        LOGE("AuthBoxType not valid, %{public}d", localServiceInfo.GetAuthBoxType());
        return false;
    }
    if (!IsServiceInfoPinExchangeTypeValid(localServiceInfo.GetPinExchangeType())) {
        LOGE("PinExchangeType not valid, %{public}d", localServiceInfo.GetPinExchangeType());
        return false;
    }
    return true;
}

std::string DmAuthManager::GenerateGroupName()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to GenerateGroupName because authResponseContext_ is nullptr.");
        return "";
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string sLocalDeviceId = localDeviceId;
    uint32_t interceptLength = sLocalDeviceId.size() / DEVICE_ID_HALF;
    std::string groupName = "";
    if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        groupName += AUTH_ALWAYS;
    } else {
        groupName += AUTH_ONCE;
    }
    groupName += authResponseContext_->targetPkgName + sLocalDeviceId.substr(0, interceptLength)
        + authResponseContext_->localDeviceId.substr(0, interceptLength);
    return groupName;
}

bool DmAuthManager::GetIsCryptoSupport()
{
    if (authResponseState_ == nullptr) {
        return false;
    }
    if (authRequestState_ == nullptr) {
        if (authResponseState_->GetStateType() == AuthState::AUTH_REQUEST_NEGOTIATE_DONE) {
            return false;
        }
    } else {
        if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_NEGOTIATE ||
            authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_NEGOTIATE_DONE) {
            return false;
        }
    }

    return isCryptoSupport_;
}

//LCOV_EXCL_START
int32_t DmAuthManager::SetAuthRequestState(std::shared_ptr<AuthRequestState> authRequestState)
{
    if (authRequestState == nullptr) {
        LOGE("authRequestState is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    // Keep current state, avoid deconstructed during state transitions
    authRequestStateTemp_ = authRequestState_;
    authRequestState_ = authRequestState;
    return DM_OK;
}

int32_t DmAuthManager::SetAuthResponseState(std::shared_ptr<AuthResponseState> authResponseState)
{
    if (authResponseState == nullptr) {
        LOGE("authResponseState is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    authResponseState_ = authResponseState;
    return DM_OK;
}
//LCOV_EXCL_STOP

int32_t DmAuthManager::GetPinCode(std::string &code)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to GetPinCode because authResponseContext_ is nullptr");
        return ERR_DM_FAILED;
    }
    LOGI("ShowConfigDialog start add member pin code, pinCode: %{public}s",
        GetAnonyString(authResponseContext_->code).c_str());
    code = authResponseContext_->code;
    return DM_OK;
}

//LCOV_EXCL_START
void DmAuthManager::CheckAndEndTvDream()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t result = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (result != SOFTBUS_OK) {
        LOGE("GetLocalNodeDeviceInfo from dsofbus fail, result=%{public}d", result);
        return;
    }
 
    if (nodeBasicInfo.deviceTypeId == TYPE_TV_ID) {
        int32_t ret = AuthManagerBase::EndDream();
        if (ret != DM_OK) {
            LOGE("fail to end dream, err:%{public}d", ret);
            return;
        }
    }
}

void DmAuthManager::ShowConfigDialog()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowConfigDialog because authResponseContext_ is nullptr");
        return;
    }
    if (authResponseContext_->authType == AUTH_TYPE_NFC &&
        serviceInfoProfile_.GetAuthBoxType() == (int32_t)DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM) {
        LOGI("no need confirm dialog");
        StartAuthProcess(serviceInfoProfile_.GetAuthType());
        return;
    }
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE && !importAuthCode_.empty()) {
        LOGI("start auth process");
        StartAuthProcess(authenticationType_);
        return;
    }
    CheckAndEndTvDream();
    LOGI("ShowConfigDialog start");
    JsonObject jsonObj;
    jsonObj[TAG_AUTH_TYPE] = AUTH_TYPE_PIN;
    jsonObj[TAG_TOKEN] = authResponseContext_->token;
    jsonObj[TARGET_PKG_NAME_KEY] = authResponseContext_->targetPkgName;
    jsonObj[TAG_CUSTOM_DESCRIPTION] = authResponseContext_->customDesc;
    jsonObj[TAG_APP_OPERATION] = authResponseContext_->appOperation;
    jsonObj[TAG_LOCAL_DEVICE_TYPE] = authResponseContext_->deviceTypeId;
    jsonObj[TAG_REQUESTER] = authResponseContext_->deviceName;
    jsonObj[TAG_HOST_PKGLABEL] = authResponseContext_->hostPkgLabel;
    const std::string params = jsonObj.Dump();
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    DeviceProfileConnector::GetInstance().SyncAclByBindType(authResponseContext_->peerBundleName,
        authResponseContext_->bindType, localUdid, remoteDeviceId_);
    DmDialogManager::GetInstance().ShowConfirmDialog(params);
    struct RadarInfo info = {
        .funcName = "ShowConfigDialog",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE),
    };
    if (!DmRadarHelper::GetInstance().ReportAuthPullAuthBox(info)) {
        LOGE("ReportAuthPullAuthBox failed");
    }
    LOGI("ShowConfigDialog end");
}
//LCOV_EXCL_STOP

void DmAuthManager::ShowAuthInfoDialog(bool authDeviceError)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowAuthInfoDialog because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::ShowAuthInfoDialog start");
    if (!authDeviceError && CanUsePincodeFromDp()) {
        LOGI("pin import from dp, not show dialog");
        return;
    }
    if (pincodeDialogEverShown_) {
        return;
    }
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE && !importAuthCode_.empty()) {
        LOGI("not show dialog.");
        return;
    }
    struct RadarInfo info = {
        .funcName = "ShowAuthInfoDialog",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
    };
    if (!DmRadarHelper::GetInstance().ReportAuthPullPinBox(info)) {
        LOGE("ReportAuthPullPinBox failed");
    }
    pincodeDialogEverShown_ = true;
    DmDialogManager::GetInstance().ShowPinDialog(authResponseContext_->code);
}

//LCOV_EXCL_START
void DmAuthManager::ShowStartAuthDialog()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowStartAuthDialog because authResponseContext_ is nullptr");
        return;
    }
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        LOGI("Add member start");
        std::string pinCode = "";
        if (GetAuthCode(authResponseContext_->hostPkgName, pinCode) != DM_OK) {
            LOGE("failed to get auth code");
            return;
        }
        if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
            (static_cast<uint32_t>(authResponseContext_->bindLevel) >= USER &&
            static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
            AuthDevice(pinCode);
        } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
            static_cast<uint32_t>(authResponseContext_->bindLevel) == INVALIED_TYPE) {
            AddMember(pinCode);
        } else {
            LOGE("Invalied bind mode.");
        }
        return;
    }
    if (IsScreenLocked()) {
        LOGE("ShowStartAuthDialog screen is locked.");
        SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL, STATUS_DM_AUTH_DEFAULT);
        return;
    }
    LOGI("DmAuthManager::ShowStartAuthDialog start");
    std::string pincode = "";
    if (authResponseContext_->authType == AUTH_TYPE_NFC &&
        GetAuthCode(authResponseContext_->hostPkgName, pincode) == DM_OK) {
        LOGI("already has pin code");
        ProcessPincode(pincode);
        return;
    }

    pincodeDialogEverShown_ = true;
    DmDialogManager::GetInstance().ShowInputDialog(authResponseContext_->targetDeviceName);
}
//LCOV_EXCL_STOP

int32_t DmAuthManager::ProcessPincode(const std::string &pinCode)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ProcessPincode because authResponseContext_ is nullptr");
        return ERR_DM_FAILED;
    }
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(INPUT_TIMEOUT_TASK));
    }
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= USER &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        return AuthDevice(pinCode);
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        static_cast<uint32_t>(authResponseContext_->bindLevel) == INVALIED_TYPE) {
        return AddMember(pinCode);
    } else {
        LOGE("Invalied bind mode.");
    }
    return ERR_DM_FAILED;
}

int32_t DmAuthManager::AuthDevice(const std::string &pinCode)
{
    LOGI("DmAuthManager::AuthDevice start.");
    if (isAuthDevice_) {
        LOGE("DmAuthManager::AuthDevice doing auth device.");
        CHECK_NULL_RETURN(authUiStateMgr_, ERR_DM_FAILED);
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_DOING_AUTH);
        return ERR_DM_FAILED;
    }
    isAuthDevice_ = true;
    int32_t osAccountId = MultipleUserConnector::GetCurrentAccountUserID();
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(INPUT_TIMEOUT_TASK));
        timer_->StartTimer(std::string(AUTH_DEVICE_TIMEOUT_TASK), AUTH_DEVICE_TIMEOUT,
            [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    }
    CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_FAILED);
    CHECK_NULL_RETURN(authResponseContext_, ERR_DM_FAILED);
    if (hiChainAuthConnector_->AuthDevice(pinCode, osAccountId, remoteDeviceId_,
        authResponseContext_->requestId) != DM_OK) {
        LOGE("DmAuthManager::AuthDevice failed.");
        isAuthDevice_ = false;
        if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
            HandleMemberJoinImportAuthCode(authResponseContext_->requestId, ERR_DM_FAILED);
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}

int32_t DmAuthManager::OnUserOperation(int32_t action, const std::string &params)
{
    if (authResponseContext_ == nullptr) {
        LOGE("Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }
    struct RadarInfo info = {
        .funcName = "OnUserOperation",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_CANCEL),
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
    };
    JsonObject paramJson;
    paramJson.Parse(params);
    std::string pinCode;
    switch (action) {
        case USER_OPERATION_TYPE_ALLOW_AUTH:
        case USER_OPERATION_TYPE_CANCEL_AUTH:
        case USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS:
            StartAuthProcess(action);
            break;
        case USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT:
            SetReasonAndFinish(ERR_DM_TIME_OUT, STATUS_DM_AUTH_DEFAULT);
            info.errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_TIME_OUT);
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY:
            SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY, STATUS_DM_AUTH_DEFAULT);
            info.errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY);
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT:
            SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL_ERROR, STATUS_DM_AUTH_DEFAULT);
            info.errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_BIND_USER_CANCEL_ERROR);
            break;
        case USER_OPERATION_TYPE_DONE_PINCODE_INPUT:
            if (paramJson.IsDiscarded() || !IsString(paramJson, PIN_CODE_KEY)) {
                LOGE("DmAuthManager OnUserOperation pinCode not found");
                return ERR_DM_INPUT_PARA_INVALID;
            }
            pinCode = paramJson[PIN_CODE_KEY].Get<std::string>();
            ProcessPincode(pinCode);
            info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
            break;
        default:
            LOGE("this action id not support");
            break;
    }
    if (!DmRadarHelper::GetInstance().ReportAuthConfirmBox(info)) {
        LOGE("ReportAuthConfirmBox failed");
    }
    return DM_OK;
}

int32_t DmAuthManager::SetPageId(int32_t pageId)
{
    if (authResponseContext_ == nullptr) {
        LOGE("Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }
    authResponseContext_->pageId = pageId;
    return DM_OK;
}

int32_t DmAuthManager::SetReasonAndFinish(int32_t reason, int32_t state)
{
    if (authResponseContext_ == nullptr) {
        LOGE("Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }
    authResponseContext_->state = state;
    authResponseContext_->reply = reason;
    if (authRequestState_ != nullptr && authRequestState_->GetStateType() != AuthState::AUTH_REQUEST_FINISH) {
        CHECK_NULL_RETURN(authRequestContext_, ERR_DM_AUTH_NOT_START);
        authRequestContext_->reason = reason;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    } else if (authResponseState_ != nullptr && authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_FINISH) {
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
    }
    return DM_OK;
}

//LCOV_EXCL_START
bool DmAuthManager::IsIdenticalAccount()
{
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
    std::string queryParams = jsonObj.Dump();

    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return false;
    }
    CHECK_NULL_RETURN(hiChainConnector_, false);
    std::vector<GroupInfo> groupList;
    if (!hiChainConnector_->GetGroupInfo(osAccountUserId, queryParams, groupList)) {
        return false;
    }
    if (authResponseContext_ == nullptr) {
        LOGE("authResponseContext_ is nullptr.");
        return false;
    }
    if (authResponseContext_->accountGroupIdHash == OLD_VERSION_ACCOUNT) {
        LOGI("The old version.");
        return true;
    }
    JsonObject jsonPeerGroupIdObj(authResponseContext_->accountGroupIdHash);
    if (jsonPeerGroupIdObj.IsDiscarded()) {
        LOGE("accountGroupIdHash string not a json type.");
        return false;
    }

    std::vector<JsonItemObject> items = jsonPeerGroupIdObj.Items();
    for (auto &groupInfo : groupList) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->IsString() && it->Get<std::string>() == Crypto::GetGroupIdHash(groupInfo.groupId)) {
                LOGI("Is identical Account.");
                return true;
            }
        }
    }
    return false;
}

std::string DmAuthManager::GetAccountGroupIdHash()
{
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
    std::string queryParams = jsonObj.Dump();

    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return "";
    }
    CHECK_NULL_RETURN(hiChainConnector_, "");
    std::vector<GroupInfo> groupList;
    if (!hiChainConnector_->GetGroupInfo(osAccountUserId, queryParams, groupList)) {
        return "";
    }
    JsonObject jsonAccountObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (auto &groupInfo : groupList) {
        jsonAccountObj.PushBack(Crypto::GetGroupIdHash(groupInfo.groupId));
    }
    return jsonAccountObj.Dump();
}
//LCOV_EXCL_STOP

int32_t DmAuthManager::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (authCode.empty() || pkgName.empty()) {
        LOGE("ImportAuthCode failed, authCode or pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    importAuthCode_ = authCode;
    importPkgName_ = pkgName;
    return DM_OK;
}

int32_t DmAuthManager::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam, int sessionId, uint64_t logicalSessionId)
{
    struct RadarInfo info = {
        .funcName = "AuthenticateDevice",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverUserRes(info)) {
        LOGE("ReportDiscoverUserRes failed");
    }
    std::string isNeedJoinLnnStr;
    if (bindParam.find(IS_NEED_JOIN_LNN) != bindParam.end()) {
        isNeedJoinLnnStr = bindParam.at(IS_NEED_JOIN_LNN);
    }
    if (isNeedJoinLnnStr == NEED_JOIN_LNN || isNeedJoinLnnStr == NO_NEED_JOIN_LNN) {
        isNeedJoinLnn_ = std::atoi(isNeedJoinLnnStr.c_str());
        LOGI("isNeedJoinLnn: %{public}d.", isNeedJoinLnn_);
    }
    if (pkgName.empty()) {
        LOGE("DmAuthManager::BindTarget failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t authType = -1;
    if (ParseAuthType(bindParam, authType) != DM_OK) {
        LOGE("DmAuthManager::BindTarget failed, key: %{public}s error.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    peerTargetId_ = targetId;
    std::string deviceId = "";
    std::string addrType;
    if (bindParam.count(PARAM_KEY_CONN_ADDR_TYPE) != 0) {
        addrType = bindParam.at(PARAM_KEY_CONN_ADDR_TYPE);
    }
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        bindParam_ = bindParam;
    }
    if (ParseConnectAddr(targetId, deviceId, addrType) == DM_OK) {
        return AuthenticateDevice(pkgName, authType, deviceId, ParseExtraFromMap(bindParam));
    } else if (!targetId.deviceId.empty()) {
        return AuthenticateDevice(pkgName, authType, targetId.deviceId, ParseExtraFromMap(bindParam));
    } else {
        LOGE("DmAuthManager::BindTarget failed, targetId is error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
}

int32_t DmAuthManager::ParseConnectAddr(const PeerTargetId &targetId, std::string &deviceId, std::string &addrType)
{
    int32_t index = 0;
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    ConnectionAddr addr;
    if (!targetId.wifiIp.empty() && targetId.wifiIp.length() <= IP_STR_MAX_LEN) {
        LOGI("DmAuthManager::ParseConnectAddr parse wifiIp: %{public}s.", GetAnonyString(targetId.wifiIp).c_str());
        if (!addrType.empty()) {
            addr.type = static_cast<ConnectionAddrType>(std::atoi(addrType.c_str()));
        } else {
            addr.type = ConnectionAddrType::CONNECTION_ADDR_WLAN;
        }
        memcpy_s(addr.info.ip.ip, IP_STR_MAX_LEN, targetId.wifiIp.c_str(), targetId.wifiIp.length());
        addr.info.ip.port = targetId.wifiPort;
        deviceInfo->addr[index] = addr;
        deviceId = targetId.wifiIp;
        index++;
    } else if (!targetId.brMac.empty() && targetId.brMac.length() <= BT_MAC_LEN) {
        LOGI("DmAuthManager::ParseConnectAddr parse brMac: %{public}s.", GetAnonyString(targetId.brMac).c_str());
        addr.type = ConnectionAddrType::CONNECTION_ADDR_BR;
        memcpy_s(addr.info.br.brMac, BT_MAC_LEN, targetId.brMac.c_str(), targetId.brMac.length());
        deviceInfo->addr[index] = addr;
        deviceId = targetId.brMac;
        index++;
    } else if (!targetId.bleMac.empty() && targetId.bleMac.length() <= BT_MAC_LEN) {
        LOGI("DmAuthManager::ParseConnectAddr parse bleMac: %{public}s.", GetAnonyString(targetId.bleMac).c_str());
        addr.type = ConnectionAddrType::CONNECTION_ADDR_BLE;
        memcpy_s(addr.info.ble.bleMac, BT_MAC_LEN, targetId.bleMac.c_str(), targetId.bleMac.length());
        if (!targetId.deviceId.empty()) {
            Crypto::ConvertHexStringToBytes(addr.info.ble.udidHash, UDID_HASH_LEN,
                targetId.deviceId.c_str(), targetId.deviceId.length());
        }
        deviceInfo->addr[index] = addr;
        deviceId = targetId.bleMac;
        index++;
    } else {
        LOGE("DmAuthManager::ParseConnectAddr failed, not addr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    deviceInfo->addrNum = static_cast<uint32_t>(index);
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_INPUT_PARA_INVALID);
    if (softbusConnector_->AddMemberToDiscoverMap(deviceId, deviceInfo) != DM_OK) {
        LOGE("DmAuthManager::ParseConnectAddr failed, AddMemberToDiscoverMap failed.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    deviceInfo = nullptr;
    return DM_OK;
}

std::string DmAuthManager::ParseExtraFromMap(const std::map<std::string, std::string> &bindParam)
{
    auto iter = bindParam.find(PARAM_KEY_BIND_EXTRA_DATA);
    if (iter != bindParam.end()) {
        return iter->second;
    }
    return ConvertMapToJsonString(bindParam);
}

bool DmAuthManager::IsAuthCodeReady(const std::string &pkgName)
{
    if (importAuthCode_.empty() || importPkgName_.empty()) {
        LOGE("DmAuthManager::IsAuthCodeReady, auth code not ready.");
        return false;
    }
    if (pkgName != importPkgName_) {
        LOGE("IsAuthCodeReady failed, pkgName not supported.");
        return false;
    }
    return true;
}

int32_t DmAuthManager::DeleteAuthCode()
{
    importAuthCode_ = "";
    importPkgName_ = "";
    return DM_OK;
}

int32_t DmAuthManager::GetAuthCode(const std::string &pkgName, std::string &pinCode)
{
    if (importAuthCode_.empty() || importPkgName_.empty()) {
        LOGE("GetAuthCode failed, auth code not exist.");
        return ERR_DM_FAILED;
    }
    if (pkgName != importPkgName_) {
        LOGE("GetAuthCode failed, pkgName not supported.");
        return ERR_DM_FAILED;
    }
    pinCode = std::to_string(std::atoi(importAuthCode_.c_str()));
    return DM_OK;
}

bool DmAuthManager::IsImportedAuthCodeValid()
{
    std::string pinCode = "";
    CHECK_NULL_RETURN(authRequestContext_, false);
    if (GetAuthCode(authRequestContext_->hostPkgName, pinCode) == DM_OK) {
        return true;
    }
    return false;
}

bool DmAuthManager::IsSrc()
{
    if (authRequestState_ != nullptr) {
        return true;
    } else {
        return false;
    }
}

bool DmAuthManager::IsAuthTypeSupported(const int32_t &authType)
{
    if (authenticationMap_.find(authType) == authenticationMap_.end()) {
        LOGE("IsAuthTypeSupported failed, authType is not supported.");
        return false;
    }
    return true;
}

std::string DmAuthManager::GenerateBindResultContent()
{
    CHECK_NULL_RETURN(authResponseContext_, "");
    JsonObject jsonObj;
    jsonObj[DM_BIND_RESULT_NETWORK_ID] = authResponseContext_->networkId;
    if (remoteDeviceId_.empty()) {
        jsonObj[TAG_DEVICE_ID] = "";
    } else {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        Crypto::GetUdidHash(remoteDeviceId_, reinterpret_cast<uint8_t *>(deviceIdHash));
        jsonObj[TAG_DEVICE_ID] = deviceIdHash;
    }
    jsonObj[TAG_CONFIRM_OPERATION] = authResponseContext_->confirmOperation;
    std::string content = jsonObj.Dump();
    return content;
}

void DmAuthManager::RequestCredential()
{
    LOGI("DmAuthManager::RequestCredential start.");
    std::string publicKey = "";
    GenerateCredential(publicKey);
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authMessageProcessor_);
    CHECK_NULL_VOID(softbusConnector_);
    authResponseContext_->publicKey = publicKey;
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_PUBLICKEY);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
}

void DmAuthManager::GenerateCredential(std::string &publicKey)
{
    LOGI("DmAuthManager::GenerateCredential start.");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = localDeviceId;
    int32_t osAccountId = MultipleUserConnector::GetCurrentAccountUserID();
    CHECK_NULL_VOID(hiChainAuthConnector_);
    hiChainAuthConnector_->GetCredential(localUdid, osAccountId, publicKey);
    if (publicKey == "") {
        hiChainAuthConnector_->GenerateCredential(localUdid, osAccountId, publicKey);
    }
}

void DmAuthManager::RequestCredentialDone()
{
    LOGI("DmAuthManager ExchangeCredentailDone start");
    CHECK_NULL_VOID(authResponseContext_);
    if (authResponseContext_->publicKey == "" ||
        ImportCredential(remoteDeviceId_, authResponseContext_->publicKey) != DM_OK) {
        LOGE("RequestCredentialDone import credential failed.");
    }
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
    }
    CHECK_NULL_VOID(softbusConnector_);
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(authRequestState_);
    if (softbusConnector_->CheckIsOnline(remoteDeviceId_) && !authResponseContext_->isOnline) {
        JoinLnn(authRequestContext_->addr, true);
    } else {
        JoinLnn(authRequestContext_->addr, false);
    }
    authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
    authRequestContext_->reason = DM_OK;
    authResponseContext_->reply = DM_OK;
    authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
}

int32_t DmAuthManager::ImportCredential(std::string &deviceId, std::string &publicKey)
{
    LOGI("DmAuthManager::ImportCredential, deviceId %{public}s", GetAnonyString(deviceId).c_str());
    int32_t osAccountId = MultipleUserConnector::GetCurrentAccountUserID();
    if ((authRequestState_ != nullptr) && (authResponseState_ == nullptr)) {
        // Source Import Credential
        LOGI("Source Import Credential remoteUserId: %{public}d", authRequestContext_->remoteUserId);
        CHECK_NULL_RETURN(authRequestContext_, ERR_DM_POINT_NULL);
        CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
        return hiChainAuthConnector_->ImportCredential(osAccountId, authRequestContext_->remoteUserId, deviceId,
            publicKey);
    } else if ((authResponseState_ != nullptr) && (authRequestState_ == nullptr)) {
        // Sink Import Credential
        LOGI("Source Import Credential remoteUserId: %{public}d", authResponseContext_->remoteUserId);
        CHECK_NULL_RETURN(authResponseContext_, ERR_DM_POINT_NULL);
        CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
        return hiChainAuthConnector_->ImportCredential(osAccountId, authResponseContext_->remoteUserId, deviceId,
            publicKey);
    } else {
        LOGE("DmAuthManager::ImportCredential failed, authRequestState_ or authResponseState_ is invalid.");
        return ERR_DM_AUTH_FAILED;
    }
}

void DmAuthManager::ResponseCredential()
{
    LOGI("DmAuthManager::ResponseCredential start.");
    CHECK_NULL_VOID(authResponseContext_);
    if (authResponseContext_->publicKey == "") {
        LOGE("authResponseContext_->publicKey is empty.");
        authResponseContext_->isFinish = false;
        isFinishOfLocal_ = false;
        CHECK_NULL_VOID(authMessageProcessor_);
        authMessageProcessor_->SetEncryptFlag(false);
        CHECK_NULL_VOID(authResponseState_);
        authResponseContext_->state = authResponseState_->GetStateType();
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
        return;
    }
    std::string publicKey = "";
    GenerateCredential(publicKey);
    if (ImportCredential(remoteDeviceId_, authResponseContext_->publicKey) != DM_OK) {
        LOGE("ResponseCredential import credential failed.");
        CHECK_NULL_VOID(authResponseState_);
        authResponseContext_->state = authResponseState_->GetStateType();
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
        return;
    }
    authResponseContext_->publicKey = publicKey;
    CHECK_NULL_VOID(authMessageProcessor_);
    CHECK_NULL_VOID(softbusConnector_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_PUBLICKEY);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
}

bool DmAuthManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("DmAuthManager::onTransmit start.");
    CHECK_NULL_RETURN(authResponseContext_, false);
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }
    std::string message = "";
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        LOGI("SoftbusSession send msgType %{public}d.", MSG_TYPE_REQ_AUTH_DEVICE_NEGOTIATE);
        message = authMessageProcessor_->CreateDeviceAuthMessage(MSG_TYPE_REQ_AUTH_DEVICE_NEGOTIATE, data, dataLen);
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        LOGI("SoftbusSession send msgType %{public}d.", MSG_TYPE_RESP_AUTH_DEVICE_NEGOTIATE);
        message = authMessageProcessor_->CreateDeviceAuthMessage(MSG_TYPE_RESP_AUTH_DEVICE_NEGOTIATE, data, dataLen);
    }
    CHECK_NULL_RETURN(softbusConnector_, false);
    if (softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message) != DM_OK) {
        LOGE("SoftbusSession send data failed.");
        return false;
    }
    return true;
}

void DmAuthManager::SrcAuthDeviceFinish()
{
    LOGI("DmAuthManager::SrcAuthDeviceFinish Start.");
    CHECK_NULL_VOID(authRequestState_);
    authRequestState_->TransitionTo(std::make_shared<AuthRequestAuthFinish>());
    if (authResponseContext_->confirmOperation != USER_OPERATION_TYPE_ALLOW_AUTH &&
        authResponseContext_->confirmOperation != USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        LOGE("auth failed %{public}d.", authResponseContext_->confirmOperation);
        return;
    }
    if (authResponseContext_->isOnline && authResponseContext_->haveCredential) {
        if (!authResponseContext_->isIdenticalAccount && !authResponseContext_->hostPkgName.empty()) {
            SetProcessInfo();
        }
        softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
        if (timer_ != nullptr) {
            timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
        }
        ConverToFinish();
        return;
    }
    if (authResponseContext_->isOnline && !authResponseContext_->haveCredential) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
        if (!authResponseContext_->isIdenticalAccount && !authResponseContext_->hostPkgName.empty()) {
            SetProcessInfo();
        }
        softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
        if (CompareVersion(remoteVersion_, std::string(DM_VERSION_5_0_2))) {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestReCheckMsg>());
        } else {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestCredential>());
        }
        return;
    }
    if (!authResponseContext_->isOnline && authResponseContext_->haveCredential) {
        JoinLnn(authRequestContext_->addr);
        CHECK_NULL_VOID(timer_);
        timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
        ConverToFinish();
        return;
    }
    if (!authResponseContext_->isOnline && !authResponseContext_->haveCredential) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
        if (CompareVersion(remoteVersion_, std::string(DM_VERSION_5_0_2))) {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestReCheckMsg>());
        } else {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestCredential>());
        }
        return;
    }
}

void DmAuthManager::SinkAuthDeviceFinish()
{
    LOGI("isNeedProcCachedSrcReqMsg %{public}d.", isNeedProcCachedSrcReqMsg_);
    CHECK_NULL_VOID(authResponseState_);
    authResponseState_->TransitionTo(std::make_shared<AuthResponseAuthFinish>());
    CHECK_NULL_VOID(authResponseContext_);
    if (!authResponseContext_->haveCredential) {
        CHECK_NULL_VOID(authUiStateMgr_);
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
    }
    if (authResponseContext_->isOnline) {
        LOGI("The device is online.");
        SetProcessInfo();
        softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
    }
    std::string srcReqMsg = "";
    bool isNeedProcCachedSrcReqMsg = false;
    {
        std::lock_guard<std::mutex> lock(srcReqMsgLock_);
        srcReqMsg = srcReqMsg_;
        isNeedProcCachedSrcReqMsg = isNeedProcCachedSrcReqMsg_;
        srcReqMsg_ = "";
        isNeedProcCachedSrcReqMsg_ = false;
    }
    if (!isNeedProcCachedSrcReqMsg || srcReqMsg.empty()) {
        LOGI("please wait client request.");
        return;
    }
    CHECK_NULL_VOID(authMessageProcessor_);
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    if (authMessageProcessor_->ParseMessage(srcReqMsg) != DM_OK) {
        LOGE("ParseMessage failed.");
        return;
    }
    CHECK_NULL_VOID(authResponseState_);
    if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_5_0_2))) {
        authResponseState_->TransitionTo(std::make_shared<AuthResponseCredential>());
    } else {
        authResponseState_->TransitionTo(std::make_shared<AuthResponseReCheckMsg>());
    }
}

void DmAuthManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("DmAuthManager::AuthDeviceFinish start.");
    CHECK_NULL_VOID(authResponseContext_);
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    isAuthDevice_ = false;
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(AUTH_DEVICE_TIMEOUT_TASK));
    }
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_5_0_2))) {
        LOGI("Set EncryptFlag true.");
        authMessageProcessor_->SetEncryptFlag(true);
    } else {
        PutAccessControlList();
    }
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        SrcAuthDeviceFinish();
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        SinkAuthDeviceFinish();
    }
}

void DmAuthManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthDeviceError start.");
    CHECK_NULL_VOID(authResponseContext_);
    if (requestId != authResponseContext_->requestId) {
        LOGE("reqId: %{public}" PRId64", RespReqId: %{public}" PRId64".", requestId, authResponseContext_->requestId);
        return;
    }
    isAuthDevice_ = false;
    if (authRequestState_ == nullptr || authResponseState_ != nullptr) {
        if (CheckNeedShowAuthInfoDialog(errorCode)) {
            return;
        }
        authTimes_++;
        if (authTimes_ >= MAX_AUTH_TIMES) {
            authResponseContext_->isFinish = false;
            authResponseContext_->reply = ERR_DM_AUTH_CODE_INCORRECT;
            authResponseContext_->state = AuthState::AUTH_RESPONSE_SHOW;
            isFinishOfLocal_ = false;
            CHECK_NULL_VOID(authResponseState_);
            authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
        }
        return;
    }
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
        authRequestContext_->reason = ERR_DM_AUTH_CODE_INCORRECT;
        authResponseContext_->reply = ERR_DM_AUTH_CODE_INCORRECT;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return;
    }
    authTimes_++;
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(AUTH_DEVICE_TIMEOUT_TASK));
    }
    if (errorCode != DM_OK || requestId != authResponseContext_->requestId) {
        if (authRequestState_ != nullptr && authTimes_ >= MAX_AUTH_TIMES) {
            authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
            authRequestContext_->reason = ERR_DM_INPUT_PARA_INVALID;
            authResponseContext_->reply = ERR_DM_INPUT_PARA_INVALID;
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        } else {
            UpdateInputPincodeDialog(errorCode);
        }
    }
}

void DmAuthManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("DmAuthManager::AuthDeviceSessionKey start. keyLen: %{public}u", sessionKeyLen);
    CHECK_NULL_VOID(authResponseContext_);
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    CHECK_NULL_VOID(authMessageProcessor_);
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_5_0_4))) {
        if (authMessageProcessor_->ProcessSessionKey(sessionKey, sessionKeyLen) != DM_OK) {
            LOGE("Process session key err.");
            return;
        }
    } else {
        if (authMessageProcessor_->SaveSessionKey(sessionKey, sessionKeyLen) != DM_OK) {
            LOGE("Save session key err.");
            return;
        }
    }
    authResponseContext_->localSessionKeyId = 0;
    {
        std::lock_guard<std::mutex> guard(sessionKeyIdMutex_);
        sessionKeyIdAsyncResult_.clear();
        sessionKeyIdAsyncResult_[requestId] = std::optional<int32_t>();
    }
    unsigned char hash[SHA256_DIGEST_LENGTH] = { 0 };
    Crypto::DmGenerateStrHash(sessionKey, sessionKeyLen, hash, SHA256_DIGEST_LENGTH, 0);
    std::vector<unsigned char> hashVector(hash, hash + SHA256_DIGEST_LENGTH);
    std::shared_ptr<DmAuthManager> sharePtrThis = shared_from_this();
    auto asyncTaskFunc = [sharePtrThis, requestId, hashVector]() {
        sharePtrThis->PutSessionKeyAsync(requestId, hashVector);
    };
    ffrt::submit(asyncTaskFunc, ffrt::task_attr().delay(0));
}

void DmAuthManager::PutSessionKeyAsync(int64_t requestId, std::vector<unsigned char> hash)
{
    {
        std::lock_guard<std::mutex> guard(sessionKeyIdMutex_);
        int32_t sessionKeyId = 0;
        int32_t ret =
            DeviceProfileConnector::GetInstance().PutSessionKey(MultipleUserConnector::GetCurrentAccountUserID(),
                hash, sessionKeyId);
        if (ret != DM_OK) {
            LOGI("PutSessionKey failed.");
            sessionKeyId = 0;
        }
        sessionKeyIdAsyncResult_[requestId] = sessionKeyId;
    }
    sessionKeyIdCondition_.notify_one();
}

int32_t DmAuthManager::GetSessionKeyIdSync(int64_t requestId)
{
    std::unique_lock<std::mutex> guard(sessionKeyIdMutex_);
    if (sessionKeyIdAsyncResult_.find(requestId) == sessionKeyIdAsyncResult_.end()) {
        LOGW("GetSessionKeyIdSync failed, not find by requestId");
        return 0;
    }
    if (sessionKeyIdAsyncResult_[requestId].has_value()) {
        LOGI("GetSessionKeyIdSync, already ready");
        return sessionKeyIdAsyncResult_[requestId].value();
    }
    LOGI("GetSessionKeyIdSync need wait");
    sessionKeyIdCondition_.wait_for(guard, std::chrono::milliseconds(MAX_PUT_SESSIONKEY_TIMEOUT));
    int32_t keyid = sessionKeyIdAsyncResult_[requestId].value_or(0);
    LOGI("GetSessionKeyIdSync exit");
    return keyid;
}

char *DmAuthManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("DmAuthManager::AuthDeviceRequest start.");
    (void)requestId;
    (void)reqParams;
    JsonObject jsonObj;
    std::string pinCode = "";
    if (GetPinCode(pinCode) == ERR_DM_FAILED || pinCode == "") {
        jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_REJECTED;
    } else {
        jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
        jsonObj[FIELD_PIN_CODE] = pinCode;
    }
    std::string pinCodeHash = GetAnonyString(Crypto::Sha256(pinCode));
    LOGI("AuthDeviceRequest pinCodeHash: %{public}s", pinCodeHash.c_str());
    std::string deviceId = "";
    GetRemoteDeviceId(deviceId);
    jsonObj[FIELD_PEER_CONN_DEVICE_ID] = deviceId;
    std::string jsonStr = jsonObj.Dump();
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}

void DmAuthManager::GetRemoteDeviceId(std::string &deviceId)
{
    LOGI("GetRemoteDeviceId start.");
    deviceId = remoteDeviceId_;
}

void DmAuthManager::CompatiblePutAcl()
{
    LOGI("DmAuthManager::CompatiblePutAcl");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    char mUdidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    Crypto::GetUdidHash(localUdid, reinterpret_cast<uint8_t *>(mUdidHash));
    std::string localUdidHash = static_cast<std::string>(mUdidHash);
    DmAclInfo aclInfo;
    aclInfo.bindLevel = USER;
    aclInfo.bindType = DM_POINT_TO_POINT;
    aclInfo.trustDeviceId = remoteDeviceId_;
    if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        aclInfo.authenticationType = ALLOW_AUTH_ALWAYS;
    } else if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH) {
        aclInfo.authenticationType = ALLOW_AUTH_ONCE;
    }
    aclInfo.deviceIdHash = localUdidHash;

    DmAccesser accesser;
    DmAccessee accessee;
    if (authRequestState_ != nullptr && authResponseState_ == nullptr && authResponseContext_ != nullptr &&
        authRequestContext_ != nullptr) {
        accesser.requestBundleName = authResponseContext_->hostPkgName;
        accesser.requestDeviceId = localUdid;
        accesser.requestUserId = MultipleUserConnector::GetCurrentAccountUserID();
        accesser.requestAccountId = MultipleUserConnector::GetAccountInfoByUserId(accesser.requestUserId).accountId;
        accesser.requestTokenId = static_cast<uint64_t>(authRequestContext_->tokenId);
        accessee.trustBundleName = authResponseContext_->hostPkgName;
        accessee.trustDeviceId = remoteDeviceId_;
        accessee.trustUserId = -1;
    }
    if (authRequestState_ == nullptr && authResponseState_ != nullptr && authResponseContext_ != nullptr) {
        accesser.requestBundleName = authResponseContext_->hostPkgName;
        accesser.requestDeviceId = remoteDeviceId_;
        accesser.requestUserId = authResponseContext_->remoteUserId;
        accesser.requestAccountId = authResponseContext_->remoteAccountId;
        accesser.requestTokenId = static_cast<uint64_t>(authResponseContext_->remoteTokenId);
        accessee.trustBundleName = authResponseContext_->hostPkgName;
        accessee.trustDeviceId = localUdid;
        accessee.trustUserId = MultipleUserConnector::GetCurrentAccountUserID();
        accessee.trustAccountId = MultipleUserConnector::GetAccountInfoByUserId(accessee.trustUserId).accountId;
        accessee.trustTokenId = static_cast<uint64_t>(authResponseContext_->tokenId);
    }
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, accesser, accessee);
}

void DmAuthManager::ProcRespNegotiateExt(const int32_t &sessionId)
{
    LOGI("DmAuthManager::ProcRespNegotiateExt start.");
    CHECK_NULL_VOID(authResponseContext_);
    remoteDeviceId_ = authResponseContext_->localDeviceId;
    authResponseContext_->remoteAccountId = authResponseContext_->localAccountId;
    authResponseContext_->remoteUserId = authResponseContext_->localUserId;
    GetBinderInfo();
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    authResponseContext_->deviceId = authResponseContext_->localDeviceId;
    authResponseContext_->localDeviceId = static_cast<std::string>(localDeviceId);
    authResponseContext_->bindType =
        DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(authResponseContext_->hostPkgName,
        authResponseContext_->localDeviceId, authResponseContext_->deviceId);
    authResponseContext_->authed = !authResponseContext_->bindType.empty();

    authResponseContext_->isIdenticalAccount = false;
    if (authResponseContext_->localAccountId == authResponseContext_->remoteAccountId &&
        authResponseContext_->localAccountId != "ohosAnonymousUid" && authResponseContext_->authed) {
        authResponseContext_->isIdenticalAccount = true;
    }

    CHECK_NULL_VOID(softbusConnector_);
    authResponseContext_->isOnline = softbusConnector_->CheckIsOnline(remoteDeviceId_);
    CHECK_NULL_VOID(hiChainAuthConnector_);
    authResponseContext_->haveCredential =
        hiChainAuthConnector_->QueryCredential(authResponseContext_->deviceId,
            MultipleUserConnector::GetFirstForegroundUserId(), authResponseContext_->remoteUserId);
    if (!IsAuthTypeSupported(authResponseContext_->authType)) {
        LOGE("DmAuthManager::AuthenticateDevice authType %{public}d not support.", authResponseContext_->authType);
        authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    } else {
        authPtr_ = authenticationMap_[authResponseContext_->authType];
    }
    if (IsAuthCodeReady(authResponseContext_->hostPkgName)) {
        authResponseContext_->isAuthCodeReady = true;
    } else {
        authResponseContext_->isAuthCodeReady = false;
    }
    CHECK_NULL_VOID(authMessageProcessor_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_NEGOTIATE);
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);

    if (authResponseContext_->authType == AUTH_TYPE_NFC) {
        GetLocalServiceInfoInDp();
    }
}

void DmAuthManager::ProcRespNegotiate(const int32_t &sessionId)
{
    LOGI("DmAuthManager::ProcRespNegotiate session id");
    AbilityNegotiate();
    CHECK_NULL_VOID(softbusConnector_);
    CHECK_NULL_VOID(authMessageProcessor_);
    authResponseContext_->isOnline = softbusConnector_->CheckIsOnline(remoteDeviceId_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_NEGOTIATE);
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
        return;
    }
    if (!IsBool(jsonObject, TAG_CRYPTO_SUPPORT)) {
        LOGE("err json string.");
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
        return;
    }
    jsonObject[TAG_ACCOUNT_GROUPID] = GetAccountGroupIdHash();
    CHECK_NULL_VOID(authResponseState_);
    authResponseContext_ = authResponseState_->GetAuthContext();
    if (jsonObject[TAG_CRYPTO_SUPPORT].Get<bool>() == true && authResponseContext_->cryptoSupport) {
        if (IsString(jsonObject, TAG_CRYPTO_NAME) && IsString(jsonObject, TAG_CRYPTO_VERSION)) {
            if (jsonObject[TAG_CRYPTO_NAME].Get<std::string>() == authResponseContext_->cryptoName &&
                jsonObject[TAG_CRYPTO_VERSION].Get<std::string>() == authResponseContext_->cryptoVer) {
                isCryptoSupport_ = true;
                softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
                return;
            }
        }
    }
    jsonObject[TAG_CRYPTO_SUPPORT] = false;
    message = jsonObject.Dump();
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
}

void DmAuthManager::ProcIncompatible(const int32_t &sessionId)
{
    LOGI("DmAuthManager::ProcIncompatible sessionId %{public}d.", sessionId);
    JsonObject respNegotiateMsg;
    respNegotiateMsg[TAG_REPLY] = ERR_DM_VERSION_INCOMPATIBLE;
    respNegotiateMsg[TAG_VER] = DM_ITF_VER;
    respNegotiateMsg[TAG_MSG_TYPE] = MSG_TYPE_RESP_NEGOTIATE;
    std::string message = respNegotiateMsg.Dump();
    CHECK_NULL_VOID(softbusConnector_);
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
}

void DmAuthManager::OnAuthDeviceDataReceived(const int32_t sessionId, const std::string message)
{
    if (authResponseContext_ == nullptr || authMessageProcessor_ == nullptr || hiChainAuthConnector_ == nullptr ||
        authResponseContext_->sessionId != sessionId) {
        LOGE("OnAuthDeviceDataReceived param is invalid");
        return;
    }
    authResponseContext_->sessionId = sessionId;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return;
    }
    if (!IsString(jsonObject, TAG_DATA) || !IsInt32(jsonObject, TAG_DATA_LEN) || !IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("Auth device data is error.");
        return;
    }
    LOGI("OnAuthDeviceDataReceived start msgType %{public}d.", jsonObject[TAG_MSG_TYPE].Get<int32_t>());
    std::string authData = jsonObject[TAG_DATA].Get<std::string>();
    int32_t osAccountId = MultipleUserConnector::GetCurrentAccountUserID();
    hiChainAuthConnector_->ProcessAuthData(authResponseContext_->requestId, authData, osAccountId);
}

int32_t DmAuthManager::DeleteGroup(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DmAuthManager::DeleteGroup");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_POINT_NULL);
    hiChainConnector_->GetRelatedGroups(deviceId, groupList);
    for (const auto &item : groupList) {
        std::string groupId = item.groupId;
        hiChainConnector_->DeleteGroup(groupId);
    }
    return DM_OK;
}

int32_t DmAuthManager::DeleteGroup(const std::string &pkgName, int32_t userId, const std::string &deviceId)
{
    LOGI("DmAuthManager::DeleteGroup");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_FAILED);
    hiChainConnector_->GetRelatedGroups(userId, deviceId, groupList);
    if (groupList.size() > 0) {
        std::string groupId = "";
        groupId = groupList.front().groupId;
        hiChainConnector_->DeleteGroup(userId, groupId);
    } else {
        LOGE("DmAuthManager::UnAuthenticateDevice groupList.size = 0");
        return ERR_DM_FAILED;
    }
    if (softbusConnector_ != nullptr) {
        softbusConnector_->EraseUdidFromMap(deviceId);
    }
    return DM_OK;
}

void DmAuthManager::PutAccessControlList()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    char mUdidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    Crypto::GetUdidHash(localUdid, reinterpret_cast<uint8_t *>(mUdidHash));
    std::string localUdidHash = static_cast<std::string>(mUdidHash);
    DmAclInfo aclInfo;
    aclInfo.bindType = DM_ACROSS_ACCOUNT;
    CHECK_NULL_VOID(authResponseContext_);
    if (authResponseContext_->localAccountId == "ohosAnonymousUid" ||
        authResponseContext_->remoteAccountId == "ohosAnonymousUid") {
        aclInfo.bindType = DM_POINT_TO_POINT;
        authForm_ = DmAuthForm::PEER_TO_PEER;
    }
    aclInfo.bindLevel = authResponseContext_->bindLevel;
    aclInfo.trustDeviceId = remoteDeviceId_;
    aclInfo.authenticationType = ALLOW_AUTH_ONCE;
    if (authResponseContext_->confirmOperation == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        aclInfo.authenticationType = ALLOW_AUTH_ALWAYS;
    }
    aclInfo.deviceIdHash = localUdidHash;
    DmAccesser accesser;
    accesser.requestBundleName = authResponseContext_->hostPkgName;
    DmAccessee accessee;
    accessee.trustBundleName = authResponseContext_->peerBundleName;
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        PutSrcAccessControlList(accesser, accessee, localUdid);
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        PutSinkAccessControlList(accesser, accessee, localUdid);
    }
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, accesser, accessee);
}

void DmAuthManager::PutSrcAccessControlList(DmAccesser &accesser, DmAccessee &accessee,
    const std::string &localUdid)
{
    CHECK_NULL_VOID(authRequestContext_);
    accesser.requestTokenId = static_cast<uint64_t>(authRequestContext_->tokenId);
    accesser.requestUserId = authRequestContext_->localUserId;
    accesser.requestAccountId = authRequestContext_->localAccountId;
    accesser.requestDeviceId = authRequestContext_->localDeviceId;
    accesser.requestDeviceName = authRequestContext_->localDeviceName;
    CHECK_NULL_VOID(authResponseContext_);
    if (authResponseContext_->remoteTokenId == authRequestContext_->tokenId) {
        accessee.trustTokenId = 0;
    } else {
        accessee.trustTokenId = static_cast<uint64_t>(authResponseContext_->remoteTokenId);
    }
    accessee.trustUserId = authRequestContext_->remoteUserId;
    accessee.trustAccountId = authRequestContext_->remoteAccountId;
    accessee.trustDeviceId = remoteDeviceId_;
    accessee.trustDeviceName = authRequestContext_->remoteDeviceName;
}

void DmAuthManager::PutSinkAccessControlList(DmAccesser &accesser, DmAccessee &accessee,
    const std::string &localUdid)
{
    CHECK_NULL_VOID(authResponseContext_);
    accesser.requestTokenId = static_cast<uint64_t>(authResponseContext_->remoteTokenId);
    accesser.requestUserId = authResponseContext_->remoteUserId;
    accesser.requestAccountId = authResponseContext_->remoteAccountId;
    accesser.requestDeviceId = remoteDeviceId_;
    accesser.requestDeviceName = authResponseContext_->remoteDeviceName;
    if (authResponseContext_->remoteTokenId == authResponseContext_->tokenId) {
        accessee.trustTokenId = 0;
    } else {
        accessee.trustTokenId = static_cast<uint64_t>(authResponseContext_->tokenId);
    }
    accessee.trustUserId = authResponseContext_->localUserId;
    accessee.trustAccountId = authResponseContext_->localAccountId;
    accessee.trustDeviceId = localUdid;
    accessee.trustDeviceName = authResponseContext_->targetDeviceName;
}

void DmAuthManager::HandleSessionHeartbeat(std::string name)
{
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK));
    }
    LOGI("DmAuthManager::HandleSessionHeartbeat name %{public}s", name.c_str());
    JsonObject jsonObj;
    jsonObj[TAG_SESSION_HEARTBEAT] = TAG_SESSION_HEARTBEAT;
    std::string message = jsonObj.Dump();
    CHECK_NULL_VOID(softbusConnector_);
    CHECK_NULL_VOID(authResponseContext_);
    softbusConnector_->GetSoftbusSession()->SendHeartbeatData(authResponseContext_->sessionId, message);

    CHECK_NULL_VOID(authRequestState_);
    if (authRequestState_ != nullptr) {
        if (timer_ != nullptr) {
            timer_->StartTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK),
                GetTaskTimeout(SESSION_HEARTBEAT_TIMEOUT_TASK, SESSION_HEARTBEAT_TIMEOUT), [this] (std::string name) {
                    DmAuthManager::HandleSessionHeartbeat(name);
                });
        }
    }
    LOGI("DmAuthManager::HandleSessionHeartbeat complete");
}

int32_t DmAuthManager::CheckTrustState()
{
    bool isSameGroup = false;
    CHECK_NULL_RETURN(authResponseContext_, ERR_DM_FAILED);
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_FAILED);
    CHECK_NULL_RETURN(authRequestContext_, ERR_DM_FAILED);
    if (authResponseContext_->reply == ERR_DM_AUTH_PEER_REJECT &&
        hiChainConnector_->IsDevicesInP2PGroup(authResponseContext_->localDeviceId,
                                               authRequestContext_->localDeviceId)) {
        isSameGroup = true;
    }
    if (isSameGroup && authResponseContext_->isOnline && authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        authResponseContext_->isFinish = true;
        SetReasonAndFinish(DM_OK, AuthState::AUTH_REQUEST_FINISH);
        return ALREADY_BIND;
    }
    if (authResponseContext_->isIdenticalAccount) {
        if (IsIdenticalAccount()) {
            JoinLnn(authResponseContext_->deviceId);
            authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
            authRequestContext_->reason = DM_OK;
            authResponseContext_->reply = DM_OK;
            CHECK_NULL_RETURN(authRequestState_, ERR_DM_FAILED);
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return ALREADY_BIND;
        }
    }
    if (isSameGroup) {
        if (!DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(authResponseContext_->hostPkgName,
            authResponseContext_->localDeviceId)) {
            CompatiblePutAcl();
        }
        JoinLnn(authResponseContext_->deviceId);
        authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
        CHECK_NULL_RETURN(authRequestState_, ERR_DM_FAILED);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return ALREADY_BIND;
    }
    if (authResponseContext_->reply == ERR_DM_UNSUPPORTED_AUTH_TYPE ||
        (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        authResponseContext_->isAuthCodeReady == false)) {
        authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
        authRequestContext_->reason = ERR_DM_BIND_PEER_UNSUPPORTED;
        CHECK_NULL_RETURN(authRequestState_, ERR_DM_FAILED);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return ERR_DM_BIND_PEER_UNSUPPORTED;
    }
    return DM_OK;
}

bool DmAuthManager::IsScreenLocked()
{
    bool isLocked = false;
#if defined(SUPPORT_SCREENLOCK)
    isLocked = OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked();
#endif
    LOGI("IsScreenLocked isLocked: %{public}d.", isLocked);
    return isLocked;
}

void DmAuthManager::OnScreenLocked()
{
    if (authResponseContext_ != nullptr && AUTH_TYPE_IMPORT_AUTH_CODE == authResponseContext_->authType) {
        LOGI("OnScreenLocked authtype is: %{public}d, no need stop bind.", authResponseContext_->authType);
        return;
    }
    if (authRequestState_ == nullptr) {
        LOGE("OnScreenLocked authRequestState_ is nullptr.");
        return;
    }
    if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_NEGOTIATE ||
        authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_INIT) {
        LOGI("OnScreenLocked stop bind.");
        SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL, STATUS_DM_AUTH_DEFAULT);
        return;
    }
    if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_JOIN) {
        LOGI("OnScreenLocked stop user input.");
        if (authUiStateMgr_ != nullptr) {
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
        }
        SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL, STATUS_DM_AUTH_DEFAULT);
        return;
    }
    if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_NEGOTIATE_DONE) {
        LOGI("OnScreenLocked stop confirm.");
        SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL, STATUS_DM_AUTH_DEFAULT);
    }
}

void DmAuthManager::HandleDeviceNotTrust(const std::string &udid)
{
    LOGI("DmAuthManager::HandleDeviceNotTrust udid: %{public}s.", GetAnonyString(udid).c_str());
    if (udid.empty()) {
        LOGE("DmAuthManager::HandleDeviceNotTrust udid is empty.");
        return;
    }
    DeviceProfileConnector::GetInstance().DeleteAccessControlList(udid);
    CHECK_NULL_VOID(hiChainConnector_);
    hiChainConnector_->DeleteAllGroupByUdid(udid);
}

std::string DmAuthManager::ConvertSinkVersion(const std::string &version)
{
    std::string sinkVersion = "";
    if (version == "") {
        sinkVersion = DM_VERSION_4_1_5_1;
    } else {
        sinkVersion = version;
    }
    LOGI("ConvertSinkVersion version %{public}s, sinkVersion is %{public}s.", version.c_str(), sinkVersion.c_str());
    return sinkVersion;
}

void DmAuthManager::SetAuthType(int32_t authType)
{
    authType_ = authType;
}

int32_t DmAuthManager::GetTaskTimeout(const char* taskName, int32_t taskTimeOut)
{
    LOGI("GetTaskTimeout, taskName: %{public}s, authType_: %{public}d", taskName, authType_.load());
    if (AUTH_TYPE_IMPORT_AUTH_CODE == authType_) {
        auto timeout = TASK_TIME_OUT_MAP.find(std::string(taskName));
        if (timeout != TASK_TIME_OUT_MAP.end()) {
            return timeout->second;
        }
    }
    return taskTimeOut;
}

int32_t DmAuthManager::GetBinderInfo()
{
    CHECK_NULL_RETURN(authResponseContext_, ERR_DM_POINT_NULL);
    if (authResponseContext_->bundleName.empty()) {
        LOGI("bundleName is empty");
        authResponseContext_->localUserId = MultipleUserConnector::GetCurrentAccountUserID();
        authResponseContext_->localAccountId = MultipleUserConnector::GetOhosAccountId();
        return DM_OK;
    }
    authResponseContext_->localUserId = MultipleUserConnector::GetFirstForegroundUserId();
    authResponseContext_->localAccountId =
        MultipleUserConnector::GetOhosAccountIdByUserId(authResponseContext_->localUserId);
    if (authResponseContext_->peerBundleName == authResponseContext_->hostPkgName) {
        bundleName_ = authResponseContext_->bundleName;
    } else {
        bundleName_ = authResponseContext_->peerBundleName;
    }
    int32_t ret = AppManager::GetInstance().
        GetNativeTokenIdByName(authResponseContext_->bundleName, authResponseContext_->tokenId);
    if (ret == DM_OK) {
        LOGI("bundleName is sa");
        return DM_OK;
    }
    ret = AppManager::GetInstance().GetHapTokenIdByName(authResponseContext_->localUserId,
        authResponseContext_->peerBundleName, 0, authResponseContext_->tokenId);
#ifndef DEVICE_MANAGER_COMMON_FLAG
    if (ret == DM_OK && static_cast<uint32_t>(authResponseContext_->bindLevel) != APP) {
        LOGI("get tokenId by bundleName failed %{public}s", GetAnonyString(authResponseContext_->bundleName).c_str());
        return ERR_DM_FAILED;
    }
#endif
    return ret;
}

void DmAuthManager::SetProcessInfo()
{
    CHECK_NULL_VOID(authResponseContext_);
    ProcessInfo processInfo;
    if (static_cast<uint32_t>(authResponseContext_->bindLevel) == APP) {
        if ((authRequestState_ != nullptr) && (authResponseState_ == nullptr) && authRequestContext_ != nullptr) {
            processInfo.pkgName = authResponseContext_->hostPkgName;
            processInfo.userId = authRequestContext_->localUserId;
        } else if ((authRequestState_ == nullptr) && (authResponseState_ != nullptr)) {
            processInfo.pkgName = authResponseContext_->peerBundleName;
            processInfo.userId = authResponseContext_->localUserId;
        } else {
            LOGE("DMAuthManager::SetProcessInfo failed, state is invalid.");
        }
    } else if (static_cast<uint32_t>(authResponseContext_->bindLevel) == USER ||
        static_cast<uint32_t>(authResponseContext_->bindLevel) == INVALIED_TYPE) {
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = authResponseContext_->localUserId;
    } else {
        LOGE("bindlevel error %{public}d.", authResponseContext_->bindLevel);
        return;
    }
    CHECK_NULL_VOID(softbusConnector_);
    softbusConnector_->SetProcessInfo(processInfo);
}

void DmAuthManager::ConverToFinish()
{
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authRequestState_);
    authRequestContext_->reason = DM_OK;
    authResponseContext_->reply = DM_OK;
    authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
    authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
}

void DmAuthManager::RequestReCheckMsg()
{
    LOGI("dmVersion %{public}s.", DM_VERSION_5_0_5);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    uint32_t tokenId = 0;
    int32_t localUserId = 0;
    MultipleUserConnector::GetTokenIdAndForegroundUserId(tokenId, localUserId);
    std::string localAccountId = MultipleUserConnector::GetOhosAccountIdByUserId(localUserId);
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(authMessageProcessor_);
    authResponseContext_->edition = DM_VERSION_5_0_5;
    authResponseContext_->localDeviceId = static_cast<std::string>(localDeviceId);
    authResponseContext_->localUserId = localUserId;
    authResponseContext_->bundleName = authRequestContext_->hostPkgName;
    authResponseContext_->bindLevel = authRequestContext_->bindLevel;
    authResponseContext_->localAccountId = localAccountId;
    authResponseContext_->tokenId = authRequestContext_->tokenId;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_RECHECK_MSG);
    CHECK_NULL_VOID(softbusConnector_);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
}

void DmAuthManager::ResponseReCheckMsg()
{
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authMessageProcessor_);
    LOGI("remoteVersion %{public}s, authResponseContext_->edition %{public}s.",
        remoteVersion_.c_str(), authResponseContext_->edition.c_str());
    if (!IsSinkMsgValid()) {
        LOGE("peer deviceId not trust.");
        authResponseContext_->isFinish = false;
        isFinishOfLocal_ = false;
        authMessageProcessor_->SetEncryptFlag(false);
        int32_t sessionId = authResponseContext_->sessionId;
        CHECK_NULL_VOID(authResponseState_);
        authResponseContext_->state = authResponseState_->GetStateType();
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
        return;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    authResponseContext_->edition = DM_VERSION_5_0_5;
    authResponseContext_->localDeviceId = std::string(localDeviceId);
    authResponseContext_->localUserId = MultipleUserConnector::GetFirstForegroundUserId();
    authResponseContext_->localAccountId =
        MultipleUserConnector::GetOhosAccountIdByUserId(authResponseContext_->localUserId);
    if (AppManager::GetInstance().GetNativeTokenIdByName(bundleName_, authResponseContext_->tokenId) != DM_OK) {
        LOGE("BundleName %{public}s, GetNativeTokenIdByName failed.", GetAnonyString(bundleName_).c_str());
        if (AppManager::GetInstance().GetHapTokenIdByName(authResponseContext_->localUserId,
            bundleName_, 0, authResponseContext_->tokenId) != DM_OK) {
            LOGE("get tokenId by bundleName failed %{public}s", GetAnonyString(bundleName_).c_str());
            authResponseContext_->tokenId = 0;
        }
    }
    authResponseContext_->bundleName = authResponseContext_->peerBundleName;
    authMessageProcessor_->SetEncryptFlag(true);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_RECHECK_MSG);
    CHECK_NULL_VOID(softbusConnector_);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    PutAccessControlList();
}

void DmAuthManager::RequestReCheckMsgDone()
{
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authRequestState_);
    LOGI("remoteVersion %{public}s, authResponseContext_->edition %{public}s.",
        remoteVersion_.c_str(), authResponseContext_->edition.c_str());
    if (!IsSourceMsgValid()) {
        LOGE("peer deviceId not trust.");
        authResponseContext_->isFinish = false;
        isFinishOfLocal_ = false;
        CHECK_NULL_VOID(authMessageProcessor_);
        authMessageProcessor_->SetEncryptFlag(false);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return;
    }
    authRequestState_->TransitionTo(std::make_shared<AuthRequestCredential>());
    PutAccessControlList();
}

bool DmAuthManager::IsSinkMsgValid()
{
    LOGI("sink remoteVersion %{public}s, remoteDeviceId %{public}s, remoteUserId %{public}d,"
        "remoteHostPkgName %{public}s, remoteBindLevel %{public}d", remoteVersion_.c_str(),
        GetAnonyString(remoteDeviceId_).c_str(), authResponseContext_->remoteUserId,
        authResponseContext_->hostPkgName.c_str(), authResponseContext_->bindLevel);
    LOGI("src version %{public}s, deviceId %{public}s, userId %{public}d, hostPkgName %{public}s, bindLevel %{public}d",
        authResponseContext_->edition.c_str(), GetAnonyString(authResponseContext_->localDeviceId).c_str(),
        authResponseContext_->localUserId, authResponseContext_->bundleName.c_str(),
        authResponseContext_->localBindLevel);
    if (authResponseContext_->edition != remoteVersion_ ||
        authResponseContext_->localDeviceId != remoteDeviceId_ ||
        authResponseContext_->localUserId != authResponseContext_->remoteUserId ||
        authResponseContext_->bundleName != authResponseContext_->hostPkgName ||
        authResponseContext_->localBindLevel != authResponseContext_->bindLevel) {
        return false;
    }
    return true;
}

bool DmAuthManager::IsSourceMsgValid()
{
    CHECK_NULL_RETURN(authResponseContext_, false);
    CHECK_NULL_RETURN(authRequestContext_, false);
    if (authResponseContext_->edition != remoteVersion_ ||
        authResponseContext_->localDeviceId != remoteDeviceId_ ||
        authResponseContext_->localUserId != authRequestContext_->remoteUserId ||
        authResponseContext_->bundleName != authResponseContext_->peerBundleName ||
        authResponseContext_->localBindLevel != authResponseContext_->bindLevel) {
        return false;
    }
    authResponseContext_->localAccountId = authRequestContext_->localAccountId;
    return true;
}

int32_t DmAuthManager::RegisterAuthenticationType(int32_t authenticationType)
{
    authenticationType_ = authenticationType;
    return DM_OK;
}

void DmAuthManager::ProcessReqPublicKey()
{
    CHECK_NULL_VOID(authResponseState_);
    if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_AUTH_FINISH ||
        authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_RECHECK_MSG) {
        authResponseState_->TransitionTo(std::make_shared<AuthResponseCredential>());
        return;
    }
    if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_SHOW) {
        std::lock_guard<std::mutex> lock(srcReqMsgLock_);
        isNeedProcCachedSrcReqMsg_ = true;
    }
}

void DmAuthManager::GetLocalServiceInfoInDp()
{
    CHECK_NULL_VOID(authResponseContext_);
    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;
    int32_t result = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        authResponseContext_->hostPkgName, (int32_t)DMLocalServiceInfoPinExchangeType::FROMDP, localServiceInfo);
    if (result != DM_OK) {
        return;
    }
    if (IsLocalServiceInfoValid(localServiceInfo)) {
        serviceInfoProfile_ = localServiceInfo;
        LOGI("authBoxType %{public}d, authType %{public}d, pinExchangeType %{public}d",
            serviceInfoProfile_.GetAuthBoxType(), serviceInfoProfile_.GetAuthType(),
            serviceInfoProfile_.GetPinExchangeType());
        auto updateProfile = serviceInfoProfile_;
        updateProfile.SetPinCode("******");
        DeviceProfileConnector::GetInstance().UpdateLocalServiceInfo(updateProfile);
    }
}

bool DmAuthManager::CheckNeedShowAuthInfoDialog(int32_t errorCode)
{
    CHECK_NULL_RETURN(authResponseContext_, false);
    if (errorCode == ERR_DM_HICHAIN_PROOFMISMATCH && !pincodeDialogEverShown_ && CanUsePincodeFromDp()) {
        authResponseContext_->code = GeneratePincode();
        LOGI("AuthDeviceError, GeneratePincode, ShowAuthInfoDialog");
        authTimes_ = 0;
        ShowAuthInfoDialog(true);
        return true;
    }
    return false;
}

void DmAuthManager::UpdateInputPincodeDialog(int32_t errorCode)
{
    if (timer_ != nullptr) {
        timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK),
            GetTaskTimeout(INPUT_TIMEOUT_TASK, INPUT_TIMEOUT), [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
    }
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(authUiStateMgr_);
    if (authResponseContext_->authType == AUTH_TYPE_NFC && !pincodeDialogEverShown_ &&
        IsImportedAuthCodeValid() && errorCode == ERR_DM_HICHAIN_PROOFMISMATCH) {
        LOGI("AuthDeviceError, ShowStartAuthDialog");
        authTimes_ = 0;
        DeleteAuthCode();
        ShowStartAuthDialog();
    } else {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_PIN_CODE_ERROR);
    }
}

void DmAuthManager::JoinLnn(const std::string &deviceId, bool isForceJoin)
{
    CHECK_NULL_VOID(authRequestContext_);
    CHECK_NULL_VOID(authResponseContext_);
    CHECK_NULL_VOID(softbusConnector_);
    if (IsHmlSessionType()) {
        if (authRequestContext_->closeSessionDelaySeconds == 0) {
            isWaitingJoinLnnCallback_ = true;
        }
        authResponseContext_->localSessionKeyId = GetSessionKeyIdSync(authResponseContext_->requestId);
        softbusConnector_->JoinLnnByHml(authRequestContext_->sessionId, authResponseContext_->localSessionKeyId,
            authResponseContext_->remoteSessionKeyId);
        return;
    }
    if (isNeedJoinLnn_) {
        LOGI("isNeedJoinLnn %{public}d", isNeedJoinLnn_);
        softbusConnector_->JoinLnn(deviceId, isForceJoin);
    }
}

void DmAuthManager::OnSoftbusJoinLNNResult(const int32_t sessionId, const char *networkId, int32_t result)
{
    (void)sessionId;
    (void)networkId;
    (void)result;
}

void DmAuthManager::CloseAuthSession(const int32_t sessionId)
{
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(WAIT_SESSION_CLOSE_TIMEOUT_TASK) + std::string(CLOSE_SESSION_TASK_SEPARATOR) +
            std::to_string(sessionId));
    }
    CHECK_NULL_VOID(softbusConnector_);
    softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
}

void DmAuthManager::RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback)
{
    cleanNotifyCallback_ = cleanNotifyCallback;
    return;
}

void DmAuthManager::GetBindCallerInfo()
{
    LOGI("start.");
    CHECK_NULL_VOID(authRequestContext_);
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        if (bindParam_.find("bindCallerTokenId") != bindParam_.end()) {
            authRequestContext_->tokenId = std::atoi(bindParam_["bindCallerTokenId"].c_str());
        }
        if (bindParam_.find("bindCallerOldBindLevel") != bindParam_.end()) {
            authRequestContext_->bindLevel = std::atoi(bindParam_["bindCallerOldBindLevel"].c_str());
        }
        if (bindParam_.find("bindCallerBundleName") != bindParam_.end()) {
            authRequestContext_->bundleName = bindParam_["bindCallerBundleName"];
        }
        if (bindParam_.find("bindCallerHostPkgLabel") != bindParam_.end()) {
            authRequestContext_->hostPkgLabel = bindParam_["bindCallerHostPkgLabel"];
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
