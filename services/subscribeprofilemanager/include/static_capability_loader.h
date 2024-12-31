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

#ifndef OHOS_DP_STATIC_CAPABILITY_LOADER_H
#define OHOS_DP_STATIC_CAPABILITY_LOADER_H

#include "cJSON.h"
#include <unordered_map>

#include "characteristic_profile.h"
#include "distributed_device_profile_log.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class StaticCapabilityLoader {
DECLARE_SINGLE_INSTANCE(StaticCapabilityLoader)

public:
    int32_t Init();
    int32_t UnInit();
    int32_t LoadStaticCapability(std::string& staticCapability);
    int32_t LoadStaticInfo(const std::string& staticCapability, std::string& staticVersion,
        std::unordered_map<std::string, CharacteristicProfile>& charProfiles);
    int32_t LoadStaticProfiles(const std::string& deviceId, const std::string& staticCapability,
        const std::string& staticVersion, std::unordered_map<std::string, CharacteristicProfile>& charProfiles);

private:
    int32_t LoadJsonFile(const std::string& filePath, std::string& fileContent);
    int32_t GetStaticCapability(const cJSON* const staticCapabilityJson, std::string& staticCapability);
    int32_t GetStaticInfo(const cJSON* const staticInfoJson, const std::string& staticCapability,
        std::string& staticVersion, std::unordered_map<std::string, CharacteristicProfile>& charProfiles);
    int32_t GetStaticInfoByVersion(const std::string& deviceId, const std::string& staticCapability,
        const cJSON* const staticInfoJson, const std::string& staticVersion,
        std::unordered_map<std::string, CharacteristicProfile>& charProfiles);
    cJSON* GetLatestStaticInfoJson(const cJSON* const staticInfoJson);
    cJSON* GetStaticInfoJsonByVersion(const cJSON* const staticInfoJson, const std::string& staticVersion);
    int32_t GetStaticVersion(const cJSON* const staticInfoJson, std::string& staticVersion);
    int32_t GenerateStaticProfiles(const std::string& deviceId, const std::string& staticCapability,
        const cJSON* const staticInfoJson, std::unordered_map<std::string, CharacteristicProfile>& charProfiles);
    void InitStaticCapability(int32_t size, std::string& staticCapability);
    void SetStaticCapability(const cJSON* const staticCapabilities, std::string& staticCapability);
    void SetStaticCapabilityFlag(const std::string& handlerName, const std::string& handlerLoc,
        std::string& staticCapability);
    bool GetStaticCapabilityValue(const std::string& handlerLoc);
    bool HasStaticCapability(const std::string& serviceId, const std::string& staticCapability);
    bool StaticVersionCheck(const std::string& peerVersion, const std::string& localVersion);
    bool IsValidVersion(const std::string& version);
};
}
}
#endif // OHOS_DP_STATIC_CAPABILITY_LOADER_H
