/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "content_sensor_manager_utils.h"
#include "distributed_device_profile_errors.h"
#include "profile_cache.h"
#include "static_capability_collector.h"
#include "static_capability_loader.h"
#include "static_profile_manager.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(StaticCapabilityCollector);
namespace {
    const std::string TAG = "StaticCapabilityCollector";
}
int32_t StaticCapabilityCollector::Init()
{
    HILOGI("call!");
    StaticCapabilityLoader::GetInstance().Init();
    CollectStaticCapability();
    return DP_SUCCESS;
}

int32_t StaticCapabilityCollector::UnInit()
{
    StaticCapabilityLoader::GetInstance().UnInit();
    HILOGI("call!");
    return DP_SUCCESS;
}

int32_t StaticCapabilityCollector::CollectStaticCapability()
{
    HILOGI("call!");
    std::string staticCapability = "";
    if (StaticCapabilityLoader::GetInstance().LoadStaticCapability(staticCapability) != DP_SUCCESS ||
        staticCapability.empty()) {
        HILOGE("staticCapability is invalid!");
        return DP_STATIC_COLLECT_FAIL;
    }
    std::string staticVersion = "";
    std::unordered_map<std::string, CharacteristicProfile> staticProfileMap;
    StaticCapabilityLoader::GetInstance().LoadStaticInfo(staticCapability, staticVersion, staticProfileMap);
    if (staticVersion.empty() || staticProfileMap.empty()) {
        HILOGE("staticVersion or staticProfileMap is invalid!");
        return DP_STATIC_COLLECT_FAIL;
    }
    AddStaticInfoToCache(staticProfileMap);
    AddStaticCapabilityToDB(staticVersion, staticCapability);
    return DP_SUCCESS;
}

int32_t StaticCapabilityCollector::AddStaticInfoToCache(
    const std::unordered_map<std::string, CharacteristicProfile>& staticProfileMap)
{
    HILOGI("call!");
    for (const auto& item : staticProfileMap) {
        HILOGI("AddStaticInfoToCache key: %{public}s, value: %{public}s!",
            ProfileUtils::GetDbKeyAnonyString(item.first).c_str(), item.second.dump().c_str());
        ProfileCache::GetInstance().AddStaticCharProfile(item.second);
    }
    return DP_SUCCESS;
}

int32_t StaticCapabilityCollector::AddStaticCapabilityToDB(const std::string& staticVersion,
    const std::string& staticCapability)
{
    HILOGI("call!");
    if (staticVersion.empty() || staticVersion.size() >= MAX_STRING_LEN) {
        HILOGE("StaticVersion is invalid!");
        return DP_INVALID_PARAM;
    }
    if (staticCapability.empty() || staticCapability.size() >= MAX_STRING_LEN) {
        HILOGE("StaticCapability is invalid!");
        return DP_INVALID_PARAM;
    }
    cJSON* characteristicValueJson = cJSON_CreateObject();
    if (!cJSON_IsObject(characteristicValueJson)) {
        HILOGE("Create cJSON failed!");
        cJSON_Delete(characteristicValueJson);
        return DP_STATIC_COLLECT_FAIL;
    }
    cJSON_AddStringToObject(characteristicValueJson, STATIC_CAPABILITY_VERSION.c_str(), staticVersion.c_str());
    cJSON_AddStringToObject(characteristicValueJson, STATIC_CAPABILITY_VALUE.c_str(), staticCapability.c_str());
    char* characteristicValueStr = cJSON_Print(characteristicValueJson);
    if (characteristicValueStr == NULL) {
        HILOGE("characteristicValueStr is null!");
        cJSON_Delete(characteristicValueJson);
        return DP_STATIC_COLLECT_FAIL;
    }
    std::string characteristicValue = characteristicValueStr;
    std::string localDeviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    std::string serviceId = STATIC_CAPABILITY_SVR_ID;
    std::string characteristicId = STATIC_CAPABILITY_CHAR_ID;
    CharacteristicProfile staticCapabilityProfile(localDeviceId, serviceId, characteristicId, characteristicValue);
    StaticProfileManager::GetInstance().PutCharacteristicProfile(staticCapabilityProfile);
    cJSON_free(characteristicValueStr);
    cJSON_Delete(characteristicValueJson);
    return DP_SUCCESS;
}
} // namespace DistributedDeviceProfile
} // namespace OHOS