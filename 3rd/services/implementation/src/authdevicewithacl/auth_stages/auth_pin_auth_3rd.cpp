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

#include "auth_manager_3rd.h"
#include "deviceprofile_connector_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_context_3rd.h"
#include "dm_auth_message_processor_3rd.h"
#include "dm_auth_state_machine_3rd.h"
#include "dm_auth_state_3rd.h"
#include "dm_auth_state_machine_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {

DmAuthStateType AuthSrcPinAuthStartState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SRC_PIN_AUTH_START_STATE;
}

int32_t AuthSrcPinAuthStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcPinAuthStartState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    auto ret = context->hiChainAuthConnector->AuthCredentialPinCode(context->accesser.userId, context->requestId,
        context->importAuthCode);
    if (ret != DM_OK) {
        LOGE("AuthSrcPinAuthStartState::AuthDevice call AuthCredentialPinCode failed.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 2030 msg
        context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_REQ_PIN_AUTH_START, context);
        return DM_OK;
    } else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSrcPinAuthStartState::AuthDevice ON_ERROR failed, maybe retry.");
        return DM_OK;
    }

    return STOP_BIND;
}

DmAuthStateType AuthSinkPinAuthStartState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SINK_PIN_AUTH_START_STATE;
}

int32_t AuthSinkPinAuthStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkPinAuthStartState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->timer, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));

    context->timer->StartTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK),
        DmAuthState3rd::GetTaskTimeout(context, WAIT_PIN_AUTH_TIMEOUT_TASK, PIN_AUTH_TIMEOUT),
        [context] (std::string name) {
            HandleAuthenticateTimeout(context, name);
        });

    std::string pinCodeHash = GetAnonyString(Crypto3rd::Sha256(context->importAuthCode));
    LOGI("AuthSinkPinAuthStartState pinCodeHash: %{public}s", pinCodeHash.c_str());
    // process pincode auth
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthSinkPinAuthStartState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 2040 msg
        context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_RESP_PIN_AUTH_START, context);
        return DM_OK;
    }
    if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSinkPinAuthStartState::AuthDevice ON_ERROR failed, maybe retry.");
        return DM_OK;
    }
    return STOP_BIND;
}

DmAuthStateType AuthSrcPinAuthMsgNegotiateState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE;
}

int32_t AuthSrcPinAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcPinAuthMsgNegotiateState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthSrcPinAuthMsgNegotiateState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 2031 msg
        context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_REQ_PIN_AUTH_MSG_NEGOTIATE, context);
        return DM_OK;
    }
    if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSrcPinAuthMsgNegotiateState::AuthDevice ON_ERROR failed, maybe retry.");
        return DM_OK;
    }
    LOGE("AuthSrcPinAuthMsgNegotiateState::Action failed.");
    return STOP_BIND;
}

DmAuthStateType AuthSinkPinAuthMsgNegotiateState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE;
}

int32_t AuthSinkPinAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkPinAuthMsgNegotiateState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthSinkPinAuthMsgNegotiateState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 2041 msg
        context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_RESP_PIN_AUTH_MSG_NEGOTIATE, context);
    } else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSinkPinAuthMsgNegotiateState::AuthDevice WAIT ON_TRANSMIT ON_ERROR failed, maybe retry.");
        return DM_OK;
    } else {
        return STOP_BIND;
    }

    retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_SESSION_KEY_RETURNED);
    if (retEvent == DmEventType::ON_SESSION_KEY_RETURNED) {
        retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_FINISH);
        if (retEvent == DmEventType::ON_FINISH || retEvent == DmEventType::ON_ERROR) {
            context->timer->DeleteTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK));
            context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinAuthDoneState>());
            return DM_OK;
        }
    }  else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSinkPinAuthMsgNegotiateState::AuthDevice WAIT ON_SESSION_KEY_RETURNED ON_ERROR failed, maybe retry.");
        return DM_OK;
    }
    LOGE("AuthSinkPinAuthMsgNegotiateState::AuthDevice failed.");
    return STOP_BIND;
}

DmAuthStateType AuthSrcPinAuthDoneState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SRC_PIN_AUTH_DONE_STATE;
}

int32_t AuthSrcPinAuthDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    LOGI("AuthSrcPinAuthDoneState::Action start");
    std::string onTransmitData = context->transmitData;
    if (context->hiChainAuthConnector->ProcessCredData(context->requestId, onTransmitData) != DM_OK) {
        LOGE("AuthSrcPinAuthDoneState::Action failed, processCredData failed.");
        return ERR_DM_FAILED;
    }

    // wait for ON_SESSION_KEY_RETURNED from hichain
    DmEventType ret = context->authStateMachine->WaitExpectEvent(ON_SESSION_KEY_RETURNED);
    if (ret != ON_SESSION_KEY_RETURNED) {
        LOGE("AuthSrcPinAuthDoneState::Action failed, ON_SESSION_KEY_RETURNED event failed, other event arriverd.");
        return ERR_DM_FAILED;
    }

    // wait for ON_FINISH from hichain
    ret = context->authStateMachine->WaitExpectEvent(ON_FINISH);
    if (ret == ON_FINISH) {
        LOGI("AuthSrcPinAuthDoneState::Action wait ON_FINISH done");
        DerivativeSessionKey(context);
        context->timer->DeleteTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK));
        return DM_OK;
    } else if (ret == ON_ERROR) {
        LOGE("AuthSrcPinAuthDoneState::Action, ON_FINISH event not arriverd, maybe retry.");
        return DM_OK;
    }

    return ERR_DM_FAILED;
}

int32_t AuthSrcPinAuthDoneState::DerivativeSessionKey(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        int32_t skId = 0;
        CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
        std::string suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            context->accesser.tokenIdHash + context->accessee.tokenIdHash;
        int32_t ret = context->authMessageProcessor->SaveDerivativeSessionKeyToDP(context->accesser.userId, suffix,
            skId, context->accesser.sessionKey);
        if (ret != DM_OK) {
            LOGE("AuthSrcCredentialAuthDoneState::Action DP save user session key failed");
            return ret;
        }
        context->accesser.transmitSkTimeStamp = static_cast<int64_t>(DmAuthState3rd::GetSysTimeMs());
        context->accesser.transmitSessionKeyId = skId;
        return DM_OK;
    }
    return DerivativeProxySessionKey(context);
}

int32_t AuthSrcPinAuthDoneState::DerivativeProxySessionKey(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (context->IsCallingProxyAsSubject) {
        int32_t skId = 0;
        CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
        std::string suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            context->accesser.tokenIdHash + context->accessee.tokenIdHash;
        int32_t ret = context->authMessageProcessor->SaveDerivativeSessionKeyToDP(context->accesser.userId, suffix,
            skId, context->accesser.sessionKey);
        if (ret != DM_OK) {
            LOGE("AuthSrcCredentialAuthDoneState::Action DP save user session key failed");
            return ret;
        }
        context->accesser.transmitSkTimeStamp = static_cast<int64_t>(DmAuthState3rd::GetSysTimeMs());
        context->accesser.transmitSessionKeyId = skId;
    }
    for (auto &app : context->subjectProxyOnes) {
        int32_t skId = 0;
        std::string suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            app.proxyAccesser.tokenIdHash + app.proxyAccessee.tokenIdHash;
        CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
        int32_t ret =
            context->authMessageProcessor->SaveDerivativeSessionKeyToDP(context->accesser.userId, suffix, skId,
            app.proxyAccesser.sessionKey);
        if (ret != DM_OK) {
            LOGE("AuthSrcCredentialAuthDoneState::Action DP save user session key failed");
            return ret;
        }
        app.proxyAccesser.transmitSkTimeStamp = static_cast<int64_t>(DmAuthState3rd::GetSysTimeMs());
        app.proxyAccesser.transmitSessionKeyId = skId;
    }
    return DM_OK;
}

DmAuthStateType AuthSinkPinAuthDoneState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SINK_PIN_AUTH_DONE_STATE;
}

int32_t AuthSinkPinAuthDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkPinAuthDoneState Action");
    DerivativeSessionKey(context);
    return DM_OK;
}

int32_t AuthSinkPinAuthDoneState::DerivativeSessionKey(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        int32_t skId = 0;
        CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
        std::string suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            context->accesser.tokenIdHash + context->accessee.tokenIdHash;
        int32_t ret = context->authMessageProcessor->SaveDerivativeSessionKeyToDP(context->accessee.userId, suffix,
            skId, context->accessee.sessionKey);
        if (ret != DM_OK) {
            LOGE("AuthSrcCredentialAuthDoneState::Action DP save user session key failed");
            return ret;
        }
        context->accessee.transmitSkTimeStamp = static_cast<int64_t>(DmAuthState3rd::GetSysTimeMs());
        context->accessee.transmitSessionKeyId = skId;
        return DM_OK;
    }
    return DerivativeProxySessionKey(context);
}

int32_t AuthSinkPinAuthDoneState::DerivativeProxySessionKey(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    if (context->IsCallingProxyAsSubject) {
        int32_t skId = 0;
        std::string suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            context->accesser.tokenIdHash + context->accessee.tokenIdHash;
        int32_t ret = context->authMessageProcessor->SaveDerivativeSessionKeyToDP(context->accessee.userId, suffix,
            skId, context->accessee.sessionKey);
        if (ret != DM_OK) {
            LOGE("AuthSinkPinAuthDoneState::Action DP save user session key failed");
            return ret;
        }
        context->accessee.transmitSkTimeStamp = static_cast<int64_t>(DmAuthState3rd::GetSysTimeMs());
        context->accessee.transmitSessionKeyId = skId;
    }
    for (auto &app : context->subjectProxyOnes) {
        int32_t skId = 0;
        std::string suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            app.proxyAccesser.tokenIdHash + app.proxyAccessee.tokenIdHash;
        CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
        int32_t ret =
            context->authMessageProcessor->SaveDerivativeSessionKeyToDP(context->accessee.userId, suffix, skId,
            app.proxyAccessee.sessionKey);
        if (ret != DM_OK) {
            LOGE("AuthSinkCredentialAuthNegotiateState::Action DP save user session key failed %{public}d", ret);
            return ret;
        }
        app.proxyAccessee.transmitSkTimeStamp = static_cast<int64_t>(DmAuthState3rd::GetSysTimeMs());
        app.proxyAccessee.transmitSessionKeyId = skId;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS