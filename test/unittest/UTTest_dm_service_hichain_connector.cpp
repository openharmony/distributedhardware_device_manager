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
#include "UTTest_dm_service_hichain_connector.h"
#include "dm_service_hichain_connector.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "multiple_user_connector.h"
#include "multiple_user_connector_mock.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

class TestDmServiceGroupResCallback : public IDmServiceGroupResCallback {
public:
    virtual ~TestDmServiceGroupResCallback() = default;
    void OnGroupResult(int64_t requestId, int32_t action, const std::string &resultInfo) override {}
};

void DmServiceHiChainConnectorTest::SetUp()
{
}

void DmServiceHiChainConnectorTest::TearDown()
{
}

void DmServiceHiChainConnectorTest::SetUpTestCase()
{
}

void DmServiceHiChainConnectorTest::TearDownTestCase()
{
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_001
 * @tc.desc: Constructor + Destructor (stack object)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    (void)connector;
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_002
 * @tc.desc: Constructor + Destructor (heap object new/delete)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_002, testing::ext::TestSize.Level1)
{
    auto *connector = new DmServiceHiChainConnector();
    delete connector;
    connector = nullptr;
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_003
 * @tc.desc: Constructor + Destructor (multiple times)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_003, testing::ext::TestSize.Level1)
{
    for (int i = 0; i < 5; ++i) {
        DmServiceHiChainConnector connector;
        (void)connector;
    }
}

/**
 * @tc.name: GetJsonStr_001
 * @tc.desc: key not exist(or invalid) -> return empty string
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonStr_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    std::string ret = connector.GetJsonStr(jsonObj, "not_exist_key");
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetJsonStr_002
 * @tc.desc: key exist and type is string -> return value
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonStr_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["userId"] = "abc123";
    std::string ret = connector.GetJsonStr(jsonObj, "userId");
    EXPECT_EQ(ret, "abc123");
}

/**
 * @tc.name: GetJsonStr_Negative_001
 * @tc.desc: GetJsonStr with key exists but value is not string
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonStr_Negative_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["userId"] = 12345;
    std::string ret = connector.GetJsonStr(jsonObj, "userId");
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetJsonInt_001
 * @tc.desc: key not exist(or invalid) -> return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonInt_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    int32_t ret = connector.GetJsonInt(jsonObj, "not_exist_key");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetJsonInt_002
 * @tc.desc: key exist and type is int -> return value
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonInt_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    int32_t ret = connector.GetJsonInt(jsonObj, "authType");
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: GetJsonInt_Negative_001
 * @tc.desc: GetJsonInt with key exists but value is not int
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonInt_Negative_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = "not_an_int";
    int32_t ret = connector.GetJsonInt(jsonObj, "authType");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupInfo_001
 * @tc.desc: GetGroupInfo-> GetGroupInfoCommon
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    (void)ret;
}

/**
 * @tc.name: GetGroupInfo_002
 * @tc.desc: GetGroupInfo with current account userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    (void)ret;
}

/**
 * @tc.name: GetGroupInfo_003
 * @tc.desc: GetGroupInfo with specific groupType query
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    JsonObject queryParams;
    queryParams["groupType"] = 1;
    std::string queryStr = queryParams.Dump();
    bool ret = connector.GetGroupInfo(0, queryStr, groupList);
    (void)ret;
}

/**
 * @tc.name: GetGroupInfoCommon_001
 * @tc.desc: GetGroupInfoCommon with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice", groupList);
    (void)ret;
}

/**
 * @tc.name: GetGroupInfoCommon_002
 * @tc.desc: GetGroupInfoCommon with empty query params
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice", groupList);
    (void)ret;
}

/**
 * @tc.name: GetGroupInfoCommon_003
 * @tc.desc: GetGroupInfoCommon with different pkgName
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice", groupList);
    (void)ret;
}

/**
 * @tc.name: RegisterHiChainGroupCallback_001
 * @tc.desc: RegisterHiChainGroupCallback with valid callback
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, RegisterHiChainGroupCallback_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    auto callback = std::make_shared<TestDmServiceGroupResCallback>();
    int32_t ret = connector.RegisterHiChainGroupCallback(callback);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterHiChainGroupCallback_002
 * @tc.desc: RegisterHiChainGroupCallback with nullptr callback
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, RegisterHiChainGroupCallback_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::shared_ptr<IDmServiceGroupResCallback> callback = nullptr;
    int32_t ret = connector.RegisterHiChainGroupCallback(callback);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterHiChainGroupCallback_001
 * @tc.desc: UnRegisterHiChainGroupCallback after register
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, UnRegisterHiChainGroupCallback_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    auto callback = std::make_shared<TestDmServiceGroupResCallback>();
    connector.RegisterHiChainGroupCallback(callback);
    int32_t ret = connector.UnRegisterHiChainGroupCallback();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterHiChainGroupCallback_002
 * @tc.desc: UnRegisterHiChainGroupCallback without register
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, UnRegisterHiChainGroupCallback_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int32_t ret = connector.UnRegisterHiChainGroupCallback();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnFinish_001
 * @tc.desc: operationCode = GROUP_CREATE + returnData == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    connector.onFinish(1, static_cast<int>(GroupOperationCode::GROUP_CREATE), nullptr);
}

/**
 * @tc.name: OnFinish_002
 * @tc.desc: operationCode = GROUP_CREATE + returnData != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *data = "create_ok";
    connector.onFinish(2, static_cast<int>(GroupOperationCode::GROUP_CREATE), data);
}

/**
 * @tc.name: OnFinish_003
 * @tc.desc: Test onFinish with GROUP_DISBAND and nullptr returnData
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    connector.onFinish(3, static_cast<int>(GroupOperationCode::GROUP_DISBAND), nullptr);
}

/**
 * @tc.name: OnFinish_004
 * @tc.desc: Cover the default path for operationCode not being create/disband
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    connector.onFinish(4, 999, nullptr);
    connector.onFinish(5, -1, "any");
}

/**
 * @tc.name: OnError_001
 * @tc.desc: operationCode = GROUP_CREATE, errorReturn == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    connector.onError(100, static_cast<int>(GroupOperationCode::GROUP_CREATE), -1, nullptr);
}

/**
 * @tc.name: OnError_002
 * @tc.desc: operationCode = GROUP_CREATE, errorReturn != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *err = "create_failed";
    connector.onError(101, static_cast<int>(GroupOperationCode::GROUP_CREATE), 123, err);
}

/**
 * @tc.name: OnError_003
 * @tc.desc: operationCode = GROUP_DISBAND, errorReturn == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    connector.onError(200, static_cast<int>(GroupOperationCode::GROUP_DISBAND), -2, nullptr);
}

/**
 * @tc.name: OnError_004
 * @tc.desc: operationCode = GROUP_DISBAND, errorReturn != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *err = "disband_failed";
    connector.onError(201, static_cast<int>(GroupOperationCode::GROUP_DISBAND), 456, err);
}

/**
 * @tc.name: OnError_005
 * @tc.desc: operationCode no create/disband
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    connector.onError(300, 999, -999, nullptr);
    connector.onError(301, -1, 0, "any");
}

/**
 * @tc.name: GetGroupId_001
 * @tc.desc: GetGroupId with empty groupList result
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "test_user_123";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    (void)ret;
}

/**
 * @tc.name: GetGroupId_002
 * @tc.desc: GetGroupId with existing group info
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "test_user_456";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    (void)ret;
}

/**
 * @tc.name: GetGroupId_003
 * @tc.desc: GetGroupId with different groupType
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "test_user_getid_003";
    int32_t groupType = 2;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    (void)ret;
}

/**
 * @tc.name: GetGroupIdExt_001
 * @tc.desc: GetGroupIdExt with empty userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupIdExt_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string groupOwner = "";
    std::string userId = "";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupIdExt(userId, groupType, groupId, groupOwner);
    (void)ret;
}

/**
 * @tc.name: GetGroupIdExt_002
 * @tc.desc: GetGroupIdExt with valid userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupIdExt_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string groupOwner = "";
    std::string userId = "test_user_ext_001";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupIdExt(userId, groupType, groupId, groupOwner);
    (void)ret;
}

/**
 * @tc.name: ParseRemoteCredential_001
 * @tc.desc: ParseRemoteCredential with empty userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredential_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "";
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    jsonDeviceList["deviceList"] = deviceList;
    std::string params = "";
    int32_t osAccountUserId = 0;
    int32_t groupType = 1;
    int32_t ret = connector.ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseRemoteCredential_002
 * @tc.desc: ParseRemoteCredential with valid userId but no deviceList
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredential_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_789";
    JsonObject jsonDeviceList;
    std::string params = "";
    int32_t osAccountUserId = 0;
    int32_t groupType = 1;
    int32_t ret = connector.ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseRemoteCredential_003
 * @tc.desc: ParseRemoteCredential with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredential_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_parse_001";
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    std::string params = "";
    int32_t osAccountUserId = 0;
    int32_t groupType = 1;
    int32_t ret = connector.ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    (void)ret;
}

/**
 * @tc.name: ParseRemoteCredential_004
 * @tc.desc: ParseRemoteCredential with userId and deviceList but group not found
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredential_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "non_existent_user_999";
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    std::string params = "";
    int32_t osAccountUserId = 0;
    int32_t groupType = 1;
    int32_t ret = connector.ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    (void)ret;
}

/**
 * @tc.name: ParseRemoteCredentialExt_001
 * @tc.desc: ParseRemoteCredentialExt with empty credentialInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string credentialInfo = "";
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    (void)ret;
}

/**
 * @tc.name: ParseRemoteCredentialExt_002
 * @tc.desc: ParseRemoteCredentialExt with invalid json string
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string credentialInfo = "invalid_json_string";
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    (void)ret;
}

/**
 * @tc.name: ParseRemoteCredentialExt_003
 * @tc.desc: ParseRemoteCredentialExt with valid credentialInfo (SAME_ACCOUNT)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user_same_account";
    jsonObj["credentialType"] = 0;
    jsonObj["operationCode"] = 0;
    jsonObj["TType"] = 0;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonObj["deviceList"] = deviceList;
    std::string credentialInfo = jsonObj.Dump();
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    (void)ret;
}

/**
 * @tc.name: ParseRemoteCredentialExt_004
 * @tc.desc: ParseRemoteCredentialExt with authType != SAME_ACCOUNT
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 2;
    jsonObj["userId"] = "test_user_diff_account";
    std::string credentialInfo = jsonObj.Dump();
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    (void)ret;
}

/**
 * @tc.name: addMultiMembers_001
 * @tc.desc: addMultiMembers with empty deviceList
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_add_001";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.addMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
}

/**
 * @tc.name: addMultiMembers_002
 * @tc.desc: addMultiMembers with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_add_002";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.addMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
}

/**
 * @tc.name: addMultiMembers_003
 * @tc.desc: addMultiMembers with multiple devices
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_add_multi_003";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    deviceList.push_back("device2");
    deviceList.push_back("device3");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.addMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
}

/**
 * @tc.name: addMultiMembersExt_001
 * @tc.desc: addMultiMembersExt with empty credentialInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string credentialInfo = "";
    int32_t ret = connector.addMultiMembersExt(credentialInfo);
    (void)ret;
}

/**
 * @tc.name: addMultiMembersExt_002
 * @tc.desc: addMultiMembersExt with valid credentialInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user_add_ext_001";
    jsonObj["credentialType"] = 0;
    jsonObj["operationCode"] = 0;
    jsonObj["TType"] = 0;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonObj["deviceList"] = deviceList;
    std::string credentialInfo = jsonObj.Dump();
    int32_t ret = connector.addMultiMembersExt(credentialInfo);
    (void)ret;
}

/**
 * @tc.name: deleteMultiMembers_001
 * @tc.desc: deleteMultiMembers with empty deviceList
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_del_001";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.deleteMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
}

/**
 * @tc.name: deleteMultiMembers_002
 * @tc.desc: deleteMultiMembers with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_del_002";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.deleteMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
}

/**
 * @tc.name: deleteMultiMembers_003
 * @tc.desc: deleteMultiMembers with multiple devices
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_del_multi_003";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    deviceList.push_back("device2");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.deleteMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
}

/**
 * @tc.name: DeleteGroup_001
 * @tc.desc: DeleteGroup(std::string) with empty groupId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    int32_t ret = connector.DeleteGroup(groupId);
    (void)ret;
}

/**
 * @tc.name: DeleteGroup_002
 * @tc.desc: DeleteGroup(std::string) with valid groupId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "test_group_id_001";
    int32_t ret = connector.DeleteGroup(groupId);
    (void)ret;
}

/**
 * @tc.name: DeleteGroup_003
 * @tc.desc: DeleteGroup(requestId, userId, authType) with empty userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 1001;
    std::string userId = "";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    (void)ret;
}

/**
 * @tc.name: DeleteGroup_004
 * @tc.desc: DeleteGroup(requestId, userId, authType) with valid userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 1002;
    std::string userId = "test_user_del_group_001";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    (void)ret;
}

/**
 * @tc.name: DeleteGroup_005
 * @tc.desc: DeleteGroup(requestId, userId, authType) with different authType
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 1005;
    std::string userId = "test_user_del_group_005";
    int32_t authType = 2;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    (void)ret;
}

/**
 * @tc.name: CreateGroup_001
 * @tc.desc: CreateGroup with invalid jsonOutObj
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 2001;
    int32_t authType = 1;
    std::string userId = "test_user_create_001";
    JsonObject jsonOutObj;
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    (void)ret;
}

/**
 * @tc.name: CreateGroup_002
 * @tc.desc: CreateGroup with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 2002;
    int32_t authType = 1;
    std::string userId = "test_user_create_002";
    JsonObject jsonOutObj;
    jsonOutObj["testKey"] = "testValue";
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    (void)ret;
}

/**
 * @tc.name: CreateGroup_003
 * @tc.desc: CreateGroup with different authType
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 2003;
    int32_t authType = 2;
    std::string userId = "test_user_create_003";
    JsonObject jsonOutObj;
    jsonOutObj["testKey"] = "testValue";
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    (void)ret;
}

/**
 * @tc.name: IsRedundanceGroup_001
 * @tc.desc: IsRedundanceGroup with invalid userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "";
    int32_t authType = 1;
    std::vector<DmGroupInfo> groupList;
    bool ret = connector.IsRedundanceGroup(userId, authType, groupList);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsRedundanceGroup_002
 * @tc.desc: IsRedundanceGroup with valid userId but no groups
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_redundance_001";
    int32_t authType = 1;
    std::vector<DmGroupInfo> groupList;
    bool ret = connector.IsRedundanceGroup(userId, authType, groupList);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsRedundanceGroup_003
 * @tc.desc: IsRedundanceGroup with valid userId and existing groups
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_redundance_002";
    int32_t authType = 1;
    std::vector<DmGroupInfo> groupList;
    bool ret = connector.IsRedundanceGroup(userId, authType, groupList);
    (void)ret;
}

/**
 * @tc.name: DeleteRedundanceGroup_001
 * @tc.desc: DeleteRedundanceGroup with empty userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteRedundanceGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "";
    connector.DeleteRedundanceGroup(userId);
}

/**
 * @tc.name: DeleteRedundanceGroup_002
 * @tc.desc: DeleteRedundanceGroup with valid userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteRedundanceGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_group_to_delete";
    connector.DeleteRedundanceGroup(userId);
}

/**
 * @tc.name: DealRedundanceGroup_001
 * @tc.desc: DealRedundanceGroup with empty userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DealRedundanceGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "";
    int32_t authType = 1;
    connector.DealRedundanceGroup(userId, authType);
}

/**
 * @tc.name: DealRedundanceGroup_002
 * @tc.desc: DealRedundanceGroup with valid userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DealRedundanceGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_deal_redundance_001";
    int32_t authType = 1;
    connector.DealRedundanceGroup(userId, authType);
}

/**
 * @tc.name: GetGroupInfo_With_Mock_001
 * @tc.desc: GetGroupInfo with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: GetGroupInfo_With_Mock_002
 * @tc.desc: GetGroupInfo with mock valid userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_With_Mock_002, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: GetGroupInfoCommon_With_Mock_001
 * @tc.desc: GetGroupInfoCommon with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: DeleteGroup_With_Mock_001
 * @tc.desc: DeleteGroup(std::string) with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::string groupId = "test_group_id";
    int32_t ret = connector.DeleteGroup(groupId);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: DeleteGroup_With_Mock_002
 * @tc.desc: DeleteGroup(requestId, userId, authType) with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_With_Mock_002, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    int64_t requestId = 1001;
    std::string userId = "test_user";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: CreateGroup_With_Mock_001
 * @tc.desc: CreateGroup with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    int64_t requestId = 2001;
    int32_t authType = 1;
    std::string userId = "test_user";
    JsonObject jsonOutObj;
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: CreateGroup_With_Mock_002
 * @tc.desc: CreateGroup with mock valid userId but null deviceGroupManager
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_With_Mock_002, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    int64_t requestId = 2002;
    int32_t authType = 1;
    std::string userId = "test_user";
    JsonObject jsonOutObj;
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: addMultiMembers_With_Mock_001
 * @tc.desc: addMultiMembers with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::string userId = "test_user";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.addMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: deleteMultiMembers_With_Mock_001
 * @tc.desc: deleteMultiMembers with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::string userId = "test_user";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.deleteMultiMembers(groupType, userId, jsonDeviceList);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: addMultiMembersExt_With_Mock_001
 * @tc.desc: addMultiMembersExt with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::string credentialInfo = "test_credential";
    int32_t ret = connector.addMultiMembersExt(credentialInfo);
    (void)ret;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

/**
 * @tc.name: IsRedundanceGroup_With_Mock_001
 * @tc.desc: IsRedundanceGroup with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    DmServiceHiChainConnector connector;
    std::string userId = "test_user";
    int32_t authType = 1;
    std::vector<DmGroupInfo> groupList;
    bool ret = connector.IsRedundanceGroup(userId, authType, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
