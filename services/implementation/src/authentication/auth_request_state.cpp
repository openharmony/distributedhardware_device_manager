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

#include "auth_request_state.h"

#include "dm_auth_manager.h"
#include "dm_error_type.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AuthRequestState::Leave()
{
    return DM_OK;
}

int32_t AuthRequestState::SetAuthManager(std::shared_ptr<DmAuthManager> authManager)
{
    authManager_ = std::move(authManager);
    return DM_OK;
}

int32_t AuthRequestState::SetAuthContext(std::shared_ptr<DmAuthRequestContext> context)
{
    context_ = std::move(context);
    return DM_OK;
}

std::shared_ptr<DmAuthRequestContext> AuthRequestState::GetAuthContext()
{
    return context_;
}

int32_t AuthRequestState::TransitionTo(std::shared_ptr<AuthRequestState> state)
{
    LOGI("AuthRequestState::TransitionTo");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<DmAuthRequestContext> contextTemp = GetAuthContext();
    state->SetAuthManager(stateAuthManager);
    stateAuthManager->SetAuthRequestState(state);
    state->SetAuthContext(contextTemp);
    this->Leave();
    state->Enter();
    return DM_OK;
}

int32_t AuthRequestInitState::GetStateType()
{
    return AuthState::AUTH_REQUEST_INIT;
}

int32_t AuthRequestInitState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->EstablishAuthChannel(context_->deviceId);
    return DM_OK;
}

int32_t AuthRequestNegotiateState::GetStateType()
{
    return AuthState::AUTH_REQUEST_NEGOTIATE;
}

int32_t AuthRequestNegotiateState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->StartNegotiate(context_->sessionId);
    return DM_OK;
}

int32_t AuthRequestNegotiateDoneState::GetStateType()
{
    return AuthState::AUTH_REQUEST_NEGOTIATE_DONE;
}

int32_t AuthRequestNegotiateDoneState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->SendAuthRequest(context_->sessionId);
    return DM_OK;
}

int32_t AuthRequestReplyState::GetStateType()
{
    return AuthState::AUTH_REQUEST_REPLY;
}

int32_t AuthRequestReplyState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->StartRespAuthProcess();
    return DM_OK;
}

int32_t AuthRequestJoinState::GetStateType()
{
    return AuthState::AUTH_REQUEST_JOIN;
}

int32_t AuthRequestJoinState::Enter()
{
    LOGI("DmAuthManager::AuthRequestJoinState");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->ShowStartAuthDialog();
    return DM_OK;
}

int32_t AuthRequestNetworkState::GetStateType()
{
    return AuthState::AUTH_REQUEST_NETWORK;
}

int32_t AuthRequestNetworkState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->JoinNetwork();
    return DM_OK;
}

int32_t AuthRequestFinishState::GetStateType()
{
    return AuthState::AUTH_REQUEST_FINISH;
}

int32_t AuthRequestFinishState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->AuthenticateFinish();
    return DM_OK;
}

// pkgbind
int32_t AuthRequestCredential::GetStateType()
{
    return AuthState::AUTH_REQUEST_CREDENTIAL;
}

int32_t AuthRequestCredential::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->RequestCredential();
    return DM_OK;
}

int32_t AuthRequestCredentialDone::GetStateType()
{
    return AuthState::AUTH_REQUEST_CREDENTIAL_DONE;
}

int32_t AuthRequestCredentialDone::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->RequestCredentialDone();
    return DM_OK;
}

int32_t AuthRequestAuthFinish::GetStateType()
{
    return AuthState::AUTH_REQUEST_AUTH_FINISH;
}

int32_t AuthRequestAuthFinish::Enter()
{
    return DM_OK;
}

int32_t AuthRequestReCheckMsg::GetStateType()
{
    return AuthState::AUTH_REQUEST_RECHECK_MSG;
}

int32_t AuthRequestReCheckMsg::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->RequestReCheckMsg();
    return DM_OK;
}

int32_t AuthRequestReCheckMsgDone::GetStateType()
{
    return AuthState::AUTH_REQUEST_RECHECK_MSG_DONE;
}

int32_t AuthRequestReCheckMsgDone::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return ERR_DM_FAILED;
    }
    stateAuthManager->RequestReCheckMsgDone();
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS