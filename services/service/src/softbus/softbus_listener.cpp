/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dm_crypto.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_transport_msg.h"
#include "ffrt.h"
#include "multiple_user_connector.h"
#endif
#include "ipc_skeleton.h"
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
constexpr const char* CREDENTIAL_AUTH_STATUS = "credentialAuthStatus";
#endif
constexpr const char* LIB_RADAR_NAME = "libdevicemanagerradar.z.so";
constexpr static char HEX_ARRAY[] = "0123456789ABCDEF";
constexpr static uint8_t BYTE_MASK = 0x0F;
constexpr static uint16_t ARRAY_DOUBLE_SIZE = 2;
constexpr static uint16_t BIN_HIGH_FOUR_NUM = 4;
constexpr uint32_t SOFTBUS_MAX_RETRY_TIME = 10;

constexpr const char* CUSTOM_DATA_ACTIONID = "actionId";
constexpr const char* CUSTOM_DATA_NETWORKID = "networkId";
constexpr const char* CUSTOM_DATA_DISPLAY_NAME = "displayName";

static std::mutex g_deviceMapMutex;
static std::mutex g_lnnCbkMapMutex;
static std::mutex g_radarLoadLock;
static std::mutex g_onlineDeviceNumLock;
static std::mutex g_lockDeviceTrustedChange;
static std::mutex g_lockUserIdCheckSumChange;
static std::mutex g_lockDeviceOnLine;
static std::mutex g_lockDeviceOffLine;
static std::mutex g_lockDevInfoChange;
static std::mutex g_lockDeviceIdSet;
static std::mutex g_lockDevScreenStatusChange;
static std::mutex g_credentialAuthStatus;
static std::map<std::string,
    std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>>> discoveredDeviceMap;
static std::map<std::string, std::shared_ptr<ISoftbusDiscoveringCallback>> lnnOpsCbkMap;
static std::map<std::string, int32_t> discoveredDeviceActionIdMap;
static std::set<std::string> deviceIdSet;
bool SoftbusListener::isRadarSoLoad_ = false;
IDmRadarHelper* SoftbusListener::dmRadarHelper_ = nullptr;
void* SoftbusListener::radarHandle_ = nullptr;
std::string SoftbusListener::hostName_ = "";
int32_t g_onlineDeviceNum = 0;

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
        EVENT_NODE_STATUS_CHANGED | EVENT_NODE_HICHAIN_PROOF_EXCEPTION,
    .onNodeOnline = SoftbusListener::OnSoftbusDeviceOnline,
    .onNodeOffline = SoftbusListener::OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = SoftbusListener::OnSoftbusDeviceInfoChanged,
    .onLocalNetworkIdChanged = SoftbusListener::OnLocalDevInfoChange,
    .onNodeDeviceTrustedChange = SoftbusListener::OnDeviceTrustedChange,
    .onNodeStatusChanged = SoftbusListener::OnDeviceScreenStatusChanged,
    .onHichainProofException = SoftbusListener::OnCredentialAuthStatus,
};

static IRefreshCallback softbusRefreshCallback_ = {
    .OnDeviceFound = SoftbusListener::OnSoftbusDeviceFound,
    .OnDiscoverResult = SoftbusListener::OnSoftbusDiscoveryResult,
};

void SoftbusListener::DeviceOnLine(DmDeviceInfo deviceInfo)
{
    std::lock_guard<std::mutex> lock(g_lockDeviceOnLine);
    LOGI("received device online deviceId: %{public}s, networkId: %{public}s.",
        GetAnonyString(deviceInfo.deviceId).c_str(), GetAnonyString(deviceInfo.networkId).c_str());
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(g_lockDeviceTrustedChange);
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
#else
    (void)msg;
#endif
}

void SoftbusListener::DeviceUserIdCheckSumChange(const std::string &msg)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<std::mutex> lock(g_lockUserIdCheckSumChange);
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);
#else
    (void)msg;
#endif
}

void SoftbusListener::DeviceScreenStatusChange(DmDeviceInfo deviceInfo)
{
    std::lock_guard<std::mutex> lock(g_lockDevScreenStatusChange);
    DeviceManagerService::GetInstance().HandleDeviceScreenStatusChange(deviceInfo);
}

void SoftbusListener::CredentialAuthStatusProcess(std::string deviceList, uint16_t deviceTypeId, int32_t errcode)
{
    std::lock_guard<std::mutex> lock(g_credentialAuthStatus);
    DeviceManagerService::GetInstance().HandleCredentialAuthStatus(deviceList, deviceTypeId, errcode);
}

int32_t SoftbusListener::OnSyncLocalAclList(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string remoteAclList)
{
    return DeviceManagerService::GetInstance().SyncLocalAclListProcess(localDevUserInfo, remoteDevUserInfo,
        remoteAclList);
}

int32_t SoftbusListener::OnGetAclListHash(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclList)
{
    return DeviceManagerService::GetInstance().GetAclListHash(localDevUserInfo, remoteDevUserInfo, aclList);
}

void SoftbusListener::OnCredentialAuthStatus(const char *deviceList, uint32_t deviceListLen,
                                             uint16_t deviceTypeId, int32_t errcode)
{
    LOGI("received credential auth status callback from softbus.");
    if (deviceListLen > MAX_SOFTBUS_MSG_LEN) {
        LOGE("[SOFTBUS]received invaild deviceList value.");
        return;
    }
    std::string deviceListStr;
    if (deviceList != nullptr) {
        deviceListStr = std::string(deviceList, deviceListLen);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { CredentialAuthStatusProcess(deviceListStr, deviceTypeId, errcode); });
#else
    std::thread credentialAuthStatus([=]() { CredentialAuthStatusProcess(deviceListStr, deviceTypeId, errcode); });
    if (pthread_setname_np(credentialAuthStatus.native_handle(), CREDENTIAL_AUTH_STATUS) != DM_OK) {
        LOGE("credentialAuthStatus setname failed.");
    }
    credentialAuthStatus.detach();
#endif
}

void SoftbusListener::OnDeviceScreenStatusChanged(NodeStatusType type, NodeStatus *status)
{
    if (status == nullptr) {
        LOGE("[SOFTBUS]status is nullptr, type = %{public}d", static_cast<int32_t>(type));
        return;
    }
    LOGI("networkId: %{public}s, screenStatus: %{public}d",
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
    UpdateDeviceName(info);
    {
        std::lock_guard<std::mutex> lock(g_onlineDeviceNumLock);
        g_onlineDeviceNum++;
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
        g_onlineDeviceNum--;
        if (g_onlineDeviceNum == 0) {
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

void SoftbusListener::UpdateDeviceName(NodeBasicInfo *info)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (info == nullptr) {
        LOGE("NodeBasicInfo is nullptr, not update device name");
        return;
    }
    std::string udid = "";
    if (GetUdidByNetworkId(info->networkId, udid) != DM_OK) {
        LOGE("GetUdidByNetworkId failed, not update device name");
        return;
    }
    LOGI("info->deviceName: %{public}s.", GetAnonyString(info->deviceName).c_str());
    DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, info->deviceName);
#endif
}

void SoftbusListener::OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    LOGI("received device info change from softbus.");
    if (info == nullptr) {
        LOGE("NodeBasicInfo is nullptr.");
        return;
    }
    if (type == NodeBasicInfoType::TYPE_DEVICE_NAME || type == NodeBasicInfoType::TYPE_NETWORK_INFO) {
        DmDeviceInfo dmDeviceInfo;
        int32_t networkType = -1;
        if (type == NodeBasicInfoType::TYPE_NETWORK_INFO) {
            if (GetNodeKeyInfo(DM_PKG_NAME, info->networkId, NodeDeviceInfoKey::NODE_KEY_NETWORK_TYPE,
                reinterpret_cast<uint8_t *>(&networkType), LNN_COMMON_LEN) != DM_OK) {
                LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
                return;
            }
            LOGI("NetworkType %{public}d.", networkType);
        }
        if (type == NodeBasicInfoType::TYPE_DEVICE_NAME) {
            UpdateDeviceName(info);
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
    LOGI("start");
    SoftbusCache::GetInstance().UpDataLocalDevInfo();
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, not update deviceName ret: %{public}d.", ret);
        return;
    }
    std::string udid = "";
    ret = GetUdidByNetworkId(nodeBasicInfo.networkId, udid);
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId failed, not update deviceName ret: %{public}d.", ret);
        return;
    }
    DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, nodeBasicInfo.deviceName);
#endif
}

void SoftbusListener::OnDeviceTrustedChange(TrustChangeType type, const char *msg, uint32_t msgLen)
{
    LOGI("start.");
    if (msg == nullptr || msgLen > MAX_SOFTBUS_MSG_LEN || strlen(msg) != msgLen) {
        LOGE("OnDeviceTrustedChange msg invalied.");
        return;
    }
    std::string softbusMsg = std::string(msg, msgLen);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (type == TrustChangeType::DEVICE_NOT_TRUSTED) {
        ffrt::submit([=]() { DeviceNotTrust(softbusMsg); });
    } else if (type == TrustChangeType::DEVICE_TRUST_RELATIONSHIP_CHANGE) {
        ffrt::submit([=]() { DeviceTrustedChange(softbusMsg); });
    } else if (type == TrustChangeType::DEVICE_FOREGROUND_USERID_CHANGE) {
        ffrt::submit([=]() { DeviceUserIdCheckSumChange(softbusMsg); });
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
    } else if (type == TrustChangeType::DEVICE_FOREGROUND_USERID_CHANGE) {
        std::thread deviceUserIdCheckSumChange([=]() { DeviceUserIdCheckSumChange(softbusMsg); });
        int32_t ret = pthread_setname_np(deviceUserIdCheckSumChange.native_handle(), DEVICE_NOT_TRUST);
        if (ret != DM_OK) {
            LOGE("deviceUserIdCheckSumChange setname failed.");
        }
        deviceUserIdCheckSumChange.detach();
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
    LOGD("DevId=%{public}s, devName=%{public}s, devType=%{public}d, range=%{public}d,"
        "isOnline=%{public}d, capability=%{public}u", GetAnonyString(dmDevInfo.deviceId).c_str(),
        GetAnonyString(dmDevInfo.deviceName).c_str(), dmDevInfo.deviceTypeId, dmDevInfo.range,
        device->isOnline, device->capabilityBitmap[0]);
    int32_t actionId = 0;
    int32_t ret = GetAttrFromExtraData(dmDevInfo, actionId);
    if (ret != DM_OK) {
        LOGE("GetAttrFromExtraData failed");
        return;
    }
    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    discoveredDeviceActionIdMap[dmDevInfo.deviceId] = actionId;
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
    LOGD("publishId: %{public}d, result: %{public}d.", publishId, result);
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
    LOGI("begin, publishId: %{public}d, mode: 0x%{public}x, medium: %{public}d, capability:"
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
    LOGI("begin, subscribeId: %{public}d.", dmSubInfo.subscribeId);
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
    LOGI("subscribeId: %{public}d, mode: 0x%{public}x, medium: %{public}d, capability:"
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
    LOGI("begin, subscribeId: %{public}d.", (int32_t)subscribeId);
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
    if (capability == DM_CAPABILITY_APPROACH) {
        publishInfo.medium = ExchangeMedium::BLE;
    } else {
        publishInfo.medium = static_cast<ExchangeMedium>(dmPubInfo.medium);
    }
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
    LOGI("size is %{public}zu.", deviceCount);
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

int32_t SoftbusListener::GetDeviceNameByUdid(const std::string &udid, std::string &deviceName)
{
    return SoftbusCache::GetInstance().GetDeviceNameFromCache(udid, deviceName);
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
    JsonObject extraJson;
    extraJson[PARAM_KEY_OS_TYPE] = nodeInfo.osType;
    extraJson[PARAM_KEY_OS_VERSION] = ConvertCharArray2String(nodeInfo.osVersion, OS_VERSION_BUF_LEN);
    extraJson[DEVICE_SCREEN_STATUS] = devScreenStatus;
    devInfo.extraData = ToString(extraJson);
    return DM_OK;
}

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceInfo &devInfo)
{
    LOGI("Begin, osType : %{public}d", nodeInfo.osType);
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
    JsonObject extraJson;
    extraJson[PARAM_KEY_OS_TYPE] = nodeInfo.osType;
    extraJson[PARAM_KEY_OS_VERSION] = ConvertCharArray2String(nodeInfo.osVersion, OS_VERSION_BUF_LEN);
    devInfo.extraData = ToString(extraJson);
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

    JsonObject jsonObj;
    std::string customData = ConvertCharArray2String(device.custData, DISC_MAX_CUST_DATA_LEN);
    jsonObj[PARAM_KEY_CUSTOM_DATA] = customData;

    const ConnectionAddr *addrInfo = &(device.addr)[0];
    if (addrInfo == nullptr) {
        LOGE("ConvertDeviceInfoToDmDevice: addrInfo is nullptr.");
        dmDevice.extraData = SafetyDump(jsonObj);
        return;
    }
    jsonObj[PARAM_KEY_DISC_CAPABILITY] = device.capabilityBitmap[0];
    ParseConnAddrInfo(addrInfo, jsonObj);
    dmDevice.extraData = SafetyDump(jsonObj);
}

void SoftbusListener::ParseConnAddrInfo(const ConnectionAddr *addrInfo, JsonObject &jsonObj)
{
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
    } else if (addrInfo->type == CONNECTION_ADDR_USB) {
        std::string usbIp((addrInfo->info).ip.ip);
        jsonObj[PARAM_KEY_USB_IP] = usbIp;
        jsonObj[PARAM_KEY_USB_PORT] = (addrInfo->info).ip.port;
        jsonObj[PARAM_KEY_CONN_ADDR_TYPE] = CONN_ADDR_TYPE_USB;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_NCM) {
        std::string ncmIp((addrInfo->info).ip.ip);
        jsonObj[PARAM_KEY_NCM_IP] = ncmIp;
        jsonObj[PARAM_KEY_NCM_PORT] = (addrInfo->info).ip.port;
        jsonObj[PARAM_KEY_CONN_ADDR_TYPE] = CONN_ADDR_TYPE_NCM;
    } else {
        LOGI("Unknown connection address type: %{public}d.", addrInfo->type);
    }
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
    LOGI("networkType %{public}d.", tempNetworkType);
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
    radarHandle_ = dlopen(LIB_RADAR_NAME, RTLD_NOW | RTLD_NOLOAD);
    if (radarHandle_ == nullptr) {
        radarHandle_ = dlopen(LIB_RADAR_NAME, RTLD_NOW);
    }
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
    LOGI("success.");
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
            addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_WLAN ||
            addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_NCM) {
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
}

std::string SoftbusListener::GetHostPkgName()
{
    return hostName_;
}

void SoftbusListener::SendAclChangedBroadcast(const std::string &msg)
{
    LOGI("start");
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
    LOGI("screenStatus: %{public}d.", devScreenStatus);
    return DM_OK;
}

int32_t SoftbusListener::SetForegroundUserIdsToDSoftBus(const std::string &remoteUdid,
    const std::vector<uint32_t> &userIds)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    NotifyUserIds notifyUserIds(remoteUdid, userIds);
    std::string msg = notifyUserIds.ToString();
    return DM_OK;
#else
    (void)remoteUdid;
    (void)userIds;
#endif
    return DM_OK;
}

void SoftbusListener::DeleteCacheDeviceInfo()
{
    LOGI("start.");
    std::vector<DmDeviceInfo> onlineDevInfoVec;
    SoftbusCache::GetInstance().GetDeviceInfoFromCache(onlineDevInfoVec);
    if (onlineDevInfoVec.empty()) {
        LOGE("onlineDevInfoVec is empty");
        return;
    }
    SoftbusCache::GetInstance().DeleteDeviceInfo();
    for (auto it : onlineDevInfoVec) {
        LOGI("networkId: %{public}s", GetAnonyString(it.networkId).c_str());
        DeviceOffLine(it);
    }
}

int32_t SoftbusListener::SetLocalDisplayName(const std::string &displayName)
{
    LOGI("start");
    uint32_t len = static_cast<uint32_t>(displayName.size());
    int32_t ret = ::SetDisplayName(DM_PKG_NAME, displayName.c_str(), len);
    if (ret != DM_OK) {
        LOGE("SoftbusListener SetLocalDisplayName failed!");
    }
    return DM_OK;
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void SoftbusListener::ConvertAclToDeviceInfo(DistributedDeviceProfile::AccessControlProfile &profile,
    DmDeviceInfo &deviceInfo)
{
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(profile.GetTrustDeviceId(), reinterpret_cast<uint8_t *>(udidHash)) != DM_OK) {
        LOGE("get udidhash by udid: %{public}s failed.", GetAnonyString(profile.GetTrustDeviceId()).c_str());
        return;
    }

    if (memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), udidHash,
                 std::min(sizeof(deviceInfo.deviceId), sizeof(udidHash))) != DM_OK) {
        LOGE("GetAllTrustedDeviceList copy deviceId failed.");
        return;
    }

    std::string networkId = "";
    if (GetNetworkIdByUdid(profile.GetTrustDeviceId(), networkId) == DM_OK) {
        if (memcpy_s(deviceInfo.networkId, sizeof(deviceInfo.networkId), networkId.c_str(),
                     std::min(sizeof(deviceInfo.networkId), networkId.size())) != DM_OK) {
            LOGE("GetAllTrustedDeviceList copy networkId data failed.");
            return;
        }
    }

    std::string deviceName = "";
    if (GetDeviceNameByUdid(profile.GetTrustDeviceId(), deviceName) == DM_OK) {
        if (memcpy_s(deviceInfo.deviceName, sizeof(deviceInfo.deviceName), deviceName.c_str(),
                     std::min(sizeof(deviceInfo.deviceName), deviceName.size())) != DM_OK) {
            LOGE("GetAllTrustedDeviceList copy deviceName data failed.");
            return;
        }
    } else {
        if (profile.GetTrustDeviceId() == profile.GetAccessee().GetAccesseeDeviceId()) {
            deviceName = profile.GetAccessee().GetAccesseeDeviceName();
        } else if (profile.GetTrustDeviceId() == profile.GetAccesser().GetAccesserDeviceId()) {
            deviceName = profile.GetAccesser().GetAccesserDeviceName();
        }
        if (memcpy_s(deviceInfo.deviceName, sizeof(deviceInfo.deviceName), deviceName.c_str(),
                     std::min(sizeof(deviceInfo.deviceName), deviceName.size())) != DM_OK) {
            LOGE("GetAllTrustedDeviceList copy deviceName data from dp failed.");
        }
    }
}
#endif

int32_t SoftbusListener::GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    (void)extra;
    int32_t currentUserId = MultipleUserConnector::GetCurrentAccountUserID();
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> allProfile =
        DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    for (DistributedDeviceProfile::AccessControlProfile profile : allProfile) {
        if (profile.GetBindType() == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP) {
            continue;
        }
        DistributedDeviceProfile::Accesser acer = profile.GetAccesser();
        if (pkgName == acer.GetAccesserBundleName() && currentUserId == acer.GetAccesserUserId() &&
            localUdid == acer.GetAccesserDeviceId()) {
            DmDeviceInfo deviceinfo;
            ConvertAclToDeviceInfo(profile, deviceinfo);
            deviceList.push_back(deviceinfo);
            continue;
        }
        DistributedDeviceProfile::Accessee acee = profile.GetAccessee();
        if (pkgName == acee.GetAccesseeBundleName() && currentUserId == acee.GetAccesseeUserId() &&
            localUdid == acee.GetAccesseeDeviceId()) {
            DmDeviceInfo deviceinfo;
            ConvertAclToDeviceInfo(profile, deviceinfo);
            deviceList.push_back(deviceinfo);
            continue;
        }
    }
#endif
    return DM_OK;
}

int32_t SoftbusListener::GetUdidFromDp(const std::string &udidHash, std::string &udid)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::vector<DistributedDeviceProfile::AccessControlProfile> allProfile =
        DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    for (DistributedDeviceProfile::AccessControlProfile profile : allProfile) {
        if (profile.GetBindType() == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP) {
            continue;
        }
        char udidHashTemp[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetUdidHash(profile.GetTrustDeviceId(), reinterpret_cast<uint8_t *>(udidHashTemp)) != DM_OK) {
            LOGE("get udidHash by udid: %{public}s failed.", GetAnonyString(profile.GetTrustDeviceId()).c_str());
            continue;
        }
        if (udidHash == std::string(udidHashTemp)) {
            udid = profile.GetTrustDeviceId();
            return DM_OK;
        }
    }
#endif
    return ERR_DM_FAILED;
}

int32_t SoftbusListener::GetAttrFromExtraData(DmDeviceInfo &dmDevInfo, int32_t &actionId)
{
    cJSON *extraDataJsonObj = cJSON_Parse(dmDevInfo.extraData.c_str());
    if (extraDataJsonObj == NULL) {
        return DM_OK;
    }
    cJSON *customData = cJSON_GetObjectItem(extraDataJsonObj, PARAM_KEY_CUSTOM_DATA);
    if (customData == NULL || customData->valuestring == NULL) {
        cJSON_Delete(extraDataJsonObj);
        return DM_OK;
    }
    cJSON *customDataJson = cJSON_Parse(customData->valuestring);
    if (customDataJson == NULL) {
        cJSON_Delete(extraDataJsonObj);
        return DM_OK;
    }
    int32_t ret = GetAttrFromCustomData(customDataJson, dmDevInfo, actionId);
    cJSON_Delete(customDataJson);
    cJSON_Delete(extraDataJsonObj);
    return ret;
}

int32_t SoftbusListener::GetAttrFromCustomData(const cJSON *const customDataJson, DmDeviceInfo &dmDevInfo,
    int32_t &actionId)
{
    cJSON *actionIdJson = cJSON_GetObjectItem(customDataJson, CUSTOM_DATA_ACTIONID);
    if (actionIdJson == NULL || !cJSON_IsNumber(actionIdJson)) {
        return DM_OK;
    }
    actionId = actionIdJson->valueint;
    cJSON *networkIdJson = cJSON_GetObjectItem(customDataJson, CUSTOM_DATA_NETWORKID);
    if (networkIdJson == NULL || !cJSON_IsString(networkIdJson)) {
        return DM_OK;
    }
    std::string networkId = networkIdJson->valuestring;
    if (memcpy_s(dmDevInfo.deviceId, sizeof(dmDevInfo.deviceId), networkId.c_str(),
        std::min(sizeof(dmDevInfo.deviceId), sizeof(networkId))) != DM_OK) {
        LOGE("copy deviceId failed.");
        return ERR_DM_FAILED;
    }
    if (memcpy_s(dmDevInfo.networkId, sizeof(dmDevInfo.networkId), networkId.c_str(),
        std::min(sizeof(dmDevInfo.networkId), sizeof(networkId))) != DM_OK) {
        LOGE("copy networkId failed.");
        return ERR_DM_FAILED;
    }
    cJSON *displayNameJson = cJSON_GetObjectItem(customDataJson, CUSTOM_DATA_DISPLAY_NAME);
    if (displayNameJson == NULL || !cJSON_IsString(displayNameJson)) {
        return DM_OK;
    }
    std::string displayName = displayNameJson->valuestring;
    if (memcpy_s(dmDevInfo.deviceName, sizeof(dmDevInfo.deviceName), displayName.c_str(),
        std::min(sizeof(dmDevInfo.deviceName), sizeof(displayName))) != DM_OK) {
        LOGE("copy deviceName failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void SoftbusListener::GetActionId(const std::string &deviceId, int32_t &actionId)
{
    std::lock_guard<std::mutex> lock(g_lnnCbkMapMutex);
    if (discoveredDeviceActionIdMap.find(deviceId) == discoveredDeviceActionIdMap.end()) {
        return;
    }
    actionId = discoveredDeviceActionIdMap.find(deviceId)->second;
}
} // namespace DistributedHardware
} // namespace OHOS