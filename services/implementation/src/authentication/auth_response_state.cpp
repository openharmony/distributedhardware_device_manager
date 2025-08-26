/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "auth_response_state.h"

#include "dm_auth_manager.h"
#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AuthResponseState::Leave()
{
    return DM_OK;
}

int32_t AuthResponseState::SetAuthContext(std::shared_ptr<DmAuthResponseContext> context)
{
    context_ = std::move(context);
    return DM_OK;
}

std::shared_ptr<DmAuthResponseContext> AuthResponseState::GetAuthContext()
{
    return context_;
}

int32_t AuthResponseState::SetAuthManager(std::shared_ptr<DmAuthManager> authManager)
{
    authManager_ = std::move(authManager);
    return DM_OK;
}

int32_t AuthResponseState::TransitionTo(std::shared_ptr<AuthResponseState> state) __attribute__((no_sanitize("cfi")))
{
    LOGI("AuthResponseState::TransitionTo");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseState::authManager_ null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<DmAuthResponseContext> contextTemp = GetAuthContext();
    CHECK_NULL_RETURN(state, ERR_DM_FAILED);
    state->SetAuthManager(stateAuthManager);
    stateAuthManager->SetAuthResponseState(state);
    state->SetAuthContext(contextTemp);
    this->Leave();
    state->Enter();
    return DM_OK;
}

int32_t AuthResponseInitState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_INIT;
}

int32_t AuthResponseInitState::Enter()
{
    LOGI("AuthResponse::AuthResponseInitState Enter");
    return DM_OK;
}

int32_t AuthResponseNegotiateState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_NEGOTIATE;
}

int32_t AuthResponseNegotiateState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseNegotiateState authManager_ is null");
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context_, ERR_DM_FAILED);
    stateAuthManager->RespNegotiate(context_->sessionId);
    return DM_OK;
}

int32_t AuthResponseConfirmState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_CONFIRM;
}

int32_t AuthResponseConfirmState::Enter()
{
    LOGI("AuthResponse::AuthResponseConfirmState Enter");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseConfirmState authManager_ is null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->ShowConfigDialog();
    return DM_OK;
}

int32_t AuthResponseGroupState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_GROUP;
}

int32_t AuthResponseGroupState::Enter()
{
    LOGI("AuthResponse::AuthResponseGroupState Enter");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseGroupState authManager_ is null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->CreateGroup();
    return DM_OK;
}

int32_t AuthResponseShowState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_SHOW;
}

int32_t AuthResponseShowState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseShowState authManager_ is null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->ShowAuthInfoDialog();
    return DM_OK;
}

int32_t AuthResponseFinishState::GetStateType()
{
    return AuthState::AUTH_RESPONSE_FINISH;
}

int32_t AuthResponseFinishState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseFinishState authManager_ is null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->AuthenticateFinish();
    return DM_OK;
}

int32_t AuthResponseCredential::GetStateType()
{
    return AuthState::AUTH_RESPONSE_CREDENTIAL;
}

int32_t AuthResponseCredential::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseCredential authManager_ is null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->ResponseCredential();
    return DM_OK;
}

int32_t AuthResponseAuthFinish::GetStateType()
{
    return AuthState::AUTH_RESPONSE_AUTH_FINISH;
}

int32_t AuthResponseAuthFinish::Enter()
{
    return DM_OK;
}

int32_t AuthResponseReCheckMsg::GetStateType()
{
    return AuthState::AUTH_RESPONSE_RECHECK_MSG;
}

int32_t AuthResponseReCheckMsg::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthResponseCredential authManager_ is null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->ResponseReCheckMsg();
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
