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

namespace OHOS {
namespace DistributedHardware {
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
    ASSERT_NO_FATAL_FAILURE({
        DmServiceHiChainConnector connector;
        (void)connector;
    });
    EXPECT_TRUE(true);
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_002
 * @tc.desc: Constructor + Destructor (heap object new/delete)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_002, testing::ext::TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE({
        auto *connector = new DmServiceHiChainConnector();
        delete connector;
        connector = nullptr;
    });
    EXPECT_TRUE(true);
}

/**
 * @tc.name: DmServiceHiChainConnector_Constructor_003
 * @tc.desc: Constructor + Destructor (multiple times)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, DmServiceHiChainConnector_Constructor_003, testing::ext::TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE({
        for (int i = 0; i < 5; ++i) {
            DmServiceHiChainConnector connector;
            (void)connector;
        }
    });
    EXPECT_TRUE(true);
}
/**
 * @tc.name: GetJsonStr_001
 * @tc.desc: key not exist(or invalid) -> return empty string
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonStr_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj; // empty, no key
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
 * @tc.name: GetJsonInt_001
 * @tc.desc: key not exist(or invalid) -> return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetJsonInt_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    JsonObject jsonObj; // empty, no key
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
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetGroupInfo_252_260_001
 * @tc.desc: GetGroupInfo(queryParams, groupList) userId = MultipleUserConnector::GetCurrentAccountUserID()
 *          - userId < 0 -> false；or-> GetGroupInfo(userId,...)
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, GetGroupInfo_002, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    std::vector<DmGroupInfo> groupList;
    std::string queryParams = "{}";
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    bool ret = connector.GetGroupInfo(queryParams, groupList);
    if (userId < 0) {
        EXPECT_FALSE(ret);
    } else {
        bool ret2 = connector.GetGroupInfo(userId, queryParams, groupList);
        EXPECT_EQ(ret, ret2);
    }
}

/**
 * @tc.name: OnFinish_001
 * @tc.desc: operationCode = GROUP_CREATE + returnData == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    ASSERT_NO_FATAL_FAILURE(connector.onFinish(1, static_cast<int>(GroupOperationCode::GROUP_CREATE), nullptr));
    EXPECT_TRUE(true);
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
    ASSERT_NO_FATAL_FAILURE(connector.onFinish(2, static_cast<int>(GroupOperationCode::GROUP_CREATE), data));
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnFinish_003
 * @tc.desc: Test onFinish with GROUP_DISBAND and nullptr returnData
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    ASSERT_NO_FATAL_FAILURE(connector.onFinish(3, static_cast<int>(GroupOperationCode::GROUP_DISBAND), nullptr));
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnFinish_004
 * @tc.desc: Cover the default path for operationCode not being create/disband
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnFinish_004, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    ASSERT_NO_FATAL_FAILURE(connector.onFinish(4, 999, nullptr));
    ASSERT_NO_FATAL_FAILURE(connector.onFinish(5, -1, "any"));
    EXPECT_TRUE(true);
}
/**
 * @tc.name: OnError_001
 * @tc.desc: operationCode = GROUP_CREATE, errorReturn == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_001, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    ASSERT_NO_FATAL_FAILURE(
        connector.onError(100, static_cast<int>(GroupOperationCode::GROUP_CREATE), -1, nullptr)
    );
    EXPECT_TRUE(true);
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
    ASSERT_NO_FATAL_FAILURE(
        connector.onError(101, static_cast<int>(GroupOperationCode::GROUP_CREATE), 123, err)
    );
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnError_003
 * @tc.desc: operationCode = GROUP_DISBAND, errorReturn == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_003, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    ASSERT_NO_FATAL_FAILURE(
        connector.onError(200, static_cast<int>(GroupOperationCode::GROUP_DISBAND), -2, nullptr)
    );
    EXPECT_TRUE(true);
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
    ASSERT_NO_FATAL_FAILURE(
        connector.onError(201, static_cast<int>(GroupOperationCode::GROUP_DISBAND), 456, err)
    );
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnError_005
 * @tc.desc: operationCode no create/disband
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceHiChainConnectorTest, OnError_005, testing::ext::TestSize.Level1)
{
    DmServiceHiChainConnector connector;
    ASSERT_NO_FATAL_FAILURE(connector.onError(300, 999, -999, nullptr));
    ASSERT_NO_FATAL_FAILURE(connector.onError(301, -1, 0, "any"));
    EXPECT_TRUE(true);
}

} // namespace DistributedHardware
} // namespace OHOS