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

#include "dm_auth_state.h"
#include "UTTest_auth_credential_state.h"
#include "dm_auth_message_processor_mock.h"
#include "device_manager_service_listener.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

constexpr const char *TEST_NONE_EMPTY_STRING = "test";

void AuthCredentialStateTest::SetUpTestCase()
{
    LOGI("AuthCredentialStateTest::SetUpTestCase start.");
    DmHiChainAuthConnector::dmHiChainAuthConnector = dmHiChainAuthConnectorMock;
    DmSoftbusSession::dmSoftbusSession = dmSoftbusSessionMock;
    DmAuthStateMachineMock::dmAuthStateMachineMock = dmAuthStateMachineMock;
    DmAuthMessageProcessorMock::dmAuthMessageProcessorMock = std::make_shared<DmAuthMessageProcessorMock>();
}

void AuthCredentialStateTest::TearDownTestCase()
{
    LOGI("AuthCredentialStateTest::TearDownTestCase start.");
    DmHiChainAuthConnector::dmHiChainAuthConnector = nullptr;
    dmHiChainAuthConnectorMock = nullptr;
    DmSoftbusSession::dmSoftbusSession = nullptr;
    dmSoftbusSessionMock = nullptr;
    DmAuthStateMachineMock::dmAuthStateMachineMock = nullptr;
    dmAuthStateMachineMock = nullptr;
    DmAuthMessageProcessorMock::dmAuthMessageProcessorMock = nullptr;
}

void AuthCredentialStateTest::SetUp()
{
    LOGI("AuthCredentialStateTest::SetUp start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
}

void AuthCredentialStateTest::TearDown()
{
    LOGI("AuthCredentialStateTest::TearDown start.");
    softbusConnector = nullptr;
    listener = nullptr;
    hiChainAuthConnector = nullptr;
    authManager = nullptr;
    context = nullptr;
    Mock::VerifyAndClearExpectations(&*DmHiChainAuthConnector::dmHiChainAuthConnector);
    Mock::VerifyAndClearExpectations(&*DmSoftbusSession::dmSoftbusSession);
    Mock::VerifyAndClearExpectations(&*DmAuthStateMachineMock::dmAuthStateMachineMock);
    Mock::VerifyAndClearExpectations(&*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialExchangeState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialExchangeState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_CREDENTIAL_EXCHANGE_STATE);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialExchangeState_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialExchangeState>();
    context->isOnline = false;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _)).WillRepeatedly(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _)).WillRepeatedly(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialExchangeState_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialExchangeState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialExchangeState_004, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialExchangeState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialExchangeState_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialExchangeState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialExchangeState_006, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialExchangeState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialExchangeState_007, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialExchangeState>();
    context->isOnline = false;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_CREDENTIAL_EXCHANGE_STATE);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
        .WillOnce(Return(DM_OK));


    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_004, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();
    context->isOnline = false;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_006, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();
    context->isOnline = false;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_007, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();
    context->isOnline = false;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_008, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
    .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialExchangeState_009, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialExchangeState>();

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AddCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ExportCredential(_, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthStartState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_START_STATE);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthStartState_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
        .WillOnce(Return(DM_OK));
    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthStartState_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthStartState_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthStartState_006, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
        .WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthStartState_007, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
    .WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthStartState_008, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    context->direction == DM_AUTH_SOURCE;
    context->accesser.isAuthed = false;
    context->needAgreeCredential = true;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AgreeCredential(_, _, _, _))
    .WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthStartState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthStartState_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthStartState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_)).WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthStartState_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthStartState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthStartState_004, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthStartState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_)).WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthStartState_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthStartState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_)).WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthNegotiateState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthNegotiateState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthNegotiateState_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_)).WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthNegotiateState_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthNegotiateState_004, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_)).WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthNegotiateState_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_)).WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, SaveSessionKeyToDP(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_004, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT))
        .WillOnce(Return(ON_ERROR));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_006, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_ERROR));
    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_007, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, SaveSessionKeyToDP(_, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSinkCredentialAuthNegotiateState_008, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_DONE_STATE);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";
    context->accesser.isGenerateLnnCredential == true;
    context->isAppCredentialVerified = false;

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH))
        .WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, SaveSessionKeyToDP(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AuthCredential(_, _, _, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_004, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_006, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH))
        .WillOnce(Return(ON_ERROR));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_007, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));
 EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, SaveSessionKeyToDP(_, _))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_008, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, SaveSessionKeyToDP(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AuthCredential(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_009, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH))
        .WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, SaveSessionKeyToDP(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AuthCredential(_, _, _, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(""));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthCredentialStateTest, AuthSrcCredentialAuthDoneState_0010, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->transmitData = "test";

    EXPECT_CALL(*dmHiChainAuthConnectorMock, ProcessCredData(_, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmAuthStateMachineMock, WaitExpectEvent(_))
        .WillOnce(Return(ON_SESSION_KEY_RETURNED))
        .WillOnce(Return(ON_FINISH))
        .WillOnce(Return(ON_TRANSMIT));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, SaveSessionKeyToDP(_, _))
        .WillOnce(Return(DM_OK));

    EXPECT_CALL(*dmHiChainAuthConnectorMock, AuthCredential(_, _, _, _)).WillOnce(Return(DM_OK));

    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_CALL(*dmSoftbusSessionMock, SendData(_, _)).WillOnce(Return(ERR_DM_FAILED));

    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

}
}
