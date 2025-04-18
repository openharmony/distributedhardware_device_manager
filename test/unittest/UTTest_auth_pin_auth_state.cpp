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


#include "dm_auth_state_machine_mock.h"
#include "UTTest_auth_pin_auth_state.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

void AuthPinAuthStateTest::SetUpTestCase()
{
    LOGI("AuthPinAuthStateTest::SetUpTestCase start.");
    DmHiChainAuthConnector::dmHiChainAuthConnector = hiChainAuthConnectorMock;
    DmAuthStateMachineMock::dmAuthStateMachineMock = std::make_shared<DmAuthStateMachineMock>();
}

void AuthPinAuthStateTest::TearDownTestCase()
{
    LOGI("AuthPinAuthStateTest::TearDownTestCase start.");
    DmHiChainAuthConnector::dmHiChainAuthConnector = nullptr;
    DmAuthStateMachineMock::dmAuthStateMachineMock = nullptr;
    hiChainAuthConnectorMock = nullptr;
}

void AuthPinAuthStateTest::SetUp()
{
    LOGI("AuthPinAuthStateTest::SetUp start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
    hiChainAuthConnector);
    context = authManager->GetAuthContext();

    Mock::VerifyAndClearExpectations(&*hiChainAuthConnectorMock);
    Mock::VerifyAndClearExpectations(&*DmAuthStateMachineMock::dmAuthStateMachineMock);
}

void AuthPinAuthStateTest::TearDown()
{
    LOGI("AuthPinAuthStateTest::TearDown start.");
    softbusConnector = nullptr;
    listener = nullptr;
    hiChainAuthConnector = nullptr;
    authManager = nullptr;
    context = nullptr;
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcPinAuthStartState> authState = std::make_shared<AuthSrcPinAuthStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthStartState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthStartState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthStartState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthStartState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthStartState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthStartState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthStartState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkPinAuthStartState> authState = std::make_shared<AuthSinkPinAuthStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthStartState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthStartState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthStartState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthMsgNegotiateState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcPinAuthMsgNegotiateState> authState = std::make_shared<AuthSrcPinAuthMsgNegotiateState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthMsgNegotiateState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthMsgNegotiateState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthMsgNegotiateState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthMsgNegotiateState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_REQUEST));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthMsgNegotiateState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkPinAuthMsgNegotiateState> authState = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthMsgNegotiateState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthMsgNegotiateState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthMsgNegotiateState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthMsgNegotiateState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthMsgNegotiateState_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthMsgNegotiateState_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthDoneState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcPinAuthDoneState> authState = std::make_shared<AuthSrcPinAuthDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthDoneState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthDoneState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthDoneState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthDoneState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthDoneState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthDoneState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthDoneState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthDoneState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_REQUEST));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthDoneState_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthDoneState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinAuthDoneState_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinAuthDoneState>();

    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_REQUEST));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthDoneState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkPinAuthDoneState> authState = std::make_shared<AuthSinkPinAuthDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_PIN_AUTH_DONE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinAuthDoneState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinAuthDoneState>();

    EXPECT_EQ(authState->Action(context), DM_OK);
}
}
}