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

#include "softbus_listener.h"

#include <securec.h>
#include <unistd.h>
#if defined(__LITEOS_M__)
#include "dm_mutex.h"
#include "dm_thread.h"
#else
#include <pthread.h>
#include <thread>
#include <mutex>
#endif

#include "device_manager_service.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t DISCOVER_STATUS_LEN = 20;
const int32_t SOFTBUS_CHECK_INTERVAL = 100000; // 100ms

constexpr const char* DISCOVER_STATUS_KEY = "persist.distributed_hardware.device_manager.discover_status";
constexpr const char* DISCOVER_STATUS_ON = "1";
constexpr const char* DISCOVER_STATUS_OFF = "0";
constexpr const char* DEVICE_ONLINE = "deviceOnLine";
constexpr const char* DEVICE_OFFLINE = "deviceOffLine";
constexpr const char* DEVICE_NAME_CHANGE = "deviceNameChange";

SoftbusListener::PulishStatus SoftbusListener::publishStatus = SoftbusListener::STATUS_UNKNOWN;
IPublishCb SoftbusListener::softbusPublishCallback_ = {
    .OnPublishResult = SoftbusListener::OnPublishResult,
};

INodeStateCb SoftbusListener::softbusNodeStateCb_ = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE | EVENT_NODE_STATE_INFO_CHANGED,
    .onNodeOnline = SoftbusListener::OnSoftBusDeviceOnline,
    .onNodeOffline = SoftbusListener::OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = SoftbusListener::OnSoftbusDeviceInfoChanged};

void DeviceOnLine(DmDeviceInfo deviceInfo)
{
#if defined(__LITEOS_M__)
    DmMutex lockDeviceOnLine;
#else
    std::mutex lockDeviceOnLine;
    std::lock_guard<std::mutex> lock(lockDeviceOnLine);
#endif
    DeviceManagerService::GetInstance().HandleDeviceOnline(deviceInfo);
}

void DeviceOffLine(DmDeviceInfo deviceInfo)
{
#if defined(__LITEOS_M__)
    DmMutex lockDeviceOffLine;
#else
    std::mutex lockDeviceOffLine;
    std::lock_guard<std::mutex> lock(lockDeviceOffLine);
#endif
    DeviceManagerService::GetInstance().HandleDeviceOffline(deviceInfo);
}

void DeviceNameChange(DmDeviceInfo deviceInfo)
{
#if defined(__LITEOS_M__)
    DmMutex lockDeviceOffLine;
#else
    std::mutex lockDeviceOffLine;
    std::lock_guard<std::mutex> lock(lockDeviceOffLine);
#endif
    DeviceManagerService::GetInstance().HandleDeviceNameChange(deviceInfo);
}

SoftbusListener::SoftbusListener()
{
    ISessionListener sessionListener = {.OnSessionOpened = SoftbusListener::OnSessionOpened,
                                        .OnSessionClosed = SoftbusListener::OnSessionClosed,
                                        .OnBytesReceived = SoftbusListener::OnBytesReceived,
                                        .OnMessageReceived = nullptr,
                                        .OnStreamReceived = nullptr};
    LOGD("SoftbusListener constructor.");
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_SESSION_NAME, &sessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer failed, ret: %d.", ret);
    } else {
        LOGI("[SOFTBUS]CreateSessionServer ok.");
    }
    Init();
}

SoftbusListener::~SoftbusListener()
{
    RemoveSessionServer(DM_PKG_NAME, DM_SESSION_NAME);
    LOGD("SoftbusListener destructor.");
}

void SoftbusListener::SetPublishInfo(PublishInfo &dmPublishInfo)
{
    dmPublishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    dmPublishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    dmPublishInfo.medium = ExchangeMedium::AUTO;
    dmPublishInfo.freq = ExchangeFreq::HIGH;
    dmPublishInfo.capability = DM_CAPABILITY_OSD;
    dmPublishInfo.ranging = false;
    return;
}

int32_t SoftbusListener::Init()
{
    int32_t ret;
    int32_t retryTimes = 0;
    do {
        ret = RegNodeDeviceStateCb(DM_PKG_NAME, &softbusNodeStateCb_);
        if (ret != DM_OK) {
            ++retryTimes;
            LOGE("[SOFTBUS]RegNodeDeviceStateCb failed with ret: %d, retryTimes: %d.", ret, retryTimes);
            usleep(SOFTBUS_CHECK_INTERVAL);
        }
    } while (ret != DM_OK);

    PublishInfo dmPublishInfo;
    (void)memset_s(&dmPublishInfo, sizeof(PublishInfo), 0, sizeof(PublishInfo));
    SetPublishInfo(dmPublishInfo);
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ret = PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
    if (ret == DM_OK) {
        publishStatus = ALLOW_BE_DISCOVERY;
    }
#else
    char discoverStatus[DISCOVER_STATUS_LEN + 1] = {0};
    ret = GetParameter(DISCOVER_STATUS_KEY, "not exist", discoverStatus, DISCOVER_STATUS_LEN);
    if (strcmp(discoverStatus, "not exist") == 0) {
        ret = SetParameter(DISCOVER_STATUS_KEY, DISCOVER_STATUS_ON);
        LOGI("[SOFTBUS]service set parameter result, ret: %d.", ret);

        ret = PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service publish result, ret: %d.", ret);
    } else if (ret >= 0 && strcmp(discoverStatus, DISCOVER_STATUS_ON) == 0) {
        ret = PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service publish result, ret: %d.", ret);
    } else if (ret >= 0 && strcmp(discoverStatus, DISCOVER_STATUS_OFF) == 0) {
        ret = StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            publishStatus = NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service unpublish. ret: %d.", ret);
    }

    ret = WatchParameter(DISCOVER_STATUS_KEY, &SoftbusListener::OnParameterChgCallback, nullptr);
#endif
    return ret;
}

int32_t SoftbusListener::GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    DmDeviceInfo *info = static_cast<DmDeviceInfo *>(malloc(sizeof(DmDeviceInfo) * (deviceCount)));
    if (info == nullptr) {
        FreeNodeInfo(nodeInfo);
        return ERR_DM_MALLOC_FAILED;
    }
    DmDeviceInfo **pInfoList = &info;
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        DmDeviceInfo *deviceInfo = *pInfoList + i;
        ConvertNodeBasicInfoToDmDevice(*nodeBasicInfo, *deviceInfo);
        deviceInfoList.push_back(*deviceInfo);
    }
    FreeNodeInfo(nodeInfo);
    free(info);
    LOGI("GetTrustDevices success, deviceCount: %d.", deviceCount);
    return ret;
}

int32_t SoftbusListener::GetAvailableDeviceList(std::vector<DmDeviceBasicInfo> &deviceBasicInfoList)
{
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    DmDeviceBasicInfo *info = static_cast<DmDeviceBasicInfo *>(malloc(sizeof(DmDeviceBasicInfo) * (deviceCount)));
    if (info == nullptr) {
        FreeNodeInfo(nodeInfo);
        return ERR_DM_MALLOC_FAILED;
    }
    DmDeviceBasicInfo **pInfoList = &info;
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        DmDeviceBasicInfo *deviceBasicInfo = *pInfoList + i;
        ConvertNodeBasicInfoToDmDevice(*nodeBasicInfo, *deviceBasicInfo);
        deviceBasicInfoList.push_back(*deviceBasicInfo);
    }
    FreeNodeInfo(nodeInfo);
    free(info);
    LOGI("GetAvailableDevices success, deviceCount: %d.", deviceCount);
    return ret;
}

int32_t SoftbusListener::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    int32_t nodeInfoCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &nodeInfoCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    for (int32_t i = 0; i < nodeInfoCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        if (networkId == nodeBasicInfo->networkId) {
            LOGI("GetDeviceInfo name : %s.", nodeBasicInfo->deviceName);
            if (memcpy_s(info.deviceName, sizeof(info.deviceName), nodeBasicInfo->deviceName,
                std::min(sizeof(info.deviceName), sizeof(nodeBasicInfo->deviceName))) != DM_OK) {
                LOGE("GetDeviceInfo deviceName copy deviceName data failed.");
            }
            info.deviceTypeId = nodeBasicInfo->deviceTypeId;
            break;
        }
    }
    FreeNodeInfo(nodeInfo);
    LOGI("GetDeviceInfo complete, deviceName : %s, deviceTypeId : %d.", info.deviceName, info.deviceTypeId);
    return ret;
}

int32_t SoftbusListener::GetLocalDeviceInfo(DmDeviceInfo &deviceInfo)
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, deviceInfo);
    return ret;
}

int32_t SoftbusListener::GetLocalDeviceNetworkId(std::string &networkId)
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    networkId = nodeBasicInfo.networkId;
    return ret;
}

int32_t SoftbusListener::GetLocalDeviceName(std::string &deviceName)
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    deviceName = nodeBasicInfo.deviceName;
    return ret;
}

int32_t SoftbusListener::GetLocalDeviceType(int32_t &deviceType)
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    deviceType = nodeBasicInfo.deviceTypeId;
    return ret;
}

int32_t SoftbusListener::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    uint8_t mUdid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    udid = reinterpret_cast<char *>(mUdid);
    return ret;
}

int32_t SoftbusListener::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    uint8_t mUuid[UUID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UUID, mUuid, sizeof(mUuid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    uuid = reinterpret_cast<char *>(mUuid);
    return ret;
}

int32_t SoftbusListener::ShiftLNNGear()
{
    GearMode mode = {
        .cycle = HIGH_FREQ_CYCLE,
        .duration = DEFAULT_DURATION,
        .wakeupFlag = false,
    };

    int32_t ret = ::ShiftLNNGear(DM_PKG_NAME, DM_PKG_NAME, nullptr, &mode);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]ShiftLNNGear error, failed ret: %d", ret);
        return ret;
    }
    LOGI("[SOFTBUS]ShiftLNNGear success.");
    return DM_OK;
}

void SoftbusListener::OnSoftBusDeviceOnline(NodeBasicInfo *info)
{
    LOGI("received device online callback from softbus.");
    if (info == nullptr) {
        LOGE("NodeBasicInfo is nullptr.");
        return;
    }
    DmDeviceInfo dmDeviceInfo;
    ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
#if defined(__LITEOS_M__)
    DmThread deviceOnLine(DeviceOnLine, dmDeviceInfo);
    deviceOnLine.DmCreatThread();
#else
    std::thread deviceOnLine(DeviceOnLine, dmDeviceInfo);
    int32_t ret = pthread_setname_np(deviceOnLine.native_handle(), DEVICE_ONLINE);
    if (ret != DM_OK) {
        LOGE("deviceOnLine setname failed.");
    }
    deviceOnLine.detach();
#endif
}

void SoftbusListener::OnSoftbusDeviceOffline(NodeBasicInfo *info)
{
    LOGI("received device offline callback from softbus.");
    if (info == nullptr) {
        LOGE("NodeBasicInfo is nullptr.");
        return;
    }
    DmDeviceInfo dmDeviceInfo;
    ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
#if defined(__LITEOS_M__)
    DmThread deviceOffLine(DeviceOffLine, dmDeviceInfo);
    deviceOffLine.DmCreatThread();
#else
    std::thread deviceOffLine(DeviceOffLine, dmDeviceInfo);
    int32_t ret = pthread_setname_np(deviceOffLine.native_handle(), DEVICE_OFFLINE);
    if (ret != DM_OK) {
        LOGE("deviceOffLine setname failed.");
    }
    deviceOffLine.detach();
#endif
}

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), nodeBasicInfo.networkId,
        std::min(sizeof(dmDeviceInfo.networkId), sizeof(nodeBasicInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), nodeBasicInfo.deviceName,
        std::min(sizeof(dmDeviceInfo.deviceName), sizeof(nodeBasicInfo.deviceName))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed.");
    }
    dmDeviceInfo.deviceTypeId = nodeBasicInfo.deviceTypeId;
    return DM_OK;
}

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo,
    DmDeviceBasicInfo &dmDeviceBasicInfo)
{
    (void)memset_s(&dmDeviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    if (memcpy_s(dmDeviceBasicInfo.networkId, sizeof(dmDeviceBasicInfo.networkId), nodeBasicInfo.networkId,
        std::min(sizeof(dmDeviceBasicInfo.networkId), sizeof(nodeBasicInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
    }

    if (memcpy_s(dmDeviceBasicInfo.deviceName, sizeof(dmDeviceBasicInfo.deviceName), nodeBasicInfo.deviceName,
        std::min(sizeof(dmDeviceBasicInfo.deviceName), sizeof(nodeBasicInfo.deviceName))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed.");
    }
    dmDeviceBasicInfo.deviceTypeId = nodeBasicInfo.deviceTypeId;
    return DM_OK;
}

void SoftbusListener::OnParameterChgCallback(const char *key, const char *value, void *context)
{
    (void)key;
    (void)context;
    if (strcmp(value, DISCOVER_STATUS_ON) == 0 && publishStatus != ALLOW_BE_DISCOVERY) {
        PublishInfo dmPublishInfo;
        (void)memset_s(&dmPublishInfo, sizeof(PublishInfo), 0, sizeof(PublishInfo));
        dmPublishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
        dmPublishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
        dmPublishInfo.medium = ExchangeMedium::AUTO;
        dmPublishInfo.freq = ExchangeFreq::HIGH;
        dmPublishInfo.capability = DM_CAPABILITY_OSD;
        dmPublishInfo.ranging = false;
        int32_t ret = ::PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]PublishLNN return ret: %d.", ret);
    } else if (strcmp(value, DISCOVER_STATUS_OFF) == 0 && publishStatus != NOT_ALLOW_BE_DISCOVERY) {
        int32_t ret = ::StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            publishStatus = NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]StopPublishLNN return ret: %d.", ret);
    }
}

int SoftbusListener::OnSessionOpened(int sessionId, int result)
{
    return DeviceManagerService::GetInstance().OnSessionOpened(sessionId, result);
}

void SoftbusListener::OnSessionClosed(int sessionId)
{
    DeviceManagerService::GetInstance().OnSessionClosed(sessionId);
}

void SoftbusListener::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    DeviceManagerService::GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

void SoftbusListener::OnPublishResult(int publishId, PublishResult result)
{
    LOGD("OnPublishResult, publishId: %d, result: %d.", publishId, result);
}

void SoftbusListener::OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    LOGI("received device info change from softbus.");
    if (info == nullptr) {
        LOGE("NodeBasicInfo is nullptr.");
        return;
    }
    if (type == NodeBasicInfoType::TYPE_DEVICE_NAME || type == NodeBasicInfoType::TYPE_NETWORK_INFO) {
        LOGI("DeviceInfo change.");
        DmDeviceInfo dmDeviceInfo;
        int32_t mNetworkType = -1;
        if (type == NodeBasicInfoType::TYPE_NETWORK_INFO) {
            if (GetNodeKeyInfo(DM_PKG_NAME, info->networkId, NodeDeviceInfoKey::NODE_KEY_NETWORK_TYPE,
                reinterpret_cast<uint8_t *>(&mNetworkType), LNN_COMMON_LEN) != DM_OK) {
                LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
                return;
            }
            LOGI("OnSoftbusDeviceInfoChanged NetworkType %d.", mNetworkType);
        }
        ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
        dmDeviceInfo.networkType = mNetworkType;
        std::thread deviceInfoChange(DeviceNameChange, dmDeviceInfo);
        if (pthread_setname_np(deviceInfoChange.native_handle(), DEVICE_NAME_CHANGE) != DM_OK) {
            LOGE("DeviceNameChange setname failed.");
        }
        deviceInfoChange.detach();
        LOGD("OnSoftbusDeviceInfoChanged.");
        return;
    }
}

int32_t SoftbusListener::GetNetworkTypeByNetworkId(const char *networkId, int32_t &networkType)
{
    int32_t mNetworkType = -1;
    if (GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_NETWORK_TYPE,
        reinterpret_cast<uint8_t *>(&mNetworkType), LNN_COMMON_LEN) != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
        return ERR_DM_FAILED;
    }
    networkType = mNetworkType;
    LOGI("GetNetworkTypeByNetworkId networkType %d.", mNetworkType);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
