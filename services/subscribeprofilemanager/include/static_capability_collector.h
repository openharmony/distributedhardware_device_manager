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

#ifndef OHOS_DP_STATIC_CAPABILITY_COLLECTOR_H
#define OHOS_DP_STATIC_CAPABILITY_COLLECTOR_H

#include "single_instance.h"
#include "characteristic_profile.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class StaticCapabilityCollector {
DECLARE_SINGLE_INSTANCE(StaticCapabilityCollector);
public:
    int32_t Init();
    int32_t UnInit();

private:
    int32_t CollectStaticCapability();
    int32_t AddStaticInfoToCache(const std::unordered_map<std::string,
        CharacteristicProfile>& staticProfileMap);
    int32_t AddStaticCapabilityToDB(const std::string& staticVersion, const std::string& staticCapability);
};
}
}
#endif // OHOS_DP_STATIC_CAPABILITY_COLLECTOR_H