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
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "dm_auth_context.h"
#include "dm_auth_manager_base.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_state.h"
#include "dm_auth_state_machine.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "deviceprofile_connector.h"
#include "hichain_auth_connector.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {

namespace {

// tag in Lowercase, need by hichain tag
constexpr const char* TAG_LOWER_DEVICE_ID = "deviceId";
constexpr const char* TAG_LOWER_USER_ID = "userId";

constexpr const char* DM_AUTH_CREDENTIAL_OWNER = "DM";

// decrypt process
int32_t g_authCredentialTransmitDecryptProcess(std::shared_ptr<DmAuthContext> context, DmEventType event)
{
    if (context->transmitData.empty()) {
        LOGE("DmAuthMessageProcessor::CreateMessageReqCredAuthStart failed, get onTransmitData failed.");
        return ERR_DM_FAILED;
    }

    int32_t ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthCredentialTransmitDecryptProcess: ProcessCredData transmit data failed");
        return ERR_DM_FAILED;
    }

    if (context->authStateMachine->WaitExpectEvent(event) != event) {
        LOGE("AuthCredentialTransmitDecryptProcess: Hichain auth transmit data failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t AuthCredentialTransmitSend(std::shared_ptr<DmAuthContext> context, DmMessageType msgType)
{
    if (context->transmitData.empty()) {
        LOGE("AuthCredentialTransmitSend: Get onTransmitData failed.");
        return ERR_DM_FAILED;
    }

    std::string message =
        context->authMessageProcessor->CreateMessage(msgType, context);
    if (message.empty()) {
        LOGE("AuthCredentialTransmitSend: CreateMessage AuthCredential transmit data failed");
        return ERR_DM_FAILED;
    }

    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}

void SetAuthContext(int32_t skId, int64_t &appSkTimeStamp, int32_t &appSessionKeyId)
{
    appSkTimeStamp =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    appSessionKeyId = skId;
    return;
}

}

DmAuthStateType AuthSrcCredentialAuthNegotiateState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE;
}

// Parse the ontransmit data, respond with 161 message
int32_t AuthSrcCredentialAuthNegotiateState::Action(std::shared_ptr<DmAuthContext> context)
{
    // decrypt and transmit transmitData
    int32_t ret = g_authCredentialTransmitDecryptProcess(context, ON_TRANSMIT);
    if (ret != DM_OK) {
        return ret;
    }

    // Send 161 message
    return AuthCredentialTransmitSend(context, DmMessageType::MSG_TYPE_REQ_CREDENTIAL_AUTH_NEGOTIATE);
}

DmAuthStateType AuthSrcCredentialAuthDoneState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_DONE_STATE;
}

int32_t AuthSrcCredentialAuthDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    // decrypt and transmit transmitData
    int32_t ret = g_authCredentialTransmitDecryptProcess(context, ON_SESSION_KEY_RETURNED);
    if (ret != DM_OK) {
        return ret;
    }

    // Authentication completion triggers the Onfinish callback event.
    if (context->authStateMachine->WaitExpectEvent(ON_FINISH) != ON_FINISH) {
        LOGE("AuthSrcCredentialAuthDoneState::Action Hichain auth SINK transmit data failed");
        return ERR_DM_FAILED;
    }
    DmMessageType msgType;
    int32_t skId;
    ret = context->authMessageProcessor->SaveSessionKeyToDP(context->accesser.userId, skId);
    if (ret != DM_OK) {
        LOGE("AuthSrcCredentialAuthDoneState::Action DP save user session key failed");
        return ret;
    }

    // first time joinLnn, auth lnnCredential
    if (context->accesser.isGenerateLnnCredential == true && context->isAppCredentialVerified == false &&
        context->accesser.bindLevel != USER) {
        context->isAppCredentialVerified = true;
        SetAuthContext(skId, context->accesser.transmitSkTimeStamp, context->accesser.transmitSessionKeyId);
        msgType = MSG_TYPE_REQ_CREDENTIAL_AUTH_START;
        ret = context->hiChainAuthConnector->AuthCredential(context->accesser.userId, context->requestId,
                                                            context->accesser.lnnCredentialId, std::string(""));
        if (ret != DM_OK) {
            LOGE("AuthSrcCredentialAuthDoneState::Action Hichain auth credentail failed");
            return ret;
        }

        // wait for onTransmit event
        if (context->authStateMachine->WaitExpectEvent(ON_TRANSMIT) != ON_TRANSMIT) {
            LOGE("AuthSrcCredentialAuthDoneState::Action failed, ON_TRANSMIT event not arrived.");
            return ERR_DM_FAILED;
        }
        // First-time authentication and Lnn credential process
    } else if (context->accesser.isGenerateLnnCredential == true && context->accesser.bindLevel != USER) {
        SetAuthContext(skId, context->accesser.lnnSkTimeStamp, context->accesser.lnnSessionKeyId);
        msgType = MSG_TYPE_REQ_DATA_SYNC;
    } else {  // Non-first-time authentication transport credential process
        SetAuthContext(skId, context->accesser.transmitSkTimeStamp, context->accesser.transmitSessionKeyId);
        msgType = MSG_TYPE_REQ_DATA_SYNC;
    }
    std::string message =
        context->authMessageProcessor->CreateMessage(msgType, context);
    if (message.empty()) {
        LOGE("AuthSrcCredentialAuthDoneState::Action CreateMessage failed");
        return ERR_DM_FAILED;
    }

    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}

DmAuthStateType AuthSinkCredentialAuthStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE;
}

int32_t AuthSinkCredentialAuthStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));

    int32_t ret = g_authCredentialTransmitDecryptProcess(context, ON_TRANSMIT);
    if (ret != DM_OK) {
        return ret;
    }

    return AuthCredentialTransmitSend(context, DmMessageType::MSG_TYPE_RESP_CREDENTIAL_AUTH_START);
}

DmAuthStateType AuthSinkCredentialAuthNegotiateState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE;
}

int32_t AuthSinkCredentialAuthNegotiateState::Action(std::shared_ptr<DmAuthContext> context)
{
    int32_t ret = g_authCredentialTransmitDecryptProcess(context, ON_TRANSMIT);
    if (ret != DM_OK) {
        return ret;
    }

    // Construct and send 171 message
    ret = AuthCredentialTransmitSend(context, DmMessageType::MSG_TYPE_RESP_CREDENTIAL_AUTH_NEGOTIATE);
    if (ret != DM_OK) {
        return ret;
    }

    if (context->authStateMachine->WaitExpectEvent(ON_SESSION_KEY_RETURNED) != ON_SESSION_KEY_RETURNED) {
        LOGE("AuthSinkCredentialAuthNegotiateState::Action Hichain auth SINK transmit data failed");
        return ERR_DM_FAILED;
    }

    if (context->authStateMachine->WaitExpectEvent(ON_FINISH) != ON_FINISH) {
        LOGE("AuthSinkCredentialAuthNegotiateState::Action Hichain auth SINK transmit data failed");
        return ERR_DM_FAILED;
    }
    int32_t skId;
    ret = context->authMessageProcessor->SaveSessionKeyToDP(context->accessee.userId, skId);
    if (ret != DM_OK) {
        LOGE("AuthSinkCredentialAuthNegotiateState::Action DP save user session key failed");
        return ret;
    }

    // First lnn cred auth, second time receiving 161 message
    if (context->accessee.isGenerateLnnCredential == true && context->accessee.bindLevel != USER &&
        context->isAppCredentialVerified == true) {
        context->accessee.lnnSkTimeStamp =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        context->accessee.lnnSessionKeyId = skId;
    } else {  // Twice transport cred auth
        context->isAppCredentialVerified = true;
        context->accessee.transmitSkTimeStamp =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        context->accessee.transmitSessionKeyId = skId;
    }
    return DM_OK;
}

// Generate the json string of authParams in the credential negotiation state
std::string AuthCredentialAgreeState::CreateAuthParamsString(DmAuthScope authorizedScope,
    DmAuthCredentialAddMethod method, const std::shared_ptr<DmAuthContext> &authContext)
{
    LOGI("AuthCredentialAgreeState::CreateAuthParamsString start.");

    if ((authorizedScope != DM_AUTH_SCOPE_USER && authorizedScope != DM_AUTH_SCOPE_APP) ||
        (method != DM_AUTH_CREDENTIAL_ADD_METHOD_GENERATE && method != DM_AUTH_CREDENTIAL_ADD_METHOD_IMPORT)) {
        return std::string("");
    }

    JsonObject jsonObj;
    if (method == DM_AUTH_CREDENTIAL_ADD_METHOD_GENERATE) {
        jsonObj[TAG_METHOD] = method;
    }

    jsonObj[TAG_LOWER_DEVICE_ID] = (method == DM_AUTH_CREDENTIAL_ADD_METHOD_GENERATE) ?
        authContext->GetDeviceId(DM_AUTH_LOCAL_SIDE) : authContext->GetDeviceId(DM_AUTH_REMOTE_SIDE);
    if (method == DM_AUTH_CREDENTIAL_ADD_METHOD_IMPORT) {
        jsonObj[TAG_PEER_USER_SPACE_ID] = std::to_string(authContext->GetUserId(DM_AUTH_REMOTE_SIDE));
    }
    jsonObj[TAG_LOWER_USER_ID] = (method == DM_AUTH_CREDENTIAL_ADD_METHOD_GENERATE) ?
        authContext->GetAccountId(DM_AUTH_LOCAL_SIDE) : authContext->GetAccountId(DM_AUTH_REMOTE_SIDE);
    jsonObj[TAG_SUBJECT] = DM_AUTH_CREDENTIAL_SUBJECT_PRIMARY;
    jsonObj[TAG_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED;
    jsonObj[TAG_KEY_FORMAT] = (method == DM_AUTH_CREDENTIAL_ADD_METHOD_GENERATE) ?
        DM_AUTH_KEY_FORMAT_ASYMM_GENERATE : DM_AUTH_KEY_FORMAT_ASYMM_IMPORT;
    jsonObj[TAG_ALGORITHM_TYPE] = DM_AUTH_ALG_TYPE_ED25519;
    jsonObj[TAG_PROOF_TYPE] = DM_AUTH_CREDENTIAL_PROOF_PSK;
    if (method == DM_AUTH_CREDENTIAL_ADD_METHOD_IMPORT) {
        jsonObj[TAG_KEY_VALUE] = authContext->GetPublicKey(DM_AUTH_REMOTE_SIDE, authorizedScope);
    }
    jsonObj[TAG_AUTHORIZED_SCOPE] = authorizedScope;
    if (authorizedScope == DM_AUTH_SCOPE_APP) {
        std::vector<std::string> tokenIds = {std::to_string(authContext->accesser.tokenId),
            std::to_string(authContext->accessee.tokenId)};
        jsonObj[TAG_AUTHORIZED_APP_LIST] = tokenIds;
    }
    jsonObj[TAG_CREDENTIAL_OWNER] = DM_AUTH_CREDENTIAL_OWNER;

    LOGI("AuthCredentialAgreeState::CreateAuthParamsString leave.");
    return jsonObj.Dump();
}

// Generate credential ID and public key
int32_t AuthCredentialAgreeState::GenerateCredIdAndPublicKey(DmAuthScope authorizedScope,
    std::shared_ptr<DmAuthContext> &authContext)
{
    LOGI("authorizedScope %{public}d.", static_cast<int32_t>(authorizedScope));
    if ((authorizedScope != DM_AUTH_SCOPE_USER && authorizedScope != DM_AUTH_SCOPE_APP) ||
        authContext == nullptr || authContext->hiChainAuthConnector == nullptr) {
        return ERR_DM_FAILED;
    }

    std::string authParamsString = CreateAuthParamsString(authorizedScope,
        DM_AUTH_CREDENTIAL_ADD_METHOD_GENERATE, authContext);
    if (authParamsString == "") {
        LOGE("AuthCredentialAgreeState::GenerateCredIdAndPublicKey() error, create authParamsString failed.");
        return ERR_DM_FAILED;
    }

    int32_t osAccountId = (authContext->direction == DM_AUTH_SOURCE) ?
        authContext->accesser.userId : authContext->accessee.userId;
    std::string credId;
    int32_t ret = authContext->hiChainAuthConnector->AddCredential(osAccountId, authParamsString, credId);
    if (ret != DM_OK) {
        LOGE("AuthCredentialAgreeState::GenerateCredIdAndPublicKey() error, add credential failed.");
        return ret;
    }

    std::string publicKey;
    ret = authContext->hiChainAuthConnector->ExportCredential(osAccountId, credId, publicKey);
    if (ret != DM_OK) {
        LOGE("AuthCredentialAgreeState::GenerateCredIdAndPublicKey(), export publicKey failed.");
        authContext->hiChainAuthConnector->DeleteCredential(osAccountId, credId);
        return ret;
    }

    (void)authContext->SetCredentialId(DM_AUTH_LOCAL_SIDE, authorizedScope, credId);
    (void)authContext->SetPublicKey(DM_AUTH_LOCAL_SIDE, authorizedScope, publicKey);
    LOGI("AuthCredentialAgreeState::GenerateCredIdAndPublicKey credId=%{public}s, publicKey=%{public}s.\n",
        authContext->GetCredentialId(DM_AUTH_LOCAL_SIDE, authorizedScope).c_str(),
        authContext->GetPublicKey(DM_AUTH_LOCAL_SIDE, authorizedScope).c_str());
    LOGI("AuthCredentialAgreeState::GenerateCredIdAndPublicKey leave.");
    return DM_OK;
}

// Get the negotiation credential ID by agree credential
int32_t AuthCredentialAgreeState::AgreeCredential(DmAuthScope authorizedScope,
    std::shared_ptr<DmAuthContext> &authContext)
{
    LOGI("AuthCredentialAgreeState::AgreeCredential start.");
    if ((authorizedScope != DM_AUTH_SCOPE_USER && authorizedScope != DM_AUTH_SCOPE_APP) || authContext == nullptr) {
        return ERR_DM_FAILED;
    }

    std::string authParamsString = CreateAuthParamsString(authorizedScope,
        DM_AUTH_CREDENTIAL_ADD_METHOD_IMPORT, authContext);
    if (authParamsString == "") {
        LOGE("AuthCredentialAgreeState::AgreeCredential error, create authParamsString failed.");
        return ERR_DM_FAILED;
    }

    int32_t osAccountId = authContext->direction == DM_AUTH_SOURCE ?
        authContext->accesser.userId : authContext->accessee.userId;
    std::string selfCredId = authContext->GetCredentialId(DM_AUTH_LOCAL_SIDE, authorizedScope);
    std::string credId;
    LOGI("AuthCredentialAgreeState::AgreeCredential agree with accountId %{public}d and param %{public}s.",
        osAccountId, authParamsString.c_str());
    int32_t ret = authContext->hiChainAuthConnector->AgreeCredential(osAccountId, selfCredId,
        authParamsString, credId);
    if (ret != DM_OK) {
        LOGE("AuthCredentialAgreeState::AgreeCredential error, agree credential failed.");
        return ret;
    }

    (void)authContext->SetCredentialId(DM_AUTH_LOCAL_SIDE, authorizedScope, credId);
    LOGI("AuthCredentialAgreeState::AgreeCredential leave.");
    return DM_OK;
}

DmAuthStateType AuthSrcCredentialExchangeState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_CREDENTIAL_EXCHANGE_STATE;
}

int32_t AuthSrcCredentialExchangeState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcCredentialExchangeState::Action() start.");
    int32_t ret = ERR_DM_FAILED;
    context->isAppCredentialVerified = false;

    if (!NeedAgreeAcl(context)) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcDataSyncState>());
        return DM_OK;
    }

    if (!NeedAgreeCredential(context)) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthStartState>());
        return DM_OK;
    }

    // First authentication, generate LNN credentials and public key
    if (context->accesser.isGenerateLnnCredential && context->accesser.bindLevel != USER) {
        ret = GenerateCredIdAndPublicKey(DM_AUTH_SCOPE_USER, context);
        if (ret != DM_OK) {
            LOGE("AuthSrcCredentialExchangeState::Action() error, generate user credId and publicKey failed.");
            return ret;
        }
    }

    // Generate transmit credentials and public key
    ret = GenerateCredIdAndPublicKey(DM_AUTH_SCOPE_APP, context);
    if (ret != DM_OK) {
        LOGE("AuthSrcCredentialExchangeState::Action() error, generate app credId and publicKey failed.");
        return ret;
    }

    std::string message = context->authMessageProcessor->CreateMessage(MSG_TYPE_REQ_CREDENTIAL_EXCHANGE, context);
    LOGI("AuthSrcCredentialExchangeState::Action() leave.");
    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}

DmAuthStateType AuthSinkCredentialExchangeState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_CREDENTIAL_EXCHANGE_STATE;
}

int32_t AuthSinkCredentialExchangeState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkCredentialExchangeState::Action start.");
    int32_t ret = ERR_DM_FAILED;
    std::string tmpCredId;
    int32_t osAccountId = context->accessee.userId;
    context->isAppCredentialVerified = false;

    if (context == nullptr || context->hiChainAuthConnector == nullptr ||
        context->authMessageProcessor == nullptr || context->softbusConnector == nullptr) {
        return ret;
    }

    // First authentication lnn cred
    if (context->accessee.isGenerateLnnCredential && context->accessee.bindLevel != USER) {
        // Generate credentials and public key
        ret = GenerateCredIdAndPublicKey(DM_AUTH_SCOPE_USER, context);
        if (ret != DM_OK) {
            LOGE("AuthSinkCredentialExchangeState::Action failed, generate user cred and publicKey failed.");
            return ret;
        }

        // Agree credentials
        tmpCredId = context->accessee.lnnCredentialId;
        ret = AgreeCredential(DM_AUTH_SCOPE_USER, context);
        if (ret != DM_OK) {
            context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);
            context->SetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_USER, "");
            LOGE("AuthSinkCredentialExchangeState::Action failed, agree user cred failed.");
            return ret;
        }

       // Delete temporary credentials
        context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);
    }

    // Generate transport credentials and public key
    ret = GenerateCredIdAndPublicKey(DM_AUTH_SCOPE_APP, context);
    if (ret != DM_OK) {
        LOGE("AuthSinkCredentialExchangeState::Action failed, generate app cred and publicKey failed.");
        return ret;
    }

    // Agree transport credentials and public key
    tmpCredId = context->accessee.transmitCredentialId;
    ret = AgreeCredential(DM_AUTH_SCOPE_APP, context);
    if (ret != DM_OK) {
        context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);
        context->SetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_APP, "");
        LOGE("AuthSinkCredentialExchangeState::Action failed, agree app cred failed.");
        return ret;
    }

    // Delete temporary transport credentials
    context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);

    std::string message = context->authMessageProcessor->CreateMessage(MSG_TYPE_RESP_CREDENTIAL_EXCHANGE, context);
    LOGI("AuthSinkCredentialExchangeState::Action leave.");
    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}

DmAuthStateType AuthSrcCredentialAuthStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_START_STATE;
}

int32_t AuthSrcCredentialAuthStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI(" AuthSrcCredentialAuthStartState::Action start.");
    int32_t ret = ERR_DM_FAILED;
    std::string tmpCredId = "";
    int32_t osAccountId = context->accesser.userId;

    if (context == nullptr || context->hiChainAuthConnector == nullptr ||
        context->authMessageProcessor == nullptr || context->softbusConnector == nullptr) {
        return ret;
    }

    if (NeedAgreeCredential(context)) {
        // First authentication
        if (context->accesser.isGenerateLnnCredential && context->accesser.bindLevel != USER) {
            // Agree lnn credentials and public key
            tmpCredId = context->accesser.lnnCredentialId;
            ret = AgreeCredential(DM_AUTH_SCOPE_USER, context);
            if (ret != DM_OK) {
                context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);
                context->SetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_USER, "");
                LOGE("AuthSrcCredentialAuthStartState::Action failed, agree user cred failed.");
                return ret;
            }

            // Delete temporary lnn credentials
            context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);
        }

        // Agree transport credentials and public key
        tmpCredId = context->accesser.transmitCredentialId;
        ret = AgreeCredential(DM_AUTH_SCOPE_APP, context);
        if (ret != DM_OK) {
            context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);
            context->SetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_APP, "");
            LOGE("AuthSrcCredentialAuthStartState::Action failed, agree app cred failed.");
            return ret;
        }

        // Delete temporary transport credentials
        context->hiChainAuthConnector->DeleteCredential(osAccountId, tmpCredId);
    }

    // Transport credential authentication
    ret = context->hiChainAuthConnector->AuthCredential(osAccountId, context->requestId,
        context->accesser.transmitCredentialId, std::string(""));
    if (ret != DM_OK) {
        LOGE("AuthSrcCredentialAuthStartState::Action failed, auth app cred failed.");
        return ret;
    }

    if (context->authStateMachine->WaitExpectEvent(ON_TRANSMIT) != ON_TRANSMIT) {
        LOGE("AuthSrcCredentialAuthStartState::Action failed, ON_TRANSMIT event not arrived.");
        return ERR_DM_FAILED;
    }

    std::string message = context->authMessageProcessor->CreateMessage(MSG_TYPE_REQ_CREDENTIAL_AUTH_START, context);
    LOGI(" AuthSrcCredentialAuthStartState::Action leave.");
    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}
} // namespace DistributedHardware
} // namespace OHOS
