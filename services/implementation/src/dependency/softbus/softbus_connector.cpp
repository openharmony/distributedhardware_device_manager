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
const int32_t SOFTBUS_DISCOVER_DEVICE_INFO_MAX_SIZE = 100;

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
std::queue<std::string> SoftbusConnector::discoveryDeviceIdQueue_ = {};
std::mutex SoftbusConnector::discoveryCallbackMutex_;
std::mutex SoftbusConnector::discoveryDeviceInfoMutex_;
std::mutex SoftbusConnector::stateCallbackMutex_;

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
    LOGD("SoftbusConnector constructor.");
}

SoftbusConnector::~SoftbusConnector()
{
    LOGD("SoftbusConnector destructor.");
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
    std::mutex registerCallback;
    std::lock_guard<std::mutex> lock(registerCallback);
#endif

    publishCallbackMap_.emplace(pkgName, callback);
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusPublishCallback(const std::string &pkgName)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::mutex unRegisterCallback;
    std::lock_guard<std::mutex> lock(unRegisterCallback);
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
    publishInfo.mode = static_cast<DiscoverMode>(dmPublishInfo.mode);
    publishInfo.medium = ExchangeMedium::AUTO;
    publishInfo.freq = static_cast<ExchangeFreq>(dmPublishInfo.freq);
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.ranging = dmPublishInfo.ranging;
    LOGI("start, publishId: %d, mode: 0x%x, ranging: %d.", publishInfo.publishId, publishInfo.mode,
        publishInfo.ranging);
    int32_t ret = ::PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]PublishLNN failed, ret %d.", ret);
        return ERR_DM_PUBLISH_FAILED;
    }
    return ret;
}

int32_t SoftbusConnector::UnPublishDiscovery(int32_t publishId)
{
    LOGI("UnPublishDiscovery begin, publishId: %d.", publishId);
    int32_t ret = ::StopPublishLNN(DM_PKG_NAME, publishId);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopPublishLNN failed with ret: %d.", ret);
        return ERR_DM_PUBLISH_FAILED;
    }
    return ret;
}

int32_t SoftbusConnector::StartDiscovery(const DmSubscribeInfo &dmSubscribeInfo)
{
    SubscribeInfo subscribeInfo;
    (void)memset_s(&subscribeInfo, sizeof(SubscribeInfo), 0, sizeof(SubscribeInfo));
    subscribeInfo.subscribeId = dmSubscribeInfo.subscribeId;
    subscribeInfo.mode = static_cast<DiscoverMode>(dmSubscribeInfo.mode);
    subscribeInfo.medium = static_cast<ExchangeMedium>(dmSubscribeInfo.medium);
    subscribeInfo.freq = static_cast<ExchangeFreq>(dmSubscribeInfo.freq);
    subscribeInfo.isSameAccount = dmSubscribeInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubscribeInfo.isWakeRemote;
    subscribeInfo.capability = dmSubscribeInfo.capability;
    LOGI("StartDiscovery begin, subscribeId: %d, mode: 0x%x, medium: %d.", subscribeInfo.subscribeId,
        subscribeInfo.mode, subscribeInfo.medium);
    int32_t ret = ::RefreshLNN(DM_PKG_NAME, &subscribeInfo, &softbusDiscoveryCallback_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]RefreshLNN failed, ret: %d.", ret);
        return ERR_DM_DISCOVERY_FAILED;
    }
    return ret;
}

int32_t SoftbusConnector::StopDiscovery(uint16_t subscribeId)
{
    LOGI("StopDiscovery begin, subscribeId: %d.", (int32_t)subscribeId);
    int32_t ret = ::StopRefreshLNN(DM_PKG_NAME, subscribeId);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopRefreshLNN failed, ret: %d.", ret);
        return ERR_DM_DISCOVERY_FAILED;
    }
    return ret;
}

void SoftbusConnector::JoinLnn(const std::string &deviceId)
{
    std::string connectAddr;
    LOGI("start, deviceId: %s.", GetAnonyString(deviceId).c_str());
    ConnectionAddr *addrInfo = GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("addrInfo is nullptr.");
        return;
    }
    int32_t ret = ::JoinLNN(DM_PKG_NAME, addrInfo, OnSoftbusJoinLNNResult);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]JoinLNN failed, ret: %d.", ret);
    }
    return;
}

int32_t SoftbusConnector::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    LOGI("start, networkId: %s.", GetAnonyString(std::string(networkId)).c_str());
    uint8_t tmpUdid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UDID, tmpUdid, sizeof(tmpUdid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    udid = reinterpret_cast<char *>(tmpUdid);
    return ret;
}

int32_t SoftbusConnector::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    LOGI("start, networkId: %s.", GetAnonyString(std::string(networkId)).c_str());
    uint8_t tmpUuid[UUID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UUID, tmpUuid, sizeof(tmpUuid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    uuid = reinterpret_cast<char *>(tmpUuid);
    return ret;
}

bool SoftbusConnector::IsDeviceOnLine(const std::string &deviceId)
{
    NodeBasicInfo *info = nullptr;
    int32_t infoNum = 0;
    LOGI("start, deviceId: %s.", GetAnonyString(deviceId).c_str());
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &info, &infoNum);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %d.", ret);
        return false;
    }
    bool bDeviceOnline = false;
    for (int32_t i = 0; i < infoNum; ++i) {
        NodeBasicInfo *nodeBasicInfo = info + i;
        if (nodeBasicInfo == nullptr) {
            LOGE("[SOFTBUS]nodeBasicInfo is empty for index: %d, infoNum: %d.", i, infoNum);
            continue;
        }
        std::string networkId = nodeBasicInfo->networkId;
        if (networkId == deviceId) {
            LOGI("[SOFTBUS]DM_IsDeviceOnLine device: %s online.", GetAnonyString(deviceId).c_str());
            bDeviceOnline = true;
            break;
        }
        uint8_t udid[UDID_BUF_LEN] = {0};
        ret = GetNodeKeyInfo(DM_PKG_NAME, networkId.c_str(), NodeDeviceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
        if (ret != DM_OK) {
            LOGE("[SOFTBUS]DM_IsDeviceOnLine GetNodeKeyInfo failed, ret: %d.", ret);
            break;
        }
        if (strcmp(reinterpret_cast<char *>(udid), deviceId.c_str()) == DM_OK) {
            LOGI("DM_IsDeviceOnLine device: %s online.", GetAnonyString(deviceId).c_str());
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
#endif

    auto iter = discoveryDeviceInfoMap_.find(deviceId);
    if (iter == discoveryDeviceInfoMap_.end()) {
        LOGE("deviceInfo not found by deviceId: %s.", GetAnonyString(deviceId).c_str());
        return false;
    }
    return true;
}

int32_t SoftbusConnector::GetConnectionIpAddress(const std::string &deviceId, std::string &ipAddress)
{
    DeviceInfo *deviceInfo = nullptr;
    {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
#endif

        auto iter = discoveryDeviceInfoMap_.find(deviceId);
        if (iter == discoveryDeviceInfoMap_.end()) {
            LOGE("deviceInfo not found by deviceId: %s.", GetAnonyString(deviceId).c_str());
            return ERR_DM_FAILED;
        }
        deviceInfo = iter->second.get();
    }

    if (deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        LOGE("deviceInfo address num not valid, addrNum: %d.", deviceInfo->addrNum);
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
    LOGE("failed to get ipAddress for deviceId: %s.", GetAnonyString(deviceId).c_str());
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
#endif
        auto iter = discoveryDeviceInfoMap_.find(deviceId);
        if (iter == discoveryDeviceInfoMap_.end()) {
            LOGE("deviceInfo not found by deviceId: %s.", GetAnonyString(deviceId).c_str());
            return nullptr;
        }
        deviceInfo = iter->second.get();
    }
    if (deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        LOGE("deviceInfo addrNum not valid, addrNum: %d.", deviceInfo->addrNum);
        return nullptr;
    }
    nlohmann::json jsonPara;
    ConnectionAddr *addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_ETH);
    if (addr != nullptr) {
        LOGI("[SOFTBUS]get ETH ConnectionAddr for deviceId: %s.", GetAnonyString(deviceId).c_str());
        jsonPara[ETH_IP] = addr->info.ip.ip;
        jsonPara[ETH_PORT] = addr->info.ip.port;
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_WLAN);
    if (addr != nullptr) {
        jsonPara[WIFI_IP] = addr->info.ip.ip;
        jsonPara[WIFI_PORT] = addr->info.ip.port;
        LOGI("[SOFTBUS]get WLAN ConnectionAddr for deviceId: %s.", GetAnonyString(deviceId).c_str());
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BR);
    if (addr != nullptr) {
        jsonPara[BR_MAC] = addr->info.br.brMac;
        LOGI("[SOFTBUS]get BR ConnectionAddr for deviceId: %s.", GetAnonyString(deviceId).c_str());
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BLE);
    if (addr != nullptr) {
        jsonPara[BLE_MAC] = addr->info.ble.bleMac;
        connectAddr = jsonPara.dump();
        return addr;
    }
    LOGE("[SOFTBUS]failed to get ConnectionAddr for deviceId: %s.", GetAnonyString(deviceId).c_str());
    return nullptr;
}

void SoftbusConnector::ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceInfo &dmDeviceInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.deviceId, sizeof(dmDeviceInfo.deviceId), deviceInfo.devId,
                 std::min(sizeof(dmDeviceInfo.deviceId), sizeof(deviceInfo.devId))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceId data failed.");
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), deviceInfo.devName,
                 std::min(sizeof(dmDeviceInfo.deviceName), sizeof(deviceInfo.devName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
    }
    dmDeviceInfo.deviceTypeId = deviceInfo.devType;
    dmDeviceInfo.range = deviceInfo.range;
}

void SoftbusConnector::HandleDeviceOnline(DmDeviceInfo &info)
{
    LOGI("start handle device online event.");
    {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<std::mutex> lock(stateCallbackMutex_);
#endif
        for (auto &iter : stateCallbackMap_) {
            iter.second->OnDeviceOnline(iter.first, info);
        }
    }
    uint8_t udid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, info.networkId, NodeDeviceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %d.", ret);
        return;
    }
    std::string deviceId = reinterpret_cast<char *>(udid);
    LOGI("device online, deviceId: %s.", GetAnonyString(deviceId).c_str());
}

void SoftbusConnector::HandleDeviceOffline(const DmDeviceInfo &info)
{
    LOGI("start handle device offline event.");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(stateCallbackMutex_);
#endif

    for (auto &iter : stateCallbackMap_) {
        iter.second->OnDeviceOffline(iter.first, info);
    }
}

void SoftbusConnector::OnSoftbusPublishResult(int32_t publishId, PublishResult result)
{
    LOGI("Callback In, publishId: %d, result: %d.", publishId, result);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::mutex publishResult;
    std::lock_guard<std::mutex> lock(publishResult);
#endif

    for (auto &iter : publishCallbackMap_) {
        iter.second->OnPublishResult(iter.first, publishId, result);
    }
}

void SoftbusConnector::OnSoftbusJoinLNNResult(ConnectionAddr *addr, const char *networkId, int32_t result)
{
    (void)addr;
    (void)networkId;
    LOGD("[SOFTBUS]OnSoftbusJoinLNNResult, result: %d.", result);
}

void SoftbusConnector::OnSoftbusDeviceFound(const DeviceInfo *device)
{
    if (device == nullptr) {
        LOGE("[SOFTBUS]device is null.");
        return;
    }
    std::string deviceId = device->devId;
    LOGI("[SOFTBUS]notify found device: %s found, range: %d.", GetAnonyString(deviceId).c_str(), device->range);
    if (!IsDeviceOnLine(deviceId)) {
        std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
        DeviceInfo *srcInfo = infoPtr.get();
        int32_t ret = memcpy_s(srcInfo, sizeof(DeviceInfo), device, sizeof(DeviceInfo));
        if (ret != DM_OK) {
            LOGE("save discovery device info failed, ret: %d.", ret);
            return;
        }
        {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
#endif

            if (discoveryDeviceInfoMap_.find(deviceId) == discoveryDeviceInfoMap_.end()) {
                discoveryDeviceIdQueue_.emplace(deviceId);
            }
            discoveryDeviceInfoMap_[deviceId] = infoPtr;

            // Remove the earliest element when reached the max size
            if (discoveryDeviceIdQueue_.size() == SOFTBUS_DISCOVER_DEVICE_INFO_MAX_SIZE) {
                discoveryDeviceInfoMap_.erase(discoveryDeviceIdQueue_.front());
                discoveryDeviceIdQueue_.pop();
            }
        }
    }

    DmDeviceInfo dmDeviceInfo;
    ConvertDeviceInfoToDmDevice(*device, dmDeviceInfo);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
#endif

    for (auto &iter : discoveryCallbackMap_) {
        iter.second->OnDeviceFound(iter.first, dmDeviceInfo);
    }
}

void SoftbusConnector::OnSoftbusDiscoveryResult(int subscribeId, RefreshResult result)
{
    uint16_t originId = static_cast<uint16_t>((static_cast<uint32_t>(subscribeId)) & SOFTBUS_SUBSCRIBE_ID_MASK);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
#endif

    if (result == REFRESH_LNN_SUCCESS) {
        LOGI("[SOFTBUS]start to discovery device successfully with subscribeId: %d, result: %d.", subscribeId, result);
        for (auto &iter : discoveryCallbackMap_) {
            iter.second->OnDiscoverySuccess(iter.first, originId);
        }
    } else {
        LOGE("[SOFTBUS]fail to discovery device with subscribeId: %d, result: %d.", subscribeId, result);
        for (auto &iter : discoveryCallbackMap_) {
            iter.second->OnDiscoveryFailed(iter.first, originId, result);
        }
    }
}

std::string SoftbusConnector::GetLocalDeviceName()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %d.", ret);
        return "";
    }
    return nodeBasicInfo.deviceName;
}

int32_t SoftbusConnector::GetLocalDeviceTypeId()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %d.", ret);
        return DmDeviceType::DEVICE_TYPE_UNKNOWN;
    }
    return nodeBasicInfo.deviceTypeId;
}
} // namespace DistributedHardware
} // namespace OHOS
