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
#include "access_control_profile.h"
#include "accesser.h"
#include "accessee.h"
#include "auth_manager_3rd.h"
#include "app_manager_3rd.h"
#include "business_event.h"
#include "distributed_device_profile_client.h"
#include "dm_auth_message_processor_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_context_3rd.h"
#include "dm_auth_state_3rd.h"
#include "deviceprofile_connector_3rd.h"
#include "distributed_device_profile_errors.h"
#include "device_auth.h"
#include "hap_token_info.h"
#include "json_object.h"
#include "multiple_user_connector_3rd.h"
#include "os_account_manager.h"
#include "parameter.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {

DmAuthStateType AuthSrcStartState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SRC_START_STATE;
}

int32_t AuthSrcStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    return DM_OK;
}

DmAuthStateType AuthSrcPinNegotiateStartState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SRC_NEGOTIATE_STATE;
}

int32_t AuthSrcPinNegotiateStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->softbusConnector->GetSoftbusSession(), ERR_DM_POINT_NULL);
    LOGI("AuthSrcPinNegotiateStartState::Action sessionId %{public}d.", context->sessionId);
    context->reply = ERR_DM_AUTH_REJECT;
    context->accesser.deviceIdHash = Crypto3rd::GetUdidHash(context->accesser.deviceId);
    context->accesser.accountIdHash = Crypto3rd::GetAccountIdHash16(context->accesser.accountId);
    context->accesser.tokenIdHash = Crypto3rd::GetTokenIdHash(std::to_string(context->accesser.tokenId));

    std::string message = context->authMessageProcessor->CreateMessage(DmMessageType::ACL_REQ_NEGOTIATE, context);
    context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
    if (context->timer != nullptr) {
        context->timer->StartTimer(std::string(NEGOTIATE_TIMEOUT_TASK),
            DmAuthState3rd::GetTaskTimeout(context, NEGOTIATE_TIMEOUT_TASK, NEGOTIATE_TIMEOUT),
            [this, context] (std::string name) {
                DmAuthState3rd::HandleAuthenticateTimeout(context, name);
            });
    }
    return DM_OK;
}

DmAuthStateType AuthSinkPinNegotiateStartState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SINK_NEGOTIATE_STATE;
}

int32_t AuthSinkPinNegotiateStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->timer, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    LOGI("AuthSinkNegotiateStateMachine::Action sessionid %{public}d", context->sessionId);
    // 1. Create an authorization timer
    context->timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK),
        AUTHENTICATE_TIMEOUT,
        [this, context] (std::string name) {
            DmAuthState3rd::HandleAuthenticateTimeout(context, name);
    });
    int32_t ret = RespQueryAcceseeIds(context);
    if (ret != DM_OK) {
        LOGE("AuthSinkNegotiateStateMachine::Action proc response negotiate failed");
        context->reason = ret;
        return ret;
    }
    context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_RESP_NEGOTIATE, context);
    context->timer->StartTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK),
        DmAuthState3rd::GetTaskTimeout(context, WAIT_REQUEST_TIMEOUT_TASK, WAIT_REQUEST_TIMEOUT),
        [this, context] (std::string name) {
            DmAuthState3rd::HandleAuthenticateTimeout(context, name);
        });
    return DM_OK;
}

int32_t AuthSinkPinNegotiateStartState::RespQueryAcceseeIds(std::shared_ptr<DmAuthContext> context)
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
        context->accessee.bindLevel = DmRole::DM_ROLE_SA;
    } else if (AppManager3rd::GetInstance().GetHapTokenIdByName(context->accessee.userId,
        context->accessee.processName, 0, context->accessee.tokenId) == DM_OK) {
        context->accessee.bindLevel = DmRole::DM_ROLE_FA;
    } else {
        LOGE("sink not contain the bundlename %{public}s.", context->accessee.processName.c_str());
        return ERR_DM_GET_TOKENID_FAILED;
    }
    context->accessee.tokenIdHash = Crypto3rd::GetTokenIdHash(std::to_string(context->accessee.tokenId));
    context->processInfo.tokenId = context->accessee.tokenId;
    context->processInfo.uid = context->accessee.uid;
    context->processInfo.userId = context->accessee.userId;
    context->processInfo.processName = context->accessee.processName;
    context->processInfo.businessName = context->accessee.businessName;
    return RespQueryProxyAcceseeIds(context);
}

int32_t AuthSinkPinNegotiateStartState::RespQueryProxyAcceseeIds(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (context->subjectProxyOnes.empty()) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        if (AppManager3rd::GetInstance().GetNativeTokenIdByName(item->proxyAccessee.processName,
            item->proxyAccessee.tokenId) == DM_OK) {
            item->proxyAccessee.bindLevel = DmRole::DM_ROLE_SA;
        } else if (AppManager3rd::GetInstance().GetHapTokenIdByName(context->accessee.userId,
            item->proxyAccessee.processName, 0, item->proxyAccessee.tokenId) == DM_OK) {
            item->proxyAccessee.bindLevel = DmRole::DM_ROLE_FA;
        } else {
            LOGE("sink not contain the bundlename %{public}s.", item->proxyAccessee.processName.c_str());
            return ERR_DM_GET_TOKENID_FAILED;
        }
        item->proxyAccessee.tokenIdHash = Crypto3rd::GetTokenIdHash(std::to_string(item->proxyAccessee.tokenId));
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS