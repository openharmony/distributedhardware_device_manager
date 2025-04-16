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

#include "device_manager_service.h"

#include <dlfcn.h>
#include <functional>

#include "app_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_hidumper.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "parameter.h"
#include "permission_manager.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "common_event_support.h"
#include "datetime_ex.h"
#include "iservice_registry.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#if defined(SUPPORT_POWER_MANAGER)
#include "power_mgr_client.h"
#endif // SUPPORT_POWER_MANAGER
#if defined(SUPPORT_BLUETOOTH)
#include "softbus_publish.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#endif // SUPPORT_BLUETOOTH
#if defined(SUPPORT_WIFI)
#include "softbus_publish.h"
#include "wifi_device.h"
#include "wifi_msg.h"
#endif // SUPPORT_WIFI
#endif

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* LIB_IMPL_NAME = "libdevicemanagerserviceimpl.z.so";
using namespace OHOS::EventFwk;
#else
constexpr const char* LIB_IMPL_NAME = "libdevicemanagerserviceimpl.so";
#endif
constexpr const char* LIB_DM_ADAPTER_NAME = "libdevicemanageradapter.z.so";

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);
const int32_t NORMAL = 0;
const int32_t SYSTEM_BASIC = 1;
const int32_t SYSTEM_CORE = 2;
constexpr const char* ALL_PKGNAME = "";
constexpr const char* NETWORKID = "NETWORK_ID";
DeviceManagerService::~DeviceManagerService()
{
    LOGI("DeviceManagerService destructor");
    UnloadDMServiceImplSo();
    UnloadDMServiceAdapter();
}

int32_t DeviceManagerService::Init()
{
    InitSoftbusListener();
    InitDMServiceListener();
    LOGI("Init success, dm service single instance initialized.");
    return DM_OK;
}

int32_t DeviceManagerService::InitSoftbusListener()
{
    if (softbusListener_ == nullptr) {
        softbusListener_ = std::make_shared<SoftbusListener>();
    }
    SoftbusCache::GetInstance().UpdateDeviceInfoCache();
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
    SubscribePublishCommonEvent();
    QueryDependsSwitchState();
#endif // SUPPORT_BLUETOOTH SUPPORT_WIFI
#endif
    LOGI("SoftbusListener init success.");
    return DM_OK;
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
void DeviceManagerService::SubscribePublishCommonEvent()
{
    LOGI("DeviceManagerServiceImpl::SubscribeCommonEvent");
    if (publshCommonEventManager_ == nullptr) {
        publshCommonEventManager_ = std::make_shared<DmPublishCommonEventManager>();
    }
    PublishEventCallback callback = [=](const auto &arg1, const auto &arg2, const auto &arg3) {
        OHOS::DistributedHardware::PublishCommonEventCallback(arg1, arg2, arg3);
    };
    std::vector<std::string> PublishCommonEventVec;
#ifdef SUPPORT_BLUETOOTH
    PublishCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
#endif // SUPPORT_BLUETOOTH

#ifdef SUPPORT_WIFI
    PublishCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
#endif // SUPPORT_WIFI
    PublishCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    PublishCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    if (publshCommonEventManager_->SubscribePublishCommonEvent(PublishCommonEventVec, callback)) {
        LOGI("subscribe ble and wifi and screen common event success");
    }
    return;
}
#endif // SUPPORT_BLUETOOTH SUPPORT_WIFI
#endif

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
void DeviceManagerService::QueryDependsSwitchState()
{
    LOGI("DeviceManagerService::QueryDependsSwitchState start.");
    std::shared_ptr<DmPublishEventSubscriber> publishSubScriber = publshCommonEventManager_->GetSubscriber();
    CHECK_NULL_VOID(publishSubScriber);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NULL_VOID(samgr);
#ifdef SUPPORT_BLUETOOTH
    if (samgr->CheckSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID) == nullptr) {
        publishSubScriber->SetBluetoothState(static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_OFF));
    } else {
        if (Bluetooth::BluetoothHost::GetDefaultHost().IsBleEnabled()) {
            publishSubScriber->SetBluetoothState(static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_ON));
        } else {
            publishSubScriber->SetBluetoothState(static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_OFF));
        }
    }
#endif // SUPPORT_BLUETOOTH

#ifdef SUPPORT_WIFI
    if (samgr->CheckSystemAbility(WIFI_DEVICE_SYS_ABILITY_ID) == nullptr) {
        publishSubScriber->SetWifiState(static_cast<int32_t>(OHOS::Wifi::WifiState::DISABLED));
    } else {
        bool isWifiActive = false;
        auto wifiMgr = Wifi::WifiDevice::GetInstance(WIFI_DEVICE_ABILITY_ID);
        CHECK_NULL_VOID(wifiMgr);
        wifiMgr->IsWifiActive(isWifiActive);
        if (isWifiActive) {
            publishSubScriber->SetWifiState(static_cast<int32_t>(OHOS::Wifi::WifiState::ENABLED));
        } else {
            publishSubScriber->SetWifiState(static_cast<int32_t>(OHOS::Wifi::WifiState::DISABLED));
        }
    }
#endif // SUPPORT_WIFI

#ifdef SUPPORT_POWER_MANAGER
    if (samgr->CheckSystemAbility(POWER_MANAGER_SERVICE_ID) == nullptr) {
        publishSubScriber->SetScreenState(DM_SCREEN_OFF);
    } else {
        if (OHOS::PowerMgr::PowerMgrClient::GetInstance().IsScreenOn()) {
            publishSubScriber->SetScreenState(DM_SCREEN_ON);
        } else {
            publishSubScriber->SetScreenState(DM_SCREEN_OFF);
        }
    }
#else
    publishSubScriber->SetScreenState(DM_SCREEN_ON);
#endif // SUPPORT_POWER_MANAGER
    OHOS::DistributedHardware::PublishCommonEventCallback(publishSubScriber->GetBluetoothState(),
        publishSubScriber->GetWifiState(), publishSubScriber->GetScreenState());
}
#endif // SUPPORT_BLUETOOTH  SUPPORT_WIFI
#endif

void DeviceManagerService::UninitSoftbusListener()
{
    softbusListener_ = nullptr;
    LOGI("SoftbusListener uninit.");
}

int32_t DeviceManagerService::InitDMServiceListener()
{
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }
    if (advertiseMgr_ == nullptr) {
        advertiseMgr_ = std::make_shared<AdvertiseManager>(softbusListener_);
    }
    if (discoveryMgr_ == nullptr) {
        discoveryMgr_ = std::make_shared<DiscoveryManager>(softbusListener_, listener_);
    }
    if (pinHolder_ == nullptr) {
        pinHolder_ = std::make_shared<PinHolder>(listener_);
    }
    LOGI("Init success.");
    return DM_OK;
}

void DeviceManagerService::UninitDMServiceListener()
{
    listener_ = nullptr;
    advertiseMgr_ = nullptr;
    discoveryMgr_ = nullptr;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    KVAdapterManager::GetInstance().UnInit();
#endif
    LOGI("Uninit.");
}

void DeviceManagerService::RegisterCallerAppId(const std::string &pkgName)
{
    AppManager::GetInstance().RegisterCallerAppId(pkgName);
}

void DeviceManagerService::UnRegisterCallerAppId(const std::string &pkgName)
{
    AppManager::GetInstance().UnRegisterCallerAppId(pkgName);
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                   std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("Begin for pkgName = %{public}s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    bool isOnlyShowNetworkId = false;
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetTrustedDeviceList.", pkgName.c_str());
        isOnlyShowNetworkId = true;
    }
    std::vector<DmDeviceInfo> onlineDeviceList;
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetTrustedDeviceList(onlineDeviceList);
    if (ret != DM_OK) {
        LOGE("GetTrustedDeviceList failed");
        return ret;
    }
    if (isOnlyShowNetworkId && !onlineDeviceList.empty()) {
        for (auto item : onlineDeviceList) {
            DmDeviceInfo tempInfo;
            if (memcpy_s(tempInfo.networkId, DM_MAX_DEVICE_ID_LEN, item.networkId, sizeof(item.networkId)) != 0) {
                LOGE("get networkId: %{public}s failed", GetAnonyString(item.networkId).c_str());
            }
            deviceList.push_back(tempInfo);
        }
        return DM_OK;
    }
    if (onlineDeviceList.size() > 0 && IsDMServiceImplReady()) {
        std::unordered_map<std::string, DmAuthForm> udidMap;
        if (PermissionManager::GetInstance().CheckSystemSA(pkgName)) {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(std::string(ALL_PKGNAME));
        } else {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(pkgName);
        }
        for (auto item : onlineDeviceList) {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            ConvertUdidHashToAnoyDeviceId(item);
#endif
            std::string udid = "";
            SoftbusListener::GetUdidByNetworkId(item.networkId, udid);
            if (udidMap.find(udid) != udidMap.end()) {
                item.authForm = udidMap[udid];
                deviceList.push_back(item);
            }
        }
    }
    return DM_OK;
}

int32_t DeviceManagerService::ShiftLNNGear(const std::string &pkgName, const std::string &callerId, bool isRefresh,
                                           bool isWakeUp)
{
    LOGD("Begin for pkgName = %{public}s, callerId = %{public}s, isRefresh ="
        "%{public}d, isWakeUp = %{public}d", pkgName.c_str(), GetAnonyString(callerId).c_str(), isRefresh, isWakeUp);
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call ShiftLNNGear, pkgName = %{public}s", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || callerId.empty()) {
        LOGE("Invalid parameter, parameter is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (isRefresh) {
        CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
        int32_t ret = softbusListener_->ShiftLNNGear(isWakeUp, callerId);
        if (ret != DM_OK) {
            LOGE("ShiftLNNGear error, failed ret: %{public}d", ret);
            return ret;
        }
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    LOGI("Begin networkId %{public}s.", GetAnonyString(networkId).c_str());
    if (!PermissionManager::GetInstance().CheckPermission() &&
        !PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call GetDeviceInfo.");
        return ERR_DM_NO_PERMISSION;
    }
    if (networkId.empty()) {
        LOGE("Invalid parameter, networkId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetDeviceInfo(networkId, info);
    if (ret != DM_OK) {
        LOGE("Get DeviceInfo By NetworkId failed, ret : %{public}d", ret);
    }
    return ret;
}

int32_t DeviceManagerService::GetLocalDeviceInfo(DmDeviceInfo &info)
{
    LOGI("Begin.");
    bool isOnlyShowNetworkId = false;
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call GetLocalDeviceInfo.");
        isOnlyShowNetworkId = true;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetLocalDeviceInfo(info);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceInfo failed");
        return ret;
    }
    if (isOnlyShowNetworkId) {
        DmDeviceInfo tempInfo;
        if (memcpy_s(tempInfo.networkId, DM_MAX_DEVICE_ID_LEN, info.networkId, sizeof(info.networkId)) != 0) {
            LOGE("get networkId: %{public}s failed", GetAnonyString(info.networkId).c_str());
        }
        info = tempInfo;
        return DM_OK;
    }
    if (localDeviceId_.empty()) {
        char localDeviceId[DEVICE_UUID_LENGTH] = {0};
        char udidHash[DEVICE_UUID_LENGTH] = {0};
        GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
        if (Crypto::GetUdidHash(localDeviceId, reinterpret_cast<uint8_t *>(udidHash)) == DM_OK) {
            localDeviceId_ = udidHash;
        }
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHashTemp = "";
    if (ConvertUdidHashToAnoyDeviceId(localDeviceId_, udidHashTemp) == DM_OK) {
        (void)memset_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
        if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, udidHashTemp.c_str(), udidHashTemp.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(udidHashTemp).c_str());
            return ERR_DM_FAILED;
        }
        return DM_OK;
    }
#endif
    if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, localDeviceId_.c_str(), localDeviceId_.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(localDeviceId_).c_str());
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &udid)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetUdidByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, netWorkId: %{public}s", pkgName.c_str(),
            GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return SoftbusListener::GetUdidByNetworkId(netWorkId.c_str(), udid);
}

int32_t DeviceManagerService::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &uuid)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetUuidByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, netWorkId: %{public}s", pkgName.c_str(),
            GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return SoftbusListener::GetUuidByNetworkId(netWorkId.c_str(), uuid);
}

int32_t DeviceManagerService::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
    const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call StartDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s, extra = %{public}s",
        pkgName.c_str(), extra.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    SoftbusListener::SetHostPkgName(pkgName);
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID,
        std::to_string(subscribeInfo.subscribeId)));
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_DISC_MEDIUM, std::to_string(subscribeInfo.medium)));

    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, extra));
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->StartDiscovering(pkgName, discParam, filterOps);
}

int32_t DeviceManagerService::StartDeviceDiscovery(const std::string &pkgName, const uint16_t subscribeId,
                                                   const std::string &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call StartDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s, filterOptions = %{public}s, subscribeId = %{public}d",
        pkgName.c_str(), filterOptions.c_str(), subscribeId);
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    SoftbusListener::SetHostPkgName(pkgName);
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID, std::to_string(subscribeId)));

    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, filterOptions));
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->StartDiscovering(pkgName, discParam, filterOps);
}

int32_t DeviceManagerService::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (!PermissionManager::GetInstance().CheckPermission() &&
        !PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call StopDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->StopDiscovering(pkgName, subscribeId);
}

int32_t DeviceManagerService::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call PublishDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::map<std::string, std::string> advertiseParam;
    advertiseParam.insert(std::pair<std::string, std::string>(PARAM_KEY_PUBLISH_ID,
        std::to_string(publishInfo.publishId)));
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StartAdvertising(pkgName, advertiseParam);
}

int32_t DeviceManagerService::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnPublishDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StopAdvertising(pkgName, publishId);
}

int32_t DeviceManagerService::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                                 const std::string &deviceId, const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call AuthenticateDevice.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::AuthenticateDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("AuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    std::string queryDeviceId = deviceId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHash = "";
    if (GetUdidHashByAnoyDeviceId(deviceId, udidHash) == DM_OK) {
        queryDeviceId = udidHash;
    }
#endif
    PeerTargetId targetId;
    ConnectionAddrType addrType;
    int32_t ret = SoftbusListener::GetTargetInfoFromCache(queryDeviceId, targetId, addrType);
    if (ret != DM_OK) {
        LOGE("AuthenticateDevice failed, cannot get target info from cached discovered device map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TYPE, std::to_string(authType)));
    bindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_BIND_EXTRA_DATA, extra));
    bindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_CONN_ADDR_TYPE, std::to_string(addrType)));
    return dmServiceImpl_->BindTarget(pkgName, targetId, bindParam);
}

int32_t DeviceManagerService::UnAuthenticateDevice(const std::string &pkgName, const std::string &networkId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnAuthenticateDevice.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s, networkId = %{public}s",
        pkgName.c_str(), GetAnonyString(networkId).c_str());
    if (pkgName.empty() || networkId.empty()) {
        LOGE("DeviceManagerService::UnAuthenticateDevice error: Invalid parameter, pkgName: %{public}s",
            pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnAuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnAuthenticateDevice(pkgName, networkId);
}

int32_t DeviceManagerService::StopAuthenticateDevice(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call StopAuthenticateDevice.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("DeviceManagerService::StopAuthenticateDevice error: Invalid parameter, pkgName: %{public}s",
            pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Begin for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceImplReady()) {
        LOGE("StopAuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    if (dmServiceImpl_->StopAuthenticateDevice(pkgName) != DM_OK) {
        LOGE("dmServiceImpl_ StopAuthenticateDevice failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerService::BindDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
    const std::string &bindParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call BindDevice.");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::BindDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("BindDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    std::string queryDeviceId = deviceId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHash = "";
    if (GetUdidHashByAnoyDeviceId(deviceId, udidHash) == DM_OK) {
        queryDeviceId = udidHash;
    }
#endif
    PeerTargetId targetId;
    ConnectionAddrType addrType;
    int32_t ret = SoftbusListener::GetTargetInfoFromCache(queryDeviceId, targetId, addrType);
    if (ret != DM_OK) {
        LOGE("BindDevice failed, cannot get target info from cached discovered device map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> bindParamMap;
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TYPE, std::to_string(authType)));
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_BIND_EXTRA_DATA, bindParam));
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_CONN_ADDR_TYPE, std::to_string(addrType)));
    return dmServiceImpl_->BindTarget(pkgName, targetId, bindParamMap);
}

int32_t DeviceManagerService::UnBindDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call UnBindDevice.");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s, deviceId = %{public}s",
        pkgName.c_str(), GetAnonyString(deviceId).c_str());
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::UnBindDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnBindDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    std::string realDeviceId = deviceId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHash = "";
    if (GetUdidHashByAnoyDeviceId(deviceId, udidHash) == DM_OK) {
        realDeviceId = udidHash;
    }
#endif
    return dmServiceImpl_->UnBindDevice(pkgName, realDeviceId);
}

int32_t DeviceManagerService::SetUserOperation(std::string &pkgName, int32_t action, const std::string &params)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call SetUserOperation.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || params.empty()) {
        LOGE("DeviceManagerService::SetUserOperation error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (IsDMServiceAdapterLoad()) {
        dmServiceImplExt_->ReplyUiAction(pkgName, action, params);
    }
    if (!IsDMServiceImplReady()) {
        LOGE("SetUserOperation failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->SetUserOperation(pkgName, action, params);
}

void DeviceManagerService::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceStatusChange(devState, devInfo);
    }
    if (IsDMServiceAdapterLoad()) {
        dmServiceImplExt_->HandleDeviceStatusChange(devState, devInfo);
    }
}

int DeviceManagerService::OnSessionOpened(int sessionId, int result)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnSessionOpened failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->OnSessionOpened(sessionId, result);
}

void DeviceManagerService::OnSessionClosed(int sessionId)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnSessionClosed failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnSessionClosed(sessionId);
}

void DeviceManagerService::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnBytesReceived failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnBytesReceived(sessionId, data, dataLen);
}

int DeviceManagerService::OnPinHolderSessionOpened(int sessionId, int result)
{
    LOGI("In");
    return PinHolderSession::OnSessionOpened(sessionId, result);
}

void DeviceManagerService::OnPinHolderSessionClosed(int sessionId)
{
    LOGI("In");
    PinHolderSession::OnSessionClosed(sessionId);
}

void DeviceManagerService::OnPinHolderBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    LOGI("In");
    PinHolderSession::OnBytesReceived(sessionId, data, dataLen);
}

int32_t DeviceManagerService::RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call RequestCredential.");
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RequestCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RequestCredential(reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ImportCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportCredential(pkgName, credentialInfo);
}

int32_t DeviceManagerService::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call DeleteCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("DeleteCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->DeleteCredential(pkgName, deleteInfo);
}

int32_t DeviceManagerService::MineRequestCredential(const std::string &pkgName, std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call RequestCredential.");
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RequestCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->MineRequestCredential(pkgName, returnJsonStr);
}

int32_t DeviceManagerService::CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call CheckCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("CheckCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->CheckCredential(pkgName, reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ImportCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportCredential(pkgName, reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call DeleteCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("DeleteCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::RegisterCredentialCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call RegisterCredentialCallback.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RegisterCredentialCallback(pkgName);
}

int32_t DeviceManagerService::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnRegisterCredentialCallback.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnRegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnRegisterCredentialCallback(pkgName);
}

int32_t DeviceManagerService::RegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerService::RegisterUiStateCallback error: Invalid parameter, pkgName: %{public}s",
            pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call RegisterUiStateCallback.",
            GetAnonyString(pkgName).c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RegisterUiStateCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RegisterUiStateCallback(pkgName);
}

int32_t DeviceManagerService::UnRegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerService::UnRegisterUiStateCallback error: Invalid parameter, pkgName: %{public}s",
            pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnRegisterUiStateCallback.",
            GetAnonyString(pkgName).c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnRegisterUiStateCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnRegisterUiStateCallback(pkgName);
}

bool DeviceManagerService::IsDMServiceImplReady()
{
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    if (isImplsoLoaded_ && (dmServiceImpl_ != nullptr)) {
        return true;
    }
    void *so_handle = dlopen(LIB_IMPL_NAME, RTLD_NOW | RTLD_NODELETE);
    if (so_handle == nullptr) {
        LOGE("load libdevicemanagerserviceimpl so failed, errMsg: %{public}s.", dlerror());
        return false;
    }
    dlerror();
    auto func = (CreateDMServiceFuncPtr)dlsym(so_handle, "CreateDMServiceObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(so_handle);
        LOGE("Create object function is not exist.");
        return false;
    }

    dmServiceImpl_ = std::shared_ptr<IDeviceManagerServiceImpl>(func());
    if (dmServiceImpl_->Initialize(listener_) != DM_OK) {
        dlclose(so_handle);
        dmServiceImpl_ = nullptr;
        isImplsoLoaded_ = false;
        return false;
    }
    isImplsoLoaded_ = true;
    return true;
}

bool DeviceManagerService::IsDMImplSoLoaded()
{
    LOGI("In");
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    return isImplsoLoaded_;
}

int32_t DeviceManagerService::DmHiDumper(const std::vector<std::string>& args, std::string &result)
{
    LOGI("HiDump GetTrustedDeviceList");
    std::vector<HidumperFlag> dumpflag;
    HiDumpHelper::GetInstance().GetArgsType(args, dumpflag);

    for (unsigned int i = 0; i < dumpflag.size(); i++) {
        if (dumpflag[i] == HidumperFlag::HIDUMPER_GET_TRUSTED_LIST) {
            std::vector<DmDeviceInfo> deviceList;
            CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
            int32_t ret = softbusListener_->GetTrustedDeviceList(deviceList);
            if (ret != DM_OK) {
                result.append("HiDumpHelper GetTrustedDeviceList failed");
                LOGE("HiDumpHelper GetTrustedDeviceList failed");
                return ret;
            }

            for (unsigned int j = 0; j < deviceList.size(); j++) {
                HiDumpHelper::GetInstance().SetNodeInfo(deviceList[j]);
                LOGI("DeviceManagerService::DmHiDumper SetNodeInfo.");
            }
        }
    }
    HiDumpHelper::GetInstance().HiDump(args, result);
    return DM_OK;
}

int32_t DeviceManagerService::NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call NotifyEvent.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("NotifyEvent failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    if (eventId == DM_NOTIFY_EVENT_ON_PINHOLDER_EVENT) {
        LOGI("NotifyEvent on pin holder event start.");
        CHECK_NULL_RETURN(pinHolder_, ERR_DM_POINT_NULL);
        return pinHolder_->NotifyPinHolderEvent(pkgName, event);
    }
    return dmServiceImpl_->NotifyEvent(pkgName, eventId, event);
}

void DeviceManagerService::LoadHardwareFwkService()
{
    std::string extra;
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = GetTrustedDeviceList(DM_PKG_NAME, extra, deviceList);
    if (ret != DM_OK) {
        LOGE("LoadHardwareFwkService failed, get trusted devicelist failed.");
        return;
    }
    if (deviceList.size() > 0) {
        dmServiceImpl_->LoadHardwareFwkService();
    }
}

int32_t DeviceManagerService::GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
    std::string &uuid)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("PkgName = %{public}s", pkgName.c_str());
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetUuidByNetworkId(networkId.c_str(), uuid);
    if (ret != DM_OK) {
        LOGE("GetUuidByNetworkId failed, ret : %{public}d", ret);
        return ret;
    }

    std::string appId = Crypto::Sha256(AppManager::GetInstance().GetAppId());
    LOGI("appId = %{public}s, uuid = %{public}s.", GetAnonyString(appId).c_str(), GetAnonyString(uuid).c_str());
    uuid = Crypto::Sha256(appId + "_" + uuid);
    LOGI("encryptedUuid = %{public}s.", GetAnonyString(uuid).c_str());
    return DM_OK;
}

int32_t DeviceManagerService::GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
    const std::string &appId, std::string &encryptedUuid)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    encryptedUuid = Crypto::Sha256(appId + "_" + uuid);
    LOGI("encryptedUuid = %{public}s.", GetAnonyString(encryptedUuid).c_str());
    return DM_OK;
}

int32_t DeviceManagerService::CheckApiPermission(int32_t permissionLevel)
{
    LOGI("PermissionLevel: %{public}d", permissionLevel);
    int32_t ret = ERR_DM_NO_PERMISSION;
    switch (permissionLevel) {
        case NORMAL:
            if (PermissionManager::GetInstance().CheckNewPermission()) {
                LOGI("The caller have permission to call");
                ret = DM_OK;
            }
            break;
        case SYSTEM_BASIC:
            if (PermissionManager::GetInstance().CheckPermission()) {
                LOGI("The caller have permission to call");
                ret = DM_OK;
            }
            break;
        case SYSTEM_CORE:
            if (PermissionManager::GetInstance().CheckMonitorPermission()) {
                LOGI("The caller have permission to call");
                ret = DM_OK;
            }
            break;
        default:
            LOGE("DM have not this permissionLevel.");
            break;
    }
    return ret;
}

int32_t DeviceManagerService::GetNetworkTypeByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                        int32_t &networkType)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetNetworkTypeByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, netWorkId: %{public}s", pkgName.c_str(),
            GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    return softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType);
}

int32_t DeviceManagerService::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportAuthCode.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::ImportAuthCode begin.");
    if (authCode.empty() || pkgName.empty()) {
        LOGE("Invalid parameter, authCode: %{public}s.", GetAnonyString(authCode).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ImportAuthCode failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportAuthCode(pkgName, authCode);
}

int32_t DeviceManagerService::ExportAuthCode(std::string &authCode)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call ExportAuthCode.");
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, processName: %{public}s.", processName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ExportAuthCode failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    LOGI("DeviceManagerService::ExportAuthCode begin.");
    return dmServiceImpl_->ExportAuthCode(authCode);
}

void DeviceManagerService::UnloadDMServiceImplSo()
{
    LOGI("Start.");
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    if (dmServiceImpl_ != nullptr) {
        dmServiceImpl_->Release();
    }
    void *so_handle = dlopen(LIB_IMPL_NAME, RTLD_NOW | RTLD_NOLOAD);
    if (so_handle != nullptr) {
        LOGI("DeviceManagerService so_handle is not nullptr.");
        dlclose(so_handle);
    }
}

bool DeviceManagerService::IsDMServiceAdapterLoad()
{
    LOGI("Start.");
    if (listener_ == nullptr) {
        LOGE("Dm service is not init.");
        return false;
    }
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (isAdapterSoLoaded_ && (dmServiceImplExt_ != nullptr)) {
        return true;
    }

    void *so_handle = dlopen(LIB_DM_ADAPTER_NAME, RTLD_NOW | RTLD_NODELETE);
    if (so_handle == nullptr) {
        LOGE("load dm service adapter so failed.");
        return false;
    }
    dlerror();
    auto func = (CreateDMServiceImplExtFuncPtr)dlsym(so_handle, "CreateDMServiceImplExtObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(so_handle);
        LOGE("Create object function is not exist.");
        return false;
    }

    dmServiceImplExt_ = std::shared_ptr<IDMServiceImplExt>(func());
    if (dmServiceImplExt_->Initialize(listener_) != DM_OK) {
        dlclose(so_handle);
        dmServiceImplExt_ = nullptr;
        isAdapterSoLoaded_ = false;
        LOGE("dm service adapter impl ext init failed.");
        return false;
    }
    isAdapterSoLoaded_ = true;
    LOGI("Success.");
    return true;
}

void DeviceManagerService::UnloadDMServiceAdapter()
{
    LOGI("Start.");
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (dmServiceImplExt_ != nullptr) {
        dmServiceImplExt_->Release();
    }
    dmServiceImplExt_ = nullptr;

    void *so_handle = dlopen(LIB_DM_ADAPTER_NAME, RTLD_NOW | RTLD_NOLOAD);
    if (so_handle != nullptr) {
        LOGI("dm service adapter so_handle is not nullptr.");
        dlclose(so_handle);
    }
}

int32_t DeviceManagerService::StartDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        LOGI("StartDiscovering input MetaType = %{public}s", (discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->StartDiscovering(pkgName, discoverParam, filterOptions);
}

int32_t DeviceManagerService::StopDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    uint16_t subscribeId = -1;
    if (discoverParam.find(PARAM_KEY_SUBSCRIBE_ID) != discoverParam.end()) {
        subscribeId = std::atoi((discoverParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
    }
    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        LOGI("StopDiscovering input MetaType = %{public}s", (discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->StopDiscovering(pkgName, subscribeId);
}

int32_t DeviceManagerService::EnableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SoftbusListener::SetHostPkgName(pkgName);
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
}

int32_t DeviceManagerService::DisableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &extraParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->DisableDiscoveryListener(pkgName, extraParam);
}

int32_t DeviceManagerService::StartAdvertising(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StartAdvertising(pkgName, advertiseParam);
}

int32_t DeviceManagerService::StopAdvertising(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (advertiseParam.find(PARAM_KEY_META_TYPE) != advertiseParam.end()) {
        LOGI("StopAdvertising input MetaType=%{public}s", (advertiseParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    int32_t publishId = -1;
    if (advertiseParam.find(PARAM_KEY_PUBLISH_ID) != advertiseParam.end()) {
        publishId = std::atoi((advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second).c_str());
    }
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StopAdvertising(pkgName, publishId);
}

int32_t DeviceManagerService::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("BindTarget failed, DMServiceImpl instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    if (bindParam.find(PARAM_KEY_META_TYPE) == bindParam.end()) {
        LOGI("BindTarget stardard begin.");
        if (targetId.wifiIp.empty() || targetId.wifiIp.length() > IP_STR_MAX_LEN) {
            return dmServiceImpl_->BindTarget(pkgName, targetId, bindParam);
        }
        ConnectionAddrType ipAddrType;
        std::map<std::string, std::string> &noConstBindParam =
            const_cast<std::map<std::string, std::string> &>(bindParam);
        if (SoftbusListener::GetIPAddrTypeFromCache(targetId.deviceId, targetId.wifiIp, ipAddrType) == DM_OK) {
            noConstBindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_CONN_ADDR_TYPE,
                std::to_string(ipAddrType)));
        }
        const std::map<std::string, std::string> &constBindParam =
            const_cast<const std::map<std::string, std::string> &>(noConstBindParam);
        return dmServiceImpl_->BindTarget(pkgName, targetId, constBindParam);
    }
    if (!IsDMServiceAdapterLoad()) {
        LOGE("BindTarget failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    LOGI("BindTarget unstardard begin.");
    return dmServiceImplExt_->BindTargetExt(pkgName, targetId, bindParam);
}

int32_t DeviceManagerService::UnbindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &unbindParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceAdapterLoad()) {
        LOGE("UnbindTarget failed, instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    if (unbindParam.find(PARAM_KEY_META_TYPE) == unbindParam.end()) {
        LOGE("input unbind parameter not contains META_TYPE, dm service adapter not supported.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return dmServiceImplExt_->UnbindTargetExt(pkgName, targetId, unbindParam);
}

int32_t DeviceManagerService::RegisterPinHolderCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportAuthCode.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::RegisterPinHolderCallback begin.");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(pinHolder_, ERR_DM_POINT_NULL);
    return pinHolder_->RegisterPinHolderCallback(pkgName);
}

int32_t DeviceManagerService::CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call CreatePinHolder.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::CreatePinHolder begin.");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(pinHolder_, ERR_DM_POINT_NULL);
    return pinHolder_->CreatePinHolder(pkgName, targetId, pinType, payload);
}

int32_t DeviceManagerService::DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call DestroyPinHolder.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Begin.");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(pinHolder_, ERR_DM_POINT_NULL);
    return pinHolder_->DestroyPinHolder(pkgName, targetId, pinType, payload);
}

void DeviceManagerService::OnUnbindSessionOpened(int32_t socket, PeerSocketInfo info)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnUnbindSessionOpened failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnUnbindSessionOpened(socket, info);
}

void DeviceManagerService::OnUnbindSessionCloseed(int32_t socket)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnUnbindSessionCloseed failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnUnbindSessionCloseed(socket);
}

void DeviceManagerService::OnUnbindBytesReceived(int32_t socket, const void *data, uint32_t dataLen)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnUnbindBytesReceived failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnUnbindBytesReceived(socket, data, dataLen);
}

int32_t DeviceManagerService::DpAclAdd(const std::string &udid)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call DpAclAdd.");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start.");
    if (!IsDMServiceImplReady()) {
        LOGE("DpAclAdd failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    dmServiceImpl_->DpAclAdd(udid);
    LOGI("DeviceManagerService::DpAclAdd completed");
    return DM_OK;
}

int32_t DeviceManagerService::GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId,
                                                     int32_t &securityLevel)
{
    LOGI("Begin pkgName: %{public}s, networkId: %{public}s",
        pkgName.c_str(), GetAnonyString(networkId).c_str());
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetDeviceSecurityLevel.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || networkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(),
            GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetDeviceSecurityLevel(networkId.c_str(), securityLevel);
    if (ret != DM_OK) {
        LOGE("GetDeviceSecurityLevel failed, ret = %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::IsSameAccount(const std::string &networkId)
{
    LOGI("NetworkId %{public}s.", GetAnonyString(networkId).c_str());
    if (!PermissionManager::GetInstance().CheckPermission()) {
        return ERR_DM_NO_PERMISSION;
    }
    std::string udid = "";
    if (SoftbusListener::GetUdidByNetworkId(networkId.c_str(), udid) != DM_OK) {
        LOGE("DeviceManagerService::IsSameAccount error: udid: %{public}s", GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("IsSameAccount failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->IsSameAccount(udid);
}

bool DeviceManagerService::CheckAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call CheckAccessControl.", caller.pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("CheckAccessControl failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    std::string srcUdid = "";
    SoftbusListener::GetUdidByNetworkId(caller.networkId.c_str(), srcUdid);
    std::string sinkUdid = "";
    SoftbusListener::GetUdidByNetworkId(callee.networkId.c_str(), sinkUdid);
    return dmServiceImpl_->CheckAccessControl(caller, srcUdid, callee, sinkUdid);
}

bool DeviceManagerService::CheckIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call CheckIsSameAccount.", caller.pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("CheckIsSameAccount failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    std::string srcUdid = "";
    SoftbusListener::GetUdidByNetworkId(caller.networkId.c_str(), srcUdid);
    std::string sinkUdid = "";
    SoftbusListener::GetUdidByNetworkId(callee.networkId.c_str(), sinkUdid);
    return dmServiceImpl_->CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
}

int32_t DeviceManagerService::InitAccountInfo()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    SubscribeAccountCommonEvent();
    LOGI("Success.");
#endif
    return DM_OK;
}

int32_t DeviceManagerService::InitScreenLockEvent()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    SubscribeScreenLockEvent();
    LOGI("Success.");
#endif
    return DM_OK;
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::SubscribeAccountCommonEvent()
{
    LOGI("Start");
    if (accountCommonEventManager_ == nullptr) {
        accountCommonEventManager_ = std::make_shared<DmAccountCommonEventManager>();
    }
    AccountEventCallback callback = [=](const auto &arg1, const auto &arg2) {
        this->AccountCommonEventCallback(arg1, arg2);
    };
    std::vector<std::string> AccountCommonEventVec;
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
    if (accountCommonEventManager_->SubscribeAccountCommonEvent(AccountCommonEventVec, callback)) {
        LOGI("Success");
    }
    return;
}

void DeviceManagerService::SubscribeScreenLockEvent()
{
    LOGI("Start");
    if (screenCommonEventManager_ == nullptr) {
        screenCommonEventManager_ = std::make_shared<DmScreenCommonEventManager>();
    }
    ScreenEventCallback callback = [=](const auto &arg1) { this->ScreenCommonEventCallback(arg1); };
    std::vector<std::string> screenEventVec;
    screenEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);
    if (screenCommonEventManager_->SubscribeScreenCommonEvent(screenEventVec, callback)) {
        LOGI("Success");
    }
    return;
}

void DeviceManagerService::AccountCommonEventCallback(int32_t userId, std::string commonEventType)
{
    LOGI("CommonEventType: %{public}s", commonEventType.c_str());
    if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        MultipleUserConnector::SetSwitchOldUserId(userId);
        MultipleUserConnector::SetSwitchOldAccountId(MultipleUserConnector::GetOhosAccountId());
        MultipleUserConnector::SetSwitchOldAccountName(MultipleUserConnector::GetOhosAccountName());
        if (IsDMServiceAdapterLoad()) {
            dmServiceImplExt_->AccountUserSwitched(userId, MultipleUserConnector::GetOhosAccountId());
        }
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_HWID_LOGIN) {
        MultipleUserConnector::SetSwitchOldAccountId(MultipleUserConnector::GetOhosAccountId());
        MultipleUserConnector::SetSwitchOldAccountName(MultipleUserConnector::GetOhosAccountName());
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_HWID_LOGOUT) {
        HandleAccountLogout(MultipleUserConnector::GetCurrentAccountUserID(),
            MultipleUserConnector::GetSwitchOldAccountId());
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        HandleUserRemoved(userId);
    } else {
        LOGE("Invalied account common event.");
    }
    return;
}

void DeviceManagerService::HandleAccountLogout(int32_t userId, const std::string &accountId)
{
    LOGI("UserId %{public}d, accountId %{public}s.", userId, GetAnonyString(accountId).c_str());
    if (IsDMServiceAdapterLoad()) {
        dmServiceImplExt_->AccountIdLogout(userId, accountId);
    }
    if (!IsDMServiceImplReady()) {
        LOGE("Init impl failed.");
        return;
    }
    std::map<std::string, int32_t> deviceMap;
    deviceMap = dmServiceImpl_->GetDeviceIdAndBindType(userId, accountId);
    for (const auto &item : deviceMap) {
        if (item.second == DM_IDENTICAL_ACCOUNT) {
            dmServiceImpl_->HandleIdentAccountLogout(item.first, userId, accountId);
        }
    }
}

void DeviceManagerService::HandleUserRemoved(int32_t preUserId)
{
    LOGI("PreUserId %{public}d.", preUserId);
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleUserRemoved(preUserId);
    }
}

void DeviceManagerService::ScreenCommonEventCallback(std::string commonEventType)
{
    if (!IsDMImplSoLoaded()) {
        LOGE("ScreenCommonEventCallback failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->ScreenCommonEventCallback(commonEventType);
}
#endif

void DeviceManagerService::HandleDeviceNotTrust(const std::string &msg)
{
    LOGI("Start.");
    if (msg.empty()) {
        LOGE("DeviceManagerService::HandleDeviceNotTrust msg is empty.");
        return;
    }
    nlohmann::json msgJsonObj = nlohmann::json::parse(msg, nullptr, false);
    if (msgJsonObj.is_discarded()) {
        LOGE("HandleDeviceNotTrust msg prase error.");
        return;
    }
    if (!IsString(msgJsonObj, NETWORKID)) {
        LOGE("HandleDeviceNotTrust msg not contain networkId.");
        return;
    }
    std::string networkId = msgJsonObj[NETWORKID].get<std::string>();
    std::string udid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(networkId.c_str(), udid);
    LOGI("NetworkId: %{public}s, udid: %{public}s.",
        GetAnonyString(networkId).c_str(), GetAnonyString(udid).c_str());
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceNotTrust(udid);
    }
    if (IsDMServiceAdapterLoad()) {
        dmServiceImplExt_->HandleDeviceNotTrust(udid);
    }
    return;
}

int32_t DeviceManagerService::SetDnPolicy(const std::string &pkgName, std::map<std::string, std::string> &policy)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto policyStrategyIter = policy.find(PARAM_KEY_POLICY_STRATEGY_FOR_BLE);
    if (policyStrategyIter == policy.end()) {
        LOGE("Invalid parameter, DM_POLICY_STRATEGY_FOR_BLE is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto timeOutIter = policy.find(PARAM_KEY_POLICY_TIME_OUT);
    if (timeOutIter == policy.end()) {
        LOGE("Invalid parameter, DM_POLICY_TIMEOUT is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsNumberString(policyStrategyIter->second)) {
        LOGE("Invalid parameter, DM_POLICY_STRATEGY_FOR_BLE is not number.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsNumberString(timeOutIter->second)) {
        LOGE("Invalid parameter, DM_POLICY_TIMEOUT is not number.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t policyStrategy = std::atoi(policyStrategyIter->second.c_str());
    int32_t timeOut = std::atoi(timeOutIter->second.c_str());
    LOGD("strategy: %{public}d, timeOut: %{public}d", policyStrategy, timeOut);
    if (!IsDMServiceAdapterLoad()) {
        LOGE("SetDnPolicy failed, instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExt_->SetDnPolicy(policyStrategy, timeOut);
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::ConvertUdidHashToAnoyDeviceId(DmDeviceInfo &deviceInfo)
{
    std::string udidHashTemp = "";
    if (ConvertUdidHashToAnoyDeviceId(deviceInfo.deviceId, udidHashTemp) == DM_OK) {
        (void)memset_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
        if (memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udidHashTemp.c_str(), udidHashTemp.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(udidHashTemp).c_str());
        }
    }
}

int32_t DeviceManagerService::ConvertUdidHashToAnoyDeviceId(const std::string &udidHash, std::string &result)
{
    LOGI("udidHash %{public}s.", GetAnonyString(udidHash).c_str());
    std::string appId = AppManager::GetInstance().GetAppId();
    if (appId.empty()) {
        LOGD("GetAppId failed");
        return ERR_DM_FAILED;
    }
    DmKVValue kvValue;
    int32_t ret = Crypto::ConvertUdidHashToAnoyAndSave(appId, udidHash, kvValue);
    if (ret != DM_OK) {
        return ERR_DM_FAILED;
    }
    result = kvValue.anoyDeviceId;
    return DM_OK;
}

int32_t DeviceManagerService::GetUdidHashByAnoyDeviceId(const std::string &anoyDeviceId, std::string &udidHash)
{
    LOGI("anoyDeviceId %{public}s.", GetAnonyString(anoyDeviceId).c_str());
    DmKVValue kvValue;
    if (KVAdapterManager::GetInstance().Get(anoyDeviceId, kvValue) != DM_OK) {
        LOGD("Get kv value from DB failed");
        return ERR_DM_FAILED;
    }
    udidHash = kvValue.udidHash;
    LOGI("udidHash %{public}s.", GetAnonyString(udidHash).c_str());
    return DM_OK;
}
#endif

int32_t DeviceManagerService::GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid,
                                                 std::string &networkId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetNetworkIdByUdid.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || udid.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, udid: %{public}s", pkgName.c_str(), GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return SoftbusListener::GetNetworkIdByUdid(udid, networkId);
}

void DeviceManagerService::SubscribePackageCommonEvent()
{
    LOGI("Start");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (packageCommonEventManager_ == nullptr) {
        packageCommonEventManager_ = std::make_shared<DmPackageCommonEventManager>();
    }
    PackageEventCallback callback = [=](const auto &arg1, const auto &arg2) {
        KVAdapterManager::GetInstance().AppUnintall(arg1);
    };
    std::vector<std::string> commonEventVec;
    commonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    commonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED);
    if (packageCommonEventManager_->SubscribePackageCommonEvent(commonEventVec, callback)) {
        LOGI("Success");
    }
#endif
}

void DeviceManagerService::HandleDeviceScreenStatusChange(DmDeviceInfo &deviceInfo)
{
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceScreenStatusChange(deviceInfo);
    }
}

int32_t DeviceManagerService::GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
    int32_t &screenStatus)
{
    LOGI("Begin pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(), GetAnonyString(networkId).c_str());
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetDeviceScreenStatus.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || networkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(),
            GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetDeviceScreenStatus(networkId.c_str(), screenStatus);
    if (ret != DM_OK) {
        LOGE("GetDeviceScreenStatus failed, ret = %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

void DeviceManagerService::HandleCredentialAuthStatus(const std::string &proofInfo, uint16_t deviceTypeId,
                                                      int32_t errcode)
{
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleCredentialAuthStatus(proofInfo, deviceTypeId, errcode);
    }
}

int32_t DeviceManagerService::GetDeviceProfileInfoList(const std::string &pkgName,
    DmDeviceProfileInfoFilterOptions &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterLoad()) {
        LOGE("GetDeviceProfileInfoList failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExt_->GetDeviceProfileInfoList(pkgName, filterOptions);
}

int32_t DeviceManagerService::RegisterAuthenticationType(const std::string &pkgName,
    const std::map<std::string, std::string> &authParam)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto authTypeIter = authParam.find(DM_AUTHENTICATION_TYPE);
    if (authTypeIter == authParam.end()) {
        LOGE("Invalid parameter, DM_AUTHENTICATION_TYPE is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsNumberString(authTypeIter->second)) {
        LOGE("Invalid parameter, DM_AUTHENTICATION_TYPE is not number.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t authenticationType = std::atoi(authTypeIter->second.c_str());

    if (!IsDMServiceImplReady()) {
        LOGE("RegisterAuthenticationType failed, instance not init or init failed.");
        return ERR_DM_INIT_FAILED;
    }
    return dmServiceImpl_->RegisterAuthenticationType(authenticationType);
}
} // namespace DistributedHardware
} // namespace OHOS