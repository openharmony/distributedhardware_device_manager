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

#include "UTTest_dm_discovery_service_param.h"

namespace OHOS {
namespace DistributedHardware {

void DmDiscoveryServiceParamTest::SetUpTestCase()
{
}

void DmDiscoveryServiceParamTest::TearDownTestCase()
{
}

void DmDiscoveryServiceParamTest::SetUp()
{
}

void DmDiscoveryServiceParamTest::TearDown()
{
}

/**
 * @tc.name: OperatorEqual_001
 * @tc.desc: Test operator== with identical DmDiscoveryServiceParam objects
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with same values
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "testType";
    param1.serviceName = "testName";
    param1.serviceDisplayName = "Test Display";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_BLE;
    param1.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "testType";
    param2.serviceName = "testName";
    param2.serviceDisplayName = "Test Display";
    param2.freq = DM_LOW;
    param2.medium = SERVICE_MEDIUM_TYPE_BLE;
    param2.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    bool result = (param1 == param2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorEqual_002
 * @tc.desc: Test operator== with different serviceType
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with different serviceType
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_002, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "type1";
    param1.serviceName = "testName";
    param1.serviceDisplayName = "Test Display";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_BLE;
    param1.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "type2";
    param2.serviceName = "testName";
    param2.serviceDisplayName = "Test Display";
    param2.freq = DM_LOW;
    param2.medium = SERVICE_MEDIUM_TYPE_BLE;
    param2.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    bool result = (param1 == param2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_003
 * @tc.desc: Test operator== with different serviceName
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with different serviceName
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_003, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "testType";
    param1.serviceName = "name1";
    param1.serviceDisplayName = "Test Display";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_BLE;
    param1.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "testType";
    param2.serviceName = "name2";
    param2.serviceDisplayName = "Test Display";
    param2.freq = DM_LOW;
    param2.medium = SERVICE_MEDIUM_TYPE_BLE;
    param2.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    bool result = (param1 == param2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_004
 * @tc.desc: Test operator== with different serviceDisplayName
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with different serviceDisplayName
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_004, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "testType";
    param1.serviceName = "testName";
    param1.serviceDisplayName = "Display 1";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_BLE;
    param1.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "testType";
    param2.serviceName = "testName";
    param2.serviceDisplayName = "Display 2";
    param2.freq = DM_LOW;
    param2.medium = SERVICE_MEDIUM_TYPE_BLE;
    param2.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    bool result = (param1 == param2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_005
 * @tc.desc: Test operator== with different freq
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with different freq
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_005, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "testType";
    param1.serviceName = "testName";
    param1.serviceDisplayName = "Test Display";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_BLE;
    param1.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "testType";
    param2.serviceName = "testName";
    param2.serviceDisplayName = "Test Display";
    param2.freq = DM_HIGH;
    param2.medium = SERVICE_MEDIUM_TYPE_BLE;
    param2.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    bool result = (param1 == param2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_006
 * @tc.desc: Test operator== with different medium
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with different medium
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_006, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "testType";
    param1.serviceName = "testName";
    param1.serviceDisplayName = "Test Display";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_BLE;
    param1.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "testType";
    param2.serviceName = "testName";
    param2.serviceDisplayName = "Test Display";
    param2.freq = DM_LOW;
    param2.medium = SERVICE_MEDIUM_TYPE_MDNS;
    param2.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    bool result = (param1 == param2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_007
 * @tc.desc: Test operator== with different mode
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with different mode
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_007, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "testType";
    param1.serviceName = "testName";
    param1.serviceDisplayName = "Test Display";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_BLE;
    param1.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "testType";
    param2.serviceName = "testName";
    param2.serviceDisplayName = "Test Display";
    param2.freq = DM_LOW;
    param2.medium = SERVICE_MEDIUM_TYPE_BLE;
    param2.mode = SERVICE_PUBLISH_MODE_PASSIVE;

    bool result = (param1 == param2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_008
 * @tc.desc: Test operator== with empty strings
 *           Step 1: Prepare two DmDiscoveryServiceParam objects with empty string fields
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true when all fields are empty
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_008, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "";
    param1.serviceName = "";
    param1.serviceDisplayName = "";
    param1.freq = DM_LOW;
    param1.medium = SERVICE_MEDIUM_TYPE_AUTO;
    param1.mode = SERVICE_PUBLISH_MODE_PASSIVE;

    DmDiscoveryServiceParam param2;
    param2.serviceType = "";
    param2.serviceName = "";
    param2.serviceDisplayName = "";
    param2.freq = DM_LOW;
    param2.medium = SERVICE_MEDIUM_TYPE_AUTO;
    param2.mode = SERVICE_PUBLISH_MODE_PASSIVE;

    bool result = (param1 == param2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DefaultConstructor_001
 * @tc.desc: Test default constructor initializes string fields to empty
 *           Step 1: Create default DmDiscoveryServiceParam object
 *           Step 2: Check string field default values
 *           Step 3: Verify string fields are empty
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, DefaultConstructor_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    EXPECT_TRUE(param.serviceType.empty());
    EXPECT_TRUE(param.serviceName.empty());
    EXPECT_TRUE(param.serviceDisplayName.empty());
}

/**
 * @tc.name: DefaultConstructor_002
 * @tc.desc: Test default constructor initializes enum fields
 *           Step 1: Create default DmDiscoveryServiceParam object
 *           Step 2: Check enum field default values
 *           Step 3: Verify enum fields are initialized to default values
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, DefaultConstructor_002, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    EXPECT_EQ(param.freq, static_cast<int>(0));
    EXPECT_EQ(param.medium, static_cast<int>(0));
    EXPECT_EQ(param.mode, static_cast<int>(0));
}

/**
 * @tc.name: FieldAssignment_001
 * @tc.desc: Test assigning values to string fields
 *           Step 1: Create DmDiscoveryServiceParam and assign string values
 *           Step 2: Read back the values
 *           Step 3: Verify all values match assigned values
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, FieldAssignment_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.serviceType = "com.test.service";
    param.serviceName = "TestService";
    param.serviceDisplayName = "Test Service Display";

    EXPECT_EQ(param.serviceType, "com.test.service");
    EXPECT_EQ(param.serviceName, "TestService");
    EXPECT_EQ(param.serviceDisplayName, "Test Service Display");
}

/**
 * @tc.name: FieldAssignment_002
 * @tc.desc: Test assigning values to enum fields
 *           Step 1: Create DmDiscoveryServiceParam and assign enum values
 *           Step 2: Read back the values
 *           Step 3: Verify all values match assigned values
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, FieldAssignment_002, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.freq = DM_MID;
    param.medium = SERVICE_MEDIUM_TYPE_BLE_TRIGGER;
    param.mode = SERVICE_PUBLISH_MODE_PASSIVE;

    EXPECT_EQ(param.freq, DM_MID);
    EXPECT_EQ(param.medium, SERVICE_MEDIUM_TYPE_BLE_TRIGGER);
    EXPECT_EQ(param.mode, SERVICE_PUBLISH_MODE_PASSIVE);
}

/**
 * @tc.name: FreqEnum_001
 * @tc.desc: Test DmExchangeFreq enum values can be assigned
 *           Step 1: Create DmDiscoveryServiceParam with different DmExchangeFreq values
 *           Step 2: Assign various enum values
 *           Step 3: Verify enum values are stored correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, FreqEnum_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.freq = DM_LOW;
    EXPECT_EQ(param.freq, DM_LOW);

    param.freq = DM_MID;
    EXPECT_EQ(param.freq, DM_MID);

    param.freq = DM_HIGH;
    EXPECT_EQ(param.freq, DM_HIGH);
}

/**
 * @tc.name: MediumEnum_001
 * @tc.desc: Test DMSrvMediumType enum values can be assigned
 *           Step 1: Create DmDiscoveryServiceParam with different DMSrvMediumType values
 *           Step 2: Assign various enum values
 *           Step 3: Verify enum values are stored correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, MediumEnum_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.medium = SERVICE_MEDIUM_TYPE_AUTO;
    EXPECT_EQ(param.medium, SERVICE_MEDIUM_TYPE_AUTO);

    param.medium = SERVICE_MEDIUM_TYPE_BLE;
    EXPECT_EQ(param.medium, SERVICE_MEDIUM_TYPE_BLE);

    param.medium = SERVICE_MEDIUM_TYPE_BLE_TRIGGER;
    EXPECT_EQ(param.medium, SERVICE_MEDIUM_TYPE_BLE_TRIGGER);

    param.medium = SERVICE_MEDIUM_TYPE_MDNS;
    EXPECT_EQ(param.medium, SERVICE_MEDIUM_TYPE_MDNS);
}

/**
 * @tc.name: ModeEnum_001
 * @tc.desc: Test DMSrvDiscoveryMode enum values can be assigned
 *           Step 1: Create DmDiscoveryServiceParam with different DMSrvDiscoveryMode values
 *           Step 2: Assign various enum values
 *           Step 3: Verify enum values are stored correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, ModeEnum_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.mode = SERVICE_PUBLISH_MODE_PASSIVE;
    EXPECT_EQ(param.mode, SERVICE_PUBLISH_MODE_PASSIVE);

    param.mode = SERVICE_PUBLISH_MODE_ACTIVE;
    EXPECT_EQ(param.mode, SERVICE_PUBLISH_MODE_ACTIVE);
}

/**
 * @tc.name: CopyAssignment_001
 * @tc.desc: Test copy assignment operator
 *           Step 1: Create and populate a DmDiscoveryServiceParam object
 *           Step 2: Copy to another object using assignment
 *           Step 3: Verify all fields are copied correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, CopyAssignment_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param1;
    param1.serviceType = "copyType";
    param1.serviceName = "copyName";
    param1.serviceDisplayName = "Copy Display";
    param1.freq = DM_HIGH;
    param1.medium = SERVICE_MEDIUM_TYPE_MDNS;
    param1.mode = SERVICE_PUBLISH_MODE_PASSIVE;

    DmDiscoveryServiceParam param2;
    param2 = param1;

    EXPECT_EQ(param2.serviceType, "copyType");
    EXPECT_EQ(param2.serviceName, "copyName");
    EXPECT_EQ(param2.serviceDisplayName, "Copy Display");
    EXPECT_EQ(param2.freq, DM_HIGH);
    EXPECT_EQ(param2.medium, SERVICE_MEDIUM_TYPE_MDNS);
    EXPECT_EQ(param2.mode, SERVICE_PUBLISH_MODE_PASSIVE);
}

/**
 * @tc.name: OperatorEqual_009
 * @tc.desc: Test operator== with same object
 *           Step 1: Create a DmDiscoveryServiceParam object
 *           Step 2: Compare object with itself
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(DmDiscoveryServiceParamTest, OperatorEqual_009, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.serviceType = "testType";
    param.serviceName = "testName";
    param.serviceDisplayName = "Test Display";
    param.freq = DM_LOW;
    param.medium = SERVICE_MEDIUM_TYPE_BLE;
    param.mode = SERVICE_PUBLISH_MODE_ACTIVE;

    bool result = (param == param);
    EXPECT_TRUE(result);
}

} // namespace DistributedHardware
} // namespace OHOS
