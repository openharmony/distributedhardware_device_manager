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

#include <memory>

#include "app_manager_3rd.h"
#include "business_event.h"
#include "distributed_device_profile_client.h"
#include "softbus_common.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "ipc_skeleton.h"
#include "multiple_user_connector_3rd.h"

#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_state_machine_cred.h"
#include "dm_auth_context_cred.h"
#include "dm_auth_message_processor_cred.h"
#include "dm_auth_state_cred.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
namespace {

}  // namespace

AuthManagerCred::AuthManagerCred(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
{
    LOGI("AuthManagerCred constructor");
    context_ = std::make_shared<DmAuthCredContext>();
    context_->listener = listener;
    context_->softbusConnector = softbusConnector;
    context_->hiChainAuthConnector = hiChainAuthConnector;
    context_->accesser.dmVersion = DM_CURRENT_VERSION;
    context_->accessee.dmVersion = DM_CURRENT_VERSION;
    context_->timer = std::make_shared<DmTimer3rd>();
    context_->authMessageProcessor = std::make_shared<DmAuthMessageProcessorCred>();
}

AuthManagerCred::~AuthManagerCred()
{
    if (context_ != nullptr) {
        if (context_->authStateMachine != nullptr) {
            context_->authStateMachine->Stop();  // Stop statemMachine thread
        }
        if (context_->timer != nullptr) {
            context_->timer->DeleteAll();
        }
        LOGI("AuthManagerCred context variables destroy successful.");
    }
    LOGI("AuthManagerCred destructor");
}

int32_t AuthManagerCred::AuthDevice3rd(const PeerTargetId3rd &targetId,
    const std::map<std::string, std::string> &authParam, int32_t sessionId, uint64_t logicalSessionId)
{
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t AuthManagerCred::AuthCredential(const PeerTargetId3rd &targetId,
    const std::map<std::string, std::string> &authParam, int32_t sessionId, uint64_t logicalSessionId)
{
    LOGI("AuthManagerCred::AuthCredential start");
    CHECK_NULL_RETURN(context_, ERR_DM_POINT_NULL);
    std::string authParamStr = ConvertMapToJsonString(authParam);
    if (!authParam.empty() && authParamStr.empty()) {
        LOGE("AuthCredential failed: invalid authParam.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    GetAuthParam(targetId, authParam);
    CHECK_NULL_RETURN(context_->authStateMachine, ERR_DM_POINT_NULL);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthCredSrcStartState>());
    context_->sessionId = sessionId;
    context_->logicalSessionId = logicalSessionId;
    context_->requestId = static_cast<int64_t>(logicalSessionId);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcNegotiateStartState>());
    return DM_OK;
}

void AuthManagerCred::GetAuthParam(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    CHECK_NULL_VOID(context_);
    context_->accesser.deviceId = std::string(localDeviceId);

    context_->accessee.deviceId = targetId.deviceId;
    context_->accesser.userId = MultipleUserConnector3rd::GetFirstForegroundUserId();
    context_->accesser.accountId = MultipleUserConnector3rd::GetOhosAccountIdByUserId(context_->accesser.userId);
    if (authParam.find(TAG_BIND_CALLER_UID) != authParam.end()) {
        context_->accesser.uid = static_cast<int32_t>(std::atoi(authParam.at(TAG_BIND_CALLER_UID).c_str()));
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
    if (authParam.find(TAG_OPENID_HASH) != authParam.end()) {
        context_->accesser.openIdHash = authParam.at(TAG_OPENID_HASH);
    }
    if (authParam.find(TAG_OWNER_ID) != authParam.end()) {
        context_->accesser.ownerId = authParam.at(TAG_OWNER_ID);
    }
    if (authParam.find(TAG_CRED_TYPE) != authParam.end()) {
        context_->accesser.credType = static_cast<uint8_t>(std::atoi(authParam.at(TAG_CRED_TYPE).c_str()));
    }
    context_->processInfo.tokenId = context_->accesser.tokenId;
    context_->processInfo.uid = context_->accesser.uid;
    context_->processInfo.userId = context_->accesser.userId;
    context_->processInfo.processName = context_->accesser.processName;
    context_->processInfo.businessName = context_->accesser.businessName;
}

std::string AuthManagerCred::GetPeerProcessName(const JsonItemObject &item, const std::string &defaultProcessName)
{
    if (item.Contains(TAG_PEER_PROCESS_NAME) && IsString(item, TAG_PEER_PROCESS_NAME)) {
        return item[TAG_PEER_PROCESS_NAME].Get<std::string>();
    }
    return defaultProcessName;
}

AuthSinkManagerCred::AuthSinkManagerCred(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
    : AuthManagerCred(softbusConnector, listener, hiChainAuthConnector)
{
    if (context_ != nullptr) {
        context_->direction = DM_AUTH_CRED_SINK;
        context_->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context_);
    }
}

void AuthSinkManagerCred::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthSinkManagerCred::OnSessionClosed(int32_t sessionId)
{
    LOGI("OnSessionClosed sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->reason = ERR_DM_SESSION_CLOSED;
    context_->authStateMachine->TransitionTo(std::make_shared<AuthCredSinkFinishState>());
}

void AuthSinkManagerCred::OnDataReceived(int32_t sessionId, const std::string &message)
{
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authMessageProcessor);
    context_->sessionId = sessionId;
    int32_t ret = context_->authMessageProcessor->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        CHECK_NULL_VOID(context_->authStateMachine);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthCredSinkFinishState>());
    }
    return;
}

bool AuthSinkManagerCred::GetIsCryptoSupport()
{
    return false;
}

AuthSrcManagerCred::AuthSrcManagerCred(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
    : AuthManagerCred(softbusConnector, listener, hiChainAuthConnector)
{
    if (context_ != nullptr) {
        context_->direction = DM_AUTH_CRED_SOURCE;
        context_->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context_);
    }
}

void AuthSrcManagerCred::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthSrcManagerCred::OnSessionClosed(int32_t sessionId)
{
    LOGI("AuthSrcManagerCred::OnSessionClosed sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    context_->reason = ERR_DM_SESSION_CLOSED;
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthCredSrcFinishState>());
}

void AuthSrcManagerCred::OnSessionDisable()
{
    CHECK_NULL_VOID(context_);
    context_->sessionId = -1;
}

void AuthSrcManagerCred::OnDataReceived(int32_t sessionId, const std::string &message)
{
    CHECK_NULL_VOID(context_);
    context_->sessionId = sessionId;
    CHECK_NULL_VOID(context_->authMessageProcessor);
    int32_t ret = context_->authMessageProcessor->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        CHECK_NULL_VOID(context_->authStateMachine);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthCredSrcFinishState>());
    }

    return;
}

bool AuthSrcManagerCred::GetIsCryptoSupport()
{
    return false;
}

void AuthSrcManagerCred::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthSrcManagerCred::AuthDeviceError start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    CHECK_NULL_VOID(context_->authStateMachine);
    LOGI("AuthSrcManagerCred::AuthDeviceError unexpected err.");
    context_->reason = (errorCode == ERR_DM_HICHAIN_PROOFMISMATCH ? ERR_DM_BIND_PIN_CODE_ERROR : errorCode);
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
}

void AuthSinkManagerCred::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthSinkManagerCred::AuthDeviceError start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    CHECK_NULL_VOID(context_->authStateMachine);
    auto curState = context_->authStateMachine->GetCurState();
    LOGI("AuthSinkManagerCred::AuthDeviceError unexpected err.");
    context_->reason = errorCode;
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
}

bool AuthSrcManagerCred::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthSrcManagerCred::AuthDeviceTransmit start.");
    // check request id first
    CHECK_NULL_RETURN(context_, false);
    if (requestId != context_->requestId) {
        LOGE("AuthSrcManagerCred::onTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }
    if (data == nullptr) {
        LOGE("data is null.");
        return false;
    }
    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    CHECK_NULL_RETURN(context_->authStateMachine, false);
    context_->authStateMachine->NotifyEventFinish(ON_TRANSMIT);
    LOGI("AuthSrcManagerCred::AuthDeviceTransmit leave.");
    return true;
}

bool AuthSinkManagerCred::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthSinkManagerCred::AuthDeviceTransmit start.");
    // check request id first
    CHECK_NULL_RETURN(context_, false);
    if (requestId != context_->requestId) {
        LOGE("AuthSinkManagerCred::onTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }

    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    CHECK_NULL_RETURN(context_->authStateMachine, false);
    context_->authStateMachine->NotifyEventFinish(ON_TRANSMIT);
    LOGI("AuthSinkManagerCred::AuthDeviceTransmit leave.");
    return true;
}

void AuthSrcManagerCred::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthSrcManagerCred::AuthDeviceFinish start.");
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->NotifyEventFinish(ON_FINISH);
    LOGI("AuthSrcManagerCred::AuthDeviceFinish leave.");
}

void AuthSinkManagerCred::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthSinkManagerCred::AuthDeviceFinish start.");
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->NotifyEventFinish(ON_FINISH);
    LOGI("AuthSinkManagerCred::AuthDeviceFinish leave.");
}

void AuthSrcManagerCred::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authStateMachine == nullptr) {
        LOGE("failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    if (sessionKey == nullptr) {
        LOGE("sessionKey is null.");
        return;
    }
    context_->accesser.sessionKey = std::vector<unsigned char>(sessionKey, sessionKey + sessionKeyLen);
    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

void AuthSinkManagerCred::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authStateMachine == nullptr) {
        LOGE("failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    if (sessionKey == nullptr) {
        LOGE("sessionKey is null.");
        return;
    }
    context_->accessee.sessionKey = std::vector<unsigned char>(sessionKey, sessionKey + sessionKeyLen);
    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

char *AuthSrcManagerCred::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("AuthSrcManagerCred::AuthDeviceRequest start");
    return nullptr;
}

char *AuthSinkManagerCred::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("AuthSinkManagerCred::AuthDeviceRequest start");
    (void)requestId;
    (void)reqParams;
    JsonObject jsonObj;
    jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
    jsonObj[FIELD_CRED_ID] = context_->accessee.transmitCredentialId;
    jsonObj[FIELD_SERVICE_PKG_NAME] = std::string(DM_PKG_NAME);
    std::string jsonStr = jsonObj.Dump();
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}

void AuthManagerCred::RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback)
{
    CHECK_NULL_VOID(context_);
    context_->cleanNotifyCallback = cleanNotifyCallback;
    return;
}
}  // namespace DistributedHardware
}  // namespace OHOS