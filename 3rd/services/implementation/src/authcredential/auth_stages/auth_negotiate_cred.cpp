/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See License for the specific language governing permissions and
 * limitations under License.
 */

#include <cstdlib>
#include <map>
#include <memory>

#include "accesstoken_kit.h"
#include "auth_manager_cred.h"
#include "app_manager_3rd.h"
#include "business_event.h"
#include "dm_auth_message_processor_cred.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_context_cred.h"
#include "dm_auth_state_cred.h"
#include "dm_auth_state_machine_cred.h"
#include "device_auth.h"
#include "hap_token_info.h"
#include "json_object.h"
#include "multiple_user_connector_3rd.h"
#include "os_account_manager.h"
#include "parameter.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {

DmAuthStateType AuthCredSrcStartState::GetStateType()
{
    return DmAuthStateType::CRED_AUTH_SRC_START_STATE;
}

int32_t AuthCredSrcStartState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    return DM_OK;
}

DmAuthStateType AuthSrcNegotiateStartState::GetStateType()
{
    return DmAuthStateType::CRED_AUTH_SRC_NEGOTIATE_STATE;
}

int32_t AuthSrcNegotiateStartState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->softbusConnector->GetSoftbusSession(), ERR_DM_POINT_NULL);
    LOGI("Action sessionId %{public}d.", context->sessionId);
    context->reply = ERR_DM_AUTH_REJECT;
    context->accesser.deviceIdHash = Crypto3rd::GetUdidHash(context->accesser.deviceId);
    context->accesser.accountIdHash = Crypto3rd::GetAccountIdHash16(context->accesser.accountId);
    context->accesser.tokenIdHash = Crypto3rd::GetTokenIdHash(std::to_string(context->accesser.tokenId));

    std::string message = context->authMessageProcessor->CreateMessage(DmCredMessageType::CRED_REQ_NEGOTIATE, context);
    context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
    if (context->timer != nullptr) {
        context->timer->StartTimer(std::string(NEGOTIATE_TIMEOUT_TASK),
            DmAuthStateCred::GetTaskTimeout(context, NEGOTIATE_TIMEOUT_TASK, NEGOTIATE_TIMEOUT),
            [this, context] (std::string name) {
                DmAuthStateCred::HandleAuthenticateTimeout(context, name);
            });
    }
    return DM_OK;
}

DmAuthStateType AuthSinkNegotiateStartState::GetStateType()
{
    return DmAuthStateType::CRED_AUTH_SINK_NEGOTIATE_STATE;
}

int32_t AuthSinkNegotiateStartState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->timer, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    LOGI("Action sessionid %{public}d", context->sessionId);
    // 1. Create an authorization timer
    context->timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK),
        AUTHENTICATE_TIMEOUT,
        [this, context] (std::string name) {
            DmAuthStateCred::HandleAuthenticateTimeout(context, name);
    });
    int32_t ret = RespQueryAcceseeIds(context);
    if (ret != DM_OK) {
        LOGE("AuthSinkNegotiateStateMachine::Action proc response negotiate failed");
        context->reason = ret;
        return ret;
    }
    context->authMessageProcessor->CreateAndSendMsg(DmCredMessageType::CRED_RESP_NEGOTIATE, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialAuthStartState>());
    context->timer->StartTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK),
        DmAuthStateCred::GetTaskTimeout(context, WAIT_REQUEST_TIMEOUT_TASK, WAIT_REQUEST_TIMEOUT),
        [this, context] (std::string name) {
            DmAuthStateCred::HandleAuthenticateTimeout(context, name);
        });
    return DM_OK;
}

int32_t AuthSinkNegotiateStartState::RespQueryAcceseeIds(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    // 1. Get deviceId
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    context->accessee.deviceId = std::string(localDeviceId);
    context->accessee.deviceIdHash = Crypto3rd::GetUdidHash(context->accessee.deviceId);
    // 2. Get userId
    context->accessee.userId = MultipleUserConnector3rd::GetFirstForegroundUserId();
    if (context->accessee.userId < 0) {
        LOGE("get accessee userId failed.");
        return ERR_DM_GET_LOCAL_USERID_FAILED;
    }
    // 3. Get accountId
    context->accessee.accountId = MultipleUserConnector3rd::GetOhosAccountIdByUserId(context->accessee.userId);
    context->accessee.accountIdHash = Crypto3rd::GetAccountIdHash16(context->accessee.accountId);
    // 4. Get tokenId
    if (AppManager3rd::GetInstance().GetNativeTokenIdByName(context->accessee.processName,
        context->accessee.tokenId) == DM_OK) {
    } else if (AppManager3rd::GetInstance().GetHapTokenIdByName(context->accessee.userId,
        context->accessee.processName, 0, context->accessee.tokenId) == DM_OK) {
    } else {
        LOGE("sink not contain the bundlename %{public}s.", context->accessee.processName.c_str());
        return ERR_DM_GET_TOKENID_FAILED;
    }
    int32_t ret = QueryCredential(context);
    if (ret != DM_OK) {
        LOGE("QueryCredential failed: %{public}d.", ret);
        return ret;
    }
    context->accessee.tokenIdHash = Crypto3rd::GetTokenIdHash(std::to_string(context->accessee.tokenId));
    context->processInfo.tokenId = context->accessee.tokenId;
    context->processInfo.uid = context->accessee.uid;
    context->processInfo.userId = context->accessee.userId;
    context->processInfo.processName = context->accessee.processName;
    context->processInfo.businessName = context->accessee.businessName;
    return DM_OK;
}

DmAuthStateType AuthSinkCredentialAuthStartState::GetStateType()
{
    return DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_START_STATE;
}

int32_t AuthSinkCredentialAuthStartState::Action(std::shared_ptr<DmAuthCredContext> context)
{
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS