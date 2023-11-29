/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <string>
#include <unistd.h>

#include "auth_message_processor.h"
#include "dm_ability_manager.h"
#include "dm_anonymous.h"
#include "dm_config_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_adapter_crypto.h"
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
const int32_t CANCEL_PIN_CODE_DISPLAY = 1;
const int32_t DEVICE_ID_HALF = 2;
const int32_t MAX_AUTH_TIMES = 3;
const int32_t MIN_PIN_TOKEN = 10000000;
const int32_t MAX_PIN_TOKEN = 90000000;
const int32_t MIN_PIN_CODE = 100000;
const int32_t MAX_PIN_CODE = 999999;
const int32_t DM_AUTH_TYPE_MAX = 5;
const int32_t DM_AUTH_TYPE_MIN = 1;
const int32_t AUTH_SESSION_SIDE_SERVER = 0;
const int32_t USLEEP_TIME_MS = 500000; // 500ms
const int32_t DM_APP = 3;
constexpr const char* APP_OPERATION_KEY = "appOperation";
constexpr const char* TARGET_PKG_NAME_KEY = "targetPkgName";
constexpr const char* CUSTOM_DESCRIPTION_KEY = "customDescription";
constexpr const char* CANCEL_DISPLAY_KEY = "cancelPinCodeDisplay";

DmAuthManager::DmAuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                             std::shared_ptr<IDeviceManagerServiceListener> listener,
                             std::shared_ptr<HiChainConnector> hiChainConnector)
    : softbusConnector_(softbusConnector), hiChainConnector_(hiChainConnector), listener_(listener)
{
    LOGI("DmAuthManager constructor");
    DmConfigManager &dmConfigManager = DmConfigManager::GetInstance();
    dmConfigManager.GetAuthAdapter(authenticationMap_);
    authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener_);
    authenticationMap_[AUTH_TYPE_IMPORT_AUTH_CODE] = nullptr;
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
        LOGE("DmAuthManager::CheckAuthParamVaild failed, pkgName is %s, deviceId is %s, extra is %s.",
            pkgName.c_str(), GetAnonyString(deviceId).c_str(), extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (listener_ == nullptr || authUiStateMgr_ == nullptr) {
        LOGE("DmAuthManager::CheckAuthParamVaild listener or authUiStateMgr is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (!IsAuthTypeSupported(authType)) {
        LOGE("DmAuthManager::CheckAuthParamVaild authType %d not support.", authType);
        listener_->OnAuthResult(pkgName, deviceId, "", STATUS_DM_AUTH_DEFAULT, ERR_DM_UNSUPPORTED_AUTH_TYPE);
        listener_->OnBindResult(pkgName, peerTargetId_, ERR_DM_UNSUPPORTED_AUTH_TYPE, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_UNSUPPORTED_AUTH_TYPE;
    }

    if (authRequestState_ != nullptr || authResponseState_ != nullptr) {
        LOGE("DmAuthManager::CheckAuthParamVaild %s is request authentication.", pkgName.c_str());
        listener_->OnAuthResult(pkgName, deviceId, "", STATUS_DM_AUTH_DEFAULT, ERR_DM_AUTH_BUSINESS_BUSY);
        listener_->OnBindResult(pkgName, peerTargetId_, ERR_DM_AUTH_BUSINESS_BUSY, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_AUTH_BUSINESS_BUSY;
    }

    if (!softbusConnector_->HaveDeviceInMap(deviceId)) {
        LOGE("CheckAuthParamVaild failed, the discoveryDeviceInfoMap_ not have this device.");
        listener_->OnAuthResult(pkgName, deviceId, "", STATUS_DM_AUTH_DEFAULT, ERR_DM_INPUT_PARA_INVALID);
        listener_->OnBindResult(pkgName, peerTargetId_, ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if ((authType == AUTH_TYPE_IMPORT_AUTH_CODE) && (!IsAuthCodeReady(pkgName))) {
        LOGE("Auth code not exist.");
        listener_->OnAuthResult(pkgName, deviceId, "", STATUS_DM_AUTH_DEFAULT, ERR_DM_INPUT_PARA_INVALID);
        listener_->OnBindResult(pkgName, peerTargetId_, ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}

int32_t DmAuthManager::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("DmAuthManager::AuthenticateDevice start auth type %d.", authType);
    int32_t ret = CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    if (ret != DM_OK) {
        LOGE("DmAuthManager::AuthenticateDevice failed, param is invaild.");
        return ret;
    }

    authPtr_ = authenticationMap_[authType];
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK), AUTHENTICATE_TIMEOUT,
        [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
    authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(shared_from_this());
    authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authRequestContext_->hostPkgName = pkgName;
    authRequestContext_->authType = authType;
    authRequestContext_->localDeviceName = softbusConnector_->GetLocalDeviceName();
    authRequestContext_->localDeviceTypeId = softbusConnector_->GetLocalDeviceTypeId();
    authRequestContext_->deviceId = deviceId;
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
    }
    authRequestContext_->token = std::to_string(GenRandInt(MIN_PIN_TOKEN, MAX_PIN_TOKEN));
    authMessageProcessor_->SetRequestContext(authRequestContext_);
    authRequestState_ = std::make_shared<AuthRequestInitState>();
    authRequestState_->SetAuthManager(shared_from_this());
    authRequestState_->SetAuthContext(authRequestContext_);
    if (!DmRadarHelper::GetInstance().ReportAuthStart(deviceId)) {
        LOGE("ReportAuthStart failed");
    }
    authRequestState_->Enter();
    LOGI("DmAuthManager::AuthenticateDevice complete");
    return DM_OK;
}

int32_t DmAuthManager::UnAuthenticateDevice(const std::string &pkgName, const std::string &networkId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::string deviceUdid;
    int32_t ret = SoftbusConnector::GetUdidByNetworkId(networkId.c_str(), deviceUdid);
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }

    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    hiChainConnector_->GetRelatedGroups(deviceUdid, groupList);
    if (groupList.size() > 0) {
        std::string groupId = "";
        groupId = groupList.front().groupId;
        LOGI("DmAuthManager::UnAuthenticateDevice groupId = %s, networkId = %s, deviceUdid = %s",
            GetAnonyString(groupId).c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(deviceUdid).c_str());
        hiChainConnector_->DeleteGroup(groupId);
    } else {
        LOGE("DmAuthManager::UnAuthenticateDevice groupList.size = 0");
        return ERR_DM_FAILED;
    }
    if (softbusConnector_ != nullptr) {
        softbusConnector_->EraseUdidFromMap(deviceUdid);
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    struct RadarInfo info = {
        .funcName = "UnAuthenticateDevice",
        .toCallPkg = HICHAINNAME,
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_START),
        .isTrust = static_cast<int32_t>(TrustStatus::NOT_TRUST),
        .peerNetId = networkId,
        .localUdid = localDeviceId,
        .peerUdid = deviceUdid,
    };
    if (!DmRadarHelper::GetInstance().ReportDeleteTrustRelation(info)) {
        LOGE("ReportDeleteTrustRelation failed");
    }
    return DM_OK;
}

int32_t DmAuthManager::UnBindDevice(const std::string &pkgName, const std::string &udidHash)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::string udid = "";
    udid = SoftbusConnector::GetDeviceUdidByUdidHash(udidHash);

    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    hiChainConnector_->GetRelatedGroups(udid, groupList);
    if (groupList.size() > 0) {
        std::string groupId = "";
        groupId = groupList.front().groupId;
        LOGI("DmAuthManager::UnBindDevice groupId = %s, udidHash = %s, udid = %s",
            GetAnonyString(groupId).c_str(), GetAnonyString(udidHash).c_str(), GetAnonyString(udid).c_str());
        hiChainConnector_->DeleteGroup(groupId);
    } else {
        LOGE("DmAuthManager::UnBindDevice groupList.size = 0");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void DmAuthManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("DmAuthManager::OnSessionOpened sessionId = %d result = %d", sessionId, result);
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
            timer_->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK), AUTHENTICATE_TIMEOUT,
                [this] (std::string name) {
                    DmAuthManager::HandleAuthenticateTimeout(name);
                });
            timer_->StartTimer(std::string(WAIT_NEGOTIATE_TIMEOUT_TASK), WAIT_NEGOTIATE_TIMEOUT,
                [this] (std::string name) {
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
            struct RadarInfo info = {
                .funcName = "OnSessionOpened",
                .channelId = sessionId,
            };
            if (!DmRadarHelper::GetInstance().ReportAuthSendRequest(info)) {
                LOGE("ReportAuthSendRequest failed");
            }
        }
    } else {
        if (authResponseState_ == nullptr && authRequestState_ != nullptr &&
            authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_INIT) {
            authRequestContext_->sessionId = sessionId;
            authMessageProcessor_->SetRequestContext(authRequestContext_);
            authRequestState_->SetAuthContext(authRequestContext_);
            authRequestState_->TransitionTo(std::make_shared<AuthRequestNegotiateState>());
        } else {
            softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
            LOGE("DmAuthManager::OnSessionOpened but request state is wrong");
        }
    }
}

void DmAuthManager::OnSessionClosed(const int32_t sessionId)
{
    LOGI("DmAuthManager::OnSessionOpened sessionId = %d", sessionId);
}

void DmAuthManager::ProcessSourceMsg()
{
    authRequestContext_ = authMessageProcessor_->GetRequestContext();
    authRequestState_->SetAuthContext(authRequestContext_);
    LOGI("OnDataReceived for source device, authResponseContext msgType = %d, authRequestState stateType = %d",
        authResponseContext_->msgType, authRequestState_->GetStateType());

    switch (authResponseContext_->msgType) {
        case MSG_TYPE_RESP_AUTH:
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
        default:
            break;
    }
}

void DmAuthManager::ProcessSinkMsg()
{
    authResponseState_->SetAuthContext(authResponseContext_);
    LOGI("OnDataReceived for sink device, authResponseContext msgType = %d, authResponseState stateType = %d",
        authResponseContext_->msgType, authResponseState_->GetStateType());

    switch (authResponseContext_->msgType) {
        case MSG_TYPE_NEGOTIATE:
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_INIT) {
                timer_->DeleteTimer(std::string(WAIT_NEGOTIATE_TIMEOUT_TASK));
                authResponseState_->TransitionTo(std::make_shared<AuthResponseNegotiateState>());
            } else {
                LOGE("Device manager auth state error");
            }
            break;
        case MSG_TYPE_REQ_AUTH:
            if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_NEGOTIATE) {
                timer_->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));
                authResponseState_->TransitionTo(std::make_shared<AuthResponseConfirmState>());
            } else {
                LOGE("Device manager auth state error");
            }
            break;
        case MSG_TYPE_REQ_AUTH_TERMINATE:
            if (authResponseState_->GetStateType() != AuthState::AUTH_RESPONSE_FINISH) {
                isFinishOfLocal_ = false;
                authResponseState_->TransitionTo(std::make_shared<AuthResponseFinishState>());
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
    LOGI("DmAuthManager::OnGroupCreated start group id %s", GetAnonyString(groupId).c_str());
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
    LOGI("DmAuthManager::OnGroupCreated start group id %s", GetAnonyString(groupId).c_str());
    authResponseContext_->groupId = groupId;
    authResponseContext_->code = pinCode;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_AUTH);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
    authResponseState_->TransitionTo(std::make_shared<AuthResponseShowState>());
}

void DmAuthManager::OnMemberJoin(int64_t requestId, int32_t status)
{
    if (authResponseContext_ == nullptr || authUiStateMgr_ == nullptr) {
        LOGE("failed to OnMemberJoin because authResponseContext_ or authUiStateMgr is nullptr");
        return;
    }
    LOGI("DmAuthManager OnMemberJoin start authTimes %d", authTimes_);
    isAddingMember_ = false;
    if ((authRequestState_ != nullptr) && (authResponseState_ == nullptr)) {
        if (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
            HandleMemberJoinImportAuthCode(requestId, status);
            return;
        }
        authTimes_++;
        timer_->DeleteTimer(std::string(ADD_TIMEOUT_TASK));
        if (status != DM_OK || authResponseContext_->requestId != requestId) {
            if (authRequestState_ != nullptr && authTimes_ >= MAX_AUTH_TIMES) {
                authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
                authRequestContext_->reason = ERR_DM_BIND_PIN_CODE_ERROR;
                authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            } else {
                timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK), INPUT_TIMEOUT,
                    [this] (std::string name) {
                        DmAuthManager::HandleAuthenticateTimeout(name);
                    });
                authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_PIN_CODE_ERROR);
            }
        } else {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestNetworkState>());
        }
    } else if ((authResponseState_ != nullptr) && (authRequestState_ == nullptr)) {
        if (status == DM_OK && authResponseContext_->requestId == requestId &&
            authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_SHOW) {
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
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
    LOGI("DmAuthManager::HandleAuthenticateTimeout start timer name %s", name.c_str());
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
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
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
        .localUdid = localDeviceId,
        .peerUdid = deviceId,
        .channelId = sessionId,
        .errCode = ERR_DM_AUTH_OPEN_SESSION_FAILED,
    };
    if (!DmRadarHelper::GetInstance().ReportAuthOpenSession(info)) {
        LOGE("ReportAuthOpenSession failed");
    }
    if (sessionId < 0) {
        LOGE("OpenAuthSession failed, stop the authentication");
        authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        authResponseContext_->state = AuthState::AUTH_REQUEST_NEGOTIATE;
        authRequestContext_->reason = ERR_DM_AUTH_OPEN_SESSION_FAILED;
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
    LOGI("DmAuthManager::StartNegotiate session id");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    authRequestContext_->localDeviceId = localDeviceId;
    authResponseContext_->localDeviceId = localDeviceId;
    authResponseContext_->reply = ERR_DM_AUTH_REJECT;
    authResponseContext_->authType = authRequestContext_->authType;
    authResponseContext_->deviceId = authRequestContext_->deviceId;
    authResponseContext_->accountGroupIdHash = GetAccountGroupIdHash();
    authResponseContext_->hostPkgName = authRequestContext_->hostPkgName;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_NEGOTIATE);
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
    timer_->StartTimer(std::string(NEGOTIATE_TIMEOUT_TASK), NEGOTIATE_TIMEOUT,
        [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
}

void DmAuthManager::AbilityNegotiate()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    bool ret = hiChainConnector_->IsDevicesInP2PGroup(authResponseContext_->localDeviceId, localDeviceId);
    if (ret) {
        LOGE("DmAuthManager::EstablishAuthChannel device is in group");
        authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
    } else {
        authResponseContext_->reply = ERR_DM_AUTH_REJECT;
    }
    authResponseContext_->localDeviceId = localDeviceId;

    if (!IsAuthTypeSupported(authResponseContext_->authType)) {
        LOGE("DmAuthManager::AuthenticateDevice authType %d not support.", authResponseContext_->authType);
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
    LOGI("DmAuthManager::RespNegotiate session id");
    AbilityNegotiate();

    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_NEGOTIATE);
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
    }

    if (IsIdenticalAccount()) {
        jsonObject[TAG_IDENTICAL_ACCOUNT] = true;
    }
    jsonObject[TAG_ACCOUNT_GROUPID] = GetAccountGroupIdHash();
    authResponseContext_ = authResponseState_->GetAuthContext();
    if (jsonObject[TAG_CRYPTO_SUPPORT] == true && authResponseContext_->cryptoSupport) {
        if (jsonObject[TAG_CRYPTO_NAME] == authResponseContext_->cryptoName &&
            jsonObject[TAG_CRYPTO_VERSION] == authResponseContext_->cryptoVer) {
            isCryptoSupport_ = true;
            softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
            return;
        }
    }
    jsonObject[TAG_CRYPTO_SUPPORT] = false;
    message = jsonObject.dump();
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
    timer_->StartTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK), WAIT_REQUEST_TIMEOUT,
        [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
}

void DmAuthManager::SendAuthRequest(const int32_t &sessionId)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to SendAuthRequest because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::SendAuthRequest session id");
    timer_->DeleteTimer(std::string(NEGOTIATE_TIMEOUT_TASK));
    if (authResponseContext_->cryptoSupport) {
        isCryptoSupport_ = true;
    }

    if (authResponseContext_->isIdenticalAccount) { // identicalAccount joinLNN indirectly, no need to verify
        if (IsIdenticalAccount()) {
            softbusConnector_->JoinLnn(authResponseContext_->deviceId);
            authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
            authRequestContext_->reason = DM_OK;
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return;
        }
    }
    if (authResponseContext_->reply == ERR_DM_AUTH_PEER_REJECT) {
        if (hiChainConnector_->IsDevicesInP2PGroup(authResponseContext_->localDeviceId,
                                                   authRequestContext_->localDeviceId)) {
            softbusConnector_->JoinLnn(authResponseContext_->deviceId);
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return;
        }
    }
    if (authResponseContext_->reply == ERR_DM_UNSUPPORTED_AUTH_TYPE ||
        (authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE &&
        authResponseContext_->isAuthCodeReady == false)) {
        authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        return;
    }

    std::vector<std::string> messageList = authMessageProcessor_->CreateAuthRequestMessage();
    for (auto msg : messageList) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, msg);
    }
    timer_->StartTimer(std::string(CONFIRM_TIMEOUT_TASK), CONFIRM_TIMEOUT,
        [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
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
        .errCode = USER_OPERATION_TYPE_CANCEL_AUTH,
    };
    if (!DmRadarHelper::GetInstance().ReportAuthConfirmBox(info)) {
        LOGE("ReportAuthConfirmBox failed");
    }
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

void DmAuthManager::StartRespAuthProcess()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to StartRespAuthProcess because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::StartRespAuthProcess sessionId = %d", authResponseContext_->sessionId);
    timer_->DeleteTimer(std::string(CONFIRM_TIMEOUT_TASK));
    if (authResponseContext_->reply == USER_OPERATION_TYPE_ALLOW_AUTH) {
        timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK), INPUT_TIMEOUT,
            [this] (std::string name) {
                DmAuthManager::HandleAuthenticateTimeout(name);
            });
        listener_->OnAuthResult(authRequestContext_->hostPkgName, authRequestContext_->deviceId,
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
    LOGI("DmAuthManager::AddMember start group id %s", GetAnonyString(authResponseContext_->groupId).c_str());
    timer_->DeleteTimer(std::string(INPUT_TIMEOUT_TASK));
    nlohmann::json jsonObject;
    jsonObject[TAG_GROUP_ID] = authResponseContext_->groupId;
    jsonObject[TAG_GROUP_NAME] = authResponseContext_->groupName;
    jsonObject[PIN_CODE_KEY] = pinCode;
    jsonObject[TAG_REQUEST_ID] = authResponseContext_->requestId;
    jsonObject[TAG_DEVICE_ID] = authResponseContext_->deviceId;
    std::string connectInfo = jsonObject.dump();
    timer_->StartTimer(std::string(ADD_TIMEOUT_TASK), ADD_TIMEOUT,
        [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
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
    int32_t ret = hiChainConnector_->AddMember(authRequestContext_->deviceId, connectInfo);
    struct RadarInfo info = {
        .funcName = "AddMember",
        .stageRes = (ret == 0) ?
            static_cast<int32_t>(StageRes::STAGE_IDLE) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .peerUdid = authResponseContext_->deviceId,
        .errCode = ret,
    };
    if (!DmRadarHelper::GetInstance().ReportAuthAddGroup(info)) {
        LOGE("ReportAuthAddGroup failed");
    }
    if (ret != 0) {
        LOGE("DmAuthManager::AddMember failed, ret: %d", ret);
        isAddingMember_ = false;
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

std::string DmAuthManager::GetConnectAddr(std::string deviceId)
{
    LOGI("DmAuthManager::GetConnectAddr");
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
    timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
    authResponseContext_->state = AuthState::AUTH_REQUEST_FINISH;
    authRequestContext_->reason = DM_OK;
    authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
    return DM_OK;
}

void DmAuthManager::AuthenticateFinish()
{
    if (authResponseContext_ == nullptr || authUiStateMgr_ == nullptr) {
        LOGE("failed to AuthenticateFinish because authResponseContext_ or authUiStateMgr is nullptr");
        return;
    }
    LOGI("DmAuthManager::AuthenticateFinish start");
    isAddingMember_ = false;
    DeleteAuthCode();
    if (authResponseState_ != nullptr) {
        if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_FINISH && authPtr_ != nullptr) {
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
        }
        if (isFinishOfLocal_) {
            authMessageProcessor_->SetResponseContext(authResponseContext_);
            std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_AUTH_TERMINATE);
            softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        }
        authResponseState_ = nullptr;
    } else if (authRequestState_ != nullptr) {
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
        listener_->OnAuthResult(authRequestContext_->hostPkgName, authRequestContext_->deviceId,
                                authRequestContext_->token, authResponseContext_->state, authRequestContext_->reason);
        listener_->OnBindResult(authRequestContext_->hostPkgName, peerTargetId_, authRequestContext_->reason,
            authResponseContext_->state, "");
        usleep(USLEEP_TIME_MS); // 500ms
        softbusConnector_->GetSoftbusSession()->CloseAuthSession(authRequestContext_->sessionId);
        authRequestContext_ = nullptr;
        authRequestState_ = nullptr;
        authTimes_ = 0;
    }
    timer_->DeleteAll();
    isFinishOfLocal_ = true;
    authResponseContext_ = nullptr;
    authMessageProcessor_ = nullptr;
    authPtr_ = nullptr;
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

int32_t DmAuthManager::GetPinCode()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to GetPinCode because authResponseContext_ is nullptr");
        return ERR_DM_AUTH_NOT_START;
    }
    LOGI("ShowConfigDialog start add member pin code.");
    return authResponseContext_->code;
}

void DmAuthManager::ShowConfigDialog()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowConfigDialog because authResponseContext_ is nullptr");
        return;
    }
    if (!authResponseContext_->isShowDialog) {
        LOGI("start auth process");
        StartAuthProcess(USER_OPERATION_TYPE_ALLOW_AUTH);
        return;
    }
    LOGI("ShowConfigDialog start");
    dmAbilityMgr_ = std::make_shared<DmAbilityManager>();
    nlohmann::json jsonObj;
    jsonObj[TAG_AUTH_TYPE] = AUTH_TYPE_PIN;
    jsonObj[TAG_TOKEN] = authResponseContext_->token;
    jsonObj[TARGET_PKG_NAME_KEY] = authResponseContext_->targetPkgName;
    jsonObj[TAG_CUSTOM_DESCRIPTION] = authResponseContext_->customDesc;
    jsonObj[TAG_APP_OPERATION] = authResponseContext_->appOperation;
    jsonObj[TAG_LOCAL_DEVICE_TYPE] = authResponseContext_->deviceTypeId;
    jsonObj[TAG_REQUESTER] = authResponseContext_->deviceName;
    const std::string params = jsonObj.dump();
    std::shared_ptr<ShowConfirm> showConfirm_ = std::make_shared<ShowConfirm>();
    showConfirm_->ShowConfirmDialog(params, shared_from_this(), dmAbilityMgr_);
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
    LOGI("DmAuthManager::ShowAuthInfoDialog start %d", authResponseContext_->code);
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
    authPtr_->ShowAuthInfo(authParam, shared_from_this());
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
        AddMember(pinCode);
        return;
    }
    struct RadarInfo info = {
        .funcName = "ShowStartAuthDialog",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE),
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_START),
        .isTrust = static_cast<int32_t>(TrustStatus::NOT_TRUST),
    };
    if (!DmRadarHelper::GetInstance().ReportAuthInputPinBox(info)) {
        LOGE("ReportAuthInputPinBox failed");
    }
    LOGI("DmAuthManager::ShowStartAuthDialog start");
    authPtr_->StartAuth(authResponseContext_->authToken, shared_from_this());
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
        .errCode = action,
    };
    switch (action) {
        case USER_OPERATION_TYPE_ALLOW_AUTH:
        case USER_OPERATION_TYPE_CANCEL_AUTH:
        case USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS:
            StartAuthProcess(action);
            break;
        case USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT:
            SetReasonAndFinish(ERR_DM_TIME_OUT, STATUS_DM_AUTH_DEFAULT);
            if (!DmRadarHelper::GetInstance().ReportAuthConfirmBox(info)) {
                LOGE("ReportAuthConfirmBox failed");
            }
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY:
            SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY, STATUS_DM_AUTH_DEFAULT);
            if (!DmRadarHelper::GetInstance().ReportAuthInputPinBox(info)) {
                LOGE("ReportAuthInputPinBox failed");
            }
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT:
            SetReasonAndFinish(ERR_DM_BIND_USER_CANCEL_ERROR, STATUS_DM_AUTH_DEFAULT);
            if (!DmRadarHelper::GetInstance().ReportAuthInputPinBox(info)) {
                LOGE("ReportAuthInputPinBox failed");
            }
            break;
        case USER_OPERATION_TYPE_DONE_PINCODE_INPUT:
            AddMember(std::stoi(params));
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

void DmAuthManager::UserSwitchEventCallback(int32_t userId)
{
    LOGI("switch user event happen and this user groups will be deleted with userId: %d", userId);
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    std::string queryParams = jsonObj.dump();
    std::vector<GroupInfo> groupList;

    int32_t oldUserId = MultipleUserConnector::GetSwitchOldUserId();
    MultipleUserConnector::SetSwitchOldUserId(userId);
    if (!hiChainConnector_->GetGroupInfo(oldUserId, queryParams, groupList)) {
        LOGE("failed to get the old user id groups");
        return;
    }
    for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
        int32_t ret = hiChainConnector_->DeleteGroup(oldUserId, iter->groupId);
        if (ret != DM_OK) {
            LOGE("failed to delete the old user id group");
        }
    }

    if (!hiChainConnector_->GetGroupInfo(userId, queryParams, groupList)) {
        LOGE("failed to get the user id groups");
        return;
    }
    for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
        int32_t ret = hiChainConnector_->DeleteGroup(userId, iter->groupId);
        if (ret != DM_OK) {
            LOGE("failed to delete the user id group");
        }
    }
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
            if ((*it) == DmSoftbusAdapterCrypto::GetGroupIdHash(groupInfo.groupId)) {
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
        jsonAccountObj.push_back(DmSoftbusAdapterCrypto::GetGroupIdHash(groupInfo.groupId));
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
    if (pkgName.empty()) {
        LOGE("DmAuthManager::BindTarget failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t authType = -1;
    if (ParseAuthType(bindParam, authType) != DM_OK) {
        LOGE("DmAuthManager::BindTarget failed, key: %s error.", PARAM_KEY_AUTH_TYPE.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string extra = "";
    ParseExtra(bindParam, extra);
    std::string deviceId = "";
    peerTargetId_ = targetId;
    if (ParseConnectAddr(targetId, deviceId) == DM_OK) {
        return AuthenticateDevice(pkgName, authType, deviceId, extra);
    } else if (!targetId.deviceId.empty()) {
        return AuthenticateDevice(pkgName, authType, targetId.deviceId, extra);
    } else {
        LOGE("DmAuthManager::BindTarget failed, key: %s error.", PARAM_KEY_AUTH_TYPE.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
}

int32_t DmAuthManager::ParseConnectAddr(const PeerTargetId &targetId, std::string &deviceId)
{
    int32_t index = 0;
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    ConnectionAddr addr;
    if (!targetId.wifiIp.empty() && targetId.wifiIp.length() <= IP_STR_MAX_LEN) {
        LOGI("DmAuthManager::ParseConnectAddr parse wifiIp: %s.", GetAnonyString(targetId.wifiIp).c_str());
        addr.type = ConnectionAddrType::CONNECTION_ADDR_WLAN;
        memcpy_s(addr.info.ip.ip, IP_STR_MAX_LEN, targetId.wifiIp.c_str(), targetId.wifiIp.length());
        addr.info.ip.port = targetId.wifiPort;
        deviceInfo->addr[index] = addr;
        deviceId = targetId.wifiIp;
        index++;
    } else if (!targetId.brMac.empty() && targetId.brMac.length() <= BT_MAC_LEN) {
        LOGI("DmAuthManager::ParseConnectAddr parse brMac: %s.", GetAnonyString(targetId.brMac).c_str());
        addr.type = ConnectionAddrType::CONNECTION_ADDR_BR;
        memcpy_s(addr.info.br.brMac, BT_MAC_LEN, targetId.brMac.c_str(), targetId.brMac.length());
        deviceInfo->addr[index] = addr;
        deviceId = targetId.brMac;
        index++;
    } else if (!targetId.bleMac.empty() && targetId.bleMac.length() <= BT_MAC_LEN) {
        LOGI("DmAuthManager::ParseConnectAddr parse bleMac: %s.", GetAnonyString(targetId.bleMac).c_str());
        addr.type = ConnectionAddrType::CONNECTION_ADDR_BLE;
        memcpy_s(addr.info.ble.bleMac, BT_MAC_LEN, targetId.bleMac.c_str(), targetId.bleMac.length());
        deviceInfo->addr[index] = addr;
        deviceId = targetId.bleMac;
        index++;
    } else {
        LOGE("DmAuthManager::ParseConnectAddr failed, not addr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    deviceInfo->addrNum = index;
    if (softbusConnector_->AddMemberToDiscoverMap(deviceId, deviceInfo) != DM_OK) {
        LOGE("DmAuthManager::ParseConnectAddr failed, AddMemberToDiscoverMap failed.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (deviceInfo != nullptr) {
        deviceInfo = nullptr;
    }

    return DM_OK;
}

int32_t DmAuthManager::ParseAuthType(const std::map<std::string, std::string> &bindParam, int32_t &authType)
{
    auto iter = bindParam.find(PARAM_KEY_AUTH_TYPE);
    if (iter == bindParam.end()) {
        LOGE("DmAuthManager::ParseAuthType bind param key: %s not exist.", PARAM_KEY_AUTH_TYPE.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string authTypeStr = iter->second;
    if (authTypeStr.empty()) {
        LOGE("DmAuthManager::ParseAuthType bind param %s is empty.", PARAM_KEY_AUTH_TYPE.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authTypeStr.length() > 1) {
        LOGE("DmAuthManager::ParseAuthType bind param %s length is unsupported.", PARAM_KEY_AUTH_TYPE.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!isdigit(authTypeStr[0])) {
        LOGE("DmAuthManager::ParseAuthType bind param %s fromat is unsupported.", PARAM_KEY_AUTH_TYPE.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    authType = std::stoi(authTypeStr);
    return DM_OK;
}

int32_t DmAuthManager::ParseExtra(const std::map<std::string, std::string> &bindParam, std::string &extra)
{
    auto iter = bindParam.find(PARAM_KEY_APP_DESC);
    if (iter == bindParam.end()) {
        LOGE("DmAuthManager::ParseExtra bind param key: %s not exist.", PARAM_KEY_APP_DESC.c_str());
        extra = "";
        return DM_OK;
    }
    extra = iter->second;
    return DM_OK;
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
    pinCode = std::stoi(importAuthCode_);
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
    timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
    softbusConnector_->JoinLnn(authRequestContext_->deviceId);
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
    std::string netWorkId = softbusConnector_->GetNetworkIdByUdidHash(deviceIdHash);
    int32_t sessionId = softbusConnector_->GetSoftbusSession()->OpenUnbindSession(netWorkId);
    if (sessionId < 0) {
        LOGE("OpenAuthSession failed, stop the syncdeleteacl.");
        authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        authResponseContext_->state = AuthState::AUTH_REQUEST_SYNCDELETE;
        authRequestContext_->reason = ERR_DM_AUTH_OPEN_SESSION_FAILED;
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
}

void DmAuthManager::SyncDeleteAclDone()
{
    LOGI("SyncDeleteAclDone start.");
    unBindFlag_ = false;
    if (authRequestState_ != nullptr) {
        if (authResponseContext_->reply == DM_OK || isFinishOfLocal_) {
            authMessageProcessor_->SetResponseContext(authResponseContext_);
            std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_SYNC_DELETE_DONE);
            softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        }
        listener_->OnAuthResult(authRequestContext_->hostPkgName, authRequestContext_->deviceId,
                                authRequestContext_->token, authResponseContext_->state, authRequestContext_->reason);
        usleep(USLEEP_TIME_MS); // 500ms
        softbusConnector_->GetSoftbusSession()->CloseAuthSession(authRequestContext_->sessionId);
        timer_->DeleteAll();
        isFinishOfLocal_ = true;
        authRequestContext_ = nullptr;
        authResponseContext_ = nullptr;
        authRequestState_ = nullptr;
        authMessageProcessor_ = nullptr;
    } else if (authResponseState_ != nullptr) {
        if (isFinishOfLocal_) {
            authMessageProcessor_->SetResponseContext(authResponseContext_);
            std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_SYNC_DELETE_DONE);
            softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        }
        timer_->DeleteAll();
        isFinishOfLocal_ = true;
        authResponseContext_ = nullptr;
        authResponseState_ = nullptr;
        authMessageProcessor_ = nullptr;
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
    timer_->DeleteTimer(std::string(SYNC_DELETE_TIMEOUT_TASK));
    authResponseContext_->reply = DM_OK;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_SYNC_DELETE_DONE);
    softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
}

bool DmAuthManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("DmAuthManager::onTransmit start.");
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %d is error.", requestId);
        return false;
    }
    std::string message = "";
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        LOGI("SoftbusSession send msgType %d.", MSG_TYPE_REQ_AUTH_DEVICE_NEGOTIATE);
        message = authMessageProcessor_->CreateDeviceAuthMessage(MSG_TYPE_REQ_AUTH_DEVICE_NEGOTIATE, data, dataLen);
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        LOGI("SoftbusSession send msgType %d.", MSG_TYPE_RESP_AUTH_DEVICE_NEGOTIATE);
        message = authMessageProcessor_->CreateDeviceAuthMessage(MSG_TYPE_RESP_AUTH_DEVICE_NEGOTIATE, data, dataLen);
    }
    if (softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message) != DM_OK) {
        LOGE("SoftbusSession send data failed.");
        return false;
    }
    return true;
}

void DmAuthManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("DmAuthManager::AuthDeviceFinish start.");
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %lld is error.", requestId);
        return;
    }
    timer_->DeleteTimer(std::string(AUTH_DEVICE_TIMEOUT_TASK));
    if (authRequestState_ != nullptr && authResponseState_ == nullptr) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_INPUT);
        if (authResponseContext_->isOnline) {
            LOGI("The device is online.");
            if (authResponseContext_->bindLevel == DM_APP && !authResponseContext_->isIdenticalAccount) {
                softbusConnector_->SetPkgName(authResponseContext_->hostPkgName);
            }
            softbusConnector_->HandleDeviceOnline(authRequestContext_->deviceId);
        }
        if (authResponseContext_->haveCredential) {
            softbusConnector_->JoinLnn(authRequestContext_->deviceId);
            timer_->DeleteTimer(std::string(AUTHENTICATE_TIMEOUT_TASK));
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        } else if (!authResponseContext_->haveCredential) {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestCredential>());
        }
    } else if (authRequestState_ == nullptr && authResponseState_ != nullptr) {
        authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_CANCEL_PIN_CODE_SHOW);
        if (authResponseContext_->isOnline) {
            LOGI("The device is online.");
            if (authResponseContext_->bindLevel == DM_APP && !authResponseContext_->isIdenticalAccount) {
                softbusConnector_->SetPkgName(authResponseContext_->hostPkgName);
            }
            softbusConnector_->HandleDeviceOnline(authResponseContext_->deviceId);
        }
    }
}

void DmAuthManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthDeviceError start.");
    if (authRequestState_ == nullptr || authResponseState_ != nullptr) {
        LOGD("AuthDeviceError sink return.");
        return;
    }
    authTimes_++;
    timer_->DeleteTimer(std::string(AUTH_DEVICE_TIMEOUT_TASK));
    if (errorCode != DM_OK || requestId != authResponseContext_->requestId) {
        if (authRequestState_ != nullptr && authTimes_ >= MAX_AUTH_TIMES) {
            authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
            authRequestContext_->reason = ERR_DM_INPUT_PARA_INVALID;
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
        } else {
            timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK), INPUT_TIMEOUT,
                [this] (std::string name) {
                    DmAuthManager::HandleAuthenticateTimeout(name);
                });
            authUiStateMgr_->UpdateUiState(DmUiStateMsg::MSG_PIN_CODE_ERROR);
        }
    }
}

void DmAuthManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("DmAuthManager::AuthDeviceSessionKey start.");
    if (requestId != authResponseContext_->requestId) {
        LOGE("DmAuthManager::onTransmit requestId %lld is error.", requestId);
        return;
    }
    sessionKey_ = sessionKey;
    sessionKeyLen_ = sessionKeyLen;
}

void DmAuthManager::GetRemoteDeviceId(std::string &deviceId)
{
    LOGI("GetRemoteDeviceId start.");
    deviceId = authResponseContext_->localDeviceId;
}
} // namespace DistributedHardware
} // namespace OHOS