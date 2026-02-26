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

#include "UTTest_dm_service_info.h"

namespace OHOS {
namespace DistributedHardware {

void DmServiceInfoTest::SetUpTestCase()
{
}

void DmServiceInfoTest::TearDownTestCase()
{
}

void DmServiceInfoTest::SetUp()
{
}

void DmServiceInfoTest::TearDown()
{
}

/**
 * @tc.name: OperatorEqual_001
 * @tc.desc: Test operator== with identical DmServiceInfo objects
 *           Step 1: Prepare two DmServiceInfo objects with same values
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorEqual_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device123";
    info2.userId = 100;
    info2.serviceId = 1000;

    bool result = (info1 == info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorEqual_002
 * @tc.desc: Test operator== with different deviceId
 *           Step 1: Prepare two DmServiceInfo objects with different deviceId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorEqual_002, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device456";
    info2.userId = 100;
    info2.serviceId = 1000;

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_003
 * @tc.desc: Test operator== with different userId
 *           Step 1: Prepare two DmServiceInfo objects with different userId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorEqual_003, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device123";
    info2.userId = 200;
    info2.serviceId = 1000;

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_004
 * @tc.desc: Test operator== with different serviceId
 *           Step 1: Prepare two DmServiceInfo objects with different serviceId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorEqual_004, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device123";
    info2.userId = 100;
    info2.serviceId = 2000;

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_005
 * @tc.desc: Test operator== with empty deviceId
 *           Step 1: Prepare two DmServiceInfo objects with empty deviceId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true when other fields match
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorEqual_005, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "";
    info2.userId = 100;
    info2.serviceId = 1000;

    bool result = (info1 == info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_001
 * @tc.desc: Test operator< with smaller deviceId
 *           Step 1: Prepare two DmServiceInfo objects with different deviceId
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is true when deviceId is less
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorLess_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device456";
    info2.userId = 100;
    info2.serviceId = 1000;

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_002
 * @tc.desc: Test operator< with smaller userId
 *           Step 1: Prepare two DmServiceInfo objects with same deviceId but different userId
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is true when userId is less
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorLess_002, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device123";
    info2.userId = 200;
    info2.serviceId = 1000;

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_003
 * @tc.desc: Test operator< with smaller serviceId
 *           Step 1: Prepare two DmServiceInfo objects with same deviceId and userId
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is true when serviceId is less
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorLess_003, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device123";
    info2.userId = 100;
    info2.serviceId = 2000;

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_004
 * @tc.desc: Test operator< returns false when object is not less
 *           Step 1: Prepare two DmServiceInfo objects with larger values
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is false when object is greater
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorLess_004, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device456";
    info1.userId = 200;
    info1.serviceId = 2000;

    DmServiceInfo info2;
    info2.deviceId = "device123";
    info2.userId = 100;
    info2.serviceId = 1000;

    bool result = (info1 < info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorLess_005
 * @tc.desc: Test operator< with equal objects
 *           Step 1: Prepare two identical DmServiceInfo objects
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is false when objects are equal
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, OperatorLess_005, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;

    DmServiceInfo info2;
    info2.deviceId = "device123";
    info2.userId = 100;
    info2.serviceId = 1000;

    bool result = (info1 < info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DefaultConstructor_001
 * @tc.desc: Test default constructor initializes numeric fields
 *           Step 1: Create default DmServiceInfo object
 *           Step 2: Check numeric field default values
 *           Step 3: Verify fields are initialized to 0 or empty
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, DefaultConstructor_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo info;
    EXPECT_EQ(info.userId, 0);
    EXPECT_EQ(info.serviceId, 0);
    EXPECT_EQ(info.displayId, 0);
    EXPECT_EQ(info.publishState, 0);
    EXPECT_EQ(info.authform, 0);
    EXPECT_EQ(info.serviceOwnerTokenId, 0);
    EXPECT_EQ(info.serviceRegisterTokenId, 0);
    EXPECT_EQ(info.dataLen, 0);
    EXPECT_EQ(info.timeStamp, 0);
}

/**
 * @tc.name: DefaultConstructor_002
 * @tc.desc: Test default constructor initializes string fields to empty
 *           Step 1: Create default DmServiceInfo object
 *           Step 2: Check string field default values
 *           Step 3: Verify string fields are empty
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, DefaultConstructor_002, testing::ext::TestSize.Level1)
{
    DmServiceInfo info;
    EXPECT_TRUE(info.deviceId.empty());
    EXPECT_TRUE(info.networkId.empty());
    EXPECT_TRUE(info.serviceOwnerPkgName.empty());
    EXPECT_TRUE(info.serviceType.empty());
    EXPECT_TRUE(info.serviceName.empty());
    EXPECT_TRUE(info.serviceDisplayName.empty());
    EXPECT_TRUE(info.serviceCode.empty());
    EXPECT_TRUE(info.customData.empty());
    EXPECT_TRUE(info.description.empty());
}

/**
 * @tc.name: FieldAssignment_001
 * @tc.desc: Test assigning values to all numeric fields
 *           Step 1: Create DmServiceInfo and assign numeric values
 *           Step 2: Read back the values
 *           Step 3: Verify all values match assigned values
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, FieldAssignment_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo info;
    info.userId = 100;
    info.serviceId = 1000;
    info.displayId = 2000;
    info.publishState = 1;
    info.authform = DM_AUTH_FORM_IDENTICAL_ACCOUNT;
    info.serviceOwnerTokenId = 3000;
    info.serviceRegisterTokenId = 4000;
    info.dataLen = 5000;
    info.timeStamp = 6000;

    EXPECT_EQ(info.userId, 100);
    EXPECT_EQ(info.serviceId, 1000);
    EXPECT_EQ(info.displayId, 2000);
    EXPECT_EQ(info.publishState, 1);
    EXPECT_EQ(info.authform, DM_AUTH_FORM_IDENTICAL_ACCOUNT);
    EXPECT_EQ(info.serviceOwnerTokenId, 3000);
    EXPECT_EQ(info.serviceRegisterTokenId, 4000);
    EXPECT_EQ(info.dataLen, 5000);
    EXPECT_EQ(info.timeStamp, 6000);
}

/**
 * @tc.name: FieldAssignment_002
 * @tc.desc: Test assigning values to all string fields
 *           Step 1: Create DmServiceInfo and assign string values
 *           Step 2: Read back the values
 *           Step 3: Verify all values match assigned values
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, FieldAssignment_002, testing::ext::TestSize.Level1)
{
    DmServiceInfo info;
    info.deviceId = "test_device_id";
    info.networkId = "test_network_id";
    info.serviceOwnerPkgName = "com.test.pkg";
    info.serviceType = "test_type";
    info.serviceName = "test_service";
    info.serviceDisplayName = "Test Service";
    info.serviceCode = "test_code";
    info.customData = "test_custom_data";
    info.description = "test_description";

    EXPECT_EQ(info.deviceId, "test_device_id");
    EXPECT_EQ(info.networkId, "test_network_id");
    EXPECT_EQ(info.serviceOwnerPkgName, "com.test.pkg");
    EXPECT_EQ(info.serviceType, "test_type");
    EXPECT_EQ(info.serviceName, "test_service");
    EXPECT_EQ(info.serviceDisplayName, "Test Service");
    EXPECT_EQ(info.serviceCode, "test_code");
    EXPECT_EQ(info.customData, "test_custom_data");
    EXPECT_EQ(info.description, "test_description");
}

/**
 * @tc.name: CopyAssignment_001
 * @tc.desc: Test copy assignment operator
 *           Step 1: Create and populate a DmServiceInfo object
 *           Step 2: Copy to another object using assignment
 *           Step 3: Verify all fields are copied correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, CopyAssignment_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo info1;
    info1.deviceId = "device123";
    info1.userId = 100;
    info1.serviceId = 1000;
    info1.serviceName = "TestService";

    DmServiceInfo info2;
    info2 = info1;

    EXPECT_EQ(info2.deviceId, "device123");
    EXPECT_EQ(info2.userId, 100);
    EXPECT_EQ(info2.serviceId, 1000);
    EXPECT_EQ(info2.serviceName, "TestService");
}

/**
 * @tc.name: AuthFormEnum_001
 * @tc.desc: Test DmAuthForm enum values can be assigned
 *           Step 1: Create DmServiceInfo with different DmAuthForm values
 *           Step 2: Assign various enum values
 *           Step 3: Verify enum values are stored correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmServiceInfoTest, AuthFormEnum_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo info;
    info.authform = DM_AUTH_FORM_IDENTICAL_ACCOUNT;
    EXPECT_EQ(info.authform, DM_AUTH_FORM_IDENTICAL_ACCOUNT);

    info.authform = DM_AUTH_FORM_ACROSS_ACCOUNT;
    EXPECT_EQ(info.authform, DM_AUTH_FORM_ACROSS_ACCOUNT);
}

} // namespace DistributedHardware
} // namespace OHOS
