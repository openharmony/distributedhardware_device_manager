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

#ifndef OHOS_DM_AUTH_RESPONSE_STATE_H
#define OHOS_DM_AUTH_RESPONSE_STATE_H

#include <memory>

namespace OHOS {
namespace DistributedHardware {
class DmAuthManager;
struct DmAuthResponseContext;
class AuthResponseState : public std::enable_shared_from_this<AuthResponseState> {
public:
    virtual ~AuthResponseState()
    {
        authManager_.reset();
    };
    virtual int32_t GetStateType() = 0;
    virtual int32_t Enter() = 0;

    /**
     * @tc.name: AuthResponseState::Leave
     * @tc.desc: Leave of the Auth Response State
     * @tc.type: FUNC
     */
    int32_t Leave();

    /**
     * @tc.name: AuthResponseState::TransitionTo
     * @tc.desc: Transition of the Auth Response State
     * @tc.type: FUNC
     */
    int32_t TransitionTo(std::shared_ptr<AuthResponseState> state);

    /**
     * @tc.name: AuthResponseState::SetAuthManager
     * @tc.desc: Set Auth Manager of the Auth Response State
     * @tc.type: FUNC
     */
    int32_t SetAuthManager(std::shared_ptr<DmAuthManager> authManager);

    /**
     * @tc.name: AuthResponseState::SetAuthContext
     * @tc.desc: Set Auth Context of the Auth Response State
     * @tc.type: FUNC
     */
    int32_t SetAuthContext(std::shared_ptr<DmAuthResponseContext> context);

    /**
     * @tc.name: AuthResponseState::GetAuthContext
     * @tc.desc: Get Auth Context of the Auth Response State
     * @tc.type: FUNC
     */
    std::shared_ptr<DmAuthResponseContext> GetAuthContext();

protected:
    std::weak_ptr<DmAuthManager> authManager_;
    std::shared_ptr<DmAuthResponseContext> context_;
};

class AuthResponseInitState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseNegotiateState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseConfirmState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseGroupState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseShowState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseFinishState : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseCredential : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseAuthFinish : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthResponseReCheckMsg : public AuthResponseState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_RESPONSE_STATE_H