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

/**
 * @tc.name: IsUint64_001
 * @tc.desc: Test IsUint64 with valid uint64 value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsUint64_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    uint64_t value = 1234567890ULL;
    jsonObj["key"] = value;
    bool result = IsUint64(jsonObj, "key");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsArray_002
 * @tc.desc: Test IsArray with non-array value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsArray_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = "test";
    bool result = IsArray(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsBool_001
 * @tc.desc: Test IsBool with true value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsBool_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = true;
    bool result = IsBool(jsonObj, "key");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsBool_002
 * @tc.desc: Test IsBool with false value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsBool_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = false;
    bool result = IsBool(jsonObj, "key");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsBool_003
 * @tc.desc: Test IsBool with non-boolean value
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsBool_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["key"] = "true";
    bool result = IsBool(jsonObj, "key");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ConvertMapToJsonString_001
 * @tc.desc: Test ConvertMapToJsonString with empty map
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ConvertMapToJsonString_001, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    std::string result = ConvertMapToJsonString(paramMap);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: ConvertMapToJsonString_002
 * @tc.desc: Test ConvertMapToJsonString with valid map
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ConvertMapToJsonString_002, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    paramMap["key1"] = "value1";
    paramMap["key2"] = "value2";
    std::string result = ConvertMapToJsonString(paramMap);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("key1"), std::string::npos);
    EXPECT_NE(result.find("value1"), std::string::npos);
}

/**
 * @tc.name: ConvertMapToJsonString_003
 * @tc.desc: Test ConvertMapToJsonString with oversized map
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ConvertMapToJsonString_003, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    for (int i = 0; i < 1001; i++) {  // MAX_MAP_LEN is 1000
        paramMap["key" + std::to_string(i)] = "value" + std::to_string(i);
    }
    std::string result = ConvertMapToJsonString(paramMap);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: ParseMapFromJsonString_001
 * @tc.desc: Test ParseMapFromJsonString with empty string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ParseMapFromJsonString_001, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    std::string jsonStr = "";
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_TRUE(paramMap.empty());
}

/**
 * @tc.name: ParseMapFromJsonString_002
 * @tc.desc: Test ParseMapFromJsonString with valid JSON string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ParseMapFromJsonString_002, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    std::string jsonStr = "{\"key1\":\"value1\",\"key2\":\"value2\"}";
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_EQ(paramMap.size(), 2);
    EXPECT_EQ(paramMap["key1"], "value1");
    EXPECT_EQ(paramMap["key2"], "value2");
}

/**
 * @tc.name: ParseMapFromJsonString_003
 * @tc.desc: Test ParseMapFromJsonString with invalid JSON string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ParseMapFromJsonString_003, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    std::string jsonStr = "invalid json";
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_TRUE(paramMap.empty());
}

/**
 * @tc.name: IsInvalidPeerTargetId_001
 * @tc.desc: Test IsInvalidPeerTargetId with all empty fields
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsInvalidPeerTargetId_001, testing::ext::TestSize.Level1)
{
    PeerTargetId targetId;
    targetId.deviceId = "";
    targetId.brMac = "";
    targetId.bleMac = "";
    targetId.wifiIp = "";
    targetId.serviceId = 0;
    bool result = IsInvalidPeerTargetId(targetId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsInvalidPeerTargetId_002
 * @tc.desc: Test IsInvalidPeerTargetId with deviceId set
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsInvalidPeerTargetId_002, testing::ext::TestSize.Level1)
{
    PeerTargetId targetId;
    targetId.deviceId = "test-device-id";
    targetId.brMac = "";
    targetId.bleMac = "";
    targetId.wifiIp = "";
    targetId.serviceId = 0;
    bool result = IsInvalidPeerTargetId(targetId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsInvalidPeerTargetId_003
 * @tc.desc: Test IsInvalidPeerTargetId with serviceId set
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, IsInvalidPeerTargetId_003, testing::ext::TestSize.Level1)
{
    PeerTargetId targetId;
    targetId.deviceId = "";
    targetId.brMac = "";
    targetId.bleMac = "";
    targetId.wifiIp = "";
    targetId.serviceId = 123;
    bool result = IsInvalidPeerTargetId(targetId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ConvertCharArray2String_001
 * @tc.desc: Test ConvertCharArray2String with null pointer
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ConvertCharArray2String_001, testing::ext::TestSize.Level1)
{
    std::string result = ConvertCharArray2String(nullptr, 10);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: ConvertCharArray2String_002
 * @tc.desc: Test ConvertCharArray2String with zero length
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ConvertCharArray2String_002, testing::ext::TestSize.Level1)
{
    char testData[] = "test";
    std::string result = ConvertCharArray2String(testData, 0);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: ConvertCharArray2String_003
 * @tc.desc: Test ConvertCharArray2String with valid input
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ConvertCharArray2String_003, testing::ext::TestSize.Level1)
{
    char testData[] = "testData";
    std::string result = ConvertCharArray2String(testData, 8);
    EXPECT_EQ(result, "testData");
}

/**
 * @tc.name: ConvertCharArray2String_004
 * @tc.desc: Test ConvertCharArray2String with oversized length
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, ConvertCharArray2String_004, testing::ext::TestSize.Level1)
{
    char testData[] = "test";
    std::string result = ConvertCharArray2String(testData, 50 * 1024 * 1024);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: StringToInt_001
 * @tc.desc: Test StringToInt with empty string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, StringToInt_001, testing::ext::TestSize.Level1)
{
    std::string str = "";
    int32_t result = StringToInt(str, 10);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: StringToInt_002
 * @tc.desc: Test StringToInt with valid decimal string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, StringToInt_002, testing::ext::TestSize.Level1)
{
    std::string str = "12345";
    int32_t result = StringToInt(str, 10);
    EXPECT_EQ(result, 12345);
}

/**
 * @tc.name: StringToInt_003
 * @tc.desc: Test StringToInt with hexadecimal string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, StringToInt_003, testing::ext::TestSize.Level1)
{
    std::string str = "FF";
    int32_t result = StringToInt(str, 16);
    EXPECT_EQ(result, 255);
}

/**
 * @tc.name: StringToInt_004
 * @tc.desc: Test StringToInt with invalid string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, StringToInt_004, testing::ext::TestSize.Level1)
{
    std::string str = "abc123";
    int32_t result = StringToInt(str, 10);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: StringToInt64_001
 * @tc.desc: Test StringToInt64 with empty string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, StringToInt64_001, testing::ext::TestSize.Level1)
{
    std::string str = "";
    int64_t result = StringToInt64(str, 10);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: StringToInt64_002
 * @tc.desc: Test StringToInt64 with valid string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, StringToInt64_002, testing::ext::TestSize.Level1)
{
    std::string str = "123456789012";
    int64_t result = StringToInt64(str, 10);
    EXPECT_EQ(result, 123456789012LL);
}

/**
 * @tc.name: StringToInt64_003
 * @tc.desc: Test StringToInt64 with invalid string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, StringToInt64_003, testing::ext::TestSize.Level1)
{
    std::string str = "invalid";
    int64_t result = StringToInt64(str, 10);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: VersionSplitToInt_001
 * @tc.desc: Test VersionSplitToInt with standard version string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, VersionSplitToInt_001, testing::ext::TestSize.Level1)
{
    std::string str = "1.2.3";
    std::vector<int32_t> numVec;
    VersionSplitToInt(str, '.', numVec);
    EXPECT_EQ(numVec.size(), 3);
    EXPECT_EQ(numVec[0], 1);
    EXPECT_EQ(numVec[1], 2);
    EXPECT_EQ(numVec[2], 3);
}

/**
 * @tc.name: VersionSplitToInt_002
 * @tc.desc: Test VersionSplitToInt with empty string
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, VersionSplitToInt_002, testing::ext::TestSize.Level1)
{
    std::string str = "";
    std::vector<int32_t> numVec;
    VersionSplitToInt(str, '.', numVec);
    EXPECT_TRUE(numVec.empty());
}

/**
 * @tc.name: CompareVecNum_001
 * @tc.desc: Test CompareVecNum with first vector greater
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, CompareVecNum_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> srcVec = {2, 0};
    std::vector<int32_t> sinkVec = {1, 9};
    bool result = CompareVecNum(srcVec, sinkVec);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CompareVecNum_002
 * @tc.desc: Test CompareVecNum with first vector smaller
 * @tc.type: FUNC
 */
HWTEST_F(DmAnonymousTwoTest, CompareVecNum_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> srcVec = {1, 0};
    std::vector<int32_t> sinkVec = {2, 9};
    bool result = CompareVecNum(srcVec, sinkVec);
    EXPECT_FALSE(result);
}
}
} // namespace DistributedHardware
} // namespace OHOS