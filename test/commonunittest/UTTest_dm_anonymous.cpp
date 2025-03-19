/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UTTest_dm_anonymous.h"
#include <iterator>

namespace OHOS {
namespace DistributedHardware {
constexpr const char* FIELD_CREDENTIAL_DATA = "credentialData";
void DmAnonymousTest::SetUp()
{
}
void DmAnonymousTest::TearDown()
{
}
void DmAnonymousTest::SetUpTestCase()
{
}
void DmAnonymousTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: GetAnonyString_001
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyString_001, testing::ext::TestSize.Level1)
{
    const std::string value = "valueTest";
    std::string str = GetAnonyString(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 8);
}

/**
 * @tc.name: GetAnonyString_002
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyString_002, testing::ext::TestSize.Level1)
{
    const std::string value = "va";
    std::string str = GetAnonyString(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 6);
}

/**
 * @tc.name: GetAnonyString_003
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyString_003, testing::ext::TestSize.Level1)
{
    const std::string value = "ohos.distributedhardware.devicemanager.resident";
    std::string str = GetAnonyString(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 14);
}

/**
 * @tc.name: GetAnonyInt32_001
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyInt32_001, testing::ext::TestSize.Level1)
{
    const int32_t value = 1;
    std::string str = GetAnonyInt32(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: GetAnonyInt32_002
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyInt32_002, testing::ext::TestSize.Level1)
{
    const int32_t value = 12;
    std::string str = GetAnonyInt32(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 2);
}

/**
 * @tc.name: GetAnonyInt32_003
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyInt32_003, testing::ext::TestSize.Level1)
{
    const int32_t value = 123456;
    std::string str = GetAnonyInt32(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 6);
}

/**
 * @tc.name: IsNumberString_001
 * @tc.desc: Return false if the string is not a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsNumberString_001, testing::ext::TestSize.Level1)
{
    const std::string inputString = "";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsNumberString_002
 * @tc.desc: Return false if the string is not a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsNumberString_002, testing::ext::TestSize.Level1)
{
    const std::string inputString = "123inputstring";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsNumberString_003
 * @tc.desc: Return true if the string is a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsNumberString_003, testing::ext::TestSize.Level1)
{
    const std::string inputString = "25633981";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetErrorString_001
 * @tc.desc: Return true if the string is a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetErrorString_001, testing::ext::TestSize.Level1)
{
    int failedReason = 96929744;
    std::string errorMessage = "dm process execution failed...";
    std::string ret = GetErrorString(failedReason);
    EXPECT_EQ(ret, errorMessage);
}

/**
 * @tc.name: IsString_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsString_001, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "pinToken" : "IsString"
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsString(jsonObj, PIN_TOKEN);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsString_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsString_002, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "pinToken" : 123
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsString(jsonObj, PIN_TOKEN);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsInt32_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt32_001, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "AUTHTYPE" : 369
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsInt32(jsonObj, TAG_AUTH_TYPE);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsInt32_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt32_002, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "AUTHTYPE" : "authtypeTest"
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsInt32(jsonObj, TAG_AUTH_TYPE);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsUint32_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsUint32_001, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "AUTHTYPE" : "authtypeTest"
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsUint32(jsonObj, TAG_AUTH_TYPE);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsInt64_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt64_001, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "REQUESTID" : 789
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsInt64(jsonObj, TAG_REQUEST_ID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsInt64_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt64_002, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "REQUESTID" : "requestidTest"
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsInt64(jsonObj, TAG_REQUEST_ID);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsArray_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsArray_001, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsArray(jsonObj, FIELD_CREDENTIAL_DATA);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsArray_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsArray_002, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "authType" : 1,
        "userId" : "123",
        "credentialData" : "credentialDataTest"
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsArray(jsonObj, FIELD_CREDENTIAL_DATA);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsBool_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsBool_001, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "CRYPTOSUPPORT" : false,
        "userId" : "123",
        "credentialData" : "credentialDataTest"
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsBool(jsonObj, TAG_CRYPTO_SUPPORT);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsBool_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsBool_002, testing::ext::TestSize.Level1)
{
    std::string str = R"(
    {
        "CRYPTOSUPPORT" : "cryptosupportTest",
        "userId" : "123",
        "credentialData" : "credentialDataTest"
    }
    )";
    JsonObject jsonObj(str);
    bool ret = IsBool(jsonObj, TAG_CRYPTO_SUPPORT);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ConvertCharArray2String_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, ConvertCharArray2String_001, testing::ext::TestSize.Level1)
{
    constexpr uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024;
    char *srcData = nullptr;
    uint32_t srcLen = 0;
    std::string ret = ConvertCharArray2String(srcData, srcLen);
    EXPECT_EQ(ret, "");
    ret = ConvertCharArray2String(srcData, MAX_MESSAGE_LEN + 1);
    EXPECT_EQ(ret, "");
    char srcData2[20] = {"1234"};
    ret = ConvertCharArray2String(srcData2, MAX_MESSAGE_LEN + 1);
    EXPECT_EQ(ret, "");
    uint32_t srcLen2 = 20;
    ret = ConvertCharArray2String(srcData2, srcLen);
    EXPECT_EQ(ret, "");
    ret = ConvertCharArray2String(srcData, srcLen2);
    EXPECT_EQ(ret, "");
    ret = ConvertCharArray2String(srcData2, srcLen2);
    EXPECT_EQ(ret, "1234");
}

HWTEST_F(DmAnonymousTest, StringToInt_001, testing::ext::TestSize.Level1)
{
    std::string str = "12344";
    int32_t base = 10;
    int32_t ret = StringToInt(str, base);
    EXPECT_EQ(ret, 12344);
}

HWTEST_F(DmAnonymousTest, StringToInt_002, testing::ext::TestSize.Level1)
{
    std::string str;
    int32_t base = 10;
    int32_t ret = StringToInt(str, base);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DmAnonymousTest, StringToInt64_001, testing::ext::TestSize.Level1)
{
    std::string str;
    int32_t base = 10;
    int64_t ret = StringToInt64(str, base);
    EXPECT_EQ(ret, 0);
}


HWTEST_F(DmAnonymousTest, ParseMapFromJsonString_001, testing::ext::TestSize.Level1)
{
    std::string jsonStr = R"(
    {
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "CRYPTOSUPPORT" : "cryptosupportTest",
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "hello",
                "pkInfoSignature" : "world",
                "pkInfo" : "pkginfo",
                "peerDeviceId" : "3515656546"
            }
        ]
    }
    )";
    std::map<std::string, std::string> paramMap;
    JsonObject jsonObj(jsonStr);
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_GE(paramMap.size(), 0);
}

HWTEST_F(DmAnonymousTest, ParseMapFromJsonString_002, testing::ext::TestSize.Level1)
{
    std::string jsonStr = R"(
    {
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "CRYPTOSUPPORT" : "cryptosupportTest",
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "hello",
                "pkInfoSignature" : "world",
                "pkInfo" : "pkginfo",
                "peerDeviceId" : "3515656546"
            }
        ]
    )";
    std::map<std::string, std::string> paramMap;
    JsonObject jsonObj(jsonStr);
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_EQ(paramMap.size(), 0);
}

HWTEST_F(DmAnonymousTest, ParseMapFromJsonString_003, testing::ext::TestSize.Level1)
{
    std::string jsonStr = "";
    std::map<std::string, std::string> paramMap;
    JsonObject jsonObj(jsonStr);
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_EQ(paramMap.size(), 0);
}

HWTEST_F(DmAnonymousTest, GetAnonyStringList_001, testing::ext::TestSize.Level1)
{
    std::string value = "valueTest";
    std::vector<std::string> valueVec;
    valueVec.push_back(value);
    std::string str = GetAnonyStringList(valueVec);
    auto ret = str.size();
    EXPECT_GE(ret, 0);
}

HWTEST_F(DmAnonymousTest, StringToInt64_002, testing::ext::TestSize.Level1)
{
    std::string str = "12345";
    int32_t base = 10;
    int32_t ret = StringToInt64(str, base);
    EXPECT_EQ(ret, 12345);
}

HWTEST_F(DmAnonymousTest, GetCallerPkgName_001, testing::ext::TestSize.Level1)
{
    std::string str = "pkgName#12345";
    std::string ret = GetCallerPkgName(str);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DmAnonymousTest, IsMessageLengthValid_001, testing::ext::TestSize.Level1)
{
    std::string inputMessage;
    bool ret = IsMessageLengthValid(inputMessage);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, IsDmCommonNotifyEventValid_001, testing::ext::TestSize.Level1)
{
    DmCommonNotifyEvent dmCommonNotifyEvent = DmCommonNotifyEvent::MAX;
    bool ret = IsDmCommonNotifyEventValid(dmCommonNotifyEvent);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, GetAnonyInt32List_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> values = {1, 1};
    std::string test = "[ *, * ]";
    EXPECT_EQ(GetAnonyInt32List(values), test);
}

HWTEST_F(DmAnonymousTest, GetAnonyInt32List_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> values = {};
    std::string test = "[  ]";
    EXPECT_EQ(GetAnonyInt32List(values), test);
}

HWTEST_F(DmAnonymousTest, GetAnonyStringList_002, testing::ext::TestSize.Level1)
{
    std::vector<std::string> valueVec = {};
    std::string test = "[  ]";
    EXPECT_EQ(GetAnonyStringList(valueVec), test);
}

HWTEST_F(DmAnonymousTest, IsNumberString_004, testing::ext::TestSize.Level1)
{
    const std::string inputString = "1233214569876678012312312312455644413123123";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DmAnonymousTest, IsNumberString_005, testing::ext::TestSize.Level1)
{
    const std::string inputString = "123*+123";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DmAnonymousTest, ConvertMapToJsonString_001, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    for (int i = 0; i < 1001; i++) {
        paramMap[std::to_string(i)] = "value";
    }
    EXPECT_EQ(ConvertMapToJsonString(paramMap), "");
}

HWTEST_F(DmAnonymousTest, ParseMapFromJsonString_004, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> paramMap;
    for (int i = 0; i < 1001; i++) {
        paramMap[std::to_string(i)] = "value";
    }
    std::string jsonStr = "{\"key1\":\"value1\",\"key2\":\"value2\"}";
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_EQ(paramMap.size(), 1001);
}

HWTEST_F(DmAnonymousTest, CompareVersion_001, testing::ext::TestSize.Level1)
{
    std::string remoteVersion = "1.2.3.4";
    std::string oldVersion = "1.2.3";
    EXPECT_EQ(CompareVersion(remoteVersion, oldVersion), true);
}

HWTEST_F(DmAnonymousTest, StringToInt_003, testing::ext::TestSize.Level1)
{
    std::string str = "!++";
    int32_t base = 10;
    int32_t ret = StringToInt(str, base);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DmAnonymousTest, StringToInt_004, testing::ext::TestSize.Level1)
{
    std::string str = "-9223372036854775809";
    int32_t base = 10;
    int32_t ret = StringToInt(str, base);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DmAnonymousTest, StringToInt64_003, testing::ext::TestSize.Level1)
{
    std::string str = "!++";
    int32_t base = 10;
    int32_t ret = StringToInt64(str, base);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DmAnonymousTest, StringToInt64_004, testing::ext::TestSize.Level1)
{
    std::string str = "-9223372036854775809";
    int32_t base = 10;
    int32_t ret = StringToInt64(str, base);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DmAnonymousTest, GetSubscribeId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "12345pkgName";
    uint16_t ret = GetSubscribeId(pkgName);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DmAnonymousTest, IsIdLengthValid_001, testing::ext::TestSize.Level1)
{
    std::string inputID;
    inputID.assign(300, 'A');
    uint16_t ret = IsIdLengthValid(inputID);
    EXPECT_EQ(ret, false);
    inputID = "";
    ret = IsIdLengthValid(inputID);
    EXPECT_EQ(ret, false);
    inputID = "123";
    ret = IsIdLengthValid(inputID);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DmAnonymousTest, IsMessageLengthValid_002, testing::ext::TestSize.Level1)
{
    std::string inputMessage;
    inputMessage.assign(40 * 1024 * 1024 + 1, 'A');
    bool ret = IsMessageLengthValid(inputMessage);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, IsValueExist_001, testing::ext::TestSize.Level1)
{
    std::multimap<std::string, int32_t> unorderedmap = {{"1", 1}};
    std::string udid = "1";
    int32_t userId = 3;
    bool ret = IsValueExist(unorderedmap, udid, userId);
    EXPECT_FALSE(ret);
    udid = "3";
    ret = IsValueExist(unorderedmap, udid, 1);
    EXPECT_FALSE(ret);
    ret = IsValueExist(unorderedmap, udid, userId);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, IsDmCommonNotifyEventValid_002, testing::ext::TestSize.Level1)
{
    DmCommonNotifyEvent dmCommonNotifyEvent = DmCommonNotifyEvent::MIN;
    bool ret = IsDmCommonNotifyEventValid(dmCommonNotifyEvent);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, GetSubStr_001, testing::ext::TestSize.Level1)
{
    std::string rawStr = "123";
    std::string separator = "2";
    int32_t index = 10;
    EXPECT_EQ(GetSubStr(rawStr, separator, index), "");
    index = -1;
    EXPECT_EQ(GetSubStr(rawStr, separator, index), "");
    separator = "";
    EXPECT_EQ(GetSubStr(rawStr, separator, index), "");
    rawStr = "";
    EXPECT_EQ(GetSubStr(rawStr, separator, index), "");
}

HWTEST_F(DmAnonymousTest, GetSubscribeId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "12345#pkgName";
    uint16_t ret = GetSubscribeId(pkgName);
    EXPECT_GE(ret, 0);
}

HWTEST_F(DmAnonymousTest, GetAnonyInt32List_005, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> values;
    values.push_back(101);
    values.push_back(102);
    std::string ret = GetAnonyInt32List(values);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DmAnonymousTest, IsIdLengthValid_002, testing::ext::TestSize.Level1)
{
    std::string inputID = "";
    bool ret = IsIdLengthValid(inputID);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, GetSubStr_002, testing::ext::TestSize.Level1)
{
    std::string rawStr = "";
    std::string separator = "";
    int32_t index = 1;
    std::string ret = GetSubStr(rawStr, separator, index);
    EXPECT_TRUE(ret.empty());
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
