/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_language_manager.h"

#include "cJSON.h"
#include <map>

#include "dm_constants.h"
#include "dm_log.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DmLanguageManager);
const int32_t MAX_LEN  = 128;
const std::string SYSTEM_LANGUAGE_KEY = "persist.global.language";
const std::string SYSTEM_LANGUAGE_LOCALE_KEY = "persist.global.locale";
const std::string DEFAULT_LANGUAGE = "zh-Hans";
const std::string LANGUAGE_EN = "en-Latn_US";
const std::string LANGUAGE_AND_LOCALE_STR =
    R"({
    "zh-Hant": [
        {
            "persist.global.locale": "zh-Hant-HK"
        },
        {
            "persist.global.locale": "zh-Hant-TW"
        }
    ]
})";

std::string DmLanguageManager::GetSystemParam(const std::string &key)
{
    char value[MAX_LEN] = {0};
    int32_t ret = GetParameter(key.c_str(), "", value, MAX_LEN);
    if (ret <= 0) {
        LOGE("GetSystemParam %{public}s failed", key.c_str());
        return "";
    }
    return std::string(value);
}

void DmLanguageManager::GetLocaleByLanguage(const std::string &language, std::set<std::string> &localeSet)
{
    cJSON *languageAndLocaleObj = cJSON_Parse(LANGUAGE_AND_LOCALE_STR.c_str());
    if (languageAndLocaleObj == NULL) {
        LOGE("parse languageAndLocaleObj failed");
        return;
    }
    cJSON *languageObj = cJSON_GetObjectItem(languageAndLocaleObj, language.c_str());
    if (languageObj == NULL || !cJSON_IsArray(languageObj)) {
        cJSON_Delete(languageAndLocaleObj);
        return;
    }
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, languageObj) {
        if (!cJSON_IsObject(item)) {
            LOGE("item is not object!");
            continue;
        }
        cJSON* localeObj = cJSON_GetObjectItemCaseSensitive(item, SYSTEM_LANGUAGE_LOCALE_KEY.c_str());
        if (!cJSON_IsString(localeObj) || localeObj->valuestring == NULL) {
            LOGE("Get localeObj fail!");
            continue;
        }
        localeSet.insert(localeObj->valuestring);
    }
    cJSON_Delete(languageAndLocaleObj);
}

std::string DmLanguageManager::GetTextBySystemLanguage(const std::string &text)
{
    if (text.empty()) {
        return "";
    }
    cJSON *textObj = cJSON_Parse(text.c_str());
    if (textObj == NULL) {
        LOGE("parse text failed");
        return text;
    }
    std::string resultText = text;
    std::string language = GetSystemParam(SYSTEM_LANGUAGE_KEY);
    language = language.empty() ? DEFAULT_LANGUAGE : language;
    std::set<std::string> localeSet;
    GetLocaleByLanguage(language, localeSet);
    if (localeSet.size() > 0) {
        resultText = GetTextBySystemLocale(textObj, localeSet);
        if (!resultText.empty()) {
            cJSON_Delete(textObj);
            return resultText;
        }
    }
    cJSON *languageJson = cJSON_GetObjectItem(textObj, language.c_str());
    if (languageJson != NULL && cJSON_IsString(languageJson)) {
        resultText = std::string(languageJson->valuestring);
        cJSON_Delete(textObj);
        return resultText;
    }
    cJSON *defaultJson = cJSON_GetObjectItem(textObj, DEFAULT_LANGUAGE.c_str());
    if (defaultJson != NULL && cJSON_IsString(defaultJson)) {
        resultText = std::string(defaultJson->valuestring);
        cJSON_Delete(textObj);
        return resultText;
    }
    cJSON *enJson = cJSON_GetObjectItem(textObj, LANGUAGE_EN.c_str());
    if (enJson != NULL && cJSON_IsString(enJson)) {
        resultText = std::string(enJson->valuestring);
        cJSON_Delete(textObj);
        return resultText;
    }
    cJSON_Delete(textObj);
    return "";
}

std::string DmLanguageManager::GetTextBySystemLocale(const cJSON *const textObj,
    const std::set<std::string> &localeSet)
{
    std::string resultText = "";
    std::string languageLocale = GetSystemParam(SYSTEM_LANGUAGE_LOCALE_KEY);
    cJSON *languageLocaleson = cJSON_GetObjectItem(textObj, languageLocale.c_str());
    if (languageLocaleson != NULL && cJSON_IsString(languageLocaleson)) {
        resultText = std::string(languageLocaleson->valuestring);
        return resultText;
    }
    for (std::string locale : localeSet) {
        cJSON *localesonObj = cJSON_GetObjectItem(textObj, locale.c_str());
        if (localesonObj != NULL && cJSON_IsString(localesonObj)) {
            resultText = std::string(localesonObj->valuestring);
            return resultText;
        }
    }
    return "";
}
} // namespace DistributedHardware
} // namespace OHOS