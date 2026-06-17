/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "softbus_listener_Refactor.h"

#include <dlfcn.h>
#include <mutex>
#include <pthread.h>
#include <securec.h>
#include <thread>
#include <unistd.h>

#include "device_manager_service.h"
#include "dm_crypto.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "datetime_ex.h"
#include "dm_transport_msg.h"
#include "ffrt.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#include "dm_constraints_manager.h"
#endif
#include "ipc_skeleton.h"
#include "parameter.h"
#include "system_ability_definition.h"
#include "ipc_server_stub.h"

namespace OHOS {
namespace DistributedHardware {

const int32_t SOFTBUS_CHECK_INTERVAL = 100000;
const int32_t SOFTBUS_SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t MAX_CACHED_DISCOVERED_DEVICE_SIZE = 100;
const int32_t MAX_SOFTBUS_MSG_LEN = 2000;
const int32_t MAX_OSTYPE_SIZE = 1000;
constexpr int32_t MAX_CACHED_MAP_NUM = 5000;
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* DEVICE_ONLINE = "deviceOnLine";
constexpr const char* DEVICE_OFFLINE = "deviceOffLine";
constexpr const char* DEVICE_NAME_CHANGE = "deviceNameChange";
constexpr const char* DEVICE_SCREEN_STATUS_CHANGE = "deviceScreenStatusChange";
#endif
constexpr const char* CREDENTIAL_AUTH_STATUS = "credentialAuthStatus";
constexpr const char* LIB_RADAR_NAME = "libdevicemanagerradar.z.so";
constexpr static char HEX_ARRAY[] = "0123456789ABCDEF";
constexpr static uint8_t BYTE_MASK = 0x0F;
constexpr static uint16_t ARRAY_DOUBLE_SIZE = 2;
constexpr static uint16_t BIN_HIGH_FOUR_NUM = 4;
constexpr uint32_t SOFTBUS_MAX_RETRY_TIME = 10;

constexpr const char* CUSTOM_DATA_ACTIONID = "actionId";
constexpr const char* CUSTOM_DATA_NETWORKID = "networkId";
constexpr const char* CUSTOM_DATA_DISPLAY_NAME = "displayName";
constexpr const char* SOFTBUS_EVENT_QUEUE_HANDLE_TASK = "SoftbusEventQueueHandleTask";
constexpr const char* SOFTBUS_EVENT_QUEUE_HANDLE_TASK_BY_ADD = "SoftbusEventQueueHandleTaskByAdd";
constexpr const char* START_DETECT_DEVICE_RISK_TASK = "StartDetectDeviceRiskTask";
constexpr const char* DEVICE_NOT_TRUST_TASK = "DeviceNotTrustTask";
constexpr const char* DEVICE_TRUSTED_CHANGE_TASK = "DeviceTrustedChangeTask";
constexpr const char* DEVICE_USER_ID_CHECK_SUM_CHANGE_TASK = "DeviceUserIdCheckSumChangeTask";

static std::mutex g_deviceMapMutex;
static std::mutex g_lnnCbkMapMutex;
static std::mutex g_radarLoadLock;
static std::mutex g_onlineDeviceNumLock;

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
static ffrt::mutex g_lockDeviceTrustedChange;
static ffrt::mutex g_lockUserIdCheckSumChange;
static ffrt::mutex g_credentialAuthStatus;
static ffrt::mutex g_dmSoftbusEventQueueLock;
static ffrt::mutex g_lockDeviceOnLine;
static ffrt::mutex g_lockDeviceOffLine;
static ffrt::mutex g_lockDevInfoChange;
static ffrt::mutex g_lockDevScreenStatusChange;
#else
static std::mutex g_lockDeviceTrustedChange;
static std::mutex g_credentialAuthStatus;
static std::mutex g_lockDeviceOnLine;
static std::mutex g_lockDeviceOffLine;
static std::mutex g_lockDevInfoChange;
static std::mutex g_lockDevScreenStatusChange;
#endif
static std::mutex g_lockDeviceIdSet;
static std::map<std::string,
    std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>>> discoveredDeviceMap;
static std::map<std::string, std::shared_ptr<ISoftbusDiscoveringCallback>> lnnOpsCbkMap;
static std::map<std::string, int32_t> discoveredDeviceActionIdMap;
static std::set<std::string> deviceIdSet;
bool SoftbusListenerRefactor::isRadarSoLoad_ = false;
IDmRadarHelper* SoftbusListenerRefactor::dmRadarHelper_ = nullptr;
void* SoftbusListenerRefactor::radarHandle_ = nullptr;
static std::mutex g_hostNameMutex;
std::string SoftbusListenerRefactor::hostName_ = "";
int32_t g_onlineDeviceNum = 0;
static std::map<std::string, std::queue<DmSoftbusEvent>> g_dmSoftbusEventQueueMap;

static IPublishCb softbusPublishCallback_ = {
    .OnPublishResult = SoftbusListenerRefactor::OnSoftbusPublishResult,
};

static INodeStateCb softbusNodeStateCb_ = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE |
              EVENT_NODE_STATE_INFO_CHANGED | EVENT_NODE_STATUS_CHANGED |
              EVENT_NODE_HICHAIN_PROOF_EXCEPTION,
    .onNodeOnline = SoftbusListenerRefactor::OnSoftbusDeviceOnline,
    .onNodeOffline = SoftbusListenerRefactor::OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = SoftbusListenerRefactor::OnSoftbusDeviceInfoChanged,
    .onLocalNetworkIdChanged = SoftbusListenerRefactor::OnLocalDevInfoChange,
    .onNodeDeviceTrustedChange = SoftbusListenerRefactor::OnDeviceTrustedChange,
    .onNodeStatusChanged = SoftbusListenerRefactor::OnDeviceScreenStatusChanged,
    .onHichainProofException = SoftbusListenerRefactor::OnCredentialAuthStatus,
};

static IRefreshCallback softbusRefreshCallback_ = {
    .OnDeviceFound = SoftbusListenerRefactor::OnSoftbusDeviceFound,
    .OnDiscoverResult = SoftbusListenerRefactor::OnSoftbusDiscoveryResult,
};

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

static int OnAuth3rdAclSessionOpened(int sessionId, int result)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return IpcServerStub::GetInstance().OnAuth3rdAclSessionOpened(sessionId, result);
#else
    (void)sessionId;
    (void)result;
    return 0;
#endif
}

static void OnAuth3rdAclSessionClosed(int sessionId)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    IpcServerStub::GetInstance().OnAuth3rdAclSessionClosed(sessionId);
#else
    (void)sessionId;
#endif
}

static void OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    IpcServerStub::GetInstance().OnAuth3rdAclBytesReceived(sessionId, data, dataLen);
#else
    (void)sessionId;
    (void)data;
    (void)dataLen;
#endif
}

static int OnAuth3rdSessionOpened(int sessionId, int result)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return IpcServerStub::GetInstance().OnAuth3rdSessionOpened(sessionId, result);
#else
    (void)sessionId;
    (void)result;
    return 0;
#endif
}

static void OnAuth3rdSessionClosed(int sessionId)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    IpcServerStub::GetInstance().OnAuth3rdSessionClosed(sessionId);
#else
    (void)sessionId;
#endif
}

static void OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    IpcServerStub::GetInstance().OnAuth3rdBytesReceived(sessionId, data, dataLen);
#else
    (void)sessionId;
    (void)data;
    (void)dataLen;
#endif
}

static int OnAuthCred3rdSessionOpened(int sessionId, int result)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return IpcServerStub::GetInstance().OnAuthCred3rdSessionOpened(sessionId, result);
#else
    (void)sessionId;
    (void)result;
    return 0;
#endif
}

static void OnAuthCred3rdSessionClosed(int sessionId)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    IpcServerStub::GetInstance().OnAuthCred3rdSessionClosed(sessionId);
#else
    (void)sessionId;
#endif
}

static void OnAuthCred3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    IpcServerStub::GetInstance().OnAuthCred3rdBytesReceived(sessionId, data, dataLen);
#else
    (void)sessionId;
    (void)data;
    (void)dataLen;
#endif
}

void SoftbusListenerRefactor::DeviceOnLine(DmDeviceInfo deviceInfo)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_lockDeviceOnLine);
#else
    std::lock_guard<std::mutex> lock(g_lockDeviceOnLine);
#endif
    LOGI("received device online deviceId: %{public}s, networkId: %{public}s.",
         GetAnonyString(deviceInfo.deviceId).c_str(), GetAnonyString(deviceInfo.networkId).c_str());
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_STATE_ONLINE, deviceInfo, true);
}

void SoftbusListenerRefactor::DeviceOffLine(DmDeviceInfo deviceInfo)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_lockDeviceOffLine);
#else
    std::lock_guard<std::mutex> lock(g_lockDeviceOffLine);
#endif
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_STATE_OFFLINE, deviceInfo, false);
}

void SoftbusListenerRefactor::DeviceNameChange(DmDeviceInfo deviceInfo)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_lockDevInfoChange);
#else
    std::lock_guard<std::mutex> lock(g_lockDevInfoChange);
#endif
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(DEVICE_INFO_CHANGED, deviceInfo, true);
}

void SoftbusListenerRefactor::DeviceNotTrust(const std::string &msg)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_lockDeviceTrustedChange);
#else
    std::lock_guard<std::mutex> lock(g_lockDeviceTrustedChange);
#endif
    DeviceManagerService::GetInstance().HandleDeviceNotTrust(msg);
}

void SoftbusListenerRefactor::DeviceTrustedChange(const std::string &msg)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_lockDeviceTrustedChange);
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
#else
    (void)msg;
#endif
}

void SoftbusListenerRefactor::DeviceUserIdCheckSumChange(const std::string &msg)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_lockUserIdCheckSumChange);
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);
#else
    (void)msg;
#endif
}

void SoftbusListenerRefactor::DeviceScreenStatusChange(DmDeviceInfo deviceInfo)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_lockDevScreenStatusChange);
#else
    std::lock_guard<std::mutex> lock(g_lockDevScreenStatusChange);
#endif
    DeviceManagerService::GetInstance().HandleDeviceScreenStatusChange(deviceInfo);
}

void SoftbusListenerRefactor::CredentialAuthStatusProcess(std::string deviceList,
                                                          uint16_t deviceTypeId, int32_t errcode)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(g_credentialAuthStatus);
#else
    std::lock_guard<std::mutex> lock(g_credentialAuthStatus);
#endif
    DeviceManagerService::GetInstance().HandleCredentialAuthStatus(deviceList, deviceTypeId, errcode);
}

int32_t SoftbusListenerRefactor::OnSyncLocalAclList(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string remoteAclList)
{
    return DeviceManagerService::GetInstance().SyncLocalAclListProcess(
        localDevUserInfo, remoteDevUserInfo, remoteAclList, true);
}

int32_t SoftbusListenerRefactor::OnGetAclListHash(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclList)
{
    return DeviceManagerService::GetInstance().GetAclListHash(
        localDevUserInfo, remoteDevUserInfo, aclList);
}

SoftbusListenerRefactor::SoftbusListenerRefactor()
{
    LOGD("constructor.");
    CreateSessionServers();
    InitSoftbusListener();
    ClearDiscoveredDevice();
}

void SoftbusListenerRefactor::CreateSessionServers()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    CreateDefaultSessionServer();
    CreatePinHolderSessionServer();
    Create3rdAuthACLSessionServer();
    Create3rdAuthSessionServer();
    Create3rdAuthCredSessionServer();
#endif
}

void SoftbusListenerRefactor::CreateDefaultSessionServer()
{
    ISessionListener sessionListener = {
        .OnSessionOpened = OnSessionOpened,
        .OnSessionClosed = OnSessionClosed,
        .OnBytesReceived = OnBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr
    };
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_SESSION_NAME, &sessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer failed, ret: %{public}d.", ret);
    }
}

void SoftbusListenerRefactor::CreatePinHolderSessionServer()
{
    ISessionListener pinHolderSessionListener = {
        .OnSessionOpened = OnPinHolderSessionOpened,
        .OnSessionClosed = OnPinHolderSessionClosed,
        .OnBytesReceived = OnPinHolderBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr
    };
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_PIN_HOLDER_SESSION_NAME,
        &pinHolderSessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer pin holder failed, ret: %{public}d.", ret);
    }
}

void SoftbusListenerRefactor::Create3rdAuthACLSessionServer()
{
    ISessionListener auth3rdAclSessionListener = {
        .OnSessionOpened = OnAuth3rdAclSessionOpened,
        .OnSessionClosed = OnAuth3rdAclSessionClosed,
        .OnBytesReceived = OnAuth3rdAclBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr
    };
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_3RD_AUTH_ACL_SESSION_NAME,
        &auth3rdAclSessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer 3rdAcl session failed, ret: %{public}d.", ret);
    }
}

void SoftbusListenerRefactor::Create3rdAuthSessionServer()
{
    ISessionListener auth3rdSessionListener = {
        .OnSessionOpened = OnAuth3rdSessionOpened,
        .OnSessionClosed = OnAuth3rdSessionClosed,
        .OnBytesReceived = OnAuth3rdBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr
    };
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_AUTH_3RD_SESSION_NAME,
        &auth3rdSessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer 3rd session failed, ret: %{public}d.", ret);
    }
}

void SoftbusListenerRefactor::Create3rdAuthCredSessionServer()
{
    ISessionListener authCred3rdSessionListener = {
        .OnSessionOpened = OnAuthCred3rdSessionOpened,
        .OnSessionClosed = OnAuthCred3rdSessionClosed,
        .OnBytesReceived = OnAuthCred3rdBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr
    };
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_AUTH_CRED_3RD_SESSION_NAME,
        &authCred3rdSessionListener);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]CreateSessionServer 3rd session failed, ret: %{public}d.", ret);
    }
}

SoftbusListenerRefactor::~SoftbusListenerRefactor()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    RemoveSessionServer(DM_PKG_NAME, DM_SESSION_NAME);
    RemoveSessionServer(DM_PKG_NAME, DM_PIN_HOLDER_SESSION_NAME);
    RemoveSessionServer(DM_PKG_NAME, DM_3RD_AUTH_ACL_SESSION_NAME);
    RemoveSessionServer(DM_PKG_NAME, DM_AUTH_3RD_SESSION_NAME);
    RemoveSessionServer(DM_PKG_NAME, DM_AUTH_CRED_3RD_SESSION_NAME);
#endif
    LOGD("SoftbusListenerRefactor destructor.");
}

int32_t SoftbusListenerRefactor::InitSoftbusListener()
{
    int32_t ret;
    uint32_t retryTimes = 0;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    do {
        ret = RegNodeDeviceStateCb(DM_PKG_NAME, &softbusNodeStateCb_);
        if (ret != DM_OK) {
            ++retryTimes;
            LOGE("[SOFTBUS]RegNodeDeviceStateCb failed with ret: %{public}d, retryTimes: %{public}u.",
                ret, retryTimes);
            usleep(SOFTBUS_CHECK_INTERVAL);
        }
    } while (ret != DM_OK && retryTimes < SOFTBUS_MAX_RETRY_TIME);
#else
    (void)retryTimes;
    ret = DM_OK;
#endif
    return InitSoftPublishLNN();
}

int32_t SoftbusListenerRefactor::InitSoftPublishLNN()
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
        "%{public}s, ranging: %{public}d, freq: %{public}d.",
        publishInfo.publishId, publishInfo.mode, publishInfo.medium,
        publishInfo.capability, publishInfo.ranging, publishInfo.freq);
    ret = PublishLNN(DM_PKG_NAME, &publishInfo, &softbusPublishCallback_);
    if (ret == DM_OK) {
        LOGI("[SOFTBUS]PublishLNN successed, ret: %{public}d", ret);
    }
#endif
    return ret;
}

int32_t SoftbusListenerRefactor::GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    int32_t ret = SoftbusCache::GetInstance().GetDeviceInfoFromCache(deviceInfoList);
    size_t deviceCount = deviceInfoList.size();
    LOGI("size is %{public}zu.", deviceCount);
    return ret;
}

int32_t SoftbusListenerRefactor::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    return SoftbusCache::GetInstance().GetDevInfoByNetworkId(networkId, info);
}

int32_t SoftbusListenerRefactor::GetLocalDeviceInfo(DmDeviceInfo &deviceInfo)
{
    return SoftbusCache::GetInstance().GetLocalDeviceInfo(deviceInfo);
}

int32_t SoftbusListenerRefactor::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    return SoftbusCache::GetInstance().GetUdidFromCache(networkId, udid);
}

int32_t SoftbusListenerRefactor::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    return SoftbusCache::GetInstance().GetUuidFromCache(networkId, uuid);
}

int32_t SoftbusListenerRefactor::GetNetworkIdByUdid(const std::string &udid, std::string &networkId)
{
    return SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, networkId);
}

int32_t SoftbusListenerRefactor::GetDeviceNameByUdid(const std::string &udid, std::string &deviceName)
{
    return SoftbusCache::GetInstance().GetDeviceNameFromCache(udid, deviceName);
}

int32_t SoftbusListenerRefactor::ShiftLNNGear(bool isWakeUp, const std::string &callerId)
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

IDmRadarHelper* SoftbusListenerRefactor::GetDmRadarHelperObj()
{
    return dmRadarHelper_;
}

bool SoftbusListenerRefactor::CloseDmRadarHelperObj(std::string name)
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

void SoftbusListenerRefactor::SetHostPkgName(const std::string hostName)
{
    std::lock_guard<std::mutex> lock(g_hostNameMutex);
    hostName_ = hostName;
}

std::string SoftbusListenerRefactor::GetHostPkgName()
{
    std::lock_guard<std::mutex> lock(g_hostNameMutex);
    return hostName_;
}

IRefreshCallback &SoftbusListenerRefactor::GetSoftbusRefreshCb()
{
    return softbusRefreshCallback_;
}

void SoftbusListenerRefactor::SendAclChangedBroadcast(const std::string &msg)
{
    LOGI("start");
    if (SyncTrustedRelationShip(DM_PKG_NAME, msg.c_str(), msg.length()) != DM_OK) {
        LOGE("SyncTrustedRelationShip failed.");
    }
}

int32_t SoftbusListenerRefactor::GetDeviceScreenStatus(const char *networkId, int32_t &screenStatus)
{
    int32_t devScreenStatus = -1;
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId,
        NodeDeviceInfoKey::NODE_KEY_DEVICE_SCREEN_STATUS,
        reinterpret_cast<uint8_t *>(&devScreenStatus), LNN_COMMON_LEN);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo screenStatus failed.");
        return ret;
    }
    screenStatus = devScreenStatus;
    LOGI("screenStatus: %{public}d.", devScreenStatus);
    return DM_OK;
}

int32_t SoftbusListenerRefactor::SetForegroundUserIdsToDSoftBus(const std::string &remoteUdid,
                                                                const std::vector<uint32_t> &userIds)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    NotifyUserIds notifyUserIds(remoteUdid, userIds);
    std::string msg = notifyUserIds.ToString();
    (void)msg;
#else
    (void)remoteUdid;
    (void)userIds;
#endif
    return DM_OK;
}

int32_t SoftbusListenerRefactor::SetLocalDisplayName(const std::string &displayName)
{
    LOGI("start");
    uint32_t len = static_cast<uint32_t>(displayName.size());
    int32_t ret = ::SetDisplayName(DM_PKG_NAME, displayName.c_str(), len);
    if (ret != DM_OK) {
        LOGE("SoftbusListenerRefactor SetLocalDisplayName failed!");
        return ret;
    }
    return DM_OK;
}

} // namespace DistributedHardware
} // namespace OHOS