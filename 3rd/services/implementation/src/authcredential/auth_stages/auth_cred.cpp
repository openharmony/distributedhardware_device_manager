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

#include "auth_manager_cred.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_context_cred.h"
#include "dm_auth_message_processor_cred.h"
#include "dm_auth_state_cred.h"
#include "dm_auth_state_machine_cred.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {

int32_t AuthCredentialTransmitSend(std::shared_ptr<DmAuthCredContext> context, DmCredMessageType msgType)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (context->transmitData.empty()) {
        LOGE("Get onTransmitData failed.");
        return ERR_DM_FAILED;
    }

    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    std::string message =
        context->authMessageProcessor->CreateMessage(msgType, context);
    if (message.empty()) {
        LOGE("CreateMessage AuthCredential transmit data failed");
        return ERR_DM_FAILED;
    }

    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_POINT_NULL);
    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}

int32_t WaitAuthFinish(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (context->authStateMachine->WaitExpectEvent(DmEventType::ON_FINISH) != DmEventType::ON_FINISH) {
        LOGE("wait auth finish failed");
        return ERR_DM_FAILED;
    }
    if (context->direction == DM_AUTH_CRED_SOURCE) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthDoneState>());
        return DM_OK;
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialAuthDoneState>());
    return DM_OK;
}

int32_t WaitSessionKey(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    std::set<DmEventType> events = {
        DmEventType::ON_NEXT,
        DmEventType::ON_SESSION_KEY_RETURNED,
        DmEventType::ON_FINISH,
        DmEventType::ON_FAIL
    };
    DmEventType event = context->authStateMachine->WaitExpectEvents(events);
    if (event == DmEventType::ON_NEXT) {
        return DM_OK;
    }
    if (event == DmEventType::ON_SESSION_KEY_RETURNED) {
        return WaitAuthFinish(context);
    }
    LOGE("failed event %{public}d", event);
    return ERR_DM_FAILED;
}

int32_t CredentialAuthMsgNegotiate(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (!context->transmitData.empty()) {
        int32_t ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
        if (ret != DM_OK) {
            LOGE("ProcessCredData transmit data failed %{public}d", ret);
            return ERR_DM_FAILED;
        }
    }
    std::set<DmEventType> events = {
        DmEventType::ON_TRANSMIT,
        DmEventType::ON_SESSION_KEY_RETURNED,
        DmEventType::ON_FINISH
    };
    DmEventType event = context->authStateMachine->WaitExpectEvents(events);
    if (event == DmEventType::ON_TRANSMIT) {
        DmCredMessageType msgType = (context->direction == DM_AUTH_CRED_SOURCE) ?
        DmCredMessageType::CRED_REQ_CREDENTIAL_AUTH_START : DmCredMessageType::CRED_RESP_CREDENTIAL_AUTH_START;
        int32_t ret = AuthCredentialTransmitSend(context, msgType);
        if (ret != DM_OK) {
            LOGE("AuthCredentialTransmitSend failed %{public}d", ret);
            return ERR_DM_FAILED;
        }
        return WaitSessionKey(context);
    }
    if (event == DmEventType::ON_SESSION_KEY_RETURNED) {
        return WaitAuthFinish(context);
    }
    return DM_OK;
}

DmAuthStateType AuthSrcCredentialAuthStartState::GetStateType()
{
    return DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_START_STATE;
}

int32_t AuthSrcCredentialAuthStartState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    LOGI("start.");
    int32_t ret = ERR_DM_FAILED;
    if (context == nullptr || context->hiChainAuthConnector == nullptr || context->authMessageProcessor == nullptr ||
        context->softbusConnector == nullptr || context->softbusConnector->GetSoftbusSession() == nullptr ||
        context->authStateMachine == nullptr || context->timer == nullptr) {
        return ret;
    }
    context->timer->DeleteTimer(std::string(NEGOTIATE_TIMEOUT_TASK));
    int32_t osAccountId = context->accesser.userId;
    ret = QueryCredential(context);
    if (ret != DM_OK) {
        LOGE("QueryCredential failed: %{public}d.", ret);
        return ret;
    }
    // Transport credential authentication
    ret = context->hiChainAuthConnector->AuthCredential(osAccountId, context->requestId,
        context->accesser.transmitCredentialId);
    if (ret != DM_OK) {
        LOGE("auth cred failed.");
        return ret;
    }
    if (context->authStateMachine->WaitExpectEvent(ON_TRANSMIT) != ON_TRANSMIT) {
        return ERR_DM_FAILED;
    }
    std::string message = context->authMessageProcessor->CreateMessage(CRED_REQ_CREDENTIAL_AUTH_START, context);
    LOGI("leave.");
    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}

DmAuthStateType AuthSrcCredentialAuthMsgNegotiateState::GetStateType()
{
    return DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE;
}

// Parse the ontransmit data
int32_t AuthSrcCredentialAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    return CredentialAuthMsgNegotiate(context);
}

DmAuthStateType AuthSinkCredentialAuthMsgNegotiateState::GetStateType()
{
    return DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE;
}

int32_t AuthSinkCredentialAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    return CredentialAuthMsgNegotiate(context);
}

DmAuthStateType AuthSrcCredentialAuthDoneState::GetStateType()
{
    return DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_DONE_STATE;
}

int32_t AuthSrcCredentialAuthDoneState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->softbusConnector->GetSoftbusSession(), ERR_DM_POINT_NULL);

    context->reply = DM_OK;
    context->reason = DM_OK;
    std::string message = context->authMessageProcessor->CreateMessage(DmCredMessageType::CRED_REQ_FINISH, context);
    if (message.empty()) {
        LOGE("CreateMessage failed");
        return ERR_DM_FAILED;
    }
    return context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
}

DmAuthStateType AuthSinkCredentialAuthDoneState::GetStateType()
{
    return DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_DONE_STATE;
}

int32_t AuthSinkCredentialAuthDoneState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    context->reply = DM_OK;
    context->reason = DM_OK;
    return DM_OK;
}

int32_t AuthCredSinkFinishState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    LOGI("Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    LOGI("reason: %{public}d", context->reason);
    context->state = static_cast<int32_t>(GetStateType());
    SinkFinish(context);
    LOGI("Action ok");
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }
    return DM_OK;
}

DmAuthStateType AuthCredSinkFinishState::GetStateType()
{
    return DmAuthStateType::CRED_AUTH_SINK_FINISH_STATE;
}

int32_t AuthCredSrcFinishState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    LOGI("Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (context->reason != DM_OK) {
        context->authMessageProcessor->CreateAndSendMsg(DmCredMessageType::CRED_REQ_FINISH, context);
    } else {
        context->state = static_cast<int32_t>(GetStateType());
    }
    SourceFinish(context);
    LOGI("AuthCredSrcFinishState::Action ok");
    if (context->reason != DM_OK) {
        context->connDelayCloseTime = 0;
    }
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }
    return DM_OK;
}

DmAuthStateType AuthCredSrcFinishState::GetStateType()
{
    return DmAuthStateType::CRED_AUTH_SRC_FINISH_STATE;
}
} // namespace DistributedHardware
} // namespace OHOS