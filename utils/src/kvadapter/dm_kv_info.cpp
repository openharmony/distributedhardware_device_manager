/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "dm_kv_info.h"

#include "cJSON.h"
#include "dm_anonymous.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* UDID_HASH_KEY = "udidHash";
constexpr const char* APP_ID_KEY = "appID";
constexpr const char* ANOY_DEVICE_ID_KEY = "anoyDeviceId";
constexpr const char* SALT_KEY = "salt";
constexpr const char* LAST_MODIFY_TIME_KEY = "lastModifyTime";

void ConvertDmKVValueToJson(const DmKVValue &kvValue, std::string &result)
{
    JsonObject jsonObj;
    jsonObj[UDID_HASH_KEY] = kvValue.udidHash;
    jsonObj[APP_ID_KEY] = kvValue.appID;
    jsonObj[ANOY_DEVICE_ID_KEY] = kvValue.anoyDeviceId;
    jsonObj[SALT_KEY] = kvValue.salt;
    jsonObj[LAST_MODIFY_TIME_KEY] = kvValue.lastModifyTime;
    result = jsonObj.Dump();
}

void ConvertJsonToDmKVValue(const std::string &result, DmKVValue &kvValue)
{
    if (result.empty()) {
        return;
    }
    JsonObject resultJson(result);
    if (resultJson.IsDiscarded()) {
        return;
    }
    if (IsString(resultJson, UDID_HASH_KEY)) {
        kvValue.udidHash = resultJson[UDID_HASH_KEY].Get<std::string>();
    }
    if (IsString(resultJson, APP_ID_KEY)) {
        kvValue.appID = resultJson[APP_ID_KEY].Get<std::string>();
    }
    if (IsString(resultJson, ANOY_DEVICE_ID_KEY)) {
        kvValue.anoyDeviceId = resultJson[ANOY_DEVICE_ID_KEY].Get<std::string>();
    }
    if (IsString(resultJson, SALT_KEY)) {
        kvValue.salt = resultJson[SALT_KEY].Get<std::string>();
    }
    if (IsInt64(resultJson, LAST_MODIFY_TIME_KEY)) {
        kvValue.lastModifyTime = resultJson[LAST_MODIFY_TIME_KEY].Get<int64_t>();
    }
}
} // namespace DistributedHardware
} // namespace OHOS
