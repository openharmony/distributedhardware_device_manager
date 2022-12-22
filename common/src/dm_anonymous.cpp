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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
std::string GetAnonyString(const std::string &value)
{
    const int32_t INT32_SHORT_ID_LENGTH = 20;
    const int32_t INT32_PLAINTEXT_LENGTH = 4;
    const int32_t INT32_MIN_ID_LENGTH = 3;

    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    std::string res;
    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

std::string GetAnonyInt32(const int32_t value)
{
    std::string tempString = std::to_string(value);
    size_t length = tempString.length();
    if (length == 0x01) {
        tempString[0] = '*';
        return tempString;
    }
    for (size_t i = 1; i < length - 1; i++) {
        tempString[i] = '*';
    }
    return tempString;
}

bool IsNumberString(const std::string &inputString)
{
    LOGI("IsNumberString for DeviceManagerNapi");
    if (inputString.length() == 0) {
        LOGE("inputString is Null");
        return false;
    }
    const int32_t MIN_ASCLL_NUM = 48;
    const int32_t MAX_ASCLL_NUM = 57;
    for (size_t i = 0; i < inputString.length(); i++) {
        int num = (int)inputString[i];
        if (num >= MIN_ASCLL_NUM && num <= MAX_ASCLL_NUM) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool IsString(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_string() && jsonObj[key].size() <= MAX_MESSAGE_LEN;
    if (!res) {
        LOGE("the key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt32(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_integer() && jsonObj[key] >= INT32_MIN &&
        jsonObj[key] <= INT32_MAX;
    if (!res) {
        LOGE("the key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt64(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_integer() && jsonObj[key] >= INT64_MIN &&
        jsonObj[key] <= INT64_MAX;
    if (!res) {
        LOGE("the key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsArray(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_array();
    if (!res) {
        LOGE("the key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsBool(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_boolean();
    if (!res) {
        LOGE("the key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}
} // namespace DistributedHardware
} // namespace OHOS