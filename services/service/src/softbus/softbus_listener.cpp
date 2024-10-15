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
#include "dm_crypto.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {

const int32_t SOFTBUS_CHECK_INTERVAL = 100000; // 100ms
const int32_t SOFTBUS_SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t MAX_CACHED_DISCOVERED_DEVICE_SIZE = 100;
const int32_t MAX_SOFTBUS_MSG_LEN = 2000;
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* DEVICE_ONLINE = "deviceOnLine";
constexpr const char* DEVICE_OFFLINE = "deviceOffLine";
constexpr const char* DEVICE_NAME_CHANGE = "deviceNameChange";
constexpr const char* DEVICE_NOT_TRUST = "deviceNotTrust";
constexpr const char* DEVICE_SCREEN_STATUS_CHANGE = "deviceScreenStatusChange";
constexpr const char* CANDIDATE_RESTRICT_STATUS = "candidateRestrictStatus";
#endif
constexpr const char* LIB_RADAR_NAME = "libdevicemanagerradar.z.so";
constexpr static char HEX_ARRAY[] = "0123456789ABCDEF";
constexpr static uint8_t BYTE_MASK = 0x0F;
constexpr static uint16_t ARRAY_DOUBLE_SIZE = 2;
constexpr static uint16_t BIN_HIGH_FOUR_NUM = 4;
constexpr uint32_t SOFTBUS_MAX_RETRY_TIME = 10;

static std::mutex g_deviceMapMutex;
static std::mutex g_lnnCbkMapMutex;
static std::mutex g_radarLoadLock;
static std::mutex g_onlineDeviceNumLock;
static std::mutex g_lockDeviceTrustedChange;
static std::mutex g_lockDeviceOnLine;
static std::mutex g_lockDeviceOffLine;
static std::mutex g_lockDevInfoChange;
static std::mutex g_lockDeviceIdSet;
static std::mutex g_lockDevScreenStatusChange;
static std::mutex g_lockCandidateRestrictStatus;
static std::map<std::string,
    std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>>> discoveredDeviceMap;
static std::map<std::string, std::shared_ptr<ISoftbusDiscoveringCallback>> lnnOpsCbkMap;
static std::set<std::string> deviceIdSet;
bool SoftbusListener::isRadarSoLoad_ = false;
IDmRadarHelper* SoftbusListener::dmRadarHelper_ = nullptr;
void* SoftbusListener::radarHandle_ = nullptr;
std::string SoftbusListener::hostName_ = "";
int32_t g_onlinDeviceNum = 0;

static int OnSessionOpened(int sessionId, int result)
{
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
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE | EVENT_NODE_STATE_INFO_CHANGED |
        EVENT_NODE_STATUS_CHANGED,
    .onNodeOnline = SoftbusListener::OnSoftbusDeviceOnline,
    .onNodeOffline = SoftbusListener::OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = SoftbusListener::OnSoftbusDeviceInfoChanged,
    .onLocalNetworkIdChanged = SoftbusListener::OnLocalDevInfoChange,
    .onNodeDeviceTrustedChange = SoftbusListener::OnDeviceTrustedChange,
    .onNodeStatusChanged = SoftbusListener::OnDeviceScreenStatusChanged,
    .onCandidateRestrict = SoftbusListener::OnCandidateRestrict,
};

static IRefreshCallback softbusRefreshCallback_ = {
    .OnDeviceFound = SoftbusListener::OnSoftbusDeviceFound,
    .OnDiscoverResult = SoftbusListener::OnSoftbusDiscoveryResult,
};

void SoftbusListener::DeviceOnLine(DmDeviceInfo deviceInfo)
{
    std::lock_guard<std::mutex> lock(g_lockDeviceOnLine);
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_STATE_ONLINE, deviceInfo);
}

void SoftbusListener::DeviceOffLine(DmDeviceInfo deviceInfo)
{
    std::lock_guard<std::mutex> lock(g_lockDeviceOffLine);
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_STATE_OFFLINE, deviceInfo);
}

void SoftbusListener::DeviceNameChange(DmDeviceInfo deviceInfo)
{
    std::lock_guard<std::mutex> lock(g_lockDevInfoChange);
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_INFO_CHANGED, deviceInfo);
}

void SoftbusListener::DeviceNotTrust(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(g_lockDeviceTrustedChange);
    DeviceManagerService::GetInstance().HandleDeviceNotTrust(msg);
}

void SoftbusListener::DeviceTrustedChange(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(g_lockDeviceTrustedChange);
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
}

void SoftbusListener::DeviceScreenStatusChange(DmDeviceInfo deviceInfo)
{
    std::lock_guard<std::mutex> lock(g_lockDevScreenStatusChange);
    DeviceManagerService::GetInstance().HandleDeviceScreenStatusChange(deviceInfo);
}

void SoftbusListener::CandidateRestrict(std::string deviceId, uint16_t deviceTypeId, int32_t errcode)
{
    std::lock_guard<std::mutex> lock(g_lockCandidateRestrictStatus);
    DeviceManagerService::GetInstance().HandleCandidateRestrictStatus(deviceId, deviceTypeId, errcode);
}

void SoftbusListener::OnCandidateRestrict(char *deviceId, uint16_t deviceTypeId, int32_t errcode)
{
    LOGI("received candidate restrict status callback from softbus.");
    if (deviceId == nullptr) {
        return;
    }
    std::string deviceIdStr(deviceId);
    #if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        ffrt::submit([=]() { CandidateRestrict(deviceIdStr, deviceTypeId, errcode); });
    #else
        std::thread candidateRestrictStatus([=]() { CandidateRestrict(deviceIdStr, deviceTypeId, errcode); });
        if (pthread_setname_np(candidateRestrictStatus.native_handle(), CANDIDATE_RESTRICT_STATUS) != DM_OK) {
            LOGE("devcandidateRestrictStatus setname failed.");
        }
        candidateRestrictStatus.detach();
    #endif
}

void SoftbusListener::OnDeviceScreenStatusChanged(NodeStatusType type, NodeStatus *status)
{
    LOGI("received device screen status change callback from softbus.");
    if (status == nullptr) {
        LOGE("[SOFTBUS]status is nullptr, type = %{public}d", static_cast<int32_t>(type));
        return;
    }
    LOGI("screenStatusChanged networkId: %{public}s, screenStatus: %{public}d",
        GetAnonyString(status->basicInfo.networkId).c_str(), static_cast<int32_t>(status->reserved[0]));
    if (type != NodeStatusType::TYPE_SCREEN_STATUS) {
        LOGE("type is not matching.");
        return;
    }
    DmDeviceInfo dmDeviceInfo;
    int32_t devScreenStatus = static_cast<int32_t>(status->reserved[0]);
    ConvertScreenStatusToDmDevice(status->basicInfo, devScreenStatus, dmDeviceInfo);
    #if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        ffrt::submit([=]() { DeviceScreenStatusChange(dmDeviceInfo); });
    #else
        std::thread devScreenStatusChange([=]() { DeviceScreenStatusChange(dmDeviceInfo); });
        if (pthread_setname_np(devScreenStatusChange.native_handle(), DEVICE_SCREEN_STATUS_CHANGE) != DM_OK) {
            LOGE("devScreenStatusChange setname failed.");
        }
        devScreenStatusChange.detach();
    #endif
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
    SoftbusCache::GetInstance().SaveDeviceInfo(dmDeviceInfo);
    SoftbusCache::GetInstance().SaveDeviceSecurityLevel(dmDeviceInfo.networkId);
    SoftbusCache::GetInstance().SaveLocalDeviceInfo();
    {
        std::lock_guard<std::mutex> lock(g_onlineDeviceNumLock);
        g_onlinDeviceNum++;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceOnLine(dmDeviceInfo); });
#else
    std::thread deviceOnLine([=]() { DeviceOnLine(dmDeviceInfo); });
    int32_t ret = pthread_setname_np(deviceOnLine.native_handle(), DEVICE_ONLINE);
    if (ret != DM_OK) {
        LOGE("deviceOnLine setname failed.");
    }
    deviceOnLine.detach();
#endif
    {
        std::string peerUdid;
        GetUdidByNetworkId(info->networkId, peerUdid);
        struct RadarInfo radarInfo = {
            .funcName = "OnSoftbusDeviceOnline",
            .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
            .bizState = static_cast<int32_t>(BizState::BIZ_STATE_START),
            .isTrust = static_cast<int32_t>(TrustStatus::IS_TRUST),
            .peerNetId = info->networkId,
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
    SoftbusCache::GetInstance().DeleteDeviceInfo(dmDeviceInfo);
    SoftbusCache::GetInstance().DeleteDeviceSecurityLevel(dmDeviceInfo.networkId);
    {
        std::lock_guard<std::mutex> lock(g_onlineDeviceNumLock);
        g_onlinDeviceNum--;
        if (g_onlinDeviceNum == 0) {
            SoftbusCache::GetInstance().DeleteLocalDeviceInfo();
        }
    }
    LOGI("device offline networkId: %{public}s.", GetAnonyString(dmDeviceInfo.networkId).c_str());
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceOffLine(dmDeviceInfo); });
#else
    std::thread deviceOffLine([=]() { DeviceOffLine(dmDeviceInfo); });
    int32_t ret = pthread_setname_np(deviceOffLine.native_handle(), DEVICE_OFFLINE);
    if (ret != DM_OK) {
        LOGE("deviceOffLine setname failed.");
    }
    deviceOffLine.detach();
#endif
    {
        std::string peerUdid;
        GetUdidByNetworkId(info->networkId, peerUdid);
        struct RadarInfo radarInfo = {
            .funcName = "OnSoftbusDeviceOffline",
            .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
            .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
            .peerNetId = info->networkId,
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
        SoftbusCache::GetInstance().ChangeDeviceInfo(dmDeviceInfo);
    #if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        ffrt::submit([=]() { DeviceNameChange(dmDeviceInfo); });
    #else
        std::thread deviceInfoChange([=]() { DeviceNameChange(dmDeviceInfo); });
        if (pthread_setname_np(deviceInfoChange.native_handle(), DEVICE_NAME_CHANGE) != DM_OK) {
            LOGE("DeviceNameChange setname failed.");
        }
        deviceInfoChange.detach();
    #endif
    }
}

void SoftbusListener::OnLocalDevInfoChange()
{
    LOGI("SoftbusListener::OnLocalDevInfoChange");
    SoftbusCache::GetInstance().UpDataLocalDevInfo();
}

void SoftbusListener::OnDeviceTrustedChange(TrustChangeType type, const char *msg, uint32_t msgLen)
{
    LOGI("OnDeviceTrustedChange.");
    if (msg == nullptr || msgLen > MAX_SOFTBUS_MSG_LEN) {
        LOGE("OnDeviceTrustedChange msg invalied.");
        return;
    }
    std::string softbusMsg = std::string(msg);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (type == TrustChangeType::DEVICE_NOT_TRUSTED) {
        ffrt::submit([=]() { DeviceNotTrust(softbusMsg); });
    } else if (type == TrustChangeType::DEVICE_TRUST_RELATIONSHIP_CHANGE) {
        ffrt::submit([=]() { DeviceTrustedChange(softbusMsg); });
    } else {
        LOGE("Invalied trust change type.");
    }
#else
    if (type == TrustChangeType::DEVICE_NOT_TRUSTED) {
        std::thread deviceNotTrust([=]() { DeviceNotTrust(softbusMsg); });
        int32_t ret = pthread_setname_np(deviceNotTrust.native_handle(), DEVICE_NOT_TRUST);
        if (ret != DM_OK) {
            LOGE("deviceNotTrust setname failed.");
        }
        deviceNotTrust.detach();
    } else if (type == TrustChangeType::DEVICE_TRUST_RELATIONSHIP_CHANGE) {
        std::thread deviceTrustedChange([=]() { DeviceTrustedChange(softbusMsg); });
        int32_t ret = pthread_setname_np(deviceTrustedChange.native_handle(), DEVICE_NOT_TRUST);
        if (ret != DM_OK) {
            LOGE("deviceTrustedChange setname failed.");
        }
        deviceTrustedChange.detach();
    } else {
        LOGE("Invalied trust change type.");
    }
#endif
}

void SoftbusListener::OnSoftbusDeviceFound(const DeviceInfo *device)
{
    if (device == nullptr) {
        LOGE("[SOFTBUS]device is null.");
        return;
    }
    DmDeviceInfo dmDevInfo;
    ConvertDeviceInfoToDmDevice(*device, dmDevInfo);
    {
        std::lock_guard<std::mutex> lock(g_lockDeviceIdSet);
        if (deviceIdSet.find(std::string(dmDevInfo.deviceId)) == deviceIdSet.end()) {
            deviceIdSet.insert(std::string(dmDevInfo.deviceId));
            struct RadarInfo info = {
                .funcName = "OnSoftbusDeviceFound",
                .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
                .peerNetId = "",
                .peerUdid = std::string(dmDevInfo.deviceId),
            };
            if (IsDmRadarHelperReady() && GetDmRadarHelperObj() != nullptr) {
                if (!GetDmRadarHelperObj()->ReportDiscoverResCallback(info)) {
                    LOGE("ReportDiscoverResCallback failed");
                }
            }
        }
    }
    LOGI("DevId=%{public}s, devName=%{public}s, devType=%{public}d, range=%{public}d,"
        "isOnline=%{public}d", GetAnonyString(dmDevInfo.deviceId).c_str(),
        GetAnonyString(dmDevInfo.deviceName).c_str(), dmDevInfo.deviceTypeId, dmDevInfo.range, device->isOnline);

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
#endif
    InitSoftbusListener();
    ClearDiscoveredDevice();
}

SoftbusListener::~SoftbusListener()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    RemoveSessionServer(DM_PKG_NAME, DM_SESSION_NAME);
    RemoveSessionServer(DM_PKG_NAME, DM_PIN_HOLDER_SESSION_NAME);
#endif
    LOGD("SoftbusListener destructor.");
}

int32_t SoftbusListener::InitSoftbusListener()
{
    int32_t ret;
    uint32_t retryTimes = 0;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    do {
        ret = RegNodeDeviceStateCb(DM_PKG_NAME, &softbusNodeStateCb_);
        if (ret != DM_OK) {
            ++retryTimes;
            LOGE("[SOFTBUS]RegNodeDeviceStateCb failed with ret: %{public}d, retryTimes: %{public}u.", ret, retryTimes);
            usleep(SOFTBUS_CHECK_INTERVAL);
        }
    } while (ret != DM_OK && retryTimes < SOFTBUS_MAX_RETRY_TIME);
#endif
    return InitSoftPublishLNN();
}

int32_t SoftbusListener::InitSoftPublishLNN()
{
    int32_t ret = DM_OK;
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    PublishInfo publishInfo;
    publishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    publishInfo.mode = DiscoverMode::DISCOVER_MODE_PASSIVE;
    publishInfo.medium = ExchangeMedium::AUTO;
    publishInfo.freq = ExchangeFreq::LOW;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.ranging = false;
    LOGI("InitSoftPublishLNN begin, publishId: %{public}d, mode: 0x%{public}x, medium: %{public}d, capability:"
        "%{public}s, ranging: %{public}d, freq: %{public}d.", publishInfo.publishId, publishInfo.mode,
        publishInfo.medium, publishInfo.capability, publishInfo.ranging, publishInfo.freq);

    ret = PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret == DM_OK) {
        LOGI("[SOFTBUS]PublishLNN successed, ret: %{public}d", ret);
    }
#endif
    return ret;
}

int32_t SoftbusListener::RefreshSoftbusLNN(const char *pkgName, const DmSubscribeInfo &dmSubInfo,
    const std::string &customData)
{
    LOGI("RefreshSoftbusLNN begin, subscribeId: %{public}d.", dmSubInfo.subscribeId);
    SubscribeInfo subscribeInfo;
    if (memset_s(&subscribeInfo, sizeof(SubscribeInfo), 0, sizeof(SubscribeInfo)) != DM_OK) {
        LOGE("RefreshSoftbusLNN memset_s failed.");
        return ERR_DM_FAILED;
    }

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
        return ret;
    }
    return DM_OK;
}

int32_t SoftbusListener::StopRefreshSoftbusLNN(uint16_t subscribeId)
{
    LOGI("StopRefreshSoftbusLNN begin, subscribeId: %{public}d.", (int32_t)subscribeId);
    {
        std::lock_guard<std::mutex> lock(g_lockDeviceIdSet);
        deviceIdSet.clear();
    }
    int32_t ret = ::StopRefreshLNN(DM_PKG_NAME, subscribeId);
    struct RadarInfo info = {
        .funcName = "StopRefreshSoftbusLNN",
        .hostName = SOFTBUSNAME,
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
        return ret;
    }
    return DM_OK;
}

int32_t SoftbusListener::PublishSoftbusLNN(const DmPublishInfo &dmPubInfo, const std::string &capability,
    const std::string &customData)
{
    LOGI("Begin, publishId: %{public}d.", dmPubInfo.publishId);
    PublishInfo publishInfo;
    publishInfo.publishId = dmPubInfo.publishId;
    publishInfo.mode = static_cast<DiscoverMode>(dmPubInfo.mode);
    publishInfo.medium = (capability == DM_CAPABILITY_APPROACH) ? ExchangeMedium::BLE : ExchangeMedium::AUTO;
    publishInfo.freq = static_cast<ExchangeFreq>(dmPubInfo.freq);
    publishInfo.capability = capability.c_str();
    publishInfo.capabilityData = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(customData.c_str()));
    publishInfo.dataLen = customData.length();
    publishInfo.ranging = dmPubInfo.ranging;

    LOGI("Begin, mode: 0x%{public}x, medium: %{public}d, capability:"
        "%{public}s, ranging: %{public}d, freq: %{public}d.", publishInfo.mode,
        publishInfo.medium, publishInfo.capability, publishInfo.ranging, publishInfo.freq);

    int32_t ret = ::PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]PublishLNN failed, ret: %{public}d.", ret);
        return ret;
    }
    return DM_OK;
}

int32_t SoftbusListener::StopPublishSoftbusLNN(int32_t publishId)
{
    LOGI("Begin, publishId: %{public}d.", publishId);
    int32_t ret = ::StopPublishLNN(DM_PKG_NAME, publishId);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]StopPublishLNN failed, ret: %{public}d.", ret);
        return ret;
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
    int32_t ret = SoftbusCache::GetInstance().GetDeviceInfoFromCache(deviceInfoList);
    size_t deviceCount = deviceInfoList.size();
    LOGI("Success from cache deviceInfoList size is %{public}zu.", deviceCount);
    return ret;
}

int32_t SoftbusListener::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    return SoftbusCache::GetInstance().GetDevInfoByNetworkId(networkId, info);
}

int32_t SoftbusListener::GetLocalDeviceInfo(DmDeviceInfo &deviceInfo)
{
    return SoftbusCache::GetInstance().GetLocalDeviceInfo(deviceInfo);
}

int32_t SoftbusListener::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    return SoftbusCache::GetInstance().GetUdidFromCache(networkId, udid);
}

int32_t SoftbusListener::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    return SoftbusCache::GetInstance().GetUuidFromCache(networkId, uuid);
}

int32_t SoftbusListener::GetNetworkIdByUdid(const std::string &udid, std::string &networkId)
{
    return SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, networkId);
}

int32_t SoftbusListener::ShiftLNNGear(bool isWakeUp, const std::string &callerId)
{
    if (callerId.empty()) {
        LOGE("Invalid parameter, callerId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGD("Begin for callerId = %{public}s", GetAnonyString(callerId).c_str());
    GearMode mode = {
        .cycle = HIGH_FREQ_CYCLE,
        .duration = DEFAULT_DURATION,
        .wakeupFlag = isWakeUp,
    };
    int32_t ret = ::ShiftLNNGear(DM_PKG_NAME, callerId.c_str(), nullptr, &mode);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]ShiftLNNGear error, failed ret: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t SoftbusListener::ConvertScreenStatusToDmDevice(const NodeBasicInfo &nodeInfo, const int32_t devScreenStatus,
    DmDeviceInfo &devInfo)
{
    if (memset_s(&devInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo)) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice memset failed.");
        return ERR_DM_FAILED;
    }
    if (memcpy_s(devInfo.networkId, sizeof(devInfo.networkId), nodeInfo.networkId,
        std::min(sizeof(devInfo.networkId), sizeof(nodeInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
        return ERR_DM_FAILED;
    }

    if (memcpy_s(devInfo.deviceName, sizeof(devInfo.deviceName), nodeInfo.deviceName,
        std::min(sizeof(devInfo.deviceName), sizeof(nodeInfo.deviceName))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed.");
        return ERR_DM_FAILED;
    }
    devInfo.deviceTypeId = nodeInfo.deviceTypeId;
    nlohmann::json extraJson;
    extraJson[PARAM_KEY_OS_TYPE] = nodeInfo.osType;
    extraJson[PARAM_KEY_OS_VERSION] = ConvertCharArray2String(nodeInfo.osVersion, OS_VERSION_BUF_LEN);
    extraJson[DEVICE_SCREEN_STATUS] = devScreenStatus;
    devInfo.extraData = to_string(extraJson);
    return DM_OK;
}

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceInfo &devInfo)
{
    if (memset_s(&devInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo)) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice memset_s failed.");
        return ERR_DM_FAILED;
    }
    
    if (memcpy_s(devInfo.networkId, sizeof(devInfo.networkId), nodeInfo.networkId,
        std::min(sizeof(devInfo.networkId), sizeof(nodeInfo.networkId))) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
        return ERR_DM_FAILED;
    }

    if (memcpy_s(devInfo.deviceName, sizeof(devInfo.deviceName), nodeInfo.deviceName,
        std::min(sizeof(devInfo.deviceName), sizeof(nodeInfo.deviceName))) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed.");
        return ERR_DM_FAILED;
    }
    devInfo.deviceTypeId = nodeInfo.deviceTypeId;
    nlohmann::json extraJson;
    extraJson[PARAM_KEY_OS_TYPE] = nodeInfo.osType;
    extraJson[PARAM_KEY_OS_VERSION] = ConvertCharArray2String(nodeInfo.osVersion, OS_VERSION_BUF_LEN);
    devInfo.extraData = to_string(extraJson);
    return DM_OK;
}

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceBasicInfo &devInfo)
{
    if (memset_s(&devInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo)) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice memset_s failed.");
        return ERR_DM_FAILED;
    }

    if (memcpy_s(devInfo.networkId, sizeof(devInfo.networkId), nodeInfo.networkId,
        std::min(sizeof(devInfo.networkId), sizeof(nodeInfo.networkId))) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
        return ERR_DM_FAILED;
    }

    if (memcpy_s(devInfo.deviceName, sizeof(devInfo.deviceName), nodeInfo.deviceName,
        std::min(sizeof(devInfo.deviceName), sizeof(nodeInfo.deviceName))) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed.");
        return ERR_DM_FAILED;
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

int32_t SoftbusListener::FillDeviceInfo(const DeviceInfo &device, DmDeviceInfo &dmDevice)
{
    if (memset_s(&dmDevice, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo)) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice memset_s failed.");
        return ERR_DM_FAILED;
    }

    if (memcpy_s(dmDevice.deviceId, sizeof(dmDevice.deviceId), device.devId,
        std::min(sizeof(dmDevice.deviceId), sizeof(device.devId))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice: copy device id failed.");
        return ERR_DM_FAILED;
    }

    if (memcpy_s(dmDevice.deviceName, sizeof(dmDevice.deviceName), device.devName,
        std::min(sizeof(dmDevice.deviceName), sizeof(device.devName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice: copy device name failed.");
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

void SoftbusListener::ConvertDeviceInfoToDmDevice(const DeviceInfo &device, DmDeviceInfo &dmDevice)
{
    if (FillDeviceInfo(device, dmDevice) != DM_OK) {
        LOGE("FillDeviceInfo failed.");
        return;
    }

    dmDevice.deviceTypeId = device.devType;
    dmDevice.range = device.range;

    nlohmann::json jsonObj;
    std::string customData = ConvertCharArray2String(device.custData, DISC_MAX_CUST_DATA_LEN);
    jsonObj[PARAM_KEY_CUSTOM_DATA] = customData;

    const ConnectionAddr *addrInfo = &(device.addr)[0];
    if (addrInfo == nullptr) {
        LOGE("ConvertDeviceInfoToDmDevice: addrInfo is nullptr.");
        dmDevice.extraData = jsonObj.dump();
        return;
    }
    jsonObj[PARAM_KEY_DISC_CAPABILITY] = device.capabilityBitmap[0];
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
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_NETWORK_TYPE,
        reinterpret_cast<uint8_t *>(&tempNetworkType), LNN_COMMON_LEN);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
        return ret;
    }
    networkType = tempNetworkType;
    LOGI("GetNetworkTypeByNetworkId networkType %{public}d.", tempNetworkType);
    return DM_OK;
}

int32_t SoftbusListener::GetDeviceSecurityLevel(const char *networkId, int32_t &securityLevel)
{
    return SoftbusCache::GetInstance().GetSecurityDeviceLevel(networkId, securityLevel);
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
    std::lock_guard<std::mutex> lock(g_radarLoadLock);
    if (isRadarSoLoad_ && (dmRadarHelper_ != nullptr) && (radarHandle_ != nullptr)) {
        LOGD("IsDmRadarHelperReady alReady.");
        return true;
    }
    radarHandle_ = dlopen(LIB_RADAR_NAME, RTLD_NOW);
    if (radarHandle_ == nullptr) {
        LOGE("load libdevicemanagerradar so failed.");
        return false;
    }
    dlerror();
    auto func = (CreateDmRadarFuncPtr)dlsym(radarHandle_, "CreateDmRadarInstance");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(radarHandle_);
        LOGE("Create object function is not exist.");
        return false;
    }
    LOGI("IsDmRadarHelperReady ready success.");
    isRadarSoLoad_ = true;
    dmRadarHelper_ = func();
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

void SoftbusListener::SendAclChangedBroadcast(const std::string &msg)
{
    LOGI("SendAclChangedBroadcast");
    if (SyncTrustedRelationShip(DM_PKG_NAME, msg.c_str(), msg.length()) != DM_OK) {
        LOGE("SyncTrustedRelationShip failed.");
    }
}

IRefreshCallback &SoftbusListener::GetSoftbusRefreshCb()
{
    return softbusRefreshCallback_;
}

int32_t SoftbusListener::GetDeviceScreenStatus(const char *networkId, int32_t &screenStatus)
{
    int32_t devScreenStatus = -1;
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_DEVICE_SCREEN_STATUS,
        reinterpret_cast<uint8_t *>(&devScreenStatus), LNN_COMMON_LEN);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo screenStatus failed.");
        return ret;
    }
    screenStatus = devScreenStatus;
    LOGI("GetDeviceScreenStatus screenStatus: %{public}d.", devScreenStatus);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS