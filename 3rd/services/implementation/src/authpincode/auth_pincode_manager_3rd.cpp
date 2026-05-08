/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "auth_pincode_manager_3rd.h"

#include <memory>

#include "app_manager_3rd.h"
#include "business_event.h"
#include "softbus_common.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "ipc_skeleton.h"

#include "dm_anonymous_3rd.h"
#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_message_processor_3rd.h"
#include "dm_auth_pincode_state_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "json_object.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {

AuthPincodeManager3rd::AuthPincodeManager3rd(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
{
    LOGI("AuthPincodeManager3rd constructor");
    context_ = std::make_shared<DmAuthPincodeContext>();
    context_->listener = listener;
    context_->softbusConnector = softbusConnector;
    context_->hiChainAuthConnector = hiChainAuthConnector;
    context_->accesser.dmVersion = DM_CURRENT_VERSION;
    context_->accessee.dmVersion = DM_CURRENT_VERSION;
    context_->timer = std::make_shared<DmTimer3rd>();
    context_->authPinMsgProc3rd = std::make_shared<DmAuthPincodeMessageProcessor3rd>();
}

AuthPincodeManager3rd::~AuthPincodeManager3rd()
{
    if (context_ != nullptr) {
        if (context_->authPinStateMac3rd != nullptr) {
            context_->authPinStateMac3rd->Stop();  // Stop statemMachine thread
        }
        if (context_->timer != nullptr) {
            context_->timer->DeleteAll();
        }
        LOGI("AuthPincodeManager3rd context variables destroy successful.");
    }
    LOGI("AuthPincodeManager3rd destructor");
}

int32_t AuthPincodeManager3rd::AuthPincode(const PeerTargetId3rd &targetId,
    const std::map<std::string, std::string> &authParam, int32_t sessionId, uint64_t logicalSessionId)
{
    LOGI("AuthPincode in, sessionId:%{public}d, logicalSessionId:%{public}" PRIu64 "", sessionId, logicalSessionId);
    CHECK_NULL_RETURN(context_, ERR_DM_POINT_NULL);
    std::string authParamStr = ConvertMapToJsonString(authParam);
    if (!authParam.empty() && authParamStr.empty()) {
        LOGE("AuthPincode failed: invalid authParam.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    GetAuthParam(targetId, authParam);
    CHECK_NULL_RETURN(context_->authPinStateMac3rd, ERR_DM_POINT_NULL);
    context_->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcStartState>());
    context_->sessionId = sessionId;
    context_->logicalSessionId = logicalSessionId;
    context_->requestId = static_cast<int64_t>(logicalSessionId);
    context_->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcPinNegotiateStartState>());
    return DM_OK;
}

void AuthPincodeManager3rd::GetAuthParam(const PeerTargetId3rd &targetId,
    const std::map<std::string, std::string> &authParam)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    CHECK_NULL_VOID(context_);
    context_->accesser.deviceId = std::string(localDeviceId);

    context_->accessee.deviceId = targetId.deviceId;
    context_->accesser.userId = MultipleUserConnector3rd::GetFirstForegroundUserId();
    context_->accesser.accountId = MultipleUserConnector3rd::GetOhosAccountIdByUserId(context_->accesser.userId);
    if (authParam.find(TAG_BIND_CALLER_BIND_LEVEL) != authParam.end()) {
        context_->accesser.bindLevel = std::atoi(authParam.at(TAG_BIND_CALLER_BIND_LEVEL).c_str());
    }
    if (authParam.find(TAG_BIND_CALLER_UID) != authParam.end()) {
        context_->accesser.uid = static_cast<uint32_t>(std::atoi(authParam.at(TAG_BIND_CALLER_UID).c_str()));
    }
    if (authParam.find(TAG_BIND_CALLER_TOKENID) != authParam.end()) {
        context_->accesser.tokenId = static_cast<uint32_t>(std::atoi(authParam.at(TAG_BIND_CALLER_TOKENID).c_str()));
    }
    if (authParam.find(TAG_BIND_CALLER_PROCESSNAME) != authParam.end()) {
        context_->accesser.processName = authParam.at(TAG_BIND_CALLER_PROCESSNAME);
        context_->accessee.processName = authParam.at(TAG_BIND_CALLER_PROCESSNAME);
    }
    if (authParam.find(TAG_BUSINESS_NAME) != authParam.end()) {
        context_->accesser.businessName = authParam.at(TAG_BUSINESS_NAME);
        context_->accessee.businessName = authParam.at(TAG_BUSINESS_NAME);
    }
    if (authParam.find(TAG_PEER_PROCESS_NAME) != authParam.end()) {
        context_->accessee.processName = authParam.at(TAG_PEER_PROCESS_NAME);
    }
    if (authParam.find(TAG_PEER_BUSINESS_NAME) != authParam.end()) {
        context_->accessee.businessName = authParam.at(TAG_PEER_BUSINESS_NAME);
    }
    context_->processInfo.tokenId = context_->accesser.tokenId;
    context_->processInfo.uid = context_->accesser.uid;
    context_->processInfo.userId = context_->accesser.userId;
    context_->processInfo.processName = context_->accesser.processName;
    context_->processInfo.businessName = context_->accesser.businessName;
}

AuthPincodeSinkManager::AuthPincodeSinkManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
    : AuthPincodeManager3rd(softbusConnector, listener, hiChainAuthConnector)
{
    if (context_ != nullptr) {
        context_->direction = DM_AUTH_PINCODE_SINK;
        context_->authPinStateMac3rd = std::make_shared<DmAuthPincodeStateMachine3rd>(context_);
    }
}

void AuthPincodeSinkManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthPincodeSinkManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("OnSessionClosed sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authPinStateMac3rd);
    context_->reason = ERR_DM_SESSION_CLOSED;
    context_->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSinkFinishState>());
}

void AuthPincodeSinkManager::OnDataReceived(int32_t sessionId, const std::string &message)
{
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authPinMsgProc3rd);
    context_->sessionId = sessionId;
    LOGI("AuthPincodeSinkManager::OnDataReceived, sessionId:%{public}d", sessionId);
    int32_t ret = context_->authPinMsgProc3rd->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error, sessionId:%{public}d", sessionId);
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        CHECK_NULL_VOID(context_->authPinStateMac3rd);
        context_->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSinkFinishState>());
    }
    return;
}

bool AuthPincodeSinkManager::GetIsCryptoSupport()
{
    return false;
}

AuthPincodeSrcManager::AuthPincodeSrcManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
    : AuthPincodeManager3rd(softbusConnector, listener, hiChainAuthConnector)
{
    context_->direction = DM_AUTH_PINCODE_SOURCE;
    context_->authPinStateMac3rd = std::make_shared<DmAuthPincodeStateMachine3rd>(context_);
}

void AuthPincodeSrcManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthPincodeSrcManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("AuthPincodeSrcManager::OnSessionClosed sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    context_->reason = ERR_DM_SESSION_CLOSED;
    CHECK_NULL_VOID(context_->authPinStateMac3rd);
    context_->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcFinishState>());
}

void AuthPincodeSrcManager::OnSessionDisable()
{
    CHECK_NULL_VOID(context_);
    context_->sessionId = -1;
}

void AuthPincodeSrcManager::OnDataReceived(int32_t sessionId, const std::string &message)
{
    CHECK_NULL_VOID(context_);
    context_->sessionId = sessionId;
    CHECK_NULL_VOID(context_->authPinMsgProc3rd);
    int32_t ret = context_->authPinMsgProc3rd->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        CHECK_NULL_VOID(context_->authPinStateMac3rd);
        context_->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcFinishState>());
    }
}

bool AuthPincodeSrcManager::GetIsCryptoSupport()
{
    return false;
}

void AuthPincodeSrcManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthPincodeSrcManager::AuthDeviceError start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    CHECK_NULL_VOID(context_->authPinStateMac3rd);
    LOGI("AuthPincodeSrcManager::AuthDeviceError unexpected err.");
    context_->reason = (errorCode == ERR_DM_HICHAIN_PROOFMISMATCH ? ERR_DM_BIND_PIN_CODE_ERROR : errorCode);
    context_->authPinStateMac3rd->NotifyEventFinish(DmEventType::ON_FAIL);
}

void AuthPincodeSinkManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthPincodeSinkManager::AuthDeviceError start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    CHECK_NULL_VOID(context_->authPinStateMac3rd);
    auto curState = context_->authPinStateMac3rd->GetCurState();
    LOGI("AuthPincodeSinkManager::AuthDeviceError unexpected err.");
    context_->reason = (errorCode == ERR_DM_HICHAIN_PROOFMISMATCH ? ERR_DM_BIND_PIN_CODE_ERROR : errorCode);
    context_->authPinStateMac3rd->NotifyEventFinish(DmEventType::ON_FAIL);
}

bool AuthPincodeSrcManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthDeviceTransmit start, requestId:%{public}" PRId64, requestId);
    // check request id first
    CHECK_NULL_RETURN(context_, false);
    if (requestId != context_->requestId) {
        LOGE("AuthPincodeSrcManager::AuthDeviceTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }

    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    CHECK_NULL_RETURN(context_->authPinStateMac3rd, false);
    context_->authPinStateMac3rd->NotifyEventFinish(ON_TRANSMIT);
    LOGI("leave.");
    return true;
}

bool AuthPincodeSinkManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthDeviceTransmit start.");
    // check request id first
    CHECK_NULL_RETURN(context_, false);
    if (requestId != context_->requestId) {
        LOGE("AuthPincodeSinkManager::AuthDeviceTransmit requestId %{public}" PRId64" is error.", requestId);
        return false;
    }

    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    CHECK_NULL_RETURN(context_->authPinStateMac3rd, false);
    context_->authPinStateMac3rd->NotifyEventFinish(ON_TRANSMIT);
    LOGI("leave.");
    return true;
}

void AuthPincodeSrcManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthPincodeSrcManager::AuthDeviceFinish start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    CHECK_NULL_VOID(context_->authPinStateMac3rd);
    context_->authPinStateMac3rd->NotifyEventFinish(ON_FINISH);

    LOGI("AuthPincodeSrcManager::AuthDeviceFinish leave.");
}

void AuthPincodeSinkManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthPincodeSinkManager::AuthDeviceFinish start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    CHECK_NULL_VOID(context_->authPinStateMac3rd);
    context_->authPinStateMac3rd->NotifyEventFinish(ON_FINISH);
    LOGI("AuthPincodeSinkManager::AuthDeviceFinish leave.");
}

void AuthPincodeSrcManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (sessionKey == nullptr || context_ == nullptr || context_->authPinMsgProc3rd == nullptr ||
        context_->authPinStateMac3rd == nullptr) {
        LOGE("failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    context_->accesser.sessionKey = std::vector<unsigned char>(sessionKey, sessionKey + sessionKeyLen);
    context_->authPinStateMac3rd->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

void AuthPincodeSinkManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (sessionKey == nullptr || context_ == nullptr || context_->authPinMsgProc3rd == nullptr ||
        context_->authPinStateMac3rd == nullptr) {
        LOGE("failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    context_->accessee.sessionKey = std::vector<unsigned char>(sessionKey, sessionKey + sessionKeyLen);
    context_->authPinStateMac3rd->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

int32_t AuthPincodeManager3rd::GetPinCode(std::string &code)
{
    if (context_ == nullptr) {
        LOGE("AuthPincodeManager3rd failed to GetPinCode because context_ is nullptr");
        return ERR_DM_FAILED;
    }
    std::string pinCodeHash = GetAnonyString(Crypto3rd::Sha256(context_->importAuthCode));
    LOGI("GetPinCode pinCodeHash: %{public}s", GetAnonyString(pinCodeHash).c_str());
    code = context_->importAuthCode;
    return DM_OK;
}

int32_t AuthPincodeManager3rd::ImportAuthCodeAndUid(const std::string &businessName, const std::string &authCode,
    int32_t uid)
{
    if (authCode.empty() || businessName.empty()) {
        LOGE("ImportAuthCode failed, authCode or businessName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(context_, ERR_DM_POINT_NULL);
    context_->importAuthCode = authCode;
    context_->importPkgName = businessName;
    DmPincodeAccess &selfAccess =
        (context_->direction == DM_AUTH_PINCODE_SOURCE) ? context_->accesser : context_->accessee;
    selfAccess.uid = uid;
    LOGI("ok");
    return DM_OK;
}

char *AuthPincodeSrcManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("start");
    return nullptr;
}

char *AuthPincodeSinkManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("start");
    (void)requestId;
    (void)reqParams;
    JsonObject jsonObj;
    if (context_->importAuthCode == "") {
        jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_REJECTED;
    } else {
        jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
        jsonObj[FIELD_PIN_CODE] = context_->importAuthCode;
    }
    std::string pinCodeHash = GetAnonyString(Crypto3rd::Sha256(context_->importAuthCode));
    LOGI("AuthDeviceRequest pinCodeHash: %{public}s", pinCodeHash.c_str());
    jsonObj[FIELD_SERVICE_PKG_NAME] = std::string(DM_PKG_NAME);
    std::string jsonStr = jsonObj.Dump();
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}

void AuthPincodeManager3rd::RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback)
{
    CHECK_NULL_VOID(context_);
    context_->cleanNotifyCallback = cleanNotifyCallback;
    return;
}
}  // namespace DistributedHardware
}  // namespace OHOS