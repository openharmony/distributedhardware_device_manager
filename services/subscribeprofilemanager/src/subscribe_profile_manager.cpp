/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include "subscribe_profile_manager.h"

#include "distributed_device_profile_errors.h"
#include "dp_radar_helper.h"
#include "profile_utils.h"
#include "profile_cache.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(SubscribeProfileManager);
namespace {
    const std::string TAG = "SubscribeProfileManager";
}

int32_t SubscribeProfileManager::Init()
{
    HILOGI("call!");
    {
        std::lock_guard<std::mutex> lockGuard(funcsMutex_);
        funcsMap_[ProfileType::DEVICE_PROFILE * ChangeType::ADD] =
            &SubscribeProfileManager::NotifyDeviceProfileAdd;
        funcsMap_[ProfileType::DEVICE_PROFILE * ChangeType::UPDATE] =
            &SubscribeProfileManager::NotifyDeviceProfileUpdate;
        funcsMap_[ProfileType::DEVICE_PROFILE * ChangeType::DELETE] =
            &SubscribeProfileManager::NotifyDeviceProfileDelete;
        funcsMap_[ProfileType::SERVICE_PROFILE * ChangeType::ADD] =
            &SubscribeProfileManager::NotifyServiceProfileAdd;
        funcsMap_[ProfileType::SERVICE_PROFILE * ChangeType::UPDATE] =
            &SubscribeProfileManager::NotifyServiceProfileUpdate;
        funcsMap_[ProfileType::SERVICE_PROFILE * ChangeType::DELETE] =
            &SubscribeProfileManager::NotifyServiceProfileDelete;
        funcsMap_[ProfileType::CHAR_PROFILE * ChangeType::ADD] =
            &SubscribeProfileManager::NotifyCharProfileAdd;
        funcsMap_[ProfileType::CHAR_PROFILE * ChangeType::UPDATE] =
            &SubscribeProfileManager::NotifyCharProfileUpdate;
        funcsMap_[ProfileType::CHAR_PROFILE * ChangeType::DELETE] =
            &SubscribeProfileManager::NotifyCharProfileDelete;
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::UnInit()
{
    HILOGI("call!");
    {
        std::lock_guard<std::mutex> lockGuard(subscribeMutex_);
        subscribeInfoMap_.clear();
    }
    {
        std::lock_guard<std::mutex> lockGuard(funcsMutex_);
        funcsMap_.clear();
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyProfileChange(ProfileType profileType, ChangeType changeType,
    const std::string& dbKey, const std::string& dbValue)
{
    int32_t code = static_cast<int32_t>(profileType) * static_cast<int32_t>(changeType);
    DpRadarHelper::GetInstance().ReportNotifyProfileChange(code);
    switch (code) {
        case ProfileType::DEVICE_PROFILE * ChangeType::ADD:
            return SubscribeProfileManager::NotifyDeviceProfileAdd(dbKey, dbValue);
        case ProfileType::DEVICE_PROFILE * ChangeType::UPDATE:
            return SubscribeProfileManager::NotifyDeviceProfileUpdate(dbKey, dbValue);
        case ProfileType::DEVICE_PROFILE * ChangeType::DELETE:
            return SubscribeProfileManager::NotifyDeviceProfileDelete(dbKey, dbValue);
        case ProfileType::SERVICE_PROFILE * ChangeType::ADD:
            return SubscribeProfileManager::NotifyServiceProfileAdd(dbKey, dbValue);
        case ProfileType::SERVICE_PROFILE * ChangeType::UPDATE:
            return SubscribeProfileManager::NotifyServiceProfileUpdate(dbKey, dbValue);
        case ProfileType::SERVICE_PROFILE * ChangeType::DELETE:
            return SubscribeProfileManager::NotifyServiceProfileDelete(dbKey, dbValue);
        case ProfileType::CHAR_PROFILE * ChangeType::ADD:
            return SubscribeProfileManager::NotifyCharProfileAdd(dbKey, dbValue);
        case ProfileType::CHAR_PROFILE * ChangeType::UPDATE:
            return SubscribeProfileManager::NotifyCharProfileUpdate(dbKey, dbValue);
        case ProfileType::CHAR_PROFILE * ChangeType::DELETE:
            return SubscribeProfileManager::NotifyCharProfileDelete(dbKey, dbValue);
        default:
            HILOGE("Params is invalid!, code = %{public}u", code);
            return DP_INVALID_PARAMS;
    }
}

int32_t SubscribeProfileManager::NotifyTrustDeviceProfileAdd(const TrustDeviceProfile& trustDeviceProfile)
{
    auto subscriberInfos = GetSubscribeInfos(SUBSCRIBE_TRUST_DEVICE_PROFILE);
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", trustDeviceProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::TRUST_DEVICE_PROFILE_ADD) != 0) {
            listenerProxy->OnTrustDeviceProfileAdd(trustDeviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyTrustDeviceProfileUpdate(const TrustDeviceProfile& oldDeviceProfile,
    const TrustDeviceProfile& newDeviceProfile)
{
    auto subscriberInfos = GetSubscribeInfos(SUBSCRIBE_TRUST_DEVICE_PROFILE);
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", newDeviceProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::TRUST_DEVICE_PROFILE_UPDATE) != 0) {
            listenerProxy->OnTrustDeviceProfileUpdate(oldDeviceProfile, newDeviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyTrustDeviceProfileDelete(const TrustDeviceProfile& trustDeviceProfile)
{
    auto subscriberInfos = GetSubscribeInfos(SUBSCRIBE_TRUST_DEVICE_PROFILE);
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", trustDeviceProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::TRUST_DEVICE_PROFILE_DELETE) != 0) {
            listenerProxy->OnTrustDeviceProfileDelete(trustDeviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyTrustDeviceProfileActive(const TrustDeviceProfile& trustDeviceProfile)
{
    auto subscriberInfos = GetSubscribeInfos(SUBSCRIBE_TRUST_DEVICE_PROFILE);
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", trustDeviceProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::TRUST_DEVICE_PROFILE_ACTIVE) != 0) {
            listenerProxy->OnTrustDeviceProfileActive(trustDeviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyTrustDeviceProfileInactive(const TrustDeviceProfile& trustDeviceProfile)
{
    auto subscriberInfos = GetSubscribeInfos(SUBSCRIBE_TRUST_DEVICE_PROFILE);
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", trustDeviceProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::TRUST_DEVICE_PROFILE_INACTIVE) != 0) {
            listenerProxy->OnTrustDeviceProfileInactive(trustDeviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::SubscribeDeviceProfile(const SubscribeInfo& subscribeInfo)
{
    HILOGI("saId: %{public}d!, subscribeKey: %{public}s", subscribeInfo.GetSaId(),
        ProfileUtils::GetDbKeyAnonyString(subscribeInfo.GetSubscribeKey()).c_str());
    {
        std::lock_guard<std::mutex> lock(subscribeMutex_);
        if (subscribeInfoMap_.size() > MAX_LISTENER_SIZE) {
            HILOGE("SubscribeInfoMap size is invalid!size: %{public}zu!", subscribeInfoMap_.size());
            return DP_EXCEED_MAX_SIZE_FAIL;
        }
        if (subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].size() > MAX_LISTENER_SIZE) {
            HILOGE("SubscribeInfoMap size is invalid!size: %{public}zu!",
                subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].size());
            return DP_EXCEED_MAX_SIZE_FAIL;
        }
        if (subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].find(subscribeInfo) !=
            subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].end()) {
            HILOGI("this sa subscribeInfo is exist, saId : %{public}d", subscribeInfo.GetSaId());
            subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].erase(subscribeInfo);
        }
        subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].emplace(subscribeInfo);
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::SubscribeDeviceProfile(std::map<std::string, SubscribeInfo> subscribeInfos)
{
    HILOGD("call!");
    for (auto item : subscribeInfos) {
        SubscribeDeviceProfile(item.second);
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::UnSubscribeDeviceProfile(const SubscribeInfo& subscribeInfo)
{
    HILOGI("saId: %{public}d!, subscribeKey: %{public}s", subscribeInfo.GetSaId(),
        ProfileUtils::GetDbKeyAnonyString(subscribeInfo.GetSubscribeKey()).c_str());
    {
        std::lock_guard<std::mutex> lock(subscribeMutex_);
        if (subscribeInfoMap_.find(subscribeInfo.GetSubscribeKey()) != subscribeInfoMap_.end()) {
            subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].erase(subscribeInfo);
            if (subscribeInfoMap_[subscribeInfo.GetSubscribeKey()].empty()) {
                subscribeInfoMap_.erase(subscribeInfo.GetSubscribeKey());
            }
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyDeviceProfileAdd(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    DeviceProfile deviceProfile;
    deviceProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    deviceProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
    deviceProfile.SetIsMultiUser(deviceProfile.GetUserId() != DEFAULT_USER_ID);
    ProfileUtils::EntriesToDeviceProfile(values, deviceProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", deviceProfile.AnnoymizeDump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::DEVICE_PROFILE_ADD) != 0) {
            listenerProxy->OnDeviceProfileAdd(deviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyDeviceProfileUpdate(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    DeviceProfile newDeviceProfile;
    newDeviceProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    newDeviceProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
    newDeviceProfile.SetIsMultiUser(newDeviceProfile.GetUserId() != DEFAULT_USER_ID);
    ProfileUtils::EntriesToDeviceProfile(values, newDeviceProfile);
    DeviceProfile oldDeviceProfile;
    ProfileCache::GetInstance().GetDeviceProfile(ProfileUtils::GetDeviceIdByDBKey(dbKey), oldDeviceProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", newDeviceProfile.AnnoymizeDump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::DEVICE_PROFILE_UPDATE) != 0) {
            listenerProxy->OnDeviceProfileUpdate(oldDeviceProfile, newDeviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyDeviceProfileDelete(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    DeviceProfile deviceProfile;
    deviceProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    deviceProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
    deviceProfile.SetIsMultiUser(deviceProfile.GetUserId() != DEFAULT_USER_ID);
    ProfileUtils::EntriesToDeviceProfile(values, deviceProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", deviceProfile.AnnoymizeDump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::DEVICE_PROFILE_DELETE) != 0) {
            listenerProxy->OnDeviceProfileDelete(deviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyServiceProfileAdd(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    ServiceProfile serviceProfile;
    serviceProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    serviceProfile.SetServiceName(ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey));
    serviceProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
    serviceProfile.SetIsMultiUser(serviceProfile.GetUserId() != DEFAULT_USER_ID);
    ProfileUtils::EntriesToServiceProfile(values, serviceProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", serviceProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::SERVICE_PROFILE_ADD) != 0) {
            listenerProxy->OnServiceProfileAdd(serviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyServiceProfileUpdate(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    ServiceProfile newServiceProfile;
    newServiceProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    newServiceProfile.SetServiceName(ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey));
    newServiceProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
    newServiceProfile.SetIsMultiUser(newServiceProfile.GetUserId() != DEFAULT_USER_ID);
    ProfileUtils::EntriesToServiceProfile(values, newServiceProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", newServiceProfile.dump().c_str());
    ServiceProfile oldServiceProfile;
    ProfileCache::GetInstance().GetServiceProfile(ProfileUtils::GetDeviceIdByDBKey(dbKey),
        ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey), oldServiceProfile);
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::SERVICE_PROFILE_UPDATE) != 0) {
            listenerProxy->OnServiceProfileUpdate(oldServiceProfile, newServiceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyServiceProfileDelete(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    ServiceProfile serviceProfile;
    serviceProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    serviceProfile.SetServiceName(ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey));
    serviceProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
    serviceProfile.SetIsMultiUser(serviceProfile.GetUserId() != DEFAULT_USER_ID);
    ProfileUtils::EntriesToServiceProfile(values, serviceProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", serviceProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::SERVICE_PROFILE_DELETE) != 0) {
            listenerProxy->OnServiceProfileDelete(serviceProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyCharProfileAdd(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    CharacteristicProfile charProfile;
    charProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    charProfile.SetServiceName(ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey));
    charProfile.SetCharacteristicKey(ProfileUtils::GetCharKeyByDBKey(dbKey));
    if (charProfile.GetCharacteristicKey() != SWITCH_STATUS) {
        charProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
        charProfile.SetIsMultiUser(charProfile.GetUserId() != DEFAULT_USER_ID);
    }
    ProfileUtils::EntriesToCharProfile(values, charProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", charProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::CHAR_PROFILE_ADD) != 0) {
            listenerProxy->OnCharacteristicProfileAdd(charProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyCharProfileUpdate(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    CharacteristicProfile newCharProfile;
    newCharProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    newCharProfile.SetServiceName(ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey));
    newCharProfile.SetCharacteristicKey(ProfileUtils::GetCharKeyByDBKey(dbKey));
    if (newCharProfile.GetCharacteristicKey() != SWITCH_STATUS) {
        newCharProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
        newCharProfile.SetIsMultiUser(newCharProfile.GetUserId() != DEFAULT_USER_ID);
    }
    ProfileUtils::EntriesToCharProfile(values, newCharProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", newCharProfile.dump().c_str());
    CharacteristicProfile oldCharProfile;
    ProfileCache::GetInstance().GetCharacteristicProfile(ProfileUtils::GetDeviceIdByDBKey(dbKey),
        ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey), ProfileUtils::GetCharKeyByDBKey(dbKey), oldCharProfile);
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::CHAR_PROFILE_UPDATE) != 0) {
            listenerProxy->OnCharacteristicProfileUpdate(oldCharProfile, newCharProfile);
        }
    }
    return DP_SUCCESS;
}

int32_t SubscribeProfileManager::NotifyCharProfileDelete(const std::string& dbKey, const std::string& dbValue)
{
    std::map<std::string, std::string> values;
    values[dbKey] = dbValue;
    CharacteristicProfile charProfile;
    charProfile.SetDeviceId(ProfileUtils::GetDeviceIdByDBKey(dbKey));
    charProfile.SetServiceName(ProfileUtils::GetNonOhSuffixServiceNameByDBKey(dbKey));
    charProfile.SetCharacteristicKey(ProfileUtils::GetCharKeyByDBKey(dbKey));
    if (charProfile.GetCharacteristicKey() != SWITCH_STATUS) {
        charProfile.SetUserId(ProfileUtils::GetUserIdFromDbKey(dbKey));
        charProfile.SetIsMultiUser(charProfile.GetUserId() != DEFAULT_USER_ID);
    }
    ProfileUtils::EntriesToCharProfile(values, charProfile);
    auto subscriberInfos = GetSubscribeInfos(DBKeyToSubcribeKey(dbKey));
    if (subscriberInfos.empty()) {
        return DP_SUCCESS;
    }
    HILOGI("%{public}s!", charProfile.dump().c_str());
    for (const auto& subscriberInfo : subscriberInfos) {
        sptr<IProfileChangeListener> listenerProxy = iface_cast<IProfileChangeListener>(subscriberInfo.GetListener());
        if (listenerProxy == nullptr) {
            HILOGE("Cast to IProfileChangeListener failed!");
            continue;
        }
        if (subscriberInfo.GetProfileChangeTypes().count(ProfileChangeType::CHAR_PROFILE_DELETE) != 0) {
            listenerProxy->OnCharacteristicProfileDelete(charProfile);
        }
    }
    return DP_SUCCESS;
}
std::unordered_set<SubscribeInfo, SubscribeHash, SubscribeCompare> SubscribeProfileManager::GetSubscribeInfos(
    const std::string& dbKey)
{
    {
        std::lock_guard<std::mutex> lock(subscribeMutex_);
        if (subscribeInfoMap_.find(dbKey) == subscribeInfoMap_.end()) {
            HILOGD("This dbKey is not subscribed, dbKey: %{public}s", ProfileUtils::GetDbKeyAnonyString(dbKey).c_str());
            return {};
        }
        return subscribeInfoMap_[dbKey];
    }
}

std::string SubscribeProfileManager::DBKeyToSubcribeKey(const std::string& dbkey)
{
    std::string subscribeKey = dbkey;
    std::vector<std::string> res;
    if (ProfileUtils::SplitString(subscribeKey, SEPARATOR, res) != DP_SUCCESS) {
        return subscribeKey;
    }
    if (res.size() > NUM_2) {
        res[NUM_2] = ProfileUtils::CheckAndRemoveOhSuffix(res[NUM_2]);
        subscribeKey = ProfileUtils::JoinString(res, SEPARATOR);
    }
    return subscribeKey;
}
} // namespace DistributedDeviceProfile
} // namespace OHOS
