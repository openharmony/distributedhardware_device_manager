/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(SoftbusCache);
bool g_online = false;
bool g_getLocalDevInfo = false;
DmDeviceInfo localDeviceInfo_;
void SoftbusCache::SaveLocalDeviceInfo()
{
    LOGI("SoftbusCache::SaveLocalDeviceInfo");
    if (g_online) {
        return;
    }
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        return;
    }
    ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, localDeviceInfo_);
    LOGI("SoftbusCache::SaveLocalDeviceInfo networkid %{public}s.",
        GetAnonyString(std::string(localDeviceInfo_.networkId)).c_str());
    SaveDeviceInfo(localDeviceInfo_);
    SaveDeviceSecurityLevel(localDeviceInfo_.networkId);
    g_online = true;
    g_getLocalDevInfo = true;
}

void SoftbusCache::DeleteLocalDeviceInfo()
{
    LOGI("SoftbusCache::DeleteLocalDeviceInfo networkid %{public}s.",
        GetAnonyString(std::string(localDeviceInfo_.networkId)).c_str());
    g_online = false;
    g_getLocalDevInfo = false;
}

int32_t SoftbusCache::GetLocalDeviceInfo(DmDeviceInfo &nodeInfo)
{
    LOGI("SoftbusCache::GetLocalDeviceInfo");
    if (g_getLocalDevInfo) {
        nodeInfo = localDeviceInfo_;
        LOGI("SoftbusCache::GetLocalDeviceInfo from dm cache.");
        return DM_OK;
    }
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        return ret;
    }
    ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, localDeviceInfo_);
    nodeInfo = localDeviceInfo_;
    SaveDeviceInfo(localDeviceInfo_);
    SaveDeviceSecurityLevel(localDeviceInfo_.networkId);
    g_getLocalDevInfo = true;
    return DM_OK;
}

void SoftbusCache::UpDataLocalDevInfo()
{
    LOGI("SoftbusCache::UpDataLocalDevInfo");
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        return;
    }
    ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, localDeviceInfo_);
    ChangeDeviceInfo(localDeviceInfo_);
}

int32_t SoftbusCache::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    uint8_t mUdid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
        return ret;
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
        return ret;
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
    deviceInfo_[udid] = std::pair<std::string, DmDeviceInfo>(uuid, deviceInfo);
    LOGI("SaveDeviceInfo success udid %{public}s, networkId %{public}s",
        GetAnonyString(udid).c_str(), GetAnonyString(std::string(deviceInfo.networkId)).c_str());
}

void SoftbusCache::DeleteDeviceInfo(const DmDeviceInfo &nodeInfo)
{
    LOGI("SoftbusCache::DeleteDeviceInfo networkId %{public}s",
        GetAnonyString(std::string(nodeInfo.networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    for (const auto &item : deviceInfo_) {
        if (std::string(item.second.second.networkId) == std::string(nodeInfo.networkId)) {
            LOGI("DeleteDeviceInfo success udid %{public}s", GetAnonyString(item.first).c_str());
            deviceInfo_.erase(item.first);
        }
    }
}

void SoftbusCache::ChangeDeviceInfo(const DmDeviceInfo deviceInfo)
{
    LOGI("SoftbusCache::ChangeDeviceInfo");
    std::string udid = "";
    GetUdidByNetworkId(deviceInfo.networkId, udid);
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
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
    LOGI("ChangeDeviceInfo sucess udid %{public}s, networkId %{public}s.",
        GetAnonyString(udid).c_str(), GetAnonyString(std::string(deviceInfo.networkId)).c_str());
}

int32_t SoftbusCache::GetDeviceInfoFromCache(std::vector<DmDeviceInfo> &deviceInfoList)
{
    LOGI("SoftbusCache::GetDeviceInfoFromCache.");
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    for (const auto &item : deviceInfo_) {
        if (std::string(item.second.second.networkId) == std::string(localDeviceInfo_.networkId)) {
            continue;
        }
        deviceInfoList.push_back(item.second.second);
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
    if (deviceCount != 0) {
        SaveLocalDeviceInfo();
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
    for (const auto &item : deviceInfo_) {
        if (std::string(item.second.second.networkId) == std::string(networkId)) {
            udid = item.first;
            LOGI("GetUdidFromCache success udid %{public}s.", GetAnonyString(udid).c_str());
            return DM_OK;
        }
    }
    if (GetUdidByNetworkId(networkId, udid) == DM_OK) {
        LOGI("Get udid from softbus success.");
        return DM_OK;
    }
    return ERR_DM_FAILED;
}

int32_t SoftbusCache::GetUuidFromCache(const char *networkId, std::string &uuid)
{
    LOGI("SoftbusCache::GetUuidFromCache networkId %{public}s", GetAnonyString(std::string(networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
    for (const auto &item : deviceInfo_) {
        if (std::string(item.second.second.networkId) == std::string(networkId)) {
            uuid = item.second.first;
            LOGI("GetUuidFromCache success uuid %{public}s.", GetAnonyString(uuid).c_str());
            return DM_OK;
        }
    }
    if (GetUuidByNetworkId(networkId, uuid) == DM_OK) {
        LOGI("Get uuid from softbus success.");
        return DM_OK;
    }
    return ERR_DM_FAILED;
}

int32_t SoftbusCache::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceInfo &devInfo)
{
    (void)memset_s(&devInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(devInfo.networkId, sizeof(devInfo.networkId), nodeInfo.networkId,
        std::min(sizeof(devInfo.networkId), sizeof(nodeInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
    }

    if (memcpy_s(devInfo.deviceName, sizeof(devInfo.deviceName), nodeInfo.deviceName,
        std::min(sizeof(devInfo.deviceName), sizeof(nodeInfo.deviceName))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed.");
    }
    devInfo.deviceTypeId = nodeInfo.deviceTypeId;
    nlohmann::json extraJson;
    extraJson[PARAM_KEY_OS_TYPE] = nodeInfo.osType;
    extraJson[PARAM_KEY_OS_VERSION] = std::string(nodeInfo.osVersion);
    devInfo.extraData = to_string(extraJson);
    return DM_OK;
}

void SoftbusCache::SaveDeviceSecurityLevel(const char *networkId)
{
    LOGI("SoftbusCache::SaveDeviceSecurityLevel networkId %{public}s.", GetAnonyString(std::string(networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceSecurityLevelMutex_);
    if (deviceSecurityLevel_.find(std::string(networkId)) != deviceSecurityLevel_.end()) {
        return;
    }
    int32_t tempSecurityLevel = -1;
    if (GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_DEVICE_SECURITY_LEVEL,
        reinterpret_cast<uint8_t *>(&tempSecurityLevel), LNN_COMMON_LEN) != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
        return;
    }
    deviceSecurityLevel_[std::string(networkId)] = tempSecurityLevel;
}

void SoftbusCache::DeleteDeviceSecurityLevel(const char *networkId)
{
    LOGI("SoftbusCache::DeleteDeviceSecurityLevel networkId %{public}s.",
        GetAnonyString(std::string(networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceSecurityLevelMutex_);
    if (deviceSecurityLevel_.find(std::string(networkId)) != deviceSecurityLevel_.end()) {
        deviceSecurityLevel_.erase(std::string(networkId));
    }
}

int32_t SoftbusCache::GetSecurityDeviceLevel(const char *networkId, int32_t &securityLevel)
{
    LOGI("SoftbusCache::GetSecurityDeviceLevel networkId %{public}s.", GetAnonyString(std::string(networkId)).c_str());
    std::lock_guard<std::mutex> mutexLock(deviceSecurityLevelMutex_);
    for (const auto &item : deviceSecurityLevel_) {
        if (item.first == std::string(networkId)) {
            securityLevel = item.second;
            return DM_OK;
        }
    }
    if (GetDevLevelFromBus(networkId, securityLevel) == DM_OK) {
        LOGI("Get dev level from softbus success.");
        return DM_OK;
    }
    return ERR_DM_FAILED;
}

int32_t SoftbusCache::GetDevLevelFromBus(const char *networkId, int32_t &securityLevel)
{
    LOGI("SoftbusCache::GetDevLevelFromBus");
    int32_t tempSecurityLevel = -1;
    if (GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_DEVICE_SECURITY_LEVEL,
        reinterpret_cast<uint8_t *>(&tempSecurityLevel), LNN_COMMON_LEN) != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
        return ERR_DM_FAILED;
    }
    securityLevel = tempSecurityLevel;
    deviceSecurityLevel_[std::string(networkId)] = tempSecurityLevel;
    return DM_OK;
}

int32_t SoftbusCache::GetDevInfoByNetworkId(const std::string &networkId, DmDeviceInfo &nodeInfo)
{
    LOGI("SoftbusCache::GetDevInfoByNetworkId networkId %{public}s.", GetAnonyString(networkId).c_str());
    {
        std::lock_guard<std::mutex> mutexLock(deviceInfosMutex_);
        for (const auto &item : deviceInfo_) {
            if (std::string(item.second.second.networkId) == networkId) {
                nodeInfo = item.second.second;
                LOGI("GetDevInfoByNetworkId success udid %{public}s.", GetAnonyString(item.first).c_str());
                return DM_OK;
            }
        }
    }
    int32_t ret = GetDevInfoFromBus(networkId, nodeInfo);
    if (ret != DM_OK) {
        LOGE("GetDevInfoFromBus failed.");
        return ret;
    }
    SaveDeviceInfo(nodeInfo);
    return DM_OK;
}

int32_t SoftbusCache::GetDevInfoFromBus(const std::string &networkId, DmDeviceInfo &devInfo)
{
    int32_t nodeInfoCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &nodeInfoCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %{public}d.", ret);
        return ret;
    }
    for (int32_t i = 0; i < nodeInfoCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        if (networkId == std::string(nodeBasicInfo->networkId)) {
            ConvertNodeBasicInfoToDmDevice(*nodeBasicInfo, devInfo);
            break;
        }
    }
    FreeNodeInfo(nodeInfo);
    LOGI("GetDeviceInfo complete, deviceName : %{public}s, deviceTypeId : %{public}d.",
        GetAnonyString(devInfo.deviceName).c_str(), devInfo.deviceTypeId);
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS