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

#include <mutex>
#include <string>
#include <unistd.h>

#include "bundle_mgr_interface.h"
#include "iservice_registry.h"
#if defined(SUPPORT_SCREENLOCK)
#include "screenlock_manager.h"
#endif
#include "system_ability_definition.h"

#include "auth_message_processor.h"
#include "common_event_support.h"
#include "dm_ability_manager.h"
#include "dm_anonymous.h"
#include "dm_config_manager.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_dialog_manager.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "dm_random.h"
#include "multiple_user_connector.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "show_confirm.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t AUTHENTICATE_TIMEOUT = 120;
const int32_t CONFIRM_TIMEOUT = 60;
const int32_t NEGOTIATE_TIMEOUT = 10;
const int32_t INPUT_TIMEOUT = 60;
const int32_t ADD_TIMEOUT = 10;
const int32_t WAIT_NEGOTIATE_TIMEOUT = 10;
const int32_t WAIT_REQUEST_TIMEOUT = 10;
const int32_t CLONE_AUTHENTICATE_TIMEOUT = 20;
const int32_t CLONE_CONFIRM_TIMEOUT = 10;
const int32_t CLONE_NEGOTIATE_TIMEOUT = 10;
const int32_t CLONE_ADD_TIMEOUT = 10;
const int32_t CLONE_WAIT_NEGOTIATE_TIMEOUT = 10;
const int32_t CLONE_WAIT_REQUEST_TIMEOUT = 10;
const int32_t CLONE_SESSION_HEARTBEAT_TIMEOUT = 20;
const int32_t CANCEL_PIN_CODE_DISPLAY = 1;
const int32_t DEVICE_ID_HALF = 2;
const int32_t MAX_AUTH_TIMES = 3;
const int32_t MIN_PIN_TOKEN = 10000000;
const int32_t MAX_PIN_TOKEN = 90000000;
const int32_t MIN_PIN_CODE = 100000;
const int32_t MAX_PIN_CODE = 999999;
const int32_t DM_AUTH_TYPE_MAX = 5;
const int32_t DM_AUTH_TYPE_MIN = 0;
const int32_t AUTH_SESSION_SIDE_SERVER = 0;
const int32_t USLEEP_TIME_MS = 500000; // 500ms
const int32_t SYNC_DELETE_TIMEOUT = 60;
const int32_t AUTH_DEVICE_TIMEOUT = 10;
const int32_t SESSION_HEARTBEAT_TIMEOUT = 50;
const int32_t ALREADY_BIND = 1;

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

constexpr const char* APP_OPERATION_KEY = "appOperation";
constexpr const char* TARGET_PKG_NAME_KEY = "targetPkgName";
constexpr const char* CUSTOM_DESCRIPTION_KEY = "customDescription";
constexpr const char* CANCEL_DISPLAY_KEY = "cancelPinCodeDisplay";
constexpr const char* DM_VERSION_4_1_5_1 = "4.1.5.1";
constexpr const char* DM_VERSION_5_0_1 = "5.0.1";
constexpr const char* DM_VERSION_5_0_2 = "5.0.2";
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
    dmVersion_ = DM_VERSION_5_0_2;
}

DmAuthManager::~DmAuthManager()
{
    LOGI("DmAuthManager destructor");
}

int32_t DmAuthManager::CheckAuthParamVaild(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("DmAuthManager::CheckAuthParamVaild start.");
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
        listener_->OnAuthResult(pkgName, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT,
            ERR_DM_UNSUPPORTED_AUTH_TYPE);
        listener_->OnBindResult(pkgName, peerTargetId_, ERR_DM_UNSUPPORTED_AUTH_TYPE, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_UNSUPPORTED_AUTH_TYPE;
    }

    if (authRequestState_ != nullptr || authResponseState_ != nullptr) {
        LOGE("DmAuthManager::CheckAuthParamVaild %{public}s is request authentication.", pkgName.c_str());
        return ERR_DM_AUTH_BUSINESS_BUSY;
    }

    if (!softbusConnector_->HaveDeviceInMap(deviceId)) {
        LOGE("CheckAuthParamVaild failed, the discoveryDeviceInfoMap_ not have this device.");
        listener_->OnAuthResult(pkgName, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT, ERR_DM_INPUT_PARA_INVALID);
        listener_->OnBindResult(pkgName, peerTargetId_, ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if ((authType == AUTH_TYPE_IMPORT_AUTH_CODE) && (!IsAuthCodeReady(pkgName))) {
        LOGE("Auth code not exist.");
        listener_->OnAuthResult(pkgName, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT, ERR_DM_INPUT_PARA_INVALID);
        listener_->OnBindResult(pkgName, peerTargetId_, ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}

void DmAuthManager::GetAuthParam(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("Get auth param.");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    authRequestContext_->hostPkgName = pkgName;
    authRequestContext_->hostPkgLabel = GetBundleLable(pkgName);
    authRequestContext_->authType = authType;
    authRequestContext_->localDeviceName = softbusConnector_->GetLocalDeviceName();
    authRequestContext_->localDeviceTypeId = softbusConnector_->GetLocalDeviceTypeId();
    authRequestContext_->localDeviceId = localUdid;
    authRequestContext_->deviceId = deviceId;
    authRequestContext_->ip = deviceId;
    authRequestContext_->dmVersion = DM_VERSION_5_0_2;
    authRequestContext_->localAccountId = MultipleUserConnector::GetOhosAccountId();
    MultipleUserConnector::SetSwitchOldAccountId(authRequestContext_->localAccountId);
    authRequestContext_->localUserId = MultipleUserConnector::GetCurrentAccountUserID();
    MultipleUserConnector::SetSwitchOldUserId(authRequestContext_->localUserId);
    authRequestContext_->isOnline = false;
    authRequestContext_->authed = !authRequestContext_->bindType.empty();
    authRequestContext_->bindLevel = INVALIED_TYPE;
    nlohmann::json jsonObject = nlohmann::json::parse(extra, nullptr, false);
    if (!jsonObject.is_discarded()) {
        if (IsString(jsonObject, TARGET_PKG_NAME_KEY)) {
            authRequestContext_->targetPkgName = jsonObject[TARGET_PKG_NAME_KEY].get<std::string>();
        }
        if (IsString(jsonObject, APP_OPERATION_KEY)) {
            authRequestContext_->appOperation = jsonObject[APP_OPERATION_KEY].get<std::string>();
        }
        if (IsString(jsonObject, CUSTOM_DESCRIPTION_KEY)) {
            authRequestContext_->customDesc = jsonObject[CUSTOM_DESCRIPTION_KEY].get<std::string>();
        }
        if (IsString(jsonObject, APP_THUMBNAIL)) {
            authRequestContext_->appThumbnail = jsonObject[APP_THUMBNAIL].get<std::string>();
        }
        if (IsInt64(jsonObject, TAG_TOKENID)) {
            authRequestContext_->tokenId = jsonObject[TAG_TOKENID].get<int64_t>();
        }
        if (IsInt32(jsonObject, TAG_BIND_LEVEL)) {
            authRequestContext_->bindLevel = jsonObject[TAG_BIND_LEVEL].get<int32_t>();
        }
        authRequestContext_->closeSessionDelaySeconds = 0;
        if (IsString(jsonObject, PARAM_CLOSE_SESSION_DELAY_SECONDS)) {
            std::string delaySecondsStr = jsonObject[PARAM_CLOSE_SESSION_DELAY_SECONDS].get<std::string>();
            authRequestContext_->closeSessionDelaySeconds = GetCloseSessionDelaySeconds(delaySecondsStr);
        }
    }
    authRequestContext_->token = std::to_string(GenRandInt(MIN_PIN_TOKEN, MAX_PIN_TOKEN));
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
    authPtr_ = authenticationMap_[authType];
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
    GetAuthParam(pkgName, authType, deviceId, extra);
    authMessageProcessor_->SetRequestContext(authRequestContext_);
    authRequestState_ = std::make_shared<AuthRequestInitState>();
    authRequestState_->SetAuthManager(shared_from_this());
    authRequestState_->SetAuthContext(authRequestContext_);
    if (!DmRadarHelper::GetInstance().ReportAuthStart(peerTargetId_.deviceId, pkgName)) {
        LOGE("ReportAuthStart failed");
    }
    authRequestState_->Enter();
    LOGI("DmAuthManager::AuthenticateDevice complete");
}

int32_t DmAuthManager::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("DmAuthManager::AuthenticateDevice start auth type %{public}d.", authType);
    SetAuthType(authType);
    int32_t ret = CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    if (ret != DM_OK) {
        LOGE("DmAuthManager::AuthenticateDevice failed, param is invaild.");
        return ret;
    }
    isAuthenticateDevice_ = true;
    if (authType == AUTH_TYPE_CRE) {
        LOGI("DmAuthManager::AuthenticateDevice for credential type, joinLNN directly.");
        softbusConnector_->JoinLnn(deviceId);
        listener_->OnAuthResult(pkgName, peerTargetId_.deviceId, "", STATUS_DM_AUTH_DEFAULT, DM_OK);
        listener_->OnBindResult(pkgName, peerTargetId_, DM_OK, STATUS_DM_AUTH_DEFAULT, "");
        return DM_OK;
    }
    InitAuthState(pkgName, authType, deviceId, extra);
    return DM_OK;
}

int32_t DmAuthManager::UnAuthenticateDevice(const std::string &pkgName, const std::string &networkId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::string deviceUdid = "";
    int32_t ret = SoftbusConnector::GetUdidByNetworkId(networkId.c_str(), deviceUdid);
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice GetNodeKeyInfo failed");
        return ret;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    struct RadarInfo info = {
        .funcName = "UnAuthenticateDevice",
        .toCallPkg = HICHAINNAME,
        .hostName = pkgName,
        .peerUdid = deviceUdid,
    };
    if (!DmRadarHelper::GetInstance().ReportDeleteTrustRelation(info)) {
        LOGE("ReportDeleteTrustRelation failed");
    }
    if (!DeviceProfileConnector::GetInstance().CheckPkgnameInAcl(pkgName, localDeviceId, deviceUdid)) {
        LOGE("The pkgName %{public}s cannot unbind.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    remoteDeviceId_ = deviceUdid;
    SyncDeleteAcl(pkgName, deviceUdid);
    return DM_OK;
}

int32_t DmAuthManager::StopAuthenticateDevice(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    if (((authRequestState_!= nullptr && authRequestContext_->hostPkgName == pkgName) ||
        (authResponseContext_ != nullptr && authResponseContext_->hostPkgName == pkgName)) &&
        isAuthenticateDevice_) {
        LOGI("Stop previous AuthenticateDevice.");
        authRequestContext_->reason = STOP_BIND;
        authResponseContext_->state = authRequestState_->GetStateType();
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    }
    return DM_OK;
}

int32_t DmAuthManager::UnBindDevice(const std::string &pkgName, const std::string &udidHash)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    remoteDeviceId_ = SoftbusConnector::GetDeviceUdidByUdidHash(udidHash);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    if (!DeviceProfileConnector::GetInstance().CheckPkgnameInAcl(pkgName, localDeviceId, remoteDeviceId_)) {
        LOGE("The pkgname %{public}s cannot unbind.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    SyncDeleteAcl(pkgName, remoteDeviceId_);
    return DM_OK;
}

void DmAuthManager::SyncDeleteAcl(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("SyncDeleteAcl start.");
    authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(shared_from_this());
    authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authRequestContext_ = std::make_shared<DmAuthRequestContext>();

    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    authRequestContext_->localDeviceId = localUdid;
    authRequestContext_->hostPkgName = pkgName;
    authRequestContext_->deviceId = deviceId;
    authMessageProcessor_->SetRequestContext(authRequestContext_);
    authRequestState_ = std::make_shared<AuthRequestDeleteInit>();
    authRequestState_->SetAuthManager(shared_from_this());
    authRequestState_->SetAuthContext(authRequestContext_);
    authRequestState_->Enter();
}

void DmAuthManager::GetPeerUdidHash(int32_t sessionId, std::string &peerUdidHash)
{
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
    LOGI("DmAuthManager::OnSessionOpened, sessionId = %{public}d and sessionSide = %{public}d result = %{public}d",
         sessionId, sessionSide, result);
    DeleteOffLineTimer(sessionId);
    if (sessionSide == AUTH_SESSION_SIDE_SERVER) {
        if (authResponseState_ == nullptr && authRequestState_ == nullptr) {
            authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(shared_from_this());
            authResponseState_ = std::make_shared<AuthResponseInitState>();
            authResponseState_->SetAuthManager(shared_from_this());
            authResponseState_->Enter();
            authResponseContext_ = std::make_shared<DmAuthResponseContext>();
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
        }
    } else {
        if (authResponseState_ == nullptr && authRequestState_ != nullptr &&
            authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_INIT) {
            authRequestContext_->sessionId = sessionId;
            authMessageProcessor_->SetRequestContext(authRequestContext_);
            authRequestState_->SetAuthContext(authRequestContext_);
            authRequestState_->TransitionTo(std::make_shared<AuthRequestNegotiateState>());
            struct RadarInfo info = { .funcName = "OnSessionOpened" };
            info.channelId = sessionId;
            if (!DmRadarHelper::GetInstance().ReportAuthSendRequest(info)) {
                LOGE("ReportAuthSendRequest failed");
            }
        } else {
            softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
            LOGE("DmAuthManager::OnSessionOpened but request state is wrong");
        }
    }
}

void DmAuthManager::OnSessionClosed(const int32_t sessionId)
{
    LOGI("DmAuthManager::OnSessionClosed sessionId = %{public}d", sessionId);
}

void DmAuthManager::ProcessSourceMsg()
{
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
        case MSG_TYPE_REQ_SYNC_DELETE_DONE:
            if (authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_SYNCDELETE) {
                if (timer_ != nullptr) {
                    timer_->DeleteTimer(std::string(SYNC_DELETE_TIMEOUT_TASK));
                }
                isFinishOfLocal_ = false;
                authRequestState_->TransitionTo(std::make_shared<AuthRequestSyncDeleteAclNone>());
            }
            break;
        default:
            break;
    }
}

void DmAuthManager::ProcessSinkMsg()
{
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
            if (authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_FINISH) {
                isFinishOfLocal_ = false;
                authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
            }
            break;
        case MSG_TYPE_REQ_PUBLICKEY:
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_SHOW) {
                authResponseState_->TransitionTo(std::make_shared<AuthResponseCredential>());
            }
            break;
        case MSG_TYPE_REQ_SYNC_DELETE:
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_INIT) {
                authResponseState_->TransitionTo(std::make_shared<AuthResponseSyncDeleteAcl>());
            }
            break;
        case MSG_TYPE_REQ_SYNC_DELETE_DONE:
            if (authResponseState_->GetStateType() == AuthState::AUTH_REQUEST_SYNCDELETE) {
                if (timer_ != nullptr) {
                    timer_->DeleteTimer(std::string(SYNC_DELETE_TIMEOUT_TASK));
                }
                isFinishOfLocal_ = false;
                authResponseState_->TransitionTo(std::make_shared<AuthResponseSyncDeleteAclNone>());
            }
            break;
        default:
            break;
    }
}

void DmAuthManager::OnDataReceived(const int32_t sessionId, const std::string message)
{
    if (authResponseContext_ == nullptr || authMessageProcessor_ == nullptr) {
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
    if (groupId == "{}") {
        authResponseContext_->reply = ERR_DM_CREATE_GROUP_FAILED;
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        return;
    }

    int32_t pinCode = -1;
    if (authResponseContext_->isShowDialog) {
        pinCode = GeneratePincode();
    } else {
        GetAuthCode(authResponseContext_->hostPkgName, pinCode);
    }
    nlohmann::json jsonObj;
    jsonObj[PIN_TOKEN] = authResponseContext_->token;
    jsonObj[QR_CODE_KEY] = GenerateGroupName();
    jsonObj[NFC_CODE_KEY] = GenerateGroupName();
    authResponseContext_->authToken = jsonObj.dump();
    LOGI("DmAuthManager::OnGroupCreated start group id %{public}s", GetAnonyString(groupId).c_str());
    authResponseContext_->groupId = groupId;
    authResponseContext_->code = pinCode;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    authResponseContext_->isFinish = true;
    authResponseState_->TransitionTo(std::make_shared<AuthResponseShowState>());
}

void DmAuthManager::OnMemberJoin(int64_t requestId, int32_t status)
{
    isAddingMember_ = false;
    if (authResponseContext_ == nullptr || authUiStateMgr_ == nullptr) {
        LOGE("failed to OnMemberJoin because authResponseContext_ or authUiStateMgr is nullptr");
        return;
    }
    LOGI("DmAuthManager OnMemberJoin start authTimes %{public}d", authTimes_);
    if ((authRequestState_ != nullptr) && (authResponseState_ == nullptr)) {
        authTimes_++;
        timer_->DeleteTimer(std::string(ADD_TIMEOUT_TASK));
        if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
            HandleMemberJoinImportAuthCode(requestId, status);
            return;
        }
        CHECK_NULL_VOID(timer_);
        if (status != DM_OK || authResponseContext_->requestId != requestId) {
            if (authRequestState_ != nullptr && authTimes_ >= MAX_AUTH_TIMES) {
                authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
                authRequestContext_->reason = ERR_DM_BIND_PIN_CODE_ERROR;
                authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            } else {
                timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK),
                    GetTaskTimeout(INPUT_TIMEOUT_TASK, INPUT_TIMEOUT), [this] (std::string name) {
                        DmAuthManager::HandleAuthenticateTimeout(name);
                    });
                authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_PIN_CODE_ERROR);
            }
        } else {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestNetworkState>());
            timer_->DeleteTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK));
        }
    } else if ((authResponseState_ != nullptr) && (authRequestState_ == nullptr)) {
        if (status == DM_OK && authResponseContext_->requestId == requestId &&
            authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_SHOW) {
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
        } else {
            if (++authTimes_ >= MAX_AUTH_TIMES) {
                authResponseContext_->isFinish = false;
            }
        }
    } else {
        LOGE("DmAuthManager::OnMemberJoin failed, authRequestState_ or authResponseState_ is invalid.");
    }
}

void DmAuthManager::HandleMemberJoinImportAuthCode(const int64_t requestId, const int32_t status)
{
    if (status != DM_OK || authResponseContext_->requestId != requestId) {
        authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
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
        authRequestContext_->reason = ERR_DM_TIME_OUT;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    }

    if (authResponseState_ != nullptr && authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_FINISH) {
        authResponseContext_->state = authResponseState_->GetStateType();
        authResponseContext_->reply = ERR_DM_TIME_OUT;
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
    }
    LOGI("DmAuthManager::HandleAuthenticateTimeout start complete");
}

int32_t DmAuthManager::EstablishAuthChannel(const std::string &deviceId)
{
    int32_t sessionId = softbusConnector_->GetSoftbusSession()->OpenAuthSession(deviceId);
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
    authResponseContext_->localDeviceId = authRequestContext_->localDeviceId;
    authResponseContext_->reply = ERR_DM_AUTH_REJECT;
    authResponseContext_->authType = authRequestContext_->authType;
    authResponseContext_->deviceId = authRequestContext_->deviceId;
    authResponseContext_->accountGroupIdHash = GetAccountGroupIdHash();
    authResponseContext_->hostPkgName = authRequestContext_->hostPkgName;
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
    authResponseContext_->edition = DM_VERSION_5_0_2;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_NEGOTIATE);
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
    if (timer_ != nullptr) {
        timer_->StartTimer(std::string(NEGOTIATE_TIMEOUT_TASK),
            GetTaskTimeout(NEGOTIATE_TIMEOUT_TASK, NEGOTIATE_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    }
}

void DmAuthManager::AbilityNegotiate()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    bool ret = hiChainConnector_->IsDevicesInP2PGroup(authResponseContext_->localDeviceId, localDeviceId);
    if (ret) {
        LOGE("DmAuthManager::EstablishAuthChannel device is in group");
        if (!DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(authResponseContext_->hostPkgName,
            authResponseContext_->localDeviceId)) {
            CompatiblePutAcl();
        }
        authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
        if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE && !importAuthCode_.empty()) {
            authResponseContext_->importAuthCode = Crypto::Sha256(importAuthCode_);
        }
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
    remoteDeviceId_ = authResponseContext_->localDeviceId;
    authResponseContext_->networkId = softbusConnector_->GetLocalDeviceNetworkId();
    authResponseContext_->targetDeviceName = softbusConnector_->GetLocalDeviceName();
    remoteVersion_ = ConvertSrcVersion(authResponseContext_->dmVersion, authResponseContext_->edition);
    NegotiateRespMsg(remoteVersion_);
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= DEVICE &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        ProcRespNegotiateExt(sessionId);
        timer_->StartTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK),
            GetTaskTimeout(WAIT_REQUEST_TIMEOUT_TASK, WAIT_REQUEST_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        authResponseContext_->bindLevel == INVALIED_TYPE) {
        ProcRespNegotiate(sessionId);
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
    if (authResponseContext_->reply == ERR_DM_VERSION_INCOMPATIBLE) {
        LOGE("The peer device version is not supported");
        authRequestContext_->reason = authResponseContext_->reply;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return;
    }
    remoteDeviceId_ = authResponseContext_->localDeviceId;
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
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= DEVICE &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        ProcessAuthRequestExt(sessionId);
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        authResponseContext_->bindLevel == INVALIED_TYPE) {
        ProcessAuthRequest(sessionId);
    } else {
        LOGE("Invalied bind mode.");
    }
}

void DmAuthManager::ProcessAuthRequest(const int32_t &sessionId)
{
    LOGI("ProcessAuthRequest start.");
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        !authResponseContext_->importAuthCode.empty() && !importAuthCode_.empty()) {
        if (authResponseContext_->importAuthCode != Crypto::Sha256(importAuthCode_)) {
            SetReasonAndFinish(ERR_DM_AUTH_CODE_INCORRECT, AuthState::AUTH_REQUEST_FINISH);
            return;
        }
    }

    if (authResponseContext_->isOnline && softbusConnector_->CheckIsOnline(remoteDeviceId_)) {
        authResponseContext_->isOnline = true;
    } else {
        authResponseContext_->isOnline = false;
    }
    if (CheckTrustState() != DM_OK) {
        LOGI("CheckTrustState end.");
        return;
    }

    std::vector<std::string> messageList = authMessageProcessor_->CreateAuthRequestMessage();
    for (auto msg : messageList) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, msg);
    }

    listener_->OnAuthResult(authResponseContext_->hostPkgName, peerTargetId_.deviceId,
        authRequestContext_->token, STATUS_DM_SHOW_AUTHORIZE_UI, DM_OK);
    listener_->OnBindResult(authResponseContext_->hostPkgName, peerTargetId_, DM_OK, STATUS_DM_SHOW_AUTHORIZE_UI, "");
    if (timer_ != nullptr) {
        timer_->StartTimer(std::string(CONFIRM_TIMEOUT_TASK),
            GetTaskTimeout(CONFIRM_TIMEOUT_TASK, CONFIRM_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    }
}

void DmAuthManager::GetAuthRequestContext()
{
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
    bool haveCredential = hiChainAuthConnector_->QueryCredential(remoteDeviceId_, authRequestContext_->localUserId);
    if (authResponseContext_->haveCredential && haveCredential) {
        authResponseContext_->haveCredential = true;
    } else {
        authResponseContext_->haveCredential = false;
    }
}

void DmAuthManager::ProcessAuthRequestExt(const int32_t &sessionId)
{
    LOGI("ProcessAuthRequestExt start.");
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
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
    if (authResponseContext_->isOnline && authResponseContext_->authed &&
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
    listener_->OnAuthResult(authResponseContext_->hostPkgName, peerTargetId_.deviceId,
        authRequestContext_->token, STATUS_DM_SHOW_AUTHORIZE_UI, DM_OK);
    listener_->OnBindResult(authResponseContext_->hostPkgName, peerTargetId_, DM_OK, STATUS_DM_SHOW_AUTHORIZE_UI, "");
    if (timer_ != nullptr) {
        timer_->StartTimer(std::string(CONFIRM_TIMEOUT_TASK),
            GetTaskTimeout(CONFIRM_TIMEOUT_TASK, CONFIRM_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    }
}

bool DmAuthManager::IsAuthFinish()
{
    if (authResponseContext_->reply == ERR_DM_UNSUPPORTED_AUTH_TYPE) {
        listener_->OnAuthResult(authResponseContext_->hostPkgName, peerTargetId_.deviceId,
            authRequestContext_->token, AuthState::AUTH_REQUEST_NEGOTIATE_DONE, ERR_DM_UNSUPPORTED_AUTH_TYPE);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return true;
    }

    if (authResponseContext_->isOnline && authResponseContext_->authed) {
        authRequestContext_->reason = DM_OK;
        authResponseContext_->reply = DM_OK;
        authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return true;
    }

    if ((authResponseContext_->isIdenticalAccount && !authResponseContext_->authed) ||
        (authResponseContext_->authed && !authResponseContext_->isOnline)) {
        softbusConnector_->JoinLnn(authRequestContext_->deviceId);
        authRequestContext_->reason = DM_OK;
        authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
        authResponseContext_->reply = DM_OK;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return true;
    }

    if (authResponseContext_->reply == ERR_DM_UNSUPPORTED_AUTH_TYPE ||
        (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        authResponseContext_->isAuthCodeReady == false)) {
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return true;
    }
    return false;
}

int32_t DmAuthManager::ConfirmProcess(const int32_t &action)
{
    LOGI("ConfirmProcess start.");
    if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH || action_ == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        authResponseContext_->reply = USER_OPERATION_TYPE_ALLOW_AUTH;
    } else {
        authResponseContext_->reply = action_;
    }

    if (authResponseContext_->reply == USER_OPERATION_TYPE_ALLOW_AUTH &&
        authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_CONFIRM) {
        authResponseState_->TransitionTo(std::make_shared<AuthResponseGroupState>());
    } else {
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    }
    return DM_OK;
}

int32_t DmAuthManager::ConfirmProcessExt(const int32_t &action)
{
    LOGI("ConfirmProcessExt start.");
    authResponseContext_->confirmOperation = action;
    if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH || action_ == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        authResponseContext_->reply = USER_OPERATION_TYPE_ALLOW_AUTH;
    } else {
        authResponseContext_->reply = USER_OPERATION_TYPE_CANCEL_AUTH;
    }
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    if (authResponseContext_->reply == USER_OPERATION_TYPE_ALLOW_AUTH &&
        authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_CONFIRM) {
        if (!authResponseContext_->isShowDialog) {
            GetAuthCode(authResponseContext_->hostPkgName, authResponseContext_->code);
        } else {
            authResponseContext_->code = GeneratePincode();
        }
        authResponseContext_->requestId = GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
        authResponseState_->TransitionTo(std::make_shared<AuthResponseShowState>());
    }
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH_EXT);
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
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= DEVICE &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        return ConfirmProcessExt(action);
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        authResponseContext_->bindLevel == INVALIED_TYPE) {
        return ConfirmProcess(action);
    } else {
        LOGE("Invalied bind mode.");
    }
    return DM_OK;
}

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
        listener_->OnAuthResult(authRequestContext_->hostPkgName, peerTargetId_.deviceId,
            authRequestContext_->token, STATUS_DM_SHOW_PIN_INPUT_UI, DM_OK);
        listener_->OnBindResult(authRequestContext_->hostPkgName, peerTargetId_, DM_OK,
            STATUS_DM_SHOW_PIN_INPUT_UI, "");
        authRequestState_->TransitionTo(std::make_shared<AuthRequestJoinState>());
    } else {
        LOGE("do not accept");
        authResponseContext_->state = AuthState::AUTH_REQUEST_REPLY;
        authRequestContext_->reason = ERR_DM_AUTH_PEER_REJECT;
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
    authResponseContext_->groupName = GenerateGroupName();
    authResponseContext_->requestId = GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
    hiChainConnector_->CreateGroup(authResponseContext_->requestId, authResponseContext_->groupName);
    return DM_OK;
}

int32_t DmAuthManager::AddMember(int32_t pinCode)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to AddMember because authResponseContext_ is nullptr");
        return ERR_DM_FAILED;
    }
    LOGI("DmAuthManager::AddMember start group id %{public}s", GetAnonyString(authResponseContext_->groupId).c_str());
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(INPUT_TIMEOUT_TASK));
    }
    nlohmann::json jsonObject;
    jsonObject[TAG_GROUP_ID] = authResponseContext_->groupId;
    jsonObject[TAG_GROUP_NAME] = authResponseContext_->groupName;
    jsonObject[PIN_CODE_KEY] = pinCode;
    jsonObject[TAG_REQUEST_ID] = authResponseContext_->requestId;
    jsonObject[TAG_DEVICE_ID] = authResponseContext_->deviceId;
    std::string connectInfo = jsonObject.dump();
    if (timer_ != nullptr) {
        timer_->StartTimer(std::string(ADD_TIMEOUT_TASK),
            GetTaskTimeout(ADD_TIMEOUT_TASK, ADD_TIMEOUT), [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
    }
    if (authUiStateMgr_ == nullptr) {
        LOGE("DmAuthManager::AddMember authUiStateMgr is null.");
        return ERR_DM_FAILED;
    }
    if (isAddingMember_) {
        LOGE("DmAuthManager::AddMember doing add member.");
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_DOING_AUTH);
        return ERR_DM_FAILED;
    }
    isAddingMember_ = true;
    int32_t ret = hiChainConnector_->AddMember(authRequestContext_->ip, connectInfo);
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
    authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
    authResponseContext_->isFinish = true;
    authRequestContext_->reason = DM_OK;
    authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    return DM_OK;
}

void DmAuthManager::SinkAuthenticateFinish()
{
    LOGI("DmAuthManager::SinkAuthenticateFinish, isFinishOfLocal: %{public}d", isFinishOfLocal_);
    if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_FINISH && authPtr_ != nullptr) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_CONFIRM_SHOW);
    }
    if (isFinishOfLocal_) {
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_AUTH_TERMINATE);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    }
    authResponseState_ = nullptr;
}

void DmAuthManager::SrcAuthenticateFinish()
{
    LOGI("DmAuthManager::SrcAuthenticateFinish, isFinishOfLocal: %{public}d", isFinishOfLocal_);
    if (isFinishOfLocal_) {
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_AUTH_TERMINATE);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    } else {
        authRequestContext_->reason = authResponseContext_->reply;
    }
    if ((authResponseContext_->state == AuthState::AUTH_REQUEST_JOIN ||
        authResponseContext_->state == AuthState::AUTH_REQUEST_FINISH) && authPtr_ != nullptr) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
    }
    usleep(USLEEP_TIME_MS); // 500ms

    int32_t sessionId = authRequestContext_->sessionId;
    auto taskFunc = [this, sessionId]() {
        CHECK_NULL_VOID(softbusConnector_);
        CHECK_NULL_VOID(softbusConnector_->GetSoftbusSession());
        softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
    };
    const int64_t MICROSECOND_PER_SECOND = 1000000L;
    int32_t delaySeconds = authRequestContext_->closeSessionDelaySeconds;
    ffrt::submit(taskFunc, ffrt::task_attr().delay(delaySeconds * MICROSECOND_PER_SECOND));

    listener_->OnAuthResult(authRequestContext_->hostPkgName, peerTargetId_.deviceId,
        authRequestContext_->token, authResponseContext_->state, authRequestContext_->reason);
    listener_->OnBindResult(authRequestContext_->hostPkgName, peerTargetId_, authRequestContext_->reason,
        authResponseContext_->state, GenerateBindResultContent());

    authRequestContext_ = nullptr;
    authRequestState_ = nullptr;
    authTimes_ = 0;
}

void DmAuthManager::AuthenticateFinish()
{
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
    if (authResponseContext_->isFinish) {
        CompatiblePutAcl();
    }
    if (DeviceProfileConnector::GetInstance().GetTrustNumber(remoteDeviceId_) >= 1 &&
        CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        authResponseContext_->bindLevel == INVALIED_TYPE && softbusConnector_->CheckIsOnline(remoteDeviceId_) &&
        authResponseContext_->isFinish) {
        softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
    }

    DeleteAuthCode();
    if (authResponseState_ != nullptr) {
        SinkAuthenticateFinish();
    } else if (authRequestState_ != nullptr) {
        SrcAuthenticateFinish();
    }
    if (timer_ != nullptr) {
        timer_->DeleteAll();
    }
    isFinishOfLocal_ = true;
    authResponseContext_ = nullptr;
    authMessageProcessor_ = nullptr;
    authPtr_ = nullptr;
    authenticationType_ = USER_OPERATION_TYPE_ALLOW_AUTH;
    LOGI("DmAuthManager::AuthenticateFinish complete");
}

void DmAuthManager::CancelDisplay()
{
    LOGI("DmAuthManager::CancelDisplay start");
    nlohmann::json jsonObj;
    jsonObj[CANCEL_DISPLAY_KEY] = CANCEL_PIN_CODE_DISPLAY;
    std::string paramJson = jsonObj.dump();
    std::string pkgName = "com.ohos.devicemanagerui";
    listener_->OnUiCall(pkgName, paramJson);
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

int32_t DmAuthManager::GeneratePincode()
{
    return GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
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

int32_t DmAuthManager::SetAuthRequestState(std::shared_ptr<AuthRequestState> authRequestState)
{
    if (authRequestState == nullptr) {
        LOGE("authRequestState is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
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

int32_t DmAuthManager::GetPinCode(int32_t &code)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to GetPinCode because authResponseContext_ is nullptr");
        code = ERR_DM_AUTH_NOT_START;
        return ERR_DM_FAILED;
    }
    LOGI("ShowConfigDialog start add member pin code.");
    code = authResponseContext_->code;
    return DM_OK;
}

void DmAuthManager::ShowConfigDialog()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowConfigDialog because authResponseContext_ is nullptr");
        return;
    }
    if (!authResponseContext_->isShowDialog) {
        LOGI("start auth process");
        StartAuthProcess(authenticationType_);
        return;
    }
    LOGI("ShowConfigDialog start");
    nlohmann::json jsonObj;
    jsonObj[TAG_AUTH_TYPE] = AUTH_TYPE_PIN;
    jsonObj[TAG_TOKEN] = authResponseContext_->token;
    jsonObj[TARGET_PKG_NAME_KEY] = authResponseContext_->targetPkgName;
    jsonObj[TAG_CUSTOM_DESCRIPTION] = authResponseContext_->customDesc;
    jsonObj[TAG_APP_OPERATION] = authResponseContext_->appOperation;
    jsonObj[TAG_LOCAL_DEVICE_TYPE] = authResponseContext_->deviceTypeId;
    jsonObj[TAG_REQUESTER] = authResponseContext_->deviceName;
    jsonObj[TAG_HOST_PKGLABEL] = authResponseContext_->hostPkgLabel;
    const std::string params = jsonObj.dump();
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    DeviceProfileConnector::GetInstance().SyncAclByBindType(authResponseContext_->hostPkgName,
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

void DmAuthManager::ShowAuthInfoDialog()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowAuthInfoDialog because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::ShowAuthInfoDialog start");
    if (!authResponseContext_->isShowDialog) {
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
    nlohmann::json jsonObj;
    jsonObj[PIN_CODE_KEY] = authResponseContext_->code;
    std::string authParam = jsonObj.dump();
    DmDialogManager::GetInstance().ShowPinDialog(std::to_string(authResponseContext_->code));
}

void DmAuthManager::ShowStartAuthDialog()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowStartAuthDialog because authResponseContext_ is nullptr");
        return;
    }
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        LOGI("Add member start");
        int32_t pinCode = -1;
        if (GetAuthCode(authResponseContext_->hostPkgName, pinCode) != DM_OK) {
            LOGE("failed to get auth code");
            return;
        }
        if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
            (static_cast<uint32_t>(authResponseContext_->bindLevel) >= DEVICE &&
            static_cast<uint32_t>(authResponseContext_->bindLevel)  <= APP)) {
            AuthDevice(pinCode);
        } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
            authResponseContext_->bindLevel == INVALIED_TYPE) {
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
    DmDialogManager::GetInstance().ShowInputDialog(authResponseContext_->targetDeviceName);
}

int32_t DmAuthManager::ProcessPincode(int32_t pinCode)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ProcessPincode because authResponseContext_ is nullptr");
        return ERR_DM_FAILED;
    }
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(INPUT_TIMEOUT_TASK));
    }
    if (CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) &&
        (static_cast<uint32_t>(authResponseContext_->bindLevel) >= DEVICE &&
        static_cast<uint32_t>(authResponseContext_->bindLevel) <= APP)) {
        return AuthDevice(pinCode);
    } else if (!CompareVersion(remoteVersion_, std::string(DM_VERSION_4_1_5_1)) ||
        authResponseContext_->bindLevel == INVALIED_TYPE) {
        return AddMember(pinCode);
    } else {
        LOGE("Invalied bind mode.");
    }
    return ERR_DM_FAILED;
}

int32_t DmAuthManager::AuthDevice(int32_t pinCode)
{
    LOGI("DmAuthManager::AuthDevice start.");
    if (isAuthDevice_) {
        LOGE("DmAuthManager::AuthDevice doing auth device.");
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
    switch (action) {
        case USER_OPERATION_TYPE_ALLOW_AUTH:
        case USER_OPERATION_TYPE_CANCEL_AUTH:
        case USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS:
            StartAuthProcess(action);
            break;
        case USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT:
            SetReasonAndFinish(ERR_DM_TIME_OUT, STATUS_DM_AUTH_DEFAULT);
            info.errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_TIME_OUT);
            if (!DmRadarHelper::GetInstance().ReportAuthConfirmBox(info)) {
                LOGE("ReportAuthConfirmBox failed");
            }
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY:
            SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY, STATUS_DM_AUTH_DEFAULT);
            info.errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY);
            if (!DmRadarHelper::GetInstance().ReportAuthInputPinBox(info)) {
                LOGE("ReportAuthInputPinBox failed");
            }
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT:
            SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL_ERROR, STATUS_DM_AUTH_DEFAULT);
            info.errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_BIND_USER_CANCEL_ERROR);
            if (!DmRadarHelper::GetInstance().ReportAuthInputPinBox(info)) {
                LOGE("ReportAuthInputPinBox failed");
            }
            break;
        case USER_OPERATION_TYPE_DONE_PINCODE_INPUT:
            ProcessPincode(std::atoi(params.c_str()));
            info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
            if (!DmRadarHelper::GetInstance().ReportAuthInputPinBox(info)) {
                LOGE("ReportAuthInputPinBox failed");
            }
            break;
        default:
            LOGE("this action id not support");
            break;
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
        authRequestContext_->reason = reason;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    } else if (authResponseState_ != nullptr && authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_FINISH) {
        authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
    }
    return DM_OK;
}

bool DmAuthManager::IsIdenticalAccount()
{
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
    std::string queryParams = jsonObj.dump();

    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return false;
    }
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
    nlohmann::json jsonPeerGroupIdObj = nlohmann::json::parse(authResponseContext_->accountGroupIdHash,
        nullptr, false);
    if (jsonPeerGroupIdObj.is_discarded()) {
        LOGE("accountGroupIdHash string not a json type.");
        return false;
    }
    for (auto &groupInfo : groupList) {
        for (nlohmann::json::iterator it = jsonPeerGroupIdObj.begin(); it != jsonPeerGroupIdObj.end(); ++it) {
            if ((*it) == Crypto::GetGroupIdHash(groupInfo.groupId)) {
                LOGI("Is identical Account.");
                return true;
            }
        }
    }
    return false;
}

std::string DmAuthManager::GetAccountGroupIdHash()
{
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
    std::string queryParams = jsonObj.dump();

    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return "";
    }
    std::vector<GroupInfo> groupList;
    if (!hiChainConnector_->GetGroupInfo(osAccountUserId, queryParams, groupList)) {
        return "";
    }
    nlohmann::json jsonAccountObj;
    for (auto &groupInfo : groupList) {
        jsonAccountObj.push_back(Crypto::GetGroupIdHash(groupInfo.groupId));
    }
    return jsonAccountObj.dump();
}

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
    const std::map<std::string, std::string> &bindParam)
{
    struct RadarInfo info = {
        .funcName = "AuthenticateDevice",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverUserRes(info)) {
        LOGE("ReportDiscoverUserRes failed");
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
    if (softbusConnector_->AddMemberToDiscoverMap(deviceId, deviceInfo) != DM_OK) {
        LOGE("DmAuthManager::ParseConnectAddr failed, AddMemberToDiscoverMap failed.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    deviceInfo = nullptr;
    return DM_OK;
}

int32_t DmAuthManager::ParseAuthType(const std::map<std::string, std::string> &bindParam, int32_t &authType)
{
    auto iter = bindParam.find(PARAM_KEY_AUTH_TYPE);
    if (iter == bindParam.end()) {
        LOGE("DmAuthManager::ParseAuthType bind param key: %{public}s not exist.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string authTypeStr = iter->second;
    if (authTypeStr.empty()) {
        LOGE("DmAuthManager::ParseAuthType bind param %{public}s is empty.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authTypeStr.length() > 1) {
        LOGE("DmAuthManager::ParseAuthType bind param %{public}s length is unsupported.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!isdigit(authTypeStr[0])) {
        LOGE("DmAuthManager::ParseAuthType bind param %{public}s fromat is unsupported.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    authType = std::atoi(authTypeStr.c_str());
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
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        importAuthCode_ = "";
        importPkgName_ = "";
    }
    return DM_OK;
}

int32_t DmAuthManager::GetAuthCode(const std::string &pkgName, int32_t &pinCode)
{
    if (importAuthCode_.empty() || importPkgName_.empty()) {
        LOGE("GetAuthCode failed, auth code not exist.");
        return ERR_DM_FAILED;
    }
    if (pkgName != importPkgName_) {
        LOGE("GetAuthCode failed, pkgName not supported.");
        return ERR_DM_FAILED;
    }
    pinCode = std::atoi(importAuthCode_.c_str());
    return DM_OK;
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
    nlohmann::json jsonObj;
    jsonObj[DM_BIND_RESULT_NETWORK_ID] = authResponseContext_->networkId;
    if (remoteDeviceId_.empty()) {
        jsonObj[TAG_DEVICE_ID] = "";
    } else {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        Crypto::GetUdidHash(remoteDeviceId_, reinterpret_cast<uint8_t *>(deviceIdHash));
        jsonObj[TAG_DEVICE_ID] = deviceIdHash;
    }
    std::string content = jsonObj.dump();
    return content;
}

void DmAuthManager::RequestCredential()
{
    LOGI("DmAuthManager::RequestCredential start.");
    std::string publicKey = "";
    GenerateCredential(publicKey);
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
    hiChainAuthConnector_->GenerateCredential(localUdid, osAccountId, publicKey);
    if (publicKey == "") {
        hiChainAuthConnector_->GetCredential(localUdid, osAccountId, publicKey);
    }
}

void DmAuthManager::RequestCredentialDone()
{
    LOGI("DmAuthManager ExchangeCredentailDone start");
    if (authResponseContext_ == nullptr) {
        LOGE("failed to JoinNeWork because authResponseContext_ is nullptr");
        return;
    }
    if (ImportCredential(remoteDeviceId_, authResponseContext_->publicKey) != DM_OK) {
        LOGE("ResponseCredential import credential failed.");
    }
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
    }

    softbusConnector_->JoinLnn(authRequestContext_->ip);
    authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
    authRequestContext_->reason = DM_OK;
    authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
}

int32_t DmAuthManager::ImportCredential(std::string &deviceId, std::string &publicKey)
{
    LOGI("DmAuthManager::ImportCredential");
    int32_t osAccountId = MultipleUserConnector::GetCurrentAccountUserID();
    return hiChainAuthConnector_->ImportCredential(osAccountId, deviceId, publicKey);
}

int32_t DmAuthManager::EstablishUnbindChannel(const std::string &deviceIdHash)
{
    LOGI("DmAuthManager::EstablishUnbindChannel");
    std::string netWorkId = softbusConnector_->GetNetworkIdByDeviceId(deviceIdHash);
    int32_t sessionId = softbusConnector_->GetSoftbusSession()->OpenUnbindSession(netWorkId);
    if (sessionId < 0) {
        LOGE("OpenAuthSession failed, stop the syncdeleteacl.");
        authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        authResponseContext_->state = AuthState::AUTH_REQUEST_SYNCDELETE;
        authResponseContext_->hostPkgName = authRequestContext_->hostPkgName;
        authRequestContext_->reason = sessionId;
        if (authRequestState_ != nullptr) {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestSyncDeleteAclNone>());
        }
    }
    return DM_OK;
}

void DmAuthManager::RequestSyncDeleteAcl()
{
    LOGI("RequestSyncDeleteAcl start.");
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_SYNC_DELETE);
    softbusConnector_->GetSoftbusSession()->SendData(authRequestContext_->sessionId, message);
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(std::string(SYNC_DELETE_TIMEOUT_TASK), SYNC_DELETE_TIMEOUT,
        [this] (std::string name) {
            DmAuthManager::HandleSyncDeleteTimeout(name);
        });
}

void DmAuthManager::SrcSyncDeleteAclDone()
{
    LOGI("DmAuthManager::SrcSyncDeleteAclDone, isFinishOfLocal: %{public}d", isFinishOfLocal_);
    if (isFinishOfLocal_) {
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_SYNC_DELETE_DONE);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        usleep(USLEEP_TIME_MS);
    }
    if (authResponseContext_->reply == DM_OK) {
        char localUdid[DEVICE_UUID_LENGTH] = {0};
        GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
        if (hiChainConnector_->IsDevicesInP2PGroup(remoteDeviceId_, localUdid) &&
            DeviceProfileConnector::GetInstance().CheckDevIdInAclForDevBind(authRequestContext_->hostPkgName,
            remoteDeviceId_)) {
            DeleteGroup(authRequestContext_->hostPkgName, remoteDeviceId_);
        }
        DeleteAcl(authRequestContext_->hostPkgName, remoteDeviceId_);
    }
    softbusConnector_->GetSoftbusSession()->CloseUnbindSession(authRequestContext_->sessionId);
    if (timer_ != nullptr) {
        timer_->DeleteAll();
    }
    isFinishOfLocal_ = true;
    authRequestContext_ = nullptr;
    authResponseContext_ = nullptr;
    authRequestState_ = nullptr;
    authMessageProcessor_ = nullptr;
}

void DmAuthManager::SinkSyncDeleteAclDone()
{
    LOGI("DmAuthManager::SinkSyncDeleteAclDone, isFinishOfLocal: %{public}d", isFinishOfLocal_);
    if (isFinishOfLocal_) {
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_SYNC_DELETE_DONE);
        softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        if (authResponseContext_->reply == DM_OK) {
            char localUdid[DEVICE_UUID_LENGTH] = {0};
            GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
            if (hiChainConnector_->IsDevicesInP2PGroup(remoteDeviceId_, localUdid) &&
                DeviceProfileConnector::GetInstance().CheckDevIdInAclForDevBind(authResponseContext_->hostPkgName,
                remoteDeviceId_)) {
                DeleteGroup(authResponseContext_->hostPkgName, remoteDeviceId_);
            }
            DeleteAcl(authResponseContext_->hostPkgName, remoteDeviceId_);
        }
    }
    if (timer_ != nullptr) {
        timer_->DeleteAll();
    }
    isFinishOfLocal_ = true;
    authResponseContext_ = nullptr;
    authResponseState_ = nullptr;
    authMessageProcessor_ = nullptr;
}

void DmAuthManager::SyncDeleteAclDone()
{
    LOGI("SyncDeleteAclDone start.");
    if (authRequestState_ != nullptr) {
        SrcSyncDeleteAclDone();
    } else if (authResponseState_ != nullptr) {
        SinkSyncDeleteAclDone();
    }
}

void DmAuthManager::ResponseCredential()
{
    LOGI("DmAuthManager::ResponseCredential start.");
    std::string publicKey = "";
    GenerateCredential(publicKey);
    if (ImportCredential(remoteDeviceId_, authResponseContext_->publicKey) != DM_OK) {
        LOGE("ResponseCredential import credential failed.");
    }
    authResponseContext_->publicKey = publicKey;
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_PUBLICKEY);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
}

void DmAuthManager::ResponseSyncDeleteAcl()
{
    LOGI("ResponseSyncDeleteAcl start.");
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(SYNC_DELETE_TIMEOUT_TASK));
    }
    remoteDeviceId_ = authResponseContext_->localDeviceId;
    authResponseState_->TransitionTo(std::make_shared<AuthResponseSyncDeleteAclNone>());
}

bool DmAuthManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("DmAuthManager::onTransmit start.");
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
    if (softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message) != DM_OK) {
        LOGE("SoftbusSession send data failed.");
        return false;
    }
    return true;
}

void DmAuthManager::SrcAuthDeviceFinish()
{
    if (authResponseContext_->isOnline) {
        if (authResponseContext_->confirmOperation == USER_OPERATION_TYPE_ALLOW_AUTH ||
            (authResponseContext_->confirmOperation == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS &&
            authResponseContext_->haveCredential)) {
            if (authResponseContext_->bindLevel == APP && !authResponseContext_->isIdenticalAccount) {
                softbusConnector_->SetPkgName(authResponseContext_->hostPkgName);
            }
            softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
            if (timer_ != nullptr) {
                timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
            }
            authRequestContext_->reason = DM_OK;
            authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return;
        }
        if (authResponseContext_->confirmOperation == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS &&
            !authResponseContext_->haveCredential) {
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
            if (authResponseContext_->bindLevel == APP && !authResponseContext_->isIdenticalAccount) {
                softbusConnector_->SetPkgName(authResponseContext_->hostPkgName);
            }
            softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
            authRequestState_->TransitionTo(std::make_shared<AuthRequestCredential>());
            return;
        }
    }
    if (!authResponseContext_->isOnline && authResponseContext_->haveCredential) {
        softbusConnector_->JoinLnn(authRequestContext_->ip);
        if (timer_ != nullptr) {
            timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
        }
        authRequestContext_->reason = DM_OK;
        authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return;
    }
    if (!authResponseContext_->isOnline && !authResponseContext_->haveCredential) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestCredential>());
        return;
    }
}

void DmAuthManager::SinkAuthDeviceFinish()
{
    if (!authResponseContext_->haveCredential) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
    }
    if (authResponseContext_->isOnline) {
        LOGI("The device is online.");
        if (authResponseContext_->bindLevel == APP && !authResponseContext_->isIdenticalAccount) {
            softbusConnector_->SetPkgName(authResponseContext_->hostPkgName);
        }
        softbusConnector_->HandleDeviceOnline(remoteDeviceId_, authForm_);
    }
}

void DmAuthManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("DmAuthManager::AuthDeviceFinish start.");
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    isAuthDevice_ = false;
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(AUTH_DEVICE_TIMEOUT_TASK));
    }

    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        PutAccessControlList();
        SrcAuthDeviceFinish();
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        PutAccessControlList();
        SinkAuthDeviceFinish();
    }
}

void DmAuthManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthDeviceError start.");
    isAuthDevice_ = false;
    if (authRequestState_ == nullptr || authResponseState_ != nullptr) {
        LOGD("AuthDeviceError sink return.");
        return;
    }
    if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
        authRequestContext_->reason = ERR_DM_AUTH_CODE_INCORRECT;
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
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        } else {
            if (timer_ != nullptr) {
                timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK),
                    GetTaskTimeout(INPUT_TIMEOUT_TASK, INPUT_TIMEOUT), [this] (std::string name) {
                        DmAuthManager::HandleAuthenticateTimeout(name);
                    });
            }
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_PIN_CODE_ERROR);
        }
    }
}

void DmAuthManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("DmAuthManager::AuthDeviceSessionKey start.");
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    sessionKey_ = sessionKey;
    sessionKeyLen_ = sessionKeyLen;
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
    aclInfo.bindLevel = DEVICE;
    aclInfo.bindType = DM_POINT_TO_POINT;
    aclInfo.trustDeviceId = remoteDeviceId_;
    if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        aclInfo.authenticationType = ALLOW_AUTH_ALWAYS;
    } else if (action_ == USER_OPERATION_TYPE_ALLOW_AUTH) {
        aclInfo.authenticationType = ALLOW_AUTH_ONCE;
    }
    aclInfo.deviceIdHash = localUdidHash;

    DmAccesser accesser;
    accesser.requestTokenId = static_cast<uint64_t>(authResponseContext_->tokenId);
    accesser.requestBundleName = authResponseContext_->hostPkgName;
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        accesser.requestUserId = MultipleUserConnector::GetCurrentAccountUserID();
        accesser.requestAccountId = MultipleUserConnector::GetOhosAccountId();
        accesser.requestDeviceId = localUdid;
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        accesser.requestDeviceId = authResponseContext_->localDeviceId;
    }

    DmAccessee accessee;
    accessee.trustTokenId = static_cast<uint64_t>(authResponseContext_->tokenId);
    accessee.trustBundleName = authResponseContext_->hostPkgName;
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        accessee.trustDeviceId = remoteDeviceId_;
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        accessee.trustUserId = MultipleUserConnector::GetCurrentAccountUserID();
        accessee.trustAccountId = MultipleUserConnector::GetOhosAccountId();
        accessee.trustDeviceId = localUdid;
    }
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, accesser, accessee);
}

void DmAuthManager::CommonEventCallback(int32_t userId, std::string commonEventType)
{
    LOGI("DmAuthManager::CommonEventCallback");
    if (commonEventType == EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT) {
        AccountIdLogoutEventCallback(userId);
    } else if (commonEventType == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
        std::string accountId = MultipleUserConnector::GetOhosAccountId();
        LOGI("user_switched event accountId: %{public}s, userId: %{public}s",
            GetAnonyString(accountId).c_str(), GetAnonyInt32(userId).c_str());
        if (userId > 0) {
            MultipleUserConnector::SetSwitchOldUserId(userId);
            MultipleUserConnector::SetSwitchOldAccountId(accountId);
        }
    }
}

void DmAuthManager::AccountIdLogoutEventCallback(int32_t userId)
{
    LOGI("DmAuthManager::AccountIdLogoutEventCallback");
    std::string oldAccountId = MultipleUserConnector::GetSwitchOldAccountId();
    std::string currentAccountId = MultipleUserConnector::GetOhosAccountId();
    MultipleUserConnector::SetSwitchOldAccountId(currentAccountId);
    if (oldAccountId == currentAccountId) {
        LOGE("The account logout is error.");
        return;
    }
    if (currentAccountId == "ohosAnonymousUid" &&
        DeviceProfileConnector::GetInstance().CheckIdenticalAccount(userId, oldAccountId)) {
        DeviceProfileConnector::GetInstance().DeleteAccessControlList(userId, oldAccountId);
        hiChainConnector_->DeleteAllGroup(userId);
    }
}

void DmAuthManager::UserSwitchEventCallback(int32_t userId)
{
    LOGI("UserSwitchEventCallback start.");
    std::string oldAccountId = MultipleUserConnector::GetSwitchOldAccountId();
    int32_t oldUserId = MultipleUserConnector::GetSwitchOldUserId();
    DeviceProfileConnector::GetInstance().DeleteP2PAccessControlList(oldUserId, oldAccountId);
    DeviceProfileConnector::GetInstance().DeleteP2PAccessControlList(userId, oldAccountId);
    hiChainConnector_->DeleteP2PGroup(userId);
}

void DmAuthManager::UserChangeEventCallback(int32_t userId)
{
    LOGI("DmAuthManager::UserChangeEventCallback");
    std::string oldAccountId = MultipleUserConnector::GetSwitchOldAccountId();
    int32_t oldUseId = MultipleUserConnector::GetSwitchOldUserId();
    DeviceProfileConnector::GetInstance().DeleteP2PAccessControlList(oldUseId, oldAccountId);
    DeviceProfileConnector::GetInstance().DeleteP2PAccessControlList(userId, oldAccountId);
    hiChainConnector_->DeleteP2PGroup(userId);
}

void DmAuthManager::HandleSyncDeleteTimeout(std::string name)
{
    LOGI("DmAuthManager::HandleSyncDeleteTimeout start timer name %{public}s", name.c_str());
    if (authRequestState_ != nullptr && authRequestState_->GetStateType() != AuthState::AUTH_REQUEST_SYNCDELETE_DONE) {
        if (authResponseContext_ == nullptr) {
            authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        }
        authResponseContext_->state = authRequestState_->GetStateType();
        authResponseContext_->reply = ERR_DM_TIME_OUT;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestSyncDeleteAclNone>());
    }

    if (authResponseState_ != nullptr &&
        authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_SYNCDELETE_DONE) {
        authResponseContext_->state = authResponseState_->GetStateType();
        authResponseContext_->reply = ERR_DM_TIME_OUT;
        authResponseState_->TransitionTo(std::make_shared<AuthResponseSyncDeleteAclNone>());
    }
    LOGI("DmAuthManager::HandleSyncDeleteTimeout start complete");
}

int32_t DmAuthManager::DeleteAcl(const std::string &pkgName, const std::string &deviceId)
{
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    std::string localDeviceId = static_cast<std::string>(localUdid);
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId, deviceId);
    if (offlineParam.bindType == INVALIED_TYPE) {
        LOGE("Acl not contain the pkgName bind data.");
        return ERR_DM_FAILED;
    } else if (offlineParam.bindType == APP_PEER_TO_PEER_TYPE && offlineParam.leftAclNumber != 0) {
        LOGI("The pkgName unbind app-level type leftAclNumber not zero.");
        softbusConnector_->SetPkgName(pkgName);
        softbusConnector_->HandleDeviceOffline(deviceId);
    } else if (offlineParam.bindType == APP_PEER_TO_PEER_TYPE && offlineParam.leftAclNumber == 0) {
        LOGI("The pkgName unbind app-level type leftAclNumber is zero.");
        softbusConnector_->SetPkgName(pkgName);
        hiChainAuthConnector_->DeleteCredential(deviceId, MultipleUserConnector::GetCurrentAccountUserID());
    } else if (offlineParam.bindType == DEVICE_PEER_TO_PEER_TYPE && offlineParam.leftAclNumber != 0) {
        LOGI("Unbind deivce-level, retain identical account bind type.");
    } else if (offlineParam.bindType == DEVICE_PEER_TO_PEER_TYPE && offlineParam.leftAclNumber == 0) {
        LOGI("Unbind deivce-level, retain null.");
        hiChainAuthConnector_->DeleteCredential(deviceId, MultipleUserConnector::GetCurrentAccountUserID());
    }
    return DM_OK;
}

void DmAuthManager::ProcRespNegotiateExt(const int32_t &sessionId)
{
    LOGI("DmAuthManager::ProcRespNegotiateExt start.");
    remoteDeviceId_ = authResponseContext_->localDeviceId;
    std::string accountId = MultipleUserConnector::GetOhosAccountId();
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    MultipleUserConnector::SetSwitchOldAccountId(accountId);
    MultipleUserConnector::SetSwitchOldUserId(userId);
    authResponseContext_->isIdenticalAccount = false;
    if (authResponseContext_->localAccountId == accountId && accountId != "ohosAnonymousUid") {
        authResponseContext_->isIdenticalAccount = true;
    }
    authResponseContext_->remoteAccountId = authResponseContext_->localAccountId;
    authResponseContext_->localAccountId = accountId;
    authResponseContext_->remoteUserId = authResponseContext_->localUserId;
    authResponseContext_->localUserId = userId;
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    authResponseContext_->deviceId = authResponseContext_->localDeviceId;
    authResponseContext_->localDeviceId = static_cast<std::string>(localDeviceId);
    authResponseContext_->bindType =
        DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(authResponseContext_->hostPkgName,
        authResponseContext_->localDeviceId, authResponseContext_->deviceId);
    authResponseContext_->authed = !authResponseContext_->bindType.empty();
    if (authResponseContext_->authed && authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        !importAuthCode_.empty()) {
        authResponseContext_->importAuthCode = Crypto::Sha256(importAuthCode_);
    }
    authResponseContext_->isOnline = softbusConnector_->CheckIsOnline(remoteDeviceId_);
    authResponseContext_->haveCredential =
        hiChainAuthConnector_->QueryCredential(authResponseContext_->deviceId, authResponseContext_->localUserId);
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
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_NEGOTIATE);
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
}

void DmAuthManager::ProcRespNegotiate(const int32_t &sessionId)
{
    LOGI("DmAuthManager::ProcRespNegotiate session id");
    AbilityNegotiate();
    authResponseContext_->isOnline = softbusConnector_->CheckIsOnline(remoteDeviceId_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_NEGOTIATE);
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
        return;
    }
    if (!IsBool(jsonObject, TAG_CRYPTO_SUPPORT)) {
        LOGE("err json string.");
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
        return;
    }
    if (IsIdenticalAccount()) {
        jsonObject[TAG_IDENTICAL_ACCOUNT] = true;
        if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE && !importAuthCode_.empty()) {
            jsonObject[TAG_IMPORT_AUTH_CODE] = Crypto::Sha256(importAuthCode_);
        }
    }
    jsonObject[TAG_ACCOUNT_GROUPID] = GetAccountGroupIdHash();
    authResponseContext_ = authResponseState_->GetAuthContext();
    if (jsonObject[TAG_CRYPTO_SUPPORT] == true && authResponseContext_->cryptoSupport) {
        if (IsString(jsonObject, TAG_CRYPTO_NAME) && IsString(jsonObject, TAG_CRYPTO_VERSION)) {
            if (jsonObject[TAG_CRYPTO_NAME] == authResponseContext_->cryptoName &&
                jsonObject[TAG_CRYPTO_VERSION] == authResponseContext_->cryptoVer) {
                isCryptoSupport_ = true;
                softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
                return;
            }
        }
    }
    jsonObject[TAG_CRYPTO_SUPPORT] = false;
    message = jsonObject.dump();
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
}

void DmAuthManager::ProcIncompatible(const int32_t &sessionId)
{
    LOGI("DmAuthManager::ProcIncompatible sessionId %{public}d.", sessionId);
    nlohmann::json respNegotiateMsg;
    respNegotiateMsg[TAG_REPLY] = ERR_DM_VERSION_INCOMPATIBLE;
    respNegotiateMsg[TAG_VER] = DM_ITF_VER;
    respNegotiateMsg[TAG_MSG_TYPE] = MSG_TYPE_RESP_NEGOTIATE;
    std::string message = respNegotiateMsg.dump();
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
}

void DmAuthManager::OnAuthDeviceDataReceived(const int32_t sessionId, const std::string message)
{
    authResponseContext_->sessionId = sessionId;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return;
    }
    if (!IsString(jsonObject, TAG_DATA) || !IsInt32(jsonObject, TAG_DATA_LEN) || !IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("Auth device data is error.");
        return;
    }
    LOGI("OnAuthDeviceDataReceived start msgType %{public}d.", jsonObject[TAG_MSG_TYPE].get<int32_t>());
    std::string authData = jsonObject[TAG_DATA].get<std::string>();
    int32_t osAccountId = MultipleUserConnector::GetCurrentAccountUserID();
    hiChainAuthConnector_->ProcessAuthData(authResponseContext_->requestId, authData, osAccountId);
}

void DmAuthManager::BindSocketFail()
{
    LOGE("BindSocketFail");
    authResponseContext_->reply = DM_OK;
    isFinishOfLocal_ = false;
    authResponseContext_->hostPkgName = authRequestContext_->hostPkgName;
}

void DmAuthManager::BindSocketSuccess(int32_t socket)
{
    LOGI("BindSocketSuccess");
    if (authResponseState_ == nullptr && authRequestState_ != nullptr &&
        authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_DELETE_INIT) {
        authRequestContext_->sessionId = socket;
        authRequestState_->SetAuthContext(authRequestContext_);
        authMessageProcessor_->SetRequestContext(authRequestContext_);
        authResponseContext_->localDeviceId = authRequestContext_->localDeviceId;
        authResponseContext_->hostPkgName = authRequestContext_->hostPkgName;
        authMessageProcessor_->SetResponseContext(authResponseContext_);
        authRequestState_->TransitionTo(std::make_shared<AuthRequestSyncDeleteAcl>());
    } else {
        softbusConnector_->GetSoftbusSession()->CloseUnbindSession(socket);
        LOGE("DmAuthManager::BindSocketSuccess but request state is wrong");
    }
}

void DmAuthManager::OnUnbindSessionOpened(int32_t socket, PeerSocketInfo info)
{
    LOGI("DmAuthManager::OnUnbindSessionOpened socket: %{public}d, peerSocketName: %{public}s, peerNetworkId:"
        "%{public}s, peerPkgName: %{public}s", socket, info.name, GetAnonyString(info.networkId).c_str(), info.pkgName);
    if (authResponseState_ == nullptr && authRequestState_ == nullptr) {
        authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(shared_from_this());
        authResponseState_ = std::make_shared<AuthResponseInitState>();
        authResponseState_->SetAuthManager(shared_from_this());
        authResponseState_->Enter();
        authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        if (timer_ == nullptr) {
            timer_ = std::make_shared<DmTimer>();
        }
        timer_->StartTimer(std::string(SYNC_DELETE_TIMEOUT_TASK), SYNC_DELETE_TIMEOUT,
            [this] (std::string name) {
                DmAuthManager::HandleSyncDeleteTimeout(name);
            });
    } else {
        std::shared_ptr<AuthMessageProcessor> authMessageProcessor =
            std::make_shared<AuthMessageProcessor>(shared_from_this());
        std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
        authResponseContext->reply = ERR_DM_SYNC_DELETE_DEVICE_REPEATED;
        authMessageProcessor->SetResponseContext(authResponseContext);
        std::string message = authMessageProcessor->CreateSimpleMessage(MSG_TYPE_REQ_SYNC_DELETE_DONE);
        softbusConnector_->GetSoftbusSession()->SendData(socket, message);
    }
}

int32_t DmAuthManager::DeleteGroup(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DmAuthManager::DeleteGroup");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    hiChainConnector_->GetRelatedGroups(deviceId, groupList);
    if (groupList.size() > 0) {
        std::string groupId = "";
        groupId = groupList.front().groupId;
        hiChainConnector_->DeleteGroup(groupId);
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
    if (authResponseContext_->isIdenticalAccount) {
        aclInfo.bindType = DM_IDENTICAL_ACCOUNT;
        authForm_ = DmAuthForm::IDENTICAL_ACCOUNT;
    } else if (authResponseContext_->localAccountId == "ohosAnonymousUid" ||
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
    accesser.requestTokenId = static_cast<uint64_t>(authResponseContext_->tokenId);
    accesser.requestBundleName = authResponseContext_->hostPkgName;
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        accesser.requestUserId = authRequestContext_->localUserId;
        accesser.requestAccountId = authRequestContext_->localAccountId;
        accesser.requestDeviceId = authRequestContext_->localDeviceId;
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        accesser.requestUserId = authResponseContext_->remoteUserId;
        accesser.requestAccountId = authResponseContext_->remoteAccountId;
        accesser.requestDeviceId = authResponseContext_->localDeviceId;
    }
    DmAccessee accessee;
    accessee.trustTokenId = static_cast<uint64_t>(authResponseContext_->tokenId);
    accessee.trustBundleName = authResponseContext_->hostPkgName;
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        accessee.trustUserId = authRequestContext_->remoteUserId;
        accessee.trustAccountId = authRequestContext_->remoteAccountId;
        accessee.trustDeviceId = authResponseContext_->deviceId;
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        accessee.trustUserId = authResponseContext_->localUserId;
        accessee.trustAccountId = authResponseContext_->localAccountId;
        accessee.trustDeviceId = localUdid;
    }
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, accesser, accessee);
}

void DmAuthManager::HandleSessionHeartbeat(std::string name)
{
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK));
    }
    LOGI("DmAuthManager::HandleSessionHeartbeat name %{public}s", name.c_str());
    nlohmann::json jsonObj;
    jsonObj[TAG_SESSION_HEARTBEAT] = TAG_SESSION_HEARTBEAT;
    std::string message = jsonObj.dump();
    softbusConnector_->GetSoftbusSession()->SendHeartbeatData(authResponseContext_->sessionId, message);

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
    if (authResponseContext_->isOnline && authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        SetReasonAndFinish(DM_OK, AuthState::AUTH_REQUEST_FINISH);
        return ALREADY_BIND;
    }
    if (authResponseContext_->isIdenticalAccount) {
        if (IsIdenticalAccount()) {
            softbusConnector_->JoinLnn(authResponseContext_->deviceId);
            authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
            authRequestContext_->reason = DM_OK;
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return ALREADY_BIND;
        }
    }
    if (authResponseContext_->reply == ERR_DM_AUTH_PEER_REJECT) {
        if (hiChainConnector_->IsDevicesInP2PGroup(authResponseContext_->localDeviceId,
                                                   authRequestContext_->localDeviceId)) {
            if (!DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(authResponseContext_->hostPkgName,
                authResponseContext_->localDeviceId)) {
                CompatiblePutAcl();
            }
            softbusConnector_->JoinLnn(authResponseContext_->deviceId);
            authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return ALREADY_BIND;
        }
    }
    if (authResponseContext_->reply == ERR_DM_UNSUPPORTED_AUTH_TYPE ||
        (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        authResponseContext_->isAuthCodeReady == false)) {
        authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
        authRequestContext_->reason = ERR_DM_BIND_PEER_UNSUPPORTED;
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return ERR_DM_BIND_PEER_UNSUPPORTED;
    }
    return DM_OK;
}

std::string DmAuthManager::GetBundleLable(const std::string &bundleName)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get ability manager failed");
        return bundleName;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (object == nullptr) {
        LOGE("object is NULL.");
        return bundleName;
    }

    sptr<OHOS::AppExecFwk::IBundleMgr> bms = iface_cast<OHOS::AppExecFwk::IBundleMgr>(object);
    if (bms == nullptr) {
        LOGE("bundle manager service is NULL.");
        return bundleName;
    }

    auto bundleResourceProxy = bms->GetBundleResourceProxy();
    if (bundleResourceProxy == nullptr) {
        LOGE("GetBundleResourceProxy fail");
        return bundleName;
    }
    AppExecFwk::BundleResourceInfo resourceInfo;
    auto result = bundleResourceProxy->GetBundleResourceInfo(bundleName,
        static_cast<uint32_t>(OHOS::AppExecFwk::ResourceFlag::GET_RESOURCE_INFO_ALL), resourceInfo);
    if (result != ERR_OK) {
        LOGE("GetBundleResourceInfo failed");
        return bundleName;
    }
    LOGI("bundle resource label is %{public}s ", (resourceInfo.label).c_str());
    return resourceInfo.label;
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

std::string DmAuthManager::ConvertSrcVersion(const std::string &version, const std::string &edition)
{
    std::string srcVersion = "";
    if (version == "" && edition != "") {
        srcVersion = edition;
    } else if (version == "" && edition == "") {
        srcVersion = DM_VERSION_5_0_1;
    } else if (version != "" && edition == "") {
        srcVersion = version;
    }
    LOGI("ConvertSrcVersion version %{public}s, edition %{public}s, srcVersion is %{public}s.",
        version.c_str(), edition.c_str(), srcVersion.c_str());
    return srcVersion;
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

bool DmAuthManager::CompareVersion(const std::string &remoteVersion, const std::string &oldVersion)
{
    LOGI("remoteVersion %{public}s, oldVersion %{public}s.", remoteVersion.c_str(), oldVersion.c_str());
    std::vector<int32_t> remoteVersionVec;
    std::vector<int32_t> oldVersionVec;
    VersionSplitToInt(remoteVersion, '.', remoteVersionVec);
    VersionSplitToInt(oldVersion, '.', oldVersionVec);
    return CompareVecNum(remoteVersionVec, oldVersionVec);
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

int32_t DmAuthManager::RegisterAuthenticationType(int32_t authenticationType)
{
    if (authenticationType != USER_OPERATION_TYPE_ALLOW_AUTH &&
        authenticationType != USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        LOGE("Invalid parameter.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    authenticationType_ = authenticationType;
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS