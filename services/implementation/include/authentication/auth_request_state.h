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

#ifndef OHOS_DM_AUTH_REQUEST_STATE_H
#define OHOS_DM_AUTH_REQUEST_STATE_H
#include <cstdint>
#include <memory>
#include <sstream>

#include "dm_log.h"
namespace OHOS {
namespace DistributedHardware {
class DmAuthManager;
struct DmAuthRequestContext;
class AuthRequestState : public std::enable_shared_from_this<AuthRequestState> {
public:
    virtual ~AuthRequestState()
    {
        authManager_.reset();
    };
    virtual int32_t GetStateType() = 0;
    virtual int32_t Enter() = 0;

    /**
     * @tc.name: AuthRequestState::Leave
     * @tc.desc: Leave of the Authenticate Request State
     * @tc.type: FUNC
     */
    int32_t Leave();

    /**
     * @tc.name: AuthRequestState::TransitionTo
     * @tc.desc: Transition of the Authenticate Request State
     * @tc.type: FUNC
     */
    int32_t TransitionTo(std::shared_ptr<AuthRequestState> state);
    /**
     * @tc.name: AuthRequestState::SetAuthManager
     * @tc.desc: Set AuthManager of the Authenticate Request State
     * @tc.type: FUNC
     */
    int32_t SetAuthManager(std::shared_ptr<DmAuthManager> authManager);
    /**
     * @tc.name: AuthRequestState::SetAuthContext
     * @tc.desc: Set Auth Context of the Authenticate Request State
     * @tc.type: FUNC
     */
    int32_t SetAuthContext(std::shared_ptr<DmAuthRequestContext> context);
    std::shared_ptr<DmAuthRequestContext> GetAuthContext();

protected:
    std::weak_ptr<DmAuthManager> authManager_;
    std::shared_ptr<DmAuthRequestContext> context_;
};

class AuthRequestInitState : public AuthRequestState {
public:
    /**
     * @tc.name: AuthRequestInitState::GetStateType
     * @tc.desc: Get State Type of the Authenticate Request Init State
     * @tc.type: FUNC
     */
    int32_t GetStateType() override;
    /**
     * @tc.name: AuthRequestInitState::Enter
     * @tc.desc: Enter of the Authenticate Request Init State
     * @tc.type: FUNC
     */
    int32_t Enter() override;
};

class AuthRequestNegotiateState : public AuthRequestState {
public:
    /**
     * @tc.name: AuthRequestNegotiateState::GetStateType
     * @tc.desc: Get State Type of the Authenticate Request Negotiate State
     * @tc.type: FUNC
     */
    int32_t GetStateType() override;
    /**
     * @tc.name: AuthRequestNegotiateState::Enter
     * @tc.desc: Enter of the Authenticate Request Negotiate State
     * @tc.type: FUNC
     */
    int32_t Enter() override;
};

class AuthRequestNegotiateDoneState : public AuthRequestState {
public:
    /**
     * @tc.name: AuthRequestNegotiateDoneState::GetStateType
     * @tc.desc: Get State Type of the Authenticate Request Negotiate Done State
     * @tc.type: FUNC
     */
    int32_t GetStateType() override;

    /**
     * @tc.name: AuthRequestNegotiateDoneState::Enter
     * @tc.desc: Enter of the Authenticate Request Negotiate Done State
     * @tc.type: FUNC
     */
    int32_t Enter() override;
};

class AuthRequestReplyState : public AuthRequestState {
public:
    /**
     * @tc.name: AuthRequestReplyState::GetStateType
     * @tc.desc: Get State Type of the AuthRequest Reply State
     * @tc.type: FUNC
     */
    int32_t GetStateType() override;

    /**
     * @tc.name: AuthRequestReplyState::Enter
     * @tc.desc: Enter of the AuthRequest Reply State
     * @tc.type: FUNC
     */
    int32_t Enter() override;
};

class AuthRequestJoinState : public AuthRequestState {
public:
    /**
     * @tc.name: AuthRequestJoinState::GetStateType
     * @tc.desc: Get State Type of the Auth Request Join State
     * @tc.type: FUNC
     */
    int32_t GetStateType() override;

    /**
     * @tc.name: AuthRequestJoinState::Enter
     * @tc.desc: Enter of the Auth Request Join State
     * @tc.type: FUNC
     */
    int32_t Enter() override;
};

class AuthRequestNetworkState : public AuthRequestState {
public:
    /**
     * @tc.name: AuthRequestNetworkState::GetStateType
     * @tc.desc: Get State Type of the AuthRequest Network State
     * @tc.type: FUNC
     */
    int32_t GetStateType() override;

    /**
     * @tc.name: AuthRequestNetworkState::Enter
     * @tc.desc: Enter of the AuthRequest Network State
     * @tc.type: FUNC
     */
    int32_t Enter() override;
};

class AuthRequestFinishState : public AuthRequestState {
public:
    /**
     * @tc.name: AuthRequestFinishState::GetStateType
     * @tc.desc: Get State Type of the Auth Request Finish State
     * @tc.type: FUNC
     */
    int32_t GetStateType() override;

    /**
     * @tc.name: AuthRequestFinishState::Enter
     * @tc.desc: Enter of the Auth Request Finish State
     * @tc.type: FUNC
     */
    int32_t Enter() override;
};

class AuthRequestCredential : public AuthRequestState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthRequestCredentialDone : public AuthRequestState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthRequestAuthFinish : public AuthRequestState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthRequestReCheckMsg : public AuthRequestState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};

class AuthRequestReCheckMsgDone : public AuthRequestState {
public:
    int32_t GetStateType() override;
    int32_t Enter() override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_REQUEST_STATE_H
