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

#include "cJSON.h"
#include <dlfcn.h>
#include <functional>
#include "app_manager.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_hidumper.h"
#include "dm_softbus_cache.h"
#include "parameter.h"
#include "permission_manager.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "common_event_support.h"
#include "datetime_ex.h"
#include "deviceprofile_connector.h"
#include "device_name_manager.h"
#include "dm_comm_tool.h"
#include "dm_random.h"
#include "dm_transport_msg.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#include "relationship_sync_mgr.h"
#include "openssl/sha.h"
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
constexpr const char* LIB_DM_RESIDENT_NAME = "libdevicemanagerresident.z.so";
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
constexpr const char* LIB_DM_CHECK_API_WHITE_LIST_NAME = "libdm_check_api_whitelist.z.so";
#endif

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);
namespace {
    const int32_t NORMAL = 0;
    const int32_t SYSTEM_BASIC = 1;
    const int32_t SYSTEM_CORE = 2;
    constexpr int32_t NETWORK_AVAILABLE = 3;
    constexpr const char *ALL_PKGNAME = "";
    constexpr const char *NETWORKID = "NETWORK_ID";
    constexpr uint32_t INVALIED_BIND_LEVEL = 0;
    constexpr uint32_t DM_IDENTICAL_ACCOUNT = 1;
    constexpr uint32_t DM_SHARE = 2;
    const std::string USERID_CHECKSUM_NETWORKID_KEY = "networkId";
    const std::string USERID_CHECKSUM_DISCOVER_TYPE_KEY = "discoverType";
    const std::string DHARD_WARE_PKG_NAME = "ohos.dhardware";
    const std::string USERID_CHECKSUM_ISCHANGE_KEY = "ischange";
    constexpr const char* USER_SWITCH_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:userSwitchByWifi";
    constexpr const char* USER_STOP_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:userStopByWifi";
    constexpr const char* ACCOUNT_COMMON_EVENT_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:accountCommonEventByWifi";
    const int32_t USER_SWITCH_BY_WIFI_TIMEOUT_S = 2;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
    const std::string GET_LOCAL_DEVICE_NAME_API_NAME = "GetLocalDeviceName";
#endif
}

DeviceManagerService::~DeviceManagerService()
{
    LOGI("DeviceManagerService destructor");
    UnloadDMServiceImplSo();
    UnloadDMServiceAdapterResident();
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
    std::vector<DmDeviceInfo> onlineDeviceList;
    SoftbusCache::GetInstance().GetDeviceInfoFromCache(onlineDeviceList);
    if (onlineDeviceList.size() > 0 && IsDMServiceImplReady()) {
        dmServiceImpl_->SaveOnlineDeviceInfo(onlineDeviceList);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
    SubscribePublishCommonEvent();
    QueryDependsSwitchState();
#endif // SUPPORT_BLUETOOTH SUPPORT_WIFI
    SubscribeDataShareCommonEvent();
#endif
    LOGI("SoftbusListener init success.");
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("load dm service resident failed.");
    }
    return DM_OK;
}

void DeviceManagerService::InitHichainListener()
{
    LOGI("DeviceManagerService::InitHichainListener Start.");
    std::lock_guard<std::mutex> lock(hichainListenerLock_);
    if (hichainListener_ == nullptr) {
        hichainListener_ = std::make_shared<HichainListener>();
    }
    hichainListener_->RegisterDataChangeCb();
    hichainListener_->RegisterCredentialCb();
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
DM_EXPORT void DeviceManagerService::SubscribeDataShareCommonEvent()
{
    LOGI("DeviceManagerServiceImpl::SubscribeDataShareCommonEvent");
    if (dataShareCommonEventManager_ == nullptr) {
        dataShareCommonEventManager_ = std::make_shared<DmDataShareCommonEventManager>();
    }
    DataShareEventCallback callback = [=](const auto &arg1, const auto &arg2) {
        if (arg1 == CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY) {
            DeviceNameManager::GetInstance().DataShareReady();
        }
        if (arg1 == CommonEventSupport::COMMON_EVENT_LOCALE_CHANGED) {
            DeviceNameManager::GetInstance().InitDeviceNameWhenLanguageOrRegionChanged();
        }
        if (arg1 == CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE && arg2 == NETWORK_AVAILABLE) {
            this->HandleNetworkConnected();
        }
    };
    std::vector<std::string> commonEventVec;
    commonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
    commonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_LOCALE_CHANGED);
    commonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    if (dataShareCommonEventManager_->SubscribeDataShareCommonEvent(commonEventVec, callback)) {
        LOGI("subscribe datashare common event success");
    }
}
#endif

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
void DeviceManagerService::QueryDependsSwitchState()
{
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
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->DeleteCacheDeviceInfo();
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DMCommTool::GetInstance()->Init();
    int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec_);
    int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec_);
    MultipleUserConnector::ClearLockedUser(foregroundUserVec_, backgroundUserVec_);
    if (retFront != DM_OK || retBack != DM_OK) {
        LOGE("retFront: %{public}d, retBack: %{public}d, frontuserids: %{public}s, backuserids: %{public}s",
            retFront, retBack, GetIntegerList(foregroundUserVec_).c_str(), GetIntegerList(backgroundUserVec_).c_str());
    }
    int32_t currentUserId = MultipleUserConnector::GetFirstForegroundUserId();
    if (IsPC() && !MultipleUserConnector::IsUserUnlocked(currentUserId)) {
        HandleUserStopEvent(currentUserId);
    }
#endif
    LOGI("Init success.");
    return DM_OK;
}

DM_EXPORT void DeviceManagerService::UninitDMServiceListener()
{
    listener_ = nullptr;
    advertiseMgr_ = nullptr;
    discoveryMgr_ = nullptr;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DeviceNameManager::GetInstance().UnInit();
    KVAdapterManager::GetInstance().UnInit();
#endif
    LOGI("Uninit.");
}

DM_EXPORT void DeviceManagerService::RegisterCallerAppId(const std::string &pkgName)
{
    AppManager::GetInstance().RegisterCallerAppId(pkgName);
}

DM_EXPORT void DeviceManagerService::UnRegisterCallerAppId(const std::string &pkgName)
{
    AppManager::GetInstance().UnRegisterCallerAppId(pkgName);
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                   std::vector<DmDeviceInfo> &deviceList)
{
    (void)extra;
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    bool isOnlyShowNetworkId = !(PermissionManager::GetInstance().CheckPermission() ||
        PermissionManager::GetInstance().CheckNewPermission());
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
                return ERR_DM_SECURITY_FUNC_FAILED;
            }
            deviceList.push_back(tempInfo);
        }
        return DM_OK;
    }
    if (!onlineDeviceList.empty() && IsDMServiceImplReady()) {
        std::unordered_map<std::string, DmAuthForm> udidMap;
        if (PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName)) {
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

int32_t DeviceManagerService::GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                      std::vector<DmDeviceInfo> &deviceList)
{
    (void)extra;
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName or extra is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetAllTrustedDeviceList.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetAllTrustedDeviceList(pkgName, extra, deviceList);
    if (ret != DM_OK) {
        LOGE("GetAllTrustedDeviceList failed");
        return ret;
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
    std::string peerDeviceId = "";
    SoftbusListener::GetUdidByNetworkId(networkId.c_str(), peerDeviceId);
    int32_t ret = DM_OK;
    if (!IsDMServiceImplReady()) {
        LOGE("GetDeviceInfo failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    if (localUdid == peerDeviceId) {
        ret = softbusListener_->GetDeviceInfo(networkId, info);
        if (ret != DM_OK) {
            LOGE("Get DeviceInfo By NetworkId failed, ret : %{public}d", ret);
        }
        return ret;
    }
    if (!AppManager::GetInstance().IsSystemSA()) {
        int32_t permissionRet = dmServiceImpl_->CheckDeviceInfoPermission(localUdid, peerDeviceId);
        if (permissionRet != DM_OK) {
            std::string processName = "";
            if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
                LOGE("Get caller process name failed.");
                return ret;
            }
            if (!PermissionManager::GetInstance().CheckProcessNameValidOnGetDeviceInfo(processName)) {
                LOGE("The caller: %{public}s is not in white list.", processName.c_str());
                return ret;
            }
        }
    }
    ret = softbusListener_->GetDeviceInfo(networkId, info);
    if (ret != DM_OK) {
        LOGE("Get DeviceInfo By NetworkId failed, ret : %{public}d", ret);
    }
    return ret;
}

int32_t DeviceManagerService::GetLocalDeviceInfo(DmDeviceInfo &info)
{
    LOGD("Begin.");
    bool isOnlyShowNetworkId = false;
    CheckPermission(isOnlyShowNetworkId);
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
            return ERR_DM_FAILED;
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
        if (memset_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN) != DM_OK) {
            LOGE("GetLocalDeviceInfo memset_s failed.");
            return ERR_DM_FAILED;
        }
        if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, udidHashTemp.c_str(), udidHashTemp.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(udidHashTemp).c_str());
            return ERR_DM_FAILED;
        }
        return DM_OK;
    }
#endif
    if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, localDeviceId_.c_str(), localDeviceId_.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(localDeviceId_).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void DeviceManagerService::CheckPermission(bool &isOnlyShowNetworkId)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
    if (!PermissionManager::GetInstance().CheckNewPermission() && !IsCallerInWhiteList()) {
        LOGE("The caller does not have permission to call GetLocalDeviceInfo.");
        isOnlyShowNetworkId = true;
    }
#else
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call GetLocalDeviceInfo.");
        isOnlyShowNetworkId = true;
    }
#endif
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
bool DeviceManagerService::IsCallerInWhiteList()
{
    if (!IsDMAdapterCheckApiWhiteListLoaded()) {
        LOGE("DMAdapterCheckApiWhiteListLoad failed.");
        return false;
    }
    std::string callerName;
    if (AppManager::GetInstance().GetCallerProcessName(callerName) != DM_OK) {
        LOGE("GetCallerProcessName failed.");
        return false;
    }
    return dmCheckApiWhiteList_->IsCallerInWhiteList(callerName, GET_LOCAL_DEVICE_NAME_API_NAME);
}

bool DeviceManagerService::IsDMAdapterCheckApiWhiteListLoaded()
{
    LOGD("Start.");
    std::lock_guard<std::mutex> lock(isAdapterCheckApiWhiteListLoadedLock_);
    if (isAdapterCheckApiWhiteListSoLoaded_ && (dmCheckApiWhiteList_ != nullptr)) {
        return true;
    }
    checkApiWhiteListSoHandle_ = dlopen(LIB_DM_CHECK_API_WHITE_LIST_NAME, RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
    if (checkApiWhiteListSoHandle_ == nullptr) {
        checkApiWhiteListSoHandle_ = dlopen(LIB_DM_CHECK_API_WHITE_LIST_NAME, RTLD_NOW | RTLD_NODELETE);
    }
    if (checkApiWhiteListSoHandle_ == nullptr) {
        LOGE("load dm check api white list so failed.");
        return false;
    }
    if (dlerror() != nullptr) {
        dlclose(checkApiWhiteListSoHandle_);
        checkApiWhiteListSoHandle_ = nullptr;
        LOGE("open dm check api white list so failed. err: %{public}s", dlerror());
        return false;
    }
    auto func = (CreateDMCheckApiWhiteListFuncPtr)dlsym(checkApiWhiteListSoHandle_, "CreateDMCheckApiWhiteListObject");
    if (func == nullptr || dlerror() != nullptr) {
        dlclose(checkApiWhiteListSoHandle_);
        checkApiWhiteListSoHandle_ = nullptr;
        LOGE("Create object function is not exist. err: %{public}s", (dlerror() == nullptr ? "null" : dlerror()));
        return false;
    }
    dmCheckApiWhiteList_ = std::shared_ptr<IDMCheckApiWhiteList>(func());
    isAdapterCheckApiWhiteListSoLoaded_ = true;
    LOGI("Success.");
    return true;
}
#endif

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
    std::string udid = "";
    if (SoftbusListener::GetUdidByNetworkId(networkId.c_str(), udid) != DM_OK) {
        LOGE("UnAuthenticateDevice GetUdidByNetworkId error: udid: %{public}s", GetAnonyString(udid).c_str());
        return ERR_DM_FAILED;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    if (!IsDMServiceImplReady()) {
        LOGE("UnAuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    uint64_t tokenId = 0;
    int32_t bindLevel = dmServiceImpl_->GetBindLevel(pkgName, std::string(localUdid), udid, tokenId);
    LOGI("UnAuthenticateDevice get bindlevel %{public}d.", bindLevel);
    if (bindLevel == INVALIED_BIND_LEVEL) {
        LOGE("UnAuthenticateDevice failed, Acl not contain the bindLevel %{public}d.", bindLevel);
        return ERR_DM_FAILED;
    }
    if (dmServiceImpl_->UnAuthenticateDevice(pkgName, udid, bindLevel) != DM_OK) {
        LOGE("dmServiceImpl_ UnAuthenticateDevice failed.");
        return ERR_DM_FAILED;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::vector<std::string> peerUdids;
    peerUdids.emplace_back(udid);
    SendUnBindBroadCast(peerUdids, MultipleUserConnector::GetCurrentAccountUserID(), tokenId, bindLevel);
#endif
    return DM_OK;
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
    std::map<std::string, std::string> bindParamMap;
    std::string bindParamStr = bindParam;
    int32_t actionId = 0;
    SoftbusListener::GetActionId(queryDeviceId, actionId);
    if (actionId > 0) {
        targetId.deviceId = queryDeviceId;
        AddHmlInfoToBindParam(actionId, bindParamStr);
    } else {
        ConnectionAddrType addrType;
        int32_t ret = SoftbusListener::GetTargetInfoFromCache(queryDeviceId, targetId, addrType);
        if (ret != DM_OK) {
            LOGE("BindDevice failed, cannot get target info from cached discovered device map.");
            return ERR_DM_BIND_INPUT_PARA_INVALID;
        }
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_CONN_ADDR_TYPE, std::to_string(addrType)));
    }
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TYPE, std::to_string(authType)));
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_BIND_EXTRA_DATA, bindParamStr));
    return dmServiceImpl_->BindTarget(pkgName, targetId, bindParamMap);
}

int32_t DeviceManagerService::UnBindDevice(const std::string &pkgName, const std::string &udidHash)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call UnBindDevice.");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s, udidHash = %{public}s", pkgName.c_str(), GetAnonyString(udidHash).c_str());
    if (pkgName.empty() || udidHash.empty()) {
        LOGE("DeviceManagerService::UnBindDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnBindDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    std::string realDeviceId = udidHash;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHashTemp = "";
    if (GetUdidHashByAnoyDeviceId(udidHash, udidHashTemp) == DM_OK) {
        realDeviceId = udidHashTemp;
    }
#endif
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    std::string udid = "";
    if (softbusListener_->GetUdidFromDp(realDeviceId, udid) != DM_OK) {
        LOGE("Get udid by udidhash failed.");
        return ERR_DM_FAILED;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    uint64_t tokenId = 0;
    int32_t bindLevel = dmServiceImpl_->GetBindLevel(pkgName, std::string(localUdid), udid, tokenId);
    LOGI("UnAuthenticateDevice get bindlevel %{public}d.", bindLevel);
    if (bindLevel == INVALIED_BIND_LEVEL) {
        LOGE("UnAuthenticateDevice failed, Acl not contain the bindLevel %{public}d.", bindLevel);
        return ERR_DM_FAILED;
    }
    if (dmServiceImpl_->UnBindDevice(pkgName, udid, bindLevel) != DM_OK) {
        LOGE("dmServiceImpl_ UnBindDevice failed.");
        return ERR_DM_FAILED;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::vector<std::string> peerUdids;
    peerUdids.emplace_back(udid);
    SendUnBindBroadCast(peerUdids, MultipleUserConnector::GetCurrentAccountUserID(), tokenId, bindLevel);
#endif
    return DM_OK;
}

int32_t DeviceManagerService::UnBindDevice(const std::string &pkgName, const std::string &udidHash,
    const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call UnBindDevice.");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s, udidHash = %{public}s", pkgName.c_str(), GetAnonyString(udidHash).c_str());
    if (pkgName.empty() || udidHash.empty()) {
        LOGE("DeviceManagerService::UnBindDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnBindDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    std::string realDeviceId = udidHash;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHashTemp = "";
    if (GetUdidHashByAnoyDeviceId(udidHash, udidHashTemp) == DM_OK) {
        realDeviceId = udidHashTemp;
    }
#endif
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    std::string udid = "";
    if (softbusListener_->GetUdidFromDp(realDeviceId, udid) != DM_OK) {
        LOGE("Get udid by udidhash failed.");
        return ERR_DM_FAILED;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    uint64_t tokenId = 0;
    int32_t bindLevel = dmServiceImpl_->GetBindLevel(pkgName, std::string(localUdid), udid, tokenId);
    LOGI("UnAuthenticateDevice get bindlevel %{public}d.", bindLevel);
    if (bindLevel == INVALIED_BIND_LEVEL) {
        LOGE("UnAuthenticateDevice failed, Acl not contain the bindLevel %{public}d.", bindLevel);
        return ERR_DM_FAILED;
    }
    [[maybe_unused]] uint64_t peerTokenId = dmServiceImpl_->GetTokenIdByNameAndDeviceId(extra, udid);
    if (dmServiceImpl_->UnBindDevice(pkgName, udid, bindLevel, extra) != DM_OK) {
        LOGE("dmServiceImpl_ UnBindDevice failed.");
        return ERR_DM_FAILED;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::vector<std::string> peerUdids;
    peerUdids.emplace_back(udid);
    SendUnBindBroadCast(peerUdids, MultipleUserConnector::GetCurrentAccountUserID(), tokenId,
        bindLevel, peerTokenId);
#endif
    return DM_OK;
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
    if (IsDMServiceAdapterSoLoaded()) {
        dmServiceImplExtResident_->ReplyUiAction(pkgName, action, params);
    }
    if (!IsDMServiceImplReady()) {
        LOGE("SetUserOperation failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->SetUserOperation(pkgName, action, params);
}

void DeviceManagerService::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    LOGD("start, devState = %{public}d", devState);
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceStatusChange(devState, devInfo);
    }
    if (IsDMServiceAdapterResidentLoad()) {
        dmServiceImplExtResident_->HandleDeviceStatusChange(devState, devInfo);
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
    return PinHolderSession::OnSessionOpened(sessionId, result);
}

void DeviceManagerService::OnPinHolderSessionClosed(int sessionId)
{
    CHECK_NULL_VOID(pinHolder_);
    pinHolder_->OnSessionClosed(sessionId);
}

void DeviceManagerService::OnPinHolderBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
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
    void *so_handle = dlopen(LIB_IMPL_NAME, RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
    if (so_handle == nullptr) {
        so_handle = dlopen(LIB_IMPL_NAME, RTLD_NOW | RTLD_NODELETE);
    }
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
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }
    if (dmServiceImpl_->Initialize(listener_) != DM_OK) {
        dmServiceImpl_ = nullptr;
        isImplsoLoaded_ = false;
        return false;
    }
    isImplsoLoaded_ = true;
    LOGI("Sussess.");
    return true;
}

bool DeviceManagerService::IsDMImplSoLoaded()
{
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    return isImplsoLoaded_;
}

bool DeviceManagerService::IsDMServiceAdapterSoLoaded()
{
    std::lock_guard<std::mutex> lock(isAdapterResidentLoadLock_);
    if (!isAdapterResidentSoLoaded_ || (dmServiceImplExtResident_ == nullptr)) {
        return false;
    }
    return dmServiceImplExtResident_->IsDMServiceAdapterSoLoaded();
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
    std::vector<DmDeviceInfo> deviceList;
    CHECK_NULL_VOID(softbusListener_);
    int32_t ret = GetTrustedDeviceList(DHARD_WARE_PKG_NAME, deviceList);
    if (ret != DM_OK) {
        LOGE("LoadHardwareFwkService failed, get trusted devicelist failed.");
        return;
    }
    if (deviceList.empty()) {
        LOGI("no trusted device.");
        return;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("LoadHardwareFwkService failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->LoadHardwareFwkService();
}

int32_t DeviceManagerService::GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
    std::string &uuid)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetUuidByNetworkId(networkId.c_str(), uuid);
    if (ret != DM_OK) {
        LOGE("GetUuidByNetworkId failed, ret : %{public}d", ret);
        return ret;
    }

    std::string appId = Crypto::Sha256(AppManager::GetInstance().GetAppId());
    uuid = Crypto::Sha256(appId + "_" + uuid);
    LOGI("appId = %{public}s, uuid = %{public}s, encryptedUuid = %{public}s.", GetAnonyString(appId).c_str(),
        GetAnonyString(uuid).c_str(), GetAnonyString(uuid).c_str());
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

bool DeviceManagerService::IsDMServiceAdapterResidentLoad()
{
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }
    std::lock_guard<std::mutex> lock(isAdapterResidentLoadLock_);
    if (isAdapterResidentSoLoaded_ && (dmServiceImplExtResident_ != nullptr)) {
        return true;
    }
    residentSoHandle_ = dlopen(LIB_DM_RESIDENT_NAME, RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
    if (residentSoHandle_ == nullptr) {
        residentSoHandle_ = dlopen(LIB_DM_RESIDENT_NAME, RTLD_NOW | RTLD_NODELETE);
    }
    if (residentSoHandle_ == nullptr) {
        LOGE("load dm service resident so failed.");
        return false;
    }
    dlerror();
    auto func = (CreateDMServiceExtResidentFuncPtr)dlsym(residentSoHandle_, "CreateDMServiceExtResidentObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(residentSoHandle_);
        residentSoHandle_ = nullptr;
        LOGE("Create object function is not exist.");
        return false;
    }

    dmServiceImplExtResident_ = std::shared_ptr<IDMServiceImplExtResident>(func());
    if (dmServiceImplExtResident_->Initialize(listener_) != DM_OK) {
        dlclose(residentSoHandle_);
        residentSoHandle_ = nullptr;
        dmServiceImplExtResident_ = nullptr;
        isAdapterResidentSoLoaded_ = false;
        LOGE("dm service impl ext resident init failed.");
        return false;
    }
    isAdapterResidentSoLoaded_ = true;
    LOGI("Success.");
    return true;
}

void DeviceManagerService::UnloadDMServiceAdapterResident()
{
    LOGI("Start.");
    std::lock_guard<std::mutex> lock(isAdapterResidentLoadLock_);
    if (dmServiceImplExtResident_ != nullptr) {
        dmServiceImplExtResident_->Release();
    }
    dmServiceImplExtResident_ = nullptr;
    if (residentSoHandle_ != nullptr) {
        LOGI("dm service resident residentSoHandle_ is not nullptr.");
        dlclose(residentSoHandle_);
        residentSoHandle_ = nullptr;
    }
}

int32_t DeviceManagerService::StartDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckNewPermission() &&
        !PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
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
    if (!PermissionManager::GetInstance().CheckNewPermission() &&
        !PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
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
    if (bindParam.find(PARAM_KEY_META_TYPE) == bindParam.end()) {
        if (!IsDMServiceImplReady()) {
            LOGE("BindTarget failed, DMServiceImpl instance not init or init failed.");
            return ERR_DM_NOT_INIT;
        }
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
    if (!AppManager::GetInstance().IsSystemSA() && !AppManager::GetInstance().IsSystemApp()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("BindTarget failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    LOGI("BindTarget unstardard begin.");
    return dmServiceImplExtResident_->BindTargetExt(pkgName, targetId, bindParam);
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
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("UnbindTarget failed, instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    if (unbindParam.find(PARAM_KEY_META_TYPE) == unbindParam.end()) {
        LOGE("input unbind parameter not contains META_TYPE, dm service adapter not supported.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!AppManager::GetInstance().IsSystemSA() && !AppManager::GetInstance().IsSystemApp()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    std::string realDeviceId = targetId.deviceId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHashTemp = "";
    if (GetUdidHashByAnoyDeviceId(realDeviceId, udidHashTemp) == DM_OK) {
        realDeviceId = udidHashTemp;
    }
#endif
    std::map<std::string, std::string> unbindParamWithUdid(unbindParam);
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    std::string udid = "";
    if (softbusListener_->GetUdidFromDp(realDeviceId, udid) == DM_OK) {
        unbindParamWithUdid.insert(std::pair<std::string, std::string>(UN_BIND_PARAM_UDID_KEY, udid));
    }
    return dmServiceImplExtResident_->UnbindTargetExt(pkgName, targetId, unbindParamWithUdid);
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
bool DeviceManagerService::InitDPLocalServiceInfo(const DMLocalServiceInfo &serviceInfo,
    DistributedDeviceProfile::LocalServiceInfo &dpLocalServiceInfo)
{
    dpLocalServiceInfo.SetBundleName(serviceInfo.bundleName);
    dpLocalServiceInfo.SetAuthBoxType(serviceInfo.authBoxType);
    dpLocalServiceInfo.SetAuthType(serviceInfo.authType);
    dpLocalServiceInfo.SetPinExchangeType(serviceInfo.pinExchangeType);
    dpLocalServiceInfo.SetPinCode(serviceInfo.pinCode);
    dpLocalServiceInfo.SetDescription(serviceInfo.description);
    dpLocalServiceInfo.SetExtraInfo(serviceInfo.extraInfo);
    return true;
}

void DeviceManagerService::InitServiceInfo(const DistributedDeviceProfile::LocalServiceInfo &dpLocalServiceInfo,
    DMLocalServiceInfo &serviceInfo)
{
    serviceInfo.bundleName = dpLocalServiceInfo.GetBundleName();
    serviceInfo.authBoxType = dpLocalServiceInfo.GetAuthBoxType();
    serviceInfo.authType = dpLocalServiceInfo.GetAuthType();
    serviceInfo.pinExchangeType = dpLocalServiceInfo.GetPinExchangeType();
    serviceInfo.pinCode = dpLocalServiceInfo.GetPinCode();
    serviceInfo.description = dpLocalServiceInfo.GetDescription();
    serviceInfo.extraInfo = dpLocalServiceInfo.GetExtraInfo();
}

void DeviceManagerService::InitServiceInfos(
    const std::vector<DistributedDeviceProfile::LocalServiceInfo> &dpLocalServiceInfos,
    std::vector<DMLocalServiceInfo> &serviceInfos)
{
    for (const auto &dpInfoItem : dpLocalServiceInfos) {
        DMLocalServiceInfo infoItem;
        InitServiceInfo(dpInfoItem, infoItem);
        serviceInfos.emplace_back(infoItem);
    }
}
#endif

int32_t DeviceManagerService::RegisterLocalServiceInfo(const DMLocalServiceInfo &serviceInfo)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfo;
    bool success = InitDPLocalServiceInfo(serviceInfo, dpLocalServiceInfo);
    if (!success) {
        LOGE("InitDPLocalServiceInfo failed");
        return ERR_DM_FAILED;
    }
    return DeviceProfileConnector::GetInstance().PutLocalServiceInfo(dpLocalServiceInfo);
#else
    (void)serviceInfo;
    return ERR_DM_FAILED;
#endif
}

int32_t DeviceManagerService::UnRegisterLocalServiceInfo(const std::string &bundleName, int32_t pinExchangeType)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return DeviceProfileConnector::GetInstance().DeleteLocalServiceInfo(bundleName, pinExchangeType);
#else
    (void)bundleName;
    (void)pinExchangeType;
    return ERR_DM_FAILED;
#endif
}

int32_t DeviceManagerService::UpdateLocalServiceInfo(const DMLocalServiceInfo &serviceInfo)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfo;
    bool success = InitDPLocalServiceInfo(serviceInfo, dpLocalServiceInfo);
    if (!success) {
        LOGE("InitDPLocalServiceInfo failed");
        return ERR_DM_FAILED;
    }
    return DeviceProfileConnector::GetInstance().UpdateLocalServiceInfo(dpLocalServiceInfo);
#else
    (void)serviceInfo;
    return ERR_DM_FAILED;
#endif
}

int32_t DeviceManagerService::GetLocalServiceInfoByBundleNameAndPinExchangeType(const std::string &bundleName,
    int32_t pinExchangeType, DMLocalServiceInfo &serviceInfo)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName,
        pinExchangeType, dpLocalServiceInfo);
    if (ret == DM_OK) {
        InitServiceInfo(dpLocalServiceInfo, serviceInfo);
    }
    return ret;
#else
    (void)bundleName;
    (void)pinExchangeType;
    (void)serviceInfo;
    return ERR_DM_FAILED;
#endif
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
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(pinHolder_, ERR_DM_POINT_NULL);
    return pinHolder_->DestroyPinHolder(pkgName, targetId, pinType, payload);
}

int32_t DeviceManagerService::DpAclAdd(const std::string &udid)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call DpAclAdd.");
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("DpAclAdd failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    dmServiceImpl_->DpAclAdd(udid);
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
        return false;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("CheckAccessControl failed, instance not init or init failed.");
        return false;
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
        return false;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("CheckIsSameAccount failed, instance not init or init failed.");
        return false;
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
    AccountEventCallback callback = [=](const auto &eventType, const auto &currentUserId, const auto &beforeUserId) {
        this->AccountCommonEventCallback(eventType, currentUserId, beforeUserId);
    };
    std::vector<std::string> AccountCommonEventVec;
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_STOPPED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    if (accountCommonEventManager_->SubscribeAccountCommonEvent(AccountCommonEventVec, callback)) {
        LOGI("Success");
    }
    return;
}

void DeviceManagerService::SendShareTypeUnBindBroadCast(const char *credId, const int32_t localUserId,
    const std::vector<std::string> &peerUdids)
{
    LOGI("Start.");
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SHARE_UNBIND;
    msg.userId = static_cast<uint32_t>(localUserId);
    msg.credId = credId;
    msg.peerUdids = peerUdids;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    LOGI("broadCastMsg = %{public}s.", broadCastMsg.c_str());
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
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

DM_EXPORT void DeviceManagerService::AccountCommonEventCallback(
    const std::string commonEventType, int32_t currentUserId, int32_t beforeUserId)
{
    if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        DeviceNameManager::GetInstance().InitDeviceNameWhenUserSwitch(currentUserId, beforeUserId);
        MultipleUserConnector::SetAccountInfo(currentUserId, MultipleUserConnector::GetCurrentDMAccountInfo());
        DMCommTool::GetInstance()->StartCommonEvent(commonEventType,
            [this, commonEventType] () {
                DeviceManagerService::HandleAccountCommonEvent(commonEventType);
            });
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_HWID_LOGIN) {
        DeviceNameManager::GetInstance().InitDeviceNameWhenLogin();
        MultipleUserConnector::SetAccountInfo(currentUserId, MultipleUserConnector::GetCurrentDMAccountInfo());
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_HWID_LOGOUT) {
        DeviceNameManager::GetInstance().InitDeviceNameWhenLogout();
        DMAccountInfo dmAccountInfo = MultipleUserConnector::GetAccountInfoByUserId(beforeUserId);
        if (dmAccountInfo.accountId.empty()) {
            return;
        }
        HandleAccountLogout(currentUserId, dmAccountInfo.accountId, dmAccountInfo.accountName);
        MultipleUserConnector::DeleteAccountInfoByUserId(currentUserId);
        MultipleUserConnector::SetAccountInfo(MultipleUserConnector::GetCurrentAccountUserID(),
            MultipleUserConnector::GetCurrentDMAccountInfo());
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        HandleUserRemoved(beforeUserId);
        MultipleUserConnector::DeleteAccountInfoByUserId(beforeUserId);
        MultipleUserConnector::SetAccountInfo(MultipleUserConnector::GetCurrentAccountUserID(),
            MultipleUserConnector::GetCurrentDMAccountInfo());
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED) {
        DeviceNameManager::GetInstance().InitDeviceNameWhenNickChange();
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_STOPPED && IsPC()) {
        DMCommTool::GetInstance()->StartCommonEvent(commonEventType,
            [this, commonEventType] () {
                DeviceManagerService::HandleAccountCommonEvent(commonEventType);
            });
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        DeviceNameManager::GetInstance().AccountSysReady(beforeUserId);
        DMCommTool::GetInstance()->StartCommonEvent(commonEventType,
            [this, commonEventType] () {
                DeviceManagerService::HandleAccountCommonEvent(commonEventType);
            });
    } else {
        LOGE("Invalied account common event.");
    }
    return;
}

bool DeviceManagerService::IsUserStatusChanged(std::vector<int32_t> foregroundUserVec,
    std::vector<int32_t> backgroundUserVec)
{
    LOGI("foregroundUserVec: %{public}s, backgroundUserVec: %{public}s",
        GetIntegerList(foregroundUserVec).c_str(), GetIntegerList(backgroundUserVec).c_str());
    std::lock_guard<std::mutex> lock(userVecLock_);
    std::sort(foregroundUserVec.begin(), foregroundUserVec.end());
    std::sort(backgroundUserVec.begin(), backgroundUserVec.end());
    std::sort(foregroundUserVec_.begin(), foregroundUserVec_.end());
    std::sort(backgroundUserVec_.begin(), backgroundUserVec_.end());
    if (foregroundUserVec == foregroundUserVec_ && backgroundUserVec == backgroundUserVec_) {
        LOGI("User status has not changed.");
        return true;
    }
    LOGD("User status has changed");
    foregroundUserVec_.clear();
    backgroundUserVec_.clear();
    foregroundUserVec_ = foregroundUserVec;
    backgroundUserVec_ = backgroundUserVec;
    return false;
}

void DeviceManagerService::HandleAccountCommonEvent(const std::string commonEventType)
{
    LOGI("commonEventType: %{public}s.", commonEventType.c_str());
    std::vector<int32_t> foregroundUserVec;
    int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec);
    std::vector<int32_t> backgroundUserVec;
    int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec);
    MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
    if (retFront != DM_OK || retBack != DM_OK) {
        LOGE("retFront: %{public}d, retBack: %{public}d, frontuserids: %{public}s, backuserids: %{public}s",
            retFront, retBack, GetIntegerList(foregroundUserVec).c_str(), GetIntegerList(backgroundUserVec).c_str());
        return;
    }
    if (IsUserStatusChanged(foregroundUserVec, backgroundUserVec)) {
        LOGI("User status has not changed.");
        return;
    }
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    if (!discoveryMgr_->GetCommonDependencyObj()->CheckAclStatusAndForegroundNotMatch(localUdid, foregroundUserVec,
        backgroundUserVec)) {
        LOGI("no unreasonable data.");
        return;
    }
    std::map<std::string, int32_t> curUserDeviceMap =
        discoveryMgr_->GetCommonDependencyObj()->GetDeviceIdAndBindLevel(foregroundUserVec, localUdid);
    std::map<std::string, int32_t> preUserDeviceMap =
        discoveryMgr_->GetCommonDependencyObj()->GetDeviceIdAndBindLevel(backgroundUserVec, localUdid);
    std::vector<std::string> peerUdids;
    for (const auto &item : curUserDeviceMap) {
        peerUdids.push_back(item.first);
    }
    for (const auto &item : preUserDeviceMap) {
        if (find(peerUdids.begin(), peerUdids.end(), item.first) == peerUdids.end()) {
            peerUdids.push_back(item.first);
        }
    }
    if (peerUdids.empty()) {
        return;
    }
    NotifyRemoteAccountCommonEvent(commonEventType, localUdid, peerUdids, foregroundUserVec, backgroundUserVec);
}

void DeviceManagerService::HandleUserSwitched()
{
    LOGI("start.");
    std::vector<int32_t> foregroundUserVec;
    int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec);
    std::vector<int32_t> backgroundUserVec;
    int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec);
    if (retFront != DM_OK || retBack != DM_OK || foregroundUserVec.empty()) {
        LOGE("Get userids failed, retFront: %{public}d, retBack: %{public}d, foreground user num: %{public}d",
            retFront, retBack, static_cast<int32_t>(foregroundUserVec.size()));
        return;
    }
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    if (!discoveryMgr_->GetCommonDependencyObj()->CheckAclStatusAndForegroundNotMatch(localUdid,
        foregroundUserVec, backgroundUserVec)) {
        LOGI("no unreasonable data.");
        return;
    }
    std::map<std::string, int32_t> curUserDeviceMap =
        discoveryMgr_->GetCommonDependencyObj()->GetDeviceIdAndBindLevel(foregroundUserVec, localUdid);
    std::map<std::string, int32_t> preUserDeviceMap =
        discoveryMgr_->GetCommonDependencyObj()->GetDeviceIdAndBindLevel(backgroundUserVec, localUdid);
    std::vector<std::string> peerUdids;
    for (const auto &item : curUserDeviceMap) {
        peerUdids.push_back(item.first);
    }
    for (const auto &item : preUserDeviceMap) {
        if (find(peerUdids.begin(), peerUdids.end(), item.first) == peerUdids.end()) {
            peerUdids.push_back(item.first);
        }
    }
    if (peerUdids.empty()) {
        return;
    }
    NotifyRemoteLocalUserSwitch(localUdid, peerUdids, foregroundUserVec, backgroundUserVec);
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::NotifyRemoteAccountCommonEvent(const std::string commonEventType,
    const std::string &localUdid, const std::vector<std::string> &peerUdids,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("foregroundUserIds: %{public}s, backgroundUserIds: %{public}s",
        GetIntegerList<int32_t>(foregroundUserIds).c_str(), GetIntegerList<int32_t>(backgroundUserIds).c_str());
    if (peerUdids.empty()) {
        return;
    }
    if (softbusListener_ == nullptr) {
        UpdateAcl(localUdid, peerUdids, foregroundUserIds, backgroundUserIds);
        LOGE("softbusListener_ is null");
        return;
    }
    std::vector<std::string> bleUdids;
    std::map<std::string, std::string> wifiDevices;
    for (const auto &udid : peerUdids) {
        std::string netWorkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, netWorkId);
        if (netWorkId.empty()) {
            LOGI("netWorkId is empty: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        int32_t networkType = 0;
        if (softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType) != DM_OK || networkType <= 0) {
            LOGI("get networkType failed: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        uint32_t addrTypeMask = 1 << NetworkType::BIT_NETWORK_TYPE_BLE;
        if ((static_cast<uint32_t>(networkType) & addrTypeMask) != 0x0) {
            bleUdids.push_back(udid);
        } else {
            wifiDevices.insert(std::pair<std::string, std::string>(udid, netWorkId));
        }
    }
    if (!bleUdids.empty()) {
        UpdateAcl(localUdid, peerUdids, foregroundUserIds, backgroundUserIds);
        if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_UNLOCKED ||
            commonEventType == CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
            SendCommonEventBroadCast(bleUdids, foregroundUserIds, backgroundUserIds, true);
        } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_STOPPED) {
            SendCommonEventBroadCast(bleUdids, foregroundUserIds, backgroundUserIds, false);
        } else {
            LOGE("commonEventType not match");
            return;
        }
    }
    if (!wifiDevices.empty()) {
        NotifyRemoteAccountCommonEventByWifi(localUdid, wifiDevices, foregroundUserIds, backgroundUserIds);
    }
}

void DeviceManagerService::NotifyRemoteAccountCommonEventByWifi(const std::string &localUdid,
    const std::map<std::string, std::string> &wifiDevices, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    for (const auto &it : wifiDevices) {
        int32_t result = SendAccountCommonEventByWifi(it.second, foregroundUserIds, backgroundUserIds);
        if (result != DM_OK) {
            LOGE("by wifi failed: %{public}s", GetAnonyString(it.first).c_str());
            std::vector<std::string> updateUdids;
            updateUdids.push_back(it.first);
            UpdateAcl(localUdid, updateUdids, foregroundUserIds, backgroundUserIds);
            continue;
        }
        if (timer_ == nullptr) {
            timer_ = std::make_shared<DmTimer>();
        }
        std::string udid = it.first;
        timer_->StartTimer(std::string(ACCOUNT_COMMON_EVENT_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(udid),
            USER_SWITCH_BY_WIFI_TIMEOUT_S,
            [this, localUdid, foregroundUserIds, backgroundUserIds, udid] (std::string name) {
                DeviceManagerService::HandleCommonEventTimeout(localUdid, foregroundUserIds, backgroundUserIds, udid);
            });
    }
}

int32_t DeviceManagerService::SendAccountCommonEventByWifi(const std::string &networkId,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("start");
    std::vector<uint32_t> foregroundUserIdsUInt;
    for (auto const &u : foregroundUserIds) {
        foregroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    std::vector<uint32_t> backgroundUserIdsUInt;
    for (auto const &u : backgroundUserIds) {
        backgroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    return DMCommTool::GetInstance()->SendUserIds(networkId, foregroundUserIdsUInt, backgroundUserIdsUInt);
}

void DeviceManagerService::HandleCommonEventTimeout(const std::string &localUdid,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds,
    const std::string &udid)
{
    LOGI("start udid: %{public}s", GetAnonyString(udid).c_str());
    std::vector<std::string> updateUdids;
    updateUdids.push_back(udid);
    UpdateAcl(localUdid, updateUdids, foregroundUserIds, backgroundUserIds);
}

void DeviceManagerService::UpdateAcl(const std::string &localUdid,
    const std::vector<std::string> &peerUdids, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    discoveryMgr_->GetCommonDependencyObj()->HandleAccountCommonEvent(localUdid, peerUdids, foregroundUserIds,
        backgroundUserIds);
}

void DeviceManagerService::NotifyRemoteLocalUserSwitch(const std::string &localUdid,
    const std::vector<std::string> &peerUdids, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("onstart UserSwitch, foregroundUserIds: %{public}s, backgroundUserIds: %{public}s",
        GetIntegerList<int32_t>(foregroundUserIds).c_str(), GetIntegerList<int32_t>(backgroundUserIds).c_str());
    if (peerUdids.empty()) {
        return;
    }
    if (softbusListener_ == nullptr) {
        UpdateAclAndDeleteGroup(localUdid, peerUdids, foregroundUserIds, backgroundUserIds);
        LOGE("softbusListener_ is null");
        return;
    }
    std::vector<std::string> bleUdids;
    std::map<std::string, std::string> wifiDevices;
    for (const auto &udid : peerUdids) {
        std::string netWorkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, netWorkId);
        if (netWorkId.empty()) {
            LOGI("netWorkId is empty: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        int32_t networkType = 0;
        int32_t ret = softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType);
        if (ret != DM_OK || networkType <= 0) {
            LOGI("get networkType failed: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        uint32_t addrTypeMask = 1 << NetworkType::BIT_NETWORK_TYPE_BLE;
        if ((static_cast<uint32_t>(networkType) & addrTypeMask) != 0x0) {
            bleUdids.push_back(udid);
        } else {
            wifiDevices.insert(std::pair<std::string, std::string>(udid, netWorkId));
        }
    }
    if (!bleUdids.empty()) {
        UpdateAclAndDeleteGroup(localUdid, peerUdids, foregroundUserIds, backgroundUserIds);
        SendUserIdsBroadCast(bleUdids, foregroundUserIds, backgroundUserIds, true);
    }
    if (!wifiDevices.empty()) {
        NotifyRemoteLocalUserSwitchByWifi(localUdid, wifiDevices, foregroundUserIds, backgroundUserIds);
    }
}

void DeviceManagerService::NotifyRemoteLocalUserSwitchByWifi(const std::string &localUdid,
    const std::map<std::string, std::string> &wifiDevices, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    for (const auto &it : wifiDevices) {
        int32_t result = SendUserIdsByWifi(it.second, foregroundUserIds, backgroundUserIds);
        if (result != DM_OK) {
            LOGE("by wifi failed: %{public}s", GetAnonyString(it.first).c_str());
            std::vector<std::string> updateUdids;
            updateUdids.push_back(it.first);
            UpdateAclAndDeleteGroup(localUdid, updateUdids, foregroundUserIds, backgroundUserIds);
            continue;
        }
        if (timer_ == nullptr) {
            timer_ = std::make_shared<DmTimer>();
        }
        std::string udid = it.first;
        timer_->StartTimer(std::string(USER_SWITCH_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(udid),
            USER_SWITCH_BY_WIFI_TIMEOUT_S,
            [this, localUdid, foregroundUserIds, backgroundUserIds, udid] (std::string name) {
                DeviceManagerService::HandleUserSwitchTimeout(localUdid, foregroundUserIds, backgroundUserIds, udid);
            });
    }
}

void DeviceManagerService::HandleUserSwitchTimeout(const std::string &localUdid,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds,
    const std::string &udid)
{
    LOGI("start udid: %{public}s", GetAnonyString(udid).c_str());
    std::vector<std::string> updateUdids;
    updateUdids.push_back(udid);
    UpdateAclAndDeleteGroup(localUdid, updateUdids, foregroundUserIds, backgroundUserIds);
}

void DeviceManagerService::UpdateAclAndDeleteGroup(const std::string &localUdid,
    const std::vector<std::string> &deviceVec, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    discoveryMgr_->GetCommonDependencyObj()->HandleUserSwitched(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds);
}
#endif

void DeviceManagerService::HandleAccountLogout(int32_t userId, const std::string &accountId,
    const std::string &accountName)
{
    LOGI("UserId: %{public}d, accountId: %{public}s, accountName: %{public}s", userId,
        GetAnonyString(accountId).c_str(), GetAnonyString(accountName).c_str());
    if (IsDMServiceAdapterResidentLoad()) {
        dmServiceImplExtResident_->ClearCacheWhenLogout(userId, accountId);
    } else {
        LOGW("ClearCacheWhenLogout fail, adapter instance not init or init failed.");
    }
    if (!IsDMServiceImplReady()) {
        LOGE("Init impl failed.");
        return;
    }
    std::multimap<std::string, int32_t> deviceMap;
    std::vector<std::string> peerUdids;
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    deviceMap = dmServiceImpl_->GetDeviceIdAndUserId(userId, accountId);
    for (const auto &item : deviceMap) {
        peerUdids.emplace_back(item.first);
    }
    if (!peerUdids.empty()) {
        //logout notify cast+
        if (IsDMServiceAdapterResidentLoad()) {
            dmServiceImplExtResident_->AccountIdLogout(userId, accountId, peerUdids);
        }
        char accountIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetAccountIdHash(accountId, reinterpret_cast<uint8_t *>(accountIdHash)) != DM_OK) {
            LOGE("GetAccountHash failed.");
            return;
        }
        NotifyRemoteLocalLogout(peerUdids, std::string(accountIdHash), accountName, userId);
    }
    for (const auto &item : deviceMap) {
        dmServiceImpl_->HandleIdentAccountLogout(localUdid, userId, item.first, item.second);
    }
}

void DeviceManagerService::HandleUserSwitched(int32_t curUserId, int32_t preUserId)
{
    LOGI("currentUserId: %{public}d. previousUserId: %{public}d", curUserId, preUserId);
    if (!IsDMServiceImplReady()) {
        LOGE("Init impl failed.");
        return;
    }
    std::map<std::string, int32_t> curUserDeviceMap;
    std::map<std::string, int32_t> preUserDeviceMap;
    std::vector<std::string> peerUdids;
    curUserDeviceMap = dmServiceImpl_->GetDeviceIdAndBindLevel(curUserId);
    preUserDeviceMap = dmServiceImpl_->GetDeviceIdAndBindLevel(preUserId);
    for (const auto &item : curUserDeviceMap) {
        peerUdids.push_back(item.first);
    }
    for (const auto &item : preUserDeviceMap) {
        if (find(peerUdids.begin(), peerUdids.end(), item.first) == peerUdids.end()) {
            peerUdids.push_back(item.first);
        }
    }
    if (peerUdids.empty()) {
        return;
    }
    std::vector<int32_t> foregroundUserVec;
    int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec);
    std::vector<int32_t> backgroundUserVec;
    int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec);
    if (retFront != DM_OK || retBack != DM_OK || foregroundUserVec.empty()) {
        LOGE("Get userids failed, retFront: %{public}d, retBack: %{public}d, foreground user num: %{public}d",
            retFront, retBack, static_cast<int32_t>(foregroundUserVec.size()));
        return;
    }
    NotifyRemoteLocalUserSwitch(curUserId, preUserId, peerUdids, foregroundUserVec, backgroundUserVec);
}

void DeviceManagerService::HandleUserRemoved(int32_t removedUserId)
{
    LOGI("PreUserId %{public}d.", removedUserId);
    if (!IsDMServiceImplReady()) {
        LOGE("Init impl failed.");
        return;
    }
    std::multimap<std::string, int32_t> deviceMap = dmServiceImpl_->GetDeviceIdAndUserId(removedUserId);
    std::vector<std::string> peerUdids;
    for (const auto &item : deviceMap) {
        if (find(peerUdids.begin(), peerUdids.end(), item.first) == peerUdids.end()) {
            peerUdids.emplace_back(item.first);
        }
    }
    if (!peerUdids.empty()) {
        // Send UserId Removed broadcast
        SendUserRemovedBroadCast(peerUdids, removedUserId);
        dmServiceImpl_->HandleUserRemoved(peerUdids, removedUserId);
    }
}

void DeviceManagerService::SendUserRemovedBroadCast(const std::vector<std::string> &peerUdids, int32_t userId)
{
    LOGI("peerUdids: %{public}s, userId %{public}d.", GetAnonyStringList(peerUdids).c_str(), userId);
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::DEL_USER;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::SendAccountLogoutBroadCast(const std::vector<std::string> &peerUdids,
    const std::string &accountId, const std::string &accountName, int32_t userId)
{
    LOGI("accountId %{public}s, accountName %{public}s, userId %{public}d.", GetAnonyString(accountId).c_str(),
        GetAnonyString(accountName).c_str(), userId);
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    msg.accountId = accountId;
    msg.accountName = accountName;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::SendUserIdsBroadCast(const std::vector<std::string> &peerUdids,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds, bool isNeedResponse)
{
    LOGI("peerUdids: %{public}s, foregroundUserIds: %{public}s, backgroundUserIds: %{public}s, isNeedRsp: %{public}s",
        GetAnonyStringList(peerUdids).c_str(), GetIntegerList<int32_t>(foregroundUserIds).c_str(),
        GetIntegerList<int32_t>(backgroundUserIds).c_str(), isNeedResponse ? "true" : "false");
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SYNC_USERID;
    msg.peerUdids = peerUdids;
    msg.syncUserIdFlag = isNeedResponse;
    for (const auto &userId : foregroundUserIds) {
        msg.userIdInfos.push_back({ true, static_cast<uint16_t>(userId) });
    }
    for (auto const &userId : backgroundUserIds) {
        msg.userIdInfos.push_back({ false, static_cast<uint16_t>(userId) });
    }
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::HandleUserIdsBroadCast(const std::vector<UserIdInfo> &remoteUserIdInfos,
    const std::string &remoteUdid, bool isNeedResponse)
{
    LOGI("rmtUdid: %{public}s, rmtUserIds: %{public}s, isNeedResponse: %{public}s,",
        GetAnonyString(remoteUdid).c_str(), GetUserIdInfoList(remoteUserIdInfos).c_str(),
        isNeedResponse ? "true" : "false");
    if (isNeedResponse) {
        std::vector<int32_t> foregroundUserVec;
        std::vector<int32_t> backgroundUserVec;
        int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec);
        int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec);
        if (IsPC()) {
            MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
        }
        if (retFront != DM_OK || retBack!= DM_OK) {
            LOGE("Get userid failed, retFront: %{public}d, retBack: %{public}d, frontUserNum:%{public}d,"
                 "backUserNum: %{public}d", retFront, retBack, static_cast<int32_t>(foregroundUserVec.size()),
                 static_cast<int32_t>(backgroundUserVec.size()));
        } else {
            LOGE("Send back local frontuserids: %{public}s, backuserids: %{public}s",
                GetIntegerList(foregroundUserVec).c_str(), GetIntegerList(backgroundUserVec).c_str());
            std::vector<std::string> remoteUdids = { remoteUdid };
            SendUserIdsBroadCast(remoteUdids, foregroundUserVec, backgroundUserVec, false);
        }
    }

    std::vector<UserIdInfo> foregroundUserIdInfos;
    std::vector<UserIdInfo> backgroundUserIdInfos;
    GetFrontAndBackUserIdInfos(remoteUserIdInfos, foregroundUserIdInfos, backgroundUserIdInfos);
    // Notify received remote foreground userids to dsoftbus
    std::vector<uint32_t> foregroundUserIds;
    for (const auto &u : foregroundUserIdInfos) {
        foregroundUserIds.push_back(static_cast<uint32_t>(u.userId));
    }
    std::vector<uint32_t> backgroundUserIds;
    for (const auto &u : backgroundUserIdInfos) {
        backgroundUserIds.push_back(static_cast<uint32_t>(u.userId));
    }
    if (softbusListener_ != nullptr) {
        softbusListener_->SetForegroundUserIdsToDSoftBus(remoteUdid, foregroundUserIds);
    }

    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, IsPC());
    }
}

void DeviceManagerService::ProcessSyncUserIds(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid)
{
    LOGI("process sync foregroundUserIds: %{public}s, backgroundUserIds: %{public}s, remote udid: %{public}s",
        GetIntegerList<uint32_t>(foregroundUserIds).c_str(), GetIntegerList<uint32_t>(backgroundUserIds).c_str(),
        GetAnonyString(remoteUdid).c_str());

    if (softbusListener_ != nullptr) {
        softbusListener_->SetForegroundUserIdsToDSoftBus(remoteUdid, foregroundUserIds);
    }
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(USER_SWITCH_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(remoteUdid));
    }
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, IsPC());
    }
}

void DeviceManagerService::SendCommonEventBroadCast(const std::vector<std::string> &peerUdids,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds, bool isNeedResponse)
{
    LOGI("peerUdids: %{public}s, foregroundUserIds: %{public}s, backgroundUserIds: %{public}s, isNeedRsp: %{public}s",
        GetAnonyStringList(peerUdids).c_str(), GetIntegerList<int32_t>(foregroundUserIds).c_str(),
        GetIntegerList<int32_t>(backgroundUserIds).c_str(), isNeedResponse ? "true" : "false");
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SYNC_USERID;
    msg.peerUdids = peerUdids;
    msg.syncUserIdFlag = isNeedResponse;
    msg.isNewEvent = true;
    for (const auto &userId : foregroundUserIds) {
        msg.userIdInfos.push_back({ true, static_cast<uint16_t>(userId) });
    }
    for (auto const &userId : backgroundUserIds) {
        msg.userIdInfos.push_back({ false, static_cast<uint16_t>(userId) });
    }
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::HandleCommonEventBroadCast(const std::vector<UserIdInfo> &remoteUserIdInfos,
    const std::string &remoteUdid, bool isNeedResponse)
{
    LOGI("rmtUdid: %{public}s, rmtUserIds: %{public}s, isNeedResponse: %{public}s,",
        GetAnonyString(remoteUdid).c_str(), GetUserIdInfoList(remoteUserIdInfos).c_str(),
        isNeedResponse ? "true" : "false");
    if (isNeedResponse) {
        std::vector<int32_t> foregroundUserVec;
        int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec);
        std::vector<int32_t> backgroundUserVec;
        int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec);
        MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
        if (retFront != DM_OK || retBack!= DM_OK) {
            LOGE("retFront: %{public}d, retBack: %{public}d, frontuserids: %{public}s, backuserids: %{public}s",
                retFront, retBack, GetIntegerList(foregroundUserVec).c_str(),
                GetIntegerList(backgroundUserVec).c_str());
        } else {
            LOGE("Send back local frontuserids: %{public}s, backuserids: %{public}s",
                GetIntegerList(foregroundUserVec).c_str(), GetIntegerList(backgroundUserVec).c_str());
            std::vector<std::string> remoteUdids = { remoteUdid };
            SendCommonEventBroadCast(remoteUdids, foregroundUserVec, backgroundUserVec, false);
        }
    }

    std::vector<UserIdInfo> foregroundUserIdInfos;
    std::vector<UserIdInfo> backgroundUserIdInfos;
    GetFrontAndBackUserIdInfos(remoteUserIdInfos, foregroundUserIdInfos, backgroundUserIdInfos);
    // Notify received remote foreground userids to dsoftbus
    std::vector<uint32_t> foregroundUserIds;
    for (const auto &u : foregroundUserIdInfos) {
        foregroundUserIds.push_back(static_cast<uint32_t>(u.userId));
    }
    std::vector<uint32_t> backgroundUserIds;
    for (const auto &u : backgroundUserIdInfos) {
        backgroundUserIds.push_back(static_cast<uint32_t>(u.userId));
    }
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleCommonEventBroadCast(foregroundUserIds, backgroundUserIds, remoteUdid);
    }
}

void DeviceManagerService::ProcessCommonUserStatusEvent(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid)
{
    LOGI("foregroundUserIds: %{public}s, backgroundUserIds: %{public}s, remoteUdid: %{public}s",
        GetIntegerList<uint32_t>(foregroundUserIds).c_str(), GetIntegerList<uint32_t>(backgroundUserIds).c_str(),
        GetAnonyString(remoteUdid).c_str());

    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(ACCOUNT_COMMON_EVENT_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(remoteUdid));
    }
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleCommonEventBroadCast(foregroundUserIds, backgroundUserIds, remoteUdid);
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
    JsonObject msgJsonObj(msg);
    if (msgJsonObj.IsDiscarded()) {
        LOGE("HandleDeviceNotTrust msg prase error.");
        return;
    }
    if (!IsString(msgJsonObj, NETWORKID)) {
        LOGE("HandleDeviceNotTrust msg not contain networkId.");
        return;
    }
    std::string networkId = msgJsonObj[NETWORKID].Get<std::string>();
    std::string udid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(networkId.c_str(), udid);
    LOGI("NetworkId: %{public}s, udid: %{public}s.",
        GetAnonyString(networkId).c_str(), GetAnonyString(udid).c_str());
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceNotTrust(udid);
    }
    if (IsDMServiceAdapterResidentLoad()) {
        dmServiceImplExtResident_->HandleDeviceNotTrust(udid);
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
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("SetDnPolicy failed, instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExtResident_->SetDnPolicy(policyStrategy, timeOut);
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
DM_EXPORT void DeviceManagerService::ConvertUdidHashToAnoyDeviceId(
    DmDeviceInfo &deviceInfo)
{
    std::string udidHashTemp = "";
    if (ConvertUdidHashToAnoyDeviceId(deviceInfo.deviceId, udidHashTemp) == DM_OK) {
        if (memset_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN) != DM_OK) {
            LOGE("ConvertUdidHashToAnoyDeviceId memset_s failed.");
            return;
        }
        if (memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udidHashTemp.c_str(), udidHashTemp.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(udidHashTemp).c_str());
            return;
        }
    }
}

DM_EXPORT int32_t DeviceManagerService::ConvertUdidHashToAnoyDeviceId(
    const std::string &udidHash, std::string &result)
{
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

DM_EXPORT int32_t DeviceManagerService::GetUdidHashByAnoyDeviceId(
    const std::string &anoyDeviceId, std::string &udidHash)
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

void DeviceManagerService::SendUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId,
    uint64_t tokenId, int32_t bindLevel)
{
    LOGI("TokenId %{public}" PRId64", bindLevel %{public}d, userId %{public}d.", tokenId, bindLevel, userId);
    if (static_cast<uint32_t>(bindLevel) == USER) {
        SendDeviceUnBindBroadCast(peerUdids, userId);
        return;
    }
    if (static_cast<uint32_t>(bindLevel) == APP) {
        SendAppUnBindBroadCast(peerUdids, userId, tokenId);
        return;
    }
    if (static_cast<uint32_t>(bindLevel) == SERVICE) {
        SendServiceUnBindBroadCast(peerUdids, userId, tokenId);
        return;
    }
}

void DeviceManagerService::SendUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId,
    uint64_t tokenId, int32_t bindLevel, uint64_t peerTokenId)
{
    if (static_cast<uint32_t>(bindLevel) == USER) {
        SendDeviceUnBindBroadCast(peerUdids, userId);
        return;
    }
    if (static_cast<uint32_t>(bindLevel) == APP) {
        SendAppUnBindBroadCast(peerUdids, userId, tokenId, peerTokenId);
        return;
    }
    if (static_cast<uint32_t>(bindLevel) == SERVICE) {
        SendServiceUnBindBroadCast(peerUdids, userId, tokenId);
        return;
    }
}

void DeviceManagerService::SendDeviceUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::DEVICE_UNBIND;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::SendAppUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId,
    uint64_t tokenId)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    msg.tokenId = tokenId;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::SendAppUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId,
    uint64_t tokenId, uint64_t peerTokenId)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    msg.tokenId = tokenId;
    msg.peerTokenId = peerTokenId;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::SendServiceUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId,
    uint64_t tokenId)
{
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SERVICE_UNBIND;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    msg.tokenId = tokenId;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::HandleCredentialDeleted(const char *credId, const char *credInfo)
{
    LOGI("start.");
    if (credId == nullptr || credInfo == nullptr) {
        LOGE("HandleCredentialDeleted credId or credInfo is nullptr.");
        return;
    }
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    if (!IsDMServiceImplReady()) {
        LOGE("HandleCredentialDeleted failed, instance not init or init failed.");
        return;
    }
    std::string remoteUdid = "";
    dmServiceImpl_->HandleCredentialDeleted(credId, credInfo, localUdid, remoteUdid);
    if (remoteUdid.empty()) {
        LOGE("HandleCredentialDeleted failed, remoteUdid is empty.");
        return;
    }
    std::vector<std::string> peerUdids;
    peerUdids.emplace_back(remoteUdid);
    SendShareTypeUnBindBroadCast(credId, MultipleUserConnector::GetCurrentAccountUserID(), peerUdids);
    return;
}

void DeviceManagerService::HandleDeviceTrustedChange(const std::string &msg)
{
    if (!IsMsgEmptyAndDMServiceImplReady(msg)) {
        return;
    }
    RelationShipChangeMsg relationShipMsg = ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    LOGI("Receive trust change msg: %{public}s", relationShipMsg.ToString().c_str());
    if (!ReleationShipSyncMgr::GetInstance().IsNewBroadCastId(relationShipMsg)) {
        LOGI("is not new broadcast");
        return;
    }
    bool ret = ParseRelationShipChangeType(relationShipMsg);
    if (!ret) {
        LOGI("ParseRelationShipChangeType failed");
        return;
    }
    return;
}

bool DeviceManagerService::ParseRelationShipChangeType(const RelationShipChangeMsg &relationShipMsg)
{
    switch (relationShipMsg.type) {
        case RelationShipChangeType::ACCOUNT_LOGOUT:
            dmServiceImpl_->HandleAccountLogoutEvent(relationShipMsg.userId, relationShipMsg.accountId,
                relationShipMsg.peerUdid);
            break;
        case RelationShipChangeType::DEVICE_UNBIND:
            dmServiceImpl_->HandleDevUnBindEvent(relationShipMsg.userId, relationShipMsg.peerUdid);
            break;
        case RelationShipChangeType::APP_UNBIND:
            if (relationShipMsg.peerTokenId != 0) {
                    dmServiceImpl_->HandleAppUnBindEvent(relationShipMsg.userId, relationShipMsg.peerUdid,
                        static_cast<int32_t>(relationShipMsg.tokenId),
                        static_cast<int32_t>(relationShipMsg.peerTokenId));
            } else {
                dmServiceImpl_->HandleAppUnBindEvent(relationShipMsg.userId, relationShipMsg.peerUdid,
                    static_cast<int32_t>(relationShipMsg.tokenId));
            }
            break;
        case RelationShipChangeType::SERVICE_UNBIND:
            dmServiceImpl_->HandleServiceUnBindEvent(relationShipMsg.userId, relationShipMsg.peerUdid,
                static_cast<int32_t>(relationShipMsg.tokenId));
            break;
        case RelationShipChangeType::SYNC_USERID:
            if (relationShipMsg.isNewEvent) {
                HandleCommonEventBroadCast(relationShipMsg.userIdInfos,
                    relationShipMsg.peerUdid, relationShipMsg.syncUserIdFlag);
            } else {
                HandleUserIdsBroadCast(relationShipMsg.userIdInfos,
                    relationShipMsg.peerUdid, relationShipMsg.syncUserIdFlag);
            }
            break;
        case RelationShipChangeType::DEL_USER:
            dmServiceImpl_->HandleRemoteUserRemoved(relationShipMsg.userId, relationShipMsg.peerUdid);
            break;
        case RelationShipChangeType::STOP_USER:
            HandleUserStopBroadCast(relationShipMsg.userId, relationShipMsg.peerUdid);
            break;
        case RelationShipChangeType::SHARE_UNBIND:
            HandleShareUnbindBroadCast(relationShipMsg.userId, relationShipMsg.credId);
            break;
        default:
            LOGI("Dm have not this event type.");
            return false;
    }
    return true;
}

bool DeviceManagerService::IsMsgEmptyAndDMServiceImplReady(const std::string &msg)
{
    if (msg.empty()) {
        LOGE("Msg is empty.");
        return false;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("Imp instance not init or init failed.");
        return false;
    }
    return true;
}

void DeviceManagerService::HandleShareUnbindBroadCast(const int32_t userId, const std::string &credId)
{
    LOGI("start.");
    if (credId == "") {
        LOGE("HandleShareUnbindBroadCast credId is null.");
        return;
    }
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleShareUnbindBroadCast(credId, userId, localUdid);
    }
    return;
}

int32_t DeviceManagerService::ParseCheckSumMsg(const std::string &msg, std::string &networkId, uint32_t &discoveryType,
    bool &isChange)
{
    JsonObject msgJsonObj(msg);
    if (msgJsonObj.IsDiscarded()) {
        LOGE("msg prase error.");
        return ERR_DM_FAILED;
    }
    if (!IsString(msgJsonObj, USERID_CHECKSUM_NETWORKID_KEY)) {
        LOGE("msg not contain networkId.");
        return ERR_DM_FAILED;
    }
    if (!IsUint32(msgJsonObj, USERID_CHECKSUM_DISCOVER_TYPE_KEY)) {
        LOGE("msg not contain discoveryType.");
        return ERR_DM_FAILED;
    }
    if (!IsBool(msgJsonObj, USERID_CHECKSUM_ISCHANGE_KEY)) {
        LOGE("msg not contain ischange.");
        return ERR_DM_FAILED;
    }
    networkId = msgJsonObj[USERID_CHECKSUM_NETWORKID_KEY].Get<std::string>();
    discoveryType = msgJsonObj[USERID_CHECKSUM_DISCOVER_TYPE_KEY].Get<uint32_t>();
    isChange = msgJsonObj[USERID_CHECKSUM_ISCHANGE_KEY].Get<bool>();
    return DM_OK;
}

void DeviceManagerService::ProcessCheckSumByWifi(std::string networkId, std::vector<int32_t> foregroundUserIds,
    std::vector<int32_t> backgroundUserIds)
{
    if (localNetWorkId_ == "") {
        DmDeviceInfo deviceInfo;
        SoftbusCache::GetInstance().GetLocalDeviceInfo(deviceInfo);
        localNetWorkId_ = std::string(deviceInfo.networkId);
    }
    if (localNetWorkId_ >= networkId) {
        LOGI("Local networkid big than remote, no need begin req");
        return;
    }
    // use connection to exchange foreground/background userid
    std::vector<uint32_t> foregroundUserIdsUInt;
    for (auto const &u : foregroundUserIds) {
        foregroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    std::vector<uint32_t> backgroundUserIdsUInt;
    for (auto const &u : backgroundUserIds) {
        backgroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    DMCommTool::GetInstance()->SendUserIds(networkId, foregroundUserIdsUInt, backgroundUserIdsUInt);
}

void DeviceManagerService::ProcessCheckSumByBT(std::string networkId, std::vector<int32_t> foregroundUserIds,
    std::vector<int32_t> backgroundUserIds)
{
    LOGI("Try send brodcast to exchange foreground userid");
    std::string udid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(networkId.c_str(), udid);
    if (udid.empty()) {
        LOGE("Can not get udid for networkid: %{public}s", GetAnonyString(networkId).c_str());
        return;
    }

    std::vector<std::string> peerUdids = { udid };
    if (!foregroundUserIds.empty()) {
        LOGI("Send local foreground and background userids");
        SendUserIdsBroadCast(peerUdids, foregroundUserIds, backgroundUserIds, true);
    } else {
        LOGE("local foreground userids empty");
    }
}

void DeviceManagerService::HandleUserIdCheckSumChange(const std::string &msg)
{
    if (msg.empty()) {
        LOGE("Msg is empty.");
        return;
    }
    LOGI("handle user trust change, msg: %{public}s", GetAnonyString(msg).c_str());
    std::string remoteNetworkId = "";
    uint32_t discoveryType = 0;
    bool isPeerUserIdChanged = true;
    int32_t ret = ParseCheckSumMsg(msg, remoteNetworkId, discoveryType, isPeerUserIdChanged);
    if (ret != DM_OK) {
        LOGE("Parse checksum msg error");
        return;
    }
    if (!isPeerUserIdChanged) {
        LOGI("Peer foreground userId not change.");
        return;
    }
    std::vector<int32_t> foregroundUserIds;
    ret = MultipleUserConnector::GetForegroundUserIds(foregroundUserIds);
    if (ret != DM_OK || foregroundUserIds.empty()) {
        LOGE("Get foreground userids failed, ret: %{public}d", ret);
        return;
    }

    std::vector<int32_t> backgroundUserIds;
    ret = MultipleUserConnector::GetBackgroundUserIds(backgroundUserIds);
    if (ret != DM_OK || backgroundUserIds.empty()) {
        LOGI("Can not get background userids, ret: %{public}d, background userid num: %{public}d",
            ret, static_cast<int32_t>(backgroundUserIds.size()));
    }
    uint32_t addrTypeMask = 1 << NetworkType::BIT_NETWORK_TYPE_BLE;
    if ((discoveryType & addrTypeMask) != 0x0) {
        ProcessCheckSumByBT(remoteNetworkId, foregroundUserIds, backgroundUserIds);
    } else {
        ProcessCheckSumByWifi(remoteNetworkId, foregroundUserIds, backgroundUserIds);
    }
}
#endif

void DeviceManagerService::ClearDiscoveryCache(const ProcessInfo &processInfo)
{
    LOGI("PkgName: %{public}s, userId: %{public}d", processInfo.pkgName.c_str(), processInfo.userId);
    CHECK_NULL_VOID(discoveryMgr_);
    discoveryMgr_->ClearDiscoveryCache(processInfo);
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
    PackageEventCallback callback = [=](const auto &arg1, const auto &arg2, const auto &arg3) {
        if (IsDMServiceImplReady()) {
            dmServiceImpl_->ProcessAppUnintall(arg1, arg3);
        }
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

int32_t DeviceManagerService::SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string remoteAclList)
{
    if (IsDMServiceImplReady()) {
        return dmServiceImpl_->SyncLocalAclListProcess(localDevUserInfo, remoteDevUserInfo, remoteAclList);
    }
    return ERR_DM_FAILED;
}

int32_t DeviceManagerService::GetAclListHash(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclList)
{
    if (IsDMServiceImplReady()) {
        return dmServiceImpl_->GetAclListHash(localDevUserInfo, remoteDevUserInfo, aclList);
    }
    return ERR_DM_FAILED;
}

void DeviceManagerService::HandleCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId,
                                                      int32_t errcode)
{
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleCredentialAuthStatus(deviceList, deviceTypeId, errcode);
    }
}

void DeviceManagerService::RemoveNotifyRecord(const ProcessInfo &processInfo)
{
    LOGI("start");
    CHECK_NULL_VOID(listener_);
    listener_->OnProcessRemove(processInfo);
}

int32_t DeviceManagerService::RegDevStateCallbackToService(const std::string &pkgName)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    CHECK_NULL_RETURN(listener_, ERR_DM_POINT_NULL);
    std::vector<DmDeviceInfo> deviceList;
    GetTrustedDeviceList(pkgName, deviceList);
    if (deviceList.size() == 0) {
        return DM_OK;
    }
    int32_t userId = -1;
    MultipleUserConnector::GetCallerUserId(userId);
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    processInfo.userId = userId;
    listener_->OnDevStateCallbackAdd(processInfo, deviceList);
#else
    (void)pkgName;
#endif
    return DM_OK;
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("Begin for pkgName = %{public}s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::vector<DmDeviceInfo> onlineDeviceList;
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetTrustedDeviceList(onlineDeviceList);
    if (ret != DM_OK) {
        LOGE("failed");
        return ret;
    }
    if (!onlineDeviceList.empty() && IsDMServiceImplReady()) {
        dmServiceImpl_->DeleteAlwaysAllowTimeOut();
        std::unordered_map<std::string, DmAuthForm> udidMap;
        if (PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName)) {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(std::string(ALL_PKGNAME));
        } else {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(pkgName);
        }
        for (auto item : onlineDeviceList) {
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

void DeviceManagerService::HandleDeviceUnBind(const char *peerUdid, const GroupInformation &groupInfo)
{
    LOGI("DeviceManagerService::HandleDeviceUnBind start.");
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceUnBind(groupInfo.groupType, std::string(peerUdid),
            localUdid, groupInfo.userId, groupInfo.osAccountId);
    }
    return;
}

int32_t DeviceManagerService::GetAnonyLocalUdid(const std::string &pkgName, std::string &anonyUdid)
{
    (void) pkgName;
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call GetAnonyLocalUdid.");
        return ERR_DM_NO_PERMISSION;
    }
    std::string udid = DmRadarHelper::GetInstance().GetAnonyLocalUdid();
    if (udid.empty()) {
        LOGE("Anony local udid is empty.");
        return ERR_DM_FAILED;
    }
    anonyUdid = udid;
    return DM_OK;
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::NotifyRemoteLocalUserSwitch(int32_t curUserId, int32_t preUserId,
    const std::vector<std::string> &peerUdids, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("Send local foreground and background userids");
    if (peerUdids.empty()) {
        return;
    }
    if (softbusListener_ == nullptr) {
        dmServiceImpl_->HandleUserSwitched(peerUdids, curUserId, preUserId);
        LOGE("softbusListener_ is null");
        return;
    }
    std::vector<std::string> bleUdids;
    std::map<std::string, std::string> wifiDevices;
    for (const auto &udid : peerUdids) {
        std::string netWorkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, netWorkId);
        if (netWorkId.empty()) {
            LOGI("netWorkId is empty: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        int32_t networkType = 0;
        int32_t ret = softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType);
        if (ret != DM_OK || networkType <= 0) {
            LOGI("get networkType failed: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        uint32_t addrTypeMask = 1 << NetworkType::BIT_NETWORK_TYPE_BLE;
        if ((static_cast<uint32_t>(networkType) & addrTypeMask) != 0x0) {
            bleUdids.push_back(udid);
        } else {
            wifiDevices.insert(std::pair<std::string, std::string>(udid, netWorkId));
        }
    }
    if (!bleUdids.empty()) {
        dmServiceImpl_->HandleUserSwitched(bleUdids, curUserId, preUserId);
        SendUserIdsBroadCast(bleUdids, foregroundUserIds, backgroundUserIds, true);
    }
    if (!wifiDevices.empty()) {
        NotifyRemoteLocalUserSwitchByWifi(curUserId, preUserId, wifiDevices, foregroundUserIds, backgroundUserIds);
    }
}

void DeviceManagerService::NotifyRemoteLocalUserSwitchByWifi(int32_t curUserId, int32_t preUserId,
    const std::map<std::string, std::string> &wifiDevices, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    for (const auto &it : wifiDevices) {
        int32_t result = SendUserIdsByWifi(it.second, foregroundUserIds, backgroundUserIds);
        if (result != DM_OK) {
            LOGE("by wifi failed: %{public}s", GetAnonyString(it.first).c_str());
            std::vector<std::string> updateUdids;
            updateUdids.push_back(it.first);
            dmServiceImpl_->HandleUserSwitched(updateUdids, curUserId, preUserId);
            continue;
        }
        if (timer_ == nullptr) {
            timer_ = std::make_shared<DmTimer>();
        }
        std::string udid = it.first;
        timer_->StartTimer(std::string(USER_SWITCH_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(udid),
            USER_SWITCH_BY_WIFI_TIMEOUT_S, [this, curUserId, preUserId, udid] (std::string name) {
                DeviceManagerService::HandleUserSwitchTimeout(curUserId, preUserId, udid);
            });
    }
}

int32_t DeviceManagerService::SendUserIdsByWifi(const std::string &networkId,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("Try open softbus session to exchange foreground/background userid");
    std::vector<uint32_t> foregroundUserIdsUInt;
    for (auto const &u : foregroundUserIds) {
        foregroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    std::vector<uint32_t> backgroundUserIdsUInt;
    for (auto const &u : backgroundUserIds) {
        backgroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    return DMCommTool::GetInstance()->SendUserIds(networkId, foregroundUserIdsUInt, backgroundUserIdsUInt);
}

void DeviceManagerService::HandleUserSwitchTimeout(int32_t curUserId, int32_t preUserId, const std::string &udid)
{
    LOGI("start udid: %{public}s", GetAnonyString(udid).c_str());
    std::vector<std::string> updateUdids;
    updateUdids.push_back(udid);
    dmServiceImpl_->HandleUserSwitched(updateUdids, curUserId, preUserId);
}

void DeviceManagerService::HandleUserSwitchedEvent(int32_t currentUserId, int32_t beforeUserId)
{
    LOGI("In");
    DeviceNameManager::GetInstance().InitDeviceNameWhenUserSwitch(currentUserId, beforeUserId);
    MultipleUserConnector::SetAccountInfo(currentUserId, MultipleUserConnector::GetCurrentDMAccountInfo());
    if (IsPC()) {
        return;
    }
    if (beforeUserId == -1 || currentUserId == -1) {
        HandleUserSwitched();
        return;
    }
    if (beforeUserId != -1 && currentUserId != -1) {
        HandleUserSwitched(currentUserId, beforeUserId);
    }
}

void DeviceManagerService::HandleUserStopEvent(int32_t stopUserId)
{
    LOGI("stopUserId %{public}s.", GetAnonyInt32(stopUserId).c_str());
    std::vector<int32_t> stopUserVec;
    stopUserVec.push_back(stopUserId);
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    std::map<std::string, int32_t> stopUserDeviceMap;
    std::vector<std::string> peerUdids;
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    stopUserDeviceMap = discoveryMgr_->GetCommonDependencyObj()->
        GetDeviceIdAndBindLevel(stopUserVec, localUdid);
    for (const auto &item : stopUserDeviceMap) {
        peerUdids.push_back(item.first);
    }
    if (peerUdids.empty()) {
        LOGI("no data to be stoped.");
        return;
    }
    NotifyRemoteLocalUserStop(localUdid, peerUdids, stopUserId);
}

void DeviceManagerService::DivideNotifyMethod(const std::vector<std::string> &peerUdids,
    std::vector<std::string> &bleUdids, std::map<std::string, std::string> &wifiDevices)
{
    if (peerUdids.empty()) {
        return;
    }
    if (softbusListener_ == nullptr) {
        bleUdids = peerUdids;
        LOGI("softbusListener_ is null");
        return;
    }
    for (const auto &udid : peerUdids) {
        std::string netWorkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, netWorkId);
        if (netWorkId.empty()) {
            LOGI("netWorkId is empty: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        int32_t networkType = 0;
        int32_t ret = softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType);
        if (ret != DM_OK || networkType <= 0) {
            LOGI("get networkType failed: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        uint32_t addrTypeMask = 1 << NetworkType::BIT_NETWORK_TYPE_BLE;
        if ((static_cast<uint32_t>(networkType) & addrTypeMask) != 0x0) {
            bleUdids.push_back(udid);
        } else {
            wifiDevices.insert(std::pair<std::string, std::string>(udid, netWorkId));
        }
    }
}

void DeviceManagerService::HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid,
    const std::vector<std::string> &acceptEventUdids)
{
    if (timer_ != nullptr) {
        for (const auto &udid : acceptEventUdids) {
            timer_->DeleteTimer(std::string(USER_STOP_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(udid));
        }
    }
    if (MultipleUserConnector::IsUserUnlocked(stopUserId)) {
        LOGE("user has unlocked %{public}s.", GetAnonyInt32(stopUserId).c_str());
        return;
    }
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    discoveryMgr_->GetCommonDependencyObj()->HandleUserStop(stopUserId, stopEventUdid, acceptEventUdids);
}

void DeviceManagerService::HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid)
{
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    discoveryMgr_->GetCommonDependencyObj()->HandleUserStop(stopUserId, stopEventUdid);
}

void DeviceManagerService::NotifyRemoteLocalUserStop(const std::string &localUdid,
    const std::vector<std::string> &peerUdids, int32_t stopUserId)
{
    std::vector<std::string> bleUdids;
    std::map<std::string, std::string> wifiDevices;
    DivideNotifyMethod(peerUdids, bleUdids, wifiDevices);
    if (!bleUdids.empty()) {
        HandleUserStop(stopUserId, localUdid, bleUdids);
        SendUserStopBroadCast(bleUdids, stopUserId);
    }
    if (!wifiDevices.empty()) {
        NotifyRemoteLocalUserStopByWifi(localUdid, wifiDevices, stopUserId);
    }
}

void DeviceManagerService::SendUserStopBroadCast(const std::vector<std::string> &peerUdids, int32_t stopUserId)
{
    LOGI("peerUdids: %{public}s", GetAnonyStringList(peerUdids).c_str());
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::STOP_USER;
    msg.userId = static_cast<uint32_t>(stopUserId);
    msg.peerUdids = peerUdids;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::HandleUserStopBroadCast(int32_t stopUserId, const std::string &remoteUdid)
{
    LOGI("start");
    HandleUserStop(stopUserId, remoteUdid);
}

void DeviceManagerService::NotifyRemoteLocalUserStopByWifi(const std::string &localUdid,
    const std::map<std::string, std::string> &wifiDevices, int32_t stopUserId)
{
    for (const auto &it : wifiDevices) {
        std::vector<std::string> updateUdids;
        updateUdids.push_back(it.first);
        int32_t result = DMCommTool::GetInstance()->SendUserStop(it.second, stopUserId);
        if (result != DM_OK) {
            LOGE("by wifi failed: %{public}s", GetAnonyString(it.first).c_str());
            HandleUserStop(stopUserId, localUdid, updateUdids);
            continue;
        }
        if (timer_ == nullptr) {
            timer_ = std::make_shared<DmTimer>();
        }
        std::string udid = it.first;
        timer_->StartTimer(std::string(USER_STOP_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(udid),
            USER_SWITCH_BY_WIFI_TIMEOUT_S,
            [this, stopUserId, localUdid, updateUdids] (std::string name) {
                DeviceManagerService::HandleUserStop(stopUserId, localUdid, updateUdids);
            });
    }
}
#endif

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

int32_t DeviceManagerService::GetDeviceProfileInfoList(const std::string &pkgName,
    DmDeviceProfileInfoFilterOptions &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("GetDeviceProfileInfoList failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExtResident_->GetDeviceProfileInfoList(pkgName, filterOptions);
}

int32_t DeviceManagerService::GetDeviceIconInfo(const std::string &pkgName,
    DmDeviceIconInfoFilterOptions &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("GetDeviceIconInfo failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExtResident_->GetDeviceIconInfo(pkgName, filterOptions);
}

int32_t DeviceManagerService::PutDeviceProfileInfoList(const std::string &pkgName,
    std::vector<DmDeviceProfileInfo> &deviceProfileInfoList)
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
    if (!PermissionManager::GetInstance().CheckProcessNameValidPutDeviceProfileInfoList(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("GetDeviceProfileInfoList failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExtResident_->PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
}

int32_t DeviceManagerService::SetLocalDisplayNameToSoftbus(const std::string &displayName)
{
    LOGI("DeviceManagerService Start SetLocalDisplayName!");
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->SetLocalDisplayName(displayName);
    if (ret != DM_OK) {
        LOGE("SetLocalDisplayName error, failed ret: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
    std::string &displayName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(maxNameLength, displayName);
#endif
    return DM_OK;
}

int32_t DeviceManagerService::SetLocalDeviceName(const std::string &pkgName, const std::string &deviceName)
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
    if (!PermissionManager::GetInstance().CheckProcessNameValidModifyLocalDeviceName(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("SetLocalDeviceName failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExtResident_->SetLocalDeviceName(pkgName, deviceName);
}

int32_t DeviceManagerService::SetRemoteDeviceName(const std::string &pkgName,
    const std::string &deviceId, const std::string &deviceName)
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
    if (!PermissionManager::GetInstance().CheckProcessNameValidModifyRemoteDeviceName(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("SetRemoteDeviceName failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    return dmServiceImplExtResident_->SetRemoteDeviceName(pkgName, deviceId, deviceName);
}

std::vector<std::string> DeviceManagerService::GetDeviceNamePrefixs()
{
    LOGI("In");
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("GetDeviceNamePrefixs failed, adapter instance not init or init failed.");
        return {};
    }
    return dmServiceImplExtResident_->GetDeviceNamePrefixs();
}

void DeviceManagerService::HandleNetworkConnected()
{
    LOGI("In");
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("HandleNetworkConnected failed, adapter instance not init or init failed.");
        return;
    }
    dmServiceImplExtResident_->HandleNetworkConnected();
}

int32_t DeviceManagerService::RestoreLocalDeviceName(const std::string &pkgName)
{
    LOGI("In");
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidModifyLocalDeviceName(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (IsDMServiceAdapterResidentLoad()) {
        dmServiceImplExtResident_->RestoreLocalDeviceName();
    } else {
        LOGW("RestoreLocalDeviceName fail, adapter instance not init or init failed.");
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return DeviceNameManager::GetInstance().RestoreLocalDeviceName();
#endif
    return DM_OK;
}

void DeviceManagerService::AddHmlInfoToBindParam(int32_t actionId, std::string &bindParam)
{
    cJSON *bindParamObj = cJSON_Parse(bindParam.c_str());
    if (bindParamObj == NULL) {
        bindParamObj = cJSON_CreateObject();
        if (bindParamObj == NULL) {
            LOGE("Create bindParamObj object failed.");
            return;
        }
    }
    cJSON_AddStringToObject(bindParamObj, PARAM_KEY_CONN_SESSIONTYPE, CONN_SESSION_TYPE_HML);
    cJSON_AddStringToObject(bindParamObj, PARAM_KEY_HML_ACTIONID, std::to_string(actionId).c_str());
    char *str = cJSON_PrintUnformatted(bindParamObj);
    if (str == nullptr) {
        cJSON_Delete(bindParamObj);
        return;
    }
    bindParam = std::string(str);
    cJSON_free(str);
    cJSON_Delete(bindParamObj);
}

void DeviceManagerService::ClearPublishIdCache(const std::string &pkgName)
{
    CHECK_NULL_VOID(advertiseMgr_);
    advertiseMgr_->ClearPublishIdCache(pkgName);
}

bool DeviceManagerService::IsPC()
{
    if (softbusListener_ == nullptr) {
        LOGE("softbusListener_ is null.");
        return false;
    }
    DmDeviceInfo info;
    GetLocalDeviceInfo(info);
    return (info.deviceTypeId == DmDeviceType::DEVICE_TYPE_PC || info.deviceTypeId == DmDeviceType::DEVICE_TYPE_2IN1);
}

int32_t DeviceManagerService::GetDeviceNetworkIdList(const std::string &pkgName,
    const NetworkIdQueryFilter &queryFilter, std::vector<std::string> &networkIds)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("GetDeviceProfileInfoList failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    std::vector<DmDeviceProfileInfo> dmDeviceProfileInfos;
    int32_t ret = dmServiceImplExtResident_->GetDeviceProfileInfosFromLocalCache(queryFilter, dmDeviceProfileInfos);
    if (ret != DM_OK) {
        LOGW("GetDeviceProfileInfosFromLocalCache failed, ret = %{public}d.", ret);
        return ret;
    }
    for (const auto &item : dmDeviceProfileInfos) {
        std::string networkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(item.deviceId, networkId);
        if (!networkId.empty()) {
            networkIds.emplace_back(networkId);
        }
    }
    if (networkIds.empty()) {
        LOGW("networkIds is empty");
        return ERR_DM_FIND_NETWORKID_LIST_EMPTY;
    }
    return DM_OK;
}

void DeviceManagerService::NotifyRemoteLocalLogout(const std::vector<std::string> &peerUdids,
    const std::string &accountIdHash, const std::string &accountName, int32_t userId)
{
    LOGI("Start.");
    std::vector<std::string> bleUdids;
    std::vector<std::string> wifiDevices;
    for (const auto &udid : peerUdids) {
        std::string netWorkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, netWorkId);
        if (netWorkId.empty()) {
            LOGI("netWorkId is empty: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        int32_t networkType = 0;
        int32_t ret = softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType);
        if (ret != DM_OK || networkType <= 0) {
            LOGI("get networkType failed: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        uint32_t addrTypeMask = 1 << NetworkType::BIT_NETWORK_TYPE_BLE;
        if ((static_cast<uint32_t>(networkType) & addrTypeMask) != 0x0) {
            bleUdids.push_back(udid);
        } else {
            wifiDevices.push_back(netWorkId);
        }
    }
    if (!bleUdids.empty()) {
        SendAccountLogoutBroadCast(bleUdids, accountIdHash, accountName, userId);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    for (const auto &it : wifiDevices) {
        int32_t ret = DMCommTool::GetInstance()->SendLogoutAccountInfo(it, accountIdHash, userId);
        if (ret != DM_OK) {
            LOGE("Send LogoutAccount Info error, ret = %{public}d", ret);
        }
    }
#endif
}

void DeviceManagerService::ProcessSyncAccountLogout(const std::string &accountId, const std::string &peerUdid,
    int32_t userId)
{
    LOGI("Start. process udid: %{public}s", GetAnonyString(peerUdid).c_str());
    if (!IsDMServiceImplReady()) {
        LOGE("Imp instance not init or init failed.");
        return;
    }
    dmServiceImpl_->HandleAccountLogoutEvent(userId, accountId, peerUdid);
}

int32_t DeviceManagerService::UnRegisterPinHolderCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnRegisterPinHolderCallback.", pkgName.c_str());
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
    LOGI("begin.");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(pinHolder_, ERR_DM_POINT_NULL);
    return pinHolder_->UnRegisterPinHolderCallback(pkgName);
}
} // namespace DistributedHardware
} // namespace OHOS
