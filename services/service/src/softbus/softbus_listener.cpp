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

#include "softbus_listener.h"

#include <dlfcn.h>
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
#include "parameter.h"
#include "system_ability_definition.h"
#include "softbus_adapter.cpp"

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
const int32_t LOAD_RADAR_TIMEOUT = 180;
constexpr const char* DISCOVER_STATUS_KEY = "persist.distributed_hardware.device_manager.discover_status";
constexpr const char* DISCOVER_STATUS_ON = "1";
constexpr const char* DISCOVER_STATUS_OFF = "0";
constexpr const char* DEVICE_ONLINE = "deviceOnLine";
constexpr const char* DEVICE_OFFLINE = "deviceOffLine";
constexpr const char* DEVICE_NAME_CHANGE = "deviceNameChange";
constexpr const char* LOAD_RADAR_TIMEOUT_TASK = "deviceManagerTimer:loadRadarSo";
constexpr const char* LIB_RADAR_NAME = "libdevicemanagerradar.z.so";
constexpr static char HEX_ARRAY[] = "0123456789ABCDEF";
constexpr static uint8_t BYTE_MASK = 0x0F;
constexpr static uint16_t ARRAY_DOUBLE_SIZE = 2;
constexpr static uint16_t BIN_HIGH_FOUR_NUM = 4;

static PulishStatus g_publishStatus = PulishStatus::STATUS_UNKNOWN;
static std::mutex g_deviceMapMutex;
static std::mutex g_lnnCbkMapMutex;
static std::mutex g_radarLoadLock;
static std::map<std::string,
    std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>>> discoveredDeviceMap;
static std::map<std::string, std::shared_ptr<ISoftbusDiscoveringCallback>> lnnOpsCbkMap;
static std::set<std::string> deviceIdSet;
bool SoftbusListener::isRadarSoLoad_ = false;
IDmRadarHelper* SoftbusListener::dmRadarHelper_ = nullptr;
std::shared_ptr<DmTimer> SoftbusListener::timer_ = std::make_shared<DmTimer>();
void* SoftbusListener::radarHandle_ = nullptr;
std::string SoftbusListener::hostName_ = "";

static int OnSessionOpened(int sessionId, int result)
{
    struct RadarInfo info = {
        .funcName = "OnSessionOpened",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .isTrust = static_cast<int32_t>(TrustStatus::NOT_TRUST),
        .channelId = sessionId,
    };
    if (SoftbusListener::IsDmRadarHelperReady() && SoftbusListener::GetDmRadarHelperObj() != nullptr) {
        if (!SoftbusListener::GetDmRadarHelperObj()->ReportAuthSessionOpenCb(info)) {
            LOGE("ReportAuthSessionOpenCb failed");
        }
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
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_STATE_ONLINE, deviceInfo);
}

void SoftbusListener::DeviceOffLine(DmDeviceInfo deviceInfo)
{
    std::mutex lockDeviceOffLine;
    std::lock_guard<std::mutex> lock(lockDeviceOffLine);
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_STATE_OFFLINE, deviceInfo);
}

void SoftbusListener::DeviceNameChange(DmDeviceInfo deviceInfo)
{
    std::mutex lockDeviceOffLine;
    std::lock_guard<std::mutex> lock(lockDeviceOffLine);
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_INFO_CHANGED, deviceInfo);
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
    LOGI("device online networkId: %{public}s.", GetAnonyString(dmDeviceInfo.networkId).c_str());
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
        if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
            if (!GetDmRadarHelperObj()->ReportNetworkOnline(radarInfo)) {
                LOGE("ReportNetworkOnline failed");
            }
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
    LOGI("device offline networkId: %{public}s.", GetAnonyString(dmDeviceInfo.networkId).c_str());
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
        if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
            if (!GetDmRadarHelperObj()->ReportNetworkOffline(radarInfo)) {
                LOGE("ReportNetworkOffline failed");
            }
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
        LOGI("DeviceInfo %{public}d change.", type);
        DmDeviceInfo dmDeviceInfo;
        int32_t networkType = -1;
        if (type == NodeBasicInfoType::TYPE_NETWORK_INFO) {
            if (GetNodeKeyInfo(DM_PKG_NAME, info->networkId, NodeDeviceInfoKey::NODE_KEY_NETWORK_TYPE,
                reinterpret_cast<uint8_t *>(&networkType), LNN_COMMON_LEN) != DM_OK) {
                LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
                return;
            }
            LOGI("OnSoftbusDeviceInfoChanged NetworkType %{public}d.", networkType);
        }
        ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
        LOGI("device changed networkId: %{public}s.", GetAnonyString(dmDeviceInfo.networkId).c_str());
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
        dmPublishInfo.freq = ExchangeFreq::LOW;
        dmPublishInfo.capability = DM_CAPABILITY_OSD;
        dmPublishInfo.ranging = false;
        int32_t ret = ::PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
        LOGI("OnParameterChgCallback begin, publishId: %{public}d, mode: 0x%{public}x, medium: %{public}d, capability:"
            "%{public}s, ranging: %{public}d, freq: %{public}d.", dmPublishInfo.publishId, dmPublishInfo.mode,
            dmPublishInfo.medium, dmPublishInfo.capability, dmPublishInfo.ranging, dmPublishInfo.freq);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]PublishLNN return ret: %{public}d.", ret);
    } else if (strcmp(value, DISCOVER_STATUS_OFF) == 0 && g_publishStatus != PulishStatus::NOT_ALLOW_BE_DISCOVERY) {
        int32_t ret = ::StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]StopPublishLNN return ret: %{public}d.", ret);
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
    if (deviceIdSet.find(device->devId) == deviceIdSet.end()) {
        deviceIdSet.insert(device->devId);
        struct RadarInfo info = {
            .funcName = "OnSoftbusDeviceFound",
            .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
            .peerNetId = "",
            .peerUdid = device->devId,
        };
        if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
            if (!GetDmRadarHelperObj()->ReportDiscoverResCallback(info)) {
                LOGE("ReportDiscoverResCallback failed");
            }
        }
    }
    LOGI("OnSoftbusDeviceFound: devId=%{public}s, devName=%{public}s, devType=%{public}d, range=%{public}d,"
        "isOnline=%{public}d", GetAnonyString(dmDevInfo.deviceId).c_str(), GetAnonyString(dmDevInfo.deviceName).c_str(),
        dmDevInfo.deviceTypeId, dmDevInfo.range, device->isOnline);

    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    CacheDiscoveredDevice(device);
    for (auto &iter : lnnOpsCbkMap) {
        iter.second->OnDeviceFound(iter.first, dmDevInfo, device->isOnline);
    }
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
    LOGD("OnSoftbusPublishResult, publishId: %{public}d, result: %{public}d.", publishId, result);
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_SESSION_NAME, &sessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer failed, ret: %{public}d.", ret);
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
        LOGE("[SOFTBUS]CreateSessionServer pin holder failed, ret: %{public}d.", ret);
    }
    SoftbusAdapter::GetInstance().CreateSoftbusSessionServer(DM_PKG_NAME, DM_UNBIND_SESSION_NAME);
#endif
    InitSoftbusListener();
    ClearDiscoveredDevice();
}

SoftbusListener::~SoftbusListener()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    RemoveSessionServer(DM_PKG_NAME, DM_SESSION_NAME);
    RemoveSessionServer(DM_PKG_NAME, DM_PIN_HOLDER_SESSION_NAME);
    SoftbusAdapter::GetInstance().RemoveSoftbusSessionServer(DM_PKG_NAME, DM_UNBIND_SESSION_NAME);
#endif
    LOGD("SoftbusListener destructor.");
}

int32_t SoftbusListener::InitSoftbusListener()
{
    int32_t ret;
    int32_t retryTimes = 0;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    do {
        ret = RegNodeDeviceStateCb(DM_PKG_NAME, &softbusNodeStateCb_);
        if (ret != DM_OK) {
            ++retryTimes;
            LOGE("[SOFTBUS]RegNodeDeviceStateCb failed with ret: %{public}d, retryTimes: %{public}d.", ret, retryTimes);
            usleep(SOFTBUS_CHECK_INTERVAL);
        }
    } while (ret != DM_OK);
#endif
    return InitSoftPublishLNN();
}

int32_t SoftbusListener::InitSoftPublishLNN()
{
    int32_t ret;

    PublishInfo publishInfo;
    publishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    publishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    publishInfo.medium = ExchangeMedium::AUTO;
    publishInfo.freq = ExchangeFreq::LOW;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.ranging = false;
    LOGI("InitSoftPublishLNN begin, publishId: %{public}d, mode: 0x%{public}x, medium: %{public}d, capability:"
        "%{public}s, ranging: %{public}d, freq: %{public}d.", publishInfo.publishId, publishInfo.mode,
        publishInfo.medium, publishInfo.capability, publishInfo.ranging, publishInfo.freq);
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
        LOGI("[SOFTBUS]service publish result, ret: %{public}d.", ret);
    } else if (strcmp(discoverStatus, DISCOVER_STATUS_ON) == 0) {
        ret = PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service publish result, ret: %{public}d.", ret);
    } else if (strcmp(discoverStatus, DISCOVER_STATUS_OFF) == 0) {
        ret = StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            g_publishStatus = PulishStatus::NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("[SOFTBUS]service unpublish. ret: %{public}d.", ret);
    }
    ret = WatchParameter(DISCOVER_STATUS_KEY, &OnParameterChgCallback, nullptr);
#endif
    return ret;
}

int32_t SoftbusListener::RefreshSoftbusLNN(const char *pkgName, const DmSubscribeInfo &dmSubInfo,
    const std::string &customData)
{
    LOGI("RefreshSoftbusLNN begin, subscribeId: %{public}d.", dmSubInfo.subscribeId);
    SubscribeInfo subscribeInfo;
    (void)memset_s(&subscribeInfo, sizeof(SubscribeInfo), 0, sizeof(SubscribeInfo));
    subscribeInfo.subscribeId = dmSubInfo.subscribeId;
    subscribeInfo.mode = static_cast<DiscoverMode>(dmSubInfo.mode);
    subscribeInfo.medium = static_cast<ExchangeMedium>(dmSubInfo.medium);
    subscribeInfo.freq = static_cast<ExchangeFreq>(dmSubInfo.freq);
    subscribeInfo.isSameAccount = dmSubInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubInfo.isWakeRemote;
    subscribeInfo.capability = dmSubInfo.capability;
    subscribeInfo.capabilityData =
        const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(customData.c_str()));
    subscribeInfo.dataLen = customData.size();
    LOGI("RefreshSoftbusLNN begin, subscribeId: %{public}d, mode: 0x%{public}x, medium: %{public}d, capability:"
        "%{public}s, freq: %{public}d.", subscribeInfo.subscribeId, subscribeInfo.mode, subscribeInfo.medium,
        subscribeInfo.capability, subscribeInfo.freq);
    int32_t ret = ::RefreshLNN(pkgName, &subscribeInfo, &softbusRefreshCallback_);
    struct RadarInfo info = {
        .funcName = "RefreshSoftbusLNN",
        .toCallPkg = "dsoftbus",
        .hostName = GetHostPkgName(),
        .stageRes = (ret == DM_OK) ?
                    static_cast<int32_t>(StageRes::STAGE_IDLE) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .bizState = (ret == DM_OK) ?
                    static_cast<int32_t>(BizState::BIZ_STATE_START) : static_cast<int32_t>(BizState::BIZ_STATE_END),
        .commServ = static_cast<int32_t>(CommServ::USE_SOFTBUS),
        .errCode = ret,
        };
    if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
        if (!GetDmRadarHelperObj()->ReportDiscoverRegCallback(info)) {
            LOGE("ReportDiscoverRegCallback failed");
        }
    }
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]RefreshLNN failed, ret: %{public}d.", ret);
        return ERR_DM_REFRESH_LNN_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::StopRefreshSoftbusLNN(uint16_t subscribeId)
{
    LOGI("StopRefreshSoftbusLNN begin, subscribeId: %{public}d.", (int32_t)subscribeId);
    deviceIdSet.clear();
    int32_t ret = ::StopRefreshLNN(DM_PKG_NAME, subscribeId);
    struct RadarInfo info = {
        .funcName = "StopRefreshSoftbusLNN",
        .hostName = "dsoftbus",
        .stageRes = (ret == DM_OK) ?
                    static_cast<int32_t>(StageRes::STAGE_CANCEL) : static_cast<int32_t>(StageRes::STAGE_FAIL),
        .bizState = (ret == DM_OK) ?
                    static_cast<int32_t>(BizState::BIZ_STATE_CANCEL) : static_cast<int32_t>(BizState::BIZ_STATE_END),
        .errCode = ret,
    };
    if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
        if (!GetDmRadarHelperObj()->ReportDiscoverUserRes(info)) {
            LOGE("ReportDiscoverUserRes failed");
        }
    }
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopRefreshLNN failed, ret: %{public}d.", ret);
        return ERR_DM_STOP_REFRESH_LNN_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::PublishSoftbusLNN(const DmPublishInfo &dmPubInfo, const std::string &capability,
    const std::string &customData)
{
    LOGI("PublishSoftbusLNN begin, publishId: %{public}d.", dmPubInfo.publishId);
    PublishInfo publishInfo;
    publishInfo.publishId = dmPubInfo.publishId;
    publishInfo.mode = static_cast<DiscoverMode>(dmPubInfo.mode);
    publishInfo.medium = (capability == DM_CAPABILITY_APPROACH) ? ExchangeMedium::BLE : ExchangeMedium::AUTO;
    publishInfo.freq = static_cast<ExchangeFreq>(dmPubInfo.freq);
    publishInfo.capability = capability.c_str();
    publishInfo.capabilityData = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(customData.c_str()));
    publishInfo.dataLen = customData.length();
    publishInfo.ranging = dmPubInfo.ranging;

    LOGI("PublishSoftbusLNN begin, publishId: %{public}d, mode: 0x%{public}x, medium: %{public}d, capability:"
        "%{public}s, ranging: %{public}d, freq: %{public}d.", publishInfo.publishId, publishInfo.mode,
        publishInfo.medium, publishInfo.capability, publishInfo.ranging, publishInfo.freq);

    int32_t ret = ::PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]PublishLNN failed, ret: %{public}d.", ret);
        return ERR_DM_PUBLISH_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::StopPublishSoftbusLNN(int32_t publishId)
{
    LOGI("StopPublishSoftbusLNN begin, publishId: %{public}d.", publishId);
    int32_t ret = ::StopPublishLNN(DM_PKG_NAME, publishId);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopPublishLNN failed, ret: %{public}d.", ret);
        return ERR_DM_STOP_PUBLISH_LNN_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::RegisterSoftbusLnnOpsCbk(const std::string &pkgName,
    const std::shared_ptr<ISoftbusDiscoveringCallback> callback)
{
    if (callback == nullptr) {
        LOGE("RegisterSoftbusDiscoveringCbk failed, input callback is null.");
        return ERR_DM_POINT_NULL;
    }
    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    lnnOpsCbkMap.erase(pkgName);
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
    static int32_t radarDeviceCount = 0;
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    struct RadarInfo radarInfo = {
        .localUdid = std::string(localDeviceId),
    };
    if (ret != DM_OK) {
        radarInfo.stageRes = static_cast<int32_t>(StageRes::STAGE_FAIL);
        radarInfo.errCode = ERR_DM_FAILED;
        radarInfo.discoverDevList = "";
        if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
            if (!GetDmRadarHelperObj()->ReportGetTrustDeviceList(radarInfo)) {
                LOGE("ReportGetTrustDeviceList failed");
            }
        }
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %{public}d.", ret);
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
    if (radarDeviceCount != deviceCount && deviceCount > 0
        && IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
        radarDeviceCount = deviceCount;
        radarInfo.discoverDevList = GetDmRadarHelperObj()->GetDeviceInfoList(deviceInfoList);
        if (!GetDmRadarHelperObj()->ReportGetTrustDeviceList(radarInfo)) {
            LOGE("ReportGetTrustDeviceList failed");
        }
    }
    FreeNodeInfo(nodeInfo);
    free(info);
    LOGI("GetTrustDevices success, deviceCount: %{public}d.", deviceCount);
    return ret;
}

int32_t SoftbusListener::GetAvailableDeviceList(std::vector<DmDeviceBasicInfo> &deviceBasicInfoList)
{
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %{public}d.", ret);
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
    LOGI("GetAvailableDevices success, deviceCount: %{public}d.", deviceCount);
    return ret;
}

int32_t SoftbusListener::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    int32_t nodeInfoCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &nodeInfoCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    for (int32_t i = 0; i < nodeInfoCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        if (networkId == nodeBasicInfo->networkId) {
            LOGI("GetDeviceInfo name : %{public}s.", GetAnonyString(nodeBasicInfo->deviceName).c_str());
            if (memcpy_s(info.deviceName, sizeof(info.deviceName), nodeBasicInfo->deviceName,
                std::min(sizeof(info.deviceName), sizeof(nodeBasicInfo->deviceName))) != DM_OK) {
                LOGE("GetDeviceInfo deviceName copy deviceName data failed.");
            }
            info.deviceTypeId = nodeBasicInfo->deviceTypeId;
            break;
        }
    }
    FreeNodeInfo(nodeInfo);
    LOGI("GetDeviceInfo complete, deviceName : %{public}s, deviceTypeId : %{public}d.",
        GetAnonyString(info.deviceName).c_str(), info.deviceTypeId);
    return ret;
}

int32_t SoftbusListener::GetLocalDeviceInfo(DmDeviceInfo &deviceInfo)
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
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
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
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
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
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
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
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
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
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
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
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
        LOGE("[SOFTBUS]ShiftLNNGear error, failed ret: %{public}d", ret);
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
    std::string extraData = devInfo.extraData;
    nlohmann::json extraJson;
    if (!extraData.empty()) {
        extraJson = nlohmann::json::parse(extraData, nullptr, false);
    }
    if (!extraJson.is_discarded()) {
        extraJson[PARAM_KEY_OS_TYPE] = nodeInfo.osType;
        extraJson[PARAM_KEY_OS_VERSION] = std::string(nodeInfo.osVersion);
        devInfo.extraData = to_string(extraJson);
    }
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
    if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_ETH) {
        std::string wifiIp((addrInfo->info).ip.ip);
        jsonObj[PARAM_KEY_WIFI_IP] = wifiIp;
        jsonObj[PARAM_KEY_WIFI_PORT] = (addrInfo->info).ip.port;
        jsonObj[PARAM_KEY_CONN_ADDR_TYPE] = CONN_ADDR_TYPE_ETH_IP;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_WLAN) {
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
        LOGI("Unknown connection address type: %{public}d.", addrInfo->type);
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
    LOGI("GetNetworkTypeByNetworkId networkType %{public}d.", tempNetworkType);
    return DM_OK;
}

int32_t SoftbusListener::GetDeviceSecurityLevel(const char *networkId, int32_t &securityLevel)
{
    int32_t tempSecurityLevel = -1;
    if (GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_DEVICE_SECURITY_LEVEL,
                       reinterpret_cast<uint8_t *>(&tempSecurityLevel), LNN_COMMON_LEN) != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
        return ERR_DM_FAILED;
    }
    securityLevel = tempSecurityLevel;
    LOGI("GetDeviceSecurityLevel success, securityLevel = %{public}d.", securityLevel);
    return DM_OK;
}

void SoftbusListener::CacheDiscoveredDevice(const DeviceInfo *device)
{
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    if (memcpy_s(infoPtr.get(), sizeof(DeviceInfo), device, sizeof(DeviceInfo)) != DM_OK) {
        LOGE("CacheDiscoveredDevice error, copy device info failed.");
        return;
    }

    std::lock_guard<std::mutex> lock(g_deviceMapMutex);
    if (discoveredDeviceMap.size() == MAX_CACHED_DISCOVERED_DEVICE_SIZE) {
        discoveredDeviceMap.erase(discoveredDeviceMap.begin());
    }
    CacheDeviceInfo(device->devId, infoPtr);
}

int32_t SoftbusListener::GetTargetInfoFromCache(const std::string &deviceId, PeerTargetId &targetId,
    ConnectionAddrType &addrType)
{
    std::lock_guard<std::mutex> lock(g_deviceMapMutex);
    auto iter = discoveredDeviceMap.find(deviceId);
    if (iter == discoveredDeviceMap.end()) {
        LOGE("GetTargetInfoFromCache failed, cannot found device in cached discovered map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    auto deviceVectorIter = iter->second;
    if (deviceVectorIter.size() == 0) {
        LOGE("GetTargetInfoFromCache failed, cannot found deviceVectorIter in cached discovered map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    const ConnectionAddr *addrInfo = &((--deviceVectorIter.end())->second->addr)[0];
    if (addrInfo == nullptr) {
        LOGE("GetTargetInfoFromCache failed, connection address of discovered device is nullptr.");
        return ERR_DM_BIND_COMMON_FAILED;
    }

    addrType = addrInfo->type;
    if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_ETH) {
        std::string wifiIp((addrInfo->info).ip.ip);
        targetId.wifiIp = wifiIp;
        targetId.wifiPort = (addrInfo->info).ip.port;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_WLAN) {
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
        LOGI("Unknown connection address type: %{public}d.", addrInfo->type);
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

IDmRadarHelper* SoftbusListener::GetDmRadarHelperObj()
{
    return dmRadarHelper_;
}

bool SoftbusListener::IsDmRadarHelperReady()
{
    LOGI("SoftbusListener::IsDmRadarHelperReady start.");
    std::lock_guard<std::mutex> lock(g_radarLoadLock);
    if (isRadarSoLoad_ && (dmRadarHelper_ != nullptr) && (radarHandle_ != nullptr)) {
        LOGI("IsDmRadarHelperReady alReady");
        if (timer_ != nullptr) {
            timer_->DeleteTimer(std::string(LOAD_RADAR_TIMEOUT_TASK));
            timer_->StartTimer(std::string(LOAD_RADAR_TIMEOUT_TASK), LOAD_RADAR_TIMEOUT,
                [=] (std::string name) {
                    SoftbusListener::CloseDmRadarHelperObj(name);
                });
        }
        return true;
    }
    char path[PATH_MAX + 1] = {0x00};
    std::string soName = std::string(DM_LIB_LOAD_PATH) + std::string(LIB_RADAR_NAME);
    if ((soName.length() == 0) || (soName.length() > PATH_MAX) || (realpath(soName.c_str(), path) == nullptr)) {
        LOGE("File %{public}s canonicalization failed.", soName.c_str());
        return false;
    }
    radarHandle_ = dlopen(path, RTLD_NOW);
    if (radarHandle_ == nullptr) {
        LOGE("load libdevicemanagerradar so %{public}s failed.", soName.c_str());
        return false;
    }
    dlerror();
    auto func = (CreateDmRadarFuncPtr)dlsym(radarHandle_, "CreateDmRadarInstance");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(radarHandle_);
        LOGE("Create object function is not exist.");
        return false;
    }
    isRadarSoLoad_ = true;
    dmRadarHelper_ = func();
    LOGI("IsDmRadarHelperReady ready success");
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(LOAD_RADAR_TIMEOUT_TASK));
        timer_->StartTimer(std::string(LOAD_RADAR_TIMEOUT_TASK), LOAD_RADAR_TIMEOUT,
            [=](std::string name) { SoftbusListener::CloseDmRadarHelperObj(name); });
    }
    return true;
}

bool SoftbusListener::CloseDmRadarHelperObj(std::string name)
{
    (void)name;
    LOGI("SoftbusListener::CloseDmRadarHelperObj start.");
    std::lock_guard<std::mutex> lock(g_radarLoadLock);
    if (!isRadarSoLoad_ && (dmRadarHelper_ == nullptr) && (radarHandle_ == nullptr)) {
        return true;
    }

    int32_t ret = dlclose(radarHandle_);
    if (ret != 0) {
        LOGE("close libdevicemanagerradar failed ret = %{public}d.", ret);
        return false;
    }
    isRadarSoLoad_ = false;
    dmRadarHelper_ = nullptr;
    radarHandle_ = nullptr;
    LOGI("close libdevicemanagerradar so success.");
    return true;
}

void SoftbusListener::SetHostPkgName(const std::string hostName)
{
    hostName_ = hostName;
    LOGI("SetHostPkgName::hostName_ :%s.", hostName_.c_str());
}

std::string SoftbusListener::GetHostPkgName()
{
    LOGI("GetHostPkgName::hostName_ :%s.", hostName_.c_str());
    return hostName_;
}

void SoftbusListener::CacheDeviceInfo(const std::string deviceId, std::shared_ptr<DeviceInfo> infoPtr)
{
    if (deviceId.empty()) {
        return;
    }
    if (infoPtr->addrNum <= 0) {
        LOGE("CacheDeviceInfo failed, infoPtr->addr is empty.");
        return;
    }
    ConnectionAddrType addrType;
    const ConnectionAddr *addrInfo = &(infoPtr->addr)[0];
    if (addrInfo == nullptr) {
        LOGE("CacheDeviceInfo failed, connection address of discovered device is nullptr.");
        return;
    }
    addrType = addrInfo->type;
    std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>> deviceVec;
    auto iter = discoveredDeviceMap.find(deviceId);
    if (iter != discoveredDeviceMap.end()) {
        deviceVec = iter->second;
        for (auto it = deviceVec.begin(); it != deviceVec.end();) {
            if (it->first == addrType) {
                it = deviceVec.erase(it);
                continue;
            } else {
                it++;
            }
        }
        discoveredDeviceMap.erase(deviceId);
    }
    deviceVec.push_back(std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>(addrType, infoPtr));
    discoveredDeviceMap.insert(std::pair<std::string,
        std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>>>(deviceId, deviceVec));
}

int32_t SoftbusListener::GetIPAddrTypeFromCache(const std::string &deviceId, const std::string &ip,
    ConnectionAddrType &addrType)
{
    std::lock_guard<std::mutex> lock(g_deviceMapMutex);
    auto iter = discoveredDeviceMap.find(deviceId);
    if (iter == discoveredDeviceMap.end()) {
        LOGE("GetIPAddrTypeFromCache failed, cannot found device in cached discovered map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    auto deviceVectorIter = iter->second;
    if (deviceVectorIter.size() == 0) {
        LOGE("GetTargetInfoFromCache failed, cannot found deviceVectorIter in cached discovered map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    for (auto it = deviceVectorIter.begin(); it != deviceVectorIter.end(); ++it) {
        const ConnectionAddr *addrInfo = &((it->second)->addr)[0];
        if (addrInfo == nullptr) {
            continue;
        }
        if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_ETH ||
            addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_WLAN) {
            std::string cacheIp((addrInfo->info).ip.ip);
            if (cacheIp == ip) {
                addrType = addrInfo->type;
                return DM_OK;
            }
        }
    }
    return ERR_DM_BIND_INPUT_PARA_INVALID;
}

IRefreshCallback &SoftbusListener::GetSoftbusRefreshCb()
{
    return softbusRefreshCallback_;
}
} // namespace DistributedHardware
} // namespace OHOS