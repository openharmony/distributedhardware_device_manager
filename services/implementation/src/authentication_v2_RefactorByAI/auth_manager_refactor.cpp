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

#include <memory>
#include <cstring>

#include "auth_manager.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_random.h"
#include "dm_log.h"
#include "dm_timer.h"
#include "dm_anonymous.h"
#include "dm_auth_state_machine.h"
#include "dm_auth_context.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_state.h"
#include "json_object.h"

#ifdef SUPPORT_MSDP
#include "spatial_awareness_mgr_client.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {

constexpr int32_t MIN_PIN_CODE = 100000;
constexpr int32_t MAX_PIN_CODE = 999999;
constexpr int32_t PIN_CODE_MAX_LIFETIME_MS = 300000;
constexpr int32_t AUTH_CONTEXT_INVALID = -1;
constexpr int32_t AUTH_CONTEXT_VALID = 0;

void SecureClearString(std::string &str)
{
    if (!str.empty()) {
        volatile char *ptr = &str[0];
        size_t len = str.size();
        for (size_t i = 0; i < len; ++i) {
            ptr[i] = '\0';
        }
        str.clear();
    }
}

void SecureClearBuffer(uint8_t *buffer, uint32_t len)
{
    if (buffer != nullptr && len > 0) {
        volatile uint8_t *ptr = buffer;
        for (uint32_t i = 0; i < len; ++i) {
            ptr[i] = 0;
        }
    }
}

}

AuthManager::AuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                         std::shared_ptr<HiChainConnector> hiChainConnector,
                         std::shared_ptr<IDeviceManagerServiceListener> listener,
                         std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
{
    LOGI("constructor");
    context_ = std::make_shared<DmAuthContext>();
    context_->softbusConnector = softbusConnector;
    context_->listener = listener;
    context_->hiChainConnector = hiChainConnector;
    context_->hiChainAuthConnector = hiChainAuthConnector;
    context_->authUiStateMgr = std::make_shared<AuthUiStateManager>(context_->listener);
    context_->authenticationMap[AUTH_TYPE_PIN] = nullptr;
    context_->authenticationMap[AUTH_TYPE_IMPORT_AUTH_CODE] = nullptr;
    context_->timer = std::make_shared<DmTimer>();
    context_->authMessageProcessor = std::make_shared<DmAuthMessageProcessor>();
    context_->authContextState = AUTH_CONTEXT_INVALID;
}

AuthManager::~AuthManager()
{
    if (context_ != nullptr) {
        context_->successFinished = true;
        if (context_->authStateMachine != nullptr) {
            context_->authStateMachine->Stop();
        }
        if (context_->timer != nullptr) {
            context_->timer->DeleteAll();
        }
        SecureClearString(context_->pinCode);
        SecureClearString(context_->importAuthCode);
        if (context_->sessionKey != nullptr && context_->sessionKeyLen > 0) {
            SecureClearBuffer(context_->sessionKey, context_->sessionKeyLen);
            context_->sessionKeyLen = 0;
        }
        context_->authContextState = AUTH_CONTEXT_INVALID;
        LOGI("AuthManager context securely cleared.");
    }
    {
        std::lock_guard<ffrt::mutex> lock(bindParamMutex_);
        for (auto &pair : bindParam_) {
            SecureClearString(pair.second);
        }
        bindParam_.clear();
    }
    LOGI("destructor");
}

bool AuthManager::ValidateAuthContextState()
{
    CHECK_NULL_RETURN(context_, false);
    if (context_->authContextState == AUTH_CONTEXT_INVALID) {
        LOGE("Auth context state is invalid.");
        return false;
    }
    if (context_->authStateMachine == nullptr) {
        LOGE("Auth state machine is null.");
        return false;
    }
    return true;
}

bool AuthManager::ValidateAuthContextForOperation(int32_t expectedOperation)
{
    CHECK_NULL_RETURN(context_, false);
    if (context_->authContextState == AUTH_CONTEXT_INVALID) {
        LOGE("Auth context state is invalid for operation %{public}d.", expectedOperation);
        return false;
    }
    return true;
}

std::string AuthManager::GeneratePincode()
{
    LOGI("start");
    int32_t pinCode = GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
    CHECK_NULL_RETURN(context_, "");
    SecureClearString(context_->pinCode);
    context_->pinCode = std::to_string(pinCode);
    context_->pinCodeGeneratedTime = std::chrono::steady_clock::now();
    context_->pinCodeMaxLifetimeMs = PIN_CODE_MAX_LIFETIME_MS;
    std::string encryptedPin = Crypto::EncryptPin(context_->pinCode);
    LOGI("PIN generated with encrypted hash.");
    return encryptedPin;
}

bool AuthManager::IsPinCodeValid()
{
    CHECK_NULL_RETURN(context_, false);
    if (context_->pinCode.empty()) {
        LOGE("PIN code is empty.");
        return false;
    }
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - context_->pinCodeGeneratedTime).count();
    if (elapsed > context_->pinCodeMaxLifetimeMs) {
        LOGE("PIN code has expired.");
        SecureClearString(context_->pinCode);
        return false;
    }
    return true;
}

void AuthManager::ConsumePinCode()
{
    CHECK_NULL_VOID(context_);
    SecureClearString(context_->pinCode);
    context_->pinCodeGeneratedTime = std::chrono::steady_clock::time_point();
    LOGI("PIN code consumed and cleared.");
}

int32_t AuthManager::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (authCode.empty() || pkgName.empty()) {
        LOGE("authCode or pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(context_, ERR_DM_POINT_NULL);
    SecureClearString(context_->importAuthCode);
    context_->importAuthCode = authCode;
    context_->importPkgName = pkgName;
    SecureClearString(context_->pinCode);
    context_->pinCode = authCode;
    LOGI("ok");
    return DM_OK;
}

int32_t AuthManager::StopAuthenticateDevice(const std::string &pkgName)
{
    (void)pkgName;
    LOGI("start");
    CHECK_NULL_RETURN(context_, ERR_DM_POINT_NULL);
    context_->connDelayCloseTime = 0;
    context_->reason = STOP_BIND;
    CHECK_NULL_RETURN(context_->authStateMachine, ERR_DM_POINT_NULL);
    if (!ValidateAuthContextState()) {
        LOGE("Invalid auth context state for stop authenticate.");
        SecureClearString(context_->pinCode);
        SecureClearString(context_->importAuthCode);
        return ERR_DM_AUTH_FAILED;
    }
    if (context_->authStateMachine->IsWaitEvent()) {
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
        SecureClearString(context_->pinCode);
        SecureClearString(context_->importAuthCode);
        return DM_OK;
    }
    if (context_->direction == DM_AUTH_SOURCE) {
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
    } else {
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
    }
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    SecureClearString(context_->pinCode);
    SecureClearString(context_->importAuthCode);
    return DM_OK;
}

void AuthManager::OnScreenLocked()
{
    LOGI("start");
    CHECK_NULL_VOID(context_);
    if (DmAuthState::IsImportAuthCodeCompatibility(context_->authType)) {
        LOGI("authtype is: %{public}d, no need stop bind.", context_->authType);
        return;
    }
    context_->reason = ERR_DM_BIND_USER_CANCEL;
    SecureClearString(context_->pinCode);
    SecureClearString(context_->importAuthCode);
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
}

void AuthSrcManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authMessageProcessor == nullptr || context_->authStateMachine == nullptr) {
        LOGE("auth context not initial.");
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
        return;
    }
    if (!ValidateAuthContextState()) {
        LOGE("Invalid auth context state for session key.");
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
        return;
    }
    int32_t ret = context_->authMessageProcessor->SaveSessionKey(sessionKey, sessionKeyLen);
    if (ret != DM_OK) {
        LOGE("save session key error, ret: %{public}d", ret);
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
    }
    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
    LOGI("leave.");
}

void AuthManager::InitAuthState(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    CHECK_NULL_VOID(context_);
    context_->authContextState = AUTH_CONTEXT_VALID;
    auto iter = context_->authenticationMap.find(authType);
    if (iter != context_->authenticationMap.end()) {
        context_->authPtr = iter->second;
    }
    CHECK_NULL_VOID(context_->timer);
    context_->timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK),
        AUTHENTICATE_TIMEOUT,
        [this] (std::string name) {
            DmAuthState::HandleAuthenticateTimeout(context_, name);
        });
    GetAuthParam(pkgName, authType, deviceId, extra);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcStartState>());
}

AuthSinkManager::AuthSinkManager(std::shared_ptr<SoftbusConnector> softbusConnector,
    std::shared_ptr<HiChainConnector> hiChainConnector,
    std::shared_ptr<IDeviceManagerServiceListener> listener,
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
    : AuthManager(softbusConnector, hiChainConnector, listener, hiChainAuthConnector)
{
    context_->direction = DM_AUTH_SINK;
    context_->authStateMachine = std::make_shared<DmAuthStateMachine>(context_);
}

void AuthSinkManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    context_->reason = ERR_DM_SESSION_CLOSED;
    SecureClearString(context_->pinCode);
    SecureClearString(context_->importAuthCode);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
}

int32_t AuthSinkManager::OnUserOperation(int32_t action, const std::string &params)
{
    LOGI("action %{public}d.", action);
    if (context_ == nullptr || context_->authStateMachine == nullptr) {
        LOGE("Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }
    if (!ValidateAuthContextForOperation(action)) {
        LOGE("Invalid auth context for user operation.");
        return ERR_DM_AUTH_NOT_START;
    }
    switch (action) {
        case USER_OPERATION_TYPE_CANCEL_AUTH:
        case USER_OPERATION_TYPE_ALLOW_AUTH:
        case USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS:
            context_->confirmOperation = static_cast<UiAction>(action);
            context_->reply = USER_OPERATION_TYPE_ALLOW_AUTH;
            context_->userOperationParam = params;
            if (action == USER_OPERATION_TYPE_CANCEL_AUTH) {
                context_->reply = USER_OPERATION_TYPE_CANCEL_AUTH;
                SecureClearString(context_->pinCode);
            }
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_USER_OPERATION);
            if (!context_->businessId.empty()) {
                HandleBusinessEvents(context_->businessId, action);
            }
            break;
        case USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT:
            LOGI("USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT.");
            context_->confirmOperation = USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT;
            context_->reason = ERR_DM_PEER_CONFIRM_TIME_OUT;
            SecureClearString(context_->pinCode);
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
            if (!context_->businessId.empty()) {
                HandleBusinessEvents(context_->businessId, action);
            }
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY:
            LOGI("USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY.");
            context_->confirmOperation = USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY;
            context_->reason = ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY;
            SecureClearString(context_->pinCode);
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
            break;
        default:
            LOGE("this action id not support");
            break;
    }
    LOGI("leave.");
    return DM_OK;
}

AuthSrcManager::AuthSrcManager(std::shared_ptr<SoftbusConnector> softbusConnector,
    std::shared_ptr<HiChainConnector> hiChainConnector,
    std::shared_ptr<IDeviceManagerServiceListener> listener,
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
    : AuthManager(softbusConnector, hiChainConnector, listener, hiChainAuthConnector)
{
    context_->direction = DM_AUTH_SOURCE;
    context_->authStateMachine = std::make_shared<DmAuthStateMachine>(context_);
}

void AuthSrcManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    context_->reason = ERR_DM_SESSION_CLOSED;
    SecureClearString(context_->pinCode);
    SecureClearString(context_->importAuthCode);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
}

int32_t AuthSrcManager::OnUserOperation(int32_t action, const std::string &params)
{
    LOGI("start.");
    if (context_ == nullptr || context_->authStateMachine == nullptr) {
        LOGE("Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }
    if (!ValidateAuthContextForOperation(action)) {
        LOGE("Invalid auth context for user operation.");
        return ERR_DM_AUTH_NOT_START;
    }
    JsonObject paramJson;
    paramJson.Parse(params);
    std::string pinCode;
    switch (action) {
        case USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT:
            LOGE("AuthSrcManager OnUserOperation user cancel");
            context_->pinInputResult = USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT;
            context_->reason = ERR_DM_BIND_USER_CANCEL_ERROR;
            SecureClearString(context_->pinCode);
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
            break;
        case USER_OPERATION_TYPE_DONE_PINCODE_INPUT:
            LOGE("AuthSrcManager OnUserOperation user input done");
            context_->pinInputResult = USER_OPERATION_TYPE_DONE_PINCODE_INPUT;
            if (paramJson.IsDiscarded() || !IsString(paramJson, PIN_CODE_KEY)) {
                LOGE("AuthSrcManager OnUserOperation pinCode not found");
                return ERR_DM_INPUT_PARA_INVALID;
            }
            pinCode = paramJson[PIN_CODE_KEY].Get<std::string>();
            {
                if (!IsNumberString(pinCode)) {
                    LOGE("jsonStr error");
                    SecureClearString(pinCode);
                    return ERR_DM_INPUT_PARA_INVALID;
                }
                SecureClearString(context_->pinCode);
                context_->pinCode = pinCode;
                SecureClearString(pinCode);
            }
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_USER_OPERATION);
            break;
        default:
            LOGE("this action id not support");
            break;
    }
    LOGI("leave.");
    return DM_OK;
}

void AuthSrcManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId mismatch.");
        return;
    }
    CHECK_NULL_VOID(context_->authStateMachine);
    auto curState = context_->authStateMachine->GetCurState();
    if (curState == DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE ||
        curState == DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE ||
        curState == DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE) {
        LOGI("Auth pin err.");
        if (context_->authType == DmAuthType::AUTH_TYPE_PIN) {
            context_->inputPinAuthFailTimes++;
        }
        SecureClearString(context_->pinCode);
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_ERROR);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
    } else {
        LOGI("unexpected err.");
        context_->reason = (errorCode == ERR_DM_HICHAIN_PROOFMISMATCH ? ERR_DM_BIND_PIN_CODE_ERROR : errorCode);
        SecureClearString(context_->pinCode);
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    LOGI("leave.");
}

void AuthSinkManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("start.");
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    auto curState = context_->authStateMachine->GetCurState();
    if (curState == DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE ||
        curState == DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE) {
        LOGI("Auth pin err.");
        if (context_->authType == DmAuthType::AUTH_TYPE_PIN) {
            context_->inputPinAuthFailTimes++;
        }
        SecureClearString(context_->pinCode);
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_ERROR);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
    } else {
        LOGI("unexpected err.");
        context_->reason = errorCode;
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    LOGI("leave.");
}

void AuthSinkManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authMessageProcessor == nullptr || context_->authStateMachine == nullptr) {
        LOGE("auth context not initial.");
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
        return;
    }
    if (!ValidateAuthContextState()) {
        LOGE("Invalid auth context state for session key.");
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
        return;
    }
    int32_t ret = context_->authMessageProcessor->SaveSessionKey(sessionKey, sessionKeyLen);
    if (ret != DM_OK) {
        LOGE("save session key error, ret: %{public}d", ret);
        SecureClearBuffer(const_cast<uint8_t*>(sessionKey), sessionKeyLen);
    }
    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

char *AuthSinkManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("start");
    (void)requestId;
    (void)reqParams;
    JsonObject jsonObj;
    DmAuthStateType curState = context_->authStateMachine->GetCurState();
    if (curState == DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE ||
        curState == DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_DONE_STATE ||
        curState == DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_DONE_STATE) {
        std::string pinCode = "";
        if (GetPinCode(pinCode) == ERR_DM_FAILED || pinCode == "") {
            jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_REJECTED;
        } else {
            jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
            jsonObj[FIELD_PIN_CODE] = Crypto::EncryptPin(pinCode);
            SecureClearString(pinCode);
        }
        std::string pinCodeHash = GetAnonyString(Crypto::Sha256(pinCode));
        LOGI("pinCodeHash: %{public}s", pinCodeHash.c_str());
    } else if (curState == DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE) {
        jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
        if (context_->isOnline) {
            jsonObj[FIELD_CRED_ID] = context_->accessee.transmitCredentialId;
        } else if (!context_->isAppCredentialVerified) {
            jsonObj[FIELD_CRED_ID] = context_->accessee.transmitCredentialId;
        } else {
            jsonObj[FIELD_CRED_ID] = context_->accessee.lnnCredentialId;
        }
    }
    jsonObj[FIELD_SERVICE_PKG_NAME] = std::string(DM_PKG_NAME);
    std::string jsonStr = jsonObj.Dump();
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}

int32_t AuthManager::GetPinCode(std::string &code)
{
    if (context_ == nullptr) {
        LOGE("context_ is nullptr");
        return ERR_DM_FAILED;
    }
    if (!IsPinCodeValid()) {
        LOGE("PIN code is invalid or expired.");
        return ERR_DM_FAILED;
    }
    std::string pinCodeHash = GetAnonyString(Crypto::Sha256(context_->pinCode));
    LOGI("pinCodeHash: %{public}s", pinCodeHash.c_str());
    code = context_->pinCode;
    return DM_OK;
}

void AuthManager::DeleteTimer()
{
    if (context_ != nullptr) {
        context_->successFinished = true;
        context_->authStateMachine->Stop();
        context_->timer->DeleteAll();
        SecureClearString(context_->pinCode);
        SecureClearString(context_->importAuthCode);
        LOGI("AuthManager context deleteTimer successful.");
    }
    {
        std::lock_guard<ffrt::mutex> lock(bindParamMutex_);
        for (auto &pair : bindParam_) {
            SecureClearString(pair.second);
        }
        bindParam_.clear();
    }
    LOGI("end.");
}

}
}