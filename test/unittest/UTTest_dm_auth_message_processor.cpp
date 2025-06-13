/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dm_log.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_context.h"
#include "dm_auth_state_machine.h"
#include "UTTest_dm_auth_message_processor.h"

using namespace testing;
namespace OHOS {
namespace DistributedHardware {

void DmAuthMessageProcessorTest::SetUpTestCase()
{
}

void DmAuthMessageProcessorTest::TearDownTestCase()
{
}

void DmAuthMessageProcessorTest::SetUp()
{
}

void DmAuthMessageProcessorTest::TearDown()
{
}

HWTEST_F(DmAuthMessageProcessorTest, CreateMessageForwardUltrasonicStart_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->CreateMessageForwardUltrasonicStart(context, json), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, CreateMessageReverseUltrasonicDone_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->CreateMessageReverseUltrasonicDone(context, json), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, CreateMessageReverseUltrasonicStart_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->CreateMessageReverseUltrasonicStart(context, json), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, CreateMessageForwardUltrasonicNegotiate_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->CreateMessageForwardUltrasonicNegotiate(context, json), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseMessageReverseUltrasonicStart_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->ParseMessageReverseUltrasonicStart(json, context), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseMessageReverseUltrasonicDone_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    json["REPLY"] = "1";
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->reply = DM_OK;
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->ParseMessageReverseUltrasonicDone(json, context), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseMessageForwardUltrasonicStart_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->ParseMessageForwardUltrasonicStart(json, context), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseMessageForwardUltrasonicNegotiate_001, testing::ext::TestSize.Level1)
{
    JsonObject json;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    EXPECT_EQ(processor->ParseMessageForwardUltrasonicNegotiate(json, context), DM_OK);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseNegotiateMessage_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    JsonObject jsonObject;
    jsonObject[DM_TAG_LOGICAL_SESSION_ID] = 12345;

    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    int32_t ret = processor->ParseNegotiateMessage(jsonObject, context);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context->logicalSessionId, 12345);
    EXPECT_EQ(context->requestId, 12345);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseNegotiateMessage_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    JsonObject jsonObject;
    jsonObject[TAG_PEER_PKG_NAME] = "testPkgName";

    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    int32_t ret = processor->ParseNegotiateMessage(jsonObject, context);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context->accessee.pkgName, "testPkgName");
}

HWTEST_F(DmAuthMessageProcessorTest, ParseNegotiateMessage_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    JsonObject jsonObject;
    jsonObject[TAG_PEER_BUNDLE_NAME_V2] = "testBundleName"; 

    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    int32_t ret = processor->ParseNegotiateMessage(jsonObject, context);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context->accessee.bundleName, "testBundleName");
}

HWTEST_F(DmAuthMessageProcessorTest, ParseNegotiateMessage_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    JsonObject jsonObject;
    jsonObject[TAG_PEER_DISPLAY_ID] = 123;

    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    int32_t ret = processor->ParseNegotiateMessage(jsonObject, context);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context->accessee.displayId, 123);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseNegotiateMessage_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    JsonObject jsonObject;
    jsonObject[TAG_HOST_PKGLABEL] = "testPkgLabel";

    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    int32_t ret = processor->ParseNegotiateMessage(jsonObject, context);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context->pkgLabel, "testPkgLabel");
}

HWTEST_F(DmAuthMessageProcessorTest, ParseNegotiateMessage_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    JsonObject jsonObject;
    jsonObject[DM_BUSINESS_ID] = "testBusinessId";

    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    int32_t ret = processor->ParseNegotiateMessage(jsonObject, context);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context->businessId, "testBusinessId");
}
} // namespace DistributedHardware
} // namespace OHOS
 