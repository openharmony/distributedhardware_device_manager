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

#include "softbus_connector.h"

#include <securec.h>
#include <unistd.h>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t SOFTBUS_SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t SOFTBUS_DISCOVER_DEVICE_INFO_MAX_SIZE = 100;
const int32_t SOFTBUS_TRUSTDEVICE_UUIDHASH_INFO_MAX_SIZE = 100;

constexpr const char* WIFI_IP = "WIFI_IP";
constexpr const char* WIFI_PORT = "WIFI_PORT";
constexpr const char* BR_MAC = "BR_MAC";
constexpr const char* BLE_MAC = "BLE_MAC";
constexpr const char* ETH_IP = "ETH_IP";
constexpr const char* ETH_PORT = "ETH_PORT";

SoftbusConnector::PulishStatus SoftbusConnector::publishStatus = SoftbusConnector::STATUS_UNKNOWN;
std::map<std::string, std::shared_ptr<DeviceInfo>> SoftbusConnector::discoveryDeviceInfoMap_ = {};
std::map<std::string, std::shared_ptr<ISoftbusDiscoveryCallback>> SoftbusConnector::discoveryCallbackMap_ = {};
std::map<std::string, std::shared_ptr<ISoftbusPublishCallback>> SoftbusConnector::publishCallbackMap_ = {};
std::queue<std::string> SoftbusConnector::discoveryDeviceIdQueue_ = {};
std::unordered_map<std::string, std::string> SoftbusConnector::deviceUdidMap_ = {};
std::vector<std::string> SoftbusConnector::pkgNameVec_ = {};
std::mutex SoftbusConnector::discoveryCallbackMutex_;
std::mutex SoftbusConnector::discoveryDeviceInfoMutex_;
std::mutex SoftbusConnector::deviceUdidLocks_;
std::mutex SoftbusConnector::pkgNameVecMutex_;

IPublishCb SoftbusConnector::softbusPublishCallback_ = {
    .OnPublishResult = SoftbusConnector::OnSoftbusPublishResult,
};
IRefreshCallback SoftbusConnector::softbusDiscoveryCallback_ = {
    .OnDeviceFound = SoftbusConnector::OnSoftbusDeviceFound,
    .OnDiscoverResult = SoftbusConnector::OnSoftbusDiscoveryResult,
};

IRefreshCallback SoftbusConnector::softbusDiscoveryByIdCallback_ = {
    .OnDeviceFound = SoftbusConnector::OnSoftbusDeviceDiscovery,
    .OnDiscoverResult = SoftbusConnector::OnSoftbusDiscoveryResult,
};

SoftbusConnector::SoftbusConnector()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    softbusSession_ = std::make_shared<SoftbusSession>();
#endif
    LOGD("SoftbusConnector constructor.");
}

SoftbusConnector::~SoftbusConnector()
{
    LOGD("SoftbusConnector destructor.");
}

int32_t SoftbusConnector::RegisterSoftbusStateCallback(const std::shared_ptr<ISoftbusStateCallback> callback)
{
    deviceStateManagerCallback_ = callback;
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusStateCallback()
{
    deviceStateManagerCallback_ = nullptr;
    return DM_OK;
}

int32_t SoftbusConnector::RegisterSoftbusDiscoveryCallback(const std::string &pkgName,
    const std::shared_ptr<ISoftbusDiscoveryCallback> callback)
{
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
    discoveryCallbackMap_.emplace(pkgName, callback);
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusDiscoveryCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
    discoveryCallbackMap_.erase(pkgName);
    return DM_OK;
}

int32_t SoftbusConnector::RegisterSoftbusPublishCallback(const std::string &pkgName,
    const std::shared_ptr<ISoftbusPublishCallback> callback)
{
    std::mutex registerCallback;
    std::lock_guard<std::mutex> lock(registerCallback);
    publishCallbackMap_.emplace(pkgName, callback);
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusPublishCallback(const std::string &pkgName)
{
    std::mutex unRegisterCallback;
    std::lock_guard<std::mutex> lock(unRegisterCallback);
    publishCallbackMap_.erase(pkgName);
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
    LOGI("start, publishId: %{public}d, mode: 0x%{public}x, ranging: %{public}d.", publishInfo.publishId,
        publishInfo.mode, publishInfo.ranging);
    int32_t ret = ::PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]PublishLNN failed, ret %{public}d.", ret);
        return ERR_DM_PUBLISH_FAILED;
    }
    return ret;
}

int32_t SoftbusConnector::UnPublishDiscovery(int32_t publishId)
{
    LOGI("UnPublishDiscovery begin, publishId: %{public}d.", publishId);
    int32_t ret = ::StopPublishLNN(DM_PKG_NAME, publishId);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopPublishLNN failed with ret: %{public}d.", ret);
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
    LOGI("StartDiscovery begin, subscribeId: %{public}d, mode: 0x%{public}x, medium: %{public}d.",
        subscribeInfo.subscribeId, subscribeInfo.mode, subscribeInfo.medium);
    int32_t ret = ::RefreshLNN(DM_PKG_NAME, &subscribeInfo, &softbusDiscoveryCallback_);
    struct RadarInfo info = {
        .funcName = "StartDiscovery",
        .toCallPkg = SOFTBUSNAME,
        .stageRes = (ret == DM_OK) ?
            static_cast<int32_t>(StageRes::STAGE_IDLE) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .bizState = (ret == DM_OK) ?
            static_cast<int32_t>(BizState::BIZ_STATE_START) : static_cast<int32_t>(BizState::BIZ_STATE_END),
        .commServ = static_cast<int32_t>(CommServ::USE_SOFTBUS),
        .errCode = ret,
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverRegCallback(info)) {
        LOGE("ReportDiscoverRegCallback failed");
    }
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]RefreshLNN failed, ret: %{public}d.", ret);
        return ERR_DM_DISCOVERY_FAILED;
    }
    return ret;
}

int32_t SoftbusConnector::StartDiscovery(const uint16_t subscribeId)
{
    SubscribeInfo subscribeInfo;
    (void)memset_s(&subscribeInfo, sizeof(SubscribeInfo), 0, sizeof(SubscribeInfo));
    subscribeInfo.subscribeId = subscribeId;
    subscribeInfo.mode = static_cast<DiscoverMode>(DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE);
    subscribeInfo.medium = static_cast<ExchangeMedium>(DmExchangeMedium::DM_AUTO);
    subscribeInfo.freq = static_cast<ExchangeFreq>(DmExchangeFreq::DM_SUPER_HIGH);
    subscribeInfo.isSameAccount = false;
    subscribeInfo.isWakeRemote = false;
    subscribeInfo.capability = DM_CAPABILITY_OSD;
    LOGI("StartDiscovery by subscribeId begin, subscribeId: %{public}d, mode: 0x%{public}x, medium: %{public}d.",
        subscribeId, subscribeInfo.mode, subscribeInfo.medium);
    int32_t ret = ::RefreshLNN(DM_PKG_NAME, &subscribeInfo, &softbusDiscoveryByIdCallback_);
    struct RadarInfo info = {
        .funcName = "StartDiscovery",
        .toCallPkg = SOFTBUSNAME,
        .stageRes = (ret == DM_OK) ?
            static_cast<int32_t>(StageRes::STAGE_IDLE) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .bizState = (ret == DM_OK) ?
            static_cast<int32_t>(BizState::BIZ_STATE_START) : static_cast<int32_t>(BizState::BIZ_STATE_END),
        .commServ = static_cast<int32_t>(CommServ::USE_SOFTBUS),
        .errCode = ret,
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverRegCallback(info)) {
        LOGE("ReportDiscoverRegCallback failed");
    }
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]RefreshLNN failed, ret: %{public}d.", ret);
        return ERR_DM_DISCOVERY_FAILED;
    }
    return ret;
}

int32_t SoftbusConnector::StopDiscovery(uint16_t subscribeId)
{
    LOGI("StopDiscovery begin, subscribeId: %{public}d.", (int32_t)subscribeId);
    int32_t ret = ::StopRefreshLNN(DM_PKG_NAME, subscribeId);
    struct RadarInfo info = {
        .funcName = "StopDiscovery",
        .hostName = SOFTBUSNAME,
        .stageRes = (ret == DM_OK) ?
            static_cast<int32_t>(StageRes::STAGE_CANCEL) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .bizState = (ret == DM_OK) ?
            static_cast<int32_t>(BizState::BIZ_STATE_CANCEL) : static_cast<int32_t>(BizState::BIZ_STATE_END),
        .errCode = ret,
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverUserRes(info)) {
        LOGE("ReportDiscoverUserRes failed");
    }
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopRefreshLNN failed, ret: %{public}d.", ret);
        return ERR_DM_DISCOVERY_FAILED;
    }
    return ret;
}

void SoftbusConnector::JoinLnn(const std::string &deviceId)
{
    std::string connectAddr;
    LOGI("start, deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
    ConnectionAddr *addrInfo = GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("addrInfo is nullptr.");
        return;
    }
    int32_t ret = ::JoinLNN(DM_PKG_NAME, addrInfo, OnSoftbusJoinLNNResult);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]JoinLNN failed, ret: %{public}d.", ret);
    }
    return;
}

int32_t SoftbusConnector::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    LOGI("start, networkId: %{public}s.", GetAnonyString(std::string(networkId)).c_str());
    uint8_t tmpUdid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UDID, tmpUdid, sizeof(tmpUdid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    udid = reinterpret_cast<char *>(tmpUdid);
    return ret;
}

int32_t SoftbusConnector::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    LOGI("start, networkId: %{public}s.", GetAnonyString(std::string(networkId)).c_str());
    uint8_t tmpUuid[UUID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UUID, tmpUuid, sizeof(tmpUuid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    uuid = reinterpret_cast<char *>(tmpUuid);
    return ret;
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
std::shared_ptr<SoftbusSession> SoftbusConnector::GetSoftbusSession()
{
    return softbusSession_;
}
#endif

bool SoftbusConnector::HaveDeviceInMap(std::string deviceId)
{
    std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
    auto iter = discoveryDeviceInfoMap_.find(deviceId);
    if (iter == discoveryDeviceInfoMap_.end()) {
        LOGE("deviceInfo not found by deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        return false;
    }
    return true;
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
            LOGE("deviceInfo not found by deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
            return nullptr;
        }
        deviceInfo = iter->second.get();
    }
    if (deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        LOGE("deviceInfo addrNum not valid, addrNum: %{public}d.", deviceInfo->addrNum);
        return nullptr;
    }
    nlohmann::json jsonPara;
    ConnectionAddr *addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_ETH);
    if (addr != nullptr) {
        LOGI("[SOFTBUS]get ETH ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        jsonPara[ETH_IP] = addr->info.ip.ip;
        jsonPara[ETH_PORT] = addr->info.ip.port;
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_WLAN);
    if (addr != nullptr) {
        jsonPara[WIFI_IP] = addr->info.ip.ip;
        jsonPara[WIFI_PORT] = addr->info.ip.port;
        LOGI("[SOFTBUS]get WLAN ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BR);
    if (addr != nullptr) {
        jsonPara[BR_MAC] = addr->info.br.brMac;
        LOGI("[SOFTBUS]get BR ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        connectAddr = jsonPara.dump();
        return addr;
    }
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BLE);
    if (addr != nullptr) {
        jsonPara[BLE_MAC] = addr->info.ble.bleMac;
        connectAddr = jsonPara.dump();
        return addr;
    }
    LOGE("[SOFTBUS]failed to get ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
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

void SoftbusConnector::ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceBasicInfo &dmDeviceBasicInfo)
{
    (void)memset_s(&dmDeviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    if (memcpy_s(dmDeviceBasicInfo.deviceId, sizeof(dmDeviceBasicInfo.deviceId), deviceInfo.devId,
                 std::min(sizeof(dmDeviceBasicInfo.deviceId), sizeof(deviceInfo.devId))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceId data failed.");
    }

    if (memcpy_s(dmDeviceBasicInfo.deviceName, sizeof(dmDeviceBasicInfo.deviceName), deviceInfo.devName,
                 std::min(sizeof(dmDeviceBasicInfo.deviceName), sizeof(deviceInfo.devName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
    }
    dmDeviceBasicInfo.deviceTypeId = deviceInfo.devType;
}

void SoftbusConnector::OnSoftbusPublishResult(int32_t publishId, PublishResult result)
{
    LOGI("Callback In, publishId: %{public}d, result: %{public}d.", publishId, result);
    std::mutex publishResult;
    std::lock_guard<std::mutex> lock(publishResult);
    for (auto &iter : publishCallbackMap_) {
        iter.second->OnPublishResult(iter.first, publishId, result);
    }
}

void SoftbusConnector::OnSoftbusJoinLNNResult(ConnectionAddr *addr, const char *networkId, int32_t result)
{
    (void)addr;
    (void)networkId;
    LOGD("[SOFTBUS]OnSoftbusJoinLNNResult, result: %{public}d.", result);
}

void SoftbusConnector::OnSoftbusDeviceFound(const DeviceInfo *device)
{
    if (device == nullptr) {
        LOGE("[SOFTBUS]device is null.");
        return;
    }
    std::string deviceId = device->devId;
    LOGI("[SOFTBUS]notify found device: %{public}s found, range: %{public}d, isOnline: %{public}d.",
        GetAnonyString(deviceId).c_str(), device->range, device->isOnline);
    if (!device->isOnline) {
        std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
        DeviceInfo *srcInfo = infoPtr.get();
        int32_t ret = memcpy_s(srcInfo, sizeof(DeviceInfo), device, sizeof(DeviceInfo));
        if (ret != DM_OK) {
            LOGE("save discovery device info failed, ret: %{public}d.", ret);
            return;
        }
        {
            std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
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
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    struct RadarInfo info = {
        .funcName = "OnSoftbusDeviceFound",
        .peerNetId = (nodeInfo != nullptr) ? std::string(nodeInfo->networkId) : "",
        .peerUdid = device->devId,
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverResCallback(info)) {
        LOGE("ReportDiscoverResCallback failed");
    }
    for (auto &iter : discoveryCallbackMap_) {
        iter.second->OnDeviceFound(iter.first, dmDeviceInfo, device->isOnline);
    }
}

void SoftbusConnector::OnSoftbusDeviceDiscovery(const DeviceInfo *device)
{
    if (device == nullptr) {
        LOGE("[SOFTBUS]device is null.");
        return;
    }
    std::string deviceId = device->devId;
    LOGI("[SOFTBUS]notify discover device: %{public}s found, range: %{public}d, isOnline: %{public}d.",
        GetAnonyString(deviceId).c_str(), device->range, device->isOnline);
    if (!device->isOnline) {
        std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
        DeviceInfo *srcInfo = infoPtr.get();
        int32_t ret = memcpy_s(srcInfo, sizeof(DeviceInfo), device, sizeof(DeviceInfo));
        if (ret != DM_OK) {
            LOGE("save discovery device info failed, ret: %{public}d.", ret);
            return;
        }
        {
            std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
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

    DmDeviceBasicInfo dmDeviceBasicInfo;
    ConvertDeviceInfoToDmDevice(*device, dmDeviceBasicInfo);
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    struct RadarInfo info = {
        .funcName = "OnSoftbusDeviceDiscovery",
        .peerNetId = (nodeInfo != nullptr) ? std::string(nodeInfo->networkId) : "",
        .peerUdid = device->devId,

    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverResCallback(info)) {
        LOGE("ReportDiscoverResCallback failed");
    }
    for (auto &iter : discoveryCallbackMap_) {
        iter.second->OnDeviceFound(iter.first, dmDeviceBasicInfo, device->range, device->isOnline);
    }
}

void SoftbusConnector::OnSoftbusDiscoveryResult(int subscribeId, RefreshResult result)
{
    uint16_t originId = static_cast<uint16_t>((static_cast<uint32_t>(subscribeId)) & SOFTBUS_SUBSCRIBE_ID_MASK);
    std::lock_guard<std::mutex> lock(discoveryCallbackMutex_);
    if (result == REFRESH_LNN_SUCCESS) {
        LOGI("[SOFTBUS]start to discovery device successfully with subscribeId: %{public}d, result: %{public}d.",
            subscribeId, result);
        for (auto &iter : discoveryCallbackMap_) {
            iter.second->OnDiscoverySuccess(iter.first, originId);
        }
    } else {
        LOGE("[SOFTBUS]fail to discovery device with subscribeId: %{public}d, result: %{public}d.", subscribeId,
            result);
        for (auto iter = discoveryCallbackMap_.begin(); iter != discoveryCallbackMap_.end();) {
            iter->second->OnDiscoveryFailed(iter->first, originId, result);
            iter = discoveryCallbackMap_.erase(iter);
        }
        struct RadarInfo info = {
            .funcName = "OnSoftbusDiscoveryResult",
            .errCode = result,
        };
        if (!DmRadarHelper::GetInstance().ReportDiscoverResCallback(info)) {
            LOGE("ReportDiscoverResCallback failed");
        }
    }
}

std::string SoftbusConnector::GetDeviceUdidByUdidHash(const std::string &udidHash)
{
    std::lock_guard<std::mutex> lock(deviceUdidLocks_);
    for (auto &iter : deviceUdidMap_) {
        if (iter.second == udidHash) {
            return iter.first;
        }
    }
    LOGE("fail to GetUdidByUdidHash, udidHash: %{public}s", GetAnonyString(udidHash).c_str());
    return udidHash;
}

std::string SoftbusConnector::GetDeviceUdidHashByUdid(const std::string &udid)
{
    {
        std::lock_guard<std::mutex> lock(deviceUdidLocks_);
        auto iter = deviceUdidMap_.find(udid);
        if (iter != deviceUdidMap_.end()) {
            return deviceUdidMap_[udid];
        }
    }

    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(udid, reinterpret_cast<uint8_t *>(udidHash)) != DM_OK) {
        LOGE("get udidhash by udid: %{public}s failed.", GetAnonyString(udid).c_str());
        return "";
    }
    LOGI("get udidhash: %{public}s by udid: %{public}s.", GetAnonyString(udidHash).c_str(),
        GetAnonyString(udid).c_str());
    std::lock_guard<std::mutex> lock(deviceUdidLocks_);
    deviceUdidMap_[udid] = udidHash;
    return udidHash;
}

void SoftbusConnector::EraseUdidFromMap(const std::string &udid)
{
    std::lock_guard<std::mutex> lock(deviceUdidLocks_);
    auto iter = deviceUdidMap_.find(udid);
    if (iter == deviceUdidMap_.end()) {
        return;
    }
    size_t mapSize = deviceUdidMap_.size();
    if (mapSize >= SOFTBUS_TRUSTDEVICE_UUIDHASH_INFO_MAX_SIZE) {
        deviceUdidMap_.erase(udid);
    }
}

std::string SoftbusConnector::GetLocalDeviceName()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        return "";
    }
    return nodeBasicInfo.deviceName;
}

int32_t SoftbusConnector::GetLocalDeviceTypeId()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        return DmDeviceType::DEVICE_TYPE_UNKNOWN;
    }
    return nodeBasicInfo.deviceTypeId;
}

std::string SoftbusConnector::GetLocalDeviceNetworkId()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalDeviceNetworkId failed, ret: %{public}d.", ret);
        return "";
    }
    return nodeBasicInfo.networkId;
}

int32_t SoftbusConnector::AddMemberToDiscoverMap(const std::string &deviceId, std::shared_ptr<DeviceInfo> deviceInfo)
{
    if (deviceId.empty()) {
        LOGE("AddMemberToDiscoverMap failed, deviceId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
    discoveryDeviceInfoMap_[deviceId] = std::move(deviceInfo);
    deviceInfo = nullptr;
    return DM_OK;
}

std::string SoftbusConnector::GetNetworkIdByDeviceId(const std::string &deviceId)
{
    LOGI("SoftbusConnector::GetNetworkIdByDeviceId");
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    if (GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount) != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed.");
        return "";
    }
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        uint8_t mUdid[UDID_BUF_LEN] = {0};
        if (GetNodeKeyInfo(DM_PKG_NAME, reinterpret_cast<char *>(nodeBasicInfo->networkId),
            NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid)) != DM_OK) {
            LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
        }
        if (reinterpret_cast<char *>(mUdid) == deviceId) {
            return static_cast<std::string>(nodeBasicInfo->networkId);
        }
    }
    return "";
}

void SoftbusConnector::SetPkgName(std::string pkgName)
{
    LOGI("SoftbusConnector::SetPkgName");
    std::lock_guard<std::mutex> lock(pkgNameVecMutex_);
    pkgNameVec_.push_back(pkgName);
}

void SoftbusConnector::SetPkgNameVec(std::vector<std::string> pkgNameVec)
{
    LOGI("SoftbusConnector::SetPkgNameVec");
    std::lock_guard<std::mutex> lock(pkgNameVecMutex_);
    pkgNameVec_ = pkgNameVec;
}

std::vector<std::string> SoftbusConnector::GetPkgName()
{
    LOGI("SoftbusConnector::GetPkgName");
    std::lock_guard<std::mutex> lock(pkgNameVecMutex_);
    return pkgNameVec_;
}

void SoftbusConnector::ClearPkgName()
{
    LOGI("SoftbusConnector::SetPkgName vec");
    std::lock_guard<std::mutex> lock(pkgNameVecMutex_);
    pkgNameVec_.clear();
}

void SoftbusConnector::HandleDeviceOnline(std::string deviceId)
{
    LOGI("SoftbusConnector::HandleDeviceOnline");
    deviceStateManagerCallback_->OnDeviceOnline(deviceId);
    return;
}

void SoftbusConnector::HandleDeviceOffline(std::string deviceId)
{
    LOGI("SoftbusConnector::HandleDeviceOffline");
    deviceStateManagerCallback_->OnDeviceOffline(deviceId);
    return;
}

bool SoftbusConnector::CheckIsOnline(const std::string &targetDeviceId)
{
    LOGI("Check the device is online.");
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    if (GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount) != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed.");
        return ERR_DM_FAILED;
    }
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        uint8_t mUdid[UDID_BUF_LEN] = {0};
        if (GetNodeKeyInfo(DM_PKG_NAME, reinterpret_cast<char *>(nodeBasicInfo->networkId),
            NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid)) != DM_OK) {
            LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
        }
        std::string udid = reinterpret_cast<char *>(mUdid);
        if (udid == targetDeviceId) {
            LOGI("The device is online.");
            return true;
        }
    }
    LOGI("The device is not online.");
    return false;
}

DmDeviceInfo SoftbusConnector::GetDeviceInfoByDeviceId(const std::string &deviceId)
{
    LOGI("SoftbusConnector::GetDeviceInfoBydeviceId");
    DmDeviceInfo info;
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    if (GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount) != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed.");
        return info;
    }
    char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(deviceId, reinterpret_cast<uint8_t *>(deviceIdHash)) != DM_OK) {
        LOGE("get deviceIdHash by deviceId: %{public}s failed.", GetAnonyString(deviceId).c_str());
        return info;
    }
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        uint8_t mUdid[UDID_BUF_LEN] = {0};
        if (GetNodeKeyInfo(DM_PKG_NAME, nodeBasicInfo->networkId, NodeDeviceInfoKey::NODE_KEY_UDID,
            mUdid, sizeof(mUdid)) != DM_OK) {
            LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
            return info;
        }
        std::string udid = reinterpret_cast<char *>(mUdid);
        if (udid != deviceId) {
            continue;
        } else {
            ConvertNodeBasicInfoToDmDevice(*nodeBasicInfo, info);
            if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, deviceIdHash, DM_MAX_DEVICE_ID_LEN) != 0) {
                LOGE("Get deviceId: %{public}s failed.", GetAnonyString(deviceId).c_str());
            }
            break;
        }
    }
    FreeNodeInfo(nodeInfo);
    return info;
}

void SoftbusConnector::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), nodeBasicInfo.networkId,
                 std::min(sizeof(dmDeviceInfo.networkId), sizeof(nodeBasicInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceId data failed.");
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), nodeBasicInfo.deviceName,
                 std::min(sizeof(dmDeviceInfo.deviceName), sizeof(nodeBasicInfo.deviceName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
    }
    dmDeviceInfo.deviceTypeId = nodeBasicInfo.deviceTypeId;
    std::string extraData = dmDeviceInfo.extraData;
    nlohmann::json extraJson;
    if (!extraData.empty()) {
        extraJson = nlohmann::json::parse(extraData, nullptr, false);
    }
    if (!extraJson.is_discarded()) {
        extraJson[PARAM_KEY_OS_TYPE] = nodeBasicInfo.osType;
        extraJson[PARAM_KEY_OS_VERSION] = std::string(nodeBasicInfo.osVersion);
        dmDeviceInfo.extraData = to_string(extraJson);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
