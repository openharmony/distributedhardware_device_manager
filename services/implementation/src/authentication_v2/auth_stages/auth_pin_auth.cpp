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

#include "auth_manager.h"
#include "deviceprofile_connector.h"
#include "dm_anonymous.h"
#include "dm_auth_context.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_state_machine.h"
#include "dm_auth_state.h"
#include "dm_auth_state_machine.h"
#include "dm_dialog_manager.h"
#include "dm_freeze_process.h"
#include "dm_log.h"
#include "dm_negotiate_process.h"
#include "dm_random.h"
#include "hichain_auth_connector.h"
#include "multiple_user_connector.h"
#include "service_info_profile.h"

#ifdef SUPPORT_MSDP
#include "spatial_location_callback_impl.h"
#include "spatial_awareness_mgr_client.h"
#endif

namespace OHOS {
namespace DistributedHardware {

constexpr int32_t MAX_AUTH_INPUT_PIN_FAIL_TIMES = 3;
constexpr int32_t GET_ULTRASONIC_PIN_TIMEOUT = 4;
constexpr int32_t MIN_PIN_CODE = 100000;
constexpr int32_t MAX_PIN_CODE = 999999;
constexpr const char* UNVALID_CREDTID = "invalidCredId";

int32_t AuthSinkStatePinAuthComm::ShowAuthInfoDialog(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkConfirmState::ShowAuthInfoDialog start");
    if (DmAuthState::IsScreenLocked()) {
        LOGE("AuthSinkConfirmState::ShowAuthInfoDialog screen is locked.");
        context->reason = ERR_DM_BIND_USER_CANCEL;
        context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
        return STOP_BIND;
    }

    DmDialogManager::GetInstance().ShowPinDialog(context->pinCode);
    context->timer->StartTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK),
        DmAuthState::GetTaskTimeout(context, SESSION_HEARTBEAT_TIMEOUT_TASK, SESSION_HEARTBEAT_TIMEOUT),
        [context] (std::string name) {
            AuthSinkStatePinAuthComm::HandleSessionHeartbeat(context, name);
        });
    return DM_OK;
}

void AuthSinkStatePinAuthComm::HandleSessionHeartbeat(std::shared_ptr<DmAuthContext> context, std::string name)
{
    context->timer->DeleteTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK));
    if (context->successFinished) {
        return;
    }

    LOGI("DmAuthManager::HandleSessionHeartbeat name %{public}s", name.c_str());
    JsonObject jsonObj;
    jsonObj[TAG_SESSION_HEARTBEAT] = TAG_SESSION_HEARTBEAT;
    std::string message = jsonObj.Dump();
    context->softbusConnector->GetSoftbusSession()->SendHeartbeatData(context->sessionId, message);

    context->timer->StartTimer(std::string(SESSION_HEARTBEAT_TIMEOUT_TASK),
        DmAuthState::GetTaskTimeout(context, SESSION_HEARTBEAT_TIMEOUT_TASK, SESSION_HEARTBEAT_TIMEOUT),
        [context] (std::string name) {
            AuthSinkStatePinAuthComm::HandleSessionHeartbeat(context, name);
        });

    LOGI("DmAuthManager::HandleSessionHeartbeat complete.");
}

bool AuthSinkStatePinAuthComm::IsPinCodeValid(int32_t numpin)
{
    if (numpin < MIN_PIN_CODE || numpin > MAX_PIN_CODE) {
        return false;
    }
    return true;
}

bool AuthSinkStatePinAuthComm::IsPinCodeValid(const std::string& strpin)
{
    if (strpin.empty()) {
        return false;
    }
    for (size_t i = 0; i < strpin.length(); i++) {
        if (!isdigit(strpin[i])) {
            return false;
        }
    }
    int32_t pinnum = std::atoi(strpin.c_str());
    return IsPinCodeValid(pinnum);
}

bool AuthSinkStatePinAuthComm::IsAuthCodeReady(std::shared_ptr<DmAuthContext> context)
{
    if (context->importAuthCode.empty() || context->importPkgName.empty()) {
        LOGE("AuthSinkStatePinAuthComm::IsAuthCodeReady, auth code not ready with authCode %{public}s and "
            "pkgName %{public}s.", GetAnonyString(context->importAuthCode).c_str(), context->importPkgName.c_str());
        return false;
    }
    if (context->pkgName != context->importPkgName) {
        LOGE("AuthSinkNegotiateStateMachine::IsAuthCodeReady pkgName %{public}s not supported with "
            "import pkgName %{public}s.", context->pkgName.c_str(), context->importPkgName.c_str());
        return false;
    }
    return true;
}

void AuthSinkStatePinAuthComm::GeneratePincode(std::shared_ptr<DmAuthContext> context)
{
    int32_t pinCode = GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
    context->pinCode = std::to_string(pinCode);
}

DmAuthStateType AuthSrcPinAuthStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE;
}

int32_t AuthSrcPinAuthStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcPinAuthStartState::Action start");
    // auth pincode
    auto ret = context->hiChainAuthConnector->AuthCredentialPinCode(context->accesser.userId, context->requestId,
        context->pinCode);
    if (ret != DM_OK) {
        LOGE("AuthSrcPinAuthStartState::AuthDevice call AuthCredentialPinCode failed.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 120 msg
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_PIN_AUTH_START, context);
        return DM_OK;
    } else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSrcPinAuthStartState::AuthDevice ON_ERROR failed, maybe retry.");
        return DM_OK;
    }

    return STOP_BIND;
}

DmAuthStateType AuthSinkPinAuthStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE;
}

int32_t AuthSinkPinAuthStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkPinAuthStartState::Action start");
    context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));
    if (!context->pinNegotiateStarted) {
        context->pinNegotiateStarted = true;
        context->timer->StartTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK),
            DmAuthState::GetTaskTimeout(context, WAIT_PIN_AUTH_TIMEOUT_TASK, PIN_AUTH_TIMEOUT),
            [context] (std::string name) {
                HandleAuthenticateTimeout(context, name);
            });
    }

    // Stop the abnormal authentication process
    if (context->authTypeList.empty() ||
        (context->confirmOperation != UiAction::USER_OPERATION_TYPE_ALLOW_AUTH &&
        context->confirmOperation != UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS)) {
        LOGE("AuthSinkPinAuthStartState::Action invalid parameter.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (FreezeProcess::GetInstance().CleanFreezeRecord(context->accessee.bundleName,
        context->accessee.deviceType) != DM_OK) {
        LOGE("CleanFreezeRecord failed.");
    }
    // process pincode auth
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthSinkPinAuthStartState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 130 msg
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_PIN_AUTH_START, context);
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
    return DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE;
}

int32_t AuthSrcPinAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcPinAuthMsgNegotiateState::Action start");
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (context->authType == AUTH_TYPE_PIN_ULTRASONIC && context->ultrasonicInfo == DM_Ultrasonic_Forward) {
        context->timer->DeleteTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK));
    }
    if (ret != DM_OK) {
        LOGE("AuthSrcPinAuthMsgNegotiateState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 121 msg
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_PIN_AUTH_MSG_NEGOTIATE, context);
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
    return DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE;
}

int32_t AuthSinkPinAuthMsgNegotiateState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkPinAuthMsgNegotiateState::Action start");
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthSinkPinAuthMsgNegotiateState::Action call ProcessCredData err.");
        return ret;
    }
    // wait for onTransmit from hiChain
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        // send 131 msg
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_PIN_AUTH_MSG_NEGOTIATE, context);
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

DmAuthStateType AuthSinkPinAuthDoneState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_PIN_AUTH_DONE_STATE;
}

int32_t AuthSinkPinAuthDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkPinAuthDoneState Action");
    return DM_OK;
}

DmAuthStateType AuthSrcPinAuthDoneState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE;
}

int32_t AuthSrcPinAuthDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcPinAuthDoneState::Action start");
    std::string onTransmitData = context->transmitData;
    if (context->hiChainAuthConnector->ProcessCredData(context->requestId, onTransmitData) != DM_OK) {
        LOGE("AuthSrcPinAuthDoneState::Action failed, processCredData failed.");
        return ERR_DM_FAILED;
    }

    // wait for ON_SESSION_KEY_RETURNED from hichain
    DmEventType ret = context->authStateMachine->WaitExpectEvent(ON_SESSION_KEY_RETURNED);
    if (ret != ON_SESSION_KEY_RETURNED) {
        if (ret == ON_ERROR) {
            LOGE("AuthSrcPinAuthDoneState::Action, ON_SESSION_KEY_RETURNED event not arriverd, maybe retry.");
            return DM_OK;
        } else {
            LOGE("AuthSrcPinAuthDoneState::Action failed, ON_SESSION_KEY_RETURNED event failed, other event arriverd.");
            return ERR_DM_FAILED;
        }
    }

    // wait for ON_FINISH from hichain
    ret = context->authStateMachine->WaitExpectEvent(ON_FINISH);
    if (ret == ON_FINISH) {
        LOGI("AuthSrcPinAuthDoneState::Action wait ON_FINISH done");
        return DM_OK;
    } else if (ret == ON_ERROR) {
        return DM_OK;
        LOGE("AuthSrcPinAuthDoneState::Action, ON_FINISH event not arriverd, maybe retry.");
    }

    return ERR_DM_FAILED;
}

DmAuthStateType AuthSrcPinNegotiateStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE;
}

int32_t AuthSrcPinNegotiateStartState::NegotiatePinAuth(std::shared_ptr<DmAuthContext> context, bool firstTime)
{
    if (firstTime) {
        if (context->authTypeList.empty()) {
            LOGE("authTypeList empty");
            context->reason = ERR_DM_AUTH_REJECT;
            return ERR_DM_AUTH_REJECT;
        }
        context->currentAuthTypeIdx = 0;
        context->authType = context->authTypeList[0];
    } else {
        if (context->authType == DmAuthType::AUTH_TYPE_PIN &&
            context->inputPinAuthFailTimes < MAX_AUTH_INPUT_PIN_FAIL_TIMES) {
            LOGI("input pin auth err, retry");
        } else {
            // try to fallback to next auth type
            if (context->currentAuthTypeIdx + 1 >= context->authTypeList.size()) {
                LOGE("all auth type failed");
                context->reason = ERR_DM_AUTH_REJECT;
                return ERR_DM_AUTH_REJECT;
            }
            context->currentAuthTypeIdx++;
            context->authType = context->authTypeList[context->currentAuthTypeIdx];
        }
    }

    // restart pin auth timer
    context->timer->DeleteTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK));
    context->timer->StartTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK),
        DmAuthState::GetTaskTimeout(context, WAIT_PIN_AUTH_TIMEOUT_TASK, PIN_AUTH_TIMEOUT),
        [context] (std::string name) {
            HandleAuthenticateTimeout(context, name);
        });
    if (DmAuthState::IsImportAuthCodeCompatibility(context->authType)) {
        if (AuthSinkStatePinAuthComm::IsAuthCodeReady(context)) {
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinAuthStartState>());
        } else {
            context->reason = ERR_DM_INPUT_PARA_INVALID;
            return ERR_DM_FAILED;
        }
    } else if (context->authType == DmAuthType::AUTH_TYPE_PIN) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinInputState>());
    } else if (context->authType == DmAuthType::AUTH_TYPE_PIN_ULTRASONIC &&
        context->ultrasonicInfo == DM_Ultrasonic_Forward) {
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcForwardUltrasonicStartState>());
    } else if (context->authType == DmAuthType::AUTH_TYPE_PIN_ULTRASONIC &&
        context->ultrasonicInfo == DM_Ultrasonic_Reverse) {
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcReverseUltrasonicStartState>());
    } else {
        LOGE("authType not support.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t AuthSrcPinNegotiateStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->pinNegotiateStarted) {
        int32_t ret = NegotiateProcess::GetInstance().HandleNegotiateResult(context);
        if (ret != DM_OK) {
            LOGE("HandleNegotiateResult failed ret %{public}d", ret);
            context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
            return ret;
        }
    }
    if (!context->needBind && !context->needAgreeCredential && context->needAuth) {
        return ProcessCredAuth(context);
    }
    if (context->needBind) {
        return ProcessPinBind(context);
    }
    if (!context->needBind && !context->needAgreeCredential && !context->needAuth) {
        context->reason = ERR_DM_BIND_TRUST_TARGET;
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
        return DM_OK;
    }
    context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
    return ERR_DM_FAILED;
}

int32_t AuthSrcPinNegotiateStartState::ProcessCredAuth(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    JsonObject accesserCredTypeList;
    accesserCredTypeList.Parse(context->accesser.credTypeList);
    if (accesserCredTypeList.IsDiscarded()) {
        LOGE("CredTypeList invalid");
        context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
        return ERR_DM_FAILED;
    }
    if (accesserCredTypeList.Contains("identicalCredType")) {
        context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
        context->accesser.transmitCredentialId = GetCredIdByCredType(context, DM_IDENTICAL_ACCOUNT);
    } else if (accesserCredTypeList.Contains("shareCredType")) {
        context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
        context->accesser.transmitCredentialId = GetCredIdByCredType(context, DM_SHARE);
    } else if (accesserCredTypeList.Contains("pointTopointCredType")) {
        context->accesser.transmitCredentialId = GetCredIdByCredType(context, DM_POINT_TO_POINT);
    } else if (accesserCredTypeList.Contains("lnnCredType")) {
        context->accesser.lnnCredentialId = GetCredIdByCredType(context, DM_LNN);
    } else {
        LOGE("credTypeList invalid.");
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthStartState>());
    return DM_OK;
}

std::string AuthSrcPinNegotiateStartState::GetCredIdByCredType(std::shared_ptr<DmAuthContext> context, int32_t credType)
{
    LOGI("credType %{public}d.", credType);
    CHECK_NULL_RETURN(context, UNVALID_CREDTID);
    if (context->accesser.credentialInfos.find(credType) != context->accesser.credentialInfos.end()) {
        LOGE("invalid credType.");
        return UNVALID_CREDTID;
    }
    std::string credInfoStr = context->accesser.credentialInfos[credType];
    JsonObject credInfoJson;
    credInfoJson.Parse(credInfoStr);
    if (credInfoJson.IsDiscarded() || !credInfoJson.Contains(FILED_CRED_ID) ||
        !credInfoJson[FILED_CRED_ID].IsNumberInteger()) {
        LOGE("credInfoStr invalid.");
        return UNVALID_CREDTID;
    }
    return credInfoJson[FILED_CRED_ID].Get<std::string>();
}

int32_t AuthSrcPinNegotiateStartState::ProcessPinBind(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->pinNegotiateStarted) {
        context->pinNegotiateStarted = true;
        CHECK_NULL_RETURN(context->timer, ERR_DM_POINT_NULL);
        context->timer->DeleteTimer(std::string(CONFIRM_TIMEOUT_TASK));
        // import pin code auth always excute
        if (DmAuthState::IsImportAuthCodeCompatibility(context->authType) &&
            (!context->authTypeList.empty()) &&
            DmAuthState::IsImportAuthCodeCompatibility(context->authTypeList[0])) {
            return NegotiatePinAuth(context, true);
        } else if (context->authType == DmAuthType::AUTH_TYPE_PIN_ULTRASONIC) {
            return NegotiatePinAuth(context, true);
        } else {
            return NegotiatePinAuth(context, false);
        }
    } else {
        return NegotiatePinAuth(context, false);
    }
    return ERR_DM_FAILED;
}

DmAuthStateType AuthSrcPinInputState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_PIN_INPUT_STATE;
}

int32_t AuthSrcPinInputState::ShowStartAuthDialog(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcPinInputState::ShowStartAuthDialog start.");
    if (DmAuthState::IsScreenLocked()) {
        LOGE("AuthSrcPinInputState screen is locked.");
        context->reason = ERR_DM_BIND_USER_CANCEL;
        return STOP_BIND;
    }

    DmDialogManager::GetInstance().ShowInputDialog(context->accessee.deviceName);
    LOGI("AuthSrcPinInputState::ShowStartAuthDialog end.");
    return DM_OK;
}

int32_t AuthSrcPinInputState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcPinInputState::Action start");
    if (context->inputPinAuthFailTimes == 0) {
        auto ret = ShowStartAuthDialog(context);
        if (ret != DM_OK) {
            return ret;
        }
    } else {
        // clear input pin box, and show try again
        context->authUiStateMgr->UpdateUiState(DmUiStateMsg::MSG_PIN_CODE_ERROR);
    }

    LOGI("AuthSrcPinInputState::Action waitting user operation");
    // wait for user operation
    if (DmEventType::ON_USER_OPERATION !=
        context->authStateMachine->WaitExpectEvent(DmEventType::ON_USER_OPERATION)) {
        LOGI("AuthSrcPinInputState::Action wait ON_USER_OPERATION err");
        return STOP_BIND;
    }

    if (context->pinInputResult != USER_OPERATION_TYPE_DONE_PINCODE_INPUT) {
        LOGE("AuthSrcPinInputState::Action not USER_OPERATION_TYPE_DONE_PINCODE_INPUT err");
        return STOP_BIND;
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinAuthStartState>());
    return DM_OK;
}

DmAuthStateType AuthSinkPinNegotiateStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE;
}

int32_t AuthSinkPinNegotiateStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    if (!context->pinNegotiateStarted) {
        context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));
        context->pinNegotiateStarted = true;
    } else {
        if (context->authType == DmAuthType::AUTH_TYPE_PIN &&
            context->inputPinAuthFailTimes < MAX_AUTH_INPUT_PIN_FAIL_TIMES) {
            LOGI("AuthSinkPinNegotiateStartState::Action input pin auth err, retry");
        } else {
            // try to fallback to next auth type
            auto idx = context->currentAuthTypeIdx;
            if (idx + 1 >= context->authTypeList.size()) {
                LOGE("AuthSinkPinNegotiateStartState::Action all auth type failed");
                context->reason = ERR_DM_AUTH_REJECT;
                return ERR_DM_AUTH_REJECT;
            }
            ++idx;
            context->currentAuthTypeIdx = idx;
            context->authType = context->authTypeList[idx];
        }
    }
    // restart pin auth timer
    context->timer->DeleteTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK));
    context->timer->StartTimer(std::string(WAIT_PIN_AUTH_TIMEOUT_TASK),
        DmAuthState::GetTaskTimeout(context, WAIT_PIN_AUTH_TIMEOUT_TASK, PIN_AUTH_TIMEOUT),
        [context] (std::string name) {
            HandleAuthenticateTimeout(context, name);
        });
    if (DmAuthState::IsImportAuthCodeCompatibility(context->authType)) {
        LOGI("AuthSinkPinNegotiateStartState::Action import auth code");
    } else if (context->authType == DmAuthType::AUTH_TYPE_PIN) {
        LOGI("AuthSinkPinNegotiateStartState::Action input pin");
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinDisplayState>());
    } else if (context->authType == DmAuthType::AUTH_TYPE_PIN_ULTRASONIC) {
        LOGI("AuthSinkPinNegotiateStartState::Action ultrasonic pin");
    } else {
        LOGE("AuthSinkPinNegotiateStartState::Action authType not support");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DmAuthStateType AuthSinkPinDisplayState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_PIN_DISPLAY_STATE;
}

int32_t AuthSinkPinDisplayState::Action(std::shared_ptr<DmAuthContext> context)
{
    if (context->inputPinAuthFailTimes == 0) {
        // gen pincode
        AuthSinkStatePinAuthComm::GeneratePincode(context);
        // show pincode
        return AuthSinkStatePinAuthComm::ShowAuthInfoDialog(context);
    }
    return DM_OK;
}

DmAuthStateType AuthSrcReverseUltrasonicStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_START_STATE;
}

int32_t AuthSrcReverseUltrasonicStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcReverseUltrasonicStartState::Action start");
    context->timer->StartTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK),
        GET_ULTRASONIC_PIN_TIMEOUT, [context] (std::string name) {
            LOGI("AuthSrcReverseUltrasonicStartState::Action timeout");
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
            return DM_OK;
        });
    context->pinCode = std::to_string(GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE));
    std::string ultraPinCode = context->pinCode;
#ifdef SUPPORT_MSDP
    Msdp::SpatialAwarenessMgrClient::GetInstance().SetPinCode(ultraPinCode);
#endif
    context->reply = DM_OK;
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REVERSE_ULTRASONIC_START, context);
    return DM_OK;
}

DmAuthStateType AuthSrcReverseUltrasonicDoneState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_DONE_STATE;
}

int32_t AuthSrcReverseUltrasonicDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcReverseUltrasonicDoneState::Action Start.");
    context->timer->DeleteTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK));
    int32_t osAccountId = MultipleUserConnector::GetCurrentAccountUserID();
    auto ret = context->hiChainAuthConnector->AuthCredentialPinCode(osAccountId, context->requestId,
        context->pinCode);
    if (ret != DM_OK) {
        LOGE("AuthSrcPinAuthStartState::AuthDevice failed.");
        return ret;
    }
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_PIN_AUTH_START, context);
        return DM_OK;
    } else if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSrcReverseUltrasonicDoneState::AuthDevice ON_ERROR failed.");
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_PIN_AUTH_START, context);
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
        return DM_OK;
    }
    return STOP_BIND;
}

DmAuthStateType AuthSrcForwardUltrasonicStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_START_STATE;
}

int32_t AuthSrcForwardUltrasonicStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcForwardUltrasonicStartState::Action Start.");
    context->reply = DM_OK;
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_FORWARD_ULTRASONIC_START, context);
    LOGI("AuthSrcForwardUltrasonicStartState::Action End.");
    return DM_OK;
}

DmAuthStateType AuthSrcForwardUltrasonicDoneState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_DONE_STATE;
}

int32_t AuthSrcForwardUltrasonicDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcForwardUltrasonicDoneState::Action Start.");
    context->timer->StartTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK),
        GET_ULTRASONIC_PIN_TIMEOUT, [context] (std::string name) {
            LOGI("AuthSrcForwardUltrasonicDoneState timeout.");
#ifdef SUPPORT_MSDP
            Msdp::SpatialAwarenessMgrClient::GetInstance().UnregisterPinCallback();
#endif
            context->authStateMachine->NotifyEventFinish(DmEventType::ON_ULTRASONIC_PIN_TIMEOUT);
        });
#ifdef SUPPORT_MSDP
    sptr<SpatialLocationCallbackImpl> callback = new(std::nothrow) SpatialLocationCallbackImpl(context);
    Msdp::SpatialAwarenessMgrClient::GetInstance().RegisterPinCallback(callback);
#endif
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_ULTRASONIC_PIN_CHANGED);
    if (retEvent == DmEventType::ON_ULTRASONIC_PIN_CHANGED) {
#ifdef SUPPORT_MSDP
        Msdp::SpatialAwarenessMgrClient::GetInstance().UnregisterPinCallback();
#endif
        auto ret = context->hiChainAuthConnector->AuthCredentialPinCode(context->accesser.userId, context->requestId,
            context->pinCode);
        if (ret != DM_OK) {
            LOGE("OnPinCodeChanged failed.");
            return STOP_BIND;
        }
        auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
        if (retEvent == DmEventType::ON_TRANSMIT) {
            LOGI("OnPinCodeChanged ON_TRANSMIT.");
            context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_PIN_AUTH_START, context);
            return DM_OK;
        } else if (retEvent == DmEventType::ON_ERROR) {
            LOGI("OnPinCodeChanged ON_ERROR failed.");
            context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_PIN_AUTH_START, context);
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
            return DM_OK;
        }
    } else if (retEvent == DmEventType::ON_ULTRASONIC_PIN_TIMEOUT) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
        return DM_OK;
    }
    return STOP_BIND;
}

DmAuthStateType AuthSinkReverseUltrasonicStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_START_STATE;
}

int32_t AuthSinkReverseUltrasonicStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkReverseUltrasonicStartState::Action Start.");
    context->timer->StartTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK),
        GET_ULTRASONIC_PIN_TIMEOUT, [context] (std::string name) {
            LOGI("AuthSinkReverseUltrasonicStartState timeout.");
#ifdef SUPPORT_MSDP
            Msdp::SpatialAwarenessMgrClient::GetInstance().UnregisterPinCallback();
#endif
            context->authStateMachine->NotifyEventFinish(DmEventType::ON_ULTRASONIC_PIN_TIMEOUT);
        });
#ifdef SUPPORT_MSDP
    sptr<SpatialLocationCallbackImpl> callback = new(std::nothrow) SpatialLocationCallbackImpl(context);
    Msdp::SpatialAwarenessMgrClient::GetInstance().RegisterPinCallback(callback);
#endif
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_ULTRASONIC_PIN_CHANGED);
    if (retEvent == DmEventType::ON_ULTRASONIC_PIN_CHANGED) {
#ifdef SUPPORT_MSDP
        Msdp::SpatialAwarenessMgrClient::GetInstance().UnregisterPinCallback();
#endif
        context->reply = DM_OK;
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REVERSE_ULTRASONIC_DONE, context);
        LOGI("OnPinCodeChanged End.");
        return DM_OK;
    } else if (retEvent == DmEventType::ON_ULTRASONIC_PIN_TIMEOUT) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
        return DM_OK;
    }
    return STOP_BIND;
}

DmAuthStateType AuthSinkReverseUltrasonicDoneState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_DONE_STATE;
}

int32_t AuthSinkReverseUltrasonicDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkReverseUltrasonicDoneState::Action Start.");
    context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));
    context->timer->DeleteTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK));
    context->pinNegotiateStarted = true;
    if (FreezeProcess::GetInstance().CleanFreezeRecord(context->accessee.bundleName,
        context->accessee.deviceType) != DM_OK) {
        LOGE("CleanFreezeRecord failed.");
    }
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthSinkPinAuthStartState::Action call ProcessCredData err");
        return ret;
    }
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        LOGI("AuthSrcPinAuthStartState::AuthDevice ON_TRANSMIT.");
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_PIN_AUTH_START, context);
        return DM_OK;
    }
    if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSrcPinAuthStartState::AuthDevice ON_ERROR failed.");
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
        return DM_OK;
    }
    return STOP_BIND;
}

DmAuthStateType AuthSinkForwardUltrasonicStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_START_STATE;
}

int32_t AuthSinkForwardUltrasonicStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkForwardUltrasonicStartState::Action Start.");
    context->timer->StartTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK),
        GET_ULTRASONIC_PIN_TIMEOUT, [context] (std::string name) {
            LOGI("AuthSinkForwardUltrasonicStartState timeout.");
            context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
            return DM_OK;
        });
    context->pinCode = std::to_string(GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE));
    std::string ultraPinCode = context->pinCode;
#ifdef SUPPORT_MSDP
    Msdp::SpatialAwarenessMgrClient::GetInstance().SetPinCode(ultraPinCode);
#endif
    context->reply = DM_OK;
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_FORWARD_ULTRASONIC_NEGOTIATE, context);
    LOGI("AuthSinkForwardUltrasonicStartState::Action End.");
    return DM_OK;
}

DmAuthStateType AuthSinkForwardUltrasonicDoneState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_DONE_STATE;
}

int32_t AuthSinkForwardUltrasonicDoneState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkForwardUltrasonicDoneState::Action Start.");
    context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));
    context->timer->DeleteTimer(std::string(GET_ULTRASONIC_PIN_TIMEOUT_TASK));
    context->pinNegotiateStarted = true;
    if (FreezeProcess::GetInstance().CleanFreezeRecord(context->accessee.bundleName,
        context->accessee.deviceType) != DM_OK) {
        LOGE("CleanFreezeRecord failed.");
    }
    auto ret = context->hiChainAuthConnector->ProcessCredData(context->requestId, context->transmitData);
    if (ret != DM_OK) {
        LOGE("AuthSinkForwardUltrasonicDoneState::Action call ProcessCredData err");
        return ret;
    }
    auto retEvent = context->authStateMachine->WaitExpectEvent(DmEventType::ON_TRANSMIT);
    if (retEvent == DmEventType::ON_TRANSMIT) {
        LOGI("AuthSinkForwardUltrasonicDoneState::AuthDevice ON_TRANSMIT.");
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_PIN_AUTH_START, context);
        return DM_OK;
    }
    if (retEvent == DmEventType::ON_ERROR) {
        LOGI("AuthSinkForwardUltrasonicDoneState::AuthDevice ON_ERROR failed.");
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
        return DM_OK;
    }
    return STOP_BIND;
}
} // namespace DistributedHardware
} // namespace OHOS