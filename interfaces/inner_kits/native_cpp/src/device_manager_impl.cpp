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

#include "device_manager_impl.h"
#include <random>
#include "device_manager_notify.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_dfx_constants.h"
#include "dm_hisysevent.h"
#include "dm_hitrace.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "dm_random.h"
#include "ipc_acl_profile_req.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_bind_device_req.h"
#include "ipc_bind_target_req.h"
#include "ipc_check_access_control.h"
#include "ipc_common_param_req.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_destroy_pin_holder_req.h"
#include "ipc_export_auth_code_rsp.h"
#include "ipc_generate_encrypted_uuid_req.h"
#include "ipc_get_anony_local_udid_rsp.h"
#include "ipc_get_device_icon_info_req.h"
#include "ipc_get_device_info_rsp.h"
#include "ipc_get_device_network_id_list_req.h"
#include "ipc_get_device_network_id_list_rsp.h"
#include "ipc_get_device_profile_info_list_req.h"
#include "ipc_get_device_screen_status_req.h"
#include "ipc_get_device_screen_status_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_local_device_name_rsp.h"
#include "ipc_get_local_display_device_name_req.h"
#include "ipc_get_local_display_device_name_rsp.h"
#include "ipc_get_localserviceinfo_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_import_auth_code_req.h"
#include "ipc_notify_event_req.h"
#include "ipc_permission_req.h"
#include "ipc_publish_req.h"
#include "ipc_put_device_profile_info_list_req.h"
#include "ipc_register_serviceinfo_req.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_set_local_device_name_req.h"
#include "ipc_set_remote_device_name_req.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_skeleton.h"
#include "ipc_sync_callback_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_unbind_device_req.h"
#include "ipc_unpublish_req.h"
#include "securec.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ipc_model_codec.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif
namespace OHOS {
namespace DistributedHardware {

constexpr const char* DM_INIT_DEVICE_MANAGER_SUCCESS = "DM_INIT_DEVICE_MANAGER_SUCCESS";
constexpr const char* DM_INIT_DEVICE_MANAGER_FAILED = "DM_INIT_DEVICE_MANAGER_FAILED";
constexpr const char* START_DEVICE_DISCOVERY_SUCCESS = "START_DEVICE_DISCOVERY_SUCCESS";
constexpr const char* GET_LOCAL_DEVICE_INFO_SUCCESS = "GET_LOCAL_DEVICE_INFO_SUCCESS";
constexpr const char* GET_LOCAL_DEVICE_INFO_FAILED = "GET_LOCAL_DEVICE_INFO_FAILED";
constexpr const char* DM_SEND_REQUEST_SUCCESS = "DM_SEND_REQUEST_SUCCESS";
constexpr const char* DM_SEND_REQUEST_FAILED = "DM_SEND_REQUEST_FAILED";
constexpr const char* UNAUTHENTICATE_DEVICE_SUCCESS = "UNAUTHENTICATE_DEVICE_SUCCESS";
constexpr const char* UNAUTHENTICATE_DEVICE_FAILED = "UNAUTHENTICATE_DEVICE_FAILED";
constexpr const char* DM_INIT_DEVICE_MANAGER_SUCCESS_MSG = "init devicemanager success.";
constexpr const char* DM_INIT_DEVICE_MANAGER_FAILED_MSG = "init devicemanager failed.";
constexpr const char* START_DEVICE_DISCOVERY_SUCCESS_MSG = "device manager discovery success.";
constexpr const char* GET_LOCAL_DEVICE_INFO_SUCCESS_MSG = "get local device info success.";
constexpr const char* GET_LOCAL_DEVICE_INFO_FAILED_MSG = "get local device info failed.";
constexpr const char* DM_SEND_REQUEST_SUCCESS_MSG = "send request success.";
constexpr const char* DM_SEND_REQUEST_FAILED_MSG = "send request failed.";
constexpr const char* UNAUTHENTICATE_DEVICE_SUCCESS_MSG = "unauthenticate device success.";
constexpr const char* UNAUTHENTICATE_DEVICE_FAILED_MSG = "unauthenticate device failed.";
constexpr const char* DM_HITRACE_START_DEVICE = "DM_HITRACE_START_DEVICE";
constexpr const char* DM_HITRACE_GET_LOCAL_DEVICE_INFO = "DM_HITRACE_GET_LOCAL_DEVICE_INFO";
constexpr const char* DM_HITRACE_AUTH_TO_CONSULT = "DM_HITRACE_AUTH_TO_CONSULT";
constexpr const char* DM_HITRACE_INIT = "DM_HITRACE_INIT";

const uint16_t DM_INVALID_FLAG_ID = 0;
const uint16_t DM_IMPORT_AUTH_CODE_MIN_LENGTH = 6;
const uint16_t DM_IMPORT_AUTH_CODE_MAX_LENGTH = 1024;
const int32_t NORMAL = 0;
const int32_t SYSTEM_BASIC = 1;
const int32_t SYSTEM_CORE = 2;
const int32_t USLEEP_TIME_US_100000 = 100000; // 100ms
constexpr int32_t SERVICE_INIT_MAX_NUM = 20;
constexpr int32_t DM_STRING_LENGTH_MAX = 1024;

DeviceManagerImpl &DeviceManagerImpl::GetInstance()
{
    static DeviceManagerImpl instance;
    return instance;
}

int32_t DeviceManagerImpl::InitDeviceManager(const std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback)
{
    if (pkgName.empty() || dmInitCallback == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "InitDeviceManager", ERR_DM_INPUT_PARA_INVALID, "***");
        LOGE("DeviceManagerImpl::InitDeviceManager error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmTraceStart(std::string(DM_HITRACE_INIT));
    LOGI("Start, pkgName: %{public}s", GetAnonyString(pkgName).c_str());

    int32_t ret = DM_OK;
    int32_t retryNum = 0;
    while (retryNum < SERVICE_INIT_MAX_NUM) {
        ret = ipcClientProxy_->Init(pkgName);
        if (ret == DM_OK) {
            break;
        }
        usleep(USLEEP_TIME_US_100000);
        retryNum++;
        if (retryNum == SERVICE_INIT_TRY_MAX_NUM) {
            DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "InitDeviceManager",
                ERR_DM_NOT_INIT, "***");
            LOGE("InitDeviceManager error, wait for device manager service starting timeout.");
            return ERR_DM_NOT_INIT;
        }
    }
    if (ret != DM_OK) {
        LOGE("InitDeviceManager error, proxy init failed ret: %{public}d", ret);
        SysEventWrite(std::string(DM_INIT_DEVICE_MANAGER_FAILED), DM_HISYEVENT_FAULT,
            std::string(DM_INIT_DEVICE_MANAGER_FAILED_MSG));
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "InitDeviceManager", ret, "***");
        return ERR_DM_INIT_FAILED;
    }
    if (anonyLocalUdid_.empty()) {
        GetAnonyLocalUdid(pkgName, anonyLocalUdid_);
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "InitDeviceManager", DM_OK, anonyLocalUdid_);
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    DmTraceEnd();
    LOGI("Success");
    SysEventWrite(std::string(DM_INIT_DEVICE_MANAGER_SUCCESS), DM_HISYEVENT_BEHAVIOR,
        std::string(DM_INIT_DEVICE_MANAGER_SUCCESS_MSG));
    return DM_OK;
}

int32_t DeviceManagerImpl::UnInitDeviceManager(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "UnInitDeviceManager", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("UnInitDeviceManager Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", GetAnonyString(pkgName).c_str());

    int32_t ret = ipcClientProxy_->UnInit(pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnInitDeviceManager", ret, anonyLocalUdid_);
    if (ret != DM_OK) {
        LOGE("UnInitDeviceManager error, proxy unInit failed ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }

    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    LOGI("Success");
    return DM_OK;
}

void DeviceManagerImpl::ConvertDeviceInfoToDeviceBasicInfo(const DmDeviceInfo &info,
    DmDeviceBasicInfo &deviceBasicInfo)
{
    if (memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo)) != DM_OK) {
        LOGE("ConvertDeviceInfoToDeviceBasicInfo memset_s failed.");
        return;
    }

    if (memcpy_s(deviceBasicInfo.deviceName, sizeof(deviceBasicInfo.deviceName), info.deviceName,
                 std::min(sizeof(deviceBasicInfo.deviceName), sizeof(info.deviceName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
        return;
    }

    if (memcpy_s(deviceBasicInfo.networkId, sizeof(deviceBasicInfo.networkId), info.networkId,
                 std::min(sizeof(deviceBasicInfo.networkId), sizeof(info.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
        return;
    }

    if (memcpy_s(deviceBasicInfo.deviceId, sizeof(deviceBasicInfo.deviceId), info.deviceId,
                 std::min(sizeof(deviceBasicInfo.deviceId), sizeof(info.deviceId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceId data failed.");
        return;
    }

    deviceBasicInfo.deviceTypeId = info.deviceTypeId;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                std::vector<DmDeviceInfo> &deviceList)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(
            pkgName, "GetTrustedDeviceList", deviceList, ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGD("Start, pkgName: %{public}s, extra: %{public}s", GetAnonyString(pkgName).c_str(), extra.c_str());

    std::shared_ptr<IpcGetTrustDeviceReq> req = std::make_shared<IpcGetTrustDeviceReq>();
    std::shared_ptr<IpcGetTrustDeviceRsp> rsp = std::make_shared<IpcGetTrustDeviceRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    int32_t ret = ipcClientProxy_->SendRequest(GET_TRUST_DEVICE_LIST, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetTrustedDeviceList",
            deviceList, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetTrustedDeviceList error, Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetTrustedDeviceList",
            deviceList, ret, anonyLocalUdid_);
        LOGI("GetTrustedDeviceList error, failed ret: %{public}d", ret);
        return ret;
    }

    deviceList = rsp->GetDeviceVec();
    LOGI("Completed, device size %{public}zu", deviceList.size());
    DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetTrustedDeviceList",
        deviceList, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                bool isRefresh, std::vector<DmDeviceInfo> &deviceList)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(
            pkgName, "GetTrustedDeviceList", deviceList, ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGD("Start, pkgName: %{public}s, extra: %{public}s, isRefresh: %{public}d", GetAnonyString(pkgName).c_str(),
         extra.c_str(), isRefresh);

    std::shared_ptr<IpcGetTrustDeviceReq> req = std::make_shared<IpcGetTrustDeviceReq>();
    std::shared_ptr<IpcGetTrustDeviceRsp> rsp = std::make_shared<IpcGetTrustDeviceRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    req->SetRefresh(isRefresh);
    int32_t ret = ipcClientProxy_->SendRequest(GET_TRUST_DEVICE_LIST, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetTrustedDeviceList",
            deviceList, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetTrustedDeviceList error, Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetTrustedDeviceList",
            deviceList, ret, anonyLocalUdid_);
        LOGE("GetTrustedDeviceList error, failed ret: %{public}d", ret);
        return ret;
    }
    deviceList = rsp->GetDeviceVec();
    LOGI("Completed, device size %{public}zu", deviceList.size());
    DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetTrustedDeviceList",
        deviceList, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::GetAvailableDeviceList(const std::string &pkgName,
    std::vector<DmDeviceBasicInfo> &deviceList)
{
    LOGI("Start, pkgName: %{public}s.", GetAnonyString(pkgName).c_str());
    std::vector<DmDeviceInfo> deviceListTemp;
    std::string extra = "";
    int32_t ret = GetTrustedDeviceList(pkgName, extra, false, deviceListTemp);
    DmRadarHelper::GetInstance().ReportGetTrustDeviceList(
        pkgName, "GetTrustedDeviceList", deviceListTemp, ret, anonyLocalUdid_);
    if (ret != DM_OK) {
        LOGE("DeviceManagerImpl::GetTrustedDeviceList error.");
        return ret;
    }
    for (auto &item : deviceListTemp) {
        DmDeviceBasicInfo deviceBasicInfo;
        ConvertDeviceInfoToDeviceBasicInfo(item, deviceBasicInfo);
        deviceList.push_back(deviceBasicInfo);
    }
    LOGI("Completed, device size %{public}zu", deviceList.size());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceInfo(const std::string &pkgName, const std::string networkId,
                                         DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty() || networkId.empty()) {
        DmRadarHelper::GetInstance().ReportGetDeviceInfo(
            pkgName, "GetDeviceInfo", deviceInfo, ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName: %{public}s, netWorkId: %{public}s", pkgName.c_str(),
            GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s networKId : %{public}s", pkgName.c_str(),
         GetAnonyString(networkId).c_str());
    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetDeviceInfoRsp> rsp = std::make_shared<IpcGetDeviceInfoRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(networkId);
    int32_t ret = ipcClientProxy_->SendRequest(GET_DEVICE_INFO, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetDeviceInfo(pkgName, "GetDeviceInfo",
            deviceInfo, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetDeviceInfo error, Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetDeviceInfo(pkgName, "GetDeviceInfo",
            deviceInfo, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetDeviceInfo error, failed ret: %{public}d", ret);
        return ret;
    }

    deviceInfo = rsp->GetDeviceInfo();
    LOGI("Completed, networKId = %{public}s deviceName = %{public}s", GetAnonyString(req->GetNetWorkId()).c_str(),
         GetAnonyString(deviceInfo.deviceName).c_str());
    DmRadarHelper::GetInstance().ReportGetDeviceInfo(pkgName, "GetDeviceInfo",
        deviceInfo, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info)
{
    LOGD("Start, pkgName: %{public}s", GetAnonyString(pkgName).c_str());
    DmTraceStart(std::string(DM_HITRACE_GET_LOCAL_DEVICE_INFO));
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcGetLocalDeviceInfoRsp> rsp = std::make_shared<IpcGetLocalDeviceInfoRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = ipcClientProxy_->SendRequest(GET_LOCAL_DEVICE_INFO, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceInfo",
            info, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetLocalDeviceInfo error, Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceInfo",
            info, ret, anonyLocalUdid_);
        LOGI("DeviceManagerImpl::GetLocalDeviceInfo error, failed ret: %{public}d", ret);
        SysEventWrite(std::string(GET_LOCAL_DEVICE_INFO_FAILED), DM_HISYEVENT_BEHAVIOR,
            std::string(GET_LOCAL_DEVICE_INFO_FAILED_MSG));
        return ret;
    }

    info = rsp->GetLocalDeviceInfo();
    DmTraceEnd();
    LOGI("Completed");
    SysEventWrite(std::string(GET_LOCAL_DEVICE_INFO_SUCCESS), DM_HISYEVENT_BEHAVIOR,
        std::string(GET_LOCAL_DEVICE_INFO_SUCCESS_MSG));
    DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceInfo",
        info, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
    std::shared_ptr<DeviceStateCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "RegisterDevStateCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("RegisterDevStateCallback error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterDevStateCallback",
            ERR_DM_INIT_FAILED, anonyLocalUdid_);
        LOGE("Get SystemAbilityManager Failed");
        return ERR_DM_INIT_FAILED;
    }
    while (samgr->CheckSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID) == nullptr) {
        LOGD("Access_token SA not start.");
        usleep(USLEEP_TIME_US_100000);
    }
    int32_t ret = CheckApiPermission(SYSTEM_CORE);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterDevStateCallback", ret, anonyLocalUdid_);
        LOGE("System SA not have permission, ret: %{public}d.", ret);
        return ret;
    }
#endif
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    SyncCallbackToService(DmCommonNotifyEvent::REG_DEVICE_STATE, pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterDevStateCallback", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDevStatusCallback(const std::string &pkgName, const std::string &extra,
    std::shared_ptr<DeviceStatusCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "RegisterDevStatusCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("RegisterDevStatusCallback error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterDeviceStatusCallback(pkgName, callback);
    SyncCallbackToService(DmCommonNotifyEvent::REG_DEVICE_STATE, pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterDevStatusCallback", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "UnRegisterDevStateCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("UnRegisterDevStateCallback Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    SyncCallbackToService(DmCommonNotifyEvent::UN_REG_DEVICE_STATE, pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnRegisterDevStateCallback", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDevStatusCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "UnRegisterDevStatusCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("UnRegisterDevStatusCallback Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    SyncCallbackToService(DmCommonNotifyEvent::UN_REG_DEVICE_STATE, pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStatusCallback(pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnRegisterDevStatusCallback", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}


int32_t DeviceManagerImpl::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                                const std::string &extra, std::shared_ptr<DiscoveryCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("DeviceManagerImpl::StartDeviceDiscovery error: Invalid para, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID,
        std::to_string(subscribeInfo.subscribeId)));
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_DISC_MEDIUM, std::to_string(subscribeInfo.medium)));
    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, extra));
    return StartDiscovering(pkgName, discParam, filterOps, callback);
}

int32_t DeviceManagerImpl::StartDeviceDiscovery(const std::string &pkgName, uint64_t tokenId,
    const std::string &filterOptions, std::shared_ptr<DiscoveryCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("DeviceManagerImpl::StartDeviceDiscovery error: Invalid para, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID, std::to_string(tokenId)));
    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, filterOptions));
    return StartDiscovering(pkgName, discParam, filterOps, callback);
}

int32_t DeviceManagerImpl::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerImpl::StopDeviceDiscovery Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID, std::to_string(subscribeId)));
    return StopDiscovering(pkgName, discParam);
}

int32_t DeviceManagerImpl::StopDeviceDiscovery(uint64_t tokenId, const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerImpl::StopDeviceDiscovery Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID, std::to_string(tokenId)));
    return StopDiscovering(pkgName, discParam);
}

int32_t DeviceManagerImpl::StartDiscovering(const std::string &pkgName,
    std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions,
    std::shared_ptr<DiscoveryCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("DeviceManagerImpl::StartDiscovering failed: input callback is null or pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DmTraceStart(std::string(DM_HITRACE_START_DEVICE));

    uint16_t subscribeId = AddDiscoveryCallback(pkgName, discoverParam, callback);
    discoverParam.emplace(PARAM_KEY_SUBSCRIBE_ID, std::to_string(subscribeId));
    std::string discParaStr = ConvertMapToJsonString(discoverParam);
    std::string filterOpStr = ConvertMapToJsonString(filterOptions);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(ComposeStr(pkgName, subscribeId));
    req->SetFirstParam(discParaStr);
    req->SetSecondParam(filterOpStr);
    int32_t ret = ipcClientProxy_->SendRequest(START_DISCOVERING, req, rsp);
    if (ret != DM_OK) {
        LOGE("StartDiscovering error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StartDiscovering error: Failed with ret %{public}d", ret);
        return ret;
    }

    DmTraceEnd();
    LOGI("Completed");
    SysEventWrite(std::string(START_DEVICE_DISCOVERY_SUCCESS), DM_HISYEVENT_BEHAVIOR,
        std::string(START_DEVICE_DISCOVERY_SUCCESS_MSG));
    return DM_OK;
}

int32_t DeviceManagerImpl::StopDiscovering(const std::string &pkgName,
    std::map<std::string, std::string> &discoverParam)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerImpl::StopDiscovering failed: input pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    uint16_t subscribeId = DM_INVALID_FLAG_ID;
    if (discoverParam.find(PARAM_KEY_SUBSCRIBE_ID) != discoverParam.end()) {
        subscribeId = std::atoi((discoverParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
    }
    std::string pkgNameTemp = ComposeStr(pkgName, subscribeId);
    subscribeId = GetSubscribeIdFromMap(pkgNameTemp);
    if (subscribeId == DM_INVALID_FLAG_ID) {
        LOGE("DeviceManagerImpl::StopDiscovering failed: cannot find pkgName in cache map.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    discoverParam.emplace(PARAM_KEY_SUBSCRIBE_ID, std::to_string(subscribeId));
    std::string discParaStr = ConvertMapToJsonString(discoverParam);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(ComposeStr(pkgName, subscribeId));
    req->SetFirstParam(discParaStr);
    int32_t ret = ipcClientProxy_->SendRequest(STOP_DISCOVERING, req, rsp);
    if (ret != DM_OK) {
        LOGE("StopDiscovering error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StopDiscovering error: Failed with ret %{public}d", ret);
        return ret;
    }
    RemoveDiscoveryCallback(pkgNameTemp);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo,
    std::shared_ptr<PublishCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "PublishDeviceDiscovery",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("PublishDeviceDiscovery error: pkgName %{public}s invalid para", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, pkgName %{public}s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishInfo.publishId, callback);

    std::shared_ptr<IpcPublishReq> req = std::make_shared<IpcPublishReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetPublishInfo(publishInfo);
    int32_t ret = ipcClientProxy_->SendRequest(PUBLISH_DEVICE_DISCOVER, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "PublishDeviceDiscovery", ret, anonyLocalUdid_);
        LOGE("PublishDeviceDiscovery error: Send Request failed ret: %{public}d", ret);
        DeviceManagerNotify::GetInstance().UnRegisterPublishCallback(pkgName, publishInfo.publishId);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "PublishDeviceDiscovery", ret, anonyLocalUdid_);
        LOGE("PublishDeviceDiscovery error: Failed with ret %{public}d", ret);
        return ret;
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "PublishDeviceDiscovery", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnPublishDeviceDiscovery",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, pkgName %{public}s", pkgName.c_str());
    std::shared_ptr<IpcUnPublishReq> req = std::make_shared<IpcUnPublishReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetPublishId(publishId);
    int32_t ret = ipcClientProxy_->SendRequest(UNPUBLISH_DEVICE_DISCOVER, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnPublishDeviceDiscovery", ret, anonyLocalUdid_);
        LOGE("UnPublishDeviceDiscovery error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnPublishDeviceDiscovery", ret, anonyLocalUdid_);
        LOGE("UnPublishDeviceDiscovery error: Failed with ret %{public}d", ret);
        return ret;
    }

    DeviceManagerNotify::GetInstance().UnRegisterPublishCallback(pkgName, publishId);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnPublishDeviceDiscovery", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                              const DmDeviceInfo &deviceInfo, const std::string &extra,
                                              std::shared_ptr<AuthenticateCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DmTraceStart(std::string(DM_HITRACE_AUTH_TO_CONSULT));

    std::string strDeviceId = deviceInfo.deviceId;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, strDeviceId, callback);
    JsonObject extraJson(extra);
    if (extraJson.IsDiscarded()) {
        LOGE("extra bindParam %{public}s.", extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    extraJson[TOKENID] = std::to_string(OHOS::IPCSkeleton::GetSelfTokenID());
#endif
    std::shared_ptr<IpcAuthenticateDeviceReq> req = std::make_shared<IpcAuthenticateDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(SafetyDump(extraJson));
    req->SetAuthType(authType);
    req->SetDeviceInfo(deviceInfo);
    int32_t ret = ipcClientProxy_->SendRequest(AUTHENTICATE_DEVICE, req, rsp);
    if (ret != DM_OK) {
        LOGE("AuthenticateDevice error: Send Request failed ret: %{public}d", ret);
        SysEventWrite(std::string(DM_SEND_REQUEST_FAILED), DM_HISYEVENT_BEHAVIOR,
            std::string(DM_SEND_REQUEST_FAILED_MSG));
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    SysEventWrite(std::string(DM_SEND_REQUEST_SUCCESS), DM_HISYEVENT_BEHAVIOR,
        std::string(DM_SEND_REQUEST_SUCCESS_MSG));

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("AuthenticateDevice error: Failed with ret %{public}d", ret);
        return ret;
    }
    DmTraceEnd();
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnAuthenticateDevice(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty() || (deviceInfo.networkId[0] == '\0')) {
        LOGE("UnAuthenticateDevice error: Invalid para. pkgName %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(),
        GetAnonyString(std::string(deviceInfo.networkId)).c_str());
    std::shared_ptr<IpcUnAuthenticateDeviceReq> req = std::make_shared<IpcUnAuthenticateDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceInfo(deviceInfo);
    int32_t ret = ipcClientProxy_->SendRequest(UNAUTHENTICATE_DEVICE, req, rsp);
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice error: Failed with ret %{public}d", ret);
        SysEventWrite(std::string(UNAUTHENTICATE_DEVICE_FAILED), DM_HISYEVENT_BEHAVIOR,
            std::string(UNAUTHENTICATE_DEVICE_FAILED_MSG));
        return ret;
    }
    SysEventWrite(std::string(UNAUTHENTICATE_DEVICE_SUCCESS), DM_HISYEVENT_BEHAVIOR,
        std::string(UNAUTHENTICATE_DEVICE_SUCCESS_MSG));

    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::StopAuthenticateDevice(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "StopAuthenticateDevice",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("StopAuthenticateDevice error: Invalid para. pkgName %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = ipcClientProxy_->SendRequest(STOP_AUTHENTICATE_DEVICE, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "StopAuthenticateDevice", ret, anonyLocalUdid_);
        LOGE("StopAuthenticateDevice error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "StopAuthenticateDevice", ret, anonyLocalUdid_);
        LOGE("StopAuthenticateDevice error: Failed with ret %{public}d", ret);
        return ret;
    }

    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDeviceManagerFaCallback(const std::string &pkgName,
                                                           std::shared_ptr<DeviceManagerUiCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "RegisterDeviceManagerFaCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("RegisterDeviceManagerFaCallback error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, callback);
    RegisterUiStateCallback(pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterDeviceManagerFaCallback", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "UnRegisterDeviceManagerFaCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    UnRegisterUiStateCallback(pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnRegisterDeviceManagerFaCallback",
        DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
                                                std::shared_ptr<VerifyAuthCallback> callback)
{
    (void)pkgName;
    (void)authPara;
    (void)callback;
    LOGI("VerifyAuthentication not support method.");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetFaParam(const std::string &pkgName, DmAuthParam &dmFaParam)
{
    (void)pkgName;
    (void)dmFaParam;
    LOGI("GetFaParam not support method.");
    return DM_OK;
}

int32_t DeviceManagerImpl::SetUserOperation(const std::string &pkgName, int32_t action, const std::string &params)
{
    if (pkgName.empty() || params.empty()) {
        LOGE("DeviceManager::SetUserOperation start, pkgName: %{public}s, params: %{public}s", pkgName.c_str(),
            GetAnonyString(params).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());

    std::shared_ptr<IpcGetOperationReq> req = std::make_shared<IpcGetOperationReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetOperation(action);
    req->SetParams(params);

    int32_t ret = ipcClientProxy_->SendRequest(SERVER_USER_AUTH_OPERATION, req, rsp);
    if (ret != DM_OK) {
        LOGI("SetUserOperation Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                              std::string &udid)
{
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("DeviceManagerImpl::GetUdidByNetworkId error: Invalid para, pkgName: %{public}s, netWorkId: %{public}s",
            pkgName.c_str(), GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGD("Start, pkgName: %{public}s", GetAnonyString(pkgName).c_str());

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);

    int32_t ret = ipcClientProxy_->SendRequest(GET_UDID_BY_NETWORK, req, rsp);
    if (ret != DM_OK) {
        LOGI("GetUdidByNetworkId Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %{public}d", ret);
        return ret;
    }
    udid = rsp->GetUdid();
    return DM_OK;
}

int32_t DeviceManagerImpl::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                              std::string &uuid)
{
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("DeviceManagerImpl::GetUuidByNetworkId error: Invalid para, pkgName: %{public}s, netWorkId: %{public}s",
            pkgName.c_str(), GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", GetAnonyString(pkgName).c_str());

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);

    int32_t ret = ipcClientProxy_->SendRequest(GET_UUID_BY_NETWORK, req, rsp);
    if (ret != DM_OK) {
        LOGI("GetUuidByNetworkId Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %{public}d", ret);
        return ret;
    }
    uuid = rsp->GetUuid();
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    (void)pkgName;
    (void)extra;
    LOGI("RegisterDevStateCallback not support method.");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    (void)pkgName;
    (void)extra;
    LOGI("UnRegisterDevStateCallback not support method.");
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterUiStateCallback",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());

    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);

    int32_t ret = ipcClientProxy_->SendRequest(REGISTER_UI_STATE_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterUiStateCallback", ret, anonyLocalUdid_);
        LOGI("RegisterUiStateCallback Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterUiStateCallback", ret, anonyLocalUdid_);
        LOGE("RegisterUiStateCallback Failed with ret %{public}d", ret);
        return ret;
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterUiStateCallback", DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "UnRegisterUiStateCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);

    int32_t ret = ipcClientProxy_->SendRequest(UNREGISTER_UI_STATE_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnRegisterUiStateCallback", ret, anonyLocalUdid_);
        LOGI("UnRegisterUiStateCallback Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnRegisterUiStateCallback", ret, anonyLocalUdid_);
        LOGE("UnRegisterUiStateCallback Failed with ret %{public}d", ret);
        return ret;
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "UnRegisterUiStateCallback", DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::RequestCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (pkgName.empty() || reqJsonStr.empty()) {
        LOGE("DeviceManagerImpl::RequestCredential error: Invalid para, pkgName is %{public}s, reqJsonStr is"
            "%{public}s", pkgName.c_str(), GetAnonyString(reqJsonStr).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("start.");
    std::map<std::string, std::string> requestParam;
    requestParam.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_OH);
    requestParam.emplace(DM_CREDENTIAL_REQJSONSTR, reqJsonStr);
    std::string reqParaStr = ConvertMapToJsonString(requestParam);
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcSetCredentialRsp> rsp = std::make_shared<IpcSetCredentialRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqParaStr);

    int32_t ret = ipcClientProxy_->SendRequest(REQUEST_CREDENTIAL, req, rsp);
    if (ret != DM_OK) {
        LOGI("RequestCredential Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while request credential.");
        return ret;
    }
    returnJsonStr = rsp->GetCredentialResult();
    LOGI("completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    if (pkgName.empty() || credentialInfo.empty()) {
        LOGE("DeviceManagerImpl::ImportCredential failed, pkgName is %{public}s, credentialInfo is %{public}s",
            pkgName.c_str(), GetAnonyString(credentialInfo).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("start.");
    std::map<std::string, std::string> requestParam;
    requestParam.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_OH);
    requestParam.emplace(DM_CREDENTIAL_REQJSONSTR, credentialInfo);
    std::string reqParaStr = ConvertMapToJsonString(requestParam);
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqParaStr);

    int32_t ret = ipcClientProxy_->SendRequest(IMPORT_CREDENTIAL, req, rsp);
    if (ret != DM_OK) {
        LOGI("ImportCredential Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while import credential.");
        return ret;
    }
    LOGI("completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    if (pkgName.empty() || deleteInfo.empty()) {
        LOGE("DeviceManagerImpl::DeleteCredential failed, pkgName is %{public}s, deleteInfo is %{public}s",
            pkgName.c_str(), GetAnonyString(deleteInfo).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("start.");
    std::map<std::string, std::string> requestParam;
    requestParam.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_OH);
    requestParam.emplace(DM_CREDENTIAL_REQJSONSTR, deleteInfo);
    std::string reqParaStr = ConvertMapToJsonString(requestParam);
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqParaStr);

    int32_t ret = ipcClientProxy_->SendRequest(DELETE_CREDENTIAL, req, rsp);
    if (ret != DM_OK) {
        LOGI("DeleteCredential Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while import credential.");
        return ret;
    }
    LOGI("completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterCredentialCallback(const std::string &pkgName,
    std::shared_ptr<CredentialCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("RegisterCredentialCallback error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterCredentialCallback(pkgName, callback);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);

    int32_t ret = ipcClientProxy_->SendRequest(REGISTER_CREDENTIAL_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        LOGI("RegisterCredentialCallback Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("RegisterCredentialCallback error: Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());

    DeviceManagerNotify::GetInstance().UnRegisterCredentialCallback(pkgName);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);

    int32_t ret = ipcClientProxy_->SendRequest(UNREGISTER_CREDENTIAL_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        LOGI("UnRegisterCredentialCallback Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnRegisterCredentialCallback Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "NotifyEvent",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("NotifyEvent error: pkgName empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if ((eventId <= DM_NOTIFY_EVENT_START) || (eventId >= DM_NOTIFY_EVENT_BUTT)) {
        LOGE("NotifyEvent eventId invalid");
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "NotifyEvent",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::shared_ptr<IpcNotifyEventReq> req = std::make_shared<IpcNotifyEventReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetEventId(eventId);
    req->SetEvent(event);

    int32_t ret = ipcClientProxy_->SendRequest(NOTIFY_EVENT, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "NotifyEvent", ret, anonyLocalUdid_);
        LOGI("NotifyEvent Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "NotifyEvent", ret, anonyLocalUdid_);
        LOGE("NotifyEvent failed with ret %{public}d", ret);
        return ret;
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "NotifyEvent", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::RequestCredential(const std::string &pkgName, std::string &returnJsonStr)
{
    if (pkgName.empty()) {
        LOGE("RequestCredential failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::map<std::string, std::string> requestParam;
    requestParam.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_MINE);
    std::string reqParaStr = ConvertMapToJsonString(requestParam);
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcSetCredentialRsp> rsp = std::make_shared<IpcSetCredentialRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqParaStr);
    int32_t ret = ipcClientProxy_->SendRequest(REQUEST_CREDENTIAL, req, rsp);
    if (ret != DM_OK) {
        LOGI("RequestCredential Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while request credential.");
        return ret;
    }
    returnJsonStr = rsp->GetCredentialResult();
    LOGI("Completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                           std::string &returnJsonStr)
{
    if (pkgName.empty() || reqJsonStr.empty()) {
        LOGE("Check the credential is invalid para.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcSetCredentialRsp> rsp = std::make_shared<IpcSetCredentialRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqJsonStr);

    int32_t ret = ipcClientProxy_->SendRequest(SERVER_GET_DMFA_INFO, req, rsp);
    if (ret != DM_OK) {
        LOGI("CheckCredential Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while Check credential.");
        return ret;
    }
    returnJsonStr = rsp->GetCredentialResult();
    LOGI("Completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                            std::string &returnJsonStr)
{
    if (pkgName.empty() || reqJsonStr.empty()) {
        LOGE("import the credential is invalid para.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::map<std::string, std::string> requestParam;
    requestParam.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_MINE);
    requestParam.emplace(DM_CREDENTIAL_REQJSONSTR, reqJsonStr);
    std::string reqParaStr = ConvertMapToJsonString(requestParam);
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcSetCredentialRsp> rsp = std::make_shared<IpcSetCredentialRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqParaStr);

    int32_t ret = ipcClientProxy_->SendRequest(IMPORT_CREDENTIAL, req, rsp);
    if (ret != DM_OK) {
        LOGI("ImportCredential Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while import credential.");
        return ret;
    }
    returnJsonStr = rsp->GetCredentialResult();
    LOGI("Completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                            std::string &returnJsonStr)
{
    if (pkgName.empty() || reqJsonStr.empty()) {
        LOGE("Delete the credential is invalid para.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::map<std::string, std::string> requestParam;
    requestParam.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_MINE);
    requestParam.emplace(DM_CREDENTIAL_REQJSONSTR, reqJsonStr);
    std::string reqParaStr = ConvertMapToJsonString(requestParam);
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::shared_ptr<IpcSetCredentialRsp> rsp = std::make_shared<IpcSetCredentialRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialParam(reqParaStr);

    int32_t ret = ipcClientProxy_->SendRequest(DELETE_CREDENTIAL, req, rsp);
    if (ret != DM_OK) {
        LOGI("DeleteCredential Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("failed to get return errcode while import credential.");
        return ret;
    }
    returnJsonStr = rsp->GetCredentialResult();
    LOGI("Completed.");
    return DM_OK;
}

int32_t DeviceManagerImpl::OnDmServiceDied()
{
    LOGI("Start");
    int32_t ret = ipcClientProxy_->OnDmServiceDied();
    if (ret != DM_OK) {
        LOGE("OnDmServiceDied failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerImpl::GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
    std::string &uuid)
{
    if (pkgName.empty() || networkId.empty()) {
        LOGE("DeviceManagerImpl::GetEncryptedUuidByNetworkId error: Invalid para, pkgName: %{public}s, netWorkId:"
            "%{public}s", pkgName.c_str(), GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(networkId);

    int32_t ret = ipcClientProxy_->SendRequest(GET_ENCRYPTED_UUID_BY_NETWOEKID, req, rsp);
    if (ret != DM_OK) {
        LOGI("GetEncryptedUuidByNetworkId Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %{public}d", ret);
        return ret;
    }
    uuid = rsp->GetUuid();
    LOGI("Complete, uuid: %{public}s", GetAnonyString(uuid).c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
    const std::string &appId, std::string &encryptedUuid)
{
    if (pkgName.empty() || uuid.empty()) {
        LOGE("DeviceManagerImpl::GenerateEncryptedUuid error: Invalid para, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());

    std::shared_ptr<IpcGenerateEncryptedUuidReq> req = std::make_shared<IpcGenerateEncryptedUuidReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetUuid(uuid);
    req->SetAppId(appId);

    int32_t ret = ipcClientProxy_->SendRequest(GENERATE_ENCRYPTED_UUID, req, rsp);
    if (ret != DM_OK) {
        LOGI("GenerateEncryptedUuid Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %{public}d", ret);
        return ret;
    }
    encryptedUuid = rsp->GetUuid();
    LOGI("Complete, encryptedUuid: %{public}s", GetAnonyString(encryptedUuid).c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::CheckAPIAccessPermission()
{
    LOGI("Start");
    return CheckApiPermission(SYSTEM_BASIC);
}

int32_t DeviceManagerImpl::CheckNewAPIAccessPermission()
{
    LOGI("Start");
    return CheckApiPermission(NORMAL);
}

int32_t DeviceManagerImpl::GetLocalDeviceNetWorkId(const std::string &pkgName, std::string &networkId)
{
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DmDeviceInfo info;
    int32_t ret = GetLocalDeviceInfo(pkgName, info);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceNetWorkId",
            info, ret, anonyLocalUdid_);
        LOGI("DeviceManagerImpl::GetLocalDeviceNetWorkId failed.");
        return ret;
    }
    networkId = std::string(info.networkId);
    LOGI("End, networkId : %{public}s", GetAnonyString(networkId).c_str());
    DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceNetWorkId",
        info, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDeviceId(const std::string &pkgName, std::string &deviceId)
{
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    DmDeviceInfo info;
    int32_t ret = GetLocalDeviceInfo(pkgName, info);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceId", info, ret, anonyLocalUdid_);
        LOGI("DeviceManagerImpl::GetLocalDeviceNetWorkId failed.");
        return ret;
    }
    deviceId = std::string(info.deviceId);
    LOGI("End, deviceId : %{public}s", GetAnonyString(deviceId).c_str());
    DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceId", info, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDeviceName(const std::string &pkgName, std::string &deviceName)
{
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcGetLocalDeviceNameRsp> rsp = std::make_shared<IpcGetLocalDeviceNameRsp>();
    int32_t ret = ipcClientProxy_->SendRequest(GET_LOCAL_DEVICE_NAME_OLD, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Get local device name failed ret: %{public}d", ret);
        return ret;
    }
    deviceName = rsp->GetLocalDeviceName();
    LOGI("End, deviceName : %{public}s", GetAnonyString(deviceName).c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDeviceName(std::string &deviceName)
{
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcGetLocalDeviceNameRsp> rsp = std::make_shared<IpcGetLocalDeviceNameRsp>();
    int32_t ret = ipcClientProxy_->SendRequest(GET_LOCAL_DEVICE_NAME, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Get local device name failed ret: %{public}d", ret);
        return ret;
    }
    deviceName = rsp->GetLocalDeviceName();
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDeviceType(const std::string &pkgName,  int32_t &deviceType)
{
    LOGI("Start, pkgName : %{public}s", pkgName.c_str());
    DmDeviceInfo info;
    int32_t ret = GetLocalDeviceInfo(pkgName, info);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceType", info, ret, anonyLocalUdid_);
        LOGI("DeviceManagerImpl::GetLocalDeviceNetWorkId failed.");
        return ret;
    }
    deviceType = info.deviceTypeId;
    LOGI("End, deviceType : %{public}d", deviceType);
    DmRadarHelper::GetInstance().ReportGetLocalDevInfo(pkgName, "GetLocalDeviceType", info, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceName(const std::string &pkgName, const std::string &networkId,
    std::string &deviceName)
{
    DmDeviceInfo deviceInfo;
    int32_t ret = GetDeviceInfo(pkgName, networkId, deviceInfo);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetDeviceInfo(pkgName, "GetDeviceName", deviceInfo, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetDeviceName error, failed ret: %{public}d", ret);
        return ret;
    }
    deviceName = std::string(deviceInfo.deviceName);
    LOGI("End, pkgName : %{public}s, networkId : %{public}s, deviceName = %{public}s",
        pkgName.c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(deviceName).c_str());
    DmRadarHelper::GetInstance().ReportGetDeviceInfo(pkgName, "GetDeviceName", deviceInfo, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceType(const std::string &pkgName, const std::string &networkId, int32_t &deviceType)
{
    DmDeviceInfo deviceInfo;
    int32_t ret = GetDeviceInfo(pkgName, networkId, deviceInfo);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetDeviceInfo(pkgName, "GetDeviceType", deviceInfo, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetDeviceType error, failed ret: %{public}d", ret);
        return ret;
    }
    deviceType = deviceInfo.deviceTypeId;
    LOGI("End, pkgName : %{public}s, networkId : %{public}s, deviceType = %{public}d",
        pkgName.c_str(), GetAnonyString(networkId).c_str(), deviceType);
    DmRadarHelper::GetInstance().ReportGetDeviceInfo(pkgName, "GetDeviceType", deviceInfo, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::BindDevice(const std::string &pkgName, int32_t bindType, const std::string &deviceId,
    const std::string &bindParam, std::shared_ptr<AuthenticateCallback> callback)
{
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("BindDevice error: Invalid para. pkgName : %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("start, pkgName: %{public}s", pkgName.c_str());
    JsonObject paramJson(bindParam);
    if (paramJson.IsDiscarded()) {
        LOGE("BindDevice bindParam %{public}s.", bindParam.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    paramJson[TOKENID] = std::to_string(OHOS::IPCSkeleton::GetSelfTokenID());
#endif
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    std::shared_ptr<IpcBindDeviceReq> req = std::make_shared<IpcBindDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetBindParam(SafetyDump(paramJson));
    req->SetBindType(bindType);
    req->SetDeviceId(deviceId);
    int32_t ret = ipcClientProxy_->SendRequest(BIND_DEVICE, req, rsp);
    if (ret != DM_OK) {
        LOGE("BindDevice error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("BindDevice error: Failed with ret %{public}d", ret);
        return ret;
    }
    DmTraceEnd();
    LOGI("End");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnBindDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("UnBindDevice error: Invalid para. pkgName %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s, deviceId: %{public}s", pkgName.c_str(),
        GetAnonyString(std::string(deviceId)).c_str());
    std::shared_ptr<IpcUnBindDeviceReq> req = std::make_shared<IpcUnBindDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceId(deviceId);
    int32_t ret = ipcClientProxy_->SendRequest(UNBIND_DEVICE, req, rsp);
    if (ret != DM_OK) {
        LOGE("UnBindDevice error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnBindDevice error: Failed with ret %{public}d", ret);
        return ret;
    }

    LOGI("End");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnBindDevice(const std::string &pkgName, const std::string &deviceId,
    const std::string &extra)
{
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("UnBindDevice error: Invalid para. pkgName %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s, deviceId: %{public}s", pkgName.c_str(),
        GetAnonyString(std::string(deviceId)).c_str());
    std::shared_ptr<IpcUnBindDeviceReq> req = std::make_shared<IpcUnBindDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceId(deviceId);
    req->SetExtraInfo(extra);
    int32_t ret = ipcClientProxy_->SendRequest(UNBIND_DEVICE, req, rsp);
    if (ret != DM_OK) {
        LOGE("UnBindDevice error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnBindDevice error: Failed with ret %{public}d", ret);
        return ret;
    }

    LOGI("End");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetNetworkTypeByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                     int32_t &netWorkType)
{
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("GetNetworkTypeByNetworkId error: Invalid para, pkgName: %{public}s, netWorkId: %{public}s, netWorkType:"
            "%{public}d", pkgName.c_str(), GetAnonyString(netWorkId).c_str(), netWorkType);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);

    int32_t ret = ipcClientProxy_->SendRequest(GET_NETWORKTYPE_BY_NETWORK, req, rsp);
    if (ret != DM_OK) {
        LOGI("GetNetworkTypeByNetworkId Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CheckAuthentication Failed with ret %{public}d", ret);
        return ret;
    }
    netWorkType = rsp->GetNetworkType();
    return DM_OK;
}

int32_t DeviceManagerImpl::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (authCode.empty() || pkgName.empty()) {
        LOGE("ImportAuthCode error: Invalid para, authCode: %{public}s, pkgName: %{public}s",
            GetAnonyString(authCode).c_str(), pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, authCode: %{public}s", GetAnonyString(authCode).c_str());
    int32_t length = static_cast<int32_t>(authCode.length());
    if (length < DM_IMPORT_AUTH_CODE_MIN_LENGTH || length > DM_IMPORT_AUTH_CODE_MAX_LENGTH) {
        LOGE("ImportAuthCode error: Invalid para, authCode size error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    for (int32_t i = 0; i < length; i++) {
        if (!isdigit(authCode[i])) {
            LOGE("ImportAuthCode error: Invalid para, authCode format error.");
            return ERR_DM_INPUT_PARA_INVALID;
        }
    }

    std::shared_ptr<IpcImportAuthCodeReq> req = std::make_shared<IpcImportAuthCodeReq>();
    req->SetAuthCode(authCode);
    req->SetPkgName(pkgName);
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    int32_t ret = ipcClientProxy_->SendRequest(IMPORT_AUTH_CODE, req, rsp);
    if (ret != DM_OK) {
        LOGI("ImportAuthCode Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("ImportAuthCode Failed with ret %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerImpl::ExportAuthCode(std::string &authCode)
{
    LOGI("Start");

    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcExportAuthCodeRsp> rsp = std::make_shared<IpcExportAuthCodeRsp>();

    int32_t ret = ipcClientProxy_->SendRequest(EXPORT_AUTH_CODE, req, rsp);
    if (ret != DM_OK) {
        LOGI("ExportAuthCode Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("ExportAuthCode Failed with ret %{public}d", ret);
        return ret;
    }

    authCode = rsp->GetAuthCode();
    LOGI("Success, authCode: %{public}s.", GetAnonyString(authCode).c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDiscoveryCallback(const std::string &pkgName,
    std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions,
    std::shared_ptr<DiscoveryCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("DeviceManagerImpl::RegisterDiscoveryCallback failed: input callback is null or pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    uint16_t subscribeId = AddDiscoveryCallback(pkgName, discoverParam, callback);
    discoverParam.emplace(PARAM_KEY_SUBSCRIBE_ID, std::to_string(subscribeId));
    std::string discParaStr = ConvertMapToJsonString(discoverParam);
    std::string filterOpStr = ConvertMapToJsonString(filterOptions);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(ComposeStr(pkgName, subscribeId));
    req->SetFirstParam(discParaStr);
    req->SetSecondParam(filterOpStr);
    int32_t ret = ipcClientProxy_->SendRequest(REGISTER_DISCOVERY_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        LOGE("RegisterDiscoveryCallback error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("RegisterDiscoveryCallback error: Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDiscoveryCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "UnRegisterDiscoveryCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::UnRegisterDiscoveryCallback failed: input pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::string pkgNameTemp = ComposeStr(pkgName, DM_INVALID_FLAG_ID);
    uint16_t subscribeId = GetSubscribeIdFromMap(pkgNameTemp);
    if (subscribeId == DM_INVALID_FLAG_ID) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "UnRegisterDiscoveryCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::UnRegisterDiscoveryCallback failed: cannot find pkgName in cache map.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> extraParam;
    extraParam.emplace(PARAM_KEY_SUBSCRIBE_ID, std::to_string(subscribeId));
    std::string extraParaStr = ConvertMapToJsonString(extraParam);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(ComposeStr(pkgName, subscribeId));
    req->SetFirstParam(extraParaStr);
    int32_t ret = ipcClientProxy_->SendRequest(UNREGISTER_DISCOVERY_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgNameTemp, "UnRegisterDiscoveryCallback",
            ret, anonyLocalUdid_);
        LOGE("UnRegisterDiscoveryCallback error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgNameTemp, "UnRegisterDiscoveryCallback",
            ret, anonyLocalUdid_);
        LOGE("UnRegisterDiscoveryCallback error: Failed with ret %{public}d", ret);
        return ret;
    }
    RemoveDiscoveryCallback(pkgNameTemp);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgNameTemp, "UnRegisterDiscoveryCallback", DM_OK, anonyLocalUdid_);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::StartAdvertising(const std::string &pkgName,
    std::map<std::string, std::string> &advertiseParam, std::shared_ptr<PublishCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("DeviceManagerImpl::StartAdvertising error: pkgName %{public}s invalid para", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName %{public}s", pkgName.c_str());

    int32_t publishId;
    if (advertiseParam.find(PARAM_KEY_PUBLISH_ID) == advertiseParam.end()) {
        publishId = AddPublishCallback(pkgName);
        advertiseParam[PARAM_KEY_PUBLISH_ID] = std::to_string(publishId);
    } else {
        publishId = std::atoi((advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second).c_str());
    }
    std::string adverParaStr = ConvertMapToJsonString(advertiseParam);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetFirstParam(adverParaStr);
    int32_t ret = ipcClientProxy_->SendRequest(START_ADVERTISING, req, rsp);
    if (ret != DM_OK) {
        LOGE("StartAdvertising error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StartAdvertising error: Failed with ret %{public}d", ret);
        return ret;
    }
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, callback);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::StopAdvertising(const std::string &pkgName,
    std::map<std::string, std::string> &advertiseParam)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerImpl::StopAdvertising failed: input pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());

    int32_t publishId;
    if (advertiseParam.find(PARAM_KEY_PUBLISH_ID) == advertiseParam.end()) {
        publishId = RemovePublishCallback(pkgName);
        advertiseParam[PARAM_KEY_PUBLISH_ID] = std::to_string(publishId);
    } else {
        publishId = std::atoi((advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second).c_str());
    }
    if (publishId == DM_INVALID_FLAG_ID) {
        LOGE("DeviceManagerImpl::StopAdvertising failed: cannot find pkgName in cache map.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string adverParaStr = ConvertMapToJsonString(advertiseParam);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetFirstParam(adverParaStr);
    int32_t ret = ipcClientProxy_->SendRequest(STOP_ADVERTISING, req, rsp);
    if (ret != DM_OK) {
        LOGE("StopAdvertising error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StopAdvertising error: Failed with ret %{public}d", ret);
        return ret;
    }
    DeviceManagerNotify::GetInstance().UnRegisterPublishCallback(pkgName, publishId);
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    std::map<std::string, std::string> &bindParam, std::shared_ptr<BindTargetCallback> callback)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId)) {
        LOGE("DeviceManagerImpl::BindTarget failed: input pkgName or targetId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("start, pkgName: %{public}s", pkgName.c_str());
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    bindParam[TOKENID] = std::to_string(OHOS::IPCSkeleton::GetSelfTokenID());
#endif
    std::string bindParamStr = ConvertMapToJsonString(bindParam);
    DeviceManagerNotify::GetInstance().RegisterBindCallback(pkgName, targetId, callback);
    std::shared_ptr<IpcBindTargetReq> req = std::make_shared<IpcBindTargetReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetPeerTargetId(targetId);
    req->SetBindParam(bindParamStr);
    int32_t ret = ipcClientProxy_->SendRequest(BIND_TARGET, req, rsp);
    if (ret != DM_OK) {
        LOGE("BindTarget error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("BindTarget error: Failed with ret %{public}d", ret);
        return ret;
    }

    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnbindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    std::map<std::string, std::string> &unbindParam, std::shared_ptr<UnbindTargetCallback> callback)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId)) {
        LOGE("DeviceManagerImpl::UnbindTarget failed: input pkgName or targetId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    std::string unbindParamStr = ConvertMapToJsonString(unbindParam);

    std::shared_ptr<IpcBindTargetReq> req = std::make_shared<IpcBindTargetReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetPeerTargetId(targetId);
    req->SetBindParam(unbindParamStr);
    int32_t ret = ipcClientProxy_->SendRequest(UNBIND_TARGET, req, rsp);
    if (ret != DM_OK) {
        LOGE("UnbindTarget error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnbindTarget error: Failed with ret %{public}d", ret);
        return ret;
    }
    DeviceManagerNotify::GetInstance().RegisterUnbindCallback(pkgName, targetId, callback);

    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName,
    const std::map<std::string, std::string> &filterOptions, bool isRefresh,
    std::vector<DmDeviceInfo> &deviceList)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerImpl::GetTrustedDeviceList failed: input pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string filterOpStr = ConvertMapToJsonString(filterOptions);
    return GetTrustedDeviceList(pkgName, filterOpStr, isRefresh, deviceList);
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName,
    const std::map<std::string, std::string> &extraParam, std::shared_ptr<DeviceStateCallback> callback)
{
    (void)extraParam;
    if (pkgName.empty() || callback == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterDevStateCallback",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::RegisterDeviceStateCallback failed: input pkgName or callback is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SyncCallbackToService(DmCommonNotifyEvent::REG_DEVICE_STATE, pkgName);
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterDevStateCallback", DM_OK, anonyLocalUdid_);
    LOGI("Completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::CheckAccessToTarget(uint64_t tokenId, const std::string &targetId)
{
    (void)tokenId;
    (void)targetId;
    LOGI("unsupport");
    return ERR_DM_UNSUPPORTED_METHOD;
}

uint16_t DeviceManagerImpl::AddDiscoveryCallback(const std::string &pkgName,
    std::map<std::string, std::string> &discoverParam, std::shared_ptr<DiscoveryCallback> callback)
{
    if (discoverParam.empty() || callback == nullptr) {
        LOGE("input param invalid.");
        return DM_INVALID_FLAG_ID;
    }
    uint16_t subscribeId = DM_INVALID_FLAG_ID;
    if (discoverParam.find(PARAM_KEY_SUBSCRIBE_ID) != discoverParam.end()) {
        subscribeId = std::atoi((discoverParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
    }
    std::string pkgNameTemp = ComposeStr(pkgName, subscribeId);
    {
        std::lock_guard<std::mutex> autoLock(subMapLock);
        auto item = pkgName2SubIdMap_.find(pkgNameTemp);
        if (item == pkgName2SubIdMap_.end() && subscribeId == DM_INVALID_FLAG_ID) {
            subscribeId = GenUniqueRandUint(randSubIdSet_);
            pkgName2SubIdMap_[pkgNameTemp] = subscribeId;
        } else if (item == pkgName2SubIdMap_.end() && subscribeId != DM_INVALID_FLAG_ID) {
            pkgName2SubIdMap_[pkgNameTemp] = subscribeId;
            randSubIdSet_.emplace(subscribeId);
        } else if (item != pkgName2SubIdMap_.end()) {
            subscribeId = pkgName2SubIdMap_[pkgNameTemp];
        } else {
            LOGE("subscribeId is unreasonable");
        }
    }
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgNameTemp, subscribeId, callback);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgNameTemp, "AddDiscoveryCallback", DM_OK, anonyLocalUdid_);
    return subscribeId;
}

uint16_t DeviceManagerImpl::RemoveDiscoveryCallback(const std::string &pkgName)
{
    uint16_t subscribeId = DM_INVALID_FLAG_ID;
    {
        std::lock_guard<std::mutex> autoLock(subMapLock);
        if (pkgName2SubIdMap_.find(pkgName) != pkgName2SubIdMap_.end()) {
            subscribeId = pkgName2SubIdMap_[pkgName];
            randSubIdSet_.erase(subscribeId);
            pkgName2SubIdMap_.erase(pkgName);
        }
    }
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(pkgName, subscribeId);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RemoveDiscoveryCallback", DM_OK, anonyLocalUdid_);
    return subscribeId;
}

int32_t DeviceManagerImpl::AddPublishCallback(const std::string &pkgName)
{
    int32_t publishId = DM_INVALID_FLAG_ID;
    {
        std::lock_guard<std::mutex> autoLock(pubMapLock);
        if (pkgName2PubIdMap_.find(pkgName) != pkgName2PubIdMap_.end()) {
            publishId = pkgName2PubIdMap_[pkgName];
        } else {
            publishId = GenUniqueRandUint(randPubIdSet_);
            pkgName2PubIdMap_[pkgName] = publishId;
        }
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "AddPublishCallback", DM_OK, anonyLocalUdid_);
    return publishId;
}

int32_t DeviceManagerImpl::RemovePublishCallback(const std::string &pkgName)
{
    uint16_t publishId = DM_INVALID_FLAG_ID;
    {
        std::lock_guard<std::mutex> autoLock(pubMapLock);
        if (pkgName2PubIdMap_.find(pkgName) != pkgName2PubIdMap_.end()) {
            publishId = pkgName2PubIdMap_[pkgName];
            randPubIdSet_.erase(publishId);
            pkgName2PubIdMap_.erase(pkgName);
        }
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RemovePublishCallback", DM_OK, anonyLocalUdid_);
    return publishId;
}

int32_t DeviceManagerImpl::RegisterPinHolderCallback(const std::string &pkgName,
    std::shared_ptr<PinHolderCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "RegisterPinHolderCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("RegisterPinHolderCallback error: Invalid para, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify::GetInstance().RegisterPinHolderCallback(pkgName, callback);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    req->SetPkgName(pkgName);
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    int32_t ret = ipcClientProxy_->SendRequest(REGISTER_PIN_HOLDER_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterPinHolderCallback", ret, anonyLocalUdid_);
        LOGI("RegisterPinHolderCallback Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterPinHolderCallback", ret, anonyLocalUdid_);
        LOGE("RegisterPinHolderCallback Failed with ret %{public}d", ret);
        return ret;
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegisterPinHolderCallback", DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId) ||
        pinType > DmPinType::SUPER_SONIC || pinType < DmPinType::NUMBER_PIN_CODE ||
        payload.length() > DM_STRING_LENGTH_MAX) {
        LOGE("CreatePinHolder error: Invalid para, pkgName: %{public}s, pinType: %{public}d, payload.length:"
            "%{public}zu", pkgName.c_str(), pinType, payload.length());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::shared_ptr<IpcCreatePinHolderReq> req = std::make_shared<IpcCreatePinHolderReq>();
    req->SetPkgName(pkgName);
    req->SetPeerTargetId(targetId);
    req->SetPinType(pinType);
    req->SetPayload(payload);
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    int32_t ret = ipcClientProxy_->SendRequest(CREATE_PIN_HOLDER, req, rsp);
    if (ret != DM_OK) {
        LOGI("CreatePinHolder Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("CreatePinHolder Failed with ret %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerImpl::DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId) ||
        pinType > DmPinType::SUPER_SONIC || pinType < DmPinType::NUMBER_PIN_CODE ||
        payload.length() > DM_STRING_LENGTH_MAX) {
        LOGE("DestroyPinHolder error: Invalid para, pkgName: %{public}s, pinType: %{public}d, payload.length"
            "%{public}zu", pkgName.c_str(), pinType, payload.length());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::shared_ptr<IpcDestroyPinHolderReq> req = std::make_shared<IpcDestroyPinHolderReq>();
    req->SetPkgName(pkgName);
    req->SetPeerTargetId(targetId);
    req->SetPinType(pinType);
    req->SetPayload(payload);
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    int32_t ret = ipcClientProxy_->SendRequest(DESTROY_PIN_HOLDER, req, rsp);
    if (ret != DM_OK) {
        LOGI("DestroyPinHolder Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("DestroyPinHolder Failed with ret %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerImpl::DpAclAdd(const int64_t accessControlId, const std::string &udid, const int32_t bindType)
{
    if (bindType != IDENTICAL_ACCOUNT) {
        LOGI("not identical account");
        return DM_OK;
    }
    LOGI("Start.");
    std::shared_ptr<IpcAclProfileReq> req = std::make_shared<IpcAclProfileReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetStr(udid);
    int32_t ret = ipcClientProxy_->SendRequest(DP_ACL_ADD, req, rsp);
    if (ret != DM_OK) {
        LOGE("DpAclAdd error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("DpAclAdd error: Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId,
                                                  int32_t &securityLevel)
{
    if (pkgName.empty() || networkId.empty()) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "GetDeviceSecurityLevel",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetDeviceSecurityLevel error: pkgName: %{public}s, networkId: %{public}s",
            pkgName.c_str(), GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start: pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(),
         GetAnonyString(networkId).c_str());

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(networkId);

    int32_t ret = ipcClientProxy_->SendRequest(GET_SECURITY_LEVEL, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "GetDeviceSecurityLevel", ret, anonyLocalUdid_);
        LOGE("GetDeviceSecurityLevel Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "GetDeviceSecurityLevel", ret, anonyLocalUdid_);
        LOGE("GetDeviceSecurityLevel Failed with ret %{public}d", ret);
        return ret;
    }
    securityLevel = rsp->GetSecurityLevel();
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "GetDeviceSecurityLevel", DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::CheckApiPermission(int32_t permissionLevel)
{
    LOGI("PermissionLevel: %{public}d", permissionLevel);
    std::shared_ptr<IpcPermissionReq> req = std::make_shared<IpcPermissionReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPermissionLevel(permissionLevel);
    int32_t ret = ipcClientProxy_->SendRequest(CHECK_API_PERMISSION, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Check permission failed with ret: %{public}d", ret);
        return ret;
    }
    LOGD("The caller declare the DM permission!");
    return DM_OK;
}

bool DeviceManagerImpl::IsSameAccount(const std::string &netWorkId)
{
    if (netWorkId.empty()) {
        LOGE("DeviceManagerImpl::IsSameAccount error: netWorkId: %{public}s", GetAnonyString(netWorkId).c_str());
        return false;
    }
    std::shared_ptr<IpcAclProfileReq> req = std::make_shared<IpcAclProfileReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetStr(netWorkId);
    int32_t ret = ipcClientProxy_->SendRequest(IS_SAME_ACCOUNT, req, rsp);
    if (ret != DM_OK) {
        LOGE("IsSameAccount Send Request failed ret: %{public}d", ret);
        return false;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("IsSameAccount Failed with ret: %{public}d", ret);
        return false;
    }
    return true;
}

int32_t DeviceManagerImpl::GetErrCode(int32_t errCode)
{
    auto flag = MAP_ERROR_CODE.find(errCode);
    if (flag == MAP_ERROR_CODE.end()) {
        return errCode;
    }
    return flag->second;
}

int32_t DeviceManagerImpl::ShiftLNNGear(const std::string &pkgName)
{
    LOGI("Start. pkgName = %{public}s", pkgName.c_str());
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = ipcClientProxy_->SendRequest(SHIFT_LNN_GEAR, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "ShiftLNNGear", ret, anonyLocalUdid_);
        LOGE("ShiftLNNGear error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "ShiftLNNGear", ret, anonyLocalUdid_);
        LOGE("ShiftLNNGear error: Failed with ret %{public}d", ret);
        return ret;
    }
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "ShiftLNNGear", DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::SetDnPolicy(const std::string &pkgName, std::map<std::string, std::string> &policy)
{
    const size_t SET_DN_POLICY_PARAM_SIZE = 2;
    if (pkgName.empty() || policy.size() != SET_DN_POLICY_PARAM_SIZE) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "SetDnPolicy",
            ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Para invalid: policy is less than two or pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start");
    std::string strategy = ConvertMapToJsonString(policy);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetFirstParam(strategy);
    int32_t ret = ipcClientProxy_->SendRequest(SET_DN_POLICY, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "SetDnPolicy", ret, anonyLocalUdid_);
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "SetDnPolicy", ret, anonyLocalUdid_);
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "SetDnPolicy", DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::RegDevTrustChangeCallback(const std::string &pkgName,
    std::shared_ptr<DevTrustChangeCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Error: Invalid para");
        DmRadarHelper::GetInstance().ReportDmBehavior(
            pkgName, "RegDevTrustChangeCallback", ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("PkgName %{public}s.", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    SyncCallbackToService(DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE, pkgName);
    DmRadarHelper::GetInstance().ReportDmBehavior(pkgName, "RegDevTrustChangeCallback", DM_OK, anonyLocalUdid_);
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDeviceScreenStatusCallback(const std::string &pkgName,
    std::shared_ptr<DeviceScreenStatusCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify::GetInstance().RegisterDeviceScreenStatusCallback(pkgName, callback);
    SyncCallbackToService(DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE, pkgName);
    LOGI("Completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDeviceScreenStatusCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SyncCallbackToService(DmCommonNotifyEvent::UN_REG_DEVICE_SCREEN_STATE, pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDeviceScreenStatusCallback(pkgName);
    LOGI("Completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
    int32_t &screenStatus)
{
    if (pkgName.empty() || networkId.empty()) {
        LOGE("Error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start: pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(), GetAnonyString(networkId).c_str());

    std::shared_ptr<IpcGetDeviceScreenStatusReq> req = std::make_shared<IpcGetDeviceScreenStatusReq>();
    std::shared_ptr<IpcGetDeviceScreenStatusRsp> rsp = std::make_shared<IpcGetDeviceScreenStatusRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(networkId);

    int32_t ret = ipcClientProxy_->SendRequest(GET_DEVICE_SCREEN_STATUS, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    screenStatus = rsp->GetScreenStatus();
    return DM_OK;
}

int32_t DeviceManagerImpl::GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid,
                                              std::string &networkId)
{
    if (pkgName.empty() || udid.empty()) {
        LOGE("DeviceManagerImpl::GetNetworkIdByUdid error: Invalid para, pkgName: %{public}s, udid: %{public}s",
            pkgName.c_str(), GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGD("Start, pkgName: %{public}s", GetAnonyString(pkgName).c_str());

    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetUdid(udid);

    int32_t ret = ipcClientProxy_->SendRequest(GET_NETWORKID_BY_UDID, req, rsp);
    if (ret != DM_OK) {
        LOGI("GetNetworkIdByUdid Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("GetNetworkIdByUdid Failed with ret %{public}d", ret);
        return ret;
    }
    networkId = rsp->GetNetWorkId();
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterCredentialAuthStatusCallback(const std::string &pkgName,
    std::shared_ptr<CredentialAuthStatusCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify::GetInstance().RegisterCredentialAuthStatusCallback(pkgName, callback);
    SyncCallbackToService(DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY, pkgName);
    LOGI("Completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterCredentialAuthStatusCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SyncCallbackToService(DmCommonNotifyEvent::UN_REG_CREDENTIAL_AUTH_STATUS_NOTIFY, pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterCredentialAuthStatusCallback(pkgName);
    LOGI("Completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterSinkBindCallback(const std::string &pkgName,
    std::shared_ptr<BindTargetCallback> callback)
{
    if (pkgName.empty()) {
        LOGE("Error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify::GetInstance().RegisterSinkBindCallback(pkgName, callback);
    LOGI("Completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterSinkBindCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify::GetInstance().UnRegisterSinkBindCallback(pkgName);
    LOGI("Completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetAnonyLocalUdid(const std::string &pkgName, std::string &anonyUdid)
{
    LOGD("Start");
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcGetAnonyLocalUdidRsp> rsp = std::make_shared<IpcGetAnonyLocalUdidRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = ipcClientProxy_->SendRequest(GET_ANONY_LOCAL_UDID, req, rsp);
    if (ret != DM_OK) {
        LOGI("GetAnonyLocalUdid Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("GetAnonyLocalUdid Failed with ret %{public}d", ret);
        return ret;
    }
    anonyUdid = rsp->GetAnonyUdid();
    return DM_OK;
}

uint16_t DeviceManagerImpl::GetSubscribeIdFromMap(const std::string &pkgName)
{
    {
        std::lock_guard<std::mutex> autoLock(subMapLock);
        if (pkgName2SubIdMap_.find(pkgName) != pkgName2SubIdMap_.end()) {
            return pkgName2SubIdMap_[pkgName];
        }
    }
    return DM_INVALID_FLAG_ID;
}

void DeviceManagerImpl::SyncCallbackToService(DmCommonNotifyEvent dmCommonNotifyEvent, const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    if (!IsDmCommonNotifyEventValid(dmCommonNotifyEvent)) {
        LOGE("Invalid dmCommonNotifyEvent: %{public}d.", dmCommonNotifyEvent);
        return;
    }
    std::shared_ptr<IpcSyncCallbackReq> req = std::make_shared<IpcSyncCallbackReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDmCommonNotifyEvent(static_cast<int32_t>(dmCommonNotifyEvent));
    int32_t ret = ipcClientProxy_->SendRequest(SYNC_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        LOGI("Send Request failed ret: %{public}d", ret);
        return;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return;
    }
}

int32_t DeviceManagerImpl::GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
    if (pkgName.empty()) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(
            pkgName, "GetAllTrustedDeviceList", deviceList, ERR_DM_INPUT_PARA_INVALID, anonyLocalUdid_);
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    deviceList.clear();
    LOGI("Start, pkgName: %{public}s, extra: %{public}s", GetAnonyString(pkgName).c_str(),
        GetAnonyString(extra).c_str());

    std::shared_ptr<IpcGetTrustDeviceReq> req = std::make_shared<IpcGetTrustDeviceReq>();
    std::shared_ptr<IpcGetTrustDeviceRsp> rsp = std::make_shared<IpcGetTrustDeviceRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    int32_t ret = ipcClientProxy_->SendRequest(GET_ALL_TRUST_DEVICE_LIST, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetAllTrustedDeviceList",
            deviceList, ret, anonyLocalUdid_);
        LOGE("DeviceManagerImpl::GetAllTrustedDeviceList error, Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetAllTrustedDeviceList",
            deviceList, ret, anonyLocalUdid_);
        LOGE("GetAllTrustedDeviceList error, failed ret: %{public}d", ret);
        return ret;
    }

    deviceList = rsp->GetDeviceVec();
    LOGI("Completed, device size %{public}zu", deviceList.size());
    DmRadarHelper::GetInstance().ReportGetTrustDeviceList(pkgName, "GetAllTrustedDeviceList",
        deviceList, DM_OK, anonyLocalUdid_);
    return DM_OK;
}

void DeviceManagerImpl::SyncCallbacksToService(std::map<DmCommonNotifyEvent, std::set<std::string>> &callbackMap)
{
    if (callbackMap.size() == 0) {
        LOGI("callbackMap is empty.");
        return;
    }
    for (auto iter : callbackMap) {
        if (iter.second.size() == 0) {
            continue;
        }
        for (auto item : iter.second) {
            SyncCallbackToService(iter.first, item);
        }
    }
}

int32_t DeviceManagerImpl::RegisterAuthenticationType(const std::string &pkgName,
    const std::map<std::string, std::string> &authParam)
{
    const size_t AUTH_TYPE_PARAM_SIZE = 1;
    if (pkgName.empty() || authParam.size() != AUTH_TYPE_PARAM_SIZE) {
        LOGE("Para invalid: authParam is less than one or pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start");
    std::string authTypeStr = ConvertMapToJsonString(authParam);

    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetFirstParam(authTypeStr);
    int32_t ret = ipcClientProxy_->SendRequest(REG_AUTHENTICATION_TYPE, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceProfileInfoList(const std::string &pkgName,
    const DmDeviceProfileInfoFilterOptions &filterOptions, std::shared_ptr<GetDeviceProfileInfoListCallback> callback)
{
    LOGI("In pkgName:%{public}s, isCloud:%{public}d", pkgName.c_str(), filterOptions.isCloud);
    int32_t ret = DeviceManagerNotify::GetInstance().RegisterGetDeviceProfileInfoListCallback(pkgName, callback);
    if (ret != DM_OK) {
        LOGE("register callback error, ret: %{public}d", ret);
        return ret;
    }
    std::shared_ptr<IpcGetDeviceProfileInfoListReq> req = std::make_shared<IpcGetDeviceProfileInfoListReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetFilterOptions(filterOptions);
    ret = ipcClientProxy_->SendRequest(GET_DEVICE_PROFILE_INFO_LIST, req, rsp);
    if (ret != DM_OK) {
        LOGE("error: Send Request failed ret: %{public}d", ret);
        DeviceManagerNotify::GetInstance().OnGetDeviceProfileInfoListResult(pkgName, {},
            ERR_DM_IPC_SEND_REQUEST_FAILED);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("error: Failed with ret %{public}d", ret);
        DeviceManagerNotify::GetInstance().OnGetDeviceProfileInfoListResult(pkgName, {}, ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceIconInfo(const std::string &pkgName,
    const DmDeviceIconInfoFilterOptions &filterOptions, std::shared_ptr<GetDeviceIconInfoCallback> callback)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string uk = IpcModelCodec::GetDeviceIconInfoUniqueKey(filterOptions);
    LOGI("In pkgName:%{public}s, uk:%{public}s", pkgName.c_str(), uk.c_str());
    int32_t ret = DeviceManagerNotify::GetInstance().RegisterGetDeviceIconInfoCallback(pkgName, uk, callback);
    if (ret != DM_OK) {
        LOGE("Register Callback failed ret: %{public}d", ret);
        return ret;
    }
    std::shared_ptr<IpcGetDeviceIconInfoReq> req = std::make_shared<IpcGetDeviceIconInfoReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetFilterOptions(filterOptions);
    ret = ipcClientProxy_->SendRequest(GET_DEVICE_ICON_INFO, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        DmDeviceIconInfo deviceIconInfo;
        deviceIconInfo.InitByDmDeviceIconInfoFilterOptions(filterOptions);
        DeviceManagerNotify::GetInstance().OnGetDeviceIconInfoResult(pkgName, deviceIconInfo,
            ERR_DM_IPC_SEND_REQUEST_FAILED);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        DmDeviceIconInfo deviceIconInfo;
        deviceIconInfo.InitByDmDeviceIconInfoFilterOptions(filterOptions);
        DeviceManagerNotify::GetInstance().OnGetDeviceIconInfoResult(pkgName, deviceIconInfo, ret);
        return ret;
    }
#endif
    (void)pkgName;
    (void)filterOptions;
    (void)callback;
    return DM_OK;
}

int32_t DeviceManagerImpl::PutDeviceProfileInfoList(const std::string &pkgName,
    const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfoList)
{
    LOGI("In pkgName:%{public}s,", pkgName.c_str());
    std::shared_ptr<IpcPutDeviceProfileInfoListReq> req = std::make_shared<IpcPutDeviceProfileInfoListReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceProfileInfoList(deviceProfileInfoList);
    int32_t ret = ipcClientProxy_->SendRequest(PUT_DEVICE_PROFILE_INFO_LIST, req, rsp);
    if (ret != DM_OK) {
        LOGE("error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("error: Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
    std::string &displayName)
{
    LOGI("In pkgName:%{public}s,", pkgName.c_str());
    std::shared_ptr<IpcGetLocalDisplayDeviceNameReq> req = std::make_shared<IpcGetLocalDisplayDeviceNameReq>();
    std::shared_ptr<IpcGetLocalDisplayDeviceNameRsp> rsp = std::make_shared<IpcGetLocalDisplayDeviceNameRsp>();
    req->SetPkgName(pkgName);
    req->SetMaxNameLength(maxNameLength);
    int32_t ret = ipcClientProxy_->SendRequest(GET_LOCAL_DISPLAY_DEVICE_NAME, req, rsp);
    if (ret != DM_OK) {
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
        LOGE("error: Send Request failed ret: %{public}d", ret);
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("error: Failed with ret %{public}d", ret);
        return ret;
    }
    displayName = rsp->GetDisplayName();
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterLocalServiceInfo(const DMLocalServiceInfo &info)
{
    LOGI("Start");
    std::shared_ptr<IpcRegServiceInfoReq> req = std::make_shared<IpcRegServiceInfoReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetLocalServiceInfo(info);
    int32_t ret = ipcClientProxy_->SendRequest(REG_LOCALSERVICE_INFO, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterLocalServiceInfo(const std::string &bundleName, int32_t pinExchangeType)
{
    LOGI("Start");
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetFirstParam(bundleName);
    req->SetInt32Param(pinExchangeType);
    int32_t ret = ipcClientProxy_->SendRequest(UNREG_LOCALSERVICE_INFO, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UpdateLocalServiceInfo(const DMLocalServiceInfo &info)
{
    LOGI("Start");
    std::shared_ptr<IpcRegServiceInfoReq> req = std::make_shared<IpcRegServiceInfoReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetLocalServiceInfo(info);
    int32_t ret = ipcClientProxy_->SendRequest(UPDATE_LOCALSERVICE_INFO, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalServiceInfoByBundleNameAndPinExchangeType(
    const std::string &bundleName, int32_t pinExchangeType, DMLocalServiceInfo &info)
{
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::shared_ptr<IpcGetLocalServiceInfoRsp> rsp = std::make_shared<IpcGetLocalServiceInfoRsp>();
    req->SetFirstParam(bundleName);
    req->SetInt32Param(pinExchangeType);
    int32_t ret = ipcClientProxy_->SendRequest(GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    info = rsp->GetLocalServiceInfo();
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::SetLocalDeviceName(const std::string &pkgName, const std::string &deviceName,
    std::shared_ptr<SetLocalDeviceNameCallback> callback)
{
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    int32_t ret = DeviceManagerNotify::GetInstance().RegisterSetLocalDeviceNameCallback(pkgName, callback);
    if (ret != DM_OK) {
        LOGE("Register Callback failed ret: %{public}d", ret);
        return ret;
    }
    if (pkgName.empty() || deviceName.empty() || deviceName.size() > DEVICE_NAME_MAX_BYTES) {
        LOGE("param invalid, pkgName=%{public}s, deviceName=%{public}s",
            pkgName.c_str(), GetAnonyString(deviceName).c_str());
        DeviceManagerNotify::GetInstance().OnSetLocalDeviceNameResult(pkgName, ERR_DM_INPUT_PARA_INVALID);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::shared_ptr<IpcSetLocalDeviceNameReq> req = std::make_shared<IpcSetLocalDeviceNameReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceName(deviceName);
    ret = ipcClientProxy_->SendRequest(SET_LOCAL_DEVICE_NAME, req, rsp);
    if (ret != DM_OK) {
        LOGE("error: Send Request failed ret: %{public}d", ret);
        DeviceManagerNotify::GetInstance().OnSetLocalDeviceNameResult(pkgName, ERR_DM_IPC_SEND_REQUEST_FAILED);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("error: Failed with ret %{public}d", ret);
        DeviceManagerNotify::GetInstance().OnSetLocalDeviceNameResult(pkgName, ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::SetRemoteDeviceName(const std::string &pkgName, const std::string &deviceId,
    const std::string &deviceName, std::shared_ptr<SetRemoteDeviceNameCallback> callback)
{
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    int32_t ret = DeviceManagerNotify::GetInstance().RegisterSetRemoteDeviceNameCallback(pkgName, deviceId, callback);
    if (ret != DM_OK) {
        LOGE("Register Callback failed ret: %{public}d", ret);
        return ret;
    }
    if (pkgName.empty() || deviceName.empty() || deviceName.size() > DEVICE_NAME_MAX_BYTES || deviceId.empty()) {
        LOGE("param invalid, pkgName=%{public}s, deviceName=%{public}s, deviceId=%{public}s",
            pkgName.c_str(), GetAnonyString(deviceName).c_str(), GetAnonyString(deviceId).c_str());
        DeviceManagerNotify::GetInstance().OnSetRemoteDeviceNameResult(pkgName, deviceId, ERR_DM_INPUT_PARA_INVALID);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::shared_ptr<IpcSetRemoteDeviceNameReq> req = std::make_shared<IpcSetRemoteDeviceNameReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceName(deviceName);
    req->SetDeviceId(deviceId);
    ret = ipcClientProxy_->SendRequest(SET_REMOTE_DEVICE_NAME, req, rsp);
    if (ret != DM_OK) {
        LOGE("error: Send Request failed ret: %{public}d", ret);
        DeviceManagerNotify::GetInstance().OnSetRemoteDeviceNameResult(pkgName, deviceId,
            ERR_DM_IPC_SEND_REQUEST_FAILED);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("error: Failed with ret %{public}d", ret);
        DeviceManagerNotify::GetInstance().OnSetRemoteDeviceNameResult(pkgName, deviceId, ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::RestoreLocalDeviceName(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("param invalid, pkgName : %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = ipcClientProxy_->SendRequest(RESTORE_LOCAL_DEVICE_NAME, req, rsp);
    if (ret != DM_OK) {
        LOGE("error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("error: Failed with ret %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetDeviceNetworkIdList(const std::string &bundleName,
    const NetworkIdQueryFilter &queryFilter, std::vector<std::string> &networkIds)
{
    std::shared_ptr<IpcGetDeviceNetworkIdListReq> req = std::make_shared<IpcGetDeviceNetworkIdListReq>();
    std::shared_ptr<IpcGetDeviceNetworkIdListRsp> rsp = std::make_shared<IpcGetDeviceNetworkIdListRsp>();
    req->SetPkgName(bundleName);
    req->SetQueryFilter(queryFilter);
    int32_t ret = ipcClientProxy_->SendRequest(GET_DEVICE_NETWORK_ID_LIST, req, rsp);
    if (ret != DM_OK) {
        LOGE("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    networkIds = rsp->GetNetworkIds();
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterPinHolderCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("error: Invalid para");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify::GetInstance().UnRegisterPinHolderCallback(pkgName);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    req->SetPkgName(pkgName);
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    int32_t ret = ipcClientProxy_->SendRequest(UNREGISTER_PIN_HOLDER_CALLBACK, req, rsp);
    if (ret != DM_OK) {
        LOGI("Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("Failed with ret %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

bool DeviceManagerImpl::CheckAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("Start");
    return CheckAclByIpcCode(caller, callee, CHECK_ACCESS_CONTROL);
}

bool DeviceManagerImpl::CheckIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("Start");
    return CheckAclByIpcCode(caller, callee, CHECK_SAME_ACCOUNT);
}


bool DeviceManagerImpl::CheckSrcAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("Start");
    return CheckAclByIpcCode(caller, callee, CHECK_SRC_ACCESS_CONTROL);
}

bool DeviceManagerImpl::CheckSinkAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("Start");
    return CheckAclByIpcCode(caller, callee, CHECK_SINK_ACCESS_CONTROL);
}

bool DeviceManagerImpl::CheckSrcIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("Start");
    return CheckAclByIpcCode(caller, callee, CHECK_SRC_SAME_ACCOUNT);
}

bool DeviceManagerImpl::CheckSinkIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("Start");
    return CheckAclByIpcCode(caller, callee, CHECK_SINK_SAME_ACCOUNT);
}

bool DeviceManagerImpl::CheckAclByIpcCode(const DmAccessCaller &caller, const DmAccessCallee &callee,
        const DMIpcCmdInterfaceCode &ipcCode)
{
    LOGI("start, ipcCode %{public}d.", static_cast<int32_t>(ipcCode));
    std::shared_ptr<IpcCheckAcl> req = std::make_shared<IpcCheckAcl>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetAccessCaller(caller);
    req->SetAccessCallee(callee);
    int32_t ret = ipcClientProxy_->SendRequest(ipcCode, req, rsp);
    if (ret != DM_OK) {
        DmRadarHelper::GetInstance().ReportDmBehavior(caller.pkgName, "CheckAclByIpcCode", ret, anonyLocalUdid_);
        LOGE("CheckIsSameAccount Send Request failed ret: %{public}d", ret);
        return false;
    }
    bool result = static_cast<bool>(rsp->GetErrCode());
    DmRadarHelper::GetInstance().ReportDmBehavior(caller.pkgName, "CheckAclByIpcCode", static_cast<int32_t>(result),
        anonyLocalUdid_);
    return result;
}
} // namespace DistributedHardware
} // namespace OHOS
