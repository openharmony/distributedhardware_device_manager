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
#include "deviceprofile_connector_3rd.h"
#include "ipc_skeleton.h"
#include "multiple_user_connector_3rd.h"

#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_state_machine_3rd.h"
#include "dm_auth_context_3rd.h"
#include "dm_auth_message_processor_3rd.h"
#include "dm_auth_state_3rd.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
namespace {

}  // namespace

AuthManager3rd::AuthManager3rd(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
{
    LOGI("AuthManager3rd constructor");
    context_ = std::make_shared<DmAuthContext>();
    context_->listener = listener;
    context_->softbusConnector = softbusConnector;
    context_->hiChainAuthConnector = hiChainAuthConnector;
    context_->accesser.dmVersion = DM_CURRENT_VERSION;
    context_->accessee.dmVersion = DM_CURRENT_VERSION;
    context_->timer = std::make_shared<DmTimer3rd>();
    context_->authMessageProcessor = std::make_shared<DmAuthMessageProcessor3rd>();
}

AuthManager3rd::~AuthManager3rd()
{
    if (context_ != nullptr) {
        if (context_->authStateMachine != nullptr) {
            context_->authStateMachine->Stop();  // Stop statemMachine thread
        }
        if (context_->timer != nullptr) {
            context_->timer->DeleteAll();
        }
        LOGI("AuthManager3rd context variables destroy successful.");
    }
    LOGI("AuthManager3rd destructor");
}

int32_t AuthManager3rd::AuthDevice3rd(const PeerTargetId3rd &targetId,
    const std::map<std::string, std::string> &authParam, int32_t sessionId, uint64_t logicalSessionId)
{
    LOGI("AuthManager3rd::AuthDevice3rd start");
    CHECK_NULL_RETURN(context_, ERR_DM_POINT_NULL);
    std::string authParamStr = ConvertMapToJsonString(authParam);
    if (!authParam.empty() && authParamStr.empty()) {
        LOGE("AuthDevice3rd failed: invalid authParam.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    GetAuthParam(targetId, authParam);
    CHECK_NULL_RETURN(context_->authStateMachine, ERR_DM_POINT_NULL);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcStartState>());
    context_->sessionId = sessionId;
    context_->logicalSessionId = logicalSessionId;
    context_->requestId = static_cast<int64_t>(logicalSessionId);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
    return DM_OK;
}

void AuthManager3rd::GetAuthParam(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam)
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
    ParseProxyParam(authParam);
}

void AuthManager3rd::ParseProxyParam(const std::map<std::string, std::string> &authParam)
{
    if (!IsProxyBindEnabled(authParam)) {
        return;
    }
    SetProxyBindFlags(authParam);
    ParseSubjectProxyedSubjects(authParam);
}

bool AuthManager3rd::IsProxyBindEnabled(const std::map<std::string, std::string> &authParam)
{
    return context_ != nullptr && authParam.find(PARAM_KEY_IS_PROXY_BIND) != authParam.end() &&
        authParam.at(PARAM_KEY_IS_PROXY_BIND) == DM_VAL_TRUE;
}

void AuthManager3rd::SetProxyBindFlags(const std::map<std::string, std::string> &authParam)
{
    context_->IsProxyBind = true;
    if (authParam.find(PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT) != authParam.end() &&
        authParam.at(PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT) == DM_VAL_FALSE) {
        context_->IsCallingProxyAsSubject = false;
    }
}

void AuthManager3rd::ParseSubjectProxyedSubjects(const std::map<std::string, std::string> &authParam)
{
    if (authParam.find(PARAM_KEY_SUBJECT_PROXYED_SUBJECTS) == authParam.end()) {
        LOGE("no subject proxyed apps");
        return;
    }
    std::string subjectProxyOnesStr = authParam.at(PARAM_KEY_SUBJECT_PROXYED_SUBJECTS);
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    if (allProxyObj.IsDiscarded()) {
        LOGI("subjectProxyOnesStr error");
        return;
    }
    for (auto &item : allProxyObj.Items()) {
        ParseProxyItem(item);
    }
}

void AuthManager3rd::ParseProxyItem(JsonItemObject &item)
{
    CHECK_NULL_VOID(context_);
    if (!ValidateProxyItem(item)) {
        return;
    }
    std::string processName = item[TAG_PROCESS_NAME].Get<std::string>();
    if (context_->accesser.processName == processName) {
        LOGE("proxy processName same as caller processName");
        return;
    }
    std::string peerProcessName = GetPeerProcessName(item, processName);
    uint32_t tokenId = item[TAG_TOKEN_ID].Get<uint32_t>();
    DmProxyAuthContext proxyAuthContext = CreateProxyAuthContext(item, processName, peerProcessName);
    AddProxyContextIfNotExists(proxyAuthContext, processName, peerProcessName, tokenId);
}

bool AuthManager3rd::ValidateProxyItem(const JsonItemObject &item)
{
    if (!IsString(item, TAG_PROCESS_NAME)) {
        LOGE("processName invalid");
        return false;
    }
    if (!IsInt64(item, TAG_TOKEN_ID)) {
        LOGE("tokenId invalid");
        return false;
    }
    return true;
}

std::string AuthManager3rd::GetPeerProcessName(const JsonItemObject &item, const std::string &defaultProcessName)
{
    if (item.Contains(TAG_PEER_PROCESS_NAME) && IsString(item, TAG_PEER_PROCESS_NAME)) {
        return item[TAG_PEER_PROCESS_NAME].Get<std::string>();
    }
    return defaultProcessName;
}

DmProxyAuthContext AuthManager3rd::CreateProxyAuthContext(const JsonItemObject &item, const std::string &processName,
    const std::string &peerProcessName)
{
    DmProxyAuthContext proxyAuthContext;
    if (IsString(item, TAG_BUSINESS_NAME)) {
        proxyAuthContext.proxyAccesser.businessName = item[TAG_BUSINESS_NAME].Get<std::string>();
        proxyAuthContext.proxyAccessee.businessName = proxyAuthContext.proxyAccesser.businessName;
    }
    if (IsString(item, TAG_PEER_BUSINESS_NAME)) {
        proxyAuthContext.proxyAccessee.businessName = item[TAG_PEER_BUSINESS_NAME].Get<std::string>();
    }
    proxyAuthContext.proxyContextId = Crypto3rd::Sha256(processName + peerProcessName);
    return proxyAuthContext;
}

void AuthManager3rd::AddProxyContextIfNotExists(DmProxyAuthContext &proxyAuthContext, const std::string &processName,
    const std::string &peerProcessName, uint32_t tokenId)
{
    CHECK_NULL_VOID(context_);
    if (std::find(context_->subjectProxyOnes.begin(), context_->subjectProxyOnes.end(), proxyAuthContext) !=
        context_->subjectProxyOnes.end()) {
        return;
    }
    proxyAuthContext.proxyAccesser.processName = processName;
    proxyAuthContext.proxyAccesser.tokenId = tokenId;
    proxyAuthContext.proxyAccesser.tokenIdHash =
        Crypto3rd::GetTokenIdHash(std::to_string(proxyAuthContext.proxyAccesser.tokenId));
    proxyAuthContext.proxyAccessee.processName = peerProcessName;
    GetBindLevelByProcessName(processName, context_->accesser.userId, proxyAuthContext.proxyAccesser.bindLevel);
    context_->subjectProxyOnes.push_back(proxyAuthContext);
}

void AuthManager3rd::GetBindLevelByProcessName(const std::string &processName, int32_t userId, int32_t &bindLevel)
{
    uint32_t tokenId = 0;
    if (AppManager3rd::GetInstance().GetHapTokenIdByName(userId, processName, 0, tokenId) == DM_OK) {
        bindLevel = DmRole::DM_ROLE_FA;
    } else if (AppManager3rd::GetInstance().GetNativeTokenIdByName(processName, tokenId) == DM_OK) {
        bindLevel = DmRole::DM_ROLE_SA;
    } else {
        bindLevel = DmRole::DM_ROLE_UNKNOWN;
        LOGE("src not contain the bundlename %{public}s.", processName.c_str());
    }
}

AuthSinkManager::AuthSinkManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
    : AuthManager3rd(softbusConnector, listener, hiChainAuthConnector)
{
    if (context_ != nullptr) {
        context_->direction = DM_AUTH_SINK;
        context_->authStateMachine = std::make_shared<DmAuthStateMachine3rd>(context_);
    }
}

void AuthSinkManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthSinkManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("OnSessionClosed sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->reason = ERR_DM_SESSION_CLOSED;
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
}

void AuthSinkManager::OnDataReceived(int32_t sessionId, const std::string &message)
{
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authMessageProcessor);
    context_->sessionId = sessionId;
    int32_t ret = context_->authMessageProcessor->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        CHECK_NULL_VOID(context_->authStateMachine);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
    }
    return;
}

bool AuthSinkManager::GetIsCryptoSupport()
{
    return false;
}

AuthSrcManager::AuthSrcManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector)
    : AuthManager3rd(softbusConnector, listener, hiChainAuthConnector)
{
    context_->direction = DM_AUTH_SOURCE;
    context_->authStateMachine = std::make_shared<DmAuthStateMachine3rd>(context_);
}

void AuthSrcManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthSrcManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("AuthSrcManager::OnSessionClosed sessionId = %{public}d", sessionId);
    CHECK_NULL_VOID(context_);
    context_->reason = ERR_DM_SESSION_CLOSED;
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
}

void AuthSrcManager::OnSessionDisable()
{
    CHECK_NULL_VOID(context_);
    context_->sessionId = -1;
}

void AuthSrcManager::OnDataReceived(int32_t sessionId, const std::string &message)
{
    CHECK_NULL_VOID(context_);
    context_->sessionId = sessionId;
    CHECK_NULL_VOID(context_->authMessageProcessor);
    int32_t ret = context_->authMessageProcessor->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        CHECK_NULL_VOID(context_->authStateMachine);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
    }

    return;
}

bool AuthSrcManager::GetIsCryptoSupport()
{
    return false;
}

void AuthSrcManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthSrcManager::AuthDeviceError start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    CHECK_NULL_VOID(context_->authStateMachine);
    LOGI("AuthSrcManager::AuthDeviceError unexpected err.");
    context_->reason = (errorCode == ERR_DM_HICHAIN_PROOFMISMATCH ? ERR_DM_BIND_PIN_CODE_ERROR : errorCode);
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
}

void AuthSinkManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthSinkManager::AuthDeviceError start.");
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    auto curState = context_->authStateMachine->GetCurState();
    LOGI("AuthSinkManager::AuthDeviceError unexpected err.");
    context_->reason = errorCode;
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
}

bool AuthSrcManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthSrcManager::AuthDeviceTransmit start.");
    // check request id first
    CHECK_NULL_RETURN(context_, false);
    if (requestId != context_->requestId) {
        LOGE("AuthSrcManager::onTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }

    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    CHECK_NULL_RETURN(context_->authStateMachine, false);
    context_->authStateMachine->NotifyEventFinish(ON_TRANSMIT);
    LOGI("AuthSrcManager::AuthDeviceTransmit leave.");
    return true;
}

bool AuthSinkManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthSinkManager::AuthDeviceTransmit start.");
    // check request id first
    CHECK_NULL_RETURN(context_, false);
    if (requestId != context_->requestId) {
        LOGE("AuthSinkManager::onTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }

    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    CHECK_NULL_RETURN(context_->authStateMachine, false);
    context_->authStateMachine->NotifyEventFinish(ON_TRANSMIT);
    LOGI("AuthSinkManager::AuthDeviceTransmit leave.");
    return true;
}

void AuthSrcManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthSrcManager::AuthDeviceFinish start.");
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->NotifyEventFinish(ON_FINISH);
    // Perform business processing based on the current state
    DmAuthStateType curState = context_->authStateMachine->GetCurState();
    switch (curState) {
        case DmAuthStateType::ACL_AUTH_SRC_PIN_AUTH_DONE_STATE:
            context_->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_REQ_DATA_SYNC, context_);
            break;
        default:
            LOGE("curState: %{public}d", curState);
            break;
    }
    LOGI("AuthSrcManager::AuthDeviceFinish leave.");
}

void AuthSinkManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthSinkManager::AuthDeviceFinish start.");
    CHECK_NULL_VOID(context_);
    CHECK_NULL_VOID(context_->authStateMachine);
    context_->authStateMachine->NotifyEventFinish(ON_FINISH);
    LOGI("AuthSinkManager::AuthDeviceFinish leave.");
}

void AuthSrcManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authMessageProcessor == nullptr || context_->authStateMachine == nullptr) {
        LOGE("failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    int32_t ret = context_->authMessageProcessor->SaveSessionKey(sessionKey, sessionKeyLen);
    if (ret != DM_OK) {
        LOGE("save session key error, ret: %{public}d", ret);
    }

    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

void AuthSinkManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authMessageProcessor == nullptr || context_->authStateMachine == nullptr) {
        LOGE("failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    int32_t ret = context_->authMessageProcessor->SaveSessionKey(sessionKey, sessionKeyLen);
    if (ret != DM_OK) {
        LOGE("save session key error, ret: %{public}d", ret);
    }

    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

int32_t AuthManager3rd::GetPinCode(std::string &code)
{
    if (context_ == nullptr) {
        LOGE("AuthManager3rd failed to GetPinCode because context_ is nullptr");
        return ERR_DM_FAILED;
    }
    std::string pinCodeHash = GetAnonyString(Crypto3rd::Sha256(context_->importAuthCode));
    LOGI("GetPinCode pinCodeHash: %{public}s", pinCodeHash.c_str());
    code = context_->importAuthCode;
    return DM_OK;
}

int32_t AuthManager3rd::ImportAuthCodeAndUid(const std::string &businessName, const std::string &authCode,
    uint32_t uid)
{
    if (authCode.empty() || businessName.empty()) {
        LOGE("ImportAuthCode failed, authCode or businessName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(context_, ERR_DM_POINT_NULL);
    context_->importAuthCode = authCode;
    context_->importPkgName = businessName;
    DmAccess &selfAccess = (context_->direction == DM_AUTH_SOURCE) ? context_->accesser : context_->accessee;
    selfAccess.uid = uid;
    LOGI("AuthManager3rd::ImportAuthCode ok");
    return DM_OK;
}

char *AuthSrcManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("AuthSrcManager::AuthDeviceRequest start");
    return nullptr;
}

char *AuthSinkManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("AuthSinkManager::AuthDeviceRequest start");
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

void AuthManager3rd::RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback)
{
    CHECK_NULL_VOID(context_);
    context_->cleanNotifyCallback = cleanNotifyCallback;
    return;
}
}  // namespace DistributedHardware
}  // namespace OHOS