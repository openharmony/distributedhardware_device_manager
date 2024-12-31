/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DP_SUBSCRIBE_DEVICE_PROFILE_MANAGER_H
#define OHOS_DP_SUBSCRIBE_DEVICE_PROFILE_MANAGER_H

#include <string>
#include <map>
#include <mutex>
#include "single_instance.h"
#include "distributed_device_profile_enums.h"
#include "device_profile.h"
#include "dp_subscribe_info.h"
#include "service_profile.h"
#include "characteristic_profile.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class SubscribeProfileManager {
    DECLARE_SINGLE_INSTANCE(SubscribeProfileManager);
public:
    int32_t Init();
    int32_t UnInit();
    int32_t NotifyProfileChange(ProfileType profileType, ChangeType changeType, const std::string& dbKey,
        const std::string& dbValue);
    int32_t NotifyTrustDeviceProfileAdd(const TrustDeviceProfile& trustDeviceProfile);
    int32_t NotifyTrustDeviceProfileUpdate(const TrustDeviceProfile& oldDeviceProfile,
                                           const TrustDeviceProfile& newDeviceProfile);
    int32_t NotifyTrustDeviceProfileDelete(const TrustDeviceProfile& trustDeviceProfile);
    int32_t NotifyTrustDeviceProfileActive(const TrustDeviceProfile& trustDeviceProfile);
    int32_t NotifyTrustDeviceProfileInactive(const TrustDeviceProfile& trustDeviceProfile);
    int32_t SubscribeDeviceProfile(const SubscribeInfo& subscribeInfo);
    int32_t SubscribeDeviceProfile(std::map<std::string, SubscribeInfo> subscribeInfos);
    int32_t UnSubscribeDeviceProfile(const SubscribeInfo& subscribeInfo);

private:
    int32_t NotifyDeviceProfileAdd(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyDeviceProfileUpdate(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyDeviceProfileDelete(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyServiceProfileAdd(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyServiceProfileUpdate(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyServiceProfileDelete(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyCharProfileAdd(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyCharProfileUpdate(const std::string& dbKey, const std::string& dbValue);
    int32_t NotifyCharProfileDelete(const std::string& dbKey, const std::string& dbValue);
    std::unordered_set<SubscribeInfo, SubscribeHash, SubscribeCompare> GetSubscribeInfos(const std::string& dbKey);
    std::string DBKeyToSubcribeKey(const std::string& dbkey);

private:
    using Func = int32_t(SubscribeProfileManager::*)(const std::string& profileKey, const std::string& profileValue);
    std::mutex funcsMutex_;
    std::map<uint32_t, Func> funcsMap_;
    std::mutex subscribeMutex_;
    std::map<std::string, std::unordered_set<SubscribeInfo, SubscribeHash, SubscribeCompare>> subscribeInfoMap_;
};
} // namespace DistributedDeviceProfile
} // namespace OHOS
#endif // OHOS_DP_SUBSCRIBE_DEVICE_PROFILE_MANAGER_H
