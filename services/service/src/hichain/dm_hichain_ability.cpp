/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef FEATURE_GNSS_SUPPORT
#include "dm_hichain_ability.h"

#include <file_ex.h>
#include <thread>

#include "agnss_ni_manager.h"
#include "event_runner.h"
#include "idevmgr_hdi.h"
#include "ipc_skeleton.h"
#include "iproxy_broker.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
#include "agnss_event_callback.h"
#endif
#include "common_hisysevent.h"
#include "common_utils.h"
#include "gnss_event_callback.h"
#include "i_cached_locations_callback.h"
#include "location_config_manager.h"
#include "location_dumper.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locationhub_ipc_interface_code.h"
#include "location_log_event_ids.h"
#include "location_data_rdb_manager.h"
#include "permission_manager.h"

#ifdef NOTIFICATION_ENABLE
#include "notification_request.h"
#include "notification_helper.h"
#endif

#include "hook_utils.h"
#include "geofence_definition.h"

#ifdef TIME_SERVICE_ENABLE
#include "time_service_client.h"
#include "ntp_time_check.h"
#endif

#ifdef LOCATION_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace Location {
namespace {
constexpr uint32_t WAIT_MS = 200;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
const int MAX_GNSS_STATUS_CALLBACK_NUM = 1000;
const int MAX_NMEA_CALLBACK_NUM = 1000;
const int MAX_GNSS_GEOFENCE_REQUEST_NUM = 1000;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
constexpr const char *AGNSS_SERVICE_NAME = "agnss_interface_service";
#endif
constexpr const char *LOCATION_HOST_NAME = "location_host";
constexpr const char *GNSS_SERVICE_NAME = "gnss_interface_service";
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
constexpr const char *GEOFENCE_SERVICE_NAME = "geofence_interface_service";
#endif
constexpr const char *UNLOAD_GNSS_TASK = "gnss_sa_unload";
const uint32_t RETRY_INTERVAL_OF_UNLOAD_SA = 4 * 60 * EVENT_INTERVAL_UNITE;
constexpr int32_t FENCE_MAX_ID = 1000000;
constexpr int NLP_FIX_VALID_TIME = 2;
const int64_t INVALID_TIME = 0;
const int TIMEOUT_WATCHDOG = 60; // s
const int64_t MILL_TO_NANOS = 1000000;
}

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    GnssAbility::GetInstance());

GnssAbility* GnssAbility::GetInstance()
{
    static GnssAbility data;
    return &data;
}

GnssAbility::GnssAbility() : SystemAbility(LOCATION_GNSS_SA_ID, true)
{
    gnssCallback_ = nullptr;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    agnssCallback_ = nullptr;
#endif
    gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
    SetAbility(GNSS_ABILITY);
    gnssHandler_ = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    if (gnssHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            static_cast<uint32_t>(GnssAbilityInterfaceCode::INIT_HDI), 0);
        gnssHandler_->SendEvent(event);
    }
    fenceId_ = 0;
    auto agnssNiManager = AGnssNiManager::GetInstance();
    if (agnssNiManager != nullptr) {
        agnssNiManager->SubscribeSaStatusChangeListerner();
    }
    MonitorNetwork();
    LBSLOGI(GNSS, "ability constructed.");
}

GnssAbility::~GnssAbility()
{
#ifdef NET_MANAGER_ENABLE
    if (netWorkObserver_ != nullptr) {
        NetManagerStandard::NetConnClient::GetInstance().UnregisterNetConnCallback(netWorkObserver_);
    }
#endif
}

bool GnssAbility::CheckIfHdiConnected()
{
    if (!IsDeviceLoaded(GNSS_SERVICE_NAME)) {
        return false;
    }
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    if (!IsDeviceLoaded(AGNSS_SERVICE_NAME)) {
        return false;
    }
#endif
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    if (!IsDeviceLoaded(GEOFENCE_SERVICE_NAME)) {
        return false;
    }
#endif
    return true;
}

void GnssAbility::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(GNSS, "ability has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(GNSS, "failed to init ability");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    LBSLOGI(GNSS, "OnStart start ability success.");
}

void GnssAbility::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToAbility_ = false;
    if (CheckIfHdiConnected()) {
        auto startTime = CommonUtils::GetCurrentTimeStamp();
        auto ret = RemoveHdi();
        auto endTime = CommonUtils::GetCurrentTimeStamp();
        WriteLocationInnerEvent(HDI_EVENT, {"ret", std::to_string(ret), "type", "DisConnectHdi",
            "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
    }
    LBSLOGI(GNSS, "OnStop ability stopped.");
}

bool GnssAbility::Init()
{
    if (!registerToAbility_) {
        bool ret = Publish(AsObject());
        if (!ret) {
            LBSLOGE(GNSS, "Init Publish failed!");
            return false;
        }
        registerToAbility_ = true;
    }
    return true;
}

LocationErrCode GnssAbility::SendLocationRequest(WorkRecord &workrecord)
{
    LocationRequest(workrecord);
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::SetEnable(bool state)
{
    if (state) {
        EnableGnss();
        StartGnss();
    } else {
        StopGnss();
        DisableGnss();
    }
    return ERRCODE_SUCCESS;
}

bool GnssAbility::CancelIdleState()
{
    SystemAbilityState state = GetAbilityState();
    if (state != SystemAbilityState::IDLE) {
        return true;
    }
    bool ret = CancelIdle();
    if (!ret) {
        LBSLOGE(GNSS, "%{public}s cancel idle failed!", __func__);
        return false;
    }
    return true;
}

void GnssAbility::UnloadGnssSystemAbility()
{
    if (gnssHandler_ == nullptr) {
        LBSLOGE(GNSS, "%{public}s gnssHandler is nullptr", __func__);
        return;
    }
    gnssHandler_->RemoveTask(UNLOAD_GNSS_TASK);
    if (CheckIfGnssConnecting()) {
        return;
    }
    auto task = [this]() {
        SaLoadWithStatistic::UnInitLocationSa(LOCATION_GNSS_SA_ID);
    };
    if (gnssHandler_ != nullptr) {
        gnssHandler_->PostTask(task, UNLOAD_GNSS_TASK, RETRY_INTERVAL_OF_UNLOAD_SA);
    }
}

bool GnssAbility::CheckIfGnssConnecting()
{
    return IsMockEnabled() || GetRequestNum() != 0 || IsMockProcessing() || IsGnssfenceRequestMapExist();
}

bool GnssAbility::IsGnssfenceRequestMapExist()
{
    std::unique_lock<ffrt::mutex> lock(gnssGeofenceRequestMapMutex_);
    return gnssGeofenceRequestMap_.size() != 0;
}

LocationErrCode GnssAbility::RefrashRequirements()
{
    HandleRefrashRequirements();
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback,
    AppIdentity &identity)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid gnssStatus callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) GnssStatusCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    std::unique_lock<ffrt::mutex> lock(gnssMutex_);
    if (gnssStatusCallbackMap_.size() <= MAX_GNSS_STATUS_CALLBACK_NUM) {
        gnssStatusCallbackMap_[callback] = identity;
    } else {
        LBSLOGE(GNSS, "RegisterGnssStatusCallback num max");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GNSS, "RegisterGnssStatusCallback uid:%{public}d register, gnssStatusCallback size:%{public}s",
        identity.GetUid(), std::to_string(gnssStatusCallbackMap_.size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "unregister an invalid gnssStatus callback");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<ffrt::mutex> lock(gnssMutex_);
    auto iter = gnssStatusCallbackMap_.find(callback);
    if (iter != gnssStatusCallbackMap_.end()) {
        gnssStatusCallbackMap_.erase(iter);
    }
    LBSLOGD(GNSS, "after unregister, gnssStatus callback size:%{public}s",
        std::to_string(gnssStatusCallbackMap_.size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback,
    AppIdentity &identity)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid nmea callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) NmeaCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    std::unique_lock<ffrt::mutex> lock(nmeaMutex_);
    if (nmeaCallbackMap_.size() <= MAX_NMEA_CALLBACK_NUM) {
        nmeaCallbackMap_[callback] = identity;
    } else {
        LBSLOGE(GNSS, "RegisterNmeaMessageCallback num max");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GNSS, "after uid:%{public}d register, nmeaCallback size:%{public}s",
        identity.GetUid(), std::to_string(nmeaCallbackMap_.size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "unregister an invalid nmea callback");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<ffrt::mutex> lock(nmeaMutex_);
    auto iter = nmeaCallbackMap_.find(callback);
    if (iter != nmeaCallbackMap_.end()) {
        nmeaCallbackMap_.erase(iter);
    }

    LBSLOGD(GNSS, "after unregister, nmea callback size:%{public}s",
        std::to_string(nmeaCallbackMap_.size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
    const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid cached location callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) CachedLocationCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    sptr<ICachedLocationsCallback> cachedCallback = iface_cast<ICachedLocationsCallback>(callback);
    if (cachedCallback == nullptr) {
        LBSLOGE(GNSS, "cast cached location callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    LBSLOGD(GNSS, "request:%{public}d %{public}d",
        request->reportingPeriodSec, request->wakeUpCacheQueueFull ? 1 : 0);
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode GnssAbility::UnregisterCachedCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid cached location callback");
        return ERRCODE_INVALID_PARAM;
    }

    sptr<ICachedLocationsCallback> cachedCallback = iface_cast<ICachedLocationsCallback>(callback);
    if (cachedCallback == nullptr) {
        LBSLOGE(GNSS, "cast cached location callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    return ERRCODE_NOT_SUPPORTED;
}

void GnssAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    LBSLOGD(GNSS, "enter RequestRecord");
    if (isAdded) {
        if (!CheckIfHdiConnected()) {
            auto startTime = CommonUtils::GetCurrentTimeStamp();
            auto ret = ConnectHdi();
            auto endTime = CommonUtils::GetCurrentTimeStamp();
            WriteLocationInnerEvent(HDI_EVENT, {"ret", std::to_string(ret), "type", "ConnectHdi",
                    "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
        }
        EnableGnss();
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
        SetAgnssServer();
#endif
        StartGnss();
        LocatorRequestStruct locatorRequestStruct;
        locatorRequestStruct.bundleName = workRecord.GetName(0);
        LocationErrCode errorCode = HookUtils::ExecuteHook(LocationProcessStage::GNSS_REQUEST_RECORD_PROCESS,
            (void *)&locatorRequestStruct, nullptr);
    } else {
        // GNSS will stop only if all requests have stopped
        if (GetRequestNum() == 0) {
            StopGnss();
        }
    }
    std::string state = isAdded ? "start" : "stop";
    WriteGnssStateEvent(state, workRecord.GetPid(0), workRecord.GetUid(0));
}

void GnssAbility::ReConnectHdi()
{
    if (gnssHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            static_cast<uint32_t>(GnssAbilityInterfaceCode::RECONNECT_HDI), 0);
        gnssHandler_->SendEvent(event);
    }
}

void GnssAbility::ReConnectHdiImpl()
{
    LBSLOGD(GNSS, "%{public}s called", __func__);
    ConnectHdi();
    EnableGnss();
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    SetAgnssCallback();
    SetAgnssServer();
#endif
    if (gnssWorkingStatus_ == GNSS_WORKING_STATUS_SESSION_BEGIN) {
        StartGnss();
    }
}

LocationErrCode GnssAbility::GetCachedGnssLocationsSize(int& size)
{
    size = -1;
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode GnssAbility::FlushCachedGnssLocations()
{
    LBSLOGE(GNSS, "%{public}s not support", __func__);
    return ERRCODE_NOT_SUPPORTED;
}

bool GnssAbility::GetCommandFlags(std::unique_ptr<LocationCommand>& commands, GnssAuxiliaryDataType& flags)
{
    std::string cmd = commands->command;
    if (cmd == "delete_auxiliary_data_ephemeris") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_EPHEMERIS;
    } else if (cmd == "delete_auxiliary_data_almanac") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALMANAC;
    } else if (cmd == "delete_auxiliary_data_position") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_POSITION;
    } else if (cmd == "delete_auxiliary_data_time") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_TIME;
    } else if (cmd == "delete_auxiliary_data_iono") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_IONO;
    } else if (cmd == "delete_auxiliary_data_utc") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_UTC;
    } else if (cmd == "delete_auxiliary_data_health") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_HEALTH;
    } else if (cmd == "delete_auxiliary_data_svdir") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVDIR;
    } else if (cmd == "delete_auxiliary_data_svsteer") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVSTEER;
    } else if (cmd == "delete_auxiliary_data_sadata") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SADATA;
    } else if (cmd == "delete_auxiliary_data_rti") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_RTI;
    } else if (cmd == "delete_auxiliary_data_celldb_info") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_CELLDB_INFO;
    } else if (cmd == "delete_auxiliary_data_all") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALL;
    } else {
        LBSLOGE(GNSS, "unknow command %{public}s", cmd.c_str());
        return false;
    }
    return true;
}

LocationErrCode GnssAbility::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    GnssAuxiliaryDataType flags;
    bool result = GetCommandFlags(commands, flags);
    LBSLOGE(GNSS, "GetCommandFlags,flags = %{public}d", flags);
    if (result) {
        gnssInterface->DeleteAuxiliaryData(flags);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::SetPositionMode()
{
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    GnssConfigPara para;
    int suplMode = LocationConfigManager::GetInstance()->GetSuplMode();
    if (suplMode == MODE_STANDALONE) {
        para.gnssBasic.gnssMode = GnssWorkingMode::GNSS_WORKING_MODE_STANDALONE;
    } else if (suplMode == MODE_MS_BASED) {
        para.gnssBasic.gnssMode = GnssWorkingMode::GNSS_WORKING_MODE_MS_BASED;
    } else if (suplMode == MODE_MS_ASSISTED) {
        para.gnssBasic.gnssMode = GnssWorkingMode::GNSS_WORKING_MODE_MS_ASSISTED;
    } else {
        LBSLOGE(GNSS, "unknow mode");
        return ERRCODE_SUCCESS;
    }
    int ret = gnssInterface->SetGnssConfigPara(para);
    if (ret != ERRCODE_SUCCESS) {
        LBSLOGE(GNSS, "SetGnssConfigPara failed , ret =%{public}d", ret);
    }
    return ERRCODE_SUCCESS;
}

void GnssAbility::MonitorNetwork()
{
#ifdef NET_MANAGER_ENABLE
    NetManagerStandard::NetSpecifier netSpecifier;
    NetManagerStandard::NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetManagerStandard::NetSpecifier> specifier(
        new (std::nothrow) NetManagerStandard::NetSpecifier(netSpecifier));
    if (specifier == nullptr) {
        LBSLOGE(GNSS, "new operator error.specifier is nullptr");
        return;
    }
    netWorkObserver_ = sptr<NetConnObserver>((new (std::nothrow) NetConnObserver()));
    if (netWorkObserver_ == nullptr) {
        LBSLOGE(GNSS, "new operator error.netWorkObserver_ is nullptr");
        return;
    }
    int ret = NetManagerStandard::NetConnClient::GetInstance().RegisterNetConnCallback(specifier, netWorkObserver_, 0);
    LBSLOGI(GNSS, "RegisterNetConnCallback retcode= %{public}d", ret);
#endif
    return;
}

LocationErrCode GnssAbility::InjectTime()
{
#ifdef TIME_SERVICE_ENABLE
    LBSLOGD(GNSS, "InjectTime");
    int64_t currentTime = ntpTime_.GetCurrentTime();
    if (currentTime == INVALID_TIME) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto elapsedTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    if (elapsedTime < 0) {
        LBSLOGE(GNSS, "get boot time failed");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto ntpTimeCheck = NtpTimeCheck::GetInstance();
    if (ntpTimeCheck != nullptr && ntpTimeCheck->CheckNtpTime(currentTime, elapsedTime)) {
        GnssRefInfo refInfo;
        refInfo.type = GnssRefInfoType::GNSS_REF_INFO_TIME;
        refInfo.time.time = currentTime;
        refInfo.time.elapsedRealtime = elapsedTime;
        refInfo.time.uncertaintyOfTime = ntpTimeCheck->GetUncertainty();
        auto gnssInterface = IGnssInterface::Get();
        if (gnssInterface != nullptr) {
            LBSLOGI(GNSS, "inject ntp time: %{public}s unert %{public}d",
                std::to_string(currentTime).c_str(), ntpTimeCheck->GetUncertainty());
            gnssInterface->SetGnssReferenceInfo(refInfo);
        }
    }
#endif
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::UpdateNtpTime(int64_t ntpTime, int64_t elapsedTime)
{
#ifdef TIME_SERVICE_ENABLE
    if (ntpTime <= 0 || elapsedTime <= 0) {
        LBSLOGE(GNSS, "failed to UpdateNtpTime");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ntpTime_.SetCurrentTime(ntpTime, elapsedTime);
#endif
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::SendNetworkLocation(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        LBSLOGE(GNSS, "location is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    nlpLocation_ = *location;
    return InjectLocation();
}

LocationErrCode GnssAbility::InjectLocation()
{
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface or location is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (nlpLocation_.GetAccuracy() < 1e-9 || nlpLocation_.GetTimeStamp() == 0) {
        LBSLOGW(GNSS, "nlp locaton acc or timesatmp is invalid");
        return ERRCODE_INVALID_PARAM;
    }
    int64_t diff = CommonUtils::GetCurrentTimeStamp() - nlpLocation_.GetTimeStamp() / MILLI_PER_SEC;
    if (diff > NLP_FIX_VALID_TIME) {
        LBSLOGI(GNSS, "nlp locaton is invalid");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    GnssRefInfo refInfo;
    refInfo.type = GnssRefInfoType::GNSS_REF_INFO_LOCATION;
    refInfo.gnssLocation.fieldValidity =
        GnssLocationValidity::GNSS_LOCATION_LAT_VALID |
        GnssLocationValidity::GNSS_LOCATION_LONG_VALID |
        GnssLocationValidity::GNSS_LOCATION_HORIZONTAL_ACCURACY_VALID;
    refInfo.gnssLocation.latitude = nlpLocation_.GetLatitude();
    refInfo.gnssLocation.longitude = nlpLocation_.GetLongitude();
    refInfo.gnssLocation.altitude = nlpLocation_.GetAltitude();
    refInfo.gnssLocation.speed = nlpLocation_.GetSpeed();
    refInfo.gnssLocation.bearing = nlpLocation_.GetDirection();
    refInfo.gnssLocation.horizontalAccuracy = nlpLocation_.GetAccuracy();
    refInfo.gnssLocation.verticalAccuracy = nlpLocation_.GetAltitudeAccuracy();
    refInfo.gnssLocation.speedAccuracy = nlpLocation_.GetSpeedAccuracy();
    refInfo.gnssLocation.bearingAccuracy = nlpLocation_.GetDirectionAccuracy();
    refInfo.gnssLocation.timeForFix = nlpLocation_.GetTimeStamp();
    refInfo.gnssLocation.timeSinceBoot = nlpLocation_.GetTimeSinceBoot();
    refInfo.gnssLocation.timeUncertainty = nlpLocation_.GetUncertaintyOfTimeSinceBoot();
    gnssInterface->SetGnssReferenceInfo(refInfo);
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::AddFence(std::shared_ptr<GeofenceRequest>& request)
{
    int fenceId = GenerateFenceId();
    request->SetFenceId(fenceId);
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    sptr<IGeofenceInterface> geofenceInterface = IGeofenceInterface::Get();
    if (geofenceInterface == nullptr) {
        LBSLOGE(GNSS, "geofenceInterface is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto geofence = request->GetGeofence();
    GeofenceInfo fenceInfo;
    fenceInfo.fenceIndex = fenceId;
    fenceInfo.latitude = geofence.latitude;
    fenceInfo.longitude = geofence.longitude;
    fenceInfo.radius = geofence.radius;
    int monitorEvent = static_cast<int>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER) |
        static_cast<int>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_EXIT);
    int32_t ret = geofenceInterface->AddGnssGeofence(fenceInfo, monitorEvent);
    LBSLOGD(GNSS, "Successfully AddFence!, %{public}d", ret);
#endif
    if (ExecuteFenceProcess(GnssInterfaceCode::ADD_FENCE_INFO, request)) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode GnssAbility::RemoveFence(std::shared_ptr<GeofenceRequest>& request)
{
    if (request == nullptr) {
        LBSLOGE(GNSS, "request is nullptr");
        return ERRCODE_GEOFENCE_FAIL;
    }
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    sptr<IGeofenceInterface> geofenceInterface = IGeofenceInterface::Get();
    if (geofenceInterface == nullptr) {
        LBSLOGE(GNSS, "geofenceInterface is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int32_t ret = geofenceInterface->DeleteGnssGeofence(request->GetFenceId());
    LBSLOGD(GNSS, "Successfully RemoveFence!, %{public}d", ret);
#endif
    if (ExecuteFenceProcess(GnssInterfaceCode::REMOVE_FENCE_INFO, request)) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_NOT_SUPPORTED;
}

int32_t GnssAbility::GenerateFenceId()
{
    LBSLOGD(GNSS, "GenerateFenceId");
    std::lock_guard<ffrt::mutex> lock(fenceIdMutex_);
    if (fenceId_ > FENCE_MAX_ID) {
        fenceId_ = 0;
    }
    fenceId_++;
    std::int32_t id = fenceId_;
    return id;
}

LocationErrCode GnssAbility::AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    int fenceId = GenerateFenceId();
    request->SetFenceId(fenceId);
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    sptr<IGeofenceInterface> geofenceInterface = IGeofenceInterface::Get();
    if (geofenceInterface == nullptr) {
        LBSLOGE(GNSS, "geofenceInterface is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto geofence = request->GetGeofence();
    GeofenceInfo fenceInfo;
    fenceInfo.fenceIndex = fenceId;
    fenceInfo.latitude = geofence.latitude;
    fenceInfo.longitude = geofence.longitude;
    fenceInfo.radius = geofence.radius;
    auto transitionList = request->GetGeofenceTransitionEventList();
    uint32_t monitorEvent = 0;
    for (size_t i = 0; i < transitionList.size(); i++) {
        GeofenceTransitionEvent status = transitionList[i];
        monitorEvent |= static_cast<uint32_t>(status);
    }
    int32_t ret = geofenceInterface->AddGnssGeofence(fenceInfo, monitorEvent);
    LBSLOGI(GNSS, "Successfully AddGnssGeofence! ret:%{public}d,fenceId:%{public}s",
        ret, std::to_string(fenceId).c_str());
#endif
    RegisterGnssGeofenceCallback(request, request->GetGeofenceTransitionCallback());
    if (ExecuteFenceProcess(GnssInterfaceCode::ADD_GNSS_GEOFENCE, request)) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_NOT_SUPPORTED;
}

bool GnssAbility::RegisterGnssGeofenceCallback(std::shared_ptr<GeofenceRequest> &request,
    const sptr<IRemoteObject>& callback)
{
    if (request == nullptr) {
        LBSLOGE(GNSS, "register an invalid request");
        return false;
    }
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid callback");
        return false;
    }
    auto geofence = request->GetGeofence();
    request->SetRequestExpirationTime(CommonUtils::GetSinceBootTime() + geofence.expiration * MILL_TO_NANOS);
    std::unique_lock<ffrt::mutex> lock(gnssGeofenceRequestMapMutex_);
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) GnssGeofenceCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    if (gnssGeofenceRequestMap_.size() <= MAX_GNSS_GEOFENCE_REQUEST_NUM) {
        gnssGeofenceRequestMap_.insert(std::make_pair(request, std::make_pair(callback, death)));
    } else {
        LBSLOGE(GNSS, "RegisterGnssGeofenceCallback num max");
        return false;
    }
    LBSLOGI(GNSS, "After RegisterGnssGeofenceCallback size %{public}zu",
        gnssGeofenceRequestMap_.size());
    return true;
}

LocationErrCode GnssAbility::RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    if (request == nullptr) {
        LBSLOGE(GNSS, "request is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!CheckBundleNameInGnssGeofenceRequestMap(request->GetBundleName(), request->GetFenceId())) {
        LBSLOGE(GNSS, "bundleName is not registered");
        return ERRCODE_GEOFENCE_INCORRECT_ID;
    }
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    sptr<IGeofenceInterface> geofenceInterface = IGeofenceInterface::Get();
    if (geofenceInterface == nullptr) {
        LBSLOGE(GNSS, "geofenceInterface is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int32_t ret = geofenceInterface->DeleteGnssGeofence(request->GetFenceId());
    LBSLOGI(GNSS, "Successfully DeleteGnssGeofence! ret:%{public}d,fenceId:%{public}s",
        ret, std::to_string(request->GetFenceId()).c_str());
#endif

    if (ExecuteFenceProcess(GnssInterfaceCode::REMOVE_GNSS_GEOFENCE, request)) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_NOT_SUPPORTED;
}

bool GnssAbility::UnregisterGnssGeofenceCallback(int fenceId)
{
    for (auto iter = gnssGeofenceRequestMap_.begin(); iter != gnssGeofenceRequestMap_.end();) {
        auto requestInMap = iter->first;
        auto fenceIdInMap = requestInMap->GetFenceId();
        auto callbackPair = iter->second;
        auto callback = callbackPair.first;
        if (fenceId == fenceIdInMap) {
            if (callback != nullptr) {
                callback->RemoveDeathRecipient(callbackPair.second);
            }
            iter = gnssGeofenceRequestMap_.erase(iter);
            break;
        } else {
            iter++;
        }
    }
    LBSLOGI(GNSS, "After UnregisterGnssGeofenceCallback size:%{public}s",
        std::to_string(gnssGeofenceRequestMap_.size()).c_str());
    return true;
}

bool GnssAbility::CheckBundleNameInGnssGeofenceRequestMap(const std::string& bundleName, int fenceId)
{
    std::unique_lock<ffrt::mutex> lock(gnssGeofenceRequestMapMutex_);
    for (auto iter = gnssGeofenceRequestMap_.begin();
        iter != gnssGeofenceRequestMap_.end(); iter++) {
        auto requestInMap = iter->first;
        auto packageName = requestInMap->GetBundleName();
        auto fenceIdInMap = requestInMap->GetFenceId();
        if (packageName.compare(bundleName) == 0 && fenceId == fenceIdInMap) {
            return true;
        }
    }
    return false;
}

bool GnssAbility::RemoveGnssGeofenceRequestByCallback(sptr<IRemoteObject> callbackObj)
{
    if (callbackObj == nullptr) {
        return false;
    }
    std::unique_lock<ffrt::mutex> lock(gnssGeofenceRequestMapMutex_);
    for (auto iter = gnssGeofenceRequestMap_.begin(); iter != gnssGeofenceRequestMap_.end();) {
        auto callbackPair = iter->second;
        auto callback = callbackPair.first;
        if (callback == callbackObj) {
            callback->RemoveDeathRecipient(callbackPair.second);
            auto requestInMap = iter->first;
            requestInMap->SetAppAliveStatus(false);
            break;
        } else {
            iter++;
        }
    }
    LBSLOGD(GNSS, "After RemoveGnssGeofenceRequestByCallback size:%{public}s",
        std::to_string(gnssGeofenceRequestMap_.size()).c_str());
    return true;
}

#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
void GnssAbility::ReportGeofenceOperationResult(
    int fenceId, GeofenceOperateType type, GeofenceOperateResult result)
{
    std::unique_lock<ffrt::mutex> lock(gnssGeofenceRequestMapMutex_);
    auto geofenceRequest = GetGeofenceRequestByFenceId(fenceId);
    if (geofenceRequest == nullptr) {
        LBSLOGE(GNSS, "request is nullptr");
        return;
    }
    auto callback = geofenceRequest->GetGeofenceTransitionCallback();
    if (callback == nullptr) {
        LBSLOGE(GNSS, "callback is nullptr");
        return;
    }
    auto appAliveStatus = geofenceRequest->GetAppAliveStatus();
    if (!appAliveStatus) {
        LBSLOGE(GNSS, "app alive status is false");
        return;
    }
    sptr<IGnssGeofenceCallback> gnssGeofenceCallback = iface_cast<IGnssGeofenceCallback>(callback);
    gnssGeofenceCallback->OnReportOperationResult(
        fenceId, static_cast<int>(type), static_cast<int>(result));
    if (type == GeofenceOperateType::TYPE_DELETE) {
        UnregisterGnssGeofenceCallback(fenceId);
    }
}
#endif

#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
void GnssAbility::ReportGeofenceEvent(int fenceIndex, GeofenceEvent event)
{
    std::unique_lock<ffrt::mutex> lock(gnssGeofenceRequestMapMutex_);
    auto request = GetGeofenceRequestByFenceId(fenceIndex);
    if (request == nullptr) {
        LBSLOGE(GNSS, "request is nullptr");
        return;
    }
    if (CommonUtils::GetSinceBootTime() > request->GetRequestExpirationTime()) {
        LBSLOGE(GNSS, "request is expiration");
        if (gnssHandler_ != nullptr) {
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
                static_cast<uint32_t>(GnssAbilityInterfaceCode::REMOVE_GEOFENCE), request);
            gnssHandler_->SendEvent(event);
        }
        return;
    }
    auto callback = request->GetGeofenceTransitionCallback();
    if (callback == nullptr) {
        LBSLOGE(GNSS, "callback is nullptr");
        return;
    }
    sptr<IGnssGeofenceCallback> gnssGeofenceCallback = iface_cast<IGnssGeofenceCallback>(callback);
    auto transitionStatusList = request->GetGeofenceTransitionEventList();
    auto appAliveStatus = request->GetAppAliveStatus();
    for (size_t i = 0; i < transitionStatusList.size(); i++) {
        if (transitionStatusList[i] !=
            static_cast<GeofenceTransitionEvent>(event)) {
            continue;
        }
        if (appAliveStatus) {
            GeofenceTransition geofenceTransition;
            geofenceTransition.fenceId = fenceIndex;
            geofenceTransition.event = transitionStatusList[i];
            gnssGeofenceCallback->OnTransitionStatusChange(geofenceTransition);
        }
#ifdef NOTIFICATION_ENABLE
        auto notificationRequestList = request->GetNotificationRequestList();
        if (transitionStatusList.size() == notificationRequestList.size()) {
            auto notificationRequest = notificationRequestList[i];
            notificationRequest.SetCreatorUid(request->GetUid());
            Notification::NotificationHelper::PublishNotification(notificationRequest);
        } else {
            LBSLOGE(GNSS, "transitionStatusList size does not equals to notificationRequestList size");
        }
#endif
    }
}
#endif

std::shared_ptr<GeofenceRequest> GnssAbility::GetGeofenceRequestByFenceId(int fenceId)
{
    for (auto iter = gnssGeofenceRequestMap_.begin(); iter != gnssGeofenceRequestMap_.end(); iter++) {
        auto request = iter->first;
        if (request->GetFenceId() == fenceId) {
            return request;
        }
    }
    LBSLOGE(GNSS, "can not get geofence request by fenceId, fenceId:%{public}d",
        fenceId);
    return nullptr;
}

bool GnssAbility::ExecuteFenceProcess(
    GnssInterfaceCode code, std::shared_ptr<GeofenceRequest>& request)
{
    FenceStruct fenceStruct;
    fenceStruct.request = request;
    fenceStruct.requestCode = static_cast<int>(code);
    fenceStruct.retCode = true;
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    std::unique_lock<ffrt::mutex> lock(hdiMutex_, std::defer_lock);
    lock.lock();
    fenceStruct.callback = geofenceCallback_;
    lock.unlock();
#endif
    HookUtils::ExecuteHook(
        LocationProcessStage::FENCE_REQUEST_PROCESS, (void *)&fenceStruct, nullptr);
    return fenceStruct.retCode;
}

#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
bool GnssAbility::SetGeofenceCallback()
{
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED) {
        LBSLOGE(GNSS, "QuerySwitchState is DISABLED");
        return false;
    }
    sptr<IGeofenceInterface> geofenceInterface = IGeofenceInterface::Get();
    if (geofenceInterface == nullptr) {
        LBSLOGE(GNSS, "geofenceInterface get failed");
        return false;
    }
    int32_t ret = geofenceInterface->SetGeofenceCallback(geofenceCallback_);
    LBSLOGD(GNSS, "set geofence callback, ret:%{public}d", ret);
    if (!ret) {
        return false;
    }
    return true;
}
#endif

void GnssAbility::ReportGnssSessionStatus(int status)
{
}

void GnssAbility::ReportNmea(int64_t timestamp, const std::string &nmea)
{
    std::unique_lock<ffrt::mutex> lock(nmeaMutex_);
    for (const auto& pair : nmeaCallbackMap_) {
        auto callback = pair.first;
        sptr<INmeaMessageCallback> nmeaCallback = iface_cast<INmeaMessageCallback>(callback);
        AppIdentity nmeaIdentity = pair.second;
        if (CommonUtils::IsAppBelongCurrentAccount(nmeaIdentity)) {
            nmeaCallback->OnMessageChange(timestamp, nmea);
        }
    }
}

void GnssAbility::ReportSv(const std::unique_ptr<SatelliteStatus> &sv)
{
    std::unique_lock<ffrt::mutex> lock(gnssMutex_);
    for (const auto& pair : gnssStatusCallbackMap_) {
        auto callback = pair.first;
        sptr<IGnssStatusCallback> gnssStatusCallback = iface_cast<IGnssStatusCallback>(callback);
        AppIdentity gnssStatusIdentity = pair.second;
        if (CommonUtils::IsAppBelongCurrentAccount(gnssStatusIdentity)) {
            gnssStatusCallback->OnStatusChange(sv);
        }
    }
}

bool GnssAbility::EnableGnss()
{
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED) {
        LBSLOGE(GNSS, "QuerySwitchState is DISABLED");
        return false;
    }
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface is nullptr");
        return false;
    }
    if (IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been enabled");
        return false;
    }
    std::unique_lock<ffrt::mutex> lock(hdiMutex_, std::defer_lock);
    lock.lock();
    if (gnssCallback_ == nullptr) {
        LBSLOGE(GNSS, "gnssCallback_ is nullptr");
        lock.unlock();
        return false;
    }
    int32_t ret = gnssInterface->EnableGnss(gnssCallback_);
    lock.unlock();
    LBSLOGD(GNSS, "Successfully enable_gnss!, %{public}d", ret);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_ON;
    } else {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret),
            "hdiName", "EnableGnss", "hdiType", "gnss"});
    }
    return true;
}

void GnssAbility::DisableGnss()
{
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    int ret = gnssInterface->DisableGnss();
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    } else {
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret),
            "hdiName", "DisableGnss", "hdiType", "gnss"});
    }
}

bool GnssAbility::IsGnssEnabled()
{
    return (gnssWorkingStatus_ != GNSS_WORKING_STATUS_ENGINE_OFF &&
        gnssWorkingStatus_ != GNSS_WORKING_STATUS_NONE);
}

void GnssAbility::RestGnssWorkStatus()
{
    std::unique_lock<ffrt::mutex> uniqueLock(statusMutex_);
    gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
}

void GnssAbility::StartGnss()
{
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED) {
        LBSLOGE(GNSS, "QuerySwitchState is DISABLED");
        return;
    }
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    if (gnssWorkingStatus_ == GNSS_WORKING_STATUS_SESSION_BEGIN) {
        LBSLOGD(GNSS, "GNSS navigation started");
        return;
    }
    if (GetRequestNum() == 0) {
        return;
    }
    SetPositionMode();
    int ret = gnssInterface->StartGnss(GNSS_START_TYPE_NORMAL);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_BEGIN;
        WriteLocationInnerEvent(START_GNSS, {});
    } else {
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret), "hdiName", "StartGnss", "hdiType", "gnss"});
    }
    LocationErrCode errCode =
        HookUtils::ExecuteHook(LocationProcessStage::START_GNSS_PROCESS, nullptr, nullptr);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(GNSS, "%{public}s ExecuteHook failed err = %{public}d", __func__, (int)errCode);
    }
}

void GnssAbility::StopGnss()
{
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }

    int ret = gnssInterface->StopGnss(GNSS_START_TYPE_NORMAL);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_END;
        WriteLocationInnerEvent(STOP_GNSS, {});
    } else {
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret), "hdiName", "StopGnss", "hdiType", "gnss"});
    }
    LocationErrCode errCode =
        HookUtils::ExecuteHook(LocationProcessStage::STOP_GNSS_PROCESS, nullptr, nullptr);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(GNSS, "%{public}s ExecuteHook failed err = %{public}d", __func__, (int)errCode);
    }
}

bool GnssAbility::IsDeviceLoaded(const std::string &servName)
{
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    std::vector<OHOS::HDI::DeviceManager::V1_0::HdiDevHostInfo> deviceInfos;
    int ret = devmgr->ListAllDevice(deviceInfos);
    if (ret != HDF_SUCCESS) {
        LBSLOGE(GNSS, "get listAllDevice failed");
        return false;
    }
    auto itDevicesInfo = deviceInfos.begin();
    for (;itDevicesInfo != deviceInfos.end(); itDevicesInfo++) {
        if (itDevicesInfo->hostName == LOCATION_HOST_NAME) {
            break;
        }
    }
    if (itDevicesInfo == deviceInfos.end()) {
        LBSLOGE(GNSS, "The host is not found:%{public}s", LOCATION_HOST_NAME);
        return false;
    }
    auto itDevInfo = itDevicesInfo->devInfo.begin();
    for (;itDevInfo != itDevicesInfo->devInfo.end(); itDevInfo++) {
        if (itDevInfo->servName == servName) {
            break;
        }
    }
    if (itDevInfo == itDevicesInfo->devInfo.end()) {
        LBSLOGE(GNSS, "The devices is not found:%{public}s in host %{public}s", servName.c_str(), LOCATION_HOST_NAME);
        return false;
    }
    std::unique_lock<ffrt::mutex> lock(hdiMutex_, std::defer_lock);
    LBSLOGD(GNSS, "check host:%{public}s dev:%{public}s loaded",
        itDevicesInfo->hostName.c_str(), itDevInfo->servName.c_str());
    return true;
}

bool GnssAbility::ConnectGnssHdi()
{
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    if (!IsDeviceLoaded(GNSS_SERVICE_NAME)) {
        if (devmgr->LoadDevice(GNSS_SERVICE_NAME) != HDF_SUCCESS) {
            LBSLOGE(GNSS, "Load gnss service failed!");
            return false;
        }
    }
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "gnssInterface get failed");
        return false;
    }
    std::unique_lock<ffrt::mutex> lock(hdiMutex_, std::defer_lock);
    lock.lock();
    if (gnssCallback_ == nullptr) {
        gnssCallback_ = new (std::nothrow) GnssEventCallback();
    }
    lock.unlock();
    RegisterLocationHdiDeathRecipient();
    LBSLOGI(GNSS, "ConnectGnssHdi success");
    return true;
}
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
bool GnssAbility::ConnectAgnssHdi()
{
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    if (!IsDeviceLoaded(AGNSS_SERVICE_NAME)) {
        if (devmgr->LoadDevice(AGNSS_SERVICE_NAME) != HDF_SUCCESS) {
            LBSLOGE(GNSS, "Load agnss service failed!");
            return false;
        }
    }
    sptr<IAGnssInterface> agnssInterface = IAGnssInterface::Get();
    if (agnssInterface == nullptr) {
        LBSLOGE(GNSS, "agnssInterface get failed");
        return false;
    }
    std::unique_lock<ffrt::mutex> lock(hdiMutex_);
    if (agnssCallback_ == nullptr) {
        agnssCallback_ = new (std::nothrow) AGnssEventCallback();
    }
    LBSLOGI(GNSS, "ConnectAgnssHdi success");
    return true;
}
#endif
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
bool GnssAbility::ConnectGeofenceHdi()
{
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    if (!IsDeviceLoaded(GEOFENCE_SERVICE_NAME)) {
        if (devmgr->LoadDevice(GEOFENCE_SERVICE_NAME) != HDF_SUCCESS) {
            LBSLOGE(GNSS, "Load geofence service failed!");
            return false;
        }
    }
    std::unique_lock<ffrt::mutex> lock(hdiMutex_);
    if (geofenceCallback_ == nullptr) {
        geofenceCallback_ = sptr<GeofenceEventCallback>(new (std::nothrow) GeofenceEventCallback);
    }
    bool ret = SetGeofenceCallback();
    if (!ret) {
        LBSLOGE(GNSS, "ConnectGeofenceHdi fail");
        return false;
    }
    LBSLOGI(GNSS, "ConnectGeofenceHdi success");
    return true;
}
#endif

bool GnssAbility::ConnectHdi()
{
    if (!ConnectGnssHdi()) {
        return false;
    }
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    if (!ConnectAgnssHdi()) {
        return false;
    }
#endif
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    if (!ConnectGeofenceHdi()) {
        return false;
    }
#endif
    LBSLOGI(GNSS, "connect v2_0 hdi success.");
    return true;
}

bool GnssAbility::RemoveHdi()
{
    std::unique_lock<ffrt::mutex> lock(hdiMutex_);
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    if (devmgr->UnloadDevice(GNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Unload gnss service failed!");
        return false;
    }
    gnssCallback_ = nullptr;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    if (devmgr->UnloadDevice(AGNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Unload agnss service failed!");
        return false;
    }
    agnssCallback_ = nullptr;
#endif
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    if (devmgr->UnloadDevice(GEOFENCE_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Unload geofence service failed!");
        return false;
    }
    geofenceCallback_ = nullptr;
#endif
    LBSLOGI(GNSS, "RemoveHdi success.");
    return true;
}

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
void GnssAbility::SetAgnssServer()
{
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    std::string addrName;
    bool result = LocationConfigManager::GetInstance()->GetAgnssServerAddr(addrName);
    if (!result || addrName.empty()) {
        LBSLOGE(GNSS, "get agnss server address failed!");
        return;
    }
    int port = LocationConfigManager::GetInstance()->GetAgnssServerPort();
    sptr<IAGnssInterface> agnssInterface = IAGnssInterface::Get();
    if (agnssInterface == nullptr) {
        LBSLOGE(GNSS, "agnssInterface is nullptr");
        return;
    }
    AGnssServerInfo info;
    info.type = AGNSS_TYPE_SUPL;
    info.server = addrName;
    info.port = port;
    agnssInterface->SetAgnssServer(info);
}

void GnssAbility::SetAgnssCallback()
{
    LBSLOGD(GNSS, "enter SetAgnssCallback");
    std::unique_lock<ffrt::mutex> lock(hdiMutex_);
    sptr<IAGnssInterface> agnssInterface = IAGnssInterface::Get();
    if (agnssInterface == nullptr || agnssCallback_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface or agnssCallback_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    agnssInterface->SetAgnssCallback(agnssCallback_);
}

void GnssAbility::SetSetId(const SubscriberSetId& id)
{
    std::unique_ptr<SubscriberSetId> subscribeSetId = std::make_unique<SubscriberSetId>();
    subscribeSetId->type = id.type;
    subscribeSetId->id = id.id;
    if (gnssHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            static_cast<uint32_t>(GnssAbilityInterfaceCode::SET_SUBSCRIBER_SET_ID), subscribeSetId);
        gnssHandler_->SendEvent(event);
    }
}

void GnssAbility::SetSetIdImpl(const SubscriberSetId& id)
{
    sptr<IAGnssInterface> agnssInterface = IAGnssInterface::Get();
    if (agnssInterface == nullptr) {
        LBSLOGE(GNSS, "agnssInterface is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    agnssInterface->SetSubscriberSetId(id);
}

void GnssAbility::SetRefInfo(const AGnssRefInfo& refInfo)
{
    std::unique_ptr<AgnssRefInfoMessage> agnssRefInfoMessage = std::make_unique<AgnssRefInfoMessage>();
    if (gnssHandler_ != nullptr) {
        agnssRefInfoMessage->SetAgnssRefInfo(refInfo);
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            static_cast<uint32_t>(GnssAbilityInterfaceCode::SET_AGNSS_REF_INFO), agnssRefInfoMessage);
        gnssHandler_->SendEvent(event);
    }
}

void GnssAbility::SetRefInfoImpl(const AGnssRefInfo &refInfo)
{
    sptr<IAGnssInterface> agnssInterface = IAGnssInterface::Get();
    if (agnssInterface == nullptr) {
        LBSLOGE(GNSS, "agnssInterface is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    agnssInterface->SetAgnssRefInfo(refInfo);
}

AGnssRefInfo AgnssRefInfoMessage::GetAgnssRefInfo()
{
    return agnssRefInfo_;
}

void AgnssRefInfoMessage::SetAgnssRefInfo(const AGnssRefInfo &refInfo)
{
    agnssRefInfo_ = refInfo;
}
#endif

void GnssAbility::SaDumpInfo(std::string& result)
{
    result += "Gnss Location enable status: true";
    result += "\n";
}

int32_t GnssAbility::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.GnssDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(GNSS, "Gnss save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}

LocationErrCode GnssAbility::EnableMock()
{
    if (!EnableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    MockLocationStruct mockLocationStruct;
    mockLocationStruct.enableMock = true;
    HookUtils::ExecuteHook(LocationProcessStage::MOCK_LOCATION_PROCESS, (void *)&mockLocationStruct, nullptr);
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::DisableMock()
{
    if (!DisableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    MockLocationStruct mockLocationStruct;
    mockLocationStruct.enableMock = false;
    HookUtils::ExecuteHook(LocationProcessStage::MOCK_LOCATION_PROCESS, (void *)&mockLocationStruct, nullptr);
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::SetMocked(const int timeInterval,
    const std::vector<std::shared_ptr<Location>> &location)
{
    if (!SetMockedLocations(timeInterval, location)) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::QuerySupportCoordinateSystemType(
    std::vector<CoordinateSystemType>& coordinateSystemTypes)
{
    std::vector<CoordinateSystemType> supportedTypes;
    supportedTypes.push_back(CoordinateSystemType::WGS84);
    FenceStruct fenceStruct;
    fenceStruct.requestCode =
        static_cast<int>(GnssInterfaceCode::GET_GEOFENCE_SUPPORT_COORDINATE_SYSTEM_TYPE);
    fenceStruct.coordinateSystemTypes = supportedTypes;
    HookUtils::ExecuteHook(
        LocationProcessStage::FENCE_REQUEST_PROCESS, (void *)&fenceStruct, nullptr);
    coordinateSystemTypes = fenceStruct.coordinateSystemTypes;

    return ERRCODE_SUCCESS;
}

bool GnssAbility::IsMockEnabled()
{
    return IsLocationMocked();
}

bool GnssAbility::IsMockProcessing()
{
    auto loc = GetLocationMock();
    return !loc.empty();
}

void GnssAbility::ProcessReportLocationMock()
{
    std::vector<std::shared_ptr<Location>> mockLocationArray = GetLocationMock();
    if (mockLocationIndex_ < mockLocationArray.size()) {
        ReportMockedLocation(mockLocationArray[mockLocationIndex_++]);
        if (gnssHandler_ != nullptr) {
            gnssHandler_->SendHighPriorityEvent(
                static_cast<uint32_t>(GnssAbilityInterfaceCode::EVENT_REPORT_MOCK_LOCATION),
                0, GetTimeIntervalMock() * EVENT_INTERVAL_UNITE);
        }
    } else {
        ClearLocationMock();
        mockLocationIndex_ = 0;
    }
}

void GnssAbility::SendReportMockLocationEvent()
{
    if (gnssHandler_ != nullptr) {
        gnssHandler_->SendHighPriorityEvent(static_cast<uint32_t>(
            GnssAbilityInterfaceCode::EVENT_REPORT_MOCK_LOCATION), 0, 0);
    }
}

int32_t GnssAbility::ReportMockedLocation(const std::shared_ptr<Location> location)
{
    if ((IsLocationMocked() && !location->GetIsFromMock()) ||
        (!IsLocationMocked() && location->GetIsFromMock())) {
        LBSLOGE(GNSS, "location mock is enabled, do not report gnss location!");
        return ERR_OK;
    }
    location->SetTimeSinceBoot(CommonUtils::GetSinceBootTime());
    location->SetTimeStamp(CommonUtils::GetCurrentTimeStamp() * MICRO_PER_MILLI);
    location->SetLocationSourceType(LocationSourceType::GNSS_TYPE);
    ReportLocationInfo(GNSS_ABILITY, location);
#ifdef FEATURE_PASSIVE_SUPPORT
    ReportLocationInfo(PASSIVE_ABILITY, location);
#endif
    return ERR_OK;
}

void GnssAbility::SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    if (gnssHandler_ == nullptr) {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return;
    }
    switch (code) {
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST): {
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
                Get(code, workrecord);
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS): {
            if (!IsMockEnabled()) {
                reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
                break;
            }
            int timeInterval = data.ReadInt32();
            int locationSize = data.ReadInt32();
            timeInterval = timeInterval < 0 ? 1 : timeInterval;
            locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
                locationSize;
            std::shared_ptr<std::vector<std::shared_ptr<Location>>> vcLoc =
                std::make_shared<std::vector<std::shared_ptr<Location>>>();
            for (int i = 0; i < locationSize; i++) {
                vcLoc->push_back(Location::UnmarshallingShared(data));
            }
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, vcLoc, timeInterval);
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS): {
            std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
            locationCommand->scenario = data.ReadInt32();
            locationCommand->command = Str16ToStr8(data.ReadString16());
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, locationCommand);
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE): {
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, static_cast<int>(data.ReadBool()));
            SendEvent(event, reply);
            break;
        }
#ifdef NOTIFICATION_ENABLE
        case static_cast<uint32_t>(GnssAbilityInterfaceCode::ADD_FENCE): {
            auto request = GeofenceRequest::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, request);
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssAbilityInterfaceCode::REMOVE_FENCE): {
            auto request = GeofenceRequest::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, request);
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssAbilityInterfaceCode::ADD_GEOFENCE): {
            auto request = GeofenceRequest::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, request);
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssAbilityInterfaceCode::REMOVE_GEOFENCE): {
            std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
            request->SetFenceId(data.ReadInt32());
            request->SetBundleName(data.ReadString());
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, request);
            SendEvent(event, reply);
            break;
        }
#endif
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_NETWORK_LOCATION): {
            LBSLOGI(GNSS, "%{public}s: send network location", __func__);
            auto request = Location::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, request);
            SendEvent(event, reply);
            break;
        }
        default:
            break;
    }
}

void GnssAbility::SendEvent(AppExecFwk::InnerEvent::Pointer& event, MessageParcel &reply)
{
    if (gnssHandler_ == nullptr) {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return;
    }
    if (gnssHandler_->SendEvent(event)) {
        reply.WriteInt32(ERRCODE_SUCCESS);
    } else {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
    }
}

void GnssAbility::RegisterLocationHdiDeathRecipient()
{
    sptr<IGnssInterface> gnssInterface = IGnssInterface::Get();
    if (gnssInterface == nullptr) {
        LBSLOGE(GNSS, "%{public}s: gnssInterface is nullptr", __func__);
        return;
    }
    sptr<IRemoteObject> obj = OHOS::HDI::hdi_objcast<IGnssInterface>(gnssInterface);
    if (obj == nullptr) {
        LBSLOGE(GNSS, "%{public}s: hdi obj is nullptr", __func__);
        return;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) LocationHdiDeathRecipient());
    obj->AddDeathRecipient(death);
}

GnssHandler::GnssHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitGnssEventProcessMap();
}

void GnssHandler::InitGnssEventProcessMap()
{
    if (gnssEventProcessMap_.size() != 0) {
        return;
    }
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::EVENT_REPORT_MOCK_LOCATION)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleReportMockLocation(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleSendLocationRequest(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleSetMockedLocations(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleSendCommands(event); };
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::SET_SUBSCRIBER_SET_ID)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleSetSubscriberSetId(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::SET_AGNSS_REF_INFO)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleSetAgnssRefInfo(event); };
#endif
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::RECONNECT_HDI)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleReconnectHdi(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleSetEnable(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::INIT_HDI)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleInitHdi(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::ADD_FENCE)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleAddFence(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::REMOVE_FENCE)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleRemoveFence(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::ADD_GEOFENCE)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleAddGeofence(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssAbilityInterfaceCode::REMOVE_GEOFENCE)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleRemoveGeofence(event); };
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_NETWORK_LOCATION)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleSendNetworkLocation(event); };
}

GnssHandler::~GnssHandler() {}

void GnssHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = GnssAbility::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(GNSS, "ProcessEvent event:%{public}d", eventId);
    auto handleFunc = gnssEventProcessMap_.find(eventId);
    if (handleFunc != gnssEventProcessMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
#ifdef LOCATION_HICOLLIE_ENABLE
        int tid = gettid();
        std::string moduleName = "GnssHandler";
        XCollieCallback callbackFunc = [moduleName, eventId, tid](void *) {
            LBSLOGE(GNSS, "TimeoutCallback tid:%{public}d moduleName:%{public}s excute eventId:%{public}u timeout.",
                tid, moduleName.c_str(), eventId);
        };
        std::string dfxInfo = moduleName + "_" + std::to_string(eventId) + "_" + std::to_string(tid);
        int timerId = HiviewDFX::XCollie::GetInstance().SetTimer(dfxInfo, TIMEOUT_WATCHDOG, callbackFunc, nullptr,
            HiviewDFX::XCOLLIE_FLAG_LOG|HiviewDFX::XCOLLIE_FLAG_RECOVERY);
        memberFunc(event);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
#else
        memberFunc(event);
#endif
    }
    gnssAbility->UnloadGnssSystemAbility();
}

void GnssHandler::HandleReportMockLocation(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility->ProcessReportLocationMock();
}

void GnssHandler::HandleSendLocationRequest(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = GnssAbility::GetInstance();
    std::unique_ptr<WorkRecord> workrecord = event->GetUniqueObject<WorkRecord>();
    if (workrecord != nullptr) {
        gnssAbility->LocationRequest(*workrecord);
    }
}

void GnssHandler::HandleSetMockedLocations(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = GnssAbility::GetInstance();
    int timeInterval = event->GetParam();
    auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
    if (vcLoc != nullptr) {
        std::vector<std::shared_ptr<Location>> mockLocations;
        for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
            mockLocations.push_back(*it);
        }
        gnssAbility->SetMocked(timeInterval, mockLocations);
    }
}

void GnssHandler::HandleSendCommands(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = GnssAbility::GetInstance();
    std::unique_ptr<LocationCommand> locationCommand = event->GetUniqueObject<LocationCommand>();
    if (locationCommand != nullptr) {
        gnssAbility->SendCommand(locationCommand);
    }
}

void GnssHandler::HandleSetSubscriberSetId(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = GnssAbility::GetInstance();
    std::unique_ptr<SubscriberSetId> subscriberSetId = event->GetUniqueObject<SubscriberSetId>();
    if (subscriberSetId != nullptr) {
        gnssAbility->SetSetIdImpl(*subscriberSetId);
    }
}
} // namespace Location
} // namespace OHOS
#endif