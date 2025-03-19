/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "UTTest_relationship_sync_mgr.h"

#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t MAX_USER_ID_NUM = 5;
void ReleationShipSyncMgrTest::SetUp()
{
}

void ReleationShipSyncMgrTest::TearDown()
{
}

void ReleationShipSyncMgrTest::SetUpTestCase()
{
}

void ReleationShipSyncMgrTest::TearDownTestCase()
{
}

namespace {

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_001, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = UINT32_MAX;
    msg.peerUdids = peerUdids;
    msg.accountId = "1";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_002, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_003, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_NE(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_004, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::DEVICE_UNBIND;
    msg.userId = UINT32_MAX;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_005, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::DEVICE_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_NE(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_006, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = UINT32_MAX;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_007, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    msg.tokenId = UINT64_MAX;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_008, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    msg.tokenId = 1;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_NE(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_009, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SERVICE_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    msg.tokenId = 1;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    msg.type = RelationShipChangeType::DEL_USER;
    broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    msg.type = RelationShipChangeType::APP_UNINSTALL;
    broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    msg.type = RelationShipChangeType::TYPE_MAX;
    broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_006, testing::ext::TestSize.Level1)
{
    std::string msg = R"(
    {
        "TYPE": 0,
        "VALUE": [
            { "type": 1, "userId": 1 },
            { "type": 2, "userId": 2 }
        ]
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
    msg = R"(
    {
        "TYPE": 1,
        "VALUE": [
            { "type": 1, "userId": 1 },
            { "type": 2, "userId": 2 }
        ]
    }
    )";
    relationShipMsg = ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
    msg = R"(
    {
        "TYPE": 2,
        "VALUE": [
            { "type": 1, "userId": 1 },
            { "type": 2, "userId": 2 }
        ]
    }
    )";
    relationShipMsg = ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_007, testing::ext::TestSize.Level1)
{
    std::string msg = R"(
    {
        "TYPE": 6,
        "VALUE": [
            { "type": 1, "userId": 1 },
            { "type": 2, "userId": 2 }
        ]
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
    msg = R"(
    {
        "TYPE": 4,
        "VALUE": [
            { "type": 1, "userId": 1 },
            { "type": 2, "userId": 2 }
        ]
    }
    )";
    relationShipMsg = ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, 0);
    msg = R"(
    {
        "TYPE": 7,
        "VALUE": [
            { "type": 1, "userId": 1 },
            { "type": 2, "userId": 2 }
        ]
    }
    )";
    relationShipMsg = ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, 0);
}

HWTEST_F(ReleationShipSyncMgrTest, IsValid_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msgObj;
    msgObj.type = static_cast<RelationShipChangeType>(10);
    bool ret = msgObj.IsValid();
    ASSERT_EQ(ret, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromBroadcastPayLoad_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msgObj;
    cJSON payloadJson;
    RelationShipChangeType type = RelationShipChangeType::TYPE_MAX;
    bool ret = msgObj.FromBroadcastPayLoad(&payloadJson, type);
    ASSERT_EQ(ret, false);
    type = RelationShipChangeType::SERVICE_UNBIND;
    ret = msgObj.FromBroadcastPayLoad(&payloadJson, type);
    ASSERT_EQ(ret, false);
}

HWTEST_F(ReleationShipSyncMgrTest, ToSyncFrontOrBackUserIdPayLoad_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.userIdInfos.push_back({1, false});
    msg.userIdInfos.push_back({2, false});
    msg.userIdInfos.push_back({3, false});
    msg.userIdInfos.push_back({4, false});
    msg.userIdInfos.push_back({5, false});
    msg.userIdInfos.push_back({6, true});
    uint8_t* msgPtr = nullptr;
    uint32_t len = 0;
    bool result = msg.ToSyncFrontOrBackUserIdPayLoad(msgPtr, len);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, ToSyncFrontOrBackUserIdPayLoad_002, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.userIdInfos.push_back({1, false});
    msg.userIdInfos.push_back({2, true});
    uint8_t* msgPtr = nullptr;
    uint32_t len = 10;
    msg.syncUserIdFlag = true;
    bool result = msg.ToSyncFrontOrBackUserIdPayLoad(msgPtr, len);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, FromAccountLogoutPayLoad_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    cJSON payloadJson;
    bool result = msg.FromAccountLogoutPayLoad(&payloadJson);
    ASSERT_EQ(result, false);
    result = msg.FromAccountLogoutPayLoad(nullptr);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromAccountLogoutPayLoad_002, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": 0,
        "userId": 12345,
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromAccountLogoutPayLoad(jsonObject);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, FromAccountLogoutPayLoad_003, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromAccountLogoutPayLoad(jsonObject);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, FromAccountLogoutPayLoad_004, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "test1": 1,
        "test2": 2,
        "test3": 3,
        "test4": 4,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromAccountLogoutPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromDeviceUnbindPayLoad_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    cJSON payloadJson;
    bool result = msg.FromDeviceUnbindPayLoad(&payloadJson);
    ASSERT_EQ(result, false);
    result = msg.FromDeviceUnbindPayLoad(nullptr);
    ASSERT_EQ(result, false);
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "test1": 1,
        "test2": 2,
        "test3": 3,
        "test4": 4,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    result = msg.FromDeviceUnbindPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromDeviceUnbindPayLoad_002, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromDeviceUnbindPayLoad(jsonObject);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, FromDeviceUnbindPayLoad_003, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": 0,
        "userId": 12345,
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromDeviceUnbindPayLoad(jsonObject);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, FromAppUnbindPayLoad_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    cJSON payloadJson;
    bool result = msg.FromAppUnbindPayLoad(&payloadJson);
    ASSERT_EQ(result, false);
    result = msg.FromAppUnbindPayLoad(nullptr);
    ASSERT_EQ(result, false);
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "test1": 1,
        "test2": 2,
        "test3": 3,
        "test4": 4,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    result = msg.FromAppUnbindPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromAppUnbindPayLoad_002, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromAppUnbindPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromAppUnbindPayLoad_003, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": 0,
        "userId": 12345,
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromAppUnbindPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromSyncFrontOrBackUserIdPayLoad_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    cJSON payloadJson;
    bool result = msg.FromSyncFrontOrBackUserIdPayLoad(&payloadJson);
    ASSERT_EQ(result, false);
    result = msg.FromSyncFrontOrBackUserIdPayLoad(nullptr);
    ASSERT_EQ(result, false);
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "test1": 1,
        "test2": 2,
        "test3": 3,
        "test4": 4,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    result = msg.FromSyncFrontOrBackUserIdPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromSyncFrontOrBackUserIdPayLoad_002, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": 6,
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromSyncFrontOrBackUserIdPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromSyncFrontOrBackUserIdPayLoad_003, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": 1,
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromSyncFrontOrBackUserIdPayLoad(jsonObject);
    ASSERT_EQ(result, false);
}

HWTEST_F(ReleationShipSyncMgrTest, FromSyncFrontOrBackUserIdPayLoad_004, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": "1",
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromSyncFrontOrBackUserIdPayLoad(jsonObject);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, FromSyncFrontOrBackUserIdPayLoad_005, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    const char* jsonString = R"({
        "MsgType": 1,
        "userId": 12345,
        "accountId": 3,
        "tokenId": 67890,
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": "1",
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    bool result = msg.FromSyncFrontOrBackUserIdPayLoad(jsonObject);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, FromDelUserPayLoad_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    cJSON payloadJson;
    bool result = msg.FromDelUserPayLoad(&payloadJson);
    ASSERT_EQ(result, false);
    result = msg.FromDelUserPayLoad(nullptr);
    ASSERT_EQ(result, false);
    const char* jsonString = R"({
        "MsgType": 0,
        "userId": "12345",
        "accountId": "a******3",
        "tokenId": 67890
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    result = msg.FromDelUserPayLoad(jsonObject);
    ASSERT_EQ(result, true);
}

HWTEST_F(ReleationShipSyncMgrTest, ToString_ValidData_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = 12345;
    msg.accountId = "account_123";
    msg.tokenId = 67890;
    msg.peerUdids = {"udid1", "udid2"};
    msg.peerUdid = "peer_udid";
    msg.accountName = "test_account";
    msg.syncUserIdFlag = true;

    std::string expected = "{ MsgType: " + std::to_string(static_cast<uint32_t>(msg.type)) +
                           ", userId: 12345, accountId: a******3, tokenId: 67890, " +
                           "peerUdids: [ u******1, u******2 ], peerUdid: p******d, " +
                           "accountName: t******t, syncUserIdFlag: 1, userIds: [  ] }";
    EXPECT_EQ(msg.ToString(), expected);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_001, testing::ext::TestSize.Level1)
{
    std::string msg = "";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_002, testing::ext::TestSize.Level1)
{
    std::string msg = R"(
    {
        "xx": "xx"
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_003, testing::ext::TestSize.Level1)
{
    std::string msg = R"(
    {
        "TYPE": "xx"
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_004, testing::ext::TestSize.Level1)
{
    std::string msg = R"(
    {
        "TYPE": 0
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    msg = R"(
    {
        "TYPE": 1
    }
    )";
    relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    msg = R"(
    {
        "TYPE": 2
    }
    )";
    relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    msg = R"(
    {
        "TYPE": 3
    }
    )";
    relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_005, testing::ext::TestSize.Level1)
{
    std::string msg = R"(
    {
        "TYPE": 0,
        "VALUE": ""
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ToSyncFrontOrBackUserIdPayLoad_ValidInput, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.userIdInfos.push_back({12345, true});
    msg.userIdInfos.push_back({67890, false});
    
    uint8_t* msgPtr = nullptr;
    uint32_t len = 0;

    bool result = msg.ToSyncFrontOrBackUserIdPayLoad(msgPtr, len);

    ASSERT_TRUE(result);
    ASSERT_EQ(len, 5);
    ASSERT_EQ(msgPtr[0], 2);

    delete[] msgPtr;
    msg.userIdInfos.clear();
}

HWTEST_F(ReleationShipSyncMgrTest, ToDelUserPayLoad_ValidInput, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.userId = 12345;

    uint8_t* msgPtr = nullptr;
    uint32_t len = 0;

    msg.ToDelUserPayLoad(msgPtr, len);

    ASSERT_EQ(len, 2);
    ASSERT_EQ(msgPtr[0], 0x39);

    delete[] msgPtr;
    msg.userIdInfos.clear();
}

HWTEST_F(ReleationShipSyncMgrTest, ToSyncFrontOrBackUserIdPayLoad_TooManyUserIds, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    for (int i = 0; i < MAX_USER_ID_NUM + 1; ++i) {
        msg.userIdInfos.push_back({i, true});
    }

    uint8_t* msgPtr = nullptr;
    uint32_t len = 0;

    bool result = msg.ToSyncFrontOrBackUserIdPayLoad(msgPtr, len);

    ASSERT_FALSE(result);
    ASSERT_EQ(msgPtr, nullptr);
    msg.userIdInfos.clear();
}

HWTEST_F(ReleationShipSyncMgrTest, ToSyncFrontOrBackUserIdPayLoad_LengthExceedsLimit, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.userIdInfos.push_back({12345, true});
    msg.userIdInfos.push_back({67890, false});
    uint8_t* msgPtr = nullptr;
    uint32_t len = 0;
    bool result = msg.ToSyncFrontOrBackUserIdPayLoad(msgPtr, len);
    ASSERT_TRUE(result);
}

HWTEST_F(ReleationShipSyncMgrTest, ToString_ValidData, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = 12345;
    msg.accountId = "account_123";
    msg.tokenId = 67890;
    msg.peerUdids = {"udid1", "udid2"};
    msg.peerUdid = "peer_udid";
    msg.accountName = "test_account";
    msg.syncUserIdFlag = true;
    msg.userIdInfos = {{true, 111}, {false, 222}};

    std::string expected = "{ MsgType: " + std::to_string(static_cast<uint32_t>(msg.type)) +
                           ", userId: 12345, accountId: a******3, tokenId: 67890, " +
                           "peerUdids: [ u******1, u******2 ], peerUdid: p******d, " +
                           "accountName: t******t, syncUserIdFlag: 1, " +
                           "userIds: [ { 1, userId: 111 }, { 0, userId: 222 } ] }";
                           
    EXPECT_EQ(msg.ToString(), expected);
}

HWTEST_F(ReleationShipSyncMgrTest, ToString_ValidData02, testing::ext::TestSize.Level1)
{
    bool isForeground = true;
    std::uint16_t userId = 123;
    UserIdInfo userIdInfo(isForeground, userId);
    std::string expected = "{ 1, userId: 123 }";
    EXPECT_EQ(userIdInfo.ToString(), expected);
}

HWTEST_F(ReleationShipSyncMgrTest, ToString_ZeroUserId, testing::ext::TestSize.Level1)
{
    bool isForeground = false;
    std::uint16_t userId = 0;
    UserIdInfo userIdInfo(isForeground, userId);

    std::string expected = "{ 0, userId: 0 }";
    EXPECT_EQ(userIdInfo.ToString(), expected);
}

HWTEST_F(ReleationShipSyncMgrTest, ValidList, testing::ext::TestSize.Level1)
{
    std::vector<UserIdInfo> list = {{true, 1}, {false, 2}};
    std::string expected = "[ { 1, userId: 1 }, { 0, userId: 2 } ]";
    
    EXPECT_EQ(GetUserIdInfoList(list), expected);
}

HWTEST_F(ReleationShipSyncMgrTest, EmptyList, testing::ext::TestSize.Level1)
{
    std::vector<UserIdInfo> list;
    std::string expected = "[  ]";
    
    EXPECT_EQ(GetUserIdInfoList(list), expected);
}

HWTEST_F(ReleationShipSyncMgrTest, ValidData, testing::ext::TestSize.Level1)
{
    std::vector<UserIdInfo> remoteUserIdInfos = {{true, 1}, {false, 2}, {true, 3}};
    std::vector<UserIdInfo> foregroundUserIdInfos;
    std::vector<UserIdInfo> backgroundUserIdInfos;

    GetFrontAndBackUserIdInfos(remoteUserIdInfos, foregroundUserIdInfos, backgroundUserIdInfos);

    EXPECT_EQ(foregroundUserIdInfos.size(), 2);
    EXPECT_EQ(backgroundUserIdInfos.size(), 1);
    EXPECT_EQ(foregroundUserIdInfos[0].userId, 1);
    EXPECT_EQ(foregroundUserIdInfos[1].userId, 3);
    EXPECT_EQ(backgroundUserIdInfos[0].userId, 2);
}

HWTEST_F(ReleationShipSyncMgrTest, EmptyRemoteList, testing::ext::TestSize.Level1)
{
    std::vector<UserIdInfo> remoteUserIdInfos;
    std::vector<UserIdInfo> foregroundUserIdInfos;
    std::vector<UserIdInfo> backgroundUserIdInfos;

    GetFrontAndBackUserIdInfos(remoteUserIdInfos, foregroundUserIdInfos, backgroundUserIdInfos);

    EXPECT_TRUE(foregroundUserIdInfos.empty());
    EXPECT_TRUE(backgroundUserIdInfos.empty());
}

HWTEST_F(ReleationShipSyncMgrTest, FromBroadcastPayLoad_010, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    cJSON *payloadJson = NULL;
    RelationShipChangeType type = RelationShipChangeType::TYPE_MAX;
    bool ret = msg.FromBroadcastPayLoad(payloadJson, type);
    EXPECT_FALSE(ret);

    type = RelationShipChangeType::ACCOUNT_LOGOUT;
    ret = msg.FromBroadcastPayLoad(payloadJson, type);
    EXPECT_FALSE(ret);

    type = RelationShipChangeType::DEVICE_UNBIND;
    ret = msg.FromBroadcastPayLoad(payloadJson, type);
    EXPECT_FALSE(ret);

    type = RelationShipChangeType::APP_UNBIND;
    ret = msg.FromBroadcastPayLoad(payloadJson, type);
    EXPECT_FALSE(ret);

    type = RelationShipChangeType::SYNC_USERID;
    ret = msg.FromBroadcastPayLoad(payloadJson, type);
    EXPECT_FALSE(ret);

    type = RelationShipChangeType::DEL_USER;
    ret = msg.FromBroadcastPayLoad(payloadJson, type);
    EXPECT_FALSE(ret);

    type = RelationShipChangeType::APP_UNINSTALL;
    ret = msg.FromBroadcastPayLoad(payloadJson, type);
    EXPECT_FALSE(ret);
}

HWTEST_F(ReleationShipSyncMgrTest, IsChangeTypeValid_001, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    bool ret = msg.IsChangeTypeValid();
    EXPECT_TRUE(ret);

    msg.type = RelationShipChangeType::DEVICE_UNBIND;
    ret = msg.IsChangeTypeValid();
    EXPECT_TRUE(ret);

    msg.type = RelationShipChangeType::APP_UNBIND;
    ret = msg.IsChangeTypeValid();
    EXPECT_TRUE(ret);

    msg.type = RelationShipChangeType::SYNC_USERID;
    ret = msg.IsChangeTypeValid();
    EXPECT_TRUE(ret);

    msg.type = RelationShipChangeType::DEL_USER;
    ret = msg.IsChangeTypeValid();
    EXPECT_TRUE(ret);
}

HWTEST_F(ReleationShipSyncMgrTest, IsValid_010, testing::ext::TestSize.Level1)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SYNC_USERID;
    msg.userIdInfos.push_back({1, false});
    msg.userIdInfos.push_back({2, true});
    bool ret = msg.IsValid();
    EXPECT_TRUE(ret);

    msg.type = static_cast<RelationShipChangeType>(9);
    ret = msg.IsValid();
    EXPECT_FALSE(ret);
}
}
} // namespace DistributedHardware
} // namespace OHOS
