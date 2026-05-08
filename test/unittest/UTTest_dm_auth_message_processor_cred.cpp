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

#include "UTTest_dm_auth_message_processor_cred.h"
#include "dm_log_3rd.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

void DmAuthMessageProcessorCredTest::SetUpTestCase()
{
    LOGI("SetUpTestCase start");
}

void DmAuthMessageProcessorCredTest::TearDownTestCase()
{
    LOGI("TearDownTestCase start");
}

void DmAuthMessageProcessorCredTest::SetUp()
{
    LOGI("SetUp start");
    messageProcessor = std::make_shared<DmAuthMessageProcessorCred>();
    context = std::make_shared<DmAuthCredContext>();
}

void DmAuthMessageProcessorCredTest::TearDown()
{
    LOGI("TearDown start");
    messageProcessor = nullptr;
    context = nullptr;
}

HWTEST_F(DmAuthMessageProcessorCredTest, Constructor_001, testing::ext::TestSize.Level1)
{
    EXPECT_NE(messageProcessor, nullptr);
}

HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage_001, testing::ext::TestSize.Level1)
{
    std::string message = "{\"msgType\":3010}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage_002, testing::ext::TestSize.Level1)
{
    std::string message = "";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    std::string message = "{\"msgType\":3010}";
    int32_t ret = messageProcessor->ParseMessage(nullContext, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3010_001, testing::ext::TestSize.Level1)
{
    std::string message = "{\"msgType\":3010,\"version\":\"1.0\",\"userId\":100,\"deviceIdHash\":\"testHash\","
        "\"businessName\":\"testBusiness\",\"processName\":\"testProcess\","
        "\"peerBusinessName\":\"peerBusiness\",\"peerProcessName\":\"peerProcess\","
        "\"openIdHash\":\"testOpenId\",\"ownerId\":\"testOwner\",\"credType\":1}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_REQ_NEGOTIATE, nullContext);
    EXPECT_EQ(message, "");
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_002, testing::ext::TestSize.Level1)
{
    context->accesser.dmVersion = "1.0";
    context->accesser.userId = 100;
    context->accesser.deviceIdHash = "testHash";
    context->accesser.businessName = "testBusiness";
    context->accesser.processName = "testProcess";
    context->accessee.businessName = "peerBusiness";
    context->accessee.processName = "peerProcess";
    context->accesser.openIdHash = "testOpenId";
    context->accesser.ownerId = "testOwner";
    context->accesser.credType = 1;
    context->logicalSessionId = 12345;
    
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_REQ_NEGOTIATE, context);
    EXPECT_NE(message, "");
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_003, testing::ext::TestSize.Level1)
{
    context->accessee.dmVersion = "1.0";
    context->accessee.userId = 100;
    context->accessee.deviceIdHash = "testHash";
    context->logicalSessionId = 12345;
    
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_RESP_NEGOTIATE, context);
    EXPECT_NE(message, "");
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_004, testing::ext::TestSize.Level1)
{
    context->transmitData = "testData";
    context->logicalSessionId = 12345;
    
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_REQ_CREDENTIAL_AUTH_START, context);
    EXPECT_NE(message, "");
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_005, testing::ext::TestSize.Level1)
{
    context->transmitData = "testData";
    context->logicalSessionId = 12345;
    
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_RESP_CREDENTIAL_AUTH_START, context);
    EXPECT_NE(message, "");
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_006, testing::ext::TestSize.Level1)
{
    context->reply = DM_OK;
    context->reason = DM_OK;
    context->logicalSessionId = 12345;
    
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_REQ_FINISH, context);
    EXPECT_NE(message, "");
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_007, testing::ext::TestSize.Level1)
{
    context->reply = DM_OK;
    context->reason = DM_OK;
    context->logicalSessionId = 12345;
    
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_RESP_FINISH, context);
    EXPECT_NE(message, "");
}

HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_008, testing::ext::TestSize.Level1)
{
    std::string message = messageProcessor->CreateMessage(static_cast<DmCredMessageType>(9999), context);
    EXPECT_EQ(message, "");
}

}
}