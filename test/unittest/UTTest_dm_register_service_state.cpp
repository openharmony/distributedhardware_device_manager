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

#include "UTTest_dm_register_service_state.h"

namespace OHOS {
namespace DistributedHardware {

void DmRegisterServiceStateTest::SetUpTestCase()
{
}

void DmRegisterServiceStateTest::TearDownTestCase()
{
}

void DmRegisterServiceStateTest::SetUp()
{
}

void DmRegisterServiceStateTest::TearDown()
{
}

/**
 * @tc.name: OperatorEqual_001
 * @tc.desc: Test operator== with identical DmRegisterServiceState objects
 *           Step 1: Prepare two DmRegisterServiceState objects with same values
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, OperatorEqual_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state1;
    state1.userId = 100;
    state1.tokenId = 1000;
    state1.pkgName = "com.test.pkg";
    state1.serviceId = 5000;

    DmRegisterServiceState state2;
    state2.userId = 100;
    state2.tokenId = 1000;
    state2.pkgName = "com.test.pkg";
    state2.serviceId = 5000;

    bool result = (state1 == state2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorEqual_002
 * @tc.desc: Test operator== with different userId
 *           Step 1: Prepare two DmRegisterServiceState objects with different userId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, OperatorEqual_002, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state1;
    state1.userId = 100;
    state1.tokenId = 1000;
    state1.pkgName = "com.test.pkg";
    state1.serviceId = 5000;

    DmRegisterServiceState state2;
    state2.userId = 200;
    state2.tokenId = 1000;
    state2.pkgName = "com.test.pkg";
    state2.serviceId = 5000;

    bool result = (state1 == state2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_003
 * @tc.desc: Test operator== with different tokenId
 *           Step 1: Prepare two DmRegisterServiceState objects with different tokenId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, OperatorEqual_003, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state1;
    state1.userId = 100;
    state1.tokenId = 1000;
    state1.pkgName = "com.test.pkg";
    state1.serviceId = 5000;

    DmRegisterServiceState state2;
    state2.userId = 100;
    state2.tokenId = 2000;
    state2.pkgName = "com.test.pkg";
    state2.serviceId = 5000;

    bool result = (state1 == state2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_004
 * @tc.desc: Test operator== with different pkgName
 *           Step 1: Prepare two DmRegisterServiceState objects with different pkgName
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, OperatorEqual_004, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state1;
    state1.userId = 100;
    state1.tokenId = 1000;
    state1.pkgName = "com.test.pkg1";
    state1.serviceId = 5000;

    DmRegisterServiceState state2;
    state2.userId = 100;
    state2.tokenId = 1000;
    state2.pkgName = "com.test.pkg2";
    state2.serviceId = 5000;

    bool result = (state1 == state2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_005
 * @tc.desc: Test operator== with different serviceId
 *           Step 1: Prepare two DmRegisterServiceState objects with different serviceId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, OperatorEqual_005, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state1;
    state1.userId = 100;
    state1.tokenId = 1000;
    state1.pkgName = "com.test.pkg";
    state1.serviceId = 5000;

    DmRegisterServiceState state2;
    state2.userId = 100;
    state2.tokenId = 1000;
    state2.pkgName = "com.test.pkg";
    state2.serviceId = 6000;

    bool result = (state1 == state2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_006
 * @tc.desc: Test operator== with default initialized objects
 *           Step 1: Prepare two DmRegisterServiceState objects with default values
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, OperatorEqual_006, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state1;
    DmRegisterServiceState state2;

    bool result = (state1 == state2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorEqual_007
 * @tc.desc: Test operator== with same object
 *           Step 1: Create a DmRegisterServiceState object
 *           Step 2: Compare object with itself
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, OperatorEqual_007, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.userId = 100;
    state.tokenId = 1000;
    state.pkgName = "com.test.pkg";
    state.serviceId = 5000;

    bool result = (state == state);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DefaultConstructor_001
 * @tc.desc: Test default constructor with field initializers
 *           Step 1: Create default DmRegisterServiceState object
 *           Step 2: Check field default values
 *           Step 3: Verify all fields are initialized to default values
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, DefaultConstructor_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    EXPECT_EQ(state.userId, 0);
    EXPECT_EQ(state.tokenId, 0);
    EXPECT_EQ(state.pkgName, "");
    EXPECT_EQ(state.serviceId, 0);
}

/**
 * @tc.name: FieldAssignment_001
 * @tc.desc: Test assigning values to all fields
 *           Step 1: Create DmRegisterServiceState and assign values
 *           Step 2: Read back the values
 *           Step 3: Verify all values match assigned values
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, FieldAssignment_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.userId = 100;
    state.tokenId = 1000;
    state.pkgName = "com.example.test";
    state.serviceId = 5000;

    EXPECT_EQ(state.userId, 100);
    EXPECT_EQ(state.tokenId, 1000);
    EXPECT_EQ(state.pkgName, "com.example.test");
    EXPECT_EQ(state.serviceId, 5000);
}

/**
 * @tc.name: CopyAssignment_001
 * @tc.desc: Test copy assignment operator
 *           Step 1: Create and populate a DmRegisterServiceState object
 *           Step 2: Copy to another object using assignment
 *           Step 3: Verify all fields are copied correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, CopyAssignment_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state1;
    state1.userId = 100;
    state1.tokenId = 1000;
    state1.pkgName = "com.test.pkg";
    state1.serviceId = 5000;

    DmRegisterServiceState state2;
    state2 = state1;

    EXPECT_EQ(state2.userId, 100);
    EXPECT_EQ(state2.tokenId, 1000);
    EXPECT_EQ(state2.pkgName, "com.test.pkg");
    EXPECT_EQ(state2.serviceId, 5000);
}

/**
 * @tc.name: UserIdTest_001
 * @tc.desc: Test userId field with positive value
 *           Step 1: Create DmRegisterServiceState and set userId
 *           Step 2: Read back userId value
 *           Step 3: Verify value is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, UserIdTest_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.userId = 12345;
    EXPECT_EQ(state.userId, 12345);
}

/**
 * @tc.name: UserIdTest_002
 * @tc.desc: Test userId field with negative value
 *           Step 1: Create DmRegisterServiceState and set negative userId
 *           Step 2: Read back userId value
 *           Step 3: Verify negative value is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, UserIdTest_002, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.userId = -100;
    EXPECT_EQ(state.userId, -100);
}

/**
 * @tc.name: TokenIdTest_001
 * @tc.desc: Test tokenId field with large value
 *           Step 1: Create DmRegisterServiceState and set tokenId
 *           Step 2: Read back tokenId value
 *           Step 3: Verify large value is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, TokenIdTest_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.tokenId = 18446744073709551615ULL;
    EXPECT_EQ(state.tokenId, 18446744073709551615ULL);
}

/**
 * @tc.name: ServiceIdTest_001
 * @tc.desc: Test serviceId field with positive value
 *           Step 1: Create DmRegisterServiceState and set serviceId
 *           Step 2: Read back serviceId value
 *           Step 3: Verify value is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, ServiceIdTest_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.serviceId = 99999;
    EXPECT_EQ(state.serviceId, 99999);
}

/**
 * @tc.name: ServiceIdTest_002
 * @tc.desc: Test serviceId field with negative value
 *           Step 1: Create DmRegisterServiceState and set negative serviceId
 *           Step 2: Read back serviceId value
 *           Step 3: Verify negative value is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, ServiceIdTest_002, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.serviceId = -5000;
    EXPECT_EQ(state.serviceId, -5000);
}

/**
 * @tc.name: PkgNameTest_001
 * @tc.desc: Test pkgName field with package name
 *           Step 1: Create DmRegisterServiceState and set pkgName
 *           Step 2: Read back pkgName value
 *           Step 3: Verify package name is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, PkgNameTest_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.pkgName = "com.ohos.distributedhardware";
    EXPECT_EQ(state.pkgName, "com.ohos.distributedhardware");
}

/**
 * @tc.name: PkgNameTest_002
 * @tc.desc: Test pkgName field with empty string
 *           Step 1: Create DmRegisterServiceState and set empty pkgName
 *           Step 2: Read back pkgName value
 *           Step 3: Verify empty string is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, PkgNameTest_002, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.pkgName = "";
    EXPECT_EQ(state.pkgName, "");
}

/**
 * @tc.name: PkgNameTest_003
 * @tc.desc: Test pkgName field with special characters
 *           Step 1: Create DmRegisterServiceState and set pkgName with special chars
 *           Step 2: Read back pkgName value
 *           Step 3: Verify special characters are correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, PkgNameTest_003, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.pkgName = "com.test_123.app-name";
    EXPECT_EQ(state.pkgName, "com.test_123.app-name");
}

/**
 * @tc.name: MultipleFieldAssignment_001
 * @tc.desc: Test modifying multiple fields sequentially
 *           Step 1: Create DmRegisterServiceState and set all fields
 *           Step 2: Modify each field sequentially
 *           Step 3: Verify all modifications are correctly applied
 * @tc.type: FUNC
 */
HWTEST_F(DmRegisterServiceStateTest, MultipleFieldAssignment_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState state;
    state.userId = 100;
    state.tokenId = 1000;
    state.pkgName = "com.test.pkg";
    state.serviceId = 5000;

    state.userId = 200;
    EXPECT_EQ(state.userId, 200);
    EXPECT_EQ(state.tokenId, 1000);

    state.tokenId = 2000;
    EXPECT_EQ(state.tokenId, 2000);

    state.pkgName = "com.modified.pkg";
    EXPECT_EQ(state.pkgName, "com.modified.pkg");

    state.serviceId = 6000;
    EXPECT_EQ(state.serviceId, 6000);
}

} // namespace DistributedHardware
} // namespace OHOS
