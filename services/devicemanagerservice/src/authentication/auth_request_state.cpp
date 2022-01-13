/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {

void AuthRequestState::Leave()
{
}

void AuthRequestState::SetAuthManager(std::shared_ptr<DmAuthManager> authManager)
{
    authManager_ = std::move(authManager);
}

void AuthRequestState::SetAuthContext(std::shared_ptr<DmAuthRequestContext> context)
{
    context_ = std::move(context);
}

std::shared_ptr<DmAuthRequestContext> AuthRequestState::GetAuthContext()
{
    return context_;
}

void AuthRequestState::TransitionTo(std::shared_ptr<AuthRequestState> state)
{
    LOGE("AuthRequestState::TransitionTo");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    state->SetAuthManager(stateAuthManager);
    stateAuthManager->SetAuthRequestState(state);
    state->SetAuthContext(context_);
    this->Leave();
    state->Enter();
}

int32_t AuthRequestInitState::GetStateType()
{
    return AuthState::AUTH_REQUEST_INIT;
}

void AuthRequestInitState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->EstablishAuthChannel(context_->deviceId);
}

int32_t AuthRequestNegotiateState::GetStateType()
{
    return AuthState::AUTH_REQUEST_NEGOTIATE;
}

void AuthRequestNegotiateState::Enter()
{
    //    //1. 检查加解密模块是否存在并获取加解密的名称和版本信息
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->StartNegotiate(context_->sessionId);
}

int32_t AuthRequestNegotiateDoneState::GetStateType()
{
    return AuthState::AUTH_REQUEST_NEGOTIATE_DONE;
}

void AuthRequestNegotiateDoneState::Enter()
{
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->SendAuthRequest(context_->sessionId);
}

int32_t AuthRequestReplyState::GetStateType()
{
    return AuthState::AUTH_REQUEST_REPLY;
}

void AuthRequestReplyState::Enter()
{
    // 1. 收到请求响应，判断用户响应结果

    // 2. 用户授权同意

    // 3. 回调给认证实现模块，启动认证
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->StartRespAuthProcess();
}

int32_t AuthRequestInputState::GetStateType()
{
    return AuthState::AUTH_REQUEST_INPUT;
}

void AuthRequestInputState::Enter()
{
    //    //1. 获取用户输入
    //
    //    //2. 验证认证信息
    LOGE("DmAuthManager::AuthRequestInputState");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->ShowStartAuthDialog();
}

int32_t AuthRequestJoinState::GetStateType()
{
    return AuthState::AUTH_REQUEST_JOIN;
}

void AuthRequestJoinState::Enter()
{
    // 1. 加入群组
    LOGE("DmAuthManager::AuthRequestJoinState");
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->AddMember(context_->deviceId);
}

int32_t AuthRequestNetworkState::GetStateType()
{
    return AuthState::AUTH_REQUEST_NETWORK;
}

void AuthRequestNetworkState::Enter()
{
    // 1. 进行组网
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->JoinNetwork();
}

int32_t AuthRequestFinishState::GetStateType()
{
    return AuthState::AUTH_REQUEST_FINISH;
}

void AuthRequestFinishState::Enter()
{
    // 1. 清理资源
    // 2. 通知对端认证结束，并关闭认证通道
    std::shared_ptr<DmAuthManager> stateAuthManager = authManager_.lock();
    if (stateAuthManager == nullptr) {
        LOGE("AuthRequestState::authManager_ null");
        return;
    }
    stateAuthManager->AuthenticateFinish();
}
} // namespace DistributedHardware
} // namespace OHOS
