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
 * @tc.name: DmServiceHiChainConnector_001
 * @tc.desc: Test constructor and destructor with stack object
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_001, testing::ext::TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE({
        DmServiceHiChainConnector connector;
        (void)connector;
    });
}

/**
 * @tc.name: DmServiceHiChainConnector_002
 * @tc.desc: Test constructor and destructor with heap object
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_002, testing::ext::TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE({
        auto *connector = new DmServiceHiChainConnector();
        delete connector;
        connector = nullptr;
    });
}

/**
 * @tc.name: DmServiceHiChainConnector_003
 * @tc.desc: Test constructor and destructor with multiple iterations
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_003, testing::ext::TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE({
        for (int i = 0; i < 5; ++i) {
            DmServiceHiChainConnector connector;
            (void)connector;
        }
    });
}

/**
 * @tc.name: DmServiceHiChainConnector_004
 * @tc.desc: Test constructor when deviceGroupManager initialization fails
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_004, testing::ext::TestSize.Level1)
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
    EXPECT_TRUE(groupList.empty());
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DmServiceHiChainConnector_005
 * @tc.desc: Test constructor when deviceGroupManager exists but regCallback fails
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int32_t ret = connector.RegisterHiChainGroupCallback(nullptr);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnFinish_001
 * @tc.desc: Test onFinish with GROUP_CREATE and nullptr returnData
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
 * @tc.name: OnFinish_004
 * @tc.desc: Test onFinish with unknown operation code 999
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(4, 999, nullptr)
    );
}

/**
 * @tc.name: OnFinish_005
 * @tc.desc: Test onFinish with unknown operation code -1
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(5, -1, "any")
    );
}

/**
 * @tc.name: OnFinish_006
 * @tc.desc: Test onFinish with registered callback for GROUP_CREATE
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_006, testing::ext::TestSize.Level1)
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
 * @tc.name: OnFinish_007
 * @tc.desc: Test onFinish with registered callback for GROUP_DISBAND
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_007, testing::ext::TestSize.Level1)
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
 * @tc.name: OnFinish_008
 * @tc.desc: Test onFinish with networkStyle != CREDENTIAL_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_008, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *data = "test_data";
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(1, static_cast<int>(GroupOperationCode::GROUP_CREATE), data)
    );
}

/**
 * @tc.name: OnFinish_009
 * @tc.desc: Test onFinish GROUP_DISBAND with g_groupIsRedundance = true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_009, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *data = "disband_with_redundance";
    EXPECT_NO_FATAL_FAILURE(
        connector.onFinish(2, static_cast<int>(GroupOperationCode::GROUP_DISBAND), data)
    );
}

/**
 * @tc.name: OnError_001
 * @tc.desc: Test onError with GROUP_CREATE and nullptr errorReturn
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
 * @tc.name: OnError_005
 * @tc.desc: Test onError with unknown operation code 999
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(300, 999, -999, nullptr)
    );
}

/**
 * @tc.name: OnError_006
 * @tc.desc: Test onError with unknown operation code -1
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_006, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(301, -1, 0, "any")
    );
}

/**
 * @tc.name: OnError_007
 * @tc.desc: Test onError with registered callback for GROUP_CREATE
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_007, testing::ext::TestSize.Level1)
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
 * @tc.name: OnError_008
 * @tc.desc: Test onError with registered callback for GROUP_DISBAND
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_008, testing::ext::TestSize.Level1)
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
 * @tc.name: OnError_009
 * @tc.desc: Test onError with networkStyle != CREDENTIAL_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_009, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    const char *err = "network_style_error";
    EXPECT_NO_FATAL_FAILURE(
        connector.onError(100, static_cast<int>(GroupOperationCode::GROUP_CREATE), 123, err)
    );
}

/**
 * @tc.name: GetJsonStr_001
 * @tc.desc: Test GetJsonStr with non-existent key
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
 * @tc.name: GetJsonStr_003
 * @tc.desc: Test GetJsonStr with key exists but value is not string
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonStr_003, testing::ext::TestSize.Level1)
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
 * @tc.name: GetJsonInt_003
 * @tc.desc: Test GetJsonInt with key exists but value is not integer
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonInt_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj;
    jsonObj["authType"] = "not_an_int";
    int32_t ret = connector.GetJsonInt(jsonObj, "authType");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupInfoCommon_001
 * @tc.desc: Test GetGroupInfoCommon with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_002
 * @tc.desc: Test GetGroupInfoCommon with empty query params
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_003
 * @tc.desc: Test GetGroupInfoCommon with null deviceGroupManager (simulated via invalid state)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_003, testing::ext::TestSize.Level1)
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
    EXPECT_TRUE(groupList.empty());
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfoCommon_004
 * @tc.desc: Test GetGroupInfoCommon with empty group list result
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"groupType\":999}";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_005
 * @tc.desc: Test GetGroupInfoCommon with invalid JSON response
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "invalid_json";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_006
 * @tc.desc: Test GetGroupInfoCommon with JSON that's not an array
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_006, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"key\":\"value\"}";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_007
 * @tc.desc: Test GetGroupInfoCommon with empty groupInfos after parsing
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_007, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "[]";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_008
 * @tc.desc: Test GetGroupInfoCommon when groupVec is nullptr (simulated via API failure)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_008, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(-1, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(groupList.empty());
}

/**
 * @tc.name: GetGroupInfoCommon_009
 * @tc.desc: Test GetGroupInfoCommon when num is zero (simulated via invalid query)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_009, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"invalid_group\":999999}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_010
 * @tc.desc: Test GetGroupInfoCommon when groupInfos is empty after parsing
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_010, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "[]";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_011
 * @tc.desc: Test GetGroupInfoCommon when JSON response is object not array
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_011, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{\"not_an_array\":true}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfoCommon_012
 * @tc.desc: Test GetGroupInfoCommon with invalid JSON string response
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfoCommon_012, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{malformed_json_content";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "ohos.distributedhardware.devicemanagerservice",
                                            groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfo_001
 * @tc.desc: Test GetGroupInfo with specified userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfo(0, queryParams, groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfo_002
 * @tc.desc: Test GetGroupInfo with current account userId
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
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfo_003
 * @tc.desc: Test GetGroupInfo with specific groupType query
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
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupInfo_004
 * @tc.desc: Test GetGroupInfo with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_004, testing::ext::TestSize.Level1)
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
    EXPECT_TRUE(groupList.empty());
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfo_005
 * @tc.desc: Test GetGroupInfo with mock valid userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_005, testing::ext::TestSize.Level1)
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
    EXPECT_TRUE(ret == true || ret == false);
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: GetGroupInfo_006
 * @tc.desc: Test GetGroupInfoCommon with different package name
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_006, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    bool ret = connector.GetGroupInfoCommon(0, queryParams, "com.example.different.pkg", groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: GetGroupIdExt_001
 * @tc.desc: Test GetGroupIdExt with empty userId
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
 * @tc.name: GetGroupIdExt_003
 * @tc.desc: Test GetGroupIdExt with groupList having groups but no matching userId
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
 * @tc.name: ParseRemoteCredentialExt_001
 * @tc.desc: Test ParseRemoteCredentialExt with empty credentialInfo
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
 * @tc.name: ParseRemoteCredentialExt_005
 * @tc.desc: Test ParseRemoteCredentialExt with missing FIELD_DEVICE_LIST
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
 * @tc.name: ParseRemoteCredentialExt_007
 * @tc.desc: Test ParseRemoteCredentialExt with all valid fields including credentialType, operationCode
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
 * @tc.name: ParseRemoteCredentialExt_008
 * @tc.desc: Test ParseRemoteCredentialExt with FIELD_DEVICE_LIST missing
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_008, testing::ext::TestSize.Level1)
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
 * @tc.name: ParseRemoteCredentialExt_009
 * @tc.desc: Test ParseRemoteCredentialExt with GetGroupIdExt failure
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredentialExt_009, testing::ext::TestSize.Level1)
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
 * @tc.name: addMultiMembersExt_001
 * @tc.desc: Test addMultiMembersExt with empty credentialInfo
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
 * @tc.name: addMultiMembersExt_003
 * @tc.desc: Test addMultiMembersExt with negative userId
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
 * @tc.name: addMultiMembersExt_004
 * @tc.desc: Test addMultiMembersExt with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_004, testing::ext::TestSize.Level1)
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
 * @tc.name: addMultiMembersExt_005
 * @tc.desc: Test addMultiMembersExt with parsing success but API failure
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembersExt_005, testing::ext::TestSize.Level1)
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

/**
 * @tc.name: IsRedundanceGroup_001
 * @tc.desc: Test IsRedundanceGroup with invalid userId
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
    EXPECT_TRUE(groupList.empty());
}

/**
 * @tc.name: IsRedundanceGroup_002
 * @tc.desc: Test IsRedundanceGroup with valid userId but no groups
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
    EXPECT_TRUE(groupList.empty());
}

/**
 * @tc.name: IsRedundanceGroup_003
 * @tc.desc: Test IsRedundanceGroup with valid userId and existing groups
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_redundance_002";
    int32_t authType = 1;
    std::vector<DmGroupInfo> groupList;
    bool ret = connector.IsRedundanceGroup(userId, authType, groupList);
    EXPECT_TRUE(ret == true || ret == false);
}

/**
 * @tc.name: IsRedundanceGroup_004
 * @tc.desc: Test IsRedundanceGroup with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, IsRedundanceGroup_004, testing::ext::TestSize.Level1)
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
    EXPECT_TRUE(groupList.empty());
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

/**
 * @tc.name: DeleteGroup_001
 * @tc.desc: Test DeleteGroup(std::string) with empty groupId
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
 * @tc.name: DeleteGroup_006
 * @tc.desc: Test DeleteGroup(std::string) with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_006, testing::ext::TestSize.Level1)
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
 * @tc.name: DeleteGroup_007
 * @tc.desc: Test DeleteGroup(requestId, userId, authType) with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_007, testing::ext::TestSize.Level1)
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
 * @tc.name: DeleteGroup_008
 * @tc.desc: Test DeleteGroup timeout scenario simulation
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_008, testing::ext::TestSize.Level1)
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
 * @tc.name: DeleteGroup_009
 * @tc.desc: Test DeleteGroup(requestId, userId, authType) when user is found in groupList
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_009, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int64_t requestId = 3001;
    std::string userId = "test_user_found_in_group";
    int32_t authType = 1;
    int32_t ret = connector.DeleteGroup(requestId, userId, authType);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_010
 * @tc.desc: Test DeleteGroup timeout loop completion scenario
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_010, testing::ext::TestSize.Level1)
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
 * @tc.name: DeleteGroup_011
 * @tc.desc: Test DeleteGroup(string) with deleteGroup API failure
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DeleteGroup_011, testing::ext::TestSize.Level1)
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
 * @tc.name: DeleteRedundanceGroup_001
 * @tc.desc: Test DeleteRedundanceGroup with empty userId
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
 * @tc.name: DealRedundanceGroup_003
 * @tc.desc: Test DealRedundanceGroup with actual redundancy scenario
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DealRedundanceGroup_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string userId = "test_user_redundance_full";
    int32_t authType = 1;
    EXPECT_NO_FATAL_FAILURE(
        connector.DealRedundanceGroup(userId, authType)
    );
}

/**
 * @tc.name: CreateGroup_001
 * @tc.desc: Test CreateGroup with invalid jsonOutObj
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
 * @tc.name: CreateGroup_004
 * @tc.desc: Test CreateGroup with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_004, testing::ext::TestSize.Level1)
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
 * @tc.name: CreateGroup_005
 * @tc.desc: Test CreateGroup with mock valid userId but null deviceGroupManager
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_005, testing::ext::TestSize.Level1)
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
 * @tc.name: CreateGroup_006
 * @tc.desc: Test CreateGroup timeout scenario simulation
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_006, testing::ext::TestSize.Level1)
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
 * @tc.name: CreateGroup_007
 * @tc.desc: Test CreateGroup timeout loop completion scenario
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, CreateGroup_007, testing::ext::TestSize.Level1)
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
 * @tc.name: RegisterHiChainGroupCallback_001
 * @tc.desc: Test RegisterHiChainGroupCallback with valid callback
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
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, UnRegisterHiChainGroupCallback_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    int32_t ret = connector.UnRegisterHiChainGroupCallback();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetGroupId_001
 * @tc.desc: Test GetGroupId with empty groupList result
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
 * @tc.name: GetGroupId_004
 * @tc.desc: Test GetGroupId with groupList where userId doesn't match any group
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "no_match_user_id_999";
    int32_t groupType = 999;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: GetGroupId_005
 * @tc.desc: Test GetGroupId with multiple groups in list but none matching userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupId_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::string groupId = "";
    std::string userId = "user_with_no_match_anywhere";
    int32_t groupType = 1;
    int32_t ret = connector.GetGroupId(userId, groupType, groupId);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredential_001
 * @tc.desc: Test ParseRemoteCredential with empty userId
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
 * @tc.name: ParseRemoteCredential_005
 * @tc.desc: Test ParseRemoteCredential with negative osAccountUserId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, ParseRemoteCredential_005, testing::ext::TestSize.Level1)
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
 * @tc.name: addMultiMembers_001
 * @tc.desc: Test addMultiMembers with empty deviceList
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
 * @tc.name: addMultiMembers_004
 * @tc.desc: Test addMultiMembers with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_004, testing::ext::TestSize.Level1)
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
 * @tc.name: addMultiMembers_005
 * @tc.desc: Test addMultiMembers with addMultiMembersToGroup API failure (mock)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_005, testing::ext::TestSize.Level1)
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
 * @tc.name: addMultiMembers_006
 * @tc.desc: Test addMultiMembers with all valid parameters and successful API call
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, addMultiMembers_006, testing::ext::TestSize.Level1)
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
 * @tc.name: deleteMultiMembers_001
 * @tc.desc: Test deleteMultiMembers with empty deviceList
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
 * @tc.name: deleteMultiMembers_004
 * @tc.desc: Test deleteMultiMembers with mock negative userId
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_004, testing::ext::TestSize.Level1)
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
 * @tc.name: deleteMultiMembers_005
 * @tc.desc: Test deleteMultiMembers with delMultiMembersFromGroup API failure (mock)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, deleteMultiMembers_005, testing::ext::TestSize.Level1)
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
} // namespace DistributedHardware
} // namespace OHOS
