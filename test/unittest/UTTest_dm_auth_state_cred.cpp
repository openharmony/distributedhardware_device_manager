/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UTTest_dm_auth_state_cred.h"
#include "device_manager_service_listener_3rd.h"
#include "dm_auth_state_machine_cred.h"
#include "dm_log_3rd.h"
#include "hichain_auth_connector_3rd.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

void DmAuthStateCredTest::SetUpTestCase()
{
    LOGI("SetUpTestCase start");
}

void DmAuthStateCredTest::TearDownTestCase()
{
    LOGI("TearDownTestCase start");
}

void DmAuthStateCredTest::SetUp()
{
    LOGI("SetUp start");
    context = std::make_shared<DmAuthCredContext>();
}

void DmAuthStateCredTest::TearDown()
{
    LOGI("TearDown start");
    context = nullptr;
    authState = nullptr;
}

HWTEST_F(DmAuthStateCredTest, GetTaskTimeout_001, testing::ext::TestSize.Level1)
{
    int32_t ret = DmAuthStateCred::GetTaskTimeout(context, AUTHENTICATE_TIMEOUT_TASK, 100);
    EXPECT_NE(ret, 100);
}

HWTEST_F(DmAuthStateCredTest, GetTaskTimeout_002, testing::ext::TestSize.Level1)
{
    int32_t ret = DmAuthStateCred::GetTaskTimeout(context, NEGOTIATE_TIMEOUT_TASK, 100);
    EXPECT_NE(ret, 100);
}

HWTEST_F(DmAuthStateCredTest, GetTaskTimeout_003, testing::ext::TestSize.Level1)
{
    int32_t ret = DmAuthStateCred::GetTaskTimeout(context, "unknown_task", 100);
    EXPECT_EQ(ret, 100);
}

HWTEST_F(DmAuthStateCredTest, GetTaskTimeout_004, testing::ext::TestSize.Level1)
{
    int32_t ret = DmAuthStateCred::GetTaskTimeout(context, nullptr, 100);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DmAuthStateCredTest, GetSysTimeMs_001, testing::ext::TestSize.Level1)
{
    uint64_t ret = DmAuthStateCred::GetSysTimeMs();
    EXPECT_NE(ret, 0);
}

HWTEST_F(DmAuthStateCredTest, BuildTrustDeviceInfos_001, testing::ext::TestSize.Level1)
{
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    context->reason = DM_OK;
    context->direction = DM_AUTH_CRED_SOURCE;
    context->accesser.sessionKey = {1, 2, 3, 4, 5};
    authState = std::make_shared<AuthCredSrcFinishState>();
    authState->BuildTrustDeviceInfos(context, deviceInfos);
    EXPECT_EQ(deviceInfos.size(), 1);
}

HWTEST_F(DmAuthStateCredTest, BuildTrustDeviceInfos_002, testing::ext::TestSize.Level1)
{
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    context->reason = ERR_DM_FAILED;
    authState = std::make_shared<AuthCredSrcFinishState>();
    authState->BuildTrustDeviceInfos(context, deviceInfos);
    EXPECT_EQ(deviceInfos.size(), 0);
}

HWTEST_F(DmAuthStateCredTest, BuildTrustDeviceInfos_003, testing::ext::TestSize.Level1)
{
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    context->reason = DM_OK;
    context->direction = DM_AUTH_CRED_SOURCE;
    context->accesser.sessionKey.clear();
    authState = std::make_shared<AuthCredSrcFinishState>();
    authState->BuildTrustDeviceInfos(context, deviceInfos);
    EXPECT_EQ(deviceInfos.size(), 0);
}

HWTEST_F(DmAuthStateCredTest, AuthCredSrcStartState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_AUTH_SRC_START_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthCredSrcStartState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcStartState>();
    int32_t ret = authState->Action(context);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthStateCredTest, AuthSrcNegotiateStartState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcNegotiateStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_AUTH_SRC_NEGOTIATE_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthSinkNegotiateStartState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkNegotiateStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_AUTH_SINK_NEGOTIATE_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthSrcCredentialAuthStartState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_START_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthSinkCredentialAuthStartState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkCredentialAuthStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_START_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthSrcCredentialAuthMsgNegotiateState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcCredentialAuthMsgNegotiateState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthSinkCredentialAuthMsgNegotiateState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkCredentialAuthMsgNegotiateState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthSrcCredentialAuthDoneState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_DONE_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthSinkCredentialAuthDoneState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkCredentialAuthDoneState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_DONE_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthCredSrcFinishState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_AUTH_SRC_FINISH_STATE);
}

HWTEST_F(DmAuthStateCredTest, AuthCredSinkFinishState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSinkFinishState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::CRED_AUTH_SINK_FINISH_STATE);
}

/**
 * @tc.name: QueryCredential_001
 * @tc.desc: Cover DmAuthStateCred::QueryCredential null-context early-return guard that returns
 *           ERR_DM_POINT_NULL without dereferencing the context.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, QueryCredential_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    int32_t ret = authState->QueryCredential(nullContext);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: QueryCredential_002
 * @tc.desc: Cover DmAuthStateCred::QueryCredential null-hiChainAuthConnector guard that returns
 *           ERR_DM_POINT_NULL when the context is valid but the connector is absent.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, QueryCredential_002, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    int32_t ret = authState->QueryCredential(context);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: QueryP2pCredential_001
 * @tc.desc: Cover DmAuthStateCred::QueryP2pCredential null-hiChainAuthConnector guard that returns
 *           ERR_DM_POINT_NULL when the connector is absent.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, QueryP2pCredential_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    context->accesser.credType = DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED;
    int32_t ret = authState->QueryP2pCredential(context);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: CheckOpenId_001
 * @tc.desc: Cover DmAuthStateCred::CheckOpenId null-context guard that returns false.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, CheckOpenId_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    JsonObject item("{\"extendInfo\":\"{}\"}");
    bool ret = authState->CheckOpenId(nullContext, item);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckOpenId_002
 * @tc.desc: Cover DmAuthStateCred::CheckOpenId short-circuit branch that returns true when the
 *           context accesser openIdHash is empty.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, CheckOpenId_002, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    JsonObject item("{\"extendInfo\":\"{}\"}");
    bool ret = authState->CheckOpenId(context, item);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: HandleAuthenticateTimeout_001
 * @tc.desc: Cover DmAuthStateCred::HandleAuthenticateTimeout null-context early-return guard that
 *           returns without dereferencing the context.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, HandleAuthenticateTimeout_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    authState->HandleAuthenticateTimeout(nullContext, "testTimer");
    SUCCEED();
}

/**
 * @tc.name: SourceFinish_001
 * @tc.desc: Cover DmAuthStateCred::SourceFinish null-context early-return guard that returns
 *           without dereferencing the context.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, SourceFinish_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    authState->SourceFinish(nullContext);
    SUCCEED();
}

/**
 * @tc.name: SinkFinish_001
 * @tc.desc: Cover DmAuthStateCred::SinkFinish null-context early-return guard that returns
 *           without dereferencing the context.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, SinkFinish_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSinkFinishState>();
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    authState->SinkFinish(nullContext);
    SUCCEED();
}

/**
 * @tc.name: BuildTrustDeviceInfos_004
 * @tc.desc: Cover DmAuthStateCred::BuildTrustDeviceInfos sink-direction branch where reason is
 *           DM_OK but the accessee sessionKey is empty, yielding zero device infos.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateCredTest, BuildTrustDeviceInfos_004, testing::ext::TestSize.Level1)
{
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    context->reason = DM_OK;
    context->direction = DM_AUTH_CRED_SINK;
    context->accessee.sessionKey.clear();
    authState = std::make_shared<AuthCredSinkFinishState>();
    authState->BuildTrustDeviceInfos(context, deviceInfos);
    EXPECT_EQ(deviceInfos.size(), 0);
}
}
}