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
#include <algorithm>

#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_state_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_constants_3rd.h"
#include "auth_pincode_manager_3rd.h"
#include "dm_timer_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_auth_info_3rd.h"
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

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_201
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with NEGOTIATE_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_201, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, NEGOTIATE_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_NEGOTIATE_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_202
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with WAIT_PIN_AUTH_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_202, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, WAIT_PIN_AUTH_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_PIN_AUTH_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_203
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with SESSION_HEARTBEAT_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_203, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, SESSION_HEARTBEAT_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_SESSION_HEARTBEAT_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_BuildResultContent_001
 * @tc.desc: Test DmAuthPincodeState3rd::BuildResultContent with error reason (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_BuildResultContent_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    context_->reason = ERR_DM_FAILED;
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    state->BuildResultContent(context_, deviceInfos);
    EXPECT_TRUE(deviceInfos.empty());
}

/**
 * @tc.name: DmAuthPincodeState_BuildResultContent_002
 * @tc.desc: Test DmAuthPincodeState3rd::BuildResultContent with empty sessionKey (invalid len branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_BuildResultContent_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    context_->reason = DM_OK;
    context_->accesser.sessionKey.clear();
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    state->BuildResultContent(context_, deviceInfos);
    EXPECT_TRUE(deviceInfos.empty());
}

/**
 * @tc.name: DmAuthPincodeState_BuildResultContent_003
 * @tc.desc: Test DmAuthPincodeState3rd::BuildResultContent with oversized sessionKey (> MAX_SESSION_KEY_LENGTH)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_BuildResultContent_003, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    context_->reason = DM_OK;
    // MAX_SESSION_KEY_LENGTH is 512; exceed it to hit the len > MAX_SESSION_KEY_LENGTH branch
    context_->accesser.sessionKey.assign(513, 0xAB);
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    state->BuildResultContent(context_, deviceInfos);
    EXPECT_TRUE(deviceInfos.empty());
}

/**
 * @tc.name: DmAuthPincodeState_BuildResultContent_004
 * @tc.desc: Test DmAuthPincodeState3rd::BuildResultContent with null context (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_BuildResultContent_004, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    state->BuildResultContent(nullptr, deviceInfos);
    EXPECT_TRUE(deviceInfos.empty());
}

/**
 * @tc.name: DmAuthPincodeState_BuildResultContent_005
 * @tc.desc: Test DmAuthPincodeState3rd::BuildResultContent with valid sessionKey (memcpy success push branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_BuildResultContent_005, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    context_->reason = DM_OK;
    context_->accesser.sessionKey = {0x01, 0x02, 0x03, 0x04};
    context_->accesser.businessName = "test_business";
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    state->BuildResultContent(context_, deviceInfos);
    ASSERT_EQ(deviceInfos.size(), 1u);
    EXPECT_EQ(deviceInfos[0].sessionKey.keyLen, 4u);
    EXPECT_EQ(deviceInfos[0].businessName, "test_business");
    FreeDeviceInfos(deviceInfos);
}

/**
 * @tc.name: DmAuthPincodeState_BuildResultContent_006
 * @tc.desc: Test BuildResultContent with sink direction selecting accessee self access branch
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_BuildResultContent_006, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    context_->reason = DM_OK;
    context_->direction = DM_AUTH_PINCODE_SINK;
    context_->accessee.sessionKey = {0x10, 0x20, 0x30};
    context_->accessee.businessName = "sink_business";
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    state->BuildResultContent(context_, deviceInfos);
    ASSERT_EQ(deviceInfos.size(), 1u);
    EXPECT_EQ(deviceInfos[0].sessionKey.keyLen, 3u);
    EXPECT_EQ(deviceInfos[0].businessName, "sink_business");
    FreeDeviceInfos(deviceInfos);
}

/**
 * @tc.name: AuthPincodeSinkPinAuthStartState_Action_201
 * @tc.desc: Test AuthPincodeSinkPinAuthStartState Action with null timer (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthStartState_Action_201, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthStartState>();
    std::shared_ptr<DmTimer3rd> savedTimer = context_->timer;
    context_->timer = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->timer = savedTimer;
}

/**
 * @tc.name: AuthPincodeSinkPinAuthStartState_Action_202
 * @tc.desc: Test AuthPincodeSinkPinAuthStartState Action with null hiChainAuthConnector (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthStartState_Action_202, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthStartState>();
    std::shared_ptr<HiChainAuthConnector3rd> savedConnector = context_->hiChainAuthConnector;
    context_->hiChainAuthConnector = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->hiChainAuthConnector = savedConnector;
}

/**
 * @tc.name: AuthPincodeSinkPinAuthStartState_Action_203
 * @tc.desc: Test AuthPincodeSinkPinAuthStartState Action with null authPinMsgProc3rd (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthStartState_Action_203, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthStartState>();
    std::shared_ptr<DmAuthPincodeMessageProcessor3rd> savedProc = context_->authPinMsgProc3rd;
    context_->authPinMsgProc3rd = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->authPinMsgProc3rd = savedProc;
}

/**
 * @tc.name: AuthPincodeSrcPinAuthMsgNegotiateState_Action_001
 * @tc.desc: Test AuthPincodeSrcPinAuthMsgNegotiateState Action with null context (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthMsgNegotiateState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthMsgNegotiateState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSrcPinAuthMsgNegotiateState_Action_002
 * @tc.desc: Test AuthPincodeSrcPinAuthMsgNegotiateState Action with null hiChainAuthConnector
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthMsgNegotiateState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthMsgNegotiateState>();
    std::shared_ptr<HiChainAuthConnector3rd> savedConnector = context_->hiChainAuthConnector;
    context_->hiChainAuthConnector = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->hiChainAuthConnector = savedConnector;
}

/**
 * @tc.name: AuthPincodeSinkPinAuthMsgNegotiateState_Action_001
 * @tc.desc: Test AuthPincodeSinkPinAuthMsgNegotiateState Action with null context (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthMsgNegotiateState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthMsgNegotiateState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSinkPinAuthMsgNegotiateState_Action_002
 * @tc.desc: Test AuthPincodeSinkPinAuthMsgNegotiateState Action with null authPinStateMac3rd
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinAuthMsgNegotiateState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinAuthMsgNegotiateState>();
    std::shared_ptr<DmAuthPincodeStateMachine3rd> savedMac = context_->authPinStateMac3rd;
    context_->authPinStateMac3rd = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->authPinStateMac3rd = savedMac;
}

/**
 * @tc.name: AuthPincodeSrcPinAuthDoneState_Action_002
 * @tc.desc: Test AuthPincodeSrcPinAuthDoneState Action with null authPinMsgProc3rd (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinAuthDoneState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinAuthDoneState>();
    std::shared_ptr<DmAuthPincodeMessageProcessor3rd> savedProc = context_->authPinMsgProc3rd;
    context_->authPinMsgProc3rd = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->authPinMsgProc3rd = savedProc;
}

/**
 * @tc.name: AuthPincodeSrcPinNegotiateStartState_Action_003
 * @tc.desc: Test AuthPincodeSrcPinNegotiateStartState Action with null authPinMsgProc3rd (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcPinNegotiateStartState_Action_003, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcPinNegotiateStartState>();
    std::shared_ptr<DmAuthPincodeMessageProcessor3rd> savedProc = context_->authPinMsgProc3rd;
    context_->authPinMsgProc3rd = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->authPinMsgProc3rd = savedProc;
}

/**
 * @tc.name: AuthPincodeSinkPinNegotiateStartState_Action_001
 * @tc.desc: Test AuthPincodeSinkPinNegotiateStartState Action with null context (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinNegotiateStartState_Action_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinNegotiateStartState>();
    int32_t ret = state->Action(nullptr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AuthPincodeSinkPinNegotiateStartState_Action_002
 * @tc.desc: Test AuthPincodeSinkPinNegotiateStartState Action with null timer (early return branch)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkPinNegotiateStartState_Action_002, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkPinNegotiateStartState>();
    std::shared_ptr<DmTimer3rd> savedTimer = context_->timer;
    context_->timer = nullptr;
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    context_->timer = savedTimer;
}

/**
 * @tc.name: AuthPincodeSinkFinishState_Action_201
 * @tc.desc: Test AuthPincodeSinkFinishState Action with non-null cleanNotifyCallback branch
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkFinishState_Action_201, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkFinishState>();
    context_->reason = DM_OK;
    context_->state = 1;
    context_->connDelayCloseTime = 500;
    bool callbackInvoked = false;
    context_->cleanNotifyCallback =
        [&callbackInvoked](uint64_t sessionId, int32_t delayTime, ProcessInfo3rd processInfo) {
            callbackInvoked = true;
        };
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_TRUE(callbackInvoked);
    context_->cleanNotifyCallback = nullptr;
}

/**
 * @tc.name: AuthPincodeSrcFinishState_Action_201
 * @tc.desc: Test AuthPincodeSrcFinishState Action success path with non-null cleanNotifyCallback branch
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcFinishState_Action_201, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcFinishState>();
    context_->reason = DM_OK;
    context_->state = 1;
    context_->connDelayCloseTime = 1000;
    int32_t capturedDelay = -1;
    context_->cleanNotifyCallback =
        [&capturedDelay](uint64_t sessionId, int32_t delayTime, ProcessInfo3rd processInfo) {
            capturedDelay = delayTime;
        };
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(capturedDelay, 1000);
    context_->cleanNotifyCallback = nullptr;
}

/**
 * @tc.name: AuthPincodeSrcFinishState_Action_202
 * @tc.desc: Test AuthPincodeSrcFinishState Action error path with cleanNotifyCallback (connDelayCloseTime reset to 0)
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSrcFinishState_Action_202, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcFinishState>();
    context_->reason = ERR_DM_FAILED;
    context_->state = 1;
    context_->reply = 1;
    context_->connDelayCloseTime = 1000;
    int32_t capturedDelay = -1;
    context_->cleanNotifyCallback =
        [&capturedDelay](uint64_t sessionId, int32_t delayTime, ProcessInfo3rd processInfo) {
            capturedDelay = delayTime;
        };
    int32_t ret = state->Action(context_);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(capturedDelay, 0);
    context_->cleanNotifyCallback = nullptr;
}

/**
 * @tc.name: DmAuthPincodeStateMachine_GetCurState_001
 * @tc.desc: Test DmAuthPincodeStateMachine3rd GetCurState/IsWaitEvent after construction
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeStateMachine_GetCurState_001, TestSize.Level1)
{
    EXPECT_EQ(context_->authPinStateMac3rd->GetCurState(), DmAuthPincodeStateType::ACL_AUTH_IDLE_STATE);
    EXPECT_FALSE(context_->authPinStateMac3rd->IsWaitEvent());
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_204
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with CONFIRM_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_204, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, CONFIRM_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_CONFIRM_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_205
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with ADD_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_205, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, ADD_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_ADD_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_206
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with WAIT_NEGOTIATE_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_206, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, WAIT_NEGOTIATE_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_WAIT_NEGOTIATE_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_207
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with WAIT_REQUEST_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_207, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, WAIT_REQUEST_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_WAIT_REQUEST_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_208
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with AUTHENTICATE_TIMEOUT_TASK mapped key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_208, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, AUTHENTICATE_TIMEOUT_TASK, 10000);
    EXPECT_EQ(timeout, CLONE_AUTHENTICATE_TIMEOUT);
}

/**
 * @tc.name: DmAuthPincodeState_GetTaskTimeout_209
 * @tc.desc: Test DmAuthPincodeState3rd::GetTaskTimeout with empty task name string
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_GetTaskTimeout_209, TestSize.Level1)
{
    int32_t timeout = DmAuthPincodeState3rd::GetTaskTimeout(context_, "", 7000);
    EXPECT_EQ(timeout, 7000);
}

/**
 * @tc.name: AuthPincodeSinkFinishState_GetStateType_001
 * @tc.desc: Test AuthPincodeSinkFinishState GetStateType returns ACL_AUTH_PINCODE_SINK_FINISH_STATE
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, AuthPincodeSinkFinishState_GetStateType_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkFinishState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE);
}

/**
 * @tc.name: DmAuthPincodeState_BuildResultContent_007
 * @tc.desc: Test BuildResultContent with sessionKey of exactly MAX_SESSION_KEY_LENGTH boundary
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeStateTest, DmAuthPincodeState_BuildResultContent_007, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    context_->reason = DM_OK;
    context_->accesser.sessionKey.assign(512, 0xCD);
    context_->accesser.businessName = "boundary_business";
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    state->BuildResultContent(context_, deviceInfos);
    ASSERT_EQ(deviceInfos.size(), 1u);
    EXPECT_EQ(deviceInfos[0].sessionKey.keyLen, 512u);
    FreeDeviceInfos(deviceInfos);
}

/**
 * @tc.name: DmTimer3rd_StartTimer_001
 * @tc.desc: Test DmTimer3rd StartTimer with valid parameters returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_StartTimer_001, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->StartTimer("testTimer001", 10, [](std::string name) {});
    EXPECT_EQ(ret, DM_OK);
    timer->DeleteAll();
}

/**
 * @tc.name: DmTimer3rd_StartTimer_002
 * @tc.desc: Test DmTimer3rd StartTimer with empty name returns invalid error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_StartTimer_002, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->StartTimer("", 10, [](std::string name) {});
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DmTimer3rd_StartTimer_003
 * @tc.desc: Test DmTimer3rd StartTimer with timeout exceeding max boundary returns invalid error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_StartTimer_003, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->StartTimer("boundaryTimer", 601, [](std::string name) {});
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DmTimer3rd_StartTimer_004
 * @tc.desc: Test DmTimer3rd StartTimer with negative timeout returns invalid error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_StartTimer_004, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->StartTimer("negTimer", -1, [](std::string name) {});
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DmTimer3rd_StartTimer_005
 * @tc.desc: Test DmTimer3rd StartTimer with null callback returns invalid error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_StartTimer_005, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->StartTimer("nullCbTimer", 10, nullptr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DmTimer3rd_StartTimer_006
 * @tc.desc: Test DmTimer3rd StartTimer at max boundary timeout (600) succeeds
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_StartTimer_006, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->StartTimer("maxBoundaryTimer", 600, [](std::string name) {});
    EXPECT_EQ(ret, DM_OK);
    timer->DeleteAll();
}

/**
 * @tc.name: DmTimer3rd_DeleteTimer_001
 * @tc.desc: Test DmTimer3rd DeleteTimer with empty name returns invalid error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_DeleteTimer_001, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->DeleteTimer("");
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DmTimer3rd_DeleteTimer_002
 * @tc.desc: Test DmTimer3rd DeleteTimer for non-existent timer returns failed error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_DeleteTimer_002, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->DeleteTimer("nonExistentTimer");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmTimer3rd_DeleteTimer_003
 * @tc.desc: Test DmTimer3rd DeleteTimer after StartTimer returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_DeleteTimer_003, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    std::string name = "deleteTimer003";
    timer->StartTimer(name, 300, [](std::string n) {});
    auto ret = timer->DeleteTimer(name);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmTimer3rd_DeleteAll_001
 * @tc.desc: Test DmTimer3rd DeleteAll on empty timer returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_DeleteAll_001, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    auto ret = timer->DeleteAll();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmTimer3rd_DeleteAll_002
 * @tc.desc: Test DmTimer3rd DeleteAll with multiple timers returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmTimer3rd_DeleteAll_002, TestSize.Level1)
{
    auto timer = std::make_shared<DmTimer3rd>();
    timer->StartTimer("multi1", 300, [](std::string n) {});
    timer->StartTimer("multi2", 300, [](std::string n) {});
    auto ret = timer->DeleteAll();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAnony3rd_GetAnonyString_001
 * @tc.desc: Test GetAnonyString with short string (len <= 20) returns masked first+last
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyString_001, TestSize.Level1)
{
    std::string result = GetAnonyString("abcdefghij");
    EXPECT_EQ(result, "a******j");
}

/**
 * @tc.name: DmAnony3rd_GetAnonyString_002
 * @tc.desc: Test GetAnonyString with long string (len > 20) masks middle characters
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyString_002, TestSize.Level1)
{
    std::string input = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJ";
    std::string result = GetAnonyString(input);
    EXPECT_EQ(result.size() > 0, true);
    EXPECT_EQ(result.substr(0, 4), input.substr(0, 4));
    EXPECT_EQ(result.find("******") != std::string::npos, true);
}

/**
 * @tc.name: DmAnony3rd_GetAnonyString_003
 * @tc.desc: Test GetAnonyString with very short string (len < 3) returns default mask
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyString_003, TestSize.Level1)
{
    std::string result = GetAnonyString("ab");
    EXPECT_EQ(result, "******");
}

/**
 * @tc.name: DmAnony3rd_GetAnonyInt32_001
 * @tc.desc: Test GetAnonyInt32 with multi-digit value masks middle digits
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyInt32_001, TestSize.Level1)
{
    std::string result = GetAnonyInt32(12345);
    EXPECT_EQ(result.size(), 5u);
    EXPECT_EQ(result[0], '1');
    EXPECT_EQ(result[result.size() - 1], '5');
}

/**
 * @tc.name: DmAnony3rd_GetAnonyInt32_002
 * @tc.desc: Test GetAnonyInt32 with single digit value returns masked digit
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyInt32_002, TestSize.Level1)
{
    std::string result = GetAnonyInt32(7);
    EXPECT_EQ(result, "*");
}

/**
 * @tc.name: DmAnony3rd_GetAnonyUint32_001
 * @tc.desc: Test GetAnonyUint32 with multi-digit value masks middle digits
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyUint32_001, TestSize.Level1)
{
    std::string result = GetAnonyUint32(987654u);
    EXPECT_EQ(result.size(), 6u);
    EXPECT_EQ(result[0], '9');
    EXPECT_EQ(result[result.size() - 1], '4');
}

/**
 * @tc.name: DmAnony3rd_ConvertMapToJsonString_001
 * @tc.desc: Test ConvertMapToJsonString with valid map returns json string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_ConvertMapToJsonString_001, TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    paramMap["key1"] = "value1";
    paramMap["key2"] = "value2";
    std::string result = ConvertMapToJsonString(paramMap);
    EXPECT_NE(result.find("key1"), std::string::npos);
    EXPECT_NE(result.find("value1"), std::string::npos);
}

/**
 * @tc.name: DmAnony3rd_ConvertMapToJsonString_002
 * @tc.desc: Test ConvertMapToJsonString with empty map returns empty string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_ConvertMapToJsonString_002, TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    std::string result = ConvertMapToJsonString(paramMap);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: DmAnony3rd_ParseMapFromJsonString_001
 * @tc.desc: Test ParseMapFromJsonString roundtrip with ConvertMapToJsonString
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_ParseMapFromJsonString_001, TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    paramMap["name"] = "device1";
    std::string jsonStr = ConvertMapToJsonString(paramMap);
    std::map<std::string, std::string> parsedMap;
    ParseMapFromJsonString(jsonStr, parsedMap);
    EXPECT_EQ(parsedMap["name"], "device1");
}

/**
 * @tc.name: DmAnony3rd_ParseMapFromJsonString_002
 * @tc.desc: Test ParseMapFromJsonString with empty string does nothing
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_ParseMapFromJsonString_002, TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    paramMap["existing"] = "value";
    ParseMapFromJsonString("", paramMap);
    EXPECT_EQ(paramMap.size(), 1u);
    EXPECT_EQ(paramMap["existing"], "value");
}

/**
 * @tc.name: DmAnony3rd_GetAnonyJsonString_001
 * @tc.desc: Test GetAnonyJsonString with sensitive key 'networkId' masks value
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyJsonString_001, TestSize.Level1)
{
    std::string input = "{\"networkId\":\"123456789abcdef\"}";
    std::string result = GetAnonyJsonString(input);
    EXPECT_NE(result.find("******"), std::string::npos);
    EXPECT_EQ(result.find("123456789abcdef"), std::string::npos);
}

/**
 * @tc.name: DmAnony3rd_GetAnonyJsonString_002
 * @tc.desc: Test GetAnonyJsonString with empty string returns empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAnony3rd_GetAnonyJsonString_002, TestSize.Level1)
{
    std::string result = GetAnonyJsonString("");
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: DmCrypto3rd_Sha256_001
 * @tc.desc: Test Crypto3rd Sha256 with valid string returns 64-char hex hash
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_Sha256_001, TestSize.Level1)
{
    std::string result = Crypto3rd::Sha256("hello");
    EXPECT_EQ(result.size(), 64u);
}

/**
 * @tc.name: DmCrypto3rd_Sha256_002
 * @tc.desc: Test Crypto3rd Sha256 with upper case flag returns uppercase hex
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_Sha256_002, TestSize.Level1)
{
    std::string lower = Crypto3rd::Sha256(std::string("test"), false);
    std::string upper = Crypto3rd::Sha256(std::string("test"), true);
    EXPECT_EQ(lower.size(), 64u);
    EXPECT_EQ(upper.size(), 64u);
    for (char c : upper) {
        EXPECT_EQ(c >= 'A' || (c >= '0' && c <= '9'), true);
    }
}

/**
 * @tc.name: DmCrypto3rd_GetUdidHash_001
 * @tc.desc: Test Crypto3rd GetUdidHash (string overload) returns 16-char hex hash
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetUdidHash_001, TestSize.Level1)
{
    std::string udid = "1234567890123456789012345678901234567890123456789012345678901234";
    std::string result = Crypto3rd::GetUdidHash(udid);
    EXPECT_EQ(result.size(), 16u);
}

/**
 * @tc.name: DmCrypto3rd_GetUdidHash_002
 * @tc.desc: Test Crypto3rd GetUdidHash (buffer overload) returns DM_OK and fills buffer
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetUdidHash_002, TestSize.Level1)
{
    std::string udid = "1234567890123456789012345678901234567890123456789012345678901234";
    unsigned char hashBuf[17] = {0};
    int32_t ret = Crypto3rd::GetUdidHash(udid, hashBuf);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmCrypto3rd_GetTokenIdHash_001
 * @tc.desc: Test Crypto3rd GetTokenIdHash returns 32-char hex hash
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetTokenIdHash_001, TestSize.Level1)
{
    std::string result = Crypto3rd::GetTokenIdHash("tokenId123456");
    EXPECT_EQ(result.size(), 32u);
}

/**
 * @tc.name: DmCrypto3rd_GetGroupIdHash_001
 * @tc.desc: Test Crypto3rd GetGroupIdHash returns 16-char hex substring
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetGroupIdHash_001, TestSize.Level1)
{
    std::string result = Crypto3rd::GetGroupIdHash("groupIdTest123");
    EXPECT_EQ(result.size(), 16u);
}

/**
 * @tc.name: DmCrypto3rd_GetHashWithSalt_001
 * @tc.desc: Test Crypto3rd GetHashWithSalt appends salt then hashes
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetHashWithSalt_001, TestSize.Level1)
{
    std::string result = Crypto3rd::GetHashWithSalt("text", "salt");
    EXPECT_EQ(result.size(), 64u);
    std::string directHash = Crypto3rd::Sha256("textsalt");
    EXPECT_EQ(result, directHash);
}

/**
 * @tc.name: DmCrypto3rd_GetSecRandom_001
 * @tc.desc: Test Crypto3rd GetSecRandom with valid buffer returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetSecRandom_001, TestSize.Level1)
{
    uint8_t out[8] = {0};
    int32_t ret = Crypto3rd::GetSecRandom(out, sizeof(out));
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmCrypto3rd_GetSecRandom_002
 * @tc.desc: Test Crypto3rd GetSecRandom with null buffer returns error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetSecRandom_002, TestSize.Level1)
{
    int32_t ret = Crypto3rd::GetSecRandom(nullptr, 8);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: DmCrypto3rd_GetSecSalt_001
 * @tc.desc: Test Crypto3rd GetSecSalt returns 16-char hex salt string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetSecSalt_001, TestSize.Level1)
{
    std::string salt = Crypto3rd::GetSecSalt();
    EXPECT_EQ(salt.size(), 16u);
}

/**
 * @tc.name: DmCrypto3rd_ConvertBytesToHexString_001
 * @tc.desc: Test Crypto3rd ConvertBytesToHexString with valid input returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_ConvertBytesToHexString_001, TestSize.Level1)
{
    unsigned char inBuf[2] = {0x1a, 0xff};
    char outBuf[5] = {0};
    int32_t ret = Crypto3rd::ConvertBytesToHexString(outBuf, sizeof(outBuf), inBuf, 2);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(std::string(outBuf), "1aff");
}

/**
 * @tc.name: DmCrypto3rd_ConvertBytesToHexString_002
 * @tc.desc: Test Crypto3rd ConvertBytesToHexString with null output returns invalid error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_ConvertBytesToHexString_002, TestSize.Level1)
{
    unsigned char inBuf[2] = {0x1a, 0xff};
    int32_t ret = Crypto3rd::ConvertBytesToHexString(nullptr, 5, inBuf, 2);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DmCrypto3rd_ConvertHexStringToBytes_001
 * @tc.desc: Test Crypto3rd ConvertHexStringToBytes with valid hex string returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_ConvertHexStringToBytes_001, TestSize.Level1)
{
    const char* hexStr = "1aff";
    unsigned char outBuf[2] = {0};
    int32_t ret = Crypto3rd::ConvertHexStringToBytes(outBuf, sizeof(outBuf), hexStr, 4);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(outBuf[0], 0x1a);
    EXPECT_EQ(outBuf[1], 0xff);
}

/**
 * @tc.name: DmCrypto3rd_ConvertHexStringToBytes_002
 * @tc.desc: Test Crypto3rd ConvertHexStringToBytes with null output returns failed error
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_ConvertHexStringToBytes_002, TestSize.Level1)
{
    const char* hexStr = "1aff";
    int32_t ret = Crypto3rd::ConvertHexStringToBytes(nullptr, 2, hexStr, 4);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmCrypto3rd_GetAccountIdHash_001
 * @tc.desc: Test Crypto3rd GetAccountIdHash (buffer overload) returns DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetAccountIdHash_001, TestSize.Level1)
{
    unsigned char hashBuf[7] = {0};
    int32_t ret = Crypto3rd::GetAccountIdHash("accountId12345", hashBuf);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmCrypto3rd_GetAccountIdHash16_001
 * @tc.desc: Test Crypto3rd GetAccountIdHash16 returns 32-char hex hash
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmCrypto3rd_GetAccountIdHash16_001, TestSize.Level1)
{
    std::string result = Crypto3rd::GetAccountIdHash16("accountId12345");
    EXPECT_EQ(result.size(), 32u);
}

/**
 * @tc.name: DmAuthInfo3rd_FreeDeviceInfos_001
 * @tc.desc: Test FreeDeviceInfos with empty vector does nothing
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_FreeDeviceInfos_001, TestSize.Level1)
{
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    FreeDeviceInfos(deviceInfos);
    EXPECT_TRUE(deviceInfos.empty());
}

/**
 * @tc.name: DmAuthInfo3rd_FreeDeviceInfos_002
 * @tc.desc: Test FreeDeviceInfos with allocated session key frees and nulls the key
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_FreeDeviceInfos_002, TestSize.Level1)
{
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    TrustDeviceInfo3rd info;
    info.sessionKey.key = static_cast<uint8_t*>(malloc(16));
    std::fill_n(info.sessionKey.key, 16, static_cast<uint8_t>(0xAB));
    info.sessionKey.keyLen = 16;
    deviceInfos.push_back(info);
    FreeDeviceInfos(deviceInfos);
    EXPECT_EQ(deviceInfos[0].sessionKey.key, nullptr);
    EXPECT_EQ(deviceInfos[0].sessionKey.keyLen, 0u);
}

/**
 * @tc.name: DmAuthInfo3rd_FreeDeviceInfos_003
 * @tc.desc: Test FreeDeviceInfos with null session key does nothing
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_FreeDeviceInfos_003, TestSize.Level1)
{
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    TrustDeviceInfo3rd info;
    info.sessionKey.key = nullptr;
    info.sessionKey.keyLen = 0;
    deviceInfos.push_back(info);
    FreeDeviceInfos(deviceInfos);
    EXPECT_EQ(deviceInfos[0].sessionKey.key, nullptr);
}

/**
 * @tc.name: DmAuthInfo3rd_FromJson_Access3rd_001
 * @tc.desc: Test FromJson for Access3rd parses all fields from JSON object
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_FromJson_Access3rd_001, TestSize.Level1)
{
    std::string jsonStr = "{\"deviceId\":\"dev123\",\"userId\":100,\"accountId\":\"acc456\","
        "\"tokenId\":999,\"processName\":\"proc\",\"uid\":200,\"businessName\":\"biz\",\"version\":\"1.0\"}";
    JsonObject jsonObj(jsonStr);
    ASSERT_FALSE(jsonObj.IsDiscarded());
    Access3rd access;
    FromJson(jsonObj, access);
    EXPECT_EQ(access.deviceId, "dev123");
    EXPECT_EQ(access.userId, 100);
    EXPECT_EQ(access.accountId, "acc456");
    EXPECT_EQ(access.tokenId, 999u);
    EXPECT_EQ(access.processName, "proc");
    EXPECT_EQ(access.uid, 200);
    EXPECT_EQ(access.businessName, "biz");
    EXPECT_EQ(access.version, "1.0");
}

/**
 * @tc.name: DmAuthInfo3rd_FromJson_Access3rd_002
 * @tc.desc: Test FromJson for Access3rd with empty JSON object leaves fields at defaults
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_FromJson_Access3rd_002, TestSize.Level1)
{
    std::string jsonStr = "{}";
    JsonObject jsonObj(jsonStr);
    ASSERT_FALSE(jsonObj.IsDiscarded());
    Access3rd access;
    FromJson(jsonObj, access);
    EXPECT_EQ(access.deviceId, "");
    EXPECT_EQ(access.userId, -1);
}

/**
 * @tc.name: DmAuthInfo3rd_FromJson_TrustDeviceInfo3rd_001
 * @tc.desc: Test FromJson for TrustDeviceInfo3rd parses fields from JSON object
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_FromJson_TrustDeviceInfo3rd_001, TestSize.Level1)
{
    std::string jsonStr = "{\"trustDeviceId\":\"trustDev\",\"sessionKeyId\":42,\"createTime\":1000,"
        "\"userId\":100,\"extra\":\"ext\",\"bindLevel\":1,\"bindType\":2}";
    JsonObject jsonObj(jsonStr);
    ASSERT_FALSE(jsonObj.IsDiscarded());
    TrustDeviceInfo3rd info;
    FromJson(jsonObj, info);
    EXPECT_EQ(info.trustDeviceId, "trustDev");
    EXPECT_EQ(info.sessionKeyId, 42);
    EXPECT_EQ(info.createTime, 1000);
    EXPECT_EQ(info.userId, 100);
    EXPECT_EQ(info.extra, "ext");
    EXPECT_EQ(info.bindLevel, 1);
    EXPECT_EQ(info.bindType, 2);
}

/**
 * @tc.name: DmAuthInfo3rd_FromJson_AccessControl3rd_001
 * @tc.desc: Test FromJson for AccessControl3rd parses scalar fields from JSON object
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_FromJson_AccessControl3rd_001, TestSize.Level1)
{
    std::string jsonStr = "{\"trustDeviceId\":\"td\",\"sessionKeyId\":1,\"createTime\":500,"
        "\"extra\":\"ex\",\"bindLevel\":2,\"bindType\":3}";
    JsonObject jsonObj(jsonStr);
    ASSERT_FALSE(jsonObj.IsDiscarded());
    AccessControl3rd ac;
    FromJson(jsonObj, ac);
    EXPECT_EQ(ac.trustDeviceId, "td");
    EXPECT_EQ(ac.sessionKeyId, 1);
    EXPECT_EQ(ac.createTime, 500);
    EXPECT_EQ(ac.extra, "ex");
    EXPECT_EQ(ac.bindLevel, 2);
    EXPECT_EQ(ac.bindType, 3);
}

/**
 * @tc.name: DmAuthInfo3rd_ToJson_Access3rd_001
 * @tc.desc: Test ToJson for Access3rd serializes all fields into JSON object
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthPincodeStateTest, DmAuthInfo3rd_ToJson_Access3rd_001, TestSize.Level1)
{
    Access3rd access;
    access.deviceId = "devId";
    access.userId = 100;
    access.accountId = "accId";
    access.tokenId = 999;
    access.processName = "procName";
    access.uid = 200;
    access.businessName = "bizName";
    access.version = "2.0";
    JsonObject jsonObj;
    ToJson(jsonObj, access);
    std::string result = jsonObj.Dump();
    EXPECT_NE(result.find("devId"), std::string::npos);
    EXPECT_NE(result.find("accId"), std::string::npos);
    EXPECT_NE(result.find("procName"), std::string::npos);
    EXPECT_NE(result.find("bizName"), std::string::npos);
    EXPECT_NE(result.find("2.0"), std::string::npos);
}

} // namespace DistributedHardware
} // namespace OHOS