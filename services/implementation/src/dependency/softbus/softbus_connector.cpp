/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "softbus_connector.h"

#include <securec.h>
#include <unistd.h>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t SOFTBUS_SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t SOFTBUS_DISCOVER_DEVICE_INFO_MAX_SIZE = 20;

constexpr const char* WIFI_IP = "WIFI_IP";
constexpr const char* WIFI_PORT = "WIFI_PORT";
constexpr const char* BR_MAC = "BR_MAC";
constexpr const char* BLE_MAC = "BLE_MAC";
constexpr const char* ETH_IP = "ETH_IP";
constexpr const char* ETH_PORT = "ETH_PORT";

SoftbusConnector::PulishStatus SoftbusConnector::publishStatus = SoftbusConnector::STATUS_UNKNOWN;
std::map<std::string, std::shared_ptr<DeviceInfo>> SoftbusConnector::discoveryDeviceInfoMap_ = {};
std::map<std::string, std::shared_ptr<ISoftbusStateCallback>> SoftbusConnector::stateCallbackMap_ = {};
std::map<std::string, std::shared_ptr<ISoftbusDiscoveryCallback>> SoftbusConnector::discoveryCallbackMap_ = {};
std::map<std::string, std::shared_ptr<ISoftbusPublishCallback>> SoftbusConnector::publishCallbackMap_ = {};
std::mutex SoftbusConnector::discoveryDeviceInfoMutex_;
std::mutex SoftbusConnector::stateCallbackMutex_;
std::mutex SoftbusConnector::discoveryCallbackMutex_;
std::mutex SoftbusConnector::publishCallbackMutex_;

IPublishCb SoftbusConnector::softbusPublishCallback_ = {
    .OnPublishResult = SoftbusConnector::OnSoftbusPublishResult,
};
IRefreshCallback SoftbusConnector::softbusDiscoveryCallback_ = {
    .OnDeviceFound = SoftbusConnector::OnSoftbusDeviceFound,
    .OnDiscoverResult = SoftbusConnector::OnSoftbusDiscoveryResult,
};

SoftbusConnector::SoftbusConnector()
{
    softbusSession_ = std::make_shared<SoftbusSession>();
}

SoftbusConnector::~SoftbusConnector()
{
    LOGI("SoftbusConnector destructor");
}

int32_t SoftbusConnector::RegisterSoftbusDiscoveryCallback(const std::string &pkgName,
    const std::shared_ptr<ISoftbusDiscoveryCallback> callback)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
#endif

    discoveryCallbackMap_.emplace(pkgName, callback);
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusDiscoveryCallback(const std::string &pkgName)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
#endif

    discoveryCallbackMap_.erase(pkgName);
    return DM_OK;
}

int32_t SoftbusConnector::RegisterSoftbusPublishCallback(const std::string &pkgName,
    const std::shared_ptr<ISoftbusPublishCallback> callback)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(publishCallbackMutex_);
#endif

    publishCallbackMap_.emplace(pkgName, callback);
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusPublishCallback(const std::string &pkgName)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(publishCallbackMutex_);
#endif

    publishCallbackMap_.erase(pkgName);
    return DM_OK;
}

int32_t SoftbusConnector::RegisterSoftbusStateCallback(const std::string &pkgName,
    const std::shared_ptr<ISoftbusStateCallback> callback)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(stateCallbackMutex_);
#endif

    stateCallbackMap_.emplace(pkgName, callback);
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusStateCallback(const std::string &pkgName)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(stateCallbackMutex_);
#endif

    stateCallbackMap_.erase(pkgName);
    return DM_OK;
}

int32_t SoftbusConnector::PublishDiscovery(const DmPublishInfo &dmPublishInfo)
{
    PublishInfo publishInfo;
    (void)memset_s(&publishInfo, sizeof(PublishInfo), 0, sizeof(PublishInfo));
    publishInfo.publishId = dmPublishInfo.publishId;
    publishInfo.mode = (DiscoverMode)dmPublishInfo.mode;
    publishInfo.medium = ExchangeMedium::AUTO;
    publishInfo.freq = (ExchangeFreq)dmPublishInfo.freq;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.ranging = dmPublishInfo.ranging;
    LOGI("PublishDiscovery begin, publishId : %d, mode : 0x%x, ranging : %d", publishInfo.publishId, publishInfo.mode,
        publishInfo.ranging);
    int32_t ret = ::PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret != DM_OK) {
        LOGE("PublishLNN failed with ret %d.", ret);
        return ERR_DM_PUBLISH_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusConnector::UnPublishDiscovery(int32_t publishId)
{
    LOGI("UnPublishDiscovery begin, publishId : %d", publishId);
    int32_t ret = ::StopPublishLNN(DM_PKG_NAME, publishId);
    if (ret != DM_OK) {
        LOGE("StopPublishLNN failed with ret %d", ret);
        return ERR_DM_PUBLISH_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusConnector::StartDiscovery(const DmSubscribeInfo &dmSubscribeInfo)
{
    SubscribeInfo subscribeInfo;
    (void)memset_s(&subscribeInfo, sizeof(SubscribeInfo), 0, sizeof(SubscribeInfo));
    subscribeInfo.subscribeId = dmSubscribeInfo.subscribeId;
    subscribeInfo.mode = (DiscoverMode)dmSubscribeInfo.mode;
    subscribeInfo.medium = (ExchangeMedium)dmSubscribeInfo.medium;
    subscribeInfo.freq = (ExchangeFreq)dmSubscribeInfo.freq;
    subscribeInfo.isSameAccount = dmSubscribeInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubscribeInfo.isWakeRemote;
    subscribeInfo.capability = dmSubscribeInfo.capability;
    LOGI("StartDiscovery begin, subscribeId : %d, mode : 0x%x, medium : %d", subscribeInfo.subscribeId,
        subscribeInfo.mode, subscribeInfo.medium);
    int32_t ret = ::RefreshLNN(DM_PKG_NAME, &subscribeInfo, &softbusDiscoveryCallback_);
    if (ret != DM_OK) {
        LOGE("RefreshLNN failed with ret %d.", ret);
        return ERR_DM_DISCOVERY_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusConnector::StopDiscovery(uint16_t subscribeId)
{
    LOGI("StopDiscovery begin, subscribeId:%d", (int32_t)subscribeId);
    int32_t ret = ::StopRefreshLNN(DM_PKG_NAME, subscribeId);
    if (ret != DM_OK) {
        LOGE("StopRefreshLNN failed with ret %d", ret);
        return ERR_DM_DISCOVERY_FAILED;
    }
    return DM_OK;
}

void SoftbusConnector::JoinLnn(const std::string &deviceId)
{
    std::string connectAddr;
    LOGI("SoftbusConnector::JoinLnn, deviceId :%s", GetAnonyString(deviceId).c_str());
    ConnectionAddr *addrInfo = GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        return;
    }
    int32_t ret = ::JoinLNN(DM_PKG_NAME, addrInfo, OnSoftbusJoinLNNResult);
    if (ret != DM_OK) {
        LOGE("JoinLNN failed with ret %d", ret);
    }
    return;
}

int32_t SoftbusConnector::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    LOGI("GetUdidByNetworkId for networkId = %s", GetAnonyString(std::string(networkId)).c_str());
    uint8_t mUdid[UDID_BUF_LEN] = {0};
    int32_t ret =
        GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid));
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }
    udid = reinterpret_cast<char *>(mUdid);
    return DM_OK;
}

int32_t SoftbusConnector::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    LOGI("GetUuidByNetworkId for networkId = %s", GetAnonyString(std::string(networkId)).c_str());
    uint8_t mUuid[UUID_BUF_LEN] = {0};
    int32_t ret =
        GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UUID, mUuid, sizeof(mUuid));
    if (ret != DM_OK) {
        LOGE("GetUuidByNetworkId GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }
    uuid = reinterpret_cast<char *>(mUuid);
    return DM_OK;
}

bool SoftbusConnector::IsDeviceOnLine(const std::string &deviceId)
{
    NodeBasicInfo *info = nullptr;
    int32_t infoNum = 0;
    if (GetAllNodeDeviceInfo(DM_PKG_NAME, &info, &infoNum) != DM_OK) {
        LOGE("GetAllNodeDeviceInfo failed");
        return false;
    }
    bool bDeviceOnline = false;
    for (int32_t i = 0; i < infoNum; ++i) {
        NodeBasicInfo *nodeBasicInfo = info + i;
        if (nodeBasicInfo == nullptr) {
            LOGE("nodeBasicInfo is empty for index %d, infoNum %d.", i, infoNum);
            continue;
        }
        std::string networkId = nodeBasicInfo->networkId;
        if (networkId == deviceId) {
            LOGI("DM_IsDeviceOnLine device %s online", GetAnonyString(deviceId).c_str());
            bDeviceOnline = true;
            break;
        }
        uint8_t udid[UDID_BUF_LEN] = {0};
        int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId.c_str(), NodeDeviceInfoKey::NODE_KEY_UDID, udid,
                                     sizeof(udid));
        if (ret != DM_OK) {
            LOGE("DM_IsDeviceOnLine GetNodeKeyInfo failed");
            break;
        }
        if (strcmp(reinterpret_cast<char *>(udid), deviceId.c_str()) == 0) {
            LOGI("DM_IsDeviceOnLine device %s online", GetAnonyString(deviceId).c_str());
            bDeviceOnline = true;
            break;
        }
    }
    FreeNodeInfo(info);
    return bDeviceOnline;
}

std::shared_ptr<SoftbusSession> SoftbusConnector::GetSoftbusSession()
{
    return softbusSession_;
}

bool SoftbusConnector::HaveDeviceInMap(std::string deviceId)
{
    std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
    auto iter = discoveryDeviceInfoMap_.find(deviceId);
    if (iter == discoveryDeviceInfoMap_.end()) {
        LOGE("deviceInfo not found by deviceId %s", GetAnonyString(deviceId).c_str());
        return false;
    }
    return true;
}

int32_t SoftbusConnector::GetConnectionIpAddress(const std::string &deviceId, std::string &ipAddress)
{
    DeviceInfo *deviceInfo = nullptr;
    {
        std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
        auto iter = discoveryDeviceInfoMap_.find(deviceId);
        if (iter == discoveryDeviceInfoMap_.end()) {
            LOGE("deviceInfo not found by deviceId %s", GetAnonyString(deviceId).c_str());
            return ERR_DM_FAILED;
        }
        deviceInfo = iter->second.get();
    }

    if (deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        LOGE("deviceInfo address num not valid, addrNum %d", deviceInfo->addrNum);
        return ERR_DM_FAILED;
    }
    for (uint32_t i = 0; i < deviceInfo->addrNum; ++i) {
        // currently, only support CONNECT_ADDR_WLAN
        if (deviceInfo->addr[i].type != ConnectionAddrType::CONNECTION_ADDR_WLAN &&
            deviceInfo->addr[i].type != ConnectionAddrType::CONNECTION_ADDR_ETH) {
            continue;
        }
        ipAddress = deviceInfo->addr[i].info.ip.ip;
        LOGI("DM_GetConnectionIpAddr get ip ok.");
        return DM_OK;
    }
    LOGE("failed to get ipAddress for deviceId %s", GetAnonyString(deviceId).c_str());
    return ERR_DM_FAILED;
}

ConnectionAddr *SoftbusConnector::GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type)
{
    if (deviceInfo == nullptr) {
        return nullptr;
    }
    for (uint32_t i = 0; i < deviceInfo->addrNum; ++i) {
        if (deviceInfo->addr[i].type == type) {
            return &deviceInfo->addr[i];
        }
    }
    return nullptr;
}

ConnectionAddr *SoftbusConnector::GetConnectAddr(const std::string &deviceId, std::string &connectAddr)
{
    DeviceInfo *deviceInfo = nullptr;
    {
        std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
        auto iter = discoveryDeviceInfoMap_.find(deviceId);
        if (iter == discoveryDeviceInfoMap_.end()) {
            LOGE("deviceInfo not found by deviceId %s", GetAnonyString(deviceId).c_str());
            return nullptr;
        }
        deviceInfo = iter->second.get();
    }

    if (deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        LOGE("deviceInfo addrNum not valid, addrNum %d", deviceInfo->addrNum);
        return nullptr;
    }
    nlohmann::json jsonPara;
    ConnectionAddr *addr = nullptr;
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_ETH);
    if (addr != nullptr) {
        LOGI("get ETH ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        jsonPara[ETH_IP] = addr->info.ip.ip;
        jsonPara[ETH_PORT] = addr->info.ip.port;
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_WLAN);
    if (addr != nullptr) {
        jsonPara[WIFI_IP] = addr->info.ip.ip;
        jsonPara[WIFI_PORT] = addr->info.ip.port;
        LOGI("get WLAN ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BR);
    if (addr != nullptr) {
        jsonPara[BR_MAC] = addr->info.br.brMac;
        LOGI("get BR ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BLE);
    if (addr != nullptr) {
        jsonPara[BLE_MAC] = addr->info.ble.bleMac;
        LOGI("get BLE ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        connectAddr = jsonPara.dump();
        return addr;
    }
    LOGE("failed to get ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
    return nullptr;
}

void SoftbusConnector::ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceInfo &dmDeviceInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.deviceId, sizeof(dmDeviceInfo.deviceId), deviceInfo.devId,
                 std::min(sizeof(dmDeviceInfo.deviceId), sizeof(deviceInfo.devId))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceId data failed");
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), deviceInfo.devName,
                 std::min(sizeof(dmDeviceInfo.deviceName), sizeof(deviceInfo.devName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed");
    }
    dmDeviceInfo.deviceTypeId = deviceInfo.devType;
    dmDeviceInfo.range = deviceInfo.range;
}

void SoftbusConnector::HandleDeviceOnline(const DmDeviceInfo &info)
{
    LOGI("HandleDeviceOnline: start handle device online event.");
    {
        std::lock_guard<std::mutex> lock(stateCallbackMutex_);
        for (auto &iter : stateCallbackMap_) {
            iter.second->OnDeviceOnline(iter.first, info);
        }
    }

    {
        std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
        if (discoveryDeviceInfoMap_.empty()) {
            return;
        }
    }
    uint8_t udid[UDID_BUF_LEN] = {0};
    int32_t ret =
        GetNodeKeyInfo(DM_PKG_NAME, info.networkId, NodeDeviceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
    if (ret != DM_OK) {
        LOGE("GetNodeKeyInfo failed");
        return;
    }
    std::string deviceId = reinterpret_cast<char *>(udid);
    LOGI("device online, deviceId: %s", GetAnonyString(deviceId).c_str());
    {
        std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
        discoveryDeviceInfoMap_.erase(deviceId);
    }
}

void SoftbusConnector::HandleDeviceOffline(const DmDeviceInfo &info)
{
    LOGI("HandleDeviceOffline: start handle device offline event.");
    {
        std::lock_guard<std::mutex> lock(stateCallbackMutex_);
        for (auto &iter : stateCallbackMap_) {
            iter.second->OnDeviceOffline(iter.first, info);
        }
    }
}

void SoftbusConnector::OnSoftbusPublishResult(int32_t publishId, PublishResult result)
{
    LOGI("Callback In, publishId %d, result %d", publishId, result);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(publishCallbackMutex_);
#endif

    for (auto &iter : publishCallbackMap_) {
        iter.second->OnPublishResult(iter.first, publishId, result);
    }
}

void SoftbusConnector::OnSoftbusJoinLNNResult(ConnectionAddr *addr, const char *networkId, int32_t result)
{
    LOGI("SoftbusConnector::OnSoftbusJoinLNNResult, result = %d.", result);
}

void SoftbusConnector::OnSoftbusDeviceFound(const DeviceInfo *device)
{
    if (device == nullptr) {
        LOGE("device is null");
        return;
    }
    std::string deviceId = device->devId;
    LOGI("SoftbusConnector::OnSoftbusDeviceFound device %s found, range : %d.", GetAnonyString(deviceId).c_str(),
        device->range);

    if (!IsDeviceOnLine(deviceId)) {
        std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
        DeviceInfo *srcInfo = infoPtr.get();
        if (memcpy_s(srcInfo, sizeof(DeviceInfo), device, sizeof(DeviceInfo)) != 0) {
            LOGE("save discovery device info failed");
            return;
        }
        {
            std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
            discoveryDeviceInfoMap_[deviceId] = infoPtr;
            // Remove the earliest element when reached the max size
            if (discoveryDeviceInfoMap_.size() == SOFTBUS_DISCOVER_DEVICE_INFO_MAX_SIZE) {
                auto iter = discoveryDeviceInfoMap_.begin();
                discoveryDeviceInfoMap_.erase(iter->second->devId);
            }
        }
    }

    DmDeviceInfo dmDeviceInfo;
    ConvertDeviceInfoToDmDevice(*device, dmDeviceInfo);
    {
        std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
        for (auto &iter : discoveryCallbackMap_) {
            iter.second->OnDeviceFound(iter.first, dmDeviceInfo);
        }
    }
}

void SoftbusConnector::OnSoftbusDiscoveryResult(int subscribeId, RefreshResult result)
{
    LOGI("In, subscribeId %d, result %d", subscribeId, result);
    uint16_t originId = (uint16_t)(((uint32_t)subscribeId) & SOFTBUS_SUBSCRIBE_ID_MASK);
    {
        std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
        if (result == REFRESH_LNN_SUCCESS) {
            for (auto &iter : discoveryCallbackMap_) {
                iter.second->OnDiscoverySuccess(iter.first, originId);
            }
        } else {
            for (auto &iter : discoveryCallbackMap_) {
                iter.second->OnDiscoveryFailed(iter.first, originId, result);
            }
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
