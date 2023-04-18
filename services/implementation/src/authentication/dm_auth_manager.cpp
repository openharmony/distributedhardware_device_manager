/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "auth_message_processor.h"
#include "dm_ability_manager.h"
#include "dm_anonymous.h"
#include "dm_config_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
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
const int32_t DM_AUTH_TYPE_MAX = 4;
const int32_t DM_AUTH_TYPE_MIN = 1;
const int32_t AUTH_SESSION_SIDE_SERVER = 0;

constexpr const char* APP_NAME_KEY = "appName";
constexpr const char* APP_ICON_KEY = "appIcon";
constexpr const char* TARGET_PKG_NAME_KEY = "targetPkgName";
constexpr const char* APP_DESCRIPTION_KEY = "appDescription";
constexpr const char* CANCEL_DISPLAY_KEY = "cancelPinCodeDisplay";
constexpr const char* VERIFY_FAILED = "verifyFailed";

DmAuthManager::DmAuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                             std::shared_ptr<IDeviceManagerServiceListener> listener,
                             std::shared_ptr<HiChainConnector> hiChainConnector)
    : softbusConnector_(softbusConnector), hiChainConnector_(hiChainConnector), listener_(listener)
{
    LOGI("DmAuthManager constructor");
    DmConfigManager &dmConfigManager = DmConfigManager::GetInstance();
    dmConfigManager.GetAuthAdapter(authenticationMap_);
}

DmAuthManager::~DmAuthManager()
{
    LOGI("DmAuthManager destructor");
}

int32_t DmAuthManager::AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                                          const std::string &extra)
{
    LOGI("DmAuthManager::AuthenticateDevice start auth type %d", authType);
    if (authType < DM_AUTH_TYPE_MIN || authType > DM_AUTH_TYPE_MAX) {
        LOGE("AuthenticateDevice failed, authType is illegal");
        return ERR_DM_AUTH_FAILED;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DmAuthManager::AuthenticateDevice failed, pkgName is %s, deviceId is %s, extra is %s",
            pkgName.c_str(), GetAnonyString(deviceId).c_str(), extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::shared_ptr<IAuthentication> authentication = authenticationMap_[authType];
    if (listener_ == nullptr) {
        LOGE("DmAuthManager::AuthenticateDevice is empty nullptr");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authentication == nullptr) {
        LOGE("DmAuthManager::AuthenticateDevice authType %d not support.", authType);
        listener_->OnAuthResult(pkgName, deviceId, "", AuthState::AUTH_REQUEST_INIT, ERR_DM_UNSUPPORTED_AUTH_TYPE);
        return ERR_DM_UNSUPPORTED_AUTH_TYPE;
    }

    if (authRequestState_ != nullptr || authResponseState_ != nullptr) {
        LOGE("DmAuthManager::AuthenticateDevice %s is request authentication.", pkgName.c_str());
        listener_->OnAuthResult(pkgName, deviceId, "", AuthState::AUTH_REQUEST_INIT, ERR_DM_AUTH_BUSINESS_BUSY);
        return ERR_DM_AUTH_BUSINESS_BUSY;
    }

    if (!softbusConnector_->HaveDeviceInMap(deviceId)) {
        LOGE("AuthenticateDevice failed, the discoveryDeviceInfoMap_ not have this device");
        listener_->OnAuthResult(pkgName, deviceId, "", AuthState::AUTH_REQUEST_INIT, ERR_DM_INPUT_PARA_INVALID);
        return ERR_DM_INPUT_PARA_INVALID;
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
    authRequestContext_->deviceId = deviceId;
    nlohmann::json jsonObject = nlohmann::json::parse(extra, nullptr, false);
    if (!jsonObject.is_discarded()) {
        if (IsString(jsonObject, TARGET_PKG_NAME_KEY)) {
            authRequestContext_->targetPkgName = jsonObject[TARGET_PKG_NAME_KEY].get<std::string>();
        }
        if (IsString(jsonObject, APP_NAME_KEY)) {
            authRequestContext_->appName = jsonObject[APP_NAME_KEY].get<std::string>();
        }
        if (IsString(jsonObject, APP_DESCRIPTION_KEY)) {
            authRequestContext_->appDesc = jsonObject[APP_DESCRIPTION_KEY].get<std::string>();
        }
        if (IsString(jsonObject, APP_THUMBNAIL)) {
            authRequestContext_->appThumbnail = jsonObject[APP_THUMBNAIL].get<std::string>();
        }
        if (IsString(jsonObject, APP_ICON_KEY)) {
            authRequestContext_->appIcon = jsonObject[APP_ICON_KEY].get<std::string>();
        }
    }
    authRequestContext_->token = std::to_string(GenRandInt(MIN_PIN_TOKEN, MAX_PIN_TOKEN));
    authMessageProcessor_->SetRequestContext(authRequestContext_);
    authRequestState_ = std::make_shared<AuthRequestInitState>();
    authRequestState_->SetAuthManager(shared_from_this());
    authRequestState_->SetAuthContext(authRequestContext_);
    authRequestState_->Enter();
    LOGI("DmAuthManager::AuthenticateDevice complete");
    return DM_OK;
}

int32_t DmAuthManager::UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::string deviceUdid;
    int32_t ret = SoftbusConnector::GetUdidByNetworkId(deviceId.c_str(), deviceUdid);
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }

    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    hiChainConnector_->GetRelatedGroups(deviceUdid, groupList);
    if (groupList.size() > 0) {
        std::string groupId = "";
        groupId = groupList.front().groupId;
        LOGI("DmAuthManager::UnAuthenticateDevice groupId = %s, deviceId = %s, deviceUdid = %s",
            GetAnonyString(groupId).c_str(), GetAnonyString(deviceId).c_str(), GetAnonyString(deviceUdid).c_str());
        hiChainConnector_->DeleteGroup(groupId);
    } else {
        LOGE("DmAuthManager::UnAuthenticateDevice groupList.size = 0");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DmAuthManager::VerifyAuthentication(const std::string &authParam)
{
    LOGI("DmAuthManager::VerifyAuthentication");
    if (authResponseContext_ == nullptr) {
        LOGE("authResponseContext_ is not init");
        return ERR_DM_AUTH_NOT_START;
    }
    timer_->DeleteTimer(std::string(INPUT_TIMEOUT_TASK));
    int32_t ret = authPtr_->VerifyAuthentication(authResponseContext_->authToken, authParam);
    switch (ret) {
        case DM_OK:
            authRequestState_->TransitionTo(std::make_shared<AuthRequestJoinState>());
            break;
        case ERR_DM_INPUT_PARA_INVALID:
            listener_->OnVerifyAuthResult(authRequestContext_->hostPkgName, authRequestContext_->deviceId,
                                          ERR_DM_INPUT_PARA_INVALID, "");
            break;
        default:
            authRequestContext_->reason = ERR_DM_INPUT_PARA_INVALID;
            authResponseContext_->state = authRequestState_->GetStateType();
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            break;
    }
    LOGI("DmAuthManager::VerifyAuthentication complete");
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
            timer_ = std::make_shared<DmTimer>();
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
        }
    } else {
        if (authResponseState_ == nullptr && authRequestState_ != nullptr &&
            authRequestState_->GetStateType() == AuthState::AUTH_REQUEST_INIT) {
            authRequestContext_->sessionId = sessionId;
            authRequestState_->SetAuthContext(authRequestContext_);
            authMessageProcessor_->SetRequestContext(authRequestContext_);
            authRequestState_->TransitionTo(std::make_shared<AuthRequestNegotiateState>());
        } else {
            LOGE("DmAuthManager::OnSessionOpened but request state is wrong");
        }
    }
}

void DmAuthManager::OnSessionClosed(const int32_t sessionId)
{
    LOGI("DmAuthManager::OnSessionOpened sessionId = %d", sessionId);
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
    } else if ((authResponseState_ != nullptr) && (authRequestState_ == nullptr)) {
        // sink device auth process
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

    int32_t pinCode = GeneratePincode();
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
    if (authResponseContext_ == nullptr) {
        LOGE("failed to OnMemberJoin because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager OnMemberJoin start authTimes %d", authTimes_);
    if ((authRequestState_ != nullptr) && (authResponseState_ == nullptr)) {
        authTimes_++;
        timer_->DeleteTimer(std::string(ADD_TIMEOUT_TASK));
        if (status != DM_OK || authResponseContext_->requestId != requestId) {
            if (authRequestState_ != nullptr && authTimes_ >= MAX_AUTH_TIMES) {
                authResponseContext_->state = AuthState::AUTH_REQUEST_JOIN;
                authRequestContext_->reason = ERR_DM_INPUT_PARA_INVALID;
                authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            } else {
                timer_->StartTimer(std::string(INPUT_TIMEOUT_TASK), INPUT_TIMEOUT,
                    [this] (std::string name) {
                        DmAuthManager::HandleAuthenticateTimeout(name);
                    });
                UpdateInputDialogDisplay(true);
            }
        } else {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestNetworkState>());
        }
    } else if ((authResponseState_ != nullptr) && (authRequestState_ == nullptr)) {
        if (status == DM_OK && authResponseContext_->requestId == requestId &&
            authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_SHOW) {
            UpdateInputDialogDisplay(false);
        }
    } else {
        LOGE("DmAuthManager::OnMemberJoin failed, authRequestState_ or authResponseState_ is invalid.");
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
    if (sessionId < 0) {
        LOGE("OpenAuthSession failed, stop the authentication");
        authResponseContext_ = std::make_shared<DmAuthResponseContext>();
        authResponseContext_->state = AuthState::AUTH_REQUEST_NEGOTIATE;
        if (authRequestContext_ != nullptr) {
            authRequestContext_->reason = ERR_DM_AUTH_OPEN_SESSION_FAILED;
        }
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
    LOGI("DmAuthManager::StartNegotiate session id is %d", sessionId);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    authRequestContext_->localDeviceId = localDeviceId;
    authResponseContext_->localDeviceId = localDeviceId;
    authResponseContext_->reply = ERR_DM_AUTH_REJECT;
    authResponseContext_->authType = authRequestContext_->authType;
    authResponseContext_->deviceId = authRequestContext_->deviceId;
    authMessageProcessor_->SetResponseContext(authResponseContext_);
    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_NEGOTIATE);
    softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
    timer_->StartTimer(std::string(NEGOTIATE_TIMEOUT_TASK), NEGOTIATE_TIMEOUT,
        [this] (std::string name) {
            DmAuthManager::HandleAuthenticateTimeout(name);
        });
}

void DmAuthManager::RespNegotiate(const int32_t &sessionId)
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to RespNegotiate because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::EstablishAuthChannel session id is %d", sessionId);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    bool ret = hiChainConnector_->IsDevicesInGroup(authResponseContext_->localDeviceId, localDeviceId);
    if (ret) {
        LOGE("DmAuthManager::EstablishAuthChannel device is in group");
        authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
    } else {
        authResponseContext_->reply = ERR_DM_AUTH_REJECT;
    }
    authResponseContext_->localDeviceId = localDeviceId;

    std::shared_ptr<IAuthentication> authentication = authenticationMap_[authResponseContext_->authType];
    if (authentication == nullptr) {
        LOGE("DmAuthManager::AuthenticateDevice authType %d not support.", authResponseContext_->authType);
        authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    } else {
        authPtr_ = authenticationMap_[authResponseContext_->authType];
    }

    std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_RESP_NEGOTIATE);
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        softbusConnector_->GetSoftbusSession()->SendData(sessionId, message);
    }

    if (IsIdenticalAccount()) {
        jsonObject[TAG_IDENTICAL_ACCOUNT] = true;
    }
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
    LOGI("DmAuthManager::EstablishAuthChannel session id");
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
        if (hiChainConnector_->IsDevicesInGroup(authResponseContext_->localDeviceId,
                                                authRequestContext_->localDeviceId)) {
            authRequestState_->TransitionTo(std::make_shared<AuthRequestFinishState>());
            return;
        }
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
    authResponseContext_->reply = action;
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
    int32_t ret = hiChainConnector_->AddMember(authRequestContext_->deviceId, connectInfo);
    if (ret != 0) {
        LOGE("DmAuthManager::AddMember failed, ret: %d", ret);
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
    if (authResponseContext_ == nullptr) {
        LOGE("failed to AuthenticateFinish because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::AuthenticateFinish start");
    if (authResponseState_ != nullptr) {
        if (authResponseState_->GetStateType() == AuthState::AUTH_RESPONSE_FINISH && authPtr_ != nullptr) {
            UpdateInputDialogDisplay(false);
        }
        if (isFinishOfLocal_) {
            authMessageProcessor_->SetResponseContext(authResponseContext_);
            std::string message = authMessageProcessor_->CreateSimpleMessage(MSG_TYPE_REQ_AUTH_TERMINATE);
            softbusConnector_->GetSoftbusSession()->SendData(authResponseContext_->sessionId, message);
        }
        timer_->DeleteAll();
        isFinishOfLocal_ = true;
        authResponseContext_ = nullptr;
        authResponseState_ = nullptr;
        authMessageProcessor_ = nullptr;
        authPtr_ = nullptr;
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
            UpdateInputDialogDisplay(false);
        }
        listener_->OnAuthResult(authRequestContext_->hostPkgName, authRequestContext_->deviceId,
                                authRequestContext_->token, authResponseContext_->state, authRequestContext_->reason);
        softbusConnector_->GetSoftbusSession()->CloseAuthSession(authRequestContext_->sessionId);
        timer_->DeleteAll();
        isFinishOfLocal_ = true;
        authRequestContext_ = nullptr;
        authResponseContext_ = nullptr;
        authRequestState_ = nullptr;
        authMessageProcessor_ = nullptr;
        authPtr_ = nullptr;
        authTimes_ = 0;
    }
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

void DmAuthManager::UpdateInputDialogDisplay(bool isShow)
{
    LOGI("DmAuthManager::UpdateInputDialogDisplay start");
    nlohmann::json jsonObj;
    jsonObj[VERIFY_FAILED] = isShow;
    jsonObj.dump();
    std::string paramJson = jsonObj.dump();
    std::string pkgName = "com.ohos.devicemanagerui";
    listener_->OnUiCall(pkgName, paramJson);
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
    std::string groupName = authResponseContext_->targetPkgName + sLocalDeviceId.substr(0, interceptLength)
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
    authRequestState_ = authRequestState;
    return DM_OK;
}

int32_t DmAuthManager::SetAuthResponseState(std::shared_ptr<AuthResponseState> authResponseState)
{
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
    LOGI("ShowConfigDialog start");
    dmAbilityMgr_ = std::make_shared<DmAbilityManager>();
    nlohmann::json jsonObj;
    jsonObj[TAG_AUTH_TYPE] = AUTH_TYPE_PIN;
    jsonObj[TAG_TOKEN] = authResponseContext_->token;
    jsonObj[TARGET_PKG_NAME_KEY] = authResponseContext_->targetPkgName;
    jsonObj.dump();
    const std::string params = jsonObj.dump();
    std::shared_ptr<ShowConfirm> showConfirm_ = std::make_shared<ShowConfirm>();
    showConfirm_->ShowConfirmDialog(params, shared_from_this(), dmAbilityMgr_);
    LOGI("ShowConfigDialog end");
}

void DmAuthManager::ShowAuthInfoDialog()
{
    if (authResponseContext_ == nullptr) {
        LOGE("failed to ShowAuthInfoDialog because authResponseContext_ is nullptr");
        return;
    }
    LOGI("DmAuthManager::ShowAuthInfoDialog start %d", authResponseContext_->code);
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
    LOGI("DmAuthManager::ShowStartAuthDialog start");
    authPtr_->StartAuth(authResponseContext_->authToken, shared_from_this());
}

int32_t DmAuthManager::GetAuthenticationParam(DmAuthParam &authParam)
{
    return DM_OK;
}

int32_t DmAuthManager::OnUserOperation(int32_t action, const std::string &params)
{
    if (authResponseContext_ == nullptr) {
        LOGE("Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }

    switch (action) {
        case USER_OPERATION_TYPE_ALLOW_AUTH:
        case USER_OPERATION_TYPE_CANCEL_AUTH:
            StartAuthProcess(action);
            break;
        case USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT:
            AuthenticateFinish();
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY:
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT:
            SetReasonAndFinish(ERR_DM_INPUT_PARA_INVALID, AuthState::AUTH_REQUEST_JOIN);
            break;
        case USER_OPERATION_TYPE_DONE_PINCODE_INPUT:
            AddMember(std::stoi(params));
            break;
        default:
            LOGE("this action id not support");
            break;
    }
    return DM_OK;
}

void DmAuthManager::UserSwitchEventCallback (int32_t userId)
{
    LOGI("switch user event happen and this user groups will be deleted with userId: %d", userId);
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    std::string queryParams = jsonObj.dump();
    std::vector<GroupInfo> groupList;

    int32_t oldUserId = MultipleUserConnector::GetSwitchOldUserId();
    MultipleUserConnector::SetSwitchOldUserId(userId);
    if (!hiChainConnector_->GetGroupInfo(oldUserId, queryParams, groupList)) {
        LOGE("failed to get device join groups");
        return;
    }
    for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
        int32_t ret = hiChainConnector_->DeleteGroup(oldUserId, iter->groupId);
        if (ret != DM_OK) {
            LOGE("fail to delete group");
        }
    }

    if (!hiChainConnector_->GetGroupInfo(userId, queryParams, groupList)) {
        LOGE("failed to get device join groups");
        return;
    }
    for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
        int32_t ret = hiChainConnector_->DeleteGroup(userId, iter->groupId);
        if (ret != DM_OK) {
            LOGE("fail to delete group");
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
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS