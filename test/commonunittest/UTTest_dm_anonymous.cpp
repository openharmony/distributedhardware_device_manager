/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
HWTEST_F(DmAnonymousTest, GetAnonyString_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, GetAnonyString_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, GetAnonyString_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, GetAnonyInt32_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, GetAnonyInt32_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, GetAnonyInt32_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, IsNumberString_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, IsNumberString_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, IsNumberString_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, GetErrorString_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DmAnonymousTest, IsString_001, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "pinToken" : "IsString"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsString(jsonObj, PIN_TOKEN);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsString_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsString_002, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "pinToken" : 123
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsString(jsonObj, PIN_TOKEN);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsInt32_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt32_001, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "AUTHTYPE" : 369
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsInt32(jsonObj, TAG_AUTH_TYPE);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsInt32_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt32_002, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "AUTHTYPE" : "authtypeTest"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsInt32(jsonObj, TAG_AUTH_TYPE);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsUint32_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsUint32_001, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "AUTHTYPE" : "authtypeTest"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsUint32(jsonObj, TAG_AUTH_TYPE);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsInt64_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt64_001, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "REQUESTID" : 789
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsInt64(jsonObj, TAG_REQUEST_ID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsInt64_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsInt64_002, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "REQUESTID" : "requestidTest"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsInt64(jsonObj, TAG_REQUEST_ID);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsArray_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsArray_001, testing::ext::TestSize.Level0)
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
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsArray(jsonObj, FIELD_CREDENTIAL_DATA);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsArray_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsArray_002, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "authType" : 1,
        "userId" : "123",
        "credentialData" : "credentialDataTest"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsArray(jsonObj, FIELD_CREDENTIAL_DATA);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsBool_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsBool_001, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "CRYPTOSUPPORT" : false,
        "userId" : "123",
        "credentialData" : "credentialDataTest"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsBool(jsonObj, TAG_CRYPTO_SUPPORT);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsBool_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsBool_002, testing::ext::TestSize.Level0)
{
    std::string str = R"(
    {
        "CRYPTOSUPPORT" : "cryptosupportTest",
        "userId" : "123",
        "credentialData" : "credentialDataTest"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    bool ret = IsBool(jsonObj, TAG_CRYPTO_SUPPORT);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ConvertCharArray2String_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, ConvertCharArray2String_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DmAnonymousTest, StringToInt_001, testing::ext::TestSize.Level0)
{
    std::string str = "12344";
    int32_t base = 10;
    int32_t ret = StringToInt(str, base);
    EXPECT_EQ(ret, 12344);
}

HWTEST_F(DmAnonymousTest, StringToInt_002, testing::ext::TestSize.Level0)
{
    std::string str;
    int32_t base = 10;
    int32_t ret = StringToInt(str, base);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DmAnonymousTest, StringToInt64_001, testing::ext::TestSize.Level0)
{
    std::string str;
    int32_t base = 10;
    int64_t ret = StringToInt64(str, base);
    EXPECT_EQ(ret, 0);
}


HWTEST_F(DmAnonymousTest, ParseMapFromJsonString_001, testing::ext::TestSize.Level0)
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
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_GE(paramMap.size(), 0);
}

HWTEST_F(DmAnonymousTest, ParseMapFromJsonString_002, testing::ext::TestSize.Level0)
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
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_EQ(paramMap.size(), 0);
}

HWTEST_F(DmAnonymousTest, ParseMapFromJsonString_003, testing::ext::TestSize.Level0)
{
    std::string jsonStr = "";
    std::map<std::string, std::string> paramMap;
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    ParseMapFromJsonString(jsonStr, paramMap);
    EXPECT_EQ(paramMap.size(), 0);
}

HWTEST_F(DmAnonymousTest, GetAnonyStringList_001, testing::ext::TestSize.Level0)
{
    std::string value = "valueTest";
    std::vector<std::string> valueVec;
    valueVec.push_back(value);
    std::string str = GetAnonyStringList(value);
    auto ret = str.size();
    EXPECT_GE(ret, 0);
}

HWTEST_F(DmAnonymousTest, StringToInt64_001, testing::ext::TestSize.Level0)
{
    std::string str = "12345";
    int32_t base = 10;
    int32_t ret = StringToInt64(str, base);
    EXPECT_EQ(ret, 12345);
}

HWTEST_F(DmAnonymousTest, GetCallerPkgName_001, testing::ext::TestSize.Level0)
{
    std::string str = "pkgName#12345";
    std::string ret = GetCallerPkgName(str);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DmAnonymousTest, IsMessageLengthValid_001, testing::ext::TestSize.Level0)
{
    std::string inputMessage;
    bool ret = IsMessageLengthValid(inputMessage);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, IsDmCommonNotifyEventValid_001, testing::ext::TestSize.Level0)
{
    DmCommonNotifyEvent dmCommonNotifyEvent = DmCommonNotifyEvent::MAX;
    bool ret = IsDmCommonNotifyEventValid(dmCommonNotifyEvent);
    EXPECT_FALSE(ret);
}

HWTEST_F(DmAnonymousTest, GetSubscribeId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "12345#pkgName";
    uint16_t ret = GetSubscribeId(pkgName);
    EXPECT_GE(ret, 0);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
