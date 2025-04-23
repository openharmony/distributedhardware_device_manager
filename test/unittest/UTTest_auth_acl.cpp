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
#include "UTTest_auth_acl.h"
#include "dm_auth_message_processor_mock.h"
#include "device_manager_service_listener.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {
constexpr const char *TEST_NONE_EMPTY_STRING = "test";
void AuthAclTest::SetUpTestCase()
{
    LOGI("AuthAclTest::SetUpTestCase start.");
    DmSoftbusConnector::dmSoftbusConnector = dmSoftbusConnectorMock;
    DmAuthMessageProcessorMock::dmAuthMessageProcessorMock = std::make_shared<DmAuthMessageProcessorMock>();
}

void AuthAclTest::TearDownTestCase()
{
    LOGI("AuthAclTest::TearDownTestCase start.");
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    dmSoftbusConnectorMock = nullptr;
    DmAuthMessageProcessorMock::dmAuthMessageProcessorMock = nullptr;
}

void AuthAclTest::SetUp()
{
    LOGI("AuthAclTest::SetUp start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
}

void AuthAclTest::TearDown()
{
    LOGI("AuthAclTest::TearDown start.");
    authManager = nullptr;
    context = nullptr;

    Mock::VerifyAndClearExpectations(&*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock);
    Mock::VerifyAndClearExpectations(&*DmSoftbusConnector::dmSoftbusConnector);
}

HWTEST_F(AuthAclTest, AuthSinkAcl_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();

    EXPECT_CALL(*DmSoftbusConnector::dmSoftbusConnector, SyncLocalAclListProcess(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthAclTest, AuthSinkAcl_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();

    EXPECT_EQ(authState->GetStateType(), DM_OK);
}

HWTEST_F(AuthAclTest, AuthSrcAcl_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcDataSyncState>();
    context->direction = DM_AUTH_SOURCE;
    context->accesser.isAuthed = true; // no need to agree acl
    context->accesser.isOnline = true; // no need to join network
    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthAclTest, AuthSrcAcl_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcDataSyncState>();

    EXPECT_EQ(authState->GetStateType(), DM_OK);
}

HWTEST_F(AuthAclTest, AuthSrcFinish_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcFinishState>();
    context->reason = DM_OK;
    context->connDelayCloseTime = 10;
    EXPECT_CALL(*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock, CreateMessage(_, _))
        .WillOnce(Return(TEST_NONE_EMPTY_STRING));
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthAclTest, AuthSrcFinish_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcFinishState>();

    EXPECT_EQ(authState->GetStateType(), DM_OK);
}
}
}