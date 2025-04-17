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

#include "device_manager_service_listener.h"
#include "dm_auth_state.h"
#include "UTTest_auth_negotiate_state.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

constexpr const char* TEST_STRING = "test_string";
constexpr int32_t TEST_NEGATIVE = -1;
constexpr int32_t TEST_POSITIVE = 1;

void AuthNegotiateStateTest::SetUpTestCase()
{
    LOGI("AuthNegotiateStateTest::SetUpTestCase start.");

    DmSoftbusConnector::dmSoftbusConnector = softbusConnectorMock;
    DmSoftbusSession::dmSoftbusSession = softbusSessionMock;
}

void AuthNegotiateStateTest::TearDownTestCase()
{
    LOGI("AuthNegotiateStateTest::TearDownTestCase done.");
    softbusConnectorMock = nullptr;
    softbusSessionMock = nullptr;
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    DmSoftbusSession::dmSoftbusSession = nullptr;
}

void AuthNegotiateStateTest::SetUp()
{
    LOGI("AuthNegotiateStateTest::SetUp start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    softbusSession = std::make_shared<SoftbusSession>();

    Mock::VerifyAndClearExpectations(&*softbusConnectorMock);
    Mock::VerifyAndClearExpectations(&*softbusSessionMock);
}

void AuthNegotiateStateTest::TearDown()
{
    LOGI("AuthNegotiateStateTest::TearDown done.");
    softbusConnector = nullptr;
    listener = nullptr;
    hiChainAuthConnector = nullptr;
    authManager = nullptr;
    context = nullptr;
    softbusSession = nullptr;
}

HWTEST_F(AuthNegotiateStateTest, AuthSrcStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcStartState> authState = std::make_shared<AuthSrcStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_START_STATE);
}

HWTEST_F(AuthNegotiateStateTest, AuthSrcStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcStartState> authState = std::make_shared<AuthSrcStartState>();
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthNegotiateStateTest, AuthSrcNegotiateStateMachine_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcNegotiateStateMachine> authState = std::make_shared<AuthSrcNegotiateStateMachine>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_NEGOTIATE_STATE);
}

HWTEST_F(AuthNegotiateStateTest, AuthSrcNegotiateStateMachine_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcNegotiateStateMachine> authState = std::make_shared<AuthSrcNegotiateStateMachine>();

    EXPECT_CALL(*softbusConnectorMock, GetSoftbusSession)
        .WillOnce(Return(softbusSession));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthNegotiateStateTest, AuthSinkNegotiateStateMachine_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_NEGOTIATE_STATE);
}
}
}