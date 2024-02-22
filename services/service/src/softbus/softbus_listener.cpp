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

#include <dlfcn.h>
#include <mutex>
#include <pthread.h>
#include <securec.h>
#include <thread>
#include <unistd.h>
#include <condition_variable>
#include <list>

#include "device_manager_service.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"
#include "system_ability_definition.h"
#include "dm_softbus_adapter_crypto.h"
#include "softbus_adapter.cpp"
#include "softbus_common.h"
#include "softbus_bus_center.h"
#include "dm_radar_helper.h"
#include "nlohmann/json.hpp"
#if (defined(MINE_HARMONY))
#include "base64.h"
#include "sha256.h"
#include "md.h"
#endif

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
constexpr uint32_t DM_MAX_SCOPE_TLV_NUM = 3;
constexpr uint32_t DM_MAX_VERTEX_TLV_NUM = 6;
constexpr int32_t SHA256_OUT_DATA_LEN = 32;
constexpr int32_t MAX_RETRY_TIMES = 30;
#if (defined(MINE_HARMONY))
constexpr int32_t DM_MAX_DEVICE_ALIAS_LEN = 65;
constexpr int32_t DM_MAX_DEVICE_UDID_LEN = 65;
constexpr int32_t DM_INVALID_DEVICE_NUMBER = -1;
constexpr int32_t DM_SEARCH_BROADCAST_MIN_LEN = 18;
constexpr int32_t DM_TLV_VERTEX_DATA_OFFSET = 2;
constexpr int32_t DM_TLV_SCOPE_DATA_OFFSET = 4;
constexpr int32_t MAX_SOFTBUS_DELAY_TIME = 10;
#endif
constexpr const char* FIELD_DEVICE_MODE = "findDeviceMode";
constexpr const char* FIELD_TRUST_OPTIONS = "tructOptions";
constexpr const char* FIELD_FILTER_OPTIONS = "filterOptions";
#if (defined(MINE_HARMONY))
constexpr const char* DEVICE_ALIAS = "persist.devicealias";
constexpr const char* DEVICE_NUMBER = "persist.devicenumber";
constexpr const char* DM_DISCOVER_TYPE = "DISCOVER_TYPE";
constexpr const char* DM_DISCOVER_SEARCHJSON = "SEARCHJSON";
constexpr const char* DM_DISCOVER_EXTRA = "EXTRA";
constexpr const char* DM_DISCOVER_SUBSCRIBEID = "SUBSCRIBEID";
#endif
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
constexpr char BROADCAST_VERSION = 1;
constexpr char FIND_ALL_DEVICE = 1;
constexpr char FIND_SCOPE_DEVICE = 2;
constexpr char FIND_VERTEX_DEVICE = 3;
constexpr char FIND_TRUST_DEVICE = 3;
constexpr char DEVICE_ALIAS_NUMBER = 1;
constexpr char DEVICE_TYPE_TYPE = 1;
constexpr char DEVICE_SN_TYPE = 2;
constexpr char DEVICE_UDID_TYPE = 3;
#if (defined(MINE_HARMONY))
        constexpr char FIND_NOTRUST_DEVICE = 2;
#endif

static PulishStatus g_publishStatus = PulishStatus::STATUS_UNKNOWN;
static std::mutex g_deviceMapMutex;
static std::mutex g_lnnCbkMapMutex;
static std::mutex g_radarLoadLock;
static std::mutex g_matchWaitDeviceLock;
static std::mutex g_publishLnnlock;
static std::list<DeviceInfo> g_matchQueue;
static std::map<std::string, std::shared_ptr<DeviceInfo>> discoveredDeviceMap;
static std::map<std::string, std::shared_ptr<ISoftbusDiscoveringCallback>> lnnOpsCbkMap;
static std::vector<std::string> pkgNameVec_ = {};
bool SoftbusListener::isRadarSoLoad_ = false;
IDmRadarHelper* SoftbusListener::dmRadarHelper_ = nullptr;
std::shared_ptr<DmTimer> SoftbusListener::timer_ = std::make_shared<DmTimer>();
void* SoftbusListener::radarHandle_ = nullptr;
bool g_publishLnnFlag = false;
bool g_matchDealFlag = false;
std::condition_variable g_matchDealNotify;
std::condition_variable g_publishLnnNotify;

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

static IPublishCb publishLNNCallback_ = {
#if (defined(MINE_HARMONY))
        .OnPublishResult = OnPublishResult,
        .OndeviceFound = OnPublishDeviceFound,
        .onRePublish = OnRePublish
#endif
};

void from_json(const json &object, ScopeOptionInfo &optionInfo)
{
    if (!object.contains("deviceAlias") || !object["deviceAlias"].is_string()) {
        LOGE("OptionInfo deviceAlias json key is not exist or error.");
        return;
    }
    if (!object.contains("startNumber") || !object["startNumber"].is_number_integer()) {
        LOGE("OptionInfo startNumber json key is not exist or error.");
        return;
    }
    if (!object.contains("endNumber") || !object["endNumber"].is_number_integer()) {
        LOGE("OptionInfo endNumber json key is not exist or error.");
        return;
    }
    object["deviceAlias"].get_to(optionInfo.deviceAlias);
    object["startNumber"].get_to(optionInfo.startNumber);
    object["endNumber"].get_to(optionInfo.endNumber);
}

void from_json(const json &object, VertexOptionInfo &optionInfo)
{
    if (!object.contains("type") || !object["type"].is_string()) {
        LOGE("OptionInfo type json key is not exist or type error.");
        return;
    }
    if (!object.contains("value") || !object["value"].is_number_integer()) {
        LOGE("OptionInfo value json key is not exist or value error.");
        return;
    }
    object["type"].get_to(optionInfo.type);
    object["value"].get_to(optionInfo.value);
}

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
    LOGI("device online networkId: %s.", GetAnonyString(dmDeviceInfo.networkId).c_str());
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
    LOGI("device offline networkId: %s.", GetAnonyString(dmDeviceInfo.networkId).c_str());
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
        LOGI("device changed networkId: %s.", GetAnonyString(dmDeviceInfo.networkId).c_str());
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
    LOGI("OnSoftbusDeviceFound: devId=%s, devName=%s, devType=%d, range=%d, isOnline=%d",
        GetAnonyString(dmDevInfo.deviceId).c_str(), GetAnonyString(dmDevInfo.deviceName).c_str(),
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
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
            LOGE("[SOFTBUS]RegNodeDeviceStateCb failed with ret: %d, retryTimes: %d.", ret, retryTimes);
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
    LOGI("RefreshSoftbusLNN begin, subscribeId: %d.", dmSubInfo.subscribeId);
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
    LOGI("RefreshSoftbusLNN begin, subscribeId: %d, mode: 0x%x, medium: %d, capability: %s.",
         subscribeInfo.subscribeId, subscribeInfo.mode, subscribeInfo.medium, subscribeInfo.capability);
    int32_t ret = ::RefreshLNN(pkgName, &subscribeInfo, &softbusRefreshCallback_);
    struct RadarInfo info = {
        .funcName = "RefreshSoftbusLNN",
        .toCallPkg = "dsoftbus",
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
        LOGE("[SOFTBUS]RefreshLNN failed, ret: %d.", ret);
        return ERR_DM_REFRESH_LNN_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::StartDiscovery(const string &pkgName, const string &searchJson,
                                        const DmSubscribeInfo &dmSubscribeInfo)
{
    LOGI("start to start dictionary device with pkgName: %s", pkgName.c_str());
    size_t outlen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    if (ParseSearchJson(pkgName, searchJson, output, &outlen) != DM_OK) {
        LOGE("failed to parse searchJson with pkgName: %s", pkgName.c_str());
        return ERR_DM_JSON_PARSE_STRING;
    }
    SubscribeInfo subscribeInfo;
    SetSubscribeInfo(dmSubscribeInfo, subscribeInfo);
    if (SendBroadcastInfo(pkgName, subscribeInfo, output, &outlen) != DM_OK) {
        LOGE("failed to start quick discovery beause sending broadcast info.");
        return ERR_DM_SOFTBUS_SEND_BROADCAST;
    }
    LOGI("start dictionary device successfully with pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t SoftbusListener::StopRefreshSoftbusLNN(uint16_t subscribeId)
{
#ifdef  MINE_HARMONY
    int retValue = StopRefreshLNN(DM_PKG_NAME, subscribeId);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to stop discovery device with ret:%d", retValue);
        return ERR_DM_SOFTBUS_DISCOVERY_DEVICE;
    }
    return DM_OK;
#else
    LOGI("StopRefreshSoftbusLNN begin, subscribeId: %d.", (int32_t)subscribeId);
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
        LOGE("[SOFTBUS]StopRefreshLNN failed, ret: %d.", ret);
        return ERR_DM_STOP_REFRESH_LNN_FAILED;
    }
    return DM_OK;
#endif
}

int32_t SoftbusListener::PublishSoftbusLNN(const DmPublishInfo &dmPubInfo, const std::string &capability,
    const std::string &customData)
{
    LOGI("PublishSoftbusLNN begin, publishId: %d.", dmPubInfo.publishId);
    PublishInfo publishInfo;
    publishInfo.publishId = dmPubInfo.publishId;
    publishInfo.mode = static_cast<DiscoverMode>(dmPubInfo.mode);
    publishInfo.medium = (capability == DM_CAPABILITY_APPROACH) ? ExchangeMedium::BLE : ExchangeMedium::AUTO;
    publishInfo.freq = static_cast<ExchangeFreq>(dmPubInfo.freq);
    publishInfo.capability = capability.c_str();
    publishInfo.capabilityData = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(customData.c_str()));
    publishInfo.dataLen = customData.length();
    publishInfo.ranging = dmPubInfo.ranging;

    LOGI("PublishSoftbusLNN begin, publishId: %d, mode: 0x%x, medium: %d, capability: %s, ranging: %d.",
        publishInfo.publishId, publishInfo.mode, publishInfo.medium, publishInfo.capability, publishInfo.ranging);

    int32_t ret = ::PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]PublishLNN failed, ret: %d.", ret);
        return ERR_DM_PUBLISH_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusListener::StopPublishSoftbusLNN(int32_t publishId)
{
    LOGI("StopPublishSoftbusLNN begin, publishId: %d.", publishId);
    int32_t ret = ::StopPublishLNN(DM_PKG_NAME, publishId);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopPublishLNN failed, ret: %d.", ret);
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
        radarInfo.discoverDevList = "";
        if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
            if (!GetDmRadarHelperObj()->ReportGetTrustDeviceList(radarInfo)) {
                LOGE("ReportGetTrustDeviceList failed");
            }
        }
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
    if (deviceCount > 0 && IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
        radarInfo.discoverDevList = GetDmRadarHelperObj()->GetDeviceInfoList(deviceInfoList);
        if (!GetDmRadarHelperObj()->ReportGetTrustDeviceList(radarInfo)) {
            LOGE("ReportGetTrustDeviceList failed");
        }
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
            LOGI("GetDeviceInfo name : %s.", GetAnonyString(nodeBasicInfo->deviceName).c_str());
            if (memcpy_s(info.deviceName, sizeof(info.deviceName), nodeBasicInfo->deviceName,
                std::min(sizeof(info.deviceName), sizeof(nodeBasicInfo->deviceName))) != DM_OK) {
                LOGE("GetDeviceInfo deviceName copy deviceName data failed.");
            }
            info.deviceTypeId = nodeBasicInfo->deviceTypeId;
            break;
        }
    }
    FreeNodeInfo(nodeInfo);
    LOGI("GetDeviceInfo complete, deviceName : %s, deviceTypeId : %d.", GetAnonyString(info.deviceName).c_str(),
        info.deviceTypeId);
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
    std::string extraData = devInfo.extraData;
    nlohmann::json extraJson;
    if (!extraData.empty()) {
        extraJson = nlohmann::json::parse(extraData, nullptr, false);
    }
    extraJson[PARAM_KEY_OS_TYPE] = nodeInfo.osType;
    extraJson[PARAM_KEY_OS_VERSION] = std::string(nodeInfo.osVersion);
    devInfo.extraData = to_string(extraJson);
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

int32_t SoftbusListener::GetTargetInfoFromCache(const std::string &deviceId, PeerTargetId &targetId,
    ConnectionAddrType &addrType)
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
        LOGE("File %s canonicalization failed.", soName.c_str());
        return false;
    }
    radarHandle_ = dlopen(path, RTLD_NOW);
    if (radarHandle_ == nullptr) {
        LOGE("load libdevicemanagerradar so %s failed.", soName.c_str());
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
        LOGE("close libdevicemanagerradar failed ret = %d.", ret);
        return false;
    }
    isRadarSoLoad_ = false;
    dmRadarHelper_ = nullptr;
    radarHandle_ = nullptr;
    LOGI("close libdevicemanagerradar so success.");
    return true;
}

void SoftbusListener::OnPublishResult(int publishId, PublishResult reason)
{
    std::unique_lock<std::mutex> locker(g_publishLnnlock);
    if (reason == PUBLISH_LNN_SUCCESS) {
        g_publishLnnFlag = true;
        LOGI("publishLNN successfully with publishId: %d.", publishId);
    } else {
        g_publishLnnFlag = false;
        LOGE("failed to publishLNN with publishId: %d, reason: %d.", publishId, (int)reason);
    }
    g_publishLnnNotify.notify_one();
}
void SoftbusListener::OnPublishDeviceFound(const DeviceInfo *deviceInfo)
{
    if (deviceInfo == nullptr) {
        LOGE("deviceInfo is nullptr.");
        return;
    }
#if (defined(MINE_HARMONY))
    if (deviceInfo->businessDataLen >= DISC_MAX_CUST_DATA_LEN ||
        deviceInfo->businessDataLen < DM_SEARCH_BROADCAST_MIN_LEN) {
        LOGE("deviceInfo data is too long or to short with dataLen: %u", deviceInfo->businessDataLen);
        return;
    }
    LOGI("broadcast data is received with DataLen: %u", deviceInfo->businessDataLen);
#endif
    std::unique_lock<std::mutex> autoLock(g_matchWaitDeviceLock);
    g_matchQueue.push_back(*deviceInfo);
    g_matchDealNotify.notify_one();
}
void SoftbusListener::OnRePublish(void)
{
    LOGI("try to rePublishLNN");
    int32_t retryTimes = 0;
    PublishInfo publishInfo;
    publishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    publishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    publishInfo.medium = ExchangeMedium::COAP;
    publishInfo.freq = ExchangeFreq::LOW;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.capabilityData = nullptr;
    publishInfo.dataLen = 0;
    retryTimes = 0;
    while (PublishLNN(DM_PKG_NAME, &publishInfo, &publishLNNCallback_) != SOFTBUS_OK &&
        retryTimes <= MAX_RETRY_TIMES) {
        retryTimes++;
        LOGW("failed to rePublishLNN with retryTimes: %d", retryTimes);
        usleep(SOFTBUS_CHECK_INTERVAL);
    }
    LOGI("rePublishLNN finish");
}

int32_t SoftbusListener::ParseSearchJson(const string &pkgName, const string &searchJson, char *output,
    size_t *outLen)
{
    json object = json::parse(searchJson, nullptr, false);
    if (object.is_discarded()) {
        LOGE("failed to parse filter options string.");
        return ERR_DM_INVALID_JSON_STRING;
    }
    int32_t retValue = DM_OK;
    uint32_t findMode = object[FIELD_DEVICE_MODE];
    LOGI("quick search device mode is: %u", findMode);
    switch (findMode) {
        case FIND_ALL_DEVICE:
            retValue = ParseSearchAllDevice(object, pkgName, output, outLen);
            break;
        case FIND_SCOPE_DEVICE:
            retValue = ParseSearchScopeDevice(object, pkgName, output, outLen);
            break;
        case FIND_VERTEX_DEVICE:
            retValue = ParseSearchVertexDevice(object, pkgName, output, outLen);
            break;
        default:
            LOGE("key type is not match key: %s.", FIELD_DEVICE_MODE);
    }
    if (retValue != DM_OK) {
        LOGE("fail to parse search find device with ret: %d.", retValue);
        return retValue;
    }
    LOGI("parse search json successfully with pkgName: %s, outLen: %u,", pkgName.c_str(), *outLen);
    return DM_OK;
}

int32_t SoftbusListener::ParseSearchAllDevice(const nlohmann::json &object, const string &pkgName, char *output,
    size_t *outLen)
{
    BroadcastHead broadcastHead;
    if (SetBroadcastHead(object, pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set broadcast head.");
        return ERR_DM_FAILED;
    }
    broadcastHead.tlvDataLen = 0;
    broadcastHead.findMode = FIND_ALL_DEVICE;
    AddHeadToBroadcast(broadcastHead, output);
    *outLen = sizeof(BroadcastHead);
    return DM_OK;
}

int32_t SoftbusListener::ParseSearchScopeDevice(const nlohmann::json &object, const string &pkgName, char *output,
    size_t *outLen)
{
    BroadcastHead broadcastHead;
    if (SetBroadcastHead(object, pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set broadcast head.");
        return ERR_DM_FAILED;
    }
    if (!object.contains(FIELD_FILTER_OPTIONS) || !object[FIELD_FILTER_OPTIONS].is_array()) {
        LOGE("failed to get %s scope cjson object or is not array.", FIELD_FILTER_OPTIONS);
        return ERR_DM_FAILED;
    }
    auto optionInfoVec = object[FIELD_FILTER_OPTIONS].get<std::vector<ScopeOptionInfo>>();
    size_t optionInfoVecSize = optionInfoVec.size();
    if (optionInfoVecSize == 0 || optionInfoVecSize > DM_MAX_SCOPE_TLV_NUM) {
        LOGE("failed to get search josn array lenght.");
        return ERR_DM_INVALID_JSON_STRING;
    }
    LOGI("start to parse scope search array json wuth size:%d.", optionInfoVecSize);
    if (ParseScopeDeviceJsonArray(optionInfoVec, output + sizeof(BroadcastHead), outLen) != DM_OK) {
        LOGE("failed to parse scope json array.");
        return ERR_DM_FAILED;
    }

    broadcastHead.findMode = FIND_SCOPE_DEVICE;
    broadcastHead.tlvDataLen = *outLen;
    AddHeadToBroadcast(broadcastHead, output);
    *outLen = *outLen + sizeof(BroadcastHead);
    return DM_OK;
}

int32_t SoftbusListener::ParseSearchVertexDevice(const nlohmann::json &object, const string &pkgName, char *output,
    size_t *outLen)
{
    BroadcastHead broadcastHead;
    if (SetBroadcastHead(object, pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set broadcast head.");
        return ERR_DM_FAILED;
    }
    if (!object.contains(FIELD_FILTER_OPTIONS) || !object[FIELD_FILTER_OPTIONS].is_array()) {
        LOGE("failed to get %s vertex cjson object or is not array.", FIELD_FILTER_OPTIONS);
        return ERR_DM_FAILED;
    }
    auto optionInfoVec = object[FIELD_FILTER_OPTIONS].get<std::vector<VertexOptionInfo>>();
    size_t optionInfoVecSize = optionInfoVec.size();
    if (optionInfoVecSize == 0 || optionInfoVecSize > DM_MAX_VERTEX_TLV_NUM) {
        LOGE("failed to get search josn array lenght.");
        return ERR_DM_FAILED;
    }
    LOGI("start to parse vertex search array json with size: %d.", optionInfoVecSize);
    if (ParseVertexDeviceJsonArray(optionInfoVec, output + sizeof(BroadcastHead), outLen) != DM_OK) {
        LOGE("failed to parse vertex json array.");
        return ERR_DM_FAILED;
    }

    broadcastHead.findMode = FIND_VERTEX_DEVICE;
    broadcastHead.tlvDataLen = *outLen;
    AddHeadToBroadcast(broadcastHead, output);
    *outLen = *outLen + sizeof(BroadcastHead);
    return DM_OK;
}

int32_t SoftbusListener::SetBroadcastHead(const json &object, const string &pkgName, BroadcastHead &broadcastHead)
{
    broadcastHead.version = BROADCAST_VERSION;
    broadcastHead.headDataLen = sizeof(BroadcastHead);
    broadcastHead.tlvDataLen = 0;
    broadcastHead.findMode = 0;
    if (SetBroadcastTrustOptions(object, broadcastHead) != DM_OK) {
        LOGE("fail to set trust options to search broadcast.");
        return ERR_DM_FAILED;
    }
    if (SetBroadcastPkgname(pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set pkgname to search broadcast.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void SoftbusListener::AddHeadToBroadcast(const BroadcastHead &broadcastHead, char *output)
{
    size_t startPos = 0;
    output[startPos++] = broadcastHead.version;
    output[startPos++] = broadcastHead.headDataLen;
    output[startPos++] = broadcastHead.tlvDataLen;
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[startPos++] = broadcastHead.pkgNameHash[i];
    }
    output[startPos++] = broadcastHead.findMode;
    output[startPos++] = broadcastHead.trustFilter;
    LOGI("find device info with version: %d, findMode: %d, HeadLen: %d, tlvDataLen: %d, trustFilter: %d",
         (int)(broadcastHead.version), (int)(broadcastHead.findMode), (int)(broadcastHead.headDataLen),
         (int)(broadcastHead.tlvDataLen), (int)(broadcastHead.trustFilter));
}

int32_t SoftbusListener::ParseScopeDeviceJsonArray(const vector<ScopeOptionInfo> &optionInfo,
    char *output, size_t *outLen)
{
    errno_t retValue = EOK;
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    size_t arraySize = optionInfo.size();

    for (size_t i = 0; i < arraySize; i++) {
        if (GetSha256Hash(optionInfo[i].deviceAlias.c_str(),
                          optionInfo[i].deviceAlias.size(), sha256Out) != DM_OK) {
            LOGE("failed to get sha256 hash with index: %d, value: %s.", i, optionInfo[i].deviceAlias.c_str());
            return ERR_DM_FAILED;
        }
        output[(*outLen)++] = DEVICE_ALIAS_NUMBER;
        output[(*outLen)++] = DM_HASH_DATA_LEN;
        output[(*outLen)++] = DM_DEVICE_NUMBER_LEN;
        output[(*outLen)++] = DM_DEVICE_NUMBER_LEN;
        for (int j = 0; j < (int32_t)DM_HASH_DATA_LEN; j++) {
            output[(*outLen)++] = sha256Out[j];
        }
        retValue = sprintf_s(&output[*outLen], DM_DEVICE_NUMBER_LEN, "%010d", optionInfo[i].startNumber);
        if (retValue == EOK) {
            LOGE("fail to add device number to data buffer");
            return ERR_DM_FAILED;
        }
        *outLen = *outLen + DM_DEVICE_NUMBER_LEN;
        retValue = sprintf_s(&output[*outLen], DM_DEVICE_NUMBER_LEN, "%010d", optionInfo[i].startNumber);
        if (retValue == EOK) {
            LOGE("fail to add device number to data buffer");
            return ERR_DM_FAILED;
        }
        *outLen = *outLen + DM_DEVICE_NUMBER_LEN;
    }
    return DM_OK;
}

int32_t SoftbusListener::ParseVertexDeviceJsonArray(const std::vector<VertexOptionInfo> &optionInfo,
    char *output, size_t *outLen)
{
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    size_t arraySize = optionInfo.size();

    for (size_t i = 0; i < arraySize; i++) {
        if (optionInfo[i].type.empty() || optionInfo[i].value.empty()) {
            LOGE("failed to get type or value cjosn object with index: %u", i);
            continue;
        }
        if (optionInfo[i].type == "deviceUdid") {
            output[(*outLen)++] = DEVICE_UDID_TYPE;
        } else if (optionInfo[i].type == "deviceType") {
            output[(*outLen)++] = DEVICE_TYPE_TYPE;
        } else if (optionInfo[i].type == "deviceSn") {
            output[(*outLen)++] = DEVICE_SN_TYPE;
        } else {
            LOGE("type:%s is not allowed with index: %u.", optionInfo[i].type.c_str(), i);
            return ERR_DM_FAILED;
        }
        output[(*outLen)++] = DM_HASH_DATA_LEN;
        if (GetSha256Hash((const char *) optionInfo[i].value.data(), optionInfo[i].value.size(),
                          sha256Out) != DM_OK) {
            LOGE("failed to get value sha256 hash with index: %u", i);
            return ERR_DM_GET_DATA_SHA256_HASH;
        }
        for (int j = 0; j < (int32_t)DM_HASH_DATA_LEN; j++) {
            output[(*outLen)++] = sha256Out[j];
        }
    }
    return DM_OK;
}

int32_t SoftbusListener::GetSha256Hash(const char *data, size_t len, char *output)
{
#if (defined(MINE_HARMONY))
    int mbedtlsHmacType = 0;
    mbedtls_md_context_t mdContext;
    const mbedtls_md_info_t* mdInfo = NULL;
    mbedtls_md_init(&mdContext);
    mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    int retValue = mbedtls_md_setup(&mdContext, mdInfo, mbedtlsHmacType);
    if (retValue != 0) {
        LOGE("failed to setup mbedtls md with ret: %d.", retValue);
        return ERR_DM_FAILED;
    }
    retValue = mbedtls_md_starts(&mdContext);
    if (retValue != 0) {
        LOGE("failed to start mbedtls md with ret: %d.", retValue);
        mbedtls_md_free(&mdContext);
        return ERR_DM_FAILED;
    }
    retValue = mbedtls_md_update(&mdContext, (const unsigned char *)data, len);
    if (retValue != 0) {
        LOGE("failed to update mbedtls md with ret: %d.", retValue);
        mbedtls_md_free(&mdContext);
        return ERR_DM_FAILED;
    }
    retValue = mbedtls_md_finish(&mdContext, (unsigned char*)output);
    if (retValue != 0) {
        LOGE("failed to finish mbedtls md with ret: %d.", retValue);
        mbedtls_md_free(&mdContext);
        return ERR_DM_FAILED;
    }
    mbedtls_md_free(&mdContext);
#endif
    return DM_OK;
}

int32_t SoftbusListener::SetBroadcastTrustOptions(const json &object, BroadcastHead &broadcastHead)
{
    if (!object.contains(FIELD_TRUST_OPTIONS)) {
        broadcastHead.trustFilter = 0;
        return DM_OK;
    } else if (object[FIELD_TRUST_OPTIONS].is_boolean() && object[FIELD_TRUST_OPTIONS]) {
        broadcastHead.trustFilter = FIND_TRUST_DEVICE;
        return DM_OK;
    } else if (object[FIELD_TRUST_OPTIONS].is_boolean() && !object[FIELD_TRUST_OPTIONS]) {
        broadcastHead.trustFilter = FIND_TRUST_DEVICE;
        return DM_OK;
    }
    LOGE("key type is error with key: %s", FIELD_TRUST_OPTIONS);
    return ERR_DM_FAILED;
}

int32_t SoftbusListener::SetBroadcastPkgname(const string &pkgName, BroadcastHead &broadcastHead)
{
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char*)pkgName.c_str(), pkgName.size(), sha256Out) != DM_OK) {
        LOGE("failed to get search pkgName sha256 hash while search all device.");
        return ERR_DM_FAILED;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        broadcastHead.pkgNameHash[i] = sha256Out[i];
    }
    return DM_OK;
}

void SoftbusListener::SetSubscribeInfo(const DmSubscribeInfo &dmSubscribeInfo, SubscribeInfo &subscribeInfo)
{
    subscribeInfo.subscribeId = dmSubscribeInfo.subscribeId;
    subscribeInfo.mode = (DiscoverMode)dmSubscribeInfo.mode;
    subscribeInfo.medium = (ExchangeMedium)dmSubscribeInfo.medium;
    subscribeInfo.freq = (ExchangeFreq)dmSubscribeInfo.freq;
    subscribeInfo.isSameAccount = dmSubscribeInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubscribeInfo.isWakeRemote;
    subscribeInfo.capability = DM_CAPABILITY_OSD;
    subscribeInfo.capabilityData = nullptr;
    subscribeInfo.dataLen = 0;
}

int32_t SoftbusListener::SendBroadcastInfo(const string &pkgName, SubscribeInfo &subscribeInfo, char *output,
    size_t *outputLen)
{
    size_t base64OutLen = 0;
    int retValue;
#if (defined(MINE_HARMONY))
    char base64Out[DISC_MAX_CUST_DATA_LEN] = {0};
    retValue = mbedtls_base64_encode((unsigned char*)base64Out, DISC_MAX_CUST_DATA_LEN, &base64OutLen,
        (const unsigned char*)output, outputLen);
    if (retValue != 0) {
    LOGE("failed to get search data base64 encode type data with ret: %d.", retValue);
        return ERR_DM_FAILED;
    }
    subscribeInfo.custData = base64Out;
    subscribeInfo.custDataLen = base64OutLen;
#endif
    retValue = RefreshLNN(DM_PKG_NAME, &subscribeInfo, &softbusRefreshCallback_);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to start to refresh quick discovery with ret : %d.", retValue);
        return ERR_DM_FAILED;
    }
    LOGI("send search broadcast info by softbus successfully with dataLen: %u, pkgName: %s.", base64OutLen,
         pkgName.c_str());
    return DM_OK;
}

#if (defined(MINE_HARMONY))

int32_t SoftbusListener::PublishDeviceDiscovery(void)
{
    PublishInfo publishInfo;
    publishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    publishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    publishInfo.medium = ExchanageMedium::AUTO;
    publishInfo.freq = ExchangeFreq::HIGH;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.capabilityData = nullptr;
    publishInfo.dataLen = 0;
    int retValue = PublishLNN(DM_PKG_NAME, &publishInfo, &publishLNNCallback_);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to call softbus publishLNN function with ret: %d.", retValue);
        return ERR_DM_SOFTBUS_PUBLISH_SERVICE;
    }
    std::chrono::seconds timeout = std::chrono::seconds(MAX_SOFTBUS_DELAY_TIME);
    std::unique_lock<std::mutex> locker(g_publishLnnlock);
    if (!g_publishLnnNotify.wait_for(locker, timeout, [] { return g_publishLnnFlag; })) {
        g_publishLnnFlag = false;
        return ERR_DM_SOFTBUS_PUBLISH_SERVICE;
    }
    g_publishLnnFlag = false;
    return DM_OK;
}

void SoftbusListener::MatchSearchDealTask(void)
{
    DeviceInfo tempDeviceInfo;
    LOGI("the match deal task has started to run.");

    while (true) {
        {
            std::unique_lock<std::mutex> autoLock(g_matchWaitDeviceLock);
            if (!g_matchDealFlag) {
                LOGI("the match deal task will stop to run.");
                break;
            }
            g_matchDealNotify.wait(autoLock, [] { return !g_matchQueue.empty(); });
            tempDeviceInfo = g_matchQueue.front();
            g_matchQueue.pop_front();
        }
        if (ParseBroadcastInfo(tempDeviceInfo) != DM_OK) {
            LOGE("failed to parse broadcast info.");
        }
    }
}

int32_t SoftbusListener::ParseBroadcastInfo(DeviceInfo &deviceInfo)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    if (!GetBroadcastData(deviceInfo, output, DISC_MAX_CUST_DATA_LEN)) {
        LOGE("fail to get broadcast data");
        return ERR_DM_FAILED;
    }
    DevicePolicyInfo  devicePolicyInfo;
    Action matchResult = BUSINESS_EXACT_NOT_MATCH;
    BroadcastHead broadcastHead = *(BroadcastHead*)output;
    LOGI("parse device info with version: %d, findMode: %d, HeadLen: %d, tlvDataLen: %d, trustFilter: %d",
         (int)(broadcastHead.version), (int)(broadcastHead.findMode), (int)(broadcastHead.headDataLen),
         (int)(broadcastHead.tlvDataLen), (int)broadcastHead.trustFilter);

    char findMode = broadcastHead.findMode;
    switch (findMode) {
        case FIND_ALL_DEVICE:
            matchResult = MatchSearchAllDevice(deviceInfo, broadcastHead);
            break;
        case FIND_SCOPE_DEVICE:
            GetScopeDevicePolicyInfo(devicePolicyInfo);
            matchResult = MatchSearchScopeDevice(deviceInfo, output + sizeof(BroadcastHead),
                devicePolicyInfo, broadcastHead);
            break;
        case FIND_VERTEX_DEVICE:
            GetVertexDevicePolicyInfo(devicePolicyInfo);
            matchResult = MatchSearchVertexDevice(deviceInfo, output + sizeof(BroadcastHead),
                devicePolicyInfo, broadcastHead);
            break;
        default:
            LOGE("key type is not match key: %d.", FIELD_DEVICE_MODE);
            return ERR_DM_FAILED;
    }
    LOGI("parse broadcast info matchResult: %d.", (int)matchResult);
    if (matchResult == BUSINESS_EXACT_MATCH) {
        return SendReturnwave(deviceInfo, broadcastHead, matchResult);
    }
    return DM_OK;
}

bool SoftbusListener::GetBroadcastData(DeviceInfo &deviceInfo, char *output, size_t outLen)
{
    size_t base64OutLen = 0;
    size_t dataLen = deviceInfo.businessDataLen;
#if (defined(MINE_HARMONY))
    unsigned char* data = (unsigned char*)deviceInfo.businessData;
    int retValue = mbedtls_base64_decode((unsigned char*)output, outLen, &base64OutLen, data, dataLen);
    if (retValue != 0) {
        LOGE("failed to with ret: %d.", retValue);
        return false;
    }
#endif
    if (base64OutLen < DM_SEARCH_BROADCAST_MIN_LEN) {
        LOGE("data length too short with outLen: %u.", base64OutLen);
        return false;
    }
    BroadcastHead *broadcastHead = (BroadcastHead*)output;
    size_t hDataLen = broadcastHead->headDataLen;
    size_t tlvDataLen = broadcastHead->tlvDataLen;
    if (hDataLen >= DISC_MAX_CUST_DATA_LEN || tlvDataLen >= DISC_MAX_CUST_DATA_LEN ||
            (hDataLen + tlvDataLen) != base64OutLen) {
        LOGE("data lenght is not valid with: headDataLen: %u, tlvDataLen: %u, base64OutLen: %d.",
             hDataLen, tlvDataLen, base64OutLen);
        return false;
    }
    return true;
}

Action SoftbusListener::MatchSearchAllDevice(DeviceInfo &deviceInfo, const BroadcastHead &broadcastHead)
{
    if (broadcastHead.trustFilter == 0) {
        return BUSINESS_EXACT_MATCH;
    }
    if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return BUSINESS_EXACT_MATCH;
    } else if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_NOTRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else if (!(deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else {
        return BUSINESS_EXACT_MATCH;
    }
}

void SoftbusListener::GetScopeDevicePolicyInfo(DevicePolicyInfo &devicePolicyInfo)
{
    devicePolicyInfo.numberValid = false;
    devicePolicyInfo.aliasHashValid = false;
    if (GetDeviceAliasHash(devicePolicyInfo.aliasHash)) {
        devicePolicyInfo.aliasHashValid = true;
    }
    if (GetDeviceNumber(devicePolicyInfo.number)) {
        devicePolicyInfo.numberValid = true;
    }
}

Action SoftbusListener::MatchSearchScopeDevice(DeviceInfo &deviceInfo, char *output,
                                               const DevicePolicyInfo &devicePolicyInfo,
                                               const BroadcastHead &broadcastHead)
{
    vector<int> matchItemNum(DM_MAX_SCOPE_TLV_NUM, 0);
    vector<int> matchItemResult(DM_MAX_SCOPE_TLV_NUM, 0);

    if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_NOTRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    }
    if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return  BUSINESS_EXACT_NOT_MATCH;
    }

    size_t tlvLen = broadcastHead.tlvDataLen;
    const size_t ONE_TLV_DATA_LEN = DM_TLV_SCOPE_DATA_OFFSET + DM_HASH_DATA_LEN +
            DM_DEVICE_NUMBER_LEN + DM_DEVICE_NUMBER_LEN;
    for (size_t i = 0; (i + ONE_TLV_DATA_LEN) <= tlvLen; i += ONE_TLV_DATA_LEN) {
        if (output[i] == DEVICE_ALIAS_NUMBER) {
            size_t dataPosition = i + DM_TLV_SCOPE_DATA_OFFSET;
            size_t startNumberPosition = dataPosition + DM_HASH_DATA_LEN;
            int startNumber = atoi(&output[startNumberPosition]);
            if (startNumber == 0) {
                LOGE("the value of start device number is not allowed");
                continue;
            }
            size_t endNumberPosition = startNumberPosition + DM_DEVICE_NUMBER_LEN;
            int endNumber = atoi(&output[endNumberPosition]);
            if (endNumber == 0) {
                LOGE("the value of end device number is not allowed.");
                continue;
            }
            matchItemNum[DEVICE_ALIAS_NUMBER] = 1;
            if (CheckDeviceAliasMatch(devicePolicyInfo, &output[dataPosition]) &&
                CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber)) {
                matchItemResult[DEVICE_ALIAS_NUMBER] = 1;
            }
        } else {
            LOGE("the value of type is not allowed with type: %u.", output[i]);
            continue;
        }
    }
    return GetMatchResult(matchItemNum, matchItemResult);
}

void SoftbusListener::GetVertexDevicePolicyInfo(DevicePolicyInfo &devicePolicyInfo)
{
    devicePolicyInfo.snHashValid = false;
    devicePolicyInfo.typeHashValid = false;
    devicePolicyInfo.udidHashValid = false;
    if (GetDeviceSnHash(devicePolicyInfo.snHash)) {
        devicePolicyInfo.snHashValid = true;
    }
    if (GetDeviceUdidHash(devicePolicyInfo.udidHash)) {
        devicePolicyInfo.udidHashValid = true;
    }
    if (GetDeviceTypeHash(devicePolicyInfo.typeHash)) {
        devicePolicyInfo.typeHashValid = true;
    }
}

Action SoftbusListener::MatchSearchVertexDevice(DeviceInfo &deviceInfo, char *output,
                                                const DevicePolicyInfo &devicePolicyInfo,
                                                const BroadcastHead &broadcastHead)
{
    vector<int> matchItemNum(DM_MAX_VERTEX_TLV_NUM, 0);
    vector<int> matchItemResult(DM_MAX_VERTEX_TLV_NUM, 0);

    if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return  BUSINESS_EXACT_NOT_MATCH;
    }

    size_t tlvLen = broadcastHead.tlvDataLen;
    const size_t ONE_TLV_DATA_LEN = DM_TLV_VERTEX_DATA_OFFSET + DM_HASH_DATA_LEN;
    for (size_t i = 0; (i + ONE_TLV_DATA_LEN) <= tlvLen; i += ONE_TLV_DATA_LEN) {
        size_t dataPosition = 0;
        if (output[i] == DEVICE_TYPE_TYPE) {
            dataPosition = i + DM_TLV_VERTEX_DATA_OFFSET;
            matchItemNum[DEVICE_TYPE_TYPE] = 1;
            if (ChecKDeviceTypeMatch(devicePolicyInfo, &output[dataPosition])) {
                matchItemResult[DEVICE_TYPE_TYPE] = 1;
            }
        } else if (output[i] == DEVICE_SN_TYPE) {
            dataPosition = i + DM_TLV_VERTEX_DATA_OFFSET;
            matchItemNum[DEVICE_SN_TYPE] = 1;
            if (ChecKDeviceSnMatch(devicePolicyInfo, &output[dataPosition])) {
                matchItemResult[DEVICE_SN_TYPE] = 1;
            }
        } else if (output[i] == DEVICE_UDID_TYPE) {
            dataPosition = i + DM_TLV_VERTEX_DATA_OFFSET;
            matchItemNum[DEVICE_UDID_TYPE] = 1;
            if (ChecKDeviceUdidMatch(devicePolicyInfo, &output[dataPosition])) {
                matchItemResult[DEVICE_UDID_TYPE] = 1;
            }
        } else {
            LOGE("the value of type is not allowed with type: %u.", output[i]);
        }
    }
    return GetMatchResult(matchItemNum, matchItemResult);
}

int32_t SoftbusListener::SendReturnwave(DeviceInfo &deviceInfo, const BroadcastHead &broadcastHead, Action matchResult)
{
    size_t outLen = 0;
    unsigned char outData[DISC_MAX_CUST_DATA_LEN] = {0};
    outData[outLen++] = BROADCAST_VERSION;
    outData[outLen++] = sizeof(ReturnwaveHead);
    if (HiChainConnector::IsCredentialExist()) {
        outData[outLen++] = true;
    } else {
        outData[outLen++] = false;
    }
    for (int i = 0; i < DM_HASH_DATA_LEN; i++) {
        outData[outLen++] = broadcastHead.pkgNameHash[i];
    }

    size_t base64OutLen = 0;
    int retValue;
#if (defined(MINE_HARMONY))
    int retValue = mbedtls_base64_encode((unsigned char*)(deviceInfo.businessData), DISC_MAX_CUST_DATA_LEN,
        &base64OutLen, outData, outLen);
    if (retValue != 0) {
        LOGE("failed to get search data base64 encode type data with ret: %d.", retValue);
        return ERR_DM_FAILED;
    }
    deviceInfo.businessData[base64OutLen] = '\0';
#endif
    retValue = SetDiscoveryPolicy(DM_PKG_NAME, &deviceInfo, (int32_t)matchResult);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to set discovery policy with ret: %d.", retValue);
    }
    LOGI("set discovery policy successfully with dataLen: %u.", base64OutLen);
    return DM_OK;
}

bool SoftbusListener::GetDeviceAliasHash(char *output)
{
    char deviceAlias[DM_MAX_DEVICE_ALIAS_LEN + 1] = {0};
    int32_t retValue = GetParameter(DEVICE_ALIAS, "not exist", deviceAlias, DM_MAX_DEVICE_ALIAS_LEN);
    if (retValue < 0 || strcmp((const char*)deviceAlias, "not exist") == 0) {
        LOGE("failed to get device alias from system parameter with ret: %d.", retValue);
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char*)deviceAlias, strlen(deviceAlias), sha256Out) != DM_OK) {
        LOGE("failed to generated device alias sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool SoftbusListener::GetDeviceSnHash(char *output)
{
    const char* deviceSn = GetSerial();
    if (deviceSn == NULL) {
        LOGE("failed to get device sn from system parameter.");
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char*)deviceSn, strlen(deviceSn), sha256Out) != DM_OK) {
        LOGE("failed to generated device sn sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool SoftbusListener::GetDeviceUdidHash(char *output)
{
    char deviceUdid[DM_MAX_DEVICE_UDID_LEN + 1] = {0};
    int32_t retValue = GetDevUdid(deviceUdid, DM_MAX_DEVICE_UDID_LEN);
    if (retValue != 0) {
        LOGE("failed to get local device udid with ret: %d.", retValue);
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char*)deviceUdid, strlen(deviceUdid), sha256Out) != DM_OK) {
        LOGE("failed to generated device udid sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool SoftbusListener::GetDeviceTypeHash(char *output)
{
    const char* deviceType = GetDeviceType();
    if (deviceType == NULL) {
        LOGE("failed to get device type from system parameter.");
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char*)deviceType, strlen(deviceType), sha256Out) != DM_OK) {
        LOGE("failed to generated device type sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool SoftbusListener::GetDeviceNumber(char *output)
{
    char deviceNumber[DM_DEVICE_NUMBER_LEN + 1] = {0};
    int32_t retValue = GetParameter(DEVICE_NUMBER, "not exist", deviceNumber, DM_DEVICE_NUMBER_LEN);
    if (retValue < 0 || strcmp((const char*)deviceAlias, "not exist") == 0) {
        LOGE("failed to get device alias from system parameter with ret: %d.", retValue);
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = deviceNumber[i];
    }
    return true;
}

bool SoftbusListener::CheckDeviceAliasMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.aliasHashValid) {
        LOGE("device alias is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.aliasHash[i]) {
            LOGI("device alias is not match.");
            return false;
        }
    }
    LOGI("device alias is match.");
    return true;
}

bool SoftbusListener::CheckDeviceNumberMatch(const DevicePolicyInfo &devicePolicyInfo,
                                             int32_t startNumber, int32_t endNumber)
{
    if (!devicePolicyInfo.numberValid) {
        LOGE("device number is not valid");
        return false;
    }
    if (startNumber <= DM_INVALID_DEVICE_NUMBER || endNumber <= DM_INVALID_DEVICE_NUMBER) {
        LOGI("device number is match");
        return true;
    }
    int deviceNumber = atoi((const char *)devicePolicyInfo.number);
    if (deviceNumber < startNumber || deviceNumber > endNumber) {
        LOGI("device number is not match.");
        return false;
    }
    LOGI("device number is match.");
    return true;
}
bool SoftbusListener::ChecKDeviceSnMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.snHashValid) {
        LOGE("device sn is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.snHash[i]) {
            LOGI("device sn is not match.");
            return false;
        }
    }
    LOGI("device sn is match.");
    return true;
}
bool SoftbusListener::ChecKDeviceTypeMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.typeHashValid) {
        LOGE("device type is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.typeHash[i]) {
            LOGI("device type is not match.");
            return false;
        }
    }
    LOGI("device type is match.");
    return true;
}

bool SoftbusListener::ChecKDeviceUdidMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.udidHashValid) {
        LOGE("device udid is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.udidHash[i]) {
            LOGI("device udid is not match.");
            return false;
        }
    }
    LOGI("device udid is match.");
    return true;
}

Action SoftbusListener::GetMatchResult(const vector<int> &matchItemNum, const vector<int> &matchItemResult)
{
    int matchItemSum = 0;
    int matchResultSum = 0;
    size_t matchItemNumLen = matchItemNum.size();
    size_t matchItemResultLen = matchItemResult.size();
    size_t minLen = (matchItemNumLen >= matchItemResultLen ? matchItemResultLen : matchItemNumLen);
    for (size_t i = 0; i < minLen; i++) {
        matchResultSum += matchItemResult[i];
        matchItemSum += matchItemNum[i];
    }
    if (matchResultSum == 0) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else if (matchItemSum == matchResultSum) {
        return BUSINESS_EXACT_MATCH;
    } else {
        return BUSINESS_PARTIAL_MATCH;
    }
}
#endif
} // namespace DistributedHardware
} // namespace OHOS