/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dm_anonymous_3rd.h"

#include <sstream>
#include <set>

#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
constexpr uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024;
constexpr uint32_t MAX_MAP_LEN = 1000;
}

std::string GetAnonyString(const std::string &value)
{
    const int32_t int32ShortIdLength = 20;
    const int32_t int32PlainTextLength = 4;
    const int32_t int32MinIdLength = 3;

    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < int32MinIdLength) {
        return tmpStr;
    }

    std::string res;
    if (strLen <= int32ShortIdLength) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, int32PlainTextLength);
        res += tmpStr;
        res.append(value, strLen - int32PlainTextLength, int32PlainTextLength);
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

std::string GetAnonyJsonString(const std::string &value)
{
    if (value.empty()) {
        LOGE("Str is empty.");
        return "";
    }
    JsonObject paramJson(value);
    if (paramJson.IsDiscarded()) {
        LOGE("Str is not json string.");
        return "";
    }
    const std::set<std::string> sensitiveKey = { "LOCALDEVICEID", "localAccountId", "networkId", "lnnPublicKey",
        "transmitPublicKey", "DEVICEID", "deviceId", "keyValue", "deviceName", "REMOTE_DEVICE_NAME", "data" };

    for (auto &element : paramJson.Items()) {
        if (element.IsString() && sensitiveKey.find(element.Key()) != sensitiveKey.end()) {
            paramJson[element.Key()] = GetAnonyString(element.Get<std::string>());
        }
    }
    return paramJson.Dump();
}

bool IsString(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = (jsonObj.Contains(key) && jsonObj[key].IsString()
        && jsonObj[key].Get<std::string>().size() <= MAX_MESSAGE_LEN);
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt32(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<int64_t>() >= INT32_MIN &&
        jsonObj[key].Get<int64_t>() <= INT32_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsBool(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsBoolean();
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt64(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<int64_t>() >= INT64_MIN &&
        jsonObj[key].Get<int64_t>() <= INT64_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

std::string ConvertMapToJsonString(const std::map<std::string, std::string> &paramMap)
{
    std::string jsonStr = "";
    if (paramMap.size() > MAX_MAP_LEN) {
        LOGE("invalid paramMap");
        return jsonStr;
    }
    if (!paramMap.empty()) {
        JsonObject jsonObj;
        for (const auto &it : paramMap) {
            jsonObj[it.first] = it.second;
        }
        jsonStr = jsonObj.Dump();
    }
    return jsonStr;
}

void ParseMapFromJsonString(const std::string &jsonStr, std::map<std::string, std::string> &paramMap)
{
    if (jsonStr.empty()) {
        return;
    }
    if (paramMap.size() > MAX_MAP_LEN) {
        LOGE("invalid paramMap");
        return;
    }
    JsonObject paramJson(jsonStr);
    if (paramJson.IsDiscarded()) {
        return;
    }
    for (auto &element : paramJson.Items()) {
        if (element.IsString()) {
            paramMap.insert(std::pair<std::string, std::string>(element.Key(), element.Get<std::string>()));
        }
    }
}

bool IsUint64(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<uint64_t>() >= 0 &&
        jsonObj[key].Get<uint64_t>() <= UINT64_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUint32(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<int64_t>() >= 0 &&
        jsonObj[key].Get<int64_t>() <= UINT32_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}
} // namespace DistributedHardware
} // namespace OHOS