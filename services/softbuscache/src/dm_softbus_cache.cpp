/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "dm_softbus_cache.h"
#include "dm_anonymous.h"
#include "dm_crypto.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(PermissionManager);

int32_t SoftbusCache::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    uint8_t mUdid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    udid = reinterpret_cast<char *>(mUdid);
    return ret;
}

int32_t SoftbusCache::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    uint8_t mUuid[UUID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UUID, mUuid, sizeof(mUuid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    uuid = reinterpret_cast<char *>(mUuid);
    return ret;
}

void SoftbusCache::SaveDeviceInfo(DmDeviceInfo deviceInfo)
{
    LOGI("SoftbusCache::SaveDeviceInfo");
    std::string udid = "";
    std::string uuid = "";
    GetUdidByNetworkId(deviceInfo.networkId, udid);
    GetUuidByNetworkId(deviceInfo.networkId, uuid);
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(udid, reinterpret_cast<uint8_t *>(udidHash)) != DM_OK) {
        LOGE("get udidhash by udid: %{public}s failed.", GetAnonyString(udid).c_str());
        return;
    }
    if (memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), udidHash,
        std::min(sizeof(deviceInfo.deviceId), sizeof(udidHash))) != DM_OK) {
        LOGE("SaveDeviceInfo copy deviceId failed.");
        return;
    }
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    {
        deviceInfo_[udid] = std::pair<std::string, DmDeviceInfo>(uuid, deviceInfo);
    }
    LOGI("SaveDeviceInfo success udid %{public}s, networkId %{public}s",
        GetAnonyString(udid).c_str(), GetAnonyString(std::string(deviceInfo.networkId)).c_str());
}

void SoftbusCache::DeleteDeviceInfo(const DmDeviceInfo &nodeInfo)
{
    LOGI("SoftbusCache::DeleteDeviceInfo networkId %{public}s",
        GetAnonyString(std::string(nodeInfo.networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    {
        for (const auto &item : deviceInfo_) {
            if (item.second.second.networkId == nodeInfo.networkId) {
                LOGI("DeleteDeviceInfo success udid %{public}s", GetAnonyString(item.first).c_str());
                deviceInfo_.erase(item.first);
            }
        }
    }
}

void SoftbusCache::ChangeDeviceInfo(const DmDeviceInfo deviceInfo)
{
    LOGI("SoftbusCache::ChangeDeviceInfo");
    std::string udid = "";
    GetUdidByNetworkId(deviceInfo.networkId, udid);
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    {
        if (deviceInfo_.find(udid) != deviceInfo_.end()) {
            if (memcpy_s(deviceInfo_[udid].second.deviceName, sizeof(deviceInfo_[udid].second.deviceName),
                deviceInfo.deviceName, sizeof(deviceInfo.deviceName)) != DM_OK) {
                LOGE("ChangeDeviceInfo deviceInfo copy deviceName failed");
            }
            if (memcpy_s(deviceInfo_[udid].second.networkId, sizeof(deviceInfo_[udid].second.networkId),
                deviceInfo.networkId, sizeof(deviceInfo.networkId)) != DM_OK) {
                LOGE("ChangeDeviceInfo deviceInfo copy networkId failed");
            }
            deviceInfo_[udid].second.deviceTypeId = deviceInfo.deviceTypeId;
        }
    }
    LOGI("ChangeDeviceInfo sucess udid %{public}s, networkId %{public}s.",
        GetAnonyString(udid).c_str(), GetAnonyString(std::string(deviceInfo.networkId)).c_str());
}

int32_t SoftbusCache::GetDeviceInfoFromCache(std::vector<DmDeviceInfo> &deviceInfoList)
{
    LOGI("SoftbusCache::GetDeviceInfoFromCache deviceInfo size is %{public}d.", deviceInfo_.size());
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    {
        if (deviceInfo_.empty()) {
            return ERR_DM_FAILED;
        }
        for (const auto &item : deviceInfo_) {
            deviceInfoList.push_back(item.second.second);
        }
    }
    return DM_OK;
}

void SoftbusCache::UpdateDeviceInfoCache()
{
    LOGI("SoftbusCache::UpdateDeviceInfoCache");
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %{public}d.", ret);
        return;
    }
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        DmDeviceInfo deviceInfo;
        ConvertNodeBasicInfoToDmDevice(*nodeBasicInfo, deviceInfo);
        SaveDeviceInfo(deviceInfo);
    }
    FreeNodeInfo(nodeInfo);
    LOGI("UpdateDeviceInfoCache success, deviceCount: %{public}d.", deviceCount);
    return;
}

int32_t SoftbusCache::GetUdidFromCache(const char *networkId, std::string &udid)
{
    LOGI("SoftbusCache::GetUdidFromCache networkId %{public}s", GetAnonyString(std::string(networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    {
        for (const auto &item : deviceInfo_) {
            if (std::string(item.second.second.networkId) == std::string(networkId)) {
                udid = item.first;
                LOGI("GetUdidFromCache success udid %{public}s.", GetAnonyString(udid).c_str());
                return DM_OK;
            }
        }
    }
    return ERR_DM_FAILED;
}

int32_t SoftbusCache::GetUuidFromCache(const char *networkId, std::string &uuid)
{
    LOGI("SoftbusCache::GetUuidFromCache networkId %{public}s", GetAnonyString(std::string(networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    {
        for (const auto &item : deviceInfo_) {
            if (std::string(item.second.second.networkId) == std::string(networkId)) {
                uuid = item.second.first;
                LOGI("GetUuidFromCache success uuid %{public}s.", GetAnonyString(uuid).c_str());
                return DM_OK;
            }
        }
    }
    return ERR_DM_FAILED;
}
} // namespace DistributedHardware
} // namespace OHOS
