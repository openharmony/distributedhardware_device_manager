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

#include "UTTest_auth_pincode_state.h"

#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_state_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_constants_3rd.h"
#include "auth_pincode_manager_3rd.h"
#include "dm_timer_3rd.h"
#include "softbus_session_3rd.h"
#include "hichain_auth_connector_3rd.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
std::shared_ptr<DmAuthPincodeContext> context_;
std::shared_ptr<SoftbusConnector3rd> mockSoftbusConnector_;
std::shared_ptr<HiChainAuthConnector3rd> mockHiChainAuthConnector_;
std::shared_ptr<SoftbusSession3rd> mockSession_;
std::shared_ptr<DmTimer3rd> timer_;
}

void AuthPincodeStateTest::SetUpTestCase()
{
    LOGI("AuthPincodeStateTest::SetUpTestCase");
}

void AuthPincodeStateTest::TearDownTestCase()
{
    LOGI("AuthPincodeStateTest::TearDownTestCase");
}

void AuthPincodeStateTest::SetUp()
{
    LOGI("AuthPincodeStateTest::SetUp");
    context_ = std::make_shared<DmAuthPincodeContext>();
    context_->direction = DM_AUTH_PINCODE_SOURCE;
    context_->sessionId = 100;
    context_->logicalSessionId = 12345;
    context_->requestId = 12345;
    context_->reason = DM_OK;

    mockSoftbusConnector_ = std::make_shared<SoftbusConnector3rd>();
    mockHiChainAuthConnector_ = std::make_shared<HiChainAuthConnector3rd>();
    mockSession_ = std::make_shared<SoftbusSession3rd>();
    timer_ = std::make_shared<DmTimer3rd>();

    context_->softbusConnector = mockSoftbusConnector_;
    context_->hiChainAuthConnector = mockHiChainAuthConnector_;
    context_->timer = timer_;
    context_->authPinStateMac3rd = std::make_shared<DmAuthPincodeStateMachine3rd>(context_);
    context_->authPinMsgProc3rd = std::make_shared<DmAuthPincodeMessageProcessor3rd>();
}

void AuthPincodeStateTest::TearDown()
{
    LOGI("AuthPincodeStateTest::TearDown");
    if (context_ != nullptr && context_->authPinStateMac3rd != nullptr) {
        context_->authPinStateMac3rd->Stop();
    }
    if (timer_ != nullptr) {
        timer_->DeleteAll();
    }
    context_ = nullptr;
    mockSoftbusConnector_ = nullptr;
    mockHiChainAuthConnector_ = nullptr;
    mockSession_ = nullptr;
    timer_ = nullptr;
}

/**
 * @tc.name: AuthPincodeSrcStartState_GetStateType_001
 * @tc.desc: Test AuthPincodeSrcStartState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcStartState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_START_STATE);
}

/**
 * @tc.name: AuthPincodeSrcStartState_Action_001
 * @tc.desc: Test AuthPincodeSrcStartState Action
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcStartState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeSrcStartState_Action_002
 * @tc.desc: Test AuthPincodeSrcStartState Action with null context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcStartState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeSrcPinNegotiateStartState_GetStateType_001
 * @tc.desc: Test AuthPincodeSrcPinNegotiateStartState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinNegotiateStartState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinNegotiateStartState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_NEGOTIATE_STATE);
}

/**
 * @tc.name: AuthPincodeSrcPinNegotiateStartState_Action_001
 * @tc.desc: Test AuthPincodeSrcPinNegotiateStartState Action with valid context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinNegotiateStartState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinNegotiateStartState>();

    context_->accesser.deviceId = "test_device_id";
    context_->accesser.accountId = "test_account_id";
    context_->accesser.tokenId = 12345;

    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeSrcPinNegotiateStartState_Action_002
 * @tc.desc: Test AuthPincodeSrcPinNegotiateStartState Action with null context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinNegotiateStartState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinNegotiateStartState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSinkPinNegotiateStartState_GetStateType_001
 * @tc.desc: Test AuthPincodeSinkPinNegotiateStartState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinNegotiateStartState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinNegotiateStartState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_NEGOTIATE_STATE);
}

/**
 * @tc.name: AuthPincodeSrcPinAuthStartState_GetStateType_001
 * @tc.desc: Test AuthPincodeSrcPinAuthStartState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthStartState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthStartState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_START_STATE);
}

/**
 * @tc.name: AuthPincodeSrcPinAuthStartState_Action_001
 * @tc.desc: Test AuthPincodeSrcPinAuthStartState Action with valid context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthStartState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthStartState>();

    context_->importAuthCode = "123456";
    context_->accesser.userId = 100;
    context_->hiChainAuthConnector = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSrcPinAuthStartState_Action_002
 * @tc.desc: Test AuthPincodeSrcPinAuthStartState Action with null context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthStartState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthStartState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSinkPinAuthStartState_GetStateType_001
 * @tc.desc: Test AuthPincodeSinkPinAuthStartState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthStartState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthStartState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_START_STATE);
}

/**
 * @tc.name: AuthPincodeSinkPinAuthStartState_Action_001
 * @tc.desc: Test AuthPincodeSinkPinAuthStartState Action with null context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthStartState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthStartState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSrcPinAuthMsgNegotiateState_GetStateType_001
 * @tc.desc: Test AuthPincodeSrcPinAuthMsgNegotiateState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthMsgNegotiateState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthMsgNegotiateState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE);
}

/**
 * @tc.name: AuthPincodeSinkPinAuthMsgNegotiateState_GetStateType_001
 * @tc.desc: Test AuthPincodeSinkPinAuthMsgNegotiateState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthMsgNegotiateState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthMsgNegotiateState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE);
}

/**
 * @tc.name: AuthPincodeSrcPinAuthDoneState_GetStateType_001
 * @tc.desc: Test AuthPincodeSrcPinAuthDoneState GetStateType
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthDoneState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthDoneState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_DONE_STATE);
}

/**
 * @tc.name: AuthPincodeSrcPinAuthDoneState_Action_001
 * @tc.desc: Test AuthPincodeSrcPinAuthDoneState Action with null context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthDoneState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthDoneState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_001
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with valid task name
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_001, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, AUTHENTICATE_TIMEOUT_TASK, 10000);
    EXPECT_GT(timeout, 0);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_002
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with invalid task name
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_002, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, "invalid_task", 5000);
    EXPECT_EQ(timeout, 5000);
}

/**
 * @tc.name: DmAuthPincodeState_HandleAuthenticateTimeout_001
 * @tc.desc: Test DmAuthPincodeState3rd::HandleAuthenticateTimeout
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_HandleAuthenticateTimeout_001, TestSize.Level1)
{
    DmAuthPincodeState3rd::HandleAuthenticateTimeout(context_, "test_task");
    EXPECT_EQ(context_->reason, ERR_DM_TIME_OUT);
}

/**
 * @tc.name: AuthPincodeSinkFinishState_Action_001
 * @tc.desc: Test AuthPincodeSinkFinishState Action with null context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkFinishState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkFinishState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSinkFinishState_Action_002
 * @tc.desc: Test AuthPincodeSinkFinishState Action with valid context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkFinishState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkFinishState>();
    context_->reason = DM_OK;
    context_->state = 1;

    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeSrcFinishState_Action_001
 * @tc.desc: Test AuthPincodeSrcFinishState Action with null context
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcFinishState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcFinishState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSrcFinishState_Action_002
 * @tc.desc: Test AuthPincodeSrcFinishState Action with valid context and success reason
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcFinishState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcFinishState>();
    context_->reason = DM_OK;
    context_->state = 1;
    context_->connDelayCloseTime = 1000;

    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeSrcFinishState_Action_003
 * @tc.desc: Test AuthPincodeSrcFinishState Action with error reason
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcFinishState_Action_003, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcFinishState>();
    context_->reason = ERR_DM_FAILED;
    context_->state = 1;
    context_->reply = 1;
    context_->connDelayCloseTime = 1000;

    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context_->connDelayCloseTime, 0);
}

} // namespace DistributedHardware
} // namespace OHOS