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
#include "dm_auth_state_machine_cred.h"
#include "dm_log_3rd.h"
#include "softbus_connector_3rd.h"

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

/**
 * @tc.name: ParseMessage_004
 * @tc.desc: Cover ParseMessage branch where msgType is a valid integer but is not present in the
 *           parse-message function map, returning ERR_DM_FAILED.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage_004, testing::ext::TestSize.Level1)
{
    std::string message = "{\"msgType\":9999}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseMessage_005
 * @tc.desc: Cover ParseMessage branch where the JSON is valid but the msgType field is a string
 *           (not int32) so IsCredMessageValid fails, returning ERR_DM_FAILED.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage_005, testing::ext::TestSize.Level1)
{
    std::string message = "{\"msgType\":\"3010\"}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseMessage_006
 * @tc.desc: Cover ParseMessage branch where the input JSON cannot be decoded (IsDiscarded), making
 *           IsCredMessageValid fail, returning ERR_DM_FAILED.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage_006, testing::ext::TestSize.Level1)
{
    std::string message = "not_a_json";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseMessage3010_002
 * @tc.desc: Cover ParseMessage3010 success path where context and authStateMachine are valid so it
 *           parses the negotiation request and transitions to AuthSinkNegotiateStartState.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3010_002, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3010,\"version\":\"1.0\",\"logicalSessionId\":200,"
        "\"userId\":100,\"deviceIdHash\":\"testHash\"}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ParseMessage3020_001
 * @tc.desc: Cover ParseMessage3020 success path where context and authStateMachine are valid so it
 *           parses the negotiation response and transitions to AuthSrcCredentialAuthStartState.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3020_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3020,\"deviceVersion\":\"1.0\",\"userId\":100,\"deviceIdHash\":\"h\"}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ParseMessage3020_002
 * @tc.desc: Cover ParseMessage3020 null-authStateMachine branch returning ERR_DM_POINT_NULL.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3020_002, testing::ext::TestSize.Level1)
{
    std::string message = "{\"msgType\":3020,\"deviceVersion\":\"1.0\"}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: ParseMessage3030_001
 * @tc.desc: Cover ParseMessage3030 success path where context and authStateMachine are valid so it
 *           parses the credential-auth-start request and transitions.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3030_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3030,\"data\":\"testData\"}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ParseMessage3030_002
 * @tc.desc: Cover ParseMessage3030 null-context branch returning ERR_DM_POINT_NULL.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3030_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthCredContext> nullContext = nullptr;
    std::string message = "{\"msgType\":3030,\"data\":\"testData\"}";
    int32_t ret = messageProcessor->ParseMessage(nullContext, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseMessage3040_001
 * @tc.desc: Cover ParseMessage3040 success path where context and authStateMachine are valid so it
 *           parses the credential-auth-start response and transitions.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3040_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3040,\"data\":\"testData\"}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ParseMessage3040_002
 * @tc.desc: Cover ParseMessage3040 null-authStateMachine branch returning ERR_DM_POINT_NULL.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3040_002, testing::ext::TestSize.Level1)
{
    std::string message = "{\"msgType\":3040,\"data\":\"testData\"}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: ParseMessage3050_001
 * @tc.desc: Cover ParseMessage3050 path where reason is non-zero so the ON_FAIL event is notified
 *           before transitioning to AuthCredSinkFinishState.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3050_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3050,\"reply\":1,\"reason\":-1}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ParseMessage3050_002
 * @tc.desc: Cover ParseMessage3050 path where reason is DM_OK so the ON_FAIL branch is skipped and
 *           only ON_NEXT is notified before transitioning to AuthCredSinkFinishState.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3050_002, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3050,\"reply\":0,\"reason\":0}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ParseMessage3060_001
 * @tc.desc: Cover ParseMessage3060 path where reason is non-zero so the ON_FAIL event is notified
 *           before transitioning to AuthCredSrcFinishState.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3060_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3060,\"reply\":1,\"reason\":-1}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ParseMessage3060_002
 * @tc.desc: Cover ParseMessage3060 path where reason is DM_OK so the ON_FAIL branch is skipped and
 *           only ON_NEXT is notified before transitioning to AuthCredSrcFinishState.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, ParseMessage3060_002, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    context->authStateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::string message = "{\"msgType\":3060,\"reply\":0,\"reason\":0}";
    int32_t ret = messageProcessor->ParseMessage(context, message);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: CreateMessage_009
 * @tc.desc: Cover CreateMessage3020 path that builds the negotiation response from accessee fields.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthMessageProcessorCredTest, CreateMessage_009, testing::ext::TestSize.Level1)
{
    context->accessee.dmVersion = "1.0";
    context->accessee.userId = 100;
    context->accessee.deviceIdHash = "testHash";
    context->logicalSessionId = 12345;
    std::string message = messageProcessor->CreateMessage(DmCredMessageType::CRED_RESP_NEGOTIATE, context);
    EXPECT_NE(message, "");
}

}
}