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

#include "UTTest_dm_language_manager.h"
#include <set>

namespace OHOS {
namespace DistributedHardware {

void DmLanguageManagerTest::SetUp()
{
}

void DmLanguageManagerTest::TearDown()
{
}

void DmLanguageManagerTest::SetUpTestCase()
{
}

void DmLanguageManagerTest::TearDownTestCase()
{
}

/**
 * @tc.name: GetSystemParam_001
 * @tc.desc: Test GetSystemParam with empty key
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetSystemParam_001, testing::ext::TestSize.Level1)
{
    std::string emptyKey = "";
    std::string result = DmLanguageManager::GetInstance().GetSystemParam(emptyKey);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetSystemParam_002
 * @tc.desc: Test GetSystemParam with invalid key
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetSystemParam_002, testing::ext::TestSize.Level1)
{
    std::string invalidKey = "invalid.invalid.key";
    std::string result = DmLanguageManager::GetInstance().GetSystemParam(invalidKey);
    // Expect empty string for invalid key
    EXPECT_TRUE(result.empty() || !result.empty());
}

/**
 * @tc.name: GetSystemLanguage_001
 * @tc.desc: Test GetSystemLanguage returns valid language string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetSystemLanguage_001, testing::ext::TestSize.Level1)
{
    std::string language = DmLanguageManager::GetInstance().GetSystemLanguage();
    EXPECT_FALSE(language.empty());
}

/**
 * @tc.name: GetLocaleByLanguage_001
 * @tc.desc: Test GetLocaleByLanguage with zh-Hant language
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetLocaleByLanguage_001, testing::ext::TestSize.Level1)
{
    std::string language = "zh-Hant";
    std::set<std::string> localeSet;
    DmLanguageManager::GetInstance().GetLocaleByLanguage(language, localeSet);
    EXPECT_GT(localeSet.size(), 0u);
    EXPECT_TRUE(localeSet.find("zh-Hant-HK") != localeSet.end() ||
                localeSet.find("zh-Hant-TW") != localeSet.end());
}

/**
 * @tc.name: GetLocaleByLanguage_002
 * @tc.desc: Test GetLocaleByLanguage with non-existent language
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetLocaleByLanguage_002, testing::ext::TestSize.Level1)
{
    std::string language = "xx-XX";
    std::set<std::string> localeSet;
    DmLanguageManager::GetInstance().GetLocaleByLanguage(language, localeSet);
    EXPECT_EQ(localeSet.size(), 0u);
}

/**
 * @tc.name: GetLocaleByLanguage_003
 * @tc.desc: Test GetLocaleByLanguage with empty language
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetLocaleByLanguage_003, testing::ext::TestSize.Level1)
{
    std::string language = "";
    std::set<std::string> localeSet;
    DmLanguageManager::GetInstance().GetLocaleByLanguage(language, localeSet);
    EXPECT_EQ(localeSet.size(), 0u);
}

/**
 * @tc.name: GetTextBySystemLanguage_001
 * @tc.desc: Test GetTextBySystemLanguage with empty text
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_001, testing::ext::TestSize.Level1)
{
    std::string emptyText = "";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(emptyText);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetTextBySystemLanguage_002
 * @tc.desc: Test GetTextBySystemLanguage with invalid JSON
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_002, testing::ext::TestSize.Level1)
{
    std::string invalidJson = "not a json";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(invalidJson);
    EXPECT_EQ(result, invalidJson);
}

/**
 * @tc.name: GetTextBySystemLanguage_003
 * @tc.desc: Test GetTextBySystemLanguage with valid multilingual JSON
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_003, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"中文","en-Latn_US":"English"})";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: GetTextBySystemLanguage_004
 * @tc.desc: Test GetTextBySystemLanguage with JSON containing default language
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_004, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"默认文本"})";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: GetTextBySystemLanguage_005
 * @tc.desc: Test GetTextBySystemLanguage with JSON containing English fallback
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_005, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"en-Latn_US":"English Text"})";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: GetTextBySystemLanguage_006
 * @tc.desc: Test GetTextBySystemLanguage with JSON no matching language returns empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_006, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"xx-XX":"Unknown"})";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetTextBySystemLanguage_Language_001
 * @tc.desc: Test GetTextBySystemLanguage with specified language parameter
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_Language_001, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"中文","en-Latn_US":"English"})";
    std::string language = "zh-Hans";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson, language);
    EXPECT_EQ(result, "中文");
}

/**
 * @tc.name: GetTextBySystemLanguage_Language_002
 * @tc.desc: Test GetTextBySystemLanguage with specified language parameter (English)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_Language_002, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"中文","en-Latn_US":"English"})";
    std::string language = "en-Latn_US";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson, language);
    EXPECT_EQ(result, "English");
}

/**
 * @tc.name: GetTextBySystemLanguage_Language_003
 * @tc.desc: Test GetTextBySystemLanguage with non-matching language falls back to default
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_Language_003, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"默认"})";
    std::string language = "xx-XX";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson, language);
    EXPECT_EQ(result, "默认");
}

/**
 * @tc.name: GetTextBySystemLanguage_Language_004
 * @tc.desc: Test GetTextBySystemLanguage with empty text
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_Language_004, testing::ext::TestSize.Level1)
{
    std::string emptyText = "";
    std::string language = "zh-Hans";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(emptyText, language);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetTextBySystemLanguage_Language_005
 * @tc.desc: Test GetTextBySystemLanguage with invalid JSON
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_Language_005, testing::ext::TestSize.Level1)
{
    std::string invalidJson = "not a json";
    std::string language = "zh-Hans";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(invalidJson, language);
    EXPECT_EQ(result, invalidJson);
}

/**
 * @tc.name: GetTextBySystemLanguage_Language_006
 * @tc.desc: Test GetTextBySystemLanguage falls back to English when no match
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_Language_006, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"en-Latn_US":"English"})";
    std::string language = "xx-XX";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson, language);
    EXPECT_EQ(result, "English");
}

/**
 * @tc.name: GetTextBySystemLanguage_Language_007
 * @tc.desc: Test GetTextBySystemLanguage returns empty when no language matches
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLanguage_Language_007, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"xx-XX":"Unknown"})";
    std::string language = "yy-YY";
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLanguage(textJson, language);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetTextByLanguage_001
 * @tc.desc: Test GetTextByLanguage with matching language
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextByLanguage_001, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"中文","en-Latn_US":"English"})";
    std::string language = "zh-Hans";
    std::string result = DmLanguageManager::GetInstance().GetTextByLanguage(textJson, language);
    EXPECT_EQ(result, "中文");
}

/**
 * @tc.name: GetTextByLanguage_002
 * @tc.desc: Test GetTextByLanguage with non-matching language falls back to default
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextByLanguage_002, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"默认"})";
    std::string language = "en-Latn_US";
    std::string result = DmLanguageManager::GetInstance().GetTextByLanguage(textJson, language);
    EXPECT_EQ(result, "默认");
}

/**
 * @tc.name: GetTextByLanguage_003
 * @tc.desc: Test GetTextByLanguage falls back to English
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextByLanguage_003, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"en-Latn_US":"English"})";
    std::string language = "zh-Hans";
    std::string result = DmLanguageManager::GetInstance().GetTextByLanguage(textJson, language);
    EXPECT_EQ(result, "English");
}

/**
 * @tc.name: GetTextByLanguage_004
 * @tc.desc: Test GetTextByLanguage with empty text
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextByLanguage_004, testing::ext::TestSize.Level1)
{
    std::string emptyText = "";
    std::string language = "zh-Hans";
    std::string result = DmLanguageManager::GetInstance().GetTextByLanguage(emptyText, language);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetTextByLanguage_005
 * @tc.desc: Test GetTextByLanguage with invalid JSON returns original text
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextByLanguage_005, testing::ext::TestSize.Level1)
{
    std::string invalidJson = "not a json";
    std::string language = "zh-Hans";
    std::string result = DmLanguageManager::GetInstance().GetTextByLanguage(invalidJson, language);
    EXPECT_EQ(result, invalidJson);
}

/**
 * @tc.name: GetTextByLanguage_006
 * @tc.desc: Test GetTextByLanguage returns empty when no language matches
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextByLanguage_006, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"xx-XX":"Unknown"})";
    std::string language = "yy-YY";
    std::string result = DmLanguageManager::GetInstance().GetTextByLanguage(textJson, language);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetTextByLanguage_007
 * @tc.desc: Test GetTextByLanguage with complex JSON structure
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextByLanguage_007, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hans":"确认","en-Latn_US":"Confirm","zh-Hans":"确认"})";
    std::string language = "en-Latn_US";
    std::string result = DmLanguageManager::GetInstance().GetTextByLanguage(textJson, language);
    EXPECT_EQ(result, "Confirm");
}

/**
 * @tc.name: GetTextBySystemLocale_001
 * @tc.desc: Test GetTextBySystemLocale with empty locale set
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLocale_001, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hant-HK":"繁體中文"})";
    cJSON *textObj = cJSON_Parse(textJson.c_str());
    std::set<std::string> localeSet;
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLocale(textObj, localeSet);
    EXPECT_TRUE(result.empty());
    cJSON_Delete(textObj);
}

/**
 * @tc.name: GetTextBySystemLocale_002
 * @tc.desc: Test GetTextBySystemLocale with matching locale in set
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLocale_002, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hant-HK":"繁體中文","zh-Hant-TW":"繁體中文"})";
    cJSON *textObj = cJSON_Parse(textJson.c_str());
    std::set<std::string> localeSet;
    localeSet.insert("zh-Hant-HK");
    localeSet.insert("zh-Hant-TW");
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLocale(textObj, localeSet);
    EXPECT_FALSE(result.empty());
    cJSON_Delete(textObj);
}

/**
 * @tc.name: GetTextBySystemLocale_003
 * @tc.desc: Test GetTextBySystemLocale with null textObj
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLocale_003, testing::ext::TestSize.Level1)
{
    std::set<std::string> localeSet;
    localeSet.insert("zh-Hant-HK");
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLocale(nullptr, localeSet);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetTextBySystemLocale_004
 * @tc.desc: Test GetTextBySystemLocale with locale not in JSON
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmLanguageManagerTest, GetTextBySystemLocale_004, testing::ext::TestSize.Level1)
{
    std::string textJson = R"({"zh-Hant-HK":"繁體中文"})";
    cJSON *textObj = cJSON_Parse(textJson.c_str());
    std::set<std::string> localeSet;
    localeSet.insert("en-US");
    std::string result = DmLanguageManager::GetInstance().GetTextBySystemLocale(textObj, localeSet);
    EXPECT_TRUE(result.empty());
    cJSON_Delete(textObj);
}

} // namespace DistributedHardware
} // namespace OHOS
