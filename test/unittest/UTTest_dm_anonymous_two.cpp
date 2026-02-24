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

#include "dm_anonymous.h"
#include "dm_device_info.h"
#include "json_object.h"
#include "gtest/gtest.h"
#include <climits>
#include <cerrno>
#include "UTTest_dm_anonymous_two.h"

namespace OHOS {
namespace DistributedHardware {

void DmAnonymousTwoTest::SetUp()
{
}

void DmAnonymousTwoTest::TearDown()
{
}

void DmAnonymousTwoTest::SetUpTestCase()
{
}

void DmAnonymousTwoTest::TearDownTestCase()
{
}
namespace {

/**
 * @tc.name: GetAnonyString_001
 * @tc.desc: Test GetAnonyString with short string (length < 3)
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyString_001, testing::ext::TestSize.Level1)
{
    std::string input = "ab";
    std::string result = GetAnonyString(input);
    EXPECT_EQ(result, "******");
}

/**
 * @tc.name: GetAnonyString_002
 * @tc.desc: Test GetAnonyString with short string (length <= 20)
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyString_002, testing::ext::TestSize.Level1)
{
    std::string input = "abcde";
    std::string result = GetAnonyString(input);
    EXPECT_EQ(result, "a******e");
}

/**
 * @tc.name: GetAnonyString_003
 * @tc.desc: Test GetAnonyString with long string (length > 20)
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyString_003, testing::ext::TestSize.Level1)
{
    std::string input = "abcdefghijklmnopqrstuvwxyz";
    std::string result = GetAnonyString(input);
    EXPECT_EQ(result, "abcd******wxyz");
}

/**
 * @tc.name: GetAnonyString_004
 * @tc.desc: Test GetAnonyString with exactly 20 chars
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyString_004, testing::ext::TestSize.Level1)
{
    std::string input = "123456789012345678901";
    std::string result = GetAnonyString(input);
    EXPECT_EQ(result, "1234******8901");
}

/**
 * @tc.name: GetAnonyStringList_001
 * @tc.desc: Test GetAnonyStringList with empty list
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyStringList_001, testing::ext::TestSize.Level1)
{
    std::vector<std::string> values;
    std::string result = GetAnonyStringList(values);
    EXPECT_EQ(result, "[  ]");
}

/**
 * @tc.name: GetAnonyStringList_002
 * @tc.desc: Test GetAnonyStringList with multiple values
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyStringList_002, testing::ext::TestSize.Level1)
{
    std::vector<std::string> values = {"abc123", "def456", "ghi789"};
    std::string result = GetAnonyStringList(values);
    EXPECT_EQ(result, "[ a******3, d******6, g******9 ]");
}

/**
 * @tc.name: GetAnonyStringList_003
 * @tc.desc: Test GetAnonyStringList with single value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyStringList_003, testing::ext::TestSize.Level1)
{
    std::vector<std::string> values = {"test123"};
    std::string result = GetAnonyStringList(values);
    EXPECT_EQ(result, "[ t******3 ]");
}

/**
 * @tc.name: GetAnonyInt32_001
 * @tc.desc: Test GetAnonyInt32 with single digit
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt32_001, testing::ext::TestSize.Level1)
{
    int32_t value = 5;
    std::string result = GetAnonyInt32(value);
    EXPECT_EQ(result, "*");
}

/**
 * @tc.name: GetAnonyInt32_002
 * @tc.desc: Test GetAnonyInt32 with two digits
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt32_002, testing::ext::TestSize.Level1)
{
    int32_t value = 12;
    std::string result = GetAnonyInt32(value);
    EXPECT_EQ(result, "12");
}

/**
 * @tc.name: GetAnonyInt32_003
 * @tc.desc: Test GetAnonyInt32 with multiple digits
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt32_003, testing::ext::TestSize.Level1)
{
    int32_t value = 12345;
    std::string result = GetAnonyInt32(value);
    EXPECT_EQ(result, "1***5");
}

/**
 * @tc.name: GetAnonyInt32_004
 * @tc.desc: Test GetAnonyInt32 with negative number
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt32_004, testing::ext::TestSize.Level1)
{
    int32_t value = -123;
    std::string result = GetAnonyInt32(value);
    EXPECT_EQ(result, "-**3");
}

/**
 * @tc.name: GetAnonyInt64_001
 * @tc.desc: Test GetAnonyInt64 with single digit
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt64_001, testing::ext::TestSize.Level1)
{
    int64_t value = 9;
    std::string result = GetAnonyInt64(value);
    EXPECT_EQ(result, "*");
}

/**
 * @tc.name: GetAnonyInt64_002
 * @tc.desc: Test GetAnonyInt64 with two digits
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt64_002, testing::ext::TestSize.Level1)
{
    int64_t value = 87;
    std::string result = GetAnonyInt64(value);
    EXPECT_EQ(result, "8*");
}

/**
 * @tc.name: GetAnonyInt64_003
 * @tc.desc: Test GetAnonyInt64 with multiple digits
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt64_003, testing::ext::TestSize.Level1)
{
    int64_t value = 123456789;
    std::string result = GetAnonyInt64(value);
    EXPECT_EQ(result, "1*******9");
}

/**
 * @tc.name: GetAnonyUint64_001
 * @tc.desc: Test GetAnonyUint64 with single digit
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyUint64_001, testing::ext::TestSize.Level1)
{
    uint64_t value = 7;
    std::string result = GetAnonyUint64(value);
    EXPECT_EQ(result, "*");
}

/**
 * @tc.name: GetAnonyUint64_002
 * @tc.desc: Test GetAnonyUint64 with two digits
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyUint64_002, testing::ext::TestSize.Level1)
{
    uint64_t value = 65;
    std::string result = GetAnonyUint64(value);
    EXPECT_EQ(result, "6*");
}

/**
 * @tc.name: GetAnonyUint64_003
 * @tc.desc: Test GetAnonyUint64 with multiple digits
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyUint64_003, testing::ext::TestSize.Level1)
{
    uint64_t value = 9876543210;
    std::string result = GetAnonyUint64(value);
    EXPECT_EQ(result, "9********0");
}

/**
 * @tc.name: GetAnonyInt32List_001
 * @tc.desc: Test GetAnonyInt32List with empty list
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt32List_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> values;
    std::string result = GetAnonyInt32List(values);
    EXPECT_EQ(result, "[  ]");
}

/**
 * @tc.name: GetAnonyInt32List_002
 * @tc.desc: Test GetAnonyInt32List with multiple values
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, GetAnonyInt32List_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> values = {123, 456, 789};
    std::string result = GetAnonyInt32List(values);
    EXPECT_EQ(result, "[ 1*3, 4*6, 7*9 ]");
}

/**
 * @tc.name: IsNumberString_001
 * @tc.desc: Test IsNumberString with empty string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsNumberString_001, testing::ext::TestSize.Level1)
{
    std::string input = "";
    bool result = IsNumberString(input);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsNumberString_002
 * @tc.desc: Test IsNumberString with valid number string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsNumberString_002, testing::ext::TestSize.Level1)
{
    std::string input = "12345";
    bool result = IsNumberString(input);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsNumberString_003
 * @tc.desc: Test IsNumberString with invalid characters
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsNumberString_003, testing::ext::TestSize.Level1)
{
    std::string input = "12a45";
    bool result = IsNumberString(input);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsNumberString_004
 * @tc.desc: Test IsNumberString with string too long
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsNumberString_004, testing::ext::TestSize.Level1)
{
    std::string input(21, '1');  // MAX_INT_LEN is 20
    bool result = IsNumberString(input);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsString_001
 * @tc.desc: Test IsString with valid string value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsString_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = "testValue";
    bool result = IsString(jsonObj, "key");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsString_002
 * @tc.desc: Test IsString with non-existent key
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsString_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = "testValue";
    bool result = IsString(jsonObj, "nonExistentKey");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsString_003
 * @tc.desc: Test IsString with non-string value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsString_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = 123;
    bool result = IsString(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsUint16_001
 * @tc.desc: Test IsUint16 with valid uint16 value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsUint16_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = 12345;
    bool result = IsUint16(jsonObj, "key");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsUint16_002
 * @tc.desc: Test IsUint16 with negative value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsUint16_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = -1;
    bool result = IsUint16(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsUint16_003
 * @tc.desc: Test IsUint16 with value exceeding UINT16_MAX
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsUint16_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = 65536;  // UINT16_MAX + 1
    bool result = IsUint16(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsInt32_001
 * @tc.desc: Test IsInt32 with valid int32 value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsInt32_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = 123456;
    bool result = IsInt32(jsonObj, "key");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsInt32_002
 * @tc.desc: Test IsInt32 with value below INT32_MIN
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsInt32_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    int64_t value = static_cast<int64_t>(INT32_MIN) - 1;
    jsonObj["key"] = value;
    bool result = IsInt32(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsInt32_003
 * @tc.desc: Test IsInt32 with value above INT32_MAX
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsInt32_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    int64_t value = static_cast<int64_t>(INT32_MAX) + 1;
    jsonObj["key"] = value;
    bool result = IsInt32(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsUint32_001
 * @tc.desc: Test IsUint32 with valid uint32 value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsUint32_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = 123456;
    bool result = IsUint32(jsonObj, "key");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsUint32_002
 * @tc.desc: Test IsUint32 with negative value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsUint32_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = -1;
    bool result = IsUint32(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsUint32_003
 * @tc.desc: Test IsUint32 with value exceeding UINT32_MAX
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsUint32_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    uint64_t value = static_cast<uint64_t>(UINT32_MAX) + 1;
    jsonObj["key"] = value;
    bool result = IsUint32(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsInt64_001
 * @tc.desc: Test IsInt64 with valid int64 value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsInt64_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    int64_t value = 1234567890LL;
    jsonObj["key"] = value;
    bool result = IsInt64(jsonObj, "key");
    EXPECT_TRUE(result);
}
}
} // namespace DistributedHardware
} // namespace OHOS