/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UTTest_auth_pincode_message_processor.h"

#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_message_processor_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_auth_pincode_state_3rd.h"
#include "dm_constants_3rd.h"
#include "json_object.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

namespace {
std::shared_ptr<DmAuthPincodeContext> context_;
std::shared_ptr<DmAuthPincodeMessageProcessor3rd> msgProcessor_;
}

void AuthPincodeMessageProcessorTest::SetUpTestCase()
{
    LOGI("AuthPincodeMessageProcessorTest::SetUpTestCase");
}

void AuthPincodeMessageProcessorTest::TearDownTestCase()
{
    LOGI("AuthPincodeMessageProcessorTest::TearDownTestCase");
}

void AuthPincodeMessageProcessorTest::SetUp()
{
    LOGI("AuthPincodeMessageProcessorTest::SetUp");
    context_ = std::make_shared<DmAuthPincodeContext>();
    context_->direction = DM_AUTH_PINCODE_SOURCE;
    context_->authPinStateMac3rd = std::make_shared<DmAuthPincodeStateMachine3rd>(context_);
    msgProcessor_ = std::make_shared<DmAuthPincodeMessageProcessor3rd>();
}

void AuthPincodeMessageProcessorTest::TearDown()
{
    LOGI("AuthPincodeMessageProcessorTest::TearDown");
    if (context_ != nullptr && context_->authPinStateMac3rd != nullptr) {
        context_->authPinStateMac3rd->Stop();
    }
    context_ = nullptr;
    msgProcessor_ = nullptr;
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_CreateMessage1010_001
 * @tc.desc: Test creating AUTH_PINCODE_REQ_NEGOTIATE message
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_CreateMessage1010_001, TestSize.Level1)
{
    context_->logicalSessionId = 12345;
    context_->accesser.deviceIdHash = "device_id_hash";
    context_->accesser.userId = 100;
    context_->accesser.accountIdHash = "account_hash";
    context_->accesser.tokenIdHash = "token_hash";
    context_->accesser.businessName = "test_business";
    context_->accesser.processName = "test_process";
    context_->accesser.uid = 1001;
    context_->accesser.dmVersion = "1.0";
    context_->accessee.businessName = "peer_business";
    context_->accessee.processName = "peer_process";

    std::string message = msgProcessor_->CreateMessage(DmPincodeMessageType::AUTH_PINCODE_REQ_NEGOTIATE, context_);
    ASSERT_FALSE(message.empty());

    JsonObject jsonObj(message);
    EXPECT_EQ(jsonObj[TAG_MSG_TYPE].Get<int32_t>(), AUTH_PINCODE_REQ_NEGOTIATE);
    EXPECT_EQ(jsonObj[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>(), 12345);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_CreateMessage1020_001
 * @tc.desc: Test creating AUTH_PINCODE_RESP_NEGOTIATE message
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_CreateMessage1020_001, TestSize.Level1)
{
    context_->logicalSessionId = 12345;
    context_->accessee.dmVersion = "1.0";
    context_->accessee.deviceIdHash = "device_id_hash";
    context_->accessee.userId = 100;
    context_->accessee.accountIdHash = "account_hash";
    context_->accessee.tokenIdHash = "token_hash";
    context_->accessee.networkId = "network_id";

    std::string message = msgProcessor_->CreateMessage(DmPincodeMessageType::AUTH_PINCODE_RESP_NEGOTIATE, context_);
    ASSERT_FALSE(message.empty());

    JsonObject jsonObj(message);
    EXPECT_EQ(jsonObj[TAG_MSG_TYPE].Get<int32_t>(), AUTH_PINCODE_RESP_NEGOTIATE);
    EXPECT_EQ(jsonObj[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>(), 12345);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_CreateMessage1030_001
 * @tc.desc: Test creating AUTH_PINCODE_REQ_PIN_AUTH_START message
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_CreateMessage1030_001, TestSize.Level1)
{
    context_->logicalSessionId = 12345;
    context_->transmitData = "transmit_data_test";

    std::string message = msgProcessor_->CreateMessage(DmPincodeMessageType::AUTH_PINCODE_REQ_PIN_AUTH_START, context_);
    ASSERT_FALSE(message.empty());

    JsonObject jsonObj(message);
    EXPECT_EQ(jsonObj[TAG_MSG_TYPE].Get<int32_t>(), AUTH_PINCODE_REQ_PIN_AUTH_START);
    EXPECT_EQ(jsonObj[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>(), 12345);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_CreateMessage1050_001
 * @tc.desc: Test creating AUTH_PINCODE_REQ_FINISH message
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_CreateMessage1050_001, TestSize.Level1)
{
    context_->logicalSessionId = 12345;
    context_->reply = 1;
    context_->reason = DM_OK;

    std::string message = msgProcessor_->CreateMessage(DmPincodeMessageType::AUTH_PINCODE_REQ_FINISH, context_);
    ASSERT_FALSE(message.empty());

    JsonObject jsonObj(message);
    EXPECT_EQ(jsonObj[TAG_MSG_TYPE].Get<int32_t>(), AUTH_PINCODE_REQ_FINISH);
    EXPECT_EQ(jsonObj[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>(), 12345);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_CreateMessage1060_001
 * @tc.desc: Test creating AUTH_PINCODE_RESP_FINISH message
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_CreateMessage1060_001, TestSize.Level1)
{
    context_->logicalSessionId = 12345;
    context_->reply = 0;
    context_->reason = DM_OK;

    std::string message = msgProcessor_->CreateMessage(DmPincodeMessageType::AUTH_PINCODE_RESP_FINISH, context_);
    ASSERT_FALSE(message.empty());

    JsonObject jsonObj(message);
    EXPECT_EQ(jsonObj[TAG_MSG_TYPE].Get<int32_t>(), AUTH_PINCODE_RESP_FINISH);
    EXPECT_EQ(jsonObj[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>(), 12345);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_ParseMessage1030_001
 * @tc.desc: Test parsing AUTH_PINCODE_REQ_PIN_AUTH_START message
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_ParseMessage1030_001, TestSize.Level1)
{
    JsonObject jsonMsg;
    jsonMsg[TAG_MSG_TYPE] = AUTH_PINCODE_REQ_PIN_AUTH_START;
    jsonMsg[TAG_DATA] = "test_data";

    std::string message = jsonMsg.Dump();
    int32_t ret = msgProcessor_->ParseMessage(context_, message);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context_->msgType, AUTH_PINCODE_REQ_PIN_AUTH_START);
    EXPECT_EQ(context_->transmitData, "test_data");
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_ParseMessage1060_001
 * @tc.desc: Test parsing AUTH_PINCODE_RESP_FINISH message
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_ParseMessage1060_001, TestSize.Level1)
{
    JsonObject jsonMsg;
    jsonMsg[TAG_MSG_TYPE] = AUTH_PINCODE_RESP_FINISH;
    jsonMsg[TAG_REPLY] = 0;
    jsonMsg[TAG_REASON] = DM_OK;

    std::string message = jsonMsg.Dump();
    int32_t ret = msgProcessor_->ParseMessage(context_, message);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context_->msgType, AUTH_PINCODE_RESP_FINISH);
    EXPECT_EQ(context_->reply, 0);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_InvalidMessageType_001
 * @tc.desc: Test creating message with invalid message type
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_InvalidMessageType_001, TestSize.Level1)
{
    std::string message = msgProcessor_->CreateMessage(static_cast<DmPincodeMessageType>(9999), context_);
    EXPECT_TRUE(message.empty());
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_ParseInvalidMessage_001
 * @tc.desc: Test parsing message with invalid message type
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_ParseInvalidMessage_001, TestSize.Level1)
{
    JsonObject jsonMsg;
    jsonMsg[TAG_MSG_TYPE] = 9999;

    std::string message = jsonMsg.Dump();
    int32_t ret = msgProcessor_->ParseMessage(context_, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthPincodeStateMachine_TransitionTo_001
 * @tc.desc: Test state machine transition with valid state
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeStateMachine_TransitionTo_001, TestSize.Level1)
{
    auto stateMachine = context_->authPinStateMac3rd;
    ASSERT_NE(stateMachine, nullptr);

    auto nextState = std::make_shared<AuthPincodeSrcStartState>();
    int32_t ret = stateMachine->TransitionTo(nextState);
    EXPECT_EQ(ret, DM_OK);

    stateMachine->Stop();
}

/**
 * @tc.name: DmAuthPincodeStateMachine_TransitionTo_002
 * @tc.desc: Test state machine transition to finish state
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeStateMachine_TransitionTo_002, TestSize.Level1)
{
    context_->direction = DM_AUTH_PINCODE_SOURCE;
    auto stateMachine = std::make_shared<DmAuthPincodeStateMachine3rd>(context_);

    auto finishState = std::make_shared<AuthPincodeSrcFinishState>();
    int32_t ret = stateMachine->TransitionTo(finishState);
    EXPECT_EQ(ret, DM_OK);

    stateMachine->Stop();
}

/**
 * @tc.name: DmAuthPincodeStateMachine_SinkTransition_001
 * @tc.desc: Test sink state machine transition
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeStateMachine_SinkTransition_001, TestSize.Level1)
{
    context_->direction = DM_AUTH_PINCODE_SINK;
    auto stateMachine = std::make_shared<DmAuthPincodeStateMachine3rd>(context_);

    auto finishState = std::make_shared<AuthPincodeSinkFinishState>();
    int32_t ret = stateMachine->TransitionTo(finishState);
    EXPECT_EQ(ret, DM_OK);

    stateMachine->Stop();
}

/**
 * @tc.name: DmAuthPincodeState_AuthPincodeSrcStartState_001
 * @tc.desc: Test AuthPincodeSrcStartState state type
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeState_AuthPincodeSrcStartState_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcStartState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_START_STATE);
}

/**
 * @tc.name: DmAuthPincodeState_AuthPincodeSinkFinishState_001
 * @tc.desc: Test AuthPincodeSinkFinishState state type
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeState_AuthPincodeSinkFinishState_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSinkFinishState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE);
}

/**
 * @tc.name: DmAuthPincodeState_AuthPincodeSrcFinishState_001
 * @tc.desc: Test AuthPincodeSrcFinishState state type
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeState_AuthPincodeSrcFinishState_001, TestSize.Level1)
{
    auto state = std::make_shared<AuthPincodeSrcFinishState>();
    EXPECT_EQ(state->GetStateType(), DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_FINISH_STATE);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_ParseMessage1050_001
 * @tc.desc: Test parsing AUTH_PINCODE_REQ_FINISH message with success
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeMessageProcessorTest, DmAuthPincodeMessageProcessor_ParseMessage1050_001, TestSize.Level1)
{
    JsonObject jsonMsg;
    jsonMsg[TAG_MSG_TYPE] = 2000;
    jsonMsg[TAG_REPLY] = 0;
    jsonMsg[TAG_REASON] = DM_OK;

    std::string message = jsonMsg.Dump();
    int32_t ret = msgProcessor_->ParseMessage(context_, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
} // namespace DistributedHardware
} // namespace OHOS