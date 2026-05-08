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

#include "UTTest_auth_cred_negotiate_cred.h"
#include "dm_log_3rd.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

void AuthCredNegotiateTest::SetUpTestCase()
{
    LOGI("SetUpTestCase start");
}

void AuthCredNegotiateTest::TearDownTestCase()
{
    LOGI("TearDownTestCase start");
}

void AuthCredNegotiateTest::SetUp()
{
    LOGI("SetUp start");
    context = std::make_shared<DmAuthCredContext>();
}

void AuthCredNegotiateTest::TearDown()
{
    LOGI("TearDown start");
    context = nullptr;
    authState = nullptr;
}

HWTEST_F(AuthCredNegotiateTest, AuthCredSrcStartState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcStartState>();
    int32_t ret = authState->Action(context);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSrcNegotiateStartState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcNegotiateStartState>();
    context->sessionId = 1;
    context->accesser.deviceId = "testDeviceId";
    context->accesser.accountId = "testAccountId";
    context->accesser.tokenId = 123;
    int32_t ret = authState->Action(context);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSinkNegotiateStartState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkNegotiateStartState>();
    context->sessionId = 1;
    context->accessee.processName = "testProcess";
    int32_t ret = authState->Action(context);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSinkCredentialAuthStartState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkCredentialAuthStartState>();
    int32_t ret = authState->Action(context);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSrcCredentialAuthStartState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcCredentialAuthStartState>();
    context->accesser.userId = 100;
    context->accesser.transmitCredentialId = "testCredentialId";
    context->requestId = 12345;
    int32_t ret = authState->Action(context);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSrcCredentialAuthMsgNegotiateState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcCredentialAuthMsgNegotiateState>();
    context->transmitData = "testData";
    int32_t ret = authState->Action(context);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSinkCredentialAuthMsgNegotiateState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkCredentialAuthMsgNegotiateState>();
    context->transmitData = "testData";
    int32_t ret = authState->Action(context);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSrcCredentialAuthDoneState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->reply = DM_OK;
    context->reason = DM_OK;
    context->sessionId = 1;
    int32_t ret = authState->Action(context);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthSinkCredentialAuthDoneState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthSinkCredentialAuthDoneState>();
    context->reply = DM_OK;
    context->reason = DM_OK;
    int32_t ret = authState->Action(context);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthCredSrcFinishState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSrcFinishState>();
    context->reason = DM_OK;
    context->logicalSessionId = 12345;
    context->connDelayCloseTime = 0;
    int32_t ret = authState->Action(context);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthCredNegotiateTest, AuthCredSinkFinishState_Action_001, testing::ext::TestSize.Level1)
{
    authState = std::make_shared<AuthCredSinkFinishState>();
    context->reason = DM_OK;
    context->logicalSessionId = 12345;
    context->connDelayCloseTime = 0;
    int32_t ret = authState->Action(context);
    EXPECT_EQ(ret, DM_OK);
}

}
}