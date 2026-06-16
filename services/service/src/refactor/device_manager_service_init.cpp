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

#include "device_manager_service.h"

#include "dm_constants.h"
#include "dm_hidumper.h"
#include "dm_softbus_cache.h"
#include "parameter.h"
#include "permission_manager.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_constraints_manager.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "deviceprofile_connector.h"
#include "device_name_manager.h"
#include "distributed_device_profile_client.h"
#include "dm_comm_tool.h"
#include "dm_library_manager.h"
#include "dm_random.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#include "openssl/sha.h"
#include "system_ability_definition.h"
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
#include "ipc_server_stub.h"

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* LIB_IMPL_NAME = "libdevicemanagerserviceimpl.z.so";
using namespace OHOS::EventFwk;
#else
constexpr const char* LIB_IMPL_NAME = "libdevicemanagerserviceimpl.so";
#endif
constexpr const char* LIB_DM_ADAPTER_NAME = "libdevicemanageradapter.z.so";
constexpr const char* LIB_DM_RESIDENT_NAME = "libdevicemanagerresident.z.so";
constexpr const char* LIB_DM_DEVICE_RISK_DETECT_NAME = "libdevicemanagerriskdetect.z.so";
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
constexpr const char* LIB_DM_CHECK_API_WHITE_LIST_NAME = "libdm_check_api_whitelist.z.so";
#endif

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);
namespace {
    constexpr const char* SYNC_SERVICE_INFO_ONLINE_TASK = "SyncServiceInfoOnlineTask";
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
    const std::string GET_LOCAL_DEVICE_NAME_API_NAME = "GetLocalDeviceName";
#endif
    constexpr int32_t GENERATE_SERVICE_ID_RETRY_TIME = 3;
    constexpr uint64_t DEFAULT_TOKEN_ID = 0;
    constexpr const char* DEFAULT_PKG_NAME = "";
    constexpr uint64_t DEFAULT_SERVICE_ID = 0;
    constexpr int32_t DM_MAX_SERVICE_TYPE = 15;
    constexpr int32_t DM_MAX_SERVICE_NAME = 64;
    constexpr int32_t DM_MIN_SERVICE_DISPLAYNAME = 7;
    constexpr int32_t DM_MAX_SERVICE_DISPLAYNAME = 128;
    constexpr int32_t DM_MAX_CUSTOMDATA = 1024;
}

DeviceManagerService::~DeviceManagerService()
{
    LOGI("destructor");
    UnloadDMServiceImplSo();
    UnloadDMServiceAdapterResident();
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
    UnloadDmCheckApiWhiteListSo();
#endif
}

int32_t DeviceManagerService::Init()
{
    InitSoftbusListener();
    InitDMServiceListener();
    LOGI("success, dm service single instance initialized.");

    int32_t ret = HandleProcessRestart();
    if (ret != DM_OK) {
        LOGE("HandleProcessRestart failed, ret: %{public}d", ret);
    }
    return DM_OK;
}

int32_t DeviceManagerService::HandleProcessRestart()
{
    LOGI("Handle DM Process Restart start");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("DM Serivce adapter resident not load");
        return ERR_DM_INIT_FAILED;
    }

    int32_t ret = dmServiceImplExtResident_->HandleDMRestart();
    if (ret != DM_OK) {
        LOGE("Update ServiceId Cache failed, ret: %{public}d", ret);
    }
#endif
    LOGI("Handle DM Process Restart success");
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (IsDMServiceAdapterResidentLoad()) {
        int32_t ret = dmServiceImplExtResident_->InitSoftbusServer();
        if (ret != DM_OK) {
            LOGE("resident InitSoftbusServer failed.");
        }
        ret = dmServiceImplExtResident_->SetServiceNodeKeyInfo(DM_PKG_NAME);
        if (ret != DM_OK) {
            LOGE("Set ServiceId NodeKeyInfo failed, ret: %{public}d", ret);
            return ret;
        }
    }
#endif
    return DM_OK;
}

void DeviceManagerService::InitHichainListener()
{
    LOGI("Start.");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(hichainListenerLock_);
#else
    std::lock_guard<std::mutex> lock(hichainListenerLock_);
#endif
    if (hichainListener_ == nullptr) {
        hichainListener_ = std::make_shared<HichainListener>();
    }
    hichainListener_->RegisterDataChangeCb();
    hichainListener_->RegisterCredentialCb();
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::StartDetectDeviceRisk()
{
    std::lock_guard<ffrt::mutex> lock(detectLock_);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NULL_VOID(samgr);
    if (samgr->CheckSystemAbility(RISK_ANALYSIS_MANAGER_SA_ID) == nullptr) {
        LOGE("%{public}d sa not start", RISK_ANALYSIS_MANAGER_SA_ID);
        return;
    }
    auto loadRet = LoadDMDeviceRiskDetect();
    if (!loadRet.first || loadRet.second == nullptr) {
        LOGE("load dm device risk detect failed.");
        return;
    }

    bool isRisk = loadRet.second->IsDeviceHasRisk();
    if (isRisk) {
        LOGI("device status is Illegal");
        DelAllRelateShip();
    }
    UnloadDMDeviceRiskDetect(loadRet.second);
    return;
}

void DeviceManagerService::DelAllRelateShip()
{
    std::lock_guard<ffrt::mutex> lock(hichainListenerLock_);
    if (hichainListener_ == nullptr) {
        hichainListener_ = std::make_shared<HichainListener>();
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    std::vector<int32_t> currentUserIds;
    currentUserIds.push_back(userId);
    hichainListener_->DeleteAllGroup(localUdid, currentUserIds);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        std::string acerDeviceId = item.GetAccesser().GetAccesserDeviceId();
        std::string aceeDeviceId = item.GetAccessee().GetAccesseeDeviceId();
        if (localUdid == acerDeviceId) {
            int32_t acerUserId = item.GetAccesser().GetAccesserUserId();
            int32_t acerSkId = item.GetAccesser().GetAccesserSessionKeyId();
            DeviceProfileConnector::GetInstance().DeleteSessionKey(acerUserId, acerSkId);

            std::string acerCredId = item.GetAccesser().GetAccesserCredentialIdStr();
            hichainListener_->DeleteCredential(acerUserId, acerCredId);
        } else if (localUdid == aceeDeviceId) {
            int32_t aceeUserId = item.GetAccessee().GetAccesseeUserId();
            int32_t aceeSkId = item.GetAccessee().GetAccesseeSessionKeyId();
            DeviceProfileConnector::GetInstance().DeleteSessionKey(aceeUserId, aceeSkId);

            std::string aceeCredId = item.GetAccessee().GetAccesseeCredentialIdStr();
            hichainListener_->DeleteCredential(aceeUserId, aceeCredId);
        }
        int32_t aclId = item.GetAccessControlId();
        DeviceProfileConnector::GetInstance().DeleteAccessControlById(aclId);
    }
}
#endif

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
void DeviceManagerService::SubscribePublishCommonEvent()
{
    LOGI("start");
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
    std::lock_guard<ffrt::mutex> lock(eventManagerLock_);
    if (publishCommonEventManager_ == nullptr) {
        publishCommonEventManager_ = std::make_shared<DmPublishCommonEventManager>();
    }
    if (publishCommonEventManager_->SubscribePublishCommonEvent(PublishCommonEventVec, callback)) {
        LOGI("subscribe ble and wifi and screen common event success");
    }
    return;
}
#endif // SUPPORT_BLUETOOTH SUPPORT_WIFI

DM_EXPORT void DeviceManagerService::SubscribeDataShareCommonEvent()
{
    LOGI("start");
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
        if (arg1 == CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE) {
            this->HandleNetworkConnected(arg2);
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
    std::shared_ptr<DmPublishEventSubscriber> publishSubScriber = nullptr;
    {
        std::lock_guard<ffrt::mutex> lock(eventManagerLock_);
        CHECK_NULL_VOID(publishCommonEventManager_);
        publishSubScriber = publishCommonEventManager_->GetSubscriber();
    }
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

    ScreenState state = DM_SCREEN_ON;
#ifdef SUPPORT_POWER_MANAGER
    if (samgr->CheckSystemAbility(POWER_MANAGER_SERVICE_ID) == nullptr) {
        state = DM_SCREEN_OFF;
    } else {
        if (!OHOS::PowerMgr::PowerMgrClient::GetInstance().IsScreenOn()) {
            state = DM_SCREEN_OFF;
        }
    }
#endif // SUPPORT_POWER_MANAGER
    publishSubScriber->SetScreenState(state);
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
    if (hiChainConnector_ == nullptr) {
        hiChainConnector_ = std::make_shared<DmServiceHiChainConnector>();
    }
    if (credentialMgr_ == nullptr) {
        credentialMgr_ = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    }
    CHECK_NULL_RETURN(DMCommTool::GetInstance(), ERR_DM_POINT_NULL);
    DMCommTool::GetInstance()->Init();
    int32_t currentUserId = MultipleUserConnector::GetFirstForegroundUserId();
    if (IsPC() && !MultipleUserConnector::IsUserUnlocked(currentUserId)) {
        HandleUserStopEvent(currentUserId);
    }
    InitTaskOfDelTimeOutAcl();
    DeviceProfileConnector::GetInstance().DeleteDpInvalidAcl();
    if (IsDMServiceAdapterResidentLoad()) {
        int32_t ret = dmServiceImplExtResident_->SubscribeAllServiceInfo();
        if (ret != DM_OK) {
            LOGE("SubscribeAllServiceInfo failed, ret: %{public}d", ret);
        }
    }
    if (dpInitedCallback_ == nullptr) {
        dpInitedCallback_ = sptr<DpInitedCallback>(new DpInitedCallback());
        DeviceProfileConnector::GetInstance().SubscribeDeviceProfileInited(dpInitedCallback_);
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
    dpInitedCallback_ = nullptr;
    DeviceProfileConnector::GetInstance().UnSubscribeDeviceProfileInited();
#endif
    LOGI("Uninit.");
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

void DeviceManagerService::UnloadDmCheckApiWhiteListSo()
{
    LOGI("Start.");
    std::lock_guard<std::mutex> lock(isAdapterCheckApiWhiteListLoadedLock_);
    if (checkApiWhiteListSoHandle_ != nullptr) {
        LOGI("checkApiWhiteListSoHandle_ is not nullptr.");
        dlclose(checkApiWhiteListSoHandle_);
        checkApiWhiteListSoHandle_ = nullptr;
    }
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

bool DeviceManagerService::IsDMServiceImplReady()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(isImplLoadLock_);
#else
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
#endif
    if (isImplsoLoaded_ && (dmServiceImpl_ != nullptr)) {
        return true;
    }
    LOGI("libdevicemanagerserviceimpl start load.");
    dmServiceImplSoHandle_ = dlopen(LIB_IMPL_NAME, RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
    if (dmServiceImplSoHandle_ == nullptr) {
        dmServiceImplSoHandle_ = dlopen(LIB_IMPL_NAME, RTLD_NOW | RTLD_NODELETE);
    }
    if (dmServiceImplSoHandle_ == nullptr) {
        LOGE("load libdevicemanagerserviceimpl so failed, errMsg: %{public}s.", dlerror());
        return false;
    }
    dlerror();
    auto func = (CreateDMServiceFuncPtr)dlsym(dmServiceImplSoHandle_, "CreateDMServiceObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(dmServiceImplSoHandle_);
        dmServiceImplSoHandle_ = nullptr;
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(isImplLoadLock_);
#else
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
#endif
    return isImplsoLoaded_;
}

bool DeviceManagerService::IsDMServiceAdapterSoLoaded()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(isAdapterResidentLoadLock_);
#else
    std::lock_guard<std::mutex> lock(isAdapterResidentLoadLock_);
#endif
    if (!isAdapterResidentSoLoaded_ || (dmServiceImplExtResident_ == nullptr)) {
        return false;
    }
    return dmServiceImplExtResident_->IsDMServiceAdapterSoLoaded();
}

void DeviceManagerService::UnloadDMServiceImplSo()
{
    LOGI("Start.");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(isImplLoadLock_);
#else
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
#endif
    if (dmServiceImpl_ != nullptr) {
        dmServiceImpl_->Release();
    }
    if (dmServiceImplSoHandle_ != nullptr) {
        LOGI("DeviceManagerService so_handle is not nullptr.");
        dlclose(dmServiceImplSoHandle_);
        dmServiceImplSoHandle_ = nullptr;
    }
}

bool DeviceManagerService::IsDMServiceAdapterResidentLoad()
{
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(isAdapterResidentLoadLock_);
#else
    std::lock_guard<std::mutex> lock(isAdapterResidentLoadLock_);
#endif
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(isAdapterResidentLoadLock_);
#else
    std::lock_guard<std::mutex> lock(isAdapterResidentLoadLock_);
#endif
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

std::pair<bool, IDMDeviceRiskDetect*> DeviceManagerService::LoadDMDeviceRiskDetect()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    LOGI("Start.");
    std::lock_guard<ffrt::mutex> lock(deviceRiskDetectSoLoadLock_);
    auto& libManager = GetLibraryManager();
    auto createRiskDetectObj = libManager.GetFunction<IDMDeviceRiskDetect*(*)()>(LIB_DM_DEVICE_RISK_DETECT_NAME,
        "CreateDMDeviceRiskDetectObject");
    if (createRiskDetectObj == nullptr) {
        LOGE("load dm device risk detect so failed");
        return {false, nullptr};
    }

    IDMDeviceRiskDetect* riskDetectPtr = createRiskDetectObj();
    if (riskDetectPtr == nullptr) {
        LOGE("get risk detect ptr failed");
        libManager.Release(LIB_DM_DEVICE_RISK_DETECT_NAME);
        return {false, nullptr};
    }

    if (riskDetectPtr->Initialize() != DM_OK) {
        libManager.Release(LIB_DM_DEVICE_RISK_DETECT_NAME);
        LOGE("dm sdevice risk detect init failed.");
        return {false, nullptr};
    }
    LOGI("Success.");
    return {true, riskDetectPtr};
#else
    return {false, nullptr};
#endif
}

void DeviceManagerService::UnloadDMDeviceRiskDetect(IDMDeviceRiskDetect* &riskDetectPtr)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    LOGI("Start.");
    std::lock_guard<ffrt::mutex> lock(deviceRiskDetectSoLoadLock_);
    if (riskDetectPtr == nullptr) {
        LOGE("The risk detect ptr is null");
        return;
    }
    riskDetectPtr->Release();
    riskDetectPtr = nullptr;
    auto& libManager = GetLibraryManager();
    libManager.Release(LIB_DM_DEVICE_RISK_DETECT_NAME);
    LOGI("Success.");
#endif
}

int32_t DeviceManagerService::InitAccountInfo()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    SubscribeAccountCommonEvent();
    LOGI("Success.");
    DmConstrainsManager::GetInstance().SubscribeOsAccountConstraints({DM_ACCOUNT_CONSTRAINT});
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
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_STOPPED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_FOREGROUND);
    AccountCommonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_BACKGROUND);
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
    screenEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    if (screenCommonEventManager_->SubscribeScreenCommonEvent(screenEventVec, callback)) {
        LOGI("Success");
    }
    return;
}
#endif

void DeviceManagerService::SubscribePackageCommonEvent()
{
    LOGI("Start");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (packageCommonEventManager_ == nullptr) {
        packageCommonEventManager_ = std::make_shared<DmPackageCommonEventManager>();
    }
    PackageEventCallback callback = [=](const auto &arg1, const auto &arg2, const auto &arg3) {
        if (!DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(arg3)) {
            return;
        }
        int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
        NotifyRemoteUninstallApp(userId, arg3);
        if (IsDMServiceImplReady()) {
            dmServiceImpl_->ProcessAppUnintall(arg1, arg3);
        }
        KVAdapterManager::GetInstance().AppUninstall(arg1);
    };
    std::vector<std::string> commonEventVec;
    commonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    commonEventVec.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED);
    if (packageCommonEventManager_->SubscribePackageCommonEvent(commonEventVec, callback)) {
        LOGI("Success");
    }
#endif
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::InitTaskOfDelTimeOutAcl()
{
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> aclInfos;
    discoveryMgr_->GetCommonDependencyObj()->GetAllAuthOnceAclInfos(aclInfos);
    if (aclInfos.empty()) {
        LOGI("no auth once data.");
        return;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("instance not init or init failed.");
        return;
    }
    for (const auto &acl : aclInfos) {
        dmServiceImpl_->InitTaskOfDelTimeOutAcl(acl.peerUdid, acl.peerUserId, acl.localUserId);
    }
}
#endif

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
} // namespace DistributedHardware
} // namespace OHOS