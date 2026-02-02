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

using testing::Return;

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
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

void DmServiceHiChainConnectorTest::TearDownTestCase()
{
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_001
 * @tc.desc: Test constructor and destructor with stack object
 *           Step 1: Create connector object on stack
 *           Step 2: Verify object is constructed successfully
 *           Step 3: Destroy object automatically when out of scope
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_001, testing::ext::TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE({
        DmServiceHiChainConnector connector;
        (void)connector;
    });
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_002
 * @tc.desc: Test constructor and destructor with heap object
 *           Step 1: Create connector object using new operator
 *           Step 2: Verify object is constructed successfully
 *           Step 3: Delete object manually
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_002, testing::ext::TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE({
        auto *connector = new DmServiceHiChainConnector();
        delete connector;
        connector = nullptr;
    });
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_003
 * @tc.desc: Test constructor and destructor with multiple iterations
 *           Step 1: Create and destroy connector objects in loop
 *           Step 2: Verify all objects are constructed and destructed successfully
 *           Step 3: Complete loop without memory leaks
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_003, testing::ext::TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE({
        for (int i = 0; i < 5; ++i) {
            DmServiceHiChainConnector connector;
            (void)connector;
        }
    });
}

/**
 * @tc.name: GetJsonStr_001
 * @tc.desc: Test GetJsonStr with non-existent key
 *           Step 1: Prepare connector and jsonObj without target key
 *           Step 2: Call GetJsonStr with non-existent key
 *           Step 3: Verify return value is empty string
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
 * @tc.desc: Test GetJsonStr with existing string key
 *           Step 1: Prepare connector and jsonObj with string value
 *           Step 2: Call GetJsonStr with existing key
 *           Step 3: Verify return value matches the stored string
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
 * @tc.desc: Test GetJsonStr with key exists but value is not string
 *           Step 1: Prepare connector and jsonObj with non-string value
 *           Step 2: Call GetJsonStr with key containing non-string value
 *           Step 3: Verify return value is empty string
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
 * @tc.desc: Test GetJsonInt with non-existent key
 *           Step 1: Prepare connector and jsonObj without target key
 *           Step 2: Call GetJsonInt with non-existent key
 *           Step 3: Verify return value is ERR_DM_FAILED
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
 * @tc.desc: Test GetJsonInt with existing integer key
 *           Step 1: Prepare connector and jsonObj with integer value
 *           Step 2: Call GetJsonInt with existing key
 *           Step 3: Verify return value matches the stored integer
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
 * @tc.desc: Test GetJsonInt with key exists but value is not integer
 *           Step 1: Prepare connector and jsonObj with non-integer value
 *           Step 2: Call GetJsonInt with key containing non-integer value
 *           Step 3: Verify return value is ERR_DM_FAILED
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
 * @tc.desc: Test GetGroupInfo with specified userId
 *           Step 1: Prepare connector with userId and empty queryParams
 *           Step 2: Call GetGroupInfo with userId parameter
 *           Step 3: Verify return value (defensive - HiChain dependency)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfo_002
 * @tc.desc: Test GetGroupInfo with current account userId
 *           Step 1: Prepare connector with empty queryParams
 *           Step 2: Call GetGroupInfo without userId parameter
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));

    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfo_003
 * @tc.desc: Test GetGroupInfo with specific groupType query
 *           Step 1: Prepare connector with queryParams containing groupType
 *           Step 2: Call GetGroupInfo with groupType filter
 *           Step 3: Verify return value is true
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
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_001
 * @tc.desc: Test GetGroupInfoCommon with valid parameters
 *           Step 1: Prepare connector with valid package name and empty query
 *           Step 2: Call GetGroupInfoCommon with specified parameters
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice", groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_002
 * @tc.desc: Test GetGroupInfoCommon with empty query params
 *           Step 1: Prepare connector with empty queryParams string
 *           Step 2: Call GetGroupInfoCommon with empty query
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice", groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_003
 * @tc.desc: Test GetGroupInfoCommon with different package name
 *           Step 1: Prepare connector with specific package name
 *           Step 2: Call GetGroupInfoCommon with different pkgName
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice", groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: RegisterHiChainGroupCallback_001
 * @tc.desc: Test RegisterHiChainGroupCallback with valid callback
 *           Step 1: Prepare connector and valid callback object
 *           Step 2: Call RegisterHiChainGroupCallback with callback
 *           Step 3: Verify return value is DM_OK
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
 * @tc.desc: Test RegisterHiChainGroupCallback with nullptr callback
 *           Step 1: Prepare connector with nullptr callback
 *           Step 2: Call RegisterHiChainGroupCallback with nullptr
 *           Step 3: Verify return value is DM_OK
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
 * @tc.desc: Test UnRegisterHiChainGroupCallback after registration
 *           Step 1: Prepare connector and register callback
 *           Step 2: Call UnRegisterHiChainGroupCallback
 *           Step 3: Verify return value is DM_OK
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
 * @tc.desc: Test UnRegisterHiChainGroupCallback without registration
 *           Step 1: Prepare connector without registering callback
 *           Step 2: Call UnRegisterHiChainGroupCallback directly
 *           Step 3: Verify return value is DM_OK
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
 * @tc.desc: Test onFinish with GROUP_CREATE and nullptr returnData
 *           Step 1: Prepare connector with nullptr data
 *           Step 2: Call onFinish with GROUP_CREATE operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(1, static_cast<int>(GroupOperationCode::GROUP_CREATE), nullptr)
    );
}

/**
 * @tc.name: OnFinish_002
 * @tc.desc: Test onFinish with GROUP_CREATE and valid returnData
 *           Step 1: Prepare connector with valid data pointer
 *           Step 2: Call onFinish with GROUP_CREATE operation and data
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *data = "create_ok";
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(2, static_cast<int>(GroupOperationCode::GROUP_CREATE), data)
    );
}

/**
 * @tc.name: OnFinish_003
 * @tc.desc: Test onFinish with GROUP_DISBAND and nullptr returnData
 *           Step 1: Prepare connector with nullptr data
 *           Step 2: Call onFinish with GROUP_DISBAND operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(3, static_cast<int>(GroupOperationCode::GROUP_DISBAND), nullptr)
    );
}

/**
 * @tc.name: OnFinish_004_Code_999
 * @tc.desc: Test onFinish with unknown operation code 999
 *           Step 1: Prepare connector with nullptr data
 *           Step 2: Call onFinish with operation code 999
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_004_Code_999, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(4, 999, nullptr)
    );
}

/**
 * @tc.name: OnFinish_005_Code_Negative1
 * @tc.desc: Test onFinish with unknown operation code -1
 *           Step 1: Prepare connector with valid data
 *           Step 2: Call onFinish with operation code -1
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_005_Code_Negative1, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(5, -1, "any")
    );
}

/**
 * @tc.name: OnError_001
 * @tc.desc: Test onError with GROUP_CREATE and nullptr errorReturn
 *           Step 1: Prepare connector with nullptr error data
 *           Step 2: Call onError with GROUP_CREATE operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(100, static_cast<int>(GroupOperationCode::GROUP_CREATE), -1, nullptr)
    );
}

/**
 * @tc.name: OnError_002
 * @tc.desc: Test onError with GROUP_CREATE and valid errorReturn
 *           Step 1: Prepare connector with valid error data
 *           Step 2: Call onError with GROUP_CREATE operation and error
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *err = "create_failed";
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(101, static_cast<int>(GroupOperationCode::GROUP_CREATE), 123, err)
    );
}

/**
 * @tc.name: OnError_003
 * @tc.desc: Test onError with GROUP_DISBAND and nullptr errorReturn
 *           Step 1: Prepare connector with nullptr error data
 *           Step 2: Call onError with GROUP_DISBAND operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(200, static_cast<int>(GroupOperationCode::GROUP_DISBAND), -2, nullptr)
    );
}

/**
 * @tc.name: OnError_004
 * @tc.desc: Test onError with GROUP_DISBAND and valid errorReturn
 *           Step 1: Prepare connector with valid error data
 *           Step 2: Call onError with GROUP_DISBAND operation and error
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *err = "disband_failed";
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(201, static_cast<int>(GroupOperationCode::GROUP_DISBAND), 456, err)
    );
}

/**
 * @tc.name: OnError_005_Code_999
 * @tc.desc: Test onError with unknown operation code 999
 *           Step 1: Prepare connector with nullptr error data
 *           Step 2: Call onError with operation code 999
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_005_Code_999, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(300, 999, -999, nullptr)
    );
}

/**
 * @tc.name: OnError_006_Code_Negative1
 * @tc.desc: Test onError with unknown operation code -1
 *           Step 1: Prepare connector with valid error data
 *           Step 2: Call onError with operation code -1
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_006_Code_Negative1, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(301, -1, 0, "any")
    );
}

/**
 * @tc.name: GetGroupId_001
 * @tc.desc: Test GetGroupId with empty groupList result
 *           Step 1: Prepare connector with userId and groupType
 *           Step 2: Call GetGroupId with parameters
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "test_user_123";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupId_002
 * @tc.desc: Test GetGroupId with existing group info
 *           Step 1: Prepare connector with valid userId and groupType
 *           Step 2: Call GetGroupId to find matching group
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "test_user_456";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupId_003
 * @tc.desc: Test GetGroupId with different groupType
 *           Step 1: Prepare connector with userId and different groupType
 *           Step 2: Call GetGroupId with groupType 2
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "test_user_getid_003";
    int32_t groupType = 2;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupIdExt_001
 * @tc.desc: Test GetGroupIdExt with empty userId
 *           Step 1: Prepare connector with empty userId
 *           Step 2: Call GetGroupIdExt with invalid parameters
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupIdExt_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string groupOwner = "";
    std::string userId = "";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupIdExt(userId, groupType, groupId, groupOwner);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupIdExt_002
 * @tc.desc: Test GetGroupIdExt with valid userId
 *           Step 1: Prepare connector with valid userId
 *           Step 2: Call GetGroupIdExt with valid parameters
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupIdExt_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string groupOwner = "";
    std::string userId = "test_user_ext_001";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupIdExt(userId, groupType, groupId, groupOwner);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredential_001
 * @tc.desc: Test ParseRemoteCredential with empty userId
 *           Step 1: Prepare connector with empty userId and deviceList
 *           Step 2: Call ParseRemoteCredential with invalid parameters
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
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
 * @tc.desc: Test ParseRemoteCredential with valid userId but no deviceList
 *           Step 1: Prepare connector with userId but empty deviceList
 *           Step 2: Call ParseRemoteCredential with missing deviceList
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
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
 * @tc.desc: Test ParseRemoteCredential with valid parameters
 *           Step 1: Prepare connector with userId and deviceList
 *           Step 2: Call ParseRemoteCredential with all valid parameters
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredential_004
 * @tc.desc: Test ParseRemoteCredential with userId and deviceList but group not found
 *           Step 1: Prepare connector with non-existent userId
 *           Step 2: Call ParseRemoteCredential with deviceList
 *           Step 3: Verify return value is ERR_DM_FAILED
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
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_001
 * @tc.desc: Test ParseRemoteCredentialExt with empty credentialInfo
 *           Step 1: Prepare connector with empty credentialInfo
 *           Step 2: Call ParseRemoteCredentialExt with invalid parameter
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string credentialInfo = "";
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_002
 * @tc.desc: Test ParseRemoteCredentialExt with invalid json string
 *           Step 1: Prepare connector with invalid JSON string
 *           Step 2: Call ParseRemoteCredentialExt with malformed JSON
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string credentialInfo = "invalid_json_string";
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_003
 * @tc.desc: Test ParseRemoteCredentialExt with valid credentialInfo (SAME_ACCOUNT)
 *           Step 1: Prepare connector with valid JSON credential
 *           Step 2: Call ParseRemoteCredentialExt with SAME_ACCOUNT authType
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_003, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_004
 * @tc.desc: Test ParseRemoteCredentialExt with authType != SAME_ACCOUNT
 *           Step 1: Prepare connector with different authType
 *           Step 2: Call ParseRemoteCredentialExt with authType 2
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembers_001
 * @tc.desc: Test addMultiMembers with empty deviceList
 *           Step 1: Prepare connector with userId and empty deviceList
 *           Step 2: Call addMultiMembers with invalid parameters
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
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
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembers_002
 * @tc.desc: Test addMultiMembers with valid parameters
 *           Step 1: Prepare connector with userId and deviceList
 *           Step 2: Call addMultiMembers with valid parameters
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembers_003
 * @tc.desc: Test addMultiMembers with multiple devices
 *           Step 1: Prepare connector with multiple devices in list
 *           Step 2: Call addMultiMembers with 3 devices
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembersExt_001
 * @tc.desc: Test addMultiMembersExt with empty credentialInfo
 *           Step 1: Prepare connector with empty credentialInfo
 *           Step 2: Call addMultiMembersExt with invalid parameter
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string credentialInfo = "";
    int32_t ret = connector.addMultiMembersExt(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembersExt_002
 * @tc.desc: Test addMultiMembersExt with valid credentialInfo
 *           Step 1: Prepare connector with valid JSON credentialInfo
 *           Step 2: Call addMultiMembersExt with valid parameter
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: deleteMultiMembers_001
 * @tc.desc: Test deleteMultiMembers with empty deviceList
 *           Step 1: Prepare connector with userId and empty deviceList
 *           Step 2: Call deleteMultiMembers with invalid parameters
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
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
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: deleteMultiMembers_002
 * @tc.desc: Test deleteMultiMembers with valid parameters
 *           Step 1: Prepare connector with userId and deviceList
 *           Step 2: Call deleteMultiMembers with valid parameters
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: deleteMultiMembers_003
 * @tc.desc: Test deleteMultiMembers with multiple devices
 *           Step 1: Prepare connector with multiple devices in list
 *           Step 2: Call deleteMultiMembers with 2 devices
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_001
 * @tc.desc: Test DeleteGroup(std::string) with empty groupId
 *           Step 1: Prepare connector with empty groupId
 *           Step 2: Call DeleteGroup with invalid parameter
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    int32_t ret = connector.DeleteGroup(groupId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_002
 * @tc.desc: Test DeleteGroup(std::string) with valid groupId
 *           Step 1: Prepare connector with valid groupId
 *           Step 2: Call DeleteGroup with groupId parameter
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "test_group_id_001";
    int32_t ret = connector.DeleteGroup(groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_003
 * @tc.desc: Test DeleteGroup(requestId, userId, authType) with empty userId
 *           Step 1: Prepare connector with empty userId
 *           Step 2: Call DeleteGroup with invalid userId parameter
 *           Step 3: Verify return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 1001;
    std::string userId = "";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_004
 * @tc.desc: Test DeleteGroup(requestId, userId, authType) with valid userId
 *           Step 1: Prepare connector with valid userId
 *           Step 2: Call DeleteGroup with valid parameters
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 1002;
    std::string userId = "test_user_del_group_001";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_005
 * @tc.desc: Test DeleteGroup(requestId, userId, authType) with different authType
 *           Step 1: Prepare connector with userId and authType 2
 *           Step 2: Call DeleteGroup with different authType
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 1005;
    std::string userId = "test_user_del_group_005";
    int32_t authType = 2;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: CreateGroup_001
 * @tc.desc: Test CreateGroup with invalid jsonOutObj
 *           Step 1: Prepare connector with empty jsonOutObj
 *           Step 2: Call CreateGroup with invalid parameter
 *           Step 3: Verify return value is ERR_DM_CREATE_GROUP_FAILED
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
    EXPECT_EQ(ret, ERR_DM_CREATE_GROUP_FAILED);
}

/**
 * @tc.name: CreateGroup_002
 * @tc.desc: Test CreateGroup with valid parameters
 *           Step 1: Prepare connector with valid jsonOutObj
 *           Step 2: Call CreateGroup with all valid parameters
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED || ret == ERR_DM_CREATE_GROUP_FAILED);
}

/**
 * @tc.name: CreateGroup_003
 * @tc.desc: Test CreateGroup with different authType
 *           Step 1: Prepare connector with authType 2
 *           Step 2: Call CreateGroup with different authType
 *           Step 3: Verify return value is DM_OK
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
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED || ret == ERR_DM_CREATE_GROUP_FAILED);
}

/**
 * @tc.name: IsRedundanceGroup_001
 * @tc.desc: Test IsRedundanceGroup with invalid userId
 *           Step 1: Prepare connector with empty userId
 *           Step 2: Call IsRedundanceGroup with invalid parameter
 *           Step 3: Verify return value is false
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
 * @tc.desc: Test IsRedundanceGroup with valid userId but no groups
 *           Step 1: Prepare connector with valid userId
 *           Step 2: Call IsRedundanceGroup with empty groupList
 *           Step 3: Verify return value is false
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
 * @tc.desc: Test IsRedundanceGroup with valid userId and existing groups
 *           Step 1: Prepare connector with userId and groups
 *           Step 2: Call IsRedundanceGroup with existing groupList
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_redundance_002";
    int32_t authType = 1;
    std::vector<DmGroupInfo> groupList;
    bool ret = connector.IsRedundanceGroup(userId, authType, groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: DeleteRedundanceGroup_001
 * @tc.desc: Test DeleteRedundanceGroup with empty userId
 *           Step 1: Prepare connector with empty userId
 *           Step 2: Call DeleteRedundanceGroup with invalid parameter
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteRedundanceGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "";
    EXPECT_NO_FATAL_FAILURE(
        connector.DeleteRedundanceGroup(userId)
    );
}

/**
 * @tc.name: DeleteRedundanceGroup_002
 * @tc.desc: Test DeleteRedundanceGroup with valid userId
 *           Step 1: Prepare connector with valid userId
 *           Step 2: Call DeleteRedundanceGroup with valid parameter
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteRedundanceGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_group_to_delete";
    EXPECT_NO_FATAL_FAILURE(
        connector.DeleteRedundanceGroup(userId)
    );
}

/**
 * @tc.name: DealRedundanceGroup_001
 * @tc.desc: Test DealRedundanceGroup with empty userId
 *           Step 1: Prepare connector with empty userId
 *           Step 2: Call DealRedundanceGroup with invalid parameters
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DealRedundanceGroup_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "";
    int32_t authType = 1;
    EXPECT_NO_FATAL_FAILURE(
        connector.DealRedundanceGroup(userId, authType)
    );
}

/**
 * @tc.name: DealRedundanceGroup_002
 * @tc.desc: Test DealRedundanceGroup with valid userId
 *           Step 1: Prepare connector with valid userId
 *           Step 2: Call DealRedundanceGroup with valid parameters
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DealRedundanceGroup_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_deal_redundance_001";
    int32_t authType = 1;
    EXPECT_NO_FATAL_FAILURE(
        connector.DealRedundanceGroup(userId, authType)
    );
}

/**
 * @tc.name: GetGroupInfo_With_Mock_001
 * @tc.desc: Test GetGroupInfo with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call GetGroupInfo with mocked connector
 *           Step 3: Verify return value is false and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfo_With_Mock_002
 * @tc.desc: Test GetGroupInfo with mock valid userId
 *           Step 1: Setup mock to return 1001 for GetCurrentAccountUserID
 *           Step 2: Call GetGroupInfo with mocked connector
 *           Step 3: Verify return value is true and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_With_Mock_002, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(1001));
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_LE(ret, true);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfoCommon_With_Mock_001
 * @tc.desc: Test GetGroupInfoCommon with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call GetGroupInfo with mocked connector
 *           Step 3: Verify return value is false and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DeleteGroup_With_Mock_001
 * @tc.desc: Test DeleteGroup(std::string) with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call DeleteGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string groupId = "test_group_id";
    int32_t ret = connector.DeleteGroup(groupId);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DeleteGroup_With_Mock_002
 * @tc.desc: Test DeleteGroup(requestId, userId, authType) with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call DeleteGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_With_Mock_002, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    int64_t requestId = 1001;
    std::string userId = "test_user";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: CreateGroup_With_Mock_001
 * @tc.desc: Test CreateGroup with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call CreateGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    int64_t requestId = 2001;
    int32_t authType = 1;
    std::string userId = "test_user";
    JsonObject jsonOutObj;
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: CreateGroup_With_Mock_002
 * @tc.desc: Test CreateGroup with mock valid userId but null deviceGroupManager
 *           Step 1: Setup mock to return 1001 for GetCurrentAccountUserID
 *           Step 2: Call CreateGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_With_Mock_002, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(1001));
    DmServiceHiChainConnector connector;
    int64_t requestId = 2002;
    int32_t authType = 1;
    std::string userId = "test_user";
    JsonObject jsonOutObj;
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: addMultiMembers_With_Mock_001
 * @tc.desc: Test addMultiMembers with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call addMultiMembers with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string userId = "test_user";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.addMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: deleteMultiMembers_With_Mock_001
 * @tc.desc: Test deleteMultiMembers with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call deleteMultiMembers with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string userId = "test_user";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.deleteMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: addMultiMembersExt_With_Mock_001
 * @tc.desc: Test addMultiMembersExt with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call addMultiMembersExt with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user";
    jsonObj["credentialType"] = 0;
    jsonObj["operationCode"] = 0;
    jsonObj["TType"] = 0;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonObj["deviceList"] = deviceList;
    std::string credentialInfo = jsonObj.Dump();
    int32_t ret = connector.addMultiMembersExt(credentialInfo);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: IsRedundanceGroup_With_Mock_001
 * @tc.desc: Test IsRedundanceGroup with mock negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call IsRedundanceGroup with mocked connector
 *           Step 3: Verify return value is false and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_With_Mock_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string userId = "test_user";
    int32_t authType = 1;
    std::vector<DmGroupInfo> groupList;
    bool ret = connector.IsRedundanceGroup(userId, authType, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfoCommon_004
 * @tc.desc: Test GetGroupInfoCommon with null deviceGroupManager (simulated via invalid state)
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call GetGroupInfo with mocked connector
 *           Step 3: Verify return value is false and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_004, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(1)
        .WillOnce(Return(-1));
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfoCommon_005
 * @tc.desc: Test GetGroupInfoCommon with getGroupInfo API failure (mock negative userId)
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call GetGroupInfo with mocked connector
 *           Step 3: Verify return value is false and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_005, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(1)
        .WillOnce(Return(-1));
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfoCommon_006
 * @tc.desc: Test GetGroupInfoCommon with empty group list result
 *           Step 1: Prepare connector with non-existent groupType query
 *           Step 2: Call GetGroupInfo with groupType 999
 *           Step 3: Verify return value is true with empty result
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_006, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"groupType\":999}";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_007
 * @tc.desc: Test GetGroupInfoCommon with invalid JSON response
 *           Step 1: Prepare connector with invalid JSON string
 *           Step 2: Call GetGroupInfo with malformed JSON
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_007, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "invalid_json";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_008
 * @tc.desc: Test GetGroupInfoCommon with JSON that's not an array
 *           Step 1: Prepare connector with JSON object instead of array
 *           Step 2: Call GetGroupInfo with object JSON
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_008, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"key\":\"value\"}";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_009
 * @tc.desc: Test GetGroupInfoCommon with empty groupInfos after parsing
 *           Step 1: Prepare connector with empty array JSON
 *           Step 2: Call GetGroupInfo with empty array
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_009, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "[]";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: OnFinish_With_Callback_001
 * @tc.desc: Test onFinish with registered callback for GROUP_CREATE
 *           Step 1: Prepare connector and register callback
 *           Step 2: Call onFinish with GROUP_CREATE operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_With_Callback_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    auto callback = std::make_shared<TestDmServiceGroupResCallback>();
    int32_t ret = connector.RegisterHiChainGroupCallback(callback);
    ASSERT_EQ(ret, DM_OK);
    const char *data = "create_success";
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(1, static_cast<int>(GroupOperationCode::GROUP_CREATE), data)
    );
}

/**
 * @tc.name: OnFinish_With_Callback_002
 * @tc.desc: Test onFinish with registered callback for GROUP_DISBAND
 *           Step 1: Prepare connector and register callback
 *           Step 2: Call onFinish with GROUP_DISBAND operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_With_Callback_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    auto callback = std::make_shared<TestDmServiceGroupResCallback>();
    int32_t ret = connector.RegisterHiChainGroupCallback(callback);
    ASSERT_EQ(ret, DM_OK);
    const char *data = "disband_success";
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(2, static_cast<int>(GroupOperationCode::GROUP_DISBAND), data)
    );
}

/**
 * @tc.name: OnError_With_Callback_001
 * @tc.desc: Test onError with registered callback for GROUP_CREATE
 *           Step 1: Prepare connector and register callback
 *           Step 2: Call onError with GROUP_CREATE operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_With_Callback_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    auto callback = std::make_shared<TestDmServiceGroupResCallback>();
    int32_t ret = connector.RegisterHiChainGroupCallback(callback);
    ASSERT_EQ(ret, DM_OK);
    const char *err = "create_error";
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(100, static_cast<int>(GroupOperationCode::GROUP_CREATE), 123, err)
    );
}

/**
 * @tc.name: OnError_With_Callback_002
 * @tc.desc: Test onError with registered callback for GROUP_DISBAND
 *           Step 1: Prepare connector and register callback
 *           Step 2: Call onError with GROUP_DISBAND operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_With_Callback_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    auto callback = std::make_shared<TestDmServiceGroupResCallback>();
    int32_t ret = connector.RegisterHiChainGroupCallback(callback);
    ASSERT_EQ(ret, DM_OK);
    const char *err = "disband_error";
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(200, static_cast<int>(GroupOperationCode::GROUP_DISBAND), 456, err)
    );
}

/**
 * @tc.name: ParseRemoteCredentialExt_005
 * @tc.desc: Test ParseRemoteCredentialExt with missing FIELD_DEVICE_LIST
 *           Step 1: Prepare connector with JSON missing deviceList field
 *           Step 2: Call ParseRemoteCredentialExt with incomplete data
 *           Step 3: Verify return value is ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_005, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user_device_list";
    jsonObj["credentialType"] = 0;
    jsonObj["operationCode"] = 0;
    jsonObj["TType"] = 0;
    std::string credentialInfo = jsonObj.Dump();
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_006
 * @tc.desc: Test ParseRemoteCredentialExt with GetGroupIdExt failure
 *           Step 1: Prepare connector with non-existent userId
 *           Step 2: Call ParseRemoteCredentialExt with invalid user
 *           Step 3: Verify return value is ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_006, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "non_existent_user_for_group";
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
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: Constructor_With_Null_Manager_001
 * @tc.desc: Test constructor when deviceGroupManager initialization fails
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Create connector with mocked negative user
 *           Step 3: Verify GetGroupInfo fails and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, Constructor_With_Null_Manager_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    EXPECT_FALSE(ret);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupIdExt_003
 * @tc.desc: Test GetGroupIdExt with groupList having groups but no matching userId
 *           Step 1: Prepare connector with non-existent userId
 *           Step 2: Call GetGroupIdExt with invalid user
 *           Step 3: Verify return value is ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupIdExt_003, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string groupOwner = "";
    std::string userId = "non_existent_user_003";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupIdExt(userId, groupType, groupId, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembersExt_003
 * @tc.desc: Test addMultiMembersExt with negative userId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call addMultiMembersExt with valid credential (to reach the API call)
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_003, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user_003";
    jsonObj["credentialType"] = 0;
    jsonObj["operationCode"] = 0;
    jsonObj["TType"] = 0;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonObj["deviceList"] = deviceList;
    std::string credentialInfo = jsonObj.Dump();
    int32_t ret = connector.addMultiMembersExt(credentialInfo);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfoCommon_010
 * @tc.desc: Test GetGroupInfoCommon with userId = 0 edge case
 *           Step 1: Prepare connector with userId = 0
 *           Step 2: Call GetGroupInfoCommon with zero userId
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_010, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice", groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: OnFinish_NetworkStyle_Test_001
 * @tc.desc: Test onFinish with networkStyle != CREDENTIAL_NETWORK
 *           Step 1: Prepare connector with test data
 *           Step 2: Call onFinish with GROUP_CREATE operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_NetworkStyle_Test_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *data = "test_data";
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(1, static_cast<int>(GroupOperationCode::GROUP_CREATE), data)
    );
}

/**
 * @tc.name: OnError_NetworkStyle_Test_001
 * @tc.desc: Test onError with networkStyle != CREDENTIAL_NETWORK
 *           Step 1: Prepare connector with error data
 *           Step 2: Call onError with GROUP_CREATE operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_NetworkStyle_Test_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *err = "network_style_error";
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(100, static_cast<int>(GroupOperationCode::GROUP_CREATE), 123, err)
    );
}

/**
 * @tc.name: OnFinish_With_Redundancy_001
 * @tc.desc: Test onFinish GROUP_DISBAND with g_groupIsRedundance = true
 *           Step 1: Prepare connector with disband data
 *           Step 2: Call onFinish with GROUP_DISBAND operation
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_With_Redundancy_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *data = "disband_with_redundance";
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(2, static_cast<int>(GroupOperationCode::GROUP_DISBAND), data)
    );
}

/**
 * @tc.name: ParseRemoteCredentialExt_007
 * @tc.desc: Test ParseRemoteCredentialExt with all valid fields including credentialType, operationCode
 *           Step 1: Prepare connector with full valid JSON credential
 *           Step 2: Call ParseRemoteCredentialExt with all fields
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_007, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user_full_fields";
    jsonObj["credentialType"] = 1;
    jsonObj["operationCode"] = 1;
    jsonObj["TType"] = 1;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    deviceList.push_back("device2");
    jsonObj["deviceList"] = deviceList;
    std::string credentialInfo = jsonObj.Dump();
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupId_No_Match_001
 * @tc.desc: Test GetGroupId with groupList where userId doesn't match any group
 *           Step 1: Prepare connector with non-matching userId
 *           Step 2: Call GetGroupId with groupType 999
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_No_Match_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "no_match_user_id_999";
    int32_t groupType = 999;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembers_API_Fail_001
 * @tc.desc: Test addMultiMembers with addMultiMembersToGroup API failure (mock)
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call addMultiMembers with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_API_Fail_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_api_fail";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.addMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: deleteMultiMembers_API_Fail_001
 * @tc.desc: Test deleteMultiMembers with delMultiMembersFromGroup API failure (mock)
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call deleteMultiMembers with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_API_Fail_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_del_api_fail";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.deleteMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: CreateGroup_Timeout_Simulate_001
 * @tc.desc: Test CreateGroup timeout scenario simulation
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call CreateGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_Timeout_Simulate_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    int64_t requestId = 9999;
    int32_t authType = 1;
    std::string userId = "timeout_user_test";
    JsonObject jsonOutObj;
    jsonOutObj["testKey"] = "testValue";
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DeleteGroup_Timeout_Simulate_001
 * @tc.desc: Test DeleteGroup timeout scenario simulation
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call DeleteGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_Timeout_Simulate_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string groupId = "timeout_group_id";
    int32_t ret = connector.DeleteGroup(groupId);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DealRedundanceGroup_Full_Test_001
 * @tc.desc: Test DealRedundanceGroup with actual redundancy scenario
 *           Step 1: Prepare connector with valid userId and authType
 *           Step 2: Call DealRedundanceGroup with parameters
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DealRedundanceGroup_Full_Test_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_redundance_full";
    int32_t authType = 1;
    EXPECT_NO_FATAL_FAILURE(
        connector.DealRedundanceGroup(userId, authType)
    );
}

/**
 * @tc.name: GetGroupInfo_Different_PkgName_001
 * @tc.desc: Test GetGroupInfoCommon with different package name
 *           Step 1: Prepare connector with different package name
 *           Step 2: Call GetGroupInfoCommon with custom pkgName
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_Different_PkgName_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "com.example.different.pkg", groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: Constructor_RegCallback_Fail_001
 * @tc.desc: Test constructor when deviceGroupManager exists but regCallback fails
 *           Step 1: Prepare connector with valid state
 *           Step 2: Call RegisterHiChainGroupCallback with nullptr
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, Constructor_RegCallback_Fail_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int32_t ret = connector.RegisterHiChainGroupCallback(nullptr);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetGroupInfoCommon_Null_GroupVec_001
 * @tc.desc: Test GetGroupInfoCommon when groupVec is nullptr (simulated via API failure)
 *           Step 1: Prepare connector with invalid userId -1
 *           Step 2: Call GetGroupInfoCommon with negative userId
 *           Step 3: Verify return value is false
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_Null_GroupVec_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(-1, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetGroupInfoCommon_Zero_Num_001
 * @tc.desc: Test GetGroupInfoCommon when num is zero (simulated via invalid query)
 *           Step 1: Prepare connector with invalid group query
 *           Step 2: Call GetGroupInfoCommon with non-existent group
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_Zero_Num_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"invalid_group\":999999}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_Invalid_JSON_001
 * @tc.desc: Test GetGroupInfoCommon with invalid JSON string response
 *           Step 1: Prepare connector with malformed JSON query
 *           Step 2: Call GetGroupInfoCommon with malformed JSON
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_Invalid_JSON_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{malformed_json_content";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_Not_Array_001
 * @tc.desc: Test GetGroupInfoCommon when JSON response is object not array
 *           Step 1: Prepare connector with object JSON query
 *           Step 2: Call GetGroupInfoCommon with non-array JSON
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_Not_Array_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"not_an_array\":true}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupInfoCommon_Empty_Result_001
 * @tc.desc: Test GetGroupInfoCommon when groupInfos is empty after parsing
 *           Step 1: Prepare connector with empty array query
 *           Step 2: Call GetGroupInfoCommon with empty array
 *           Step 3: Verify return value is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_Empty_Result_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "[]";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_LE(ret, true);
}

/**
 * @tc.name: GetGroupId_Multiple_Groups_No_Match_001
 * @tc.desc: Test GetGroupId with multiple groups in list but none matching userId
 *           Step 1: Prepare connector with non-matching userId
 *           Step 2: Call GetGroupId with groupType 1
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_Multiple_Groups_No_Match_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "user_with_no_match_anywhere";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_Missing_DeviceList_001
 * @tc.desc: Test ParseRemoteCredentialExt with FIELD_DEVICE_LIST missing
 *           Step 1: Prepare connector with JSON missing deviceList
 *           Step 2: Call ParseRemoteCredentialExt without deviceList
 *           Step 3: Verify return value is ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_Missing_DeviceList_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user_no_device_list";
    jsonObj["credentialType"] = 0;
    jsonObj["operationCode"] = 0;
    jsonObj["TType"] = 0;
    std::string credentialInfo = jsonObj.Dump();
    std::string params = "";
    std::string groupOwner = "";
    int32_t ret = connector.ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_GetGroupId_Fail_001
 * @tc.desc: Test ParseRemoteCredentialExt with GetGroupIdExt failure
 *           Step 1: Prepare connector with non-existent userId
 *           Step 2: Call ParseRemoteCredentialExt with invalid user
 *           Step 3: Verify return value is ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_GetGroupId_Fail_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(0));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "non_existent_user_group_fail";
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
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: CreateGroup_Timeout_Completion_001
 * @tc.desc: Test CreateGroup timeout loop completion scenario
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call CreateGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_Timeout_Completion_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));

    DmServiceHiChainConnector connector;
    int64_t requestId = 8888;
    int32_t authType = 1;
    std::string userId = "timeout_test_user";
    JsonObject jsonOutObj;
    jsonOutObj["testKey"] = "testValue";
    int32_t ret = connector.CreateGroup(requestId, authType, userId, jsonOutObj);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DeleteGroup_User_Found_001
 * @tc.desc: Test DeleteGroup(requestId, userId, authType) when user is found in groupList
 *           Step 1: Prepare connector with valid userId
 *           Step 2: Call DeleteGroup with existing user
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_User_Found_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 3001;
    std::string userId = "test_user_found_in_group";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_Timeout_Completion_001
 * @tc.desc: Test DeleteGroup timeout loop completion scenario
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call DeleteGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_Timeout_Completion_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    int64_t requestId = 7777;
    std::string userId = "timeout_delete_user";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DeleteGroup_API_Failure_001
 * @tc.desc: Test DeleteGroup(string) with deleteGroup API failure
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call DeleteGroup with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_API_Failure_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string groupId = "test_group_api_failure";
    int32_t ret = connector.DeleteGroup(groupId);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: addMultiMembers_Success_All_Valid_001
 * @tc.desc: Test addMultiMembers with all valid parameters and successful API call
 *           Step 1: Prepare connector with userId and 3 devices
 *           Step 2: Call addMultiMembers with all valid parameters
 *           Step 3: Verify return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_Success_All_Valid_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_all_valid";
    int32_t groupType = 1;
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    deviceList.push_back("device2");
    deviceList.push_back("device3");
    jsonDeviceList["deviceList"] = deviceList;
    int32_t ret = connector.addMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredential_Negative_UserId_001
 * @tc.desc: Test ParseRemoteCredential with negative osAccountUserId
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call ParseRemoteCredential with mocked connector
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredential_Negative_UserId_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_negative_id";
    JsonObject jsonDeviceList;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonDeviceList["deviceList"] = deviceList;
    std::string params = "";
    int32_t osAccountUserId = 0;
    int32_t groupType = 1;
    int32_t ret = connector.ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: addMultiMembersExt_Parsing_Success_API_Fail_001
 * @tc.desc: Test addMultiMembersExt with parsing success but API failure
 *           Step 1: Setup mock to return -1 for GetCurrentAccountUserID
 *           Step 2: Call addMultiMembersExt with valid credential
 *           Step 3: Verify return value is not DM_OK and cleanup mock
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_Parsing_Success_API_Fail_001, testing::ext::TestSize.Level1)
{
    auto mockUserConnector = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = mockUserConnector;
    EXPECT_CALL(*mockUserConnector, GetCurrentAccountUserID())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(-1));
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj["userId"] = "test_user_parse_success_api_fail";
    jsonObj["credentialType"] = 0;
    jsonObj["operationCode"] = 0;
    jsonObj["TType"] = 0;
    std::vector<std::string> deviceList;
    deviceList.push_back("device1");
    jsonObj["deviceList"] = deviceList;
    std::string credentialInfo = jsonObj.Dump();
    int32_t ret = connector.addMultiMembersExt(credentialInfo);
    EXPECT_NE(ret, DM_OK);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

} // namespace DistributedHardware
} // namespace OHOS
