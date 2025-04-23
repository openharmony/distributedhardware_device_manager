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

bool DmAuthState::IsScreenLocked()
{
    return false;
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

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcPinNegotiateStartState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = false;

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_012, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_022, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_032, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE);
    context->importAuthCode = "123456";
    context->importPkgName = "com.test.example";
    context->pkgName = "com.test.example";
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->accesser.credentialInfos[DM_IDENTICAL_ACCOUNT] = R"({"credId": 1234})";

    context->pinNegotiateStarted = true;
    context->needBind = false;
    context->needAgreeCredential = false;
    context->needAuth = true;
    context->accesser.credTypeList = R"({"identicalCredType": 1})";
    EXPECT_EQ(authState->Action(context), DM_OK);
    context->accesser.credTypeList = R"({"shareCredType": 2})";
    EXPECT_EQ(authState->Action(context), DM_OK);
    context->accesser.credTypeList = R"({"pointTopointCredType": 256})";
    EXPECT_EQ(authState->Action(context), DM_OK);
    context->accesser.credTypeList = R"({"lnnCredType": 3})";
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = true;
    context->needBind = false;
    context->needAgreeCredential = false;
    context->needAuth = false;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = true;
    context->needBind = false;
    context->needAgreeCredential = true;
    context->needAuth = false;
    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = false;

    EXPECT_EQ(authState->Action(context), DM_OK);
    EXPECT_EQ(context->pinNegotiateStarted, true);
    context->needBind = true;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinNegotiateStartState_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_UNKNOW);
    context->authType = DmAuthType::AUTH_TYPE_UNKNOW;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    context->pinNegotiateStarted = true;
    context->needBind = true;
    EXPECT_EQ(authState->Action(context), ERR_DM_AUTH_REJECT);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinInputState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcPinInputState> authState = std::make_shared<AuthSrcPinInputState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_PIN_INPUT_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcPinInputState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcPinInputState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_USER_OPERATION))
        .WillOnce(Return(DmEventType::ON_USER_OPERATION))
        .WillOnce(Return(DmEventType::ON_USER_OPERATION))
        .WillOnce(Return(DmEventType::ON_FAIL));

    context->pinInputResult = USER_OPERATION_TYPE_DONE_PINCODE_INPUT;
    EXPECT_EQ(authState->Action(context), DM_OK);
    context->inputPinAuthFailTimes = 1;
    EXPECT_EQ(authState->Action(context), DM_OK);

    context->pinInputResult = USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT;
    EXPECT_EQ(authState->Action(context), STOP_BIND);

    context->pinInputResult = USER_OPERATION_TYPE_DONE_PINCODE_INPUT;
    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinNegotiateStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkPinNegotiateStartState> authState = std::make_shared<AuthSinkPinNegotiateStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinNegotiateStartState_012, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), DM_OK);

    EXPECT_EQ(context->pinNegotiateStarted, true);
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinNegotiateStartState_022, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE);
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), DM_OK);

    EXPECT_EQ(context->pinNegotiateStarted, true);
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinNegotiateStartState_032, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE);
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), DM_OK);

    EXPECT_EQ(context->pinNegotiateStarted, true);
    EXPECT_EQ(authState->Action(context), ERR_DM_AUTH_REJECT);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinNegotiateStartState_042, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinNegotiateStartState_052, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinNegotiateStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_UNKNOW);
    context->authType = DmAuthType::AUTH_TYPE_UNKNOW;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    context->pinNegotiateStarted = false;
    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinDisplayState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkPinDisplayState> authState = std::make_shared<AuthSinkPinDisplayState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_PIN_DISPLAY_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkPinDisplayState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkPinDisplayState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN);
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    context->inputPinAuthFailTimes = 0;
    EXPECT_EQ(authState->Action(context), DM_OK);

    context->inputPinAuthFailTimes = 1;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcReverseUltrasonicStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcReverseUltrasonicStartState> authState =
        std::make_shared<AuthSrcReverseUltrasonicStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcReverseUltrasonicStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcReverseUltrasonicStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcReverseUltrasonicDoneState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcReverseUltrasonicDoneState> authState =
        std::make_shared<AuthSrcReverseUltrasonicDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_DONE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcReverseUltrasonicDoneState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_TRANSMIT));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcReverseUltrasonicDoneState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ERROR));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcReverseUltrasonicDoneState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_FAIL));
    EXPECT_EQ(authState->Action(context), STOP_BIND);
}
HWTEST_F(AuthPinAuthStateTest, AuthSrcReverseUltrasonicDoneState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(STOP_BIND));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcForwardUltrasonicStartState> authState =
        std::make_shared<AuthSrcForwardUltrasonicStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcForwardUltrasonicStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicDoneState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcForwardUltrasonicDoneState> authState =
        std::make_shared<AuthSrcForwardUltrasonicDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_DONE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicDoneState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ULTRASONIC_PIN_CHANGED))
        .WillOnce(Return(DmEventType::ON_TRANSMIT));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicDoneState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ULTRASONIC_PIN_CHANGED))
        .WillOnce(Return(DmEventType::ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicDoneState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ULTRASONIC_PIN_TIMEOUT));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicDoneState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_FAIL));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSrcForwardUltrasonicDoneState_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SOURCE;
    EXPECT_CALL(*hiChainAuthConnectorMock, AuthCredentialPinCode(_, _, _))
        .WillOnce(Return(STOP_BIND));

    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ULTRASONIC_PIN_CHANGED));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkReverseUltrasonicStartState> authState =
        std::make_shared<AuthSinkReverseUltrasonicStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkReverseUltrasonicStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ULTRASONIC_PIN_CHANGED));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicStartState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkReverseUltrasonicStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ULTRASONIC_PIN_TIMEOUT));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicStartState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkReverseUltrasonicStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_FAIL));
    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicDoneState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkReverseUltrasonicDoneState> authState =
        std::make_shared<AuthSinkReverseUltrasonicDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_DONE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicDoneState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_TRANSMIT));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicDoneState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ERROR));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicDoneState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_FAIL));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkReverseUltrasonicDoneState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkReverseUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Reverse;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(STOP_BIND));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkForwardUltrasonicStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkForwardUltrasonicStartState> authState =
        std::make_shared<AuthSinkForwardUltrasonicStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_START_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkForwardUltrasonicStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkForwardUltrasonicStartState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SINK;

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkForwardUltrasonicDoneState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkForwardUltrasonicDoneState> authState =
        std::make_shared<AuthSinkForwardUltrasonicDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_DONE_STATE);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkForwardUltrasonicDoneState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_TRANSMIT));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkForwardUltrasonicDoneState_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_ERROR));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkForwardUltrasonicDoneState_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*DmAuthStateMachineMock::dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(DmEventType::ON_FAIL));
    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

HWTEST_F(AuthPinAuthStateTest, AuthSinkForwardUltrasonicDoneState_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkForwardUltrasonicDoneState>();
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    context->authTypeList.push_back(DmAuthType::AUTH_TYPE_PIN_ULTRASONIC);
    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    context->ultrasonicInfo = DM_Ultrasonic_Forward;
    context->direction = DmAuthDirection::DM_AUTH_SINK;
    EXPECT_CALL(*hiChainAuthConnectorMock, ProcessCredData(_, _))
        .WillOnce(Return(STOP_BIND));

    EXPECT_EQ(authState->Action(context), STOP_BIND);
}

}
}