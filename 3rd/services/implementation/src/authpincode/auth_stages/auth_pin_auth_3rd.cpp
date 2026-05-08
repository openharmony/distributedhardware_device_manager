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
#include "dm_anonymous_3rd.h"
#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_message_processor_3rd.h"
#include "dm_auth_pincode_state_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {

DmAuthPincodeStateType AuthPincodeSrcPinAuthStartState::GetStateType()
{
    return DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_START_STATE;
}

int32_t AuthPincodeSrcPinAuthStartState::Action(std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("AuthPincodeSrcPinAuthStartState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    auto ret = context->hiChainAuthConnector->AuthCredentialPinCode(context->accesser.userId, context->requestId,
        context->importAuthCode);
    if (ret != DM_OK) {
        LOGE("AuthPincodeSrcPinAuthStartState::Action call AuthCredentialPinCode failed.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authPinStateMac3rd->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 1030 msg
        context->authPinMsgProc3rd->CreateAndSendMsg(DmPincodeMessageType::AUTH_PINCODE_REQ_PIN_AUTH_START, context);
        return DM_OK;
    } else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthPincodeSrcPinAuthStartState::Action ON_ERROR failed, maybe retry.");
        return DM_OK;
    }

    return STOP_BIND;
}

DmAuthPincodeStateType AuthPincodeSinkPinAuthStartState::GetStateType()
{
    return DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_START_STATE;
}

int32_t AuthPincodeSinkPinAuthStartState::Action(std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("AuthPincodeSinkPinAuthStartState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->timer, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinMsgProc3rd, ERR_DM_POINT_NULL);
    context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));

    context->timer->StartTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK),
        DmAuthPincodeState3rd::GetTaskTimeout(context, WAIT_PIN_AUTH_TIMEOUT_TASK, PIN_AUTH_TIMEOUT),
        [context] (std::string name) {
            HandleAuthenticateTimeout(context, name);
        });

    std::string pinCodeHash = GetAnonyString(Crypto3rd::Sha256(context->importAuthCode));
    LOGI("AuthPincodeSinkPinAuthStartState pinCodeHash: %{public}s", GetAnonyString(pinCodeHash).c_str());
    // process pincode auth
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthPincodeSinkPinAuthStartState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authPinStateMac3rd->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 1040 msg
        context->authPinMsgProc3rd->CreateAndSendMsg(DmPincodeMessageType::AUTH_PINCODE_RESP_PIN_AUTH_START, context);
        return DM_OK;
    }
    if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthPincodeSinkPinAuthStartState::Action ON_ERROR failed, maybe retry.");
        return DM_OK;
    }
    return STOP_BIND;
}

DmAuthPincodeStateType AuthPincodeSrcPinAuthMsgNegotiateState::GetStateType()
{
    return DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE;
}

int32_t AuthPincodeSrcPinAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("AuthPincodeSrcPinAuthMsgNegotiateState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinMsgProc3rd, ERR_DM_POINT_NULL);
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthPincodeSrcPinAuthMsgNegotiateState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authPinStateMac3rd->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 1031 msg
        context->authPinMsgProc3rd->CreateAndSendMsg(DmPincodeMessageType::AUTH_PINCODE_REQ_PIN_AUTH_MSG_NEGOTIATE,
            context);
        return DM_OK;
    }
    if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthPincodeSrcPinAuthMsgNegotiateState::AuthDevice ON_ERROR failed, maybe retry.");
        return DM_OK;
    }
    LOGE("AuthPincodeSrcPinAuthMsgNegotiateState::Action failed.");
    return STOP_BIND;
}

DmAuthPincodeStateType AuthPincodeSinkPinAuthMsgNegotiateState::GetStateType()
{
    return DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE;
}

int32_t AuthPincodeSinkPinAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("AuthPincodeSinkPinAuthMsgNegotiateState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinMsgProc3rd, ERR_DM_POINT_NULL);
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthPincodeSinkPinAuthMsgNegotiateState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authPinStateMac3rd->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 1041 msg
        context->authPinMsgProc3rd->CreateAndSendMsg(DmPincodeMessageType::AUTH_PINCODE_RESP_PIN_AUTH_MSG_NEGOTIATE,
            context);
    } else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthPincodeSinkPinAuthMsgNegotiateState::Action WAIT ON_TRANSMIT ON_ERROR failed, maybe retry.");
        return DM_OK;
    } else {
        return STOP_BIND;
    }

    retEvent = context->authPinStateMac3rd->WaitExpectEvent(DmEventType::ON_SESSION_KEY_RETURNED);
    if (retEvent == DmEventType::ON_SESSION_KEY_RETURNED) {
        retEvent = context->authPinStateMac3rd->WaitExpectEvent(DmEventType::ON_FINISH);
        if (retEvent == DmEventType::ON_FINISH || retEvent == DmEventType::ON_ERROR) {
            context->timer->DeleteTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK));
            return DM_OK;
        }
    }  else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("WAIT ON_SESSION_KEY_RETURNED ON_ERROR failed, maybe retry.");
        return DM_OK;
    }
    LOGE("AuthPincodeSinkPinAuthMsgNegotiateState::Action failed.");
    return STOP_BIND;
}

DmAuthPincodeStateType AuthPincodeSrcPinAuthDoneState::GetStateType()
{
    return DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_DONE_STATE;  // 5
}

int32_t AuthPincodeSrcPinAuthDoneState::Action(std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("AuthPincodeSrcPinAuthDoneState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinMsgProc3rd, ERR_DM_POINT_NULL);
    std::string onTransmitData = context->transmitData;
    if (context->hiChainAuthConnector->ProcessCredData(context->requestId, onTransmitData) != DM_OK) {
        LOGE("AuthPincodeSrcPinAuthDoneState::Action failed, processCredData failed.");
        return ERR_DM_FAILED;
    }

    // wait for ON_SESSION_KEY_RETURNED from hichain
    DmEventType ret = context->authPinStateMac3rd->WaitExpectEvent(ON_SESSION_KEY_RETURNED);
    if (ret != ON_SESSION_KEY_RETURNED) {
        LOGE("Action failed, ON_SESSION_KEY_RETURNED event failed, other event arriverd.");
        return ERR_DM_FAILED;
    }

    // wait for ON_FINISH from hichain
    ret = context->authPinStateMac3rd->WaitExpectEvent(ON_FINISH);
    if (ret == ON_FINISH) {
        LOGI("AuthPincodeSrcPinAuthDoneState::Action wait ON_FINISH done");
        context->timer->DeleteTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK));
        //0416 HIplay send 1050
        context->authPinMsgProc3rd->CreateAndSendMsg(DmPincodeMessageType::AUTH_PINCODE_REQ_FINISH, context);
        return DM_OK;
    } else if (ret == ON_ERROR) {
        LOGE("AuthPincodeSrcPinAuthDoneState::Action, ON_FINISH event not arriverd, maybe retry.");
        return DM_OK;
    }

    return ERR_DM_FAILED;
}
} // namespace DistributedHardware
} // namespace OHOS