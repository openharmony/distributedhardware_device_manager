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

#include "UTTest_dm_notify_key.h"

namespace OHOS {
namespace DistributedHardware {

void DmNotifyKeyTest::SetUp()
{
}

void DmNotifyKeyTest::TearDown()
{
}

void DmNotifyKeyTest::SetUpTestCase()
{
}

void DmNotifyKeyTest::TearDownTestCase()
{
}

namespace {

/**
 * @tc.name: DmNotifyKeyEquals_001
 * @tc.desc: Test operator== with identical DmNotifyKey objects
 *           Step 1: Prepare two DmNotifyKey objects with same values
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_001, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-12345";

    bool result = (key1 == key2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DmNotifyKeyEquals_002
 * @tc.desc: Test operator== with different processUserId
 *           Step 1: Prepare two DmNotifyKey objects with different processUserId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_002, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 101;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-12345";

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DmNotifyKeyEquals_003
 * @tc.desc: Test operator== with different processPkgName
 *           Step 1: Prepare two DmNotifyKey objects with different processPkgName
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_003, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test1";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test2";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-12345";

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DmNotifyKeyEquals_004
 * @tc.desc: Test operator== with different notifyUserId
 *           Step 1: Prepare two DmNotifyKey objects with different notifyUserId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_004, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 201;
    key2.udid = "test-udid-12345";

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DmNotifyKeyEquals_005
 * @tc.desc: Test operator== with different udid
 *           Step 1: Prepare two DmNotifyKey objects with different udid
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_005, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-11111";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-22222";

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DmNotifyKeyEquals_006
 * @tc.desc: Test operator== with empty strings
 *           Step 1: Prepare two DmNotifyKey objects with empty processPkgName and udid
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_006, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 0;
    key1.processPkgName = "";
    key1.notifyUserId = 0;
    key1.udid = "";

    DmNotifyKey key2;
    key2.processUserId = 0;
    key2.processPkgName = "";
    key2.notifyUserId = 0;
    key2.udid = "";

    bool result = (key1 == key2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DmNotifyKeyEquals_007
 * @tc.desc: Test operator== with self comparison
 *           Step 1: Prepare a DmNotifyKey object
 *           Step 2: Compare object with itself using operator==
 *           Step 3: Verify it equals itself
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_007, testing::ext::TestSize.Level1)
{
    DmNotifyKey key;
    key.processUserId = 100;
    key.processPkgName = "com.example.test";
    key.notifyUserId = 200;
    key.udid = "test-udid-12345";

    bool result = (key == key);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DmNotifyKeyEquals_008
 * @tc.desc: Test operator== with different processUserId and same other fields
 *           Step 1: Prepare two DmNotifyKey objects with different processUserId
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyEquals_008, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = -100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-12345";

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DmNotifyKeyLessThan_001
 * @tc.desc: Test operator< with processUserId comparison
 *           Step 1: Prepare two DmNotifyKey objects with different processUserId
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on processUserId
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_001, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 200;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 300;
    key2.udid = "test-udid-67890";

    bool result1 = (key1 < key2);
    EXPECT_TRUE(result1);

    bool result2 = (key2 < key1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: DmNotifyKeyLessThan_002
 * @tc.desc: Test operator< with same processUserId but different processPkgName
 *           Step 1: Prepare two DmNotifyKey objects with same processUserId but different processPkgName
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on processPkgName
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_002, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "aaa.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "bbb.example.test";
    key2.notifyUserId = 300;
    key2.udid = "test-udid-67890";

    bool result1 = (key1 < key2);
    EXPECT_TRUE(result1);

    bool result2 = (key2 < key1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: DmNotifyKeyLessThan_003
 * @tc.desc: Test operator< with same processUserId and processPkgName but different notifyUserId
 *           Step 1: Prepare two DmNotifyKey objects with same first two fields
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on notifyUserId
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_003, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 300;
    key2.udid = "test-udid-67890";

    bool result1 = (key1 < key2);
    EXPECT_TRUE(result1);

    bool result2 = (key2 < key1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: DmNotifyKeyLessThan_004
 * @tc.desc: Test operator< with same processUserId, processPkgName, notifyUserId but different udid
 *           Step 1: Prepare two DmNotifyKey objects with same first three fields
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship based on udid
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_004, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "aaa-udid";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "bbb-udid";

    bool result1 = (key1 < key2);
    EXPECT_TRUE(result1);

    bool result2 = (key2 < key1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: DmNotifyKeyLessThan_005
 * @tc.desc: Test operator< with identical objects
 *           Step 1: Prepare two identical DmNotifyKey objects
 *           Step 2: Compare using operator<
 *           Step 3: Verify they are not less than each other
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_005, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-12345";

    bool result = (key1 < key2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DmNotifyKeyLessThan_006
 * @tc.desc: Test operator< with self comparison
 *           Step 1: Prepare a DmNotifyKey object
 *           Step 2: Compare object with itself using operator<
 *           Step 3: Verify object is not less than itself
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_006, testing::ext::TestSize.Level1)
{
    DmNotifyKey key;
    key.processUserId = 100;
    key.processPkgName = "com.example.test";
    key.notifyUserId = 200;
    key.udid = "test-udid-12345";

    bool result = (key < key);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DmNotifyKeyLessThan_007
 * @tc.desc: Test operator< with negative processUserId
 *           Step 1: Prepare two DmNotifyKey objects with negative processUserId
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship with negative values
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_007, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = -200;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = -100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-12345";

    bool result = (key1 < key2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DmNotifyKeyLessThan_008
 * @tc.desc: Test operator< with negative notifyUserId
 *           Step 1: Prepare two DmNotifyKey objects with negative notifyUserId
 *           Step 2: Compare using operator<
 *           Step 3: Verify correct less than relationship with negative values
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyLessThan_008, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = -300;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = -200;
    key2.udid = "test-udid-12345";

    bool result = (key1 < key2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DmNotifyKeyDefaultValues_001
 * @tc.desc: Test DmNotifyKey default values
 *           Step 1: Create a DmNotifyKey object without initialization
 *           Step 2: Verify default values are correct
 *           Step 3: Check integers are zero and strings are empty
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyDefaultValues_001, testing::ext::TestSize.Level1)
{
    DmNotifyKey key;

    EXPECT_EQ(key.processUserId, 0);
    EXPECT_EQ(key.notifyUserId, 0);
    EXPECT_TRUE(key.processPkgName.empty());
    EXPECT_TRUE(key.udid.empty());
}

/**
 * @tc.name: DmNotifyKeyAssignment_001
 * @tc.desc: Test DmNotifyKey assignment operator
 *           Step 1: Create a DmNotifyKey object with values
 *           Step 2: Assign to another DmNotifyKey object
 *           Step 3: Verify both objects are equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyAssignment_001, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2 = key1;

    EXPECT_EQ(key1, key2);
    EXPECT_EQ(key2.processUserId, 100);
    EXPECT_EQ(key2.processPkgName, "com.example.test");
    EXPECT_EQ(key2.notifyUserId, 200);
    EXPECT_EQ(key2.udid, "test-udid-12345");
}

/**
 * @tc.name: DmNotifyKeyCopy_001
 * @tc.desc: Test DmNotifyKey copy constructor
 *           Step 1: Create a DmNotifyKey object with values
 *           Step 2: Copy to another DmNotifyKey object
 *           Step 3: Verify both objects are equal
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyCopy_001, testing::ext::TestSize.Level1)
{
    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2 = key1;

    EXPECT_EQ(key1, key2);
    EXPECT_EQ(key2.processUserId, 100);
    EXPECT_EQ(key2.processPkgName, "com.example.test");
    EXPECT_EQ(key2.notifyUserId, 200);
    EXPECT_EQ(key2.udid, "test-udid-12345");
}

/**
 * @tc.name: DmNotifyKeyInMap_001
 * @tc.desc: Test DmNotifyKey usage in std::map
 *           Step 1: Create a std::map with DmNotifyKey as key
 *           Step 2: Insert multiple DmNotifyKey objects
 *           Step 3: Verify map operations work correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyInMap_001, testing::ext::TestSize.Level1)
{
    std::map<DmNotifyKey, int32_t> keyMap;

    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test1";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-11111";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test2";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-22222";

    keyMap[key1] = 111;
    keyMap[key2] = 222;

    EXPECT_EQ(keyMap.size(), 2);
    EXPECT_EQ(keyMap[key1], 111);
    EXPECT_EQ(keyMap[key2], 222);
}

/**
 * @tc.name: DmNotifyKeyInSet_001
 * @tc.desc: Test DmNotifyKey usage in std::set
 *           Step 1: Create a std::set with DmNotifyKey
 *           Step 2: Insert multiple DmNotifyKey objects
 *           Step 3: Verify set operations work correctly
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyInSet_001, testing::ext::TestSize.Level1)
{
    std::set<DmNotifyKey> keySet;

    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 200;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 300;
    key2.udid = "test-udid-67890";

    keySet.insert(key1);
    keySet.insert(key2);

    EXPECT_EQ(keySet.size(), 2);
    EXPECT_TRUE(keySet.find(key1) != keySet.end());
    EXPECT_TRUE(keySet.find(key2) != keySet.end());
}

/**
 * @tc.name: DmNotifyKeyInSet_002
 * @tc.desc: Test DmNotifyKey uniqueness in std::set
 *           Step 1: Create a std::set with DmNotifyKey
 *           Step 2: Insert duplicate keys
 *           Step 3: Verify set only stores unique keys
 * @tc.type: FUNC
 */
HWTEST_F(DmNotifyKeyTest, DmNotifyKeyInSet_002, testing::ext::TestSize.Level1)
{
    std::set<DmNotifyKey> keySet;

    DmNotifyKey key1;
    key1.processUserId = 100;
    key1.processPkgName = "com.example.test";
    key1.notifyUserId = 200;
    key1.udid = "test-udid-12345";

    DmNotifyKey key2;
    key2.processUserId = 100;
    key2.processPkgName = "com.example.test";
    key2.notifyUserId = 200;
    key2.udid = "test-udid-12345";

    keySet.insert(key1);
    keySet.insert(key2);

    EXPECT_EQ(keySet.size(), 1);
}

} // namespace
} // namespace DistributedHardware
} // namespace OHOS
