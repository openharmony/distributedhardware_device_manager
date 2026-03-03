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

#include "UTTest_process_info.h"

namespace OHOS {
namespace DistributedHardware {

void ProcessInfoTest::SetUpTestCase()
{
}

void ProcessInfoTest::TearDownTestCase()
{
}

void ProcessInfoTest::SetUp()
{
}

void ProcessInfoTest::TearDown()
{
}

/**
 * @tc.name: OperatorEqual_001
 * @tc.desc: Test operator== with identical ProcessInfo objects
 *           Step 1: Prepare two ProcessInfo objects with same values
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorEqual_001, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2.userId = 100;
    info2.pkgName = "com.test.pkg";

    bool result = (info1 == info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorEqual_002
 * @tc.desc: Test operator== with different userId
 *           Step 1: Prepare two ProcessInfo objects with different userId
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorEqual_002, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2.userId = 200;
    info2.pkgName = "com.test.pkg";

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_003
 * @tc.desc: Test operator== with different pkgName
 *           Step 1: Prepare two ProcessInfo objects with different pkgName
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorEqual_003, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.pkg1";

    ProcessInfo info2;
    info2.userId = 100;
    info2.pkgName = "com.test.pkg2";

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_004
 * @tc.desc: Test operator== with both fields different
 *           Step 1: Prepare two ProcessInfo objects with both fields different
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is false
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorEqual_004, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.pkg1";

    ProcessInfo info2;
    info2.userId = 200;
    info2.pkgName = "com.test.pkg2";

    bool result = (info1 == info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorEqual_005
 * @tc.desc: Test operator== with default initialized objects
 *           Step 1: Prepare two ProcessInfo objects with default values
 *           Step 2: Compare using operator==
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorEqual_005, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    ProcessInfo info2;

    bool result = (info1 == info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorEqual_006
 * @tc.desc: Test operator== with same object
 *           Step 1: Create a ProcessInfo object
 *           Step 2: Compare object with itself
 *           Step 3: Verify result is true
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorEqual_006, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    info.userId = 100;
    info.pkgName = "com.test.pkg";

    bool result = (info == info);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_001
 * @tc.desc: Test operator< with smaller userId
 *           Step 1: Prepare two ProcessInfo objects with different userId
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is true when userId is less
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorLess_001, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2.userId = 200;
    info2.pkgName = "com.test.pkg";

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_002
 * @tc.desc: Test operator< with same userId and smaller pkgName
 *           Step 1: Prepare two ProcessInfo objects with same userId but different pkgName
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is true when pkgName is less
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorLess_002, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.a";

    ProcessInfo info2;
    info2.userId = 100;
    info2.pkgName = "com.test.b";

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_003
 * @tc.desc: Test operator< returns false when userId is greater
 *           Step 1: Prepare two ProcessInfo objects with greater userId
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is false when userId is greater
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorLess_003, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 200;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2.userId = 100;
    info2.pkgName = "com.test.pkg";

    bool result = (info1 < info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorLess_004
 * @tc.desc: Test operator< returns false when userId is same and pkgName is greater
 *           Step 1: Prepare two ProcessInfo objects with same userId but greater pkgName
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is false when pkgName is greater
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorLess_004, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.b";

    ProcessInfo info2;
    info2.userId = 100;
    info2.pkgName = "com.test.a";

    bool result = (info1 < info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: OperatorLess_005
 * @tc.desc: Test operator< with equal objects
 *           Step 1: Prepare two identical ProcessInfo objects
 *           Step 2: Compare using operator<
 *           Step 3: Verify result is false when objects are equal
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorLess_005, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2.userId = 100;
    info2.pkgName = "com.test.pkg";

    bool result = (info1 < info2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DefaultConstructor_001
 * @tc.desc: Test default constructor initializes fields
 *           Step 1: Create default ProcessInfo object
 *           Step 2: Check field default values
 *           Step 3: Verify fields are initialized to default values
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, DefaultConstructor_001, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    EXPECT_EQ(info.userId, 0);
    EXPECT_EQ(info.pkgName, "");
}

/**
 * @tc.name: FieldAssignment_001
 * @tc.desc: Test assigning values to all fields
 *           Step 1: Create ProcessInfo and assign values
 *           Step 2: Read back the values
 *           Step 3: Verify all values match assigned values
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, FieldAssignment_001, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    info.userId = 100;
    info.pkgName = "com.example.test";

    EXPECT_EQ(info.userId, 100);
    EXPECT_EQ(info.pkgName, "com.example.test");
}

/**
 * @tc.name: CopyAssignment_001
 * @tc.desc: Test copy assignment operator
 *           Step 1: Create and populate a ProcessInfo object
 *           Step 2: Copy to another object using assignment
 *           Step 3: Verify all fields are copied correctly
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, CopyAssignment_001, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 100;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2 = info1;

    EXPECT_EQ(info2.userId, 100);
    EXPECT_EQ(info2.pkgName, "com.test.pkg");
}

/**
 * @tc.name: UserIdTest_001
 * @tc.desc: Test userId field with positive value
 *           Step 1: Create ProcessInfo and set positive userId
 *           Step 2: Read back userId value
 *           Step 3: Verify positive value is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, UserIdTest_001, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    info.userId = 12345;
    EXPECT_EQ(info.userId, 12345);
}

/**
 * @tc.name: UserIdTest_002
 * @tc.desc: Test userId field with negative value
 *           Step 1: Create ProcessInfo and set negative userId
 *           Step 2: Read back userId value
 *           Step 3: Verify negative value is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, UserIdTest_002, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    info.userId = -100;
    EXPECT_EQ(info.userId, -100);
}

/**
 * @tc.name: PkgNameTest_001
 * @tc.desc: Test pkgName field with package name
 *           Step 1: Create ProcessInfo and set pkgName
 *           Step 2: Read back pkgName value
 *           Step 3: Verify package name is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, PkgNameTest_001, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    info.pkgName = "com.ohos.distributedhardware";
    EXPECT_EQ(info.pkgName, "com.ohos.distributedhardware");
}

/**
 * @tc.name: PkgNameTest_002
 * @tc.desc: Test pkgName field with empty string
 *           Step 1: Create ProcessInfo and set empty pkgName
 *           Step 2: Read back pkgName value
 *           Step 3: Verify empty string is correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, PkgNameTest_002, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    info.pkgName = "";
    EXPECT_EQ(info.pkgName, "");
}

/**
 * @tc.name: PkgNameTest_003
 * @tc.desc: Test pkgName field with special characters
 *           Step 1: Create ProcessInfo and set pkgName with special chars
 *           Step 2: Read back pkgName value
 *           Step 3: Verify special characters are correctly stored
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, PkgNameTest_003, testing::ext::TestSize.Level1)
{
    ProcessInfo info;
    info.pkgName = "com.test_123.app-name";
    EXPECT_EQ(info.pkgName, "com.test_123.app-name");
}

/**
 * @tc.name: OperatorLess_006
 * @tc.desc: Test operator< with negative userId values
 *           Step 1: Prepare two ProcessInfo objects with negative userId
 *           Step 2: Compare using operator<
 *           Step 3: Verify result follows negative number ordering
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorLess_006, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = -200;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2.userId = -100;
    info2.pkgName = "com.test.pkg";

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OperatorLess_007
 * @tc.desc: Test operator< with userId 0 and positive value
 *           Step 1: Prepare ProcessInfo with userId 0 and positive userId
 *           Step 2: Compare using operator<
 *           Step 3: Verify 0 is less than positive value
 * @tc.type: FUNC
 */
HWTEST_F(ProcessInfoTest, OperatorLess_007, testing::ext::TestSize.Level1)
{
    ProcessInfo info1;
    info1.userId = 0;
    info1.pkgName = "com.test.pkg";

    ProcessInfo info2;
    info2.userId = 100;
    info2.pkgName = "com.test.pkg";

    bool result = (info1 < info2);
    EXPECT_TRUE(result);
}

} // namespace DistributedHardware
} // namespace OHOS
