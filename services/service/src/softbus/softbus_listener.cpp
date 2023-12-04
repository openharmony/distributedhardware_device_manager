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

#include <mutex>
#include <pthread.h>
#include <securec.h>
#include <thread>
#include <unistd.h>

#include "device_manager_service.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
enum PulishStatus {
    STATUS_UNKNOWN = 0,
    ALLOW_BE_DISCOVERY = 1,
    NOT_ALLOW_BE_DISCOVERY = 2,
};
const int32_t DISCOVER_STATUS_LEN = 20;
const int32_t SOFTBUS_CHECK_INTERVAL = 100000; // 100ms
const int32_t SOFTBUS_SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t MAX_CACHED_DISCOVERED_DEVICE_SIZE = 100;
constexpr const char* DISCOVER_STATUS_KEY = "persist.distributed_hardware.device_manager.discover_status";
constexpr const char* DISCOVER_STATUS_ON = "1";
constexpr const char* DISCOVER_STATUS_OFF = "0";
constexpr const char* DEVICE_ONLINE = "deviceOnLine";
constexpr const char* DEVICE_OFFLINE = "deviceOffLine";
constexpr const char* DEVICE_NAME_CHANGE = "deviceNameChange";
constexpr static char HEX_ARRAY[] = "0123456789ABCDEF";
constexpr static uint8_t BYTE_MASK = 0x0F;
constexpr static uint16_t ARRAY_DOUBLE_SIZE = 2;
constexpr static uint16_t BIN_HIGH_FOUR_NUM = 4;

static PulishStatus g_publishStatus = PulishStatus::STATUS_UNKNOWN;
static std::mutex g_deviceMapMutex;
static std::mutex g_lnnCbkMapMutex;
static std::map<std::string, std::shared_ptr<DeviceInfo>> discoveredDeviceMap;
static std::map<std::string, std::shared_ptr<ISoftbusLnnOpsCallback>> lnnOpsCbkMap;

static int OnSessionOpened(int sessionId, int result)
{
    struct RadarInfo info = {
        .funcName = "OnSessionOpened",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .isTrust = static_cast<int32_t>(TrustStatus::NOT_TRUST),
        .channelId = sessionId,
    };
    if (!DmRadarHelper::GetInstance().ReportAuthSessionOpenCb(info)) {
        LOGE("ReportAuthSessionOpenCb failed");
    }
    return DeviceManagerService::GetInstance().OnSessionOpened(sessionId, result);
}

static void OnSessionClosed(int sessionId)
{
    DeviceManagerService::GetInstance().OnSessionClosed(sessionId);
}

static void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    DeviceManagerService::GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

static int OnPinHolderSessionOpened(int sessionId, int result)
{
    return DeviceManagerService::GetInstance().OnPinHolderSessionOpened(sessionId, result);
}

static void OnPinHolderSessionClosed(int sessionId)
{
    DeviceManagerService::GetInstance().OnPinHolderSessionClosed(sessionId);
}

static void OnPinHolderBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    DeviceManagerService::GetInstance().OnPinHolderBytesReceived(sessionId, data, dataLen);
}

static IPublishCb softbusPublishCallback_ = {
    .OnPublishResult = SoftbusListener::OnSoftbusPublishResult,
};

static INodeStateCb softbusNodeStateCb_ = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE | EVENT_NODE_STATE_INFO_CHANGED,
    .onNodeOnline = SoftbusListener::OnSoftbusDeviceOnline,
    .onNodeOffline = SoftbusListener::OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = SoftbusListener::OnSoftbusDeviceInfoChanged
};

static IRefreshCallback softbusRefreshCallback_ = {
    .OnDeviceFound = SoftbusListener::OnSoftbusDeviceFound,
    .OnDiscoverResult = SoftbusListener::OnSoftbusDiscoveryResult,
};

void SoftbusListener::DeviceOnLine(DmDeviceInfo deviceInfo)
{
    std::mutex lockDeviceOnLine;
    std::lock_guard<std::mutex> lock(lockDeviceOnLine);
    DeviceManagerService::GetInstance().HandleDeviceOnline(deviceInfo);
}

void SoftbusListener::DeviceOffLine(DmDeviceInfo deviceInfo)
{
    std::mutex lockDeviceOffLine;
    std::lock_guard<std::mutex> lock(lockDeviceOffLine);
    DeviceManagerService::GetInstance().HandleDeviceOffline(deviceInfo);
}

void SoftbusListener::DeviceNameChange(DmDeviceInfo deviceInfo)
{
    std::mutex lockDeviceOffLine;
    std::lock_guard<std::mutex> lock(lockDeviceOffLine);
    DeviceManagerService::GetInstance().HandleDeviceNameChange(deviceInfo);
}

void SoftbusListener::OnSoftbusDeviceOnline(NodeBasicInfo *info)
{
    LOGI("received device online callback from softbus.");
    if (info == nullptr) {
        LOGE("NodeBasicInfo is nullptr.");
        return;
    }
    DmDeviceInfo dmDeviceInfo;
    ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
    std::thread deviceOnLine(DeviceOnLine, dmDeviceInfo);
    int32_t ret = pthread_setname_np(deviceOnLine.native_handle(), DEVICE_ONLINE);
    if (ret != DM_OK) {
        LOGE("deviceOnLine setname failed.");
    }
    deviceOnLine.detach();
    {
        char localDeviceId[DEVICE_UUID_LENGTH] = {0};
        GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
        std::string peerUdid;
        GetUdidByNetworkId(info->networkId, peerUdid);
        struct RadarInfo radarInfo = {
            .funcName = "OnSoftbusDeviceOnline",
            .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
            .bizState = static_cast<int32_t>(BizState::BIZ_STATE_START),
            .isTrust = static_cast<int32_t>(TrustStatus::IS_TRUST),
            .peerNetId = info->networkId,
            .localUdid = std::string(localDeviceId),
            .peerUdid = peerUdid,
        };
        if (!DmRadarHelper::GetInstance().ReportNetworkOnline(radarInfo)) {
            LOGE("ReportNetworkOnline failed");
        }
    }
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
    std::thread deviceOffLine(DeviceOffLine, dmDeviceInfo);
    int32_t ret = pthread_setname_np(deviceOffLine.native_handle(), DEVICE_OFFLINE);
    if (ret != DM_OK) {
        LOGE("deviceOffLine setname failed.");
    }
    deviceOffLine.detach();
    {
        char localDeviceId[DEVICE_UUID_LENGTH] = {0};
        GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
        std::string peerUdid;
        GetUdidByNetworkId(info->networkId, peerUdid);
        struct RadarInfo radarInfo = {
            .funcName = "OnSoftbusDeviceOffline",
            .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
            .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
            .peerNetId = info->networkId,
            .localUdid = std::string(localDeviceId),
            .peerUdid = peerUdid,
        };
        if (!DmRadarHelper::GetInstance().ReportNetworkOffline(radarInfo)) {
            LOGE("ReportNetworkOffline failed");
        }
    }
}

void SoftbusListener::OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    LOGI("received device info change from softbus.");
    if (info == nullptr) {
        LOGE("NodeBasicInfo is nullptr.");
        return;
    }
    if (type == NodeBasicInfoType::TYPE_DEVICE_NAME || type == NodeBasicInfoType::TYPE_NETWORK_INFO) {
        LOGI("DeviceInfo %d change.", type);
        DmDeviceInfo dmDeviceInfo;
        int32_t networkType = -1;
        if (type == NodeBasicInfoType::TYPE_NETWORK_INFO) {
            if (GetNodeKeyInfo(DM_PKG_NAME, info->networkId, NodeDeviceInfoKey::NODE_KEY_NETWORK_TYPE,
                reinterpret_cast<uint8_t *>(&networkType), LNN_COMMON_LEN) != DM_OK) {
                LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
                return;
            }
            LOGI("OnSoftbusDeviceInfoChanged NetworkType %d.", networkType);
        }
        ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
        dmDeviceInfo.networkType = networkType;
        std::thread deviceInfoChange(DeviceNameChange, dmDeviceInfo);
        if (pthread_setname_np(deviceInfoChange.native_handle(), DEVICE_NAME_CHANGE) != DM_OK) {
            LOGE("DeviceNameChange setname failed.");
        }
        deviceInfoChange.detach();
    }
}

void SoftbusListener::OnParameterChgCallback(const char *key, const char *value, void *context)
{
    (void)key;
    (void)context;
    if (strcmp(value, DISCOVER_STATUS_ON) == 0 && g_publishStatus != PulishStatus::ALLOW_BE_DISCOVERY) {
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
            g_publishStatus = PulishStatus::ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]PublishLNN return ret: %d.", ret);
    } else if (strcmp(value, DISCOVER_STATUS_OFF) == 0 && g_publishStatus != PulishStatus::NOT_ALLOW_BE_DISCOVERY) {
        int32_t ret = ::StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]StopPublishLNN return ret: %d.", ret);
    }
}

void SoftbusListener::OnSoftbusDeviceFound(const DeviceInfo *device)
{
    if (device == nullptr) {
        LOGE("[SOFTBUS]device is null.");
        return;
    }
    DmDeviceInfo dmDevInfo;
    ConvertDeviceInfoToDmDevice(*device, dmDevInfo);

    LOGI("OnSoftbusDeviceFound: devId=%s, devName=%s, devType=%d, range=%d, isOnline=%d, extraData=%s",
        GetAnonyString(dmDevInfo.deviceId).c_str(), dmDevInfo.deviceName, dmDevInfo.deviceTypeId,
        dmDevInfo.range, device->isOnline, dmDevInfo.extraData.c_str());

    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    for (auto &iter : lnnOpsCbkMap) {
        iter.second->OnDeviceFound(iter.first, dmDevInfo, device->isOnline);
    }
    CacheDiscoveredDevice(device);
}

void SoftbusListener::OnSoftbusDiscoveryResult(int subscribeId, RefreshResult result)
{
    uint16_t originId = static_cast<uint16_t>((static_cast<uint32_t>(subscribeId)) & SOFTBUS_SUBSCRIBE_ID_MASK);
    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    for (auto &iter : lnnOpsCbkMap) {
        iter.second->OnDiscoveringResult(iter.first, originId, result);
    }
}

void SoftbusListener::OnSoftbusPublishResult(int publishId, PublishResult result)
{
    LOGD("OnSoftbusPublishResult, publishId: %d, result: %d.", publishId, result);
}

SoftbusListener::SoftbusListener()
{
    ISessionListener sessionListener = {
        .OnSessionOpened = OnSessionOpened,
        .OnSessionClosed = OnSessionClosed,
        .OnBytesReceived = OnBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr
    };
    LOGD("SoftbusListener constructor.");
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_SESSION_NAME, &sessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer failed, ret: %d.", ret);
    }
    ISessionListener pinHolderSessionListener = {
        .OnSessionOpened = OnPinHolderSessionOpened,
        .OnSessionClosed = OnPinHolderSessionClosed,
        .OnBytesReceived = OnPinHolderBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr
    };
    ret = CreateSessionServer(DM_PKG_NAME, DM_PIN_HOLDER_SESSION_NAME, &pinHolderSessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer pin holder failed, ret: %d.", ret);
    }
    InitSoftbusListener();
    ClearDiscoveredDevice();
}

SoftbusListener::~SoftbusListener()
{
    RemoveSessionServer(DM_PKG_NAME, DM_SESSION_NAME);
    RemoveSessionServer(DM_PKG_NAME, DM_PIN_HOLDER_SESSION_NAME);
    LOGD("SoftbusListener destructor.");
}

int32_t SoftbusListener::InitSoftbusListener()
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

    PublishInfo publishInfo;
    publishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    publishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    publishInfo.medium = ExchangeMedium::AUTO;
    publishInfo.freq = ExchangeFreq::HIGH;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.ranging = false;

#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ret = PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret == DM_OK) {
        g_publishStatus = PulishStatus::ALLOW_BE_DISCOVERY;
    }
#else
    char discoverStatus[DISCOVER_STATUS_LEN + 1] = {0};
    GetParameter(DISCOVER_STATUS_KEY, "not exist", discoverStatus, DISCOVER_STATUS_LEN);
    if (strcmp(discoverStatus, "not exist") == 0) {
        SetParameter(DISCOVER_STATUS_KEY, DISCOVER_STATUS_ON);
        ret = PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service publish result, ret: %d.", ret);
    } else if (strcmp(discoverStatus, DISCOVER_STATUS_ON) == 0) {
        ret = PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service publish result, ret: %d.", ret);
    } else if (strcmp(discoverStatus, DISCOVER_STATUS_OFF) == 0) {
        ret = StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service unpublish. ret: %d.", ret);
    }
    ret = WatchParameter(DISCOVER_STATUS_KEY, &OnParameterChgCallback, nullptr);
#endif
    return ret;
}

int32_t SoftbusListener::RefreshSoftbusLNN(const char *pkgName, const DmSubscribeInfo &dmSubInfo,
    const std::string &customData)
{
    SubscribeInfo subscribeInfo;
    (void)memset_s(&subscribeInfo, sizeof(SubscribeInfo), 0, sizeof(SubscribeInfo));
    subscribeInfo.subscribeId = dmSubInfo.subscribeId;
    subscribeInfo.mode = static_cast<DiscoverMode>(dmSubInfo.mode);
    subscribeInfo.medium = static_cast<ExchangeMedium>(dmSubInfo.medium);
    subscribeInfo.freq = static_cast<ExchangeFreq>(dmSubInfo.freq);
    subscribeInfo.isSameAccount = dmSubInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubInfo.isWakeRemote;
    subscribeInfo.capability = dmSubInfo.capability;
    subscribeInfo.capabilityData = (unsigned char*)(customData.c_str());
    subscribeInfo.dataLen = customData.size();

    LOGI("RefreshSoftbusLNN begin, subscribeId: %d, mode: 0x%x, medium: %d, capability: %s.",
        subscribeInfo.subscribeId, subscribeInfo.mode, subscribeInfo.medium, subscribeInfo.capability);

    int32_t ret = ::RefreshLNN(pkgName, &subscribeInfo, &softbusRefreshCallback_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]RefreshLNN failed, ret: %d.", ret);
        return ERR_DM_REFRESH_LNN_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::StopRefreshSoftbusLNN(uint16_t subscribeId)
{
    LOGI("StopRefreshSoftbusLNN begin, subscribeId: %d.", (int32_t)subscribeId);
    int32_t ret = ::StopRefreshLNN(DM_PKG_NAME, subscribeId);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopRefreshLNN failed, ret: %d.", ret);
        return ERR_DM_STOP_REFRESH_LNN_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::RegisterSoftbusLnnOpsCbk(const std::string &pkgName,
    const std::shared_ptr<ISoftbusLnnOpsCallback> callback)
{
    if (callback == nullptr) {
        LOGE("RegisterSoftbusDiscoveringCbk failed, input callback is null.");
        return ERR_DM_POINT_NULL;
    }
    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    lnnOpsCbkMap.emplace(pkgName, callback);
    return DM_OK;
}

int32_t SoftbusListener::UnRegisterSoftbusLnnOpsCbk(const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    lnnOpsCbkMap.erase(pkgName);
    return DM_OK;
}

int32_t SoftbusListener::GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    struct RadarInfo radarInfo = {
        .funcName = "GetTrustedDeviceList",
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
        .localUdid = std::string(localDeviceId),
    };
    if (ret != DM_OK) {
        radarInfo.stageRes = static_cast<int32_t>(StageRes::STAGE_FAIL);
        radarInfo.errCode = ERR_DM_FAILED;
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
    radarInfo.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    radarInfo.discoverDevList = DmRadarHelper::GetInstance().GetDeviceInfoList(deviceInfoList);
    if (!DmRadarHelper::GetInstance().ReportGetTrustDeviceList(radarInfo)) {
        LOGE("ReportGetTrustDeviceList failed!");
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

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceInfo &devInfo)
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
    return DM_OK;
}

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceBasicInfo &devInfo)
{
    (void)memset_s(&devInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    if (memcpy_s(devInfo.networkId, sizeof(devInfo.networkId), nodeInfo.networkId,
        std::min(sizeof(devInfo.networkId), sizeof(nodeInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
    }

    if (memcpy_s(devInfo.deviceName, sizeof(devInfo.deviceName), nodeInfo.deviceName,
        std::min(sizeof(devInfo.deviceName), sizeof(nodeInfo.deviceName))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed.");
    }
    devInfo.deviceTypeId = nodeInfo.deviceTypeId;
    return DM_OK;
}

std::string SoftbusListener::ConvertBytesToUpperCaseHexString(const uint8_t arr[], const size_t size)
{
    char result[size * ARRAY_DOUBLE_SIZE + 1];
    int index = 0;
    for (size_t i = 0; i < size; i++) {
        uint8_t num = arr[i];
        result[index++] = HEX_ARRAY[(num >> BIN_HIGH_FOUR_NUM) & BYTE_MASK];
        result[index++] = HEX_ARRAY[num & BYTE_MASK];
    }
    result[index] = '\0';
    return result;
}

void SoftbusListener::ConvertDeviceInfoToDmDevice(const DeviceInfo &device, DmDeviceInfo &dmDevice)
{
    (void)memset_s(&dmDevice, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));

    if (memcpy_s(dmDevice.deviceId, sizeof(dmDevice.deviceId), device.devId,
        std::min(sizeof(dmDevice.deviceId), sizeof(device.devId))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice: copy device id failed.");
    }

    if (memcpy_s(dmDevice.deviceName, sizeof(dmDevice.deviceName), device.devName,
        std::min(sizeof(dmDevice.deviceName), sizeof(device.devName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice: copy device name failed.");
    }

    dmDevice.deviceTypeId = device.devType;
    dmDevice.range = device.range;

    nlohmann::json jsonObj;
    std::string customData(device.custData);
    jsonObj[PARAM_KEY_CUSTOM_DATA] = customData;

    const ConnectionAddr *addrInfo = &(device.addr)[0];
    if (addrInfo == nullptr) {
        LOGE("ConvertDeviceInfoToDmDevice: addrInfo is nullptr.");
        dmDevice.extraData = jsonObj.dump();
        return;
    }
    if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_WLAN) {
        std::string wifiIp((addrInfo->info).ip.ip);
        jsonObj[PARAM_KEY_WIFI_IP] = wifiIp;
        jsonObj[PARAM_KEY_WIFI_PORT] = (addrInfo->info).ip.port;
        jsonObj[PARAM_KEY_CONN_ADDR_TYPE] = CONN_ADDR_TYPE_WLAN_IP;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BR) {
        std::string brMac((addrInfo->info).br.brMac);
        jsonObj[PARAM_KEY_BR_MAC] = brMac;
        jsonObj[PARAM_KEY_CONN_ADDR_TYPE] = CONN_ADDR_TYPE_BR;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BLE) {
        std::string bleMac((addrInfo->info).ble.bleMac);
        jsonObj[PARAM_KEY_BLE_MAC] = bleMac;
        jsonObj[PARAM_KEY_CONN_ADDR_TYPE] = CONN_ADDR_TYPE_BLE;
        std::string udidHash(ConvertBytesToUpperCaseHexString((addrInfo->info).ble.udidHash,
            sizeof((addrInfo->info).ble.udidHash) / sizeof(*((addrInfo->info).ble.udidHash))));
        jsonObj[PARAM_KEY_BLE_UDID_HASH] = udidHash;
    } else {
        LOGI("Unknown connection address type: %d.", addrInfo->type);
    }
    dmDevice.extraData = jsonObj.dump();
}

int32_t SoftbusListener::GetNetworkTypeByNetworkId(const char *networkId, int32_t &networkType)
{
    int32_t tempNetworkType = -1;
    if (GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_NETWORK_TYPE,
        reinterpret_cast<uint8_t *>(&tempNetworkType), LNN_COMMON_LEN) != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
        return ERR_DM_FAILED;
    }
    networkType = tempNetworkType;
    LOGI("GetNetworkTypeByNetworkId networkType %d.", tempNetworkType);
    return DM_OK;
}

void SoftbusListener::CacheDiscoveredDevice(const DeviceInfo *device)
{
    if (device->isOnline) {
        return;
    }
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    if (memcpy_s(infoPtr.get(), sizeof(DeviceInfo), device, sizeof(DeviceInfo)) != DM_OK) {
        LOGE("CacheDiscoveredDevice error, copy device info failed.");
        return;
    }

    std::lock_guard<std::mutex> lock(g_deviceMapMutex);
    if (discoveredDeviceMap.size() == MAX_CACHED_DISCOVERED_DEVICE_SIZE) {
        discoveredDeviceMap.erase(discoveredDeviceMap.begin());
    }
    discoveredDeviceMap.erase(device->devId);
    discoveredDeviceMap.insert(std::pair<std::string, std::shared_ptr<DeviceInfo>>(device->devId, infoPtr));
}

int32_t SoftbusListener::GetTargetInfoFromCache(const std::string &deviceId, PeerTargetId &targetId)
{
    std::lock_guard<std::mutex> lock(g_deviceMapMutex);
    auto iter = discoveredDeviceMap.find(deviceId);
    if (iter == discoveredDeviceMap.end()) {
        LOGE("GetTargetInfoFromCache failed, cannot found device in cached discovered map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }

    const ConnectionAddr *addrInfo = &(iter->second->addr)[0];
    if (addrInfo == nullptr) {
        LOGE("GetTargetInfoFromCache failed, connection address of discovered device is nullptr.");
        return ERR_DM_BIND_COMMON_FAILED;
    }

    if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_WLAN) {
        std::string wifiIp((addrInfo->info).ip.ip);
        targetId.wifiIp = wifiIp;
        targetId.wifiPort = (addrInfo->info).ip.port;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BR) {
        std::string brMac((addrInfo->info).br.brMac);
        targetId.brMac = brMac;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BLE) {
        std::string bleMac((addrInfo->info).ble.bleMac);
        targetId.bleMac = bleMac;
    } else {
        LOGI("Unknown connection address type: %d.", addrInfo->type);
        return ERR_DM_BIND_COMMON_FAILED;
    }
    targetId.deviceId = deviceId;
    return DM_OK;
}

void SoftbusListener::ClearDiscoveredDevice()
{
    std::lock_guard<std::mutex> lock(g_deviceMapMutex);
    discoveredDeviceMap.clear();
}
} // namespace DistributedHardware
} // namespace OHOS