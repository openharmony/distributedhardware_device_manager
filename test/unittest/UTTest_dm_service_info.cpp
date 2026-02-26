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

#include "UTTest_dm_service_info.h"

namespace OHOS {
namespace DistributedHardware {

void ServiceSyncInfoTest::SetUp()
{
}

void ServiceSyncInfoTest::TearDown()
{
}

void ServiceSyncInfoTest::SetUpTestCase()
{
}

void ServiceSyncInfoTest::TearDownTestCase()
{
}

namespace {

/**
 * @tc.name: ServiceSyncInfoEquals_001
 * @tc.desc: Test operator== with identical ServiceSyncInfo objects
 *           Step 1: Prepare two ServiceSyncInfo objects with same values
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_001, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;
    info1.callerUserId = 200;
    info1.callerTokenId = 300;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 1001;
    info2.callerUserId = 200;
    info2.callerTokenId = 300;

    bool result = (info1 == info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ServiceSyncInfoEquals_002
 * @tc.desc: Test operator== with different pkgName
 *           Step 1: Prepare two ServiceSyncInfo objects with different pkgName
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_002, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test1";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test2";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 1001;

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ServiceSyncInfoEquals_003
 * @tc.desc: Test operator== with different localUserId
 *           Step 1: Prepare two ServiceSyncInfo objects with different localUserId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_003, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 200;
    info2.networkId = "network123";
    info2.serviceId = 1001;

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ServiceSyncInfoEquals_004
 * @tc.desc: Test operator== with different networkId
 *           Step 1: Prepare two ServiceSyncInfo objects with different networkId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_004, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network456";
    info2.serviceId = 1001;

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ServiceSyncInfoEquals_005
 * @tc.desc: Test operator== with different serviceId
 *           Step 1: Prepare two ServiceSyncInfo objects with different serviceId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_005, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 2002;

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ServiceSyncInfoEquals_006
 * @tc.desc: Test operator== ignores callerUserId and callerTokenId
 *           Step 1: Prepare two ServiceSyncInfo objects with same key fields but different callerUserId/callerTokenId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are equal (callerUserId/callerTokenId not compared)
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_006, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;
    info1.callerUserId = 200;
    info1.callerTokenId = 300;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 1001;
    info2.callerUserId = 400;
    info2.callerTokenId = 500;

    bool result = (info1 == info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ServiceSyncInfoEquals_007
 * @tc.desc: Test operator== with empty strings
 *           Step 1: Prepare two ServiceSyncInfo objects with empty pkgName and networkId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_007, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "";
    info1.localUserId = 0;
    info1.networkId = "";
    info1.serviceId = 0;

    ServiceSyncInfo info2;
    info2.pkgName = "";
    info2.localUserId = 0;
    info2.networkId = "";
    info2.serviceId = 0;

    bool result = (info1 == info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ServiceSyncInfoEquals_008
 * @tc.desc: Test operator== with self comparison
 *           Step 1: Prepare a ServiceSyncInfo object
 *           Step 2: Compare object with itself using operator==
 *           Step 3: Verify it equals itself
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoEquals_008, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info;
    info.pkgName = "com.example.test";
    info.localUserId = 100;
    info.networkId = "network123";
    info.serviceId = 1001;

    bool result = (info == info);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_001
 * @tc.desc: Test operator< with pkgName comparison
 *           Step 1: Prepare two ServiceSyncInfo objects with different pkgName
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on pkgName
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_001, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.aaa";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.bbb";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 1001;

    bool result1 = (info1 < info2);
    EXPECT_TRUE(result1);

    bool result2 = (info2 < info1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_002
 * @tc.desc: Test operator< with same pkgName but different localUserId
 *           Step 1: Prepare two ServiceSyncInfo objects with same pkgName but different localUserId
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on localUserId
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_002, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 200;
    info2.networkId = "network123";
    info2.serviceId = 1001;

    bool result1 = (info1 < info2);
    EXPECT_TRUE(result1);

    bool result2 = (info2 < info1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_003
 * @tc.desc: Test operator< with same pkgName and localUserId but different networkId
 *           Step 1: Prepare two ServiceSyncInfo objects with same pkgName and localUserId
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on networkId
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_003, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network111";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network222";
    info2.serviceId = 1001;

    bool result1 = (info1 < info2);
    EXPECT_TRUE(result1);

    bool result2 = (info2 < info1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_004
 * @tc.desc: Test operator< with same pkgName, localUserId, networkId but different serviceId
 *           Step 1: Prepare two ServiceSyncInfo objects with same first three fields
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on serviceId
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_004, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1000;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 2000;

    bool result1 = (info1 < info2);
    EXPECT_TRUE(result1);

    bool result2 = (info2 < info1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_005
 * @tc.desc: Test operator< with identical objects
 *           Step 1: Prepare two identical ServiceSyncInfo objects
 *           Step 2: Compare using operator<
 *           Step 3: Verify they are not less than each other
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_005, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 1001;

    bool result = (info1 < info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_006
 * @tc.desc: Test operator< with self comparison
 *           Step 1: Prepare a ServiceSyncInfo object
 *           Step 2: Compare object with itself using operator<
 *           Step 3: Verify object is not less than itself
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_006, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info;
    info.pkgName = "com.example.test";
    info.localUserId = 100;
    info.networkId = "network123";
    info.serviceId = 1001;

    bool result = (info < info);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_007
 * @tc.desc: Test operator< with negative localUserId
 *           Step 1: Prepare two ServiceSyncInfo objects with negative localUserId
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship with negative values
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_007, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = -100;
    info1.networkId = "network123";
    info1.serviceId = 1001;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 1001;

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ServiceSyncInfoLessThan_008
 * @tc.desc: Test operator< with negative serviceId
 *           Step 1: Prepare two ServiceSyncInfo objects with negative serviceId
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship with negative values
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoLessThan_008, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = -1000;

    ServiceSyncInfo info2;
    info2.pkgName = "com.example.test";
    info2.localUserId = 100;
    info2.networkId = "network123";
    info2.serviceId = 1000;

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ServiceSyncInfoDefaultValues_001
 * @tc.desc: Test ServiceSyncInfo default values
 *           Step 1: Create a ServiceSyncInfo object without initialization
 *           Step 2: Verify default values are correct
 *           Step 3: Check strings are empty and integers are zero
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoDefaultValues_001, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info;

    EXPECT_EQ(info.localUserId, 0);
    EXPECT_EQ(info.serviceId, 0);
    EXPECT_EQ(info.callerUserId, 0);
    EXPECT_EQ(info.callerTokenId, 0);
    EXPECT_TRUE(info.pkgName.empty());
    EXPECT_TRUE(info.networkId.empty());
}

/**
 * @tc.name: ServiceSyncInfoAssignment_001
 * @tc.desc: Test ServiceSyncInfo assignment operator
 *           Step 1: Create a ServiceSyncInfo object with values
 *           Step 2: Assign to another ServiceSyncInfo object
 *           Step 3: Verify both objects are equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoAssignment_001, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;
    info1.callerUserId = 200;
    info1.callerTokenId = 300;

    ServiceSyncInfo info2;
    info2 = info1;

    EXPECT_EQ(info1, info2);
    EXPECT_EQ(info2.pkgName, "com.example.test");
    EXPECT_EQ(info2.localUserId, 100);
    EXPECT_EQ(info2.networkId, "network123");
    EXPECT_EQ(info2.serviceId, 1001);
    EXPECT_EQ(info2.callerUserId, 200);
    EXPECT_EQ(info2.callerTokenId, 300);
}

/**
 * @tc.name: ServiceSyncInfoCopy_001
 * @tc.desc: Test ServiceSyncInfo copy constructor
 *           Step 1: Create a ServiceSyncInfo object with values
 *           Step 2: Copy to another ServiceSyncInfo object
 *           Step 3: Verify both objects are equal
 * @tc.type: FUNC
 */
HWTEST_F(ServiceSyncInfoTest, ServiceSyncInfoCopy_001, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo info1;
    info1.pkgName = "com.example.test";
    info1.localUserId = 100;
    info1.networkId = "network123";
    info1.serviceId = 1001;
    info1.callerUserId = 200;
    info1.callerTokenId = 300;

    ServiceSyncInfo info2 = info1;

    EXPECT_EQ(info1, info2);
    EXPECT_EQ(info2.pkgName, "com.example.test");
    EXPECT_EQ(info2.localUserId, 100);
    EXPECT_EQ(info2.networkId, "network123");
    EXPECT_EQ(info2.serviceId, 1001);
    EXPECT_EQ(info2.callerUserId, 200);
    EXPECT_EQ(info2.callerTokenId, 300);
}

} // namespace
} // namespace DistributedHardware
} // namespace OHOS
