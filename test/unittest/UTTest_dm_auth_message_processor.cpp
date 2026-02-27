/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "deviceprofile_connector.h"
#include "UTTest_dm_auth_message_processor.h"

using namespace testing;
namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DP_PERMISSION_DENIED = 98566155;
void DmAuthMessageProcessorTest::SetUpTestCase()
{
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock_;
}

void DmAuthMessageProcessorTest::TearDownTestCase()
{
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    deviceProfileConnectorMock_ = nullptr;
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

HWTEST_F(DmAuthMessageProcessorTest, ParseNegotiateMessage_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authStateMachine = std::make_shared<DmAuthStateMachine>(context);
    std::shared_ptr<DmAuthMessageProcessor> processor = std::make_shared<DmAuthMessageProcessor>();
    JsonObject jsonObj;
    jsonObj["ultrasonicSide"] = 0;
    int32_t ret = processor->ParseNegotiateMessage(jsonObj, context);
    jsonObj["ultrasonicSide"] = 1;
    ret = processor->ParseNegotiateMessage(jsonObj, context);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(context->accessee.extraInfo.empty(), false);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseServiceNego_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_IS_SERVICE_BIND] = false;
    auto context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    processor->ParseServiceNego(jsonObject, context);
    EXPECT_FALSE(context->isServiceBind);
}

HWTEST_F(DmAuthMessageProcessorTest, SetSyncMsgJson_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();

    context->accesser.deviceId = "localUdid";
    context->accessee.deviceId = "remoteUdid";
    context->accesser.userId = 100;
    context->accessee.userId = 200;
    context->direction = DM_AUTH_SINK;
    context->confirmOperation = USER_OPERATION_TYPE_ALLOW_AUTH;
    context->accesser.isCommonFlag = true;
    context->accesser.cert = "certData";
    context->isServiceBind = true;
    context->accessee.serviceId = 123456;

    DmAccess accessSide;
    accessSide.transmitSessionKeyId = 1;
    accessSide.transmitSkTimeStamp = 123;
    accessSide.transmitCredentialId = "credId";
    accessSide.isGenerateLnnCredential = true;
    accessSide.bindLevel = 2;
    accessSide.lnnSessionKeyId = 3;
    accessSide.lnnSkTimeStamp = 456;
    accessSide.lnnCredentialId = "lnnCredId";
    accessSide.dmVersion = "1.0.0";

    DmAccessToSync accessToSync;
    accessToSync.deviceName = "devName";
    accessToSync.deviceNameFull = "devNameFull";
    accessToSync.deviceId = "devId";
    accessToSync.userId = 100;
    accessToSync.accountId = "accId";
    accessToSync.tokenId = 789;
    accessToSync.bundleName = "bundle";
    accessToSync.pkgName = "pkg";
    accessToSync.bindLevel = 2;
    accessToSync.sessionKeyId = 1;
    accessToSync.skTimeStamp = 123;

    JsonObject syncMsgJson;
    int ret = processor->SetSyncMsgJson(context, accessSide, accessToSync, syncMsgJson);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(syncMsgJson[TAG_TRANSMIT_SK_ID].Get<std::string>(), "1");
    EXPECT_EQ(syncMsgJson[TAG_TRANSMIT_SK_TIMESTAMP].Get<int64_t>(), 123);
    EXPECT_EQ(syncMsgJson[TAG_TRANSMIT_CREDENTIAL_ID].Get<std::string>(), "credId");
    EXPECT_EQ(syncMsgJson[TAG_LNN_SK_ID].Get<std::string>(), "3");
    EXPECT_EQ(syncMsgJson[TAG_LNN_SK_TIMESTAMP].Get<int64_t>(), 456);
    EXPECT_EQ(syncMsgJson[TAG_LNN_CREDENTIAL_ID].Get<std::string>(), "lnnCredId");
    EXPECT_EQ(syncMsgJson[TAG_DMVERSION].Get<std::string>(), "1.0.0");
    EXPECT_EQ(syncMsgJson[TAG_IS_COMMON_FLAG].Get<bool>(), true);
    EXPECT_EQ(syncMsgJson[TAG_DM_CERT_CHAIN].Get<std::string>(), "certData");
    EXPECT_TRUE(syncMsgJson.Contains(TAG_ACL_CHECKSUM));
    EXPECT_TRUE(syncMsgJson.Contains(TAG_USER_CONFIRM_OPT));
}

HWTEST_F(DmAuthMessageProcessorTest, SetSyncMsgJson_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    context->authType = AUTH_TYPE_NFC;
    context->accesser.pkgName = "pkgTest";
    DmAccess accessSide;
    DmAccessToSync accessToSync;
    JsonObject syncMsgJson;
    int ret = processor->SetSyncMsgJson(context, accessSide, accessToSync, syncMsgJson);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(syncMsgJson[PIN_MATCH_FLAG].Get<bool>(), false);
}

HWTEST_F(DmAuthMessageProcessorTest, SetSyncMsgJson_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    context->authType = AUTH_TYPE_NFC;
    context->accesser.pkgName = "pkgTest";
    EXPECT_CALL(*deviceProfileConnectorMock_, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(Return(DM_OK));
    DmAccess accessSide;
    DmAccessToSync accessToSync;
    JsonObject syncMsgJson;
    int ret = processor->SetSyncMsgJson(context, accessSide, accessToSync, syncMsgJson);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(syncMsgJson[PIN_MATCH_FLAG].Get<bool>(), false);
}

HWTEST_F(DmAuthMessageProcessorTest, SetSyncMsgJson_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    context->authType = AUTH_TYPE_NFC;
    context->accesser.pkgName = "pkgTest";
    EXPECT_CALL(*deviceProfileConnectorMock_, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(Return(DM_OK));
    DmAccess accessSide;
    DmAccessToSync accessToSync;
    DistributedDeviceProfile::LocalServiceInfo oldSrvInfo;
    oldSrvInfo.SetExtraInfo(R"({})");
    std::string strJson = oldSrvInfo.GetExtraInfo();
    JsonObject syncMsgJson(strJson);
    int ret = processor->SetSyncMsgJson(context, accessSide, accessToSync, syncMsgJson);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(syncMsgJson[PIN_MATCH_FLAG].Get<bool>(), false);
}

HWTEST_F(DmAuthMessageProcessorTest, SetSyncMsgJson_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    context->authType = AUTH_TYPE_NFC;
    context->accesser.pkgName = "pkgTest";
    EXPECT_CALL(*deviceProfileConnectorMock_, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(Return(DM_OK));
    DmAccess accessSide;
    DmAccessToSync accessToSync;
    DistributedDeviceProfile::LocalServiceInfo oldSrvInfo;
    oldSrvInfo.SetExtraInfo(R"({"pinMatchFlag": false})");
    std::string strJson = oldSrvInfo.GetExtraInfo();
    JsonObject syncMsgJson(strJson);
    int ret = processor->SetSyncMsgJson(context, accessSide, accessToSync, syncMsgJson);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(syncMsgJson[PIN_MATCH_FLAG].Get<bool>(), false);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseSyncMessage_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    std::string accessStr = "test_access";
    std::string aclStr = "test_acl";
    JsonObject jsonObject;
    jsonObject[PIN_MATCH_FLAG] = true;
    jsonObject["ncm_bind_target"] = false;
    DmAccess access;
    int32_t result = processor->ParseSyncMessage(context, access, jsonObject);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseSyncMessage_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    std::string accessStr = "test_access";
    std::string aclStr = "test_acl";
    JsonObject jsonObject;
    jsonObject[PIN_MATCH_FLAG] = true;
    DmAccess access;
    int32_t result = processor->ParseSyncMessage(context, access, jsonObject);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseSyncMessage_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    std::string accessStr = "test_access";
    std::string aclStr = "test_acl";
    JsonObject jsonObject;
    jsonObject["ncm_bind_target"] = false;
    DmAccess access;
    int32_t result = processor->ParseSyncMessage(context, access, jsonObject);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DmAuthMessageProcessorTest, ParseSyncMessage_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    auto processor = std::make_shared<DmAuthMessageProcessor>();
    context->direction = DM_AUTH_SOURCE;
    std::string accessStr = "test_access";
    std::string aclStr = "test_acl";
    JsonObject jsonObject;
    DmAccess access;
    int32_t result = processor->ParseSyncMessage(context, access, jsonObject);
    EXPECT_EQ(result, ERR_DM_FAILED);
}
} // namespace DistributedHardware
} // namespace OHOS

