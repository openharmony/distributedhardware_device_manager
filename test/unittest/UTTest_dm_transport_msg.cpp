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

#include "UTTest_dm_transport_msg.h"

namespace OHOS {
namespace DistributedHardware {
const char* const DSOFTBUS_NOTIFY_USERIDS_UDIDKEY = "remoteUdid";
const char* const DSOFTBUS_NOTIFY_USERIDS_USERIDKEY = "foregroundUserIds";
void DMTransportMsgTest::SetUp()
{
}
void DMTransportMsgTest::TearDown()
{
}
void DMTransportMsgTest::SetUpTestCase()
{
}
void DMTransportMsgTest::TearDownTestCase()
{
}

/**
 * @tc.name: ToJsonAndFromJson
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson, testing::ext::TestSize.Level1)
{
    UserIdsMsg userIdsMsg;
    userIdsMsg.foregroundUserIds = {1, 2, 3};

    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, userIdsMsg);
    
    UserIdsMsg newUserIdsMsg;
    FromJson(jsonObject, newUserIdsMsg);

    EXPECT_EQ(newUserIdsMsg.foregroundUserIds.size(), 3);
    EXPECT_EQ(newUserIdsMsg.foregroundUserIds[0], 1);
    EXPECT_EQ(newUserIdsMsg.foregroundUserIds[1], 2);
    EXPECT_EQ(newUserIdsMsg.foregroundUserIds[2], 3);

    cJSON_Delete(jsonObject);

    cJSON *emptyJsonObject = cJSON_CreateObject();
    UserIdsMsg emptyUserIdsMsg;
    FromJson(emptyJsonObject, emptyUserIdsMsg);
    
    EXPECT_EQ(emptyUserIdsMsg.foregroundUserIds.size(), 0);
    cJSON_Delete(emptyJsonObject);
}

/**
 * @tc.name: ToJsonAndFromJson_Invaild
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson_Invaild, testing::ext::TestSize.Level1)
{
    UserIdsMsg userIdsMsg;
    userIdsMsg.foregroundUserIds = {1, 2, 3};

    cJSON *jsonObject = nullptr;
    ToJson(jsonObject, userIdsMsg);
    
    UserIdsMsg newUserIdsMsg;
    FromJson(jsonObject, newUserIdsMsg);
    EXPECT_EQ(jsonObject, nullptr);
    EXPECT_TRUE(newUserIdsMsg.foregroundUserIds.empty());
}

/**
 * @tc.name: ToJsonAndFromJson01
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson01, testing::ext::TestSize.Level1)
{
    CommMsg commMsg;
    commMsg.code = 200;
    commMsg.msg = "Success";

    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, commMsg);

    CommMsg newCommMsg;
    FromJson(jsonObject, newCommMsg);

    EXPECT_EQ(newCommMsg.code, 200);
    EXPECT_EQ(newCommMsg.msg, "Success");

    cJSON_Delete(jsonObject);

    cJSON *emptyCommJsonObject = cJSON_CreateObject();
    CommMsg emptyCommMsg;
    FromJson(emptyCommJsonObject, emptyCommMsg);

    EXPECT_EQ(emptyCommMsg.code, -1);
    EXPECT_EQ(emptyCommMsg.msg, "");
    cJSON_Delete(emptyCommJsonObject);
}

/**
 * @tc.name: ToJsonAndFromJson01_Invaild
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson01_Invaild, testing::ext::TestSize.Level1)
{
    CommMsg commMsg;
    commMsg.code = 200;
    commMsg.msg = "Success";

    cJSON *jsonObject = nullptr;
    ToJson(jsonObject, commMsg);

    CommMsg newCommMsg;
    FromJson(jsonObject, newCommMsg);
    
    EXPECT_EQ(jsonObject, nullptr);
    EXPECT_TRUE(newCommMsg.msg.empty());
}

/**
 * @tc.name: ToJsonAndFromJson02
 * @tc.type: FUNC
 */

HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson02, testing::ext::TestSize.Level1)
{
    std::string remoteUdid = "test_udid";
    std::vector<uint32_t> userIds = {10, 20, 30};
    NotifyUserIds notifyUserIds(remoteUdid, userIds);
    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, notifyUserIds);

    NotifyUserIds newNotifyUserIds;
    FromJson(jsonObject, newNotifyUserIds);
    EXPECT_EQ(newNotifyUserIds.remoteUdid, remoteUdid);
    EXPECT_EQ(newNotifyUserIds.userIds.size(), 3);
    EXPECT_EQ(newNotifyUserIds.userIds[0], 10);
    EXPECT_EQ(newNotifyUserIds.userIds[1], 20);
    EXPECT_EQ(newNotifyUserIds.userIds[2], 30);
    cJSON_Delete(jsonObject);

    cJSON *emptyCommJsonObject = cJSON_CreateObject();
    NotifyUserIds emptyNotifyUserIds;
    FromJson(emptyCommJsonObject, emptyNotifyUserIds);
    EXPECT_EQ(emptyNotifyUserIds.userIds.size(), 0);
    EXPECT_EQ(emptyNotifyUserIds.remoteUdid, "");
    cJSON_Delete(emptyCommJsonObject);
}

/**
 * @tc.name: ToJsonAndFromJson02_Invaild
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson02_Invaild, testing::ext::TestSize.Level1)
{
    std::string remoteUdid = "test_udid";
    std::vector<uint32_t> userIds = {10, 20, 30};
    NotifyUserIds notifyUserIds(remoteUdid, userIds);
    cJSON *jsonObject = nullptr;
    ToJson(jsonObject, notifyUserIds);
    NotifyUserIds newNotifyUserIds;
    FromJson(jsonObject, newNotifyUserIds);
    EXPECT_EQ(jsonObject, nullptr);
    EXPECT_EQ(newNotifyUserIds.remoteUdid, "");
    EXPECT_EQ(newNotifyUserIds.userIds.size(), 0);
}

/**
 * @tc.name: ToJsonAndFromJson03
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson03, testing::ext::TestSize.Level1)
{
    LogoutAccountMsg logoutAccountMsg;
    logoutAccountMsg.accountId = "test_account_id";
    logoutAccountMsg.userId = 123;
    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, logoutAccountMsg);
    LogoutAccountMsg newLogoutAccountMsg;
    FromJson(jsonObject, newLogoutAccountMsg);
    EXPECT_EQ(newLogoutAccountMsg.accountId, "test_account_id");
    EXPECT_EQ(newLogoutAccountMsg.userId, 123);
    cJSON_Delete(jsonObject);

    cJSON *emptyJsonObject = cJSON_CreateObject();
    LogoutAccountMsg emptyLogoutAccountMsg;
    FromJson(emptyJsonObject, emptyLogoutAccountMsg);
    EXPECT_EQ(emptyLogoutAccountMsg.userId, -1);
}

/**
 * @tc.name: ToJsonAndFromJson03_Invaild
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJsonAndFromJson03_Invaild, testing::ext::TestSize.Level1)
{
    LogoutAccountMsg logoutAccountMsg;
    logoutAccountMsg.accountId = "test_account_id";
    logoutAccountMsg.userId = 123;
    cJSON *jsonObject = nullptr;
    ToJson(jsonObject, logoutAccountMsg);
    LogoutAccountMsg newLogoutAccountMsg;
    FromJson(jsonObject, newLogoutAccountMsg);
    EXPECT_EQ(jsonObject, nullptr);
    EXPECT_EQ(newLogoutAccountMsg.userId, -1);
}

/**
 * @tc.name: PerformanceTest
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, PerformanceTest, testing::ext::TestSize.Level1)
{
    UserIdsMsg userIdsMsg;
    for (int i = 0; i < 10000; ++i) {
        userIdsMsg.foregroundUserIds.push_back(i);
    }

    cJSON *jsonObject = cJSON_CreateObject();
    auto start = std::chrono::high_resolution_clock::now();
    ToJson(jsonObject, userIdsMsg);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    EXPECT_LT(elapsed.count(), 1.0);
    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: GetCommMsgString_EmptyInput
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, GetCommMsgString_EmptyInput, testing::ext::TestSize.Level1)
{
    CommMsg commMsg;
    std::string result = GetCommMsgString(commMsg);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: ToJson_ValidInput
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToJson_ValidInput, testing::ext::TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    NotifyUserIds notifyUserIds;
    notifyUserIds.remoteUdid = "test_udid";
    notifyUserIds.userIds = {1, 2, 3};

    ToJson(jsonObject, notifyUserIds);
    cJSON *udidItem = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_USERIDS_UDIDKEY);
    EXPECT_STREQ(udidItem->valuestring, "test_udid");
    cJSON *userIdsArr = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_USERIDS_USERIDKEY);
    EXPECT_EQ(cJSON_GetArraySize(userIdsArr), 3);
}

/**
 * @tc.name: FromJson_InvalidJson
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, FromJson_InvalidJson, testing::ext::TestSize.Level1)
{
    const char *jsonString = "{\"udid\":123,\"userIds\":\"invalid\"}";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    NotifyUserIds notifyUserIds;

    FromJson(jsonObject, notifyUserIds);
    EXPECT_TRUE(notifyUserIds.remoteUdid.empty());
    EXPECT_TRUE(notifyUserIds.userIds.empty());

    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: NotifyUserIds_ToString_ValidInput
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, NotifyUserIds_ToString_ValidInput, testing::ext::TestSize.Level1)
{
    NotifyUserIds notifyUserIds;
    notifyUserIds.remoteUdid = "test_udid";
    notifyUserIds.userIds = {1, 2, 3};

    std::string result = notifyUserIds.ToString();
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: NotifyUserIds_ToString_EmptyInput
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, NotifyUserIds_ToString_EmptyInput, testing::ext::TestSize.Level1)
{
    NotifyUserIds notifyUserIds;
    std::string result = notifyUserIds.ToString();
    EXPECT_FALSE(result.empty());
}

HWTEST_F(DMTransportMsgTest, ToJson_UserIdsMsg, testing::ext::TestSize.Level1)
{
    std::vector<uint32_t> foregroundUserIds{1, 2, 3};
    std::vector<uint32_t> backgroundUserIds{4, 5, 6};
    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds, true);
    const char* jsonStr = R"({
        "MsgType": "0",
        "msg": "messgaeinfo",
        "code": 145,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonStr);
    ToJson(jsonObject, userIdsMsg);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(userIdsMsg.foregroundUserIds.empty());
}

/**
 * @tc.name: GetCommMsgString_01
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, GetCommMsgString_01, testing::ext::TestSize.Level1)
{
    const char* jsonstr = R"({"code":123,"msg":"messageinfo"})";
    std::string jsonObj(jsonstr);
    CommMsg commMsg(123, "messageinfo");
    auto CommMsgString = GetCommMsgString(commMsg);
    EXPECT_EQ(CommMsgString, jsonObj);
}

/**
 * @tc.name: ToString_01
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, ToString_01, testing::ext::TestSize.Level1)
{
    const char* jsonstr = R"({"remoteUdid":"test_udid","foregroundUserIds":[10,20,30]})";
    std::string jsonObj(jsonstr);
    std::string remoteUdid = "test_udid";
    std::vector<uint32_t> userIds = {10, 20, 30};
    NotifyUserIds notifyUserIds(remoteUdid, userIds);
    auto notifyUserIdsString = notifyUserIds.ToString();
    EXPECT_EQ(notifyUserIdsString, jsonObj);
}

HWTEST_F(DMTransportMsgTest, UninstAppToJsonAndFromJson, testing::ext::TestSize.Level1)
{
    UninstAppMsg uninstAppMsg(2, 3);

    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, uninstAppMsg);
    
    UninstAppMsg newUninstAppMsg;
    FromJson(jsonObject, newUninstAppMsg);

    EXPECT_EQ(newUninstAppMsg.userId_, 2);
    EXPECT_EQ(newUninstAppMsg.tokenId_, 3);

    cJSON_Delete(jsonObject);

    cJSON *nullJsonObject = nullptr;
    ToJson(nullJsonObject, uninstAppMsg);

    UninstAppMsg emptyMsg;
    FromJson(nullJsonObject, emptyMsg);
    
    EXPECT_EQ(emptyMsg.userId_, -1);
    cJSON *emptyObject = cJSON_CreateObject();
    FromJson(emptyObject, emptyMsg);
    EXPECT_EQ(emptyMsg.userId_, -1);

    cJSON_Delete(nullJsonObject);
    cJSON_Delete(emptyObject);
}

HWTEST_F(DMTransportMsgTest, UnbindAppToJsonAndFromJson, testing::ext::TestSize.Level1)
{
    UnBindAppMsg unBindAppMsg(2, 3, "", "test_udid");

    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, unBindAppMsg);

    UnBindAppMsg newUnBindAppMsg;
    FromJson(jsonObject, newUnBindAppMsg);

    EXPECT_EQ(newUnBindAppMsg.userId_, 2);
    EXPECT_EQ(newUnBindAppMsg.tokenId_, 3);
    EXPECT_EQ(newUnBindAppMsg.extra_, "");
    EXPECT_EQ(newUnBindAppMsg.udid_, "test_udid");

    cJSON_Delete(jsonObject);

    cJSON *nullJsonObject = nullptr;
    ToJson(nullJsonObject, unBindAppMsg);

    UnBindAppMsg emptyMsg;
    FromJson(nullJsonObject, emptyMsg);

    EXPECT_EQ(emptyMsg.userId_, -1);
    cJSON *emptyObject = cJSON_CreateObject();
    FromJson(emptyObject, emptyMsg);
    EXPECT_EQ(emptyMsg.userId_, -1);

    cJSON_Delete(nullJsonObject);
    cJSON_Delete(emptyObject);
}

/**
 * @tc.name: UnbindServiceProxyToJsonAndFromJson
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, UnbindServiceProxyToJsonAndFromJson, testing::ext::TestSize.Level1)
{
    UnbindServiceProxyParam param;
    param.userId = 1;
    param.localTokenId = 100;
    param.subjectTokenId = 200;
    param.serviceId = 300;
    param.localUdid = "local_udid";
    param.isProxyUnbind = true;

    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, param);

    UnbindServiceProxyParam newParam;
    FromJson(jsonObject, newParam);

    EXPECT_EQ(newParam.userId, 1);
    EXPECT_EQ(newParam.localTokenId, 100);
    EXPECT_EQ(newParam.subjectTokenId, 200);
    EXPECT_EQ(newParam.serviceId, 300);
    EXPECT_EQ(newParam.localUdid, "local_udid");
    EXPECT_TRUE(newParam.isProxyUnbind);

    cJSON_Delete(jsonObject);

    cJSON *nullJsonObject = nullptr;
    ToJson(nullJsonObject, param);

    UnbindServiceProxyParam emptyParam;
    FromJson(nullJsonObject, emptyParam);
    EXPECT_EQ(emptyParam.userId, -1);

    cJSON *emptyObject = cJSON_CreateObject();
    FromJson(emptyObject, emptyParam);
    EXPECT_EQ(emptyParam.userId, -1);
    EXPECT_FALSE(emptyParam.isProxyUnbind);

    cJSON_Delete(nullJsonObject);
    cJSON_Delete(emptyObject);
}

/**
 * @tc.name: FromJson_UserIdsMsg_TooManyForeground
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, FromJson_UserIdsMsg_TooManyForeground, testing::ext::TestSize.Level1)
{
    const char *jsonStr = R"({"foregroundUserIds":[1,2,3,4,5,6],"backgroundUserIds":[7],"isNewEvent":true})";
    cJSON *jsonObject = cJSON_Parse(jsonStr);
    ASSERT_NE(jsonObject, nullptr);

    UserIdsMsg userIdsMsg;
    FromJson(jsonObject, userIdsMsg);
    EXPECT_TRUE(userIdsMsg.foregroundUserIds.empty());

    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: FromJson_UserIdsMsg_TooManyBackground
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, FromJson_UserIdsMsg_TooManyBackground, testing::ext::TestSize.Level1)
{
    const char *jsonStr =
        R"({"foregroundUserIds":[1,2],"backgroundUserIds":[10,11,12,13,14,15],"isNewEvent":true})";
    cJSON *jsonObject = cJSON_Parse(jsonStr);
    ASSERT_NE(jsonObject, nullptr);

    UserIdsMsg userIdsMsg;
    FromJson(jsonObject, userIdsMsg);
    EXPECT_TRUE(userIdsMsg.backgroundUserIds.empty());

    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: FromJson_CommMsg_NonNumberCode
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, FromJson_CommMsg_NonNumberCode, testing::ext::TestSize.Level1)
{
    const char *jsonStr = R"({"code":"abc","msg":123})";
    cJSON *jsonObject = cJSON_Parse(jsonStr);
    ASSERT_NE(jsonObject, nullptr);

    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    EXPECT_EQ(commMsg.code, -1);
    EXPECT_EQ(commMsg.msg, "");

    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: FromJson_NotifyUserIds_TooManyUserIds
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportMsgTest, FromJson_NotifyUserIds_TooManyUserIds, testing::ext::TestSize.Level1)
{
    const char *jsonStr = R"({"remoteUdid":"udid","foregroundUserIds":[1,2,3,4,5,6]})";
    cJSON *jsonObject = cJSON_Parse(jsonStr);
    ASSERT_NE(jsonObject, nullptr);

    NotifyUserIds notifyUserIds;
    FromJson(jsonObject, notifyUserIds);
    EXPECT_TRUE(notifyUserIds.userIds.empty());

    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: CommMsg_DefaultConstructor
 * @tc.desc: Verify CommMsg default constructor initializes code to -1 and msg to empty.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, CommMsg_DefaultConstructor, testing::ext::TestSize.Level1)
{
    CommMsg commMsg;
    EXPECT_EQ(commMsg.code, -1);
    EXPECT_TRUE(commMsg.msg.empty());
}

/**
 * @tc.name: UserIdsMsg_DefaultConstructor
 * @tc.desc: Verify UserIdsMsg default constructor initializes fields correctly.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, UserIdsMsg_DefaultConstructor, testing::ext::TestSize.Level1)
{
    UserIdsMsg userIdsMsg;
    EXPECT_TRUE(userIdsMsg.foregroundUserIds.empty());
    EXPECT_TRUE(userIdsMsg.backgroundUserIds.empty());
    EXPECT_FALSE(userIdsMsg.isNewEvent);
}

/**
 * @tc.name: UserIdsMsg_ParameterConstructor
 * @tc.desc: Verify UserIdsMsg parameter constructor sets fields correctly.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, UserIdsMsg_ParameterConstructor, testing::ext::TestSize.Level1)
{
    std::vector<uint32_t> foregroundUserIds{1, 2};
    std::vector<uint32_t> backgroundUserIds{3, 4};
    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds, true);
    EXPECT_EQ(userIdsMsg.foregroundUserIds.size(), 2);
    EXPECT_EQ(userIdsMsg.backgroundUserIds.size(), 2);
    EXPECT_TRUE(userIdsMsg.isNewEvent);
}

/**
 * @tc.name: LogoutAccountMsg_DefaultConstructor
 * @tc.desc: Verify LogoutAccountMsg default constructor initializes userId to -1.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, LogoutAccountMsg_DefaultConstructor, testing::ext::TestSize.Level1)
{
    LogoutAccountMsg logoutAccountMsg;
    EXPECT_EQ(logoutAccountMsg.userId, -1);
    EXPECT_TRUE(logoutAccountMsg.accountId.empty());
}

/**
 * @tc.name: LogoutAccountMsg_ParameterConstructor
 * @tc.desc: Verify LogoutAccountMsg parameter constructor sets fields.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, LogoutAccountMsg_ParameterConstructor, testing::ext::TestSize.Level1)
{
    LogoutAccountMsg logoutAccountMsg("acct_001", 42);
    EXPECT_EQ(logoutAccountMsg.accountId, "acct_001");
    EXPECT_EQ(logoutAccountMsg.userId, 42);
}

/**
 * @tc.name: GetCommMsgString_002
 * @tc.desc: Verify GetCommMsgString returns empty for default (empty msg) CommMsg is non-empty json.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, GetCommMsgString_002, testing::ext::TestSize.Level1)
{
    CommMsg commMsg(0, "");
    std::string result = GetCommMsgString(commMsg);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("\"code\":0"), std::string::npos);
}

/**
 * @tc.name: NotifyUserIds_EmptyUserIds_ToString
 * @tc.desc: Verify NotifyUserIds ToString with empty userIds vector produces valid json.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, NotifyUserIds_EmptyUserIds_ToString, testing::ext::TestSize.Level1)
{
    NotifyUserIds notifyUserIds("udid_only", {});
    std::string result = notifyUserIds.ToString();
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("\"remoteUdid\":\"udid_only\""), std::string::npos);
}

/**
 * @tc.name: UninstAppMsg_DefaultConstructor
 * @tc.desc: Verify UninstAppMsg default constructor initializes fields to -1.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, UninstAppMsg_DefaultConstructor, testing::ext::TestSize.Level1)
{
    UninstAppMsg uninstAppMsg;
    EXPECT_EQ(uninstAppMsg.userId_, -1);
    EXPECT_EQ(uninstAppMsg.tokenId_, -1);
}

/**
 * @tc.name: UnBindAppMsg_DefaultConstructor
 * @tc.desc: Verify UnBindAppMsg default constructor initializes fields correctly.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, UnBindAppMsg_DefaultConstructor, testing::ext::TestSize.Level1)
{
    UnBindAppMsg unBindAppMsg;
    EXPECT_EQ(unBindAppMsg.userId_, -1);
    EXPECT_EQ(unBindAppMsg.tokenId_, -1);
    EXPECT_TRUE(unBindAppMsg.extra_.empty());
    EXPECT_TRUE(unBindAppMsg.udid_.empty());
}

/**
 * @tc.name: UnbindServiceProxyParam_DefaultIsProxyUnbind
 * @tc.desc: Verify UnbindServiceProxyParam default isProxyUnbind is false after FromJson on empty object.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportMsgTest, UnbindServiceProxyParam_DefaultIsProxyUnbind, testing::ext::TestSize.Level1)
{
    UnbindServiceProxyParam param;
    param.userId = 5;
    param.localTokenId = 50;
    param.subjectTokenId = 500;
    param.serviceId = 5000;
    param.localUdid = "udid_5";
    param.isProxyUnbind = false;

    cJSON *jsonObject = cJSON_CreateObject();
    ToJson(jsonObject, param);

    UnbindServiceProxyParam newParam;
    FromJson(jsonObject, newParam);
    EXPECT_EQ(newParam.userId, 5);
    EXPECT_EQ(newParam.localTokenId, 50);
    EXPECT_EQ(newParam.subjectTokenId, 500);
    EXPECT_EQ(newParam.serviceId, 5000);
    EXPECT_EQ(newParam.localUdid, "udid_5");
    EXPECT_FALSE(newParam.isProxyUnbind);

    cJSON_Delete(jsonObject);
}
} // DistributedHardware
} // OHOS