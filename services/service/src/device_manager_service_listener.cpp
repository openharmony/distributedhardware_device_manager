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

#include <set>
#include "cJSON.h"

#include "device_manager_service_listener.h"

#include "app_manager.h"
#include "device_manager_ipc_interface_code.h"
#include "device_manager_service_notify.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_credential_auth_status_req.h"
#include "ipc_destroy_pin_holder_req.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_bind_result_req.h"
#include "ipc_notify_credential_req.h"
#include "ipc_notify_devicetrustchange_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_get_device_icon_info_req.h"
#include "ipc_notify_get_device_profile_info_list_req.h"
#include "ipc_notify_pin_holder_event_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_notify_set_local_device_name_req.h"
#include "ipc_notify_set_remote_device_name_req.h"
#include "ipc_server_stub.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "datetime_ex.h"
#include "device_name_manager.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#endif
#include "parameter.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
std::mutex DeviceManagerServiceListener::alreadyNotifyPkgNameLock_;
std::map<std::string, DmDeviceInfo> DeviceManagerServiceListener::alreadyOnlinePkgName_ = {};
std::unordered_set<std::string> DeviceManagerServiceListener::highPriorityPkgNameSet_ = { "ohos.deviceprofile",
    "ohos.distributeddata.service" };

void handleExtraData(const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo)
{
    cJSON *extraDataJsonObj = cJSON_Parse(info.extraData.c_str());
    if (extraDataJsonObj == NULL) {
        return;
    }
    cJSON *customDataJson = cJSON_GetObjectItem(extraDataJsonObj, PARAM_KEY_CUSTOM_DATA);
    if (customDataJson == NULL || !cJSON_IsString(customDataJson)) {
        cJSON_Delete(extraDataJsonObj);
        return;
    }
    char *customData = cJSON_PrintUnformatted(customDataJson);
    if (customData == nullptr) {
        cJSON_Delete(extraDataJsonObj);
        return;
    }
    cJSON_Delete(extraDataJsonObj);
    cJSON *basicExtraDataJsonObj = cJSON_CreateObject();
    if (basicExtraDataJsonObj == NULL) {
        cJSON_free(customData);
        return;
    }
    cJSON_AddStringToObject(basicExtraDataJsonObj, PARAM_KEY_CUSTOM_DATA, customData);
    char *basicExtraData = cJSON_PrintUnformatted(basicExtraDataJsonObj);
    if (basicExtraData == nullptr) {
        cJSON_free(customData);
        cJSON_Delete(basicExtraDataJsonObj);
        return;
    }
    deviceBasicInfo.extraData = std::string(basicExtraData);
    cJSON_free(customData);
    cJSON_free(basicExtraData);
    cJSON_Delete(basicExtraDataJsonObj);
}

void DeviceManagerServiceListener::ConvertDeviceInfoToDeviceBasicInfo(const std::string &pkgName,
    const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo)
{
    (void)pkgName;
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
    handleExtraData(info, deviceBasicInfo);
}

void DeviceManagerServiceListener::SetDeviceInfo(std::shared_ptr<IpcNotifyDeviceStateReq> pReq,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &deviceInfo,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetProcessInfo(processInfo);
    pReq->SetDeviceState(state);
    DmDeviceInfo dmDeviceInfo = deviceInfo;
    FillUdidAndUuidToDeviceInfo(processInfo.pkgName, dmDeviceInfo);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(processInfo.pkgName, appId) != DM_OK) {
        pReq->SetDeviceInfo(dmDeviceInfo);
        pReq->SetDeviceBasicInfo(deviceBasicInfo);
        return;
    }
    ConvertUdidHashToAnoyAndSave(processInfo.pkgName, dmDeviceInfo);
    DmDeviceBasicInfo dmDeviceBasicInfo = deviceBasicInfo;
    if (memset_s(dmDeviceBasicInfo.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice memset failed.");
        return;
    }
    if (memcpy_s(dmDeviceBasicInfo.deviceId, sizeof(dmDeviceBasicInfo.deviceId), dmDeviceInfo.deviceId,
                 std::min(sizeof(dmDeviceBasicInfo.deviceId), sizeof(dmDeviceInfo.deviceId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceId data failed.");
        return;
    }
    pReq->SetDeviceInfo(dmDeviceInfo);
    pReq->SetDeviceBasicInfo(dmDeviceBasicInfo);
    return;
#endif
    pReq->SetDeviceInfo(dmDeviceInfo);
    pReq->SetDeviceBasicInfo(deviceBasicInfo);
}

int32_t DeviceManagerServiceListener::FillUdidAndUuidToDeviceInfo(const std::string &pkgName,
    DmDeviceInfo &dmDeviceInfo)
{
    if (highPriorityPkgNameSet_.find(pkgName) == highPriorityPkgNameSet_.end()) {
        return DM_OK;
    }
    std::string udid = "";
    std::string uuid = "";
    if (SoftbusCache::GetInstance().GetUdidFromCache(dmDeviceInfo.networkId, udid) != DM_OK) {
        LOGE("GetUdidFromCache fail, networkId:%{public}s ", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    if (SoftbusCache::GetInstance().GetUuidFromCache(dmDeviceInfo.networkId, uuid) != DM_OK) {
        LOGE("GetUuidFromCache fail, networkId:%{public}s ", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    std::string extraData = dmDeviceInfo.extraData;
    if (extraData.empty()) {
        LOGE("extraData is empty, networkId:%{public}s ", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    JsonObject extraJson(extraData);
    if (extraJson.IsDiscarded()) {
        LOGE("parse extraData fail, networkId:%{public}s ", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    extraJson[PARAM_KEY_UDID] = udid;
    extraJson[PARAM_KEY_UUID] = uuid;
    dmDeviceInfo.extraData = ToString(extraJson);
    return DM_OK;
}

void DeviceManagerServiceListener::ProcessDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
    const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("DeviceManagerServiceListener::ProcessDeviceStateChange, state = %{public}d", state);
    std::vector<ProcessInfo> processInfoVec = GetNotifyProcessInfoByUserId(processInfo.userId,
        DmCommonNotifyEvent::REG_DEVICE_STATE);
    std::vector<ProcessInfo> hpProcessInfoVec;
    std::vector<ProcessInfo> lpProcessInfoVec;
    for (const auto &it : processInfoVec) {
        if (highPriorityPkgNameSet_.find(it.pkgName) != highPriorityPkgNameSet_.end()) {
            hpProcessInfoVec.push_back(it);
        } else {
            lpProcessInfoVec.push_back(it);
        }
    }

    switch (static_cast<int32_t>(state)) {
        case static_cast<int32_t>(DmDeviceState::DEVICE_STATE_ONLINE):
            ProcessDeviceOnline(hpProcessInfoVec, processInfo, state, info, deviceBasicInfo);
            ProcessDeviceOnline(lpProcessInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        case static_cast<int32_t>(DmDeviceState::DEVICE_STATE_OFFLINE):
            ProcessDeviceOffline(lpProcessInfoVec, processInfo, state, info, deviceBasicInfo);
            ProcessDeviceOffline(hpProcessInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        case static_cast<int32_t>(DmDeviceState::DEVICE_INFO_READY):
        case static_cast<int32_t>(DmDeviceState::DEVICE_INFO_CHANGED):
            ProcessDeviceInfoChange(hpProcessInfoVec, processInfo, state, info, deviceBasicInfo);
            ProcessDeviceInfoChange(lpProcessInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        default:
            break;
    }
}

void DeviceManagerServiceListener::ProcessAppStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
    const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("In");
    std::vector<ProcessInfo> processInfoVec = GetWhiteListSAProcessInfo(DmCommonNotifyEvent::REG_DEVICE_STATE);
    ProcessInfo bindProcessInfo = DealBindProcessInfo(processInfo);
    processInfoVec.push_back(bindProcessInfo);
    std::vector<ProcessInfo> allProcessInfos = ipcServerListener_.GetAllProcessInfo();
    for (auto item : allProcessInfos) {
        if (item.pkgName.find(PICKER_PROXY_SPLIT + processInfo.pkgName) != std::string::npos) {
            processInfoVec.push_back(item);
        }
    }
    switch (static_cast<int32_t>(state)) {
        case static_cast<int32_t>(DmDeviceState::DEVICE_STATE_ONLINE):
            ProcessAppOnline(processInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        case static_cast<int32_t>(DmDeviceState::DEVICE_STATE_OFFLINE):
            ProcessAppOffline(processInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        case static_cast<int32_t>(DmDeviceState::DEVICE_INFO_READY):
        case static_cast<int32_t>(DmDeviceState::DEVICE_INFO_CHANGED):
            ProcessDeviceInfoChange(processInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        default:
            break;
    }
}

void DeviceManagerServiceListener::OnDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
                                                       const DmDeviceInfo &info)
{
    LOGI("OnDeviceStateChange, state = %{public}d", state);
    DmDeviceBasicInfo deviceBasicInfo;
    ConvertDeviceInfoToDeviceBasicInfo(processInfo.pkgName, info, deviceBasicInfo);
    if (processInfo.pkgName == std::string(DM_PKG_NAME)) {
        ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo);
    } else {
        ProcessAppStateChange(processInfo, state, info, deviceBasicInfo);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    KVAdapterManager::GetInstance().DeleteAgedEntry();
#endif
}

void DeviceManagerServiceListener::OnDeviceFound(const ProcessInfo &processInfo, uint16_t subscribeId,
                                                 const DmDeviceInfo &info)
{
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::make_shared<IpcNotifyDeviceFoundReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    DmDeviceInfo deviceInfo = info;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ConvertUdidHashToAnoyAndSave(processInfo.pkgName, deviceInfo);
#endif
    DmDeviceBasicInfo devBasicInfo;
    ConvertDeviceInfoToDeviceBasicInfo(processInfo.pkgName, deviceInfo, devBasicInfo);
    pReq->SetDeviceBasicInfo(devBasicInfo);
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetSubscribeId(subscribeId);
    pReq->SetDeviceInfo(deviceInfo);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_DEVICE_FOUND, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDiscoveryFailed(const ProcessInfo &processInfo, uint16_t subscribeId,
                                                     int32_t failedReason)
{
    LOGI("DeviceManagerServiceListener::OnDiscoveryFailed");
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::make_shared<IpcNotifyDiscoverResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetSubscribeId(subscribeId);
    pReq->SetResult(failedReason);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_DISCOVER_FINISH, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDiscoverySuccess(const ProcessInfo &processInfo, int32_t subscribeId)
{
    LOGI("DeviceManagerServiceListener::OnDiscoverySuccess");
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::make_shared<IpcNotifyDiscoverResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetSubscribeId((uint16_t)subscribeId);
    pReq->SetResult(DM_OK);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_DISCOVER_FINISH, pReq, pRsp);
}

void DeviceManagerServiceListener::OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult)
{
    LOGI("DeviceManagerServiceListener::OnPublishResult : %{public}d", publishResult);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::make_shared<IpcNotifyPublishResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetPublishId(publishId);
    pReq->SetResult(publishResult);
    ipcServerListener_.SendRequest(SERVER_PUBLISH_FINISH, pReq, pRsp);
}

void DeviceManagerServiceListener::OnAuthResult(const ProcessInfo &processInfo, const std::string &deviceId,
                                                const std::string &token, int32_t status, int32_t reason)
{
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::make_shared<IpcNotifyAuthResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    if (status < STATUS_DM_AUTH_FINISH && status > STATUS_DM_AUTH_DEFAULT) {
        status = STATUS_DM_AUTH_DEFAULT;
    }
    pReq->SetDeviceId(deviceId);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string deviceIdTemp = "";
    if (ConvertUdidHashToAnoyDeviceId(processInfo.pkgName, deviceId, deviceIdTemp) == DM_OK) {
        pReq->SetDeviceId(deviceIdTemp);
    }
#endif
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetToken(token);
    pReq->SetStatus(status);
    pReq->SetReason(reason);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_AUTH_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnUiCall(const ProcessInfo &processInfo, std::string &paramJson)
{
    LOGI("OnUiCall in");
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::make_shared<IpcNotifyDMFAResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetJsonParam(paramJson);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_DEVICE_FA_NOTIFY, pReq, pRsp);
}

void DeviceManagerServiceListener::OnCredentialResult(const ProcessInfo &processInfo, int32_t action,
    const std::string &resultInfo)
{
    LOGI("call OnCredentialResult for %{public}s, action %{public}d", processInfo.pkgName.c_str(), action);
    std::shared_ptr<IpcNotifyCredentialReq> pReq = std::make_shared<IpcNotifyCredentialReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetCredentialAction(action);
    pReq->SetCredentialResult(resultInfo);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_CREDENTIAL_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId,
    int32_t result, int32_t status, std::string content)
{
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::make_shared<IpcNotifyBindResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    if (status < STATUS_DM_AUTH_FINISH && status > STATUS_DM_AUTH_DEFAULT) {
        status = STATUS_DM_AUTH_DEFAULT;
    }
    PeerTargetId returnTargetId = targetId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string deviceIdTemp = "";
    DmKVValue kvValue;
    if (ConvertUdidHashToAnoyDeviceId(processInfo.pkgName, targetId.deviceId, deviceIdTemp) == DM_OK &&
        KVAdapterManager::GetInstance().Get(deviceIdTemp, kvValue) == DM_OK) {
        returnTargetId.deviceId = deviceIdTemp;
    }
#endif
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetPeerTargetId(returnTargetId);
    pReq->SetResult(result);
    pReq->SetStatus(status);
    pReq->SetContent(content);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(BIND_TARGET_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnUnbindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId,
    int32_t result, std::string content)
{
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::make_shared<IpcNotifyBindResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    PeerTargetId returnTargetId = targetId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string deviceIdTemp = "";
    DmKVValue kvValue;
    if (ConvertUdidHashToAnoyDeviceId(processInfo.pkgName, targetId.deviceId, deviceIdTemp) == DM_OK &&
        KVAdapterManager::GetInstance().Get(deviceIdTemp, kvValue) == DM_OK) {
        returnTargetId.deviceId = deviceIdTemp;
    }
#endif
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetPeerTargetId(returnTargetId);
    pReq->SetResult(result);
    pReq->SetContent(content);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(UNBIND_TARGET_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnPinHolderCreate(const ProcessInfo &processInfo, const std::string &deviceId,
    DmPinType pinType, const std::string &payload)
{
    LOGI("DeviceManagerServiceListener::OnPinHolderCreate : %{public}s", processInfo.pkgName.c_str());
    std::shared_ptr<IpcCreatePinHolderReq> pReq = std::make_shared<IpcCreatePinHolderReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetDeviceId(deviceId);
    pReq->SetPinType(pinType);
    pReq->SetPayload(payload);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_CREATE_PIN_HOLDER, pReq, pRsp);
}

void DeviceManagerServiceListener::OnPinHolderDestroy(const ProcessInfo &processInfo, DmPinType pinType,
    const std::string &payload)
{
    LOGI("DeviceManagerServiceListener::OnPinHolderDestroy : %{public}s", processInfo.pkgName.c_str());
    std::shared_ptr<IpcDestroyPinHolderReq> pReq = std::make_shared<IpcDestroyPinHolderReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetPinType(pinType);
    pReq->SetPayload(payload);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_DESTROY_PIN_HOLDER, pReq, pRsp);
}

void DeviceManagerServiceListener::OnCreateResult(const ProcessInfo &processInfo, int32_t result)
{
    LOGI("DeviceManagerServiceListener::OnCreateResult : %{public}d", result);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::make_shared<IpcNotifyPublishResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetResult(result);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_CREATE_PIN_HOLDER_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDestroyResult(const ProcessInfo &processInfo, int32_t result)
{
    LOGI("DeviceManagerServiceListener::OnDestroyResult : %{public}d", result);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::make_shared<IpcNotifyPublishResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetResult(result);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_DESTROY_PIN_HOLDER_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnPinHolderEvent(const ProcessInfo &processInfo, DmPinHolderEvent event,
    int32_t result, const std::string &content)
{
    LOGI("OnPinHolderEvent pkgName: %{public}s, event: %{public}d, result: %{public}d",
        processInfo.pkgName.c_str(), event, result);
    std::shared_ptr<IpcNotifyPinHolderEventReq> pReq = std::make_shared<IpcNotifyPinHolderEventReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetPinHolderEvent(event);
    pReq->SetResult(result);
    pReq->SetContent(content);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_ON_PIN_HOLDER_EVENT, pReq, pRsp);
}
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
int32_t DeviceManagerServiceListener::ConvertUdidHashToAnoyAndSave(const std::string &pkgName, DmDeviceInfo &deviceInfo)
{
    LOGD("pkgName %{public}s.", pkgName.c_str());
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(pkgName, appId) != DM_OK) {
        LOGD("GetAppIdByPkgName failed");
        return ERR_DM_FAILED;
    }
    DmKVValue kvValue;
    int32_t ret = Crypto::ConvertUdidHashToAnoyAndSave(appId, std::string(deviceInfo.deviceId), kvValue);
    if (ret != DM_OK) {
        return ERR_DM_FAILED;
    }
    if (memset_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN) != DM_OK) {
        LOGE("ConvertUdidHashToAnoyAndSave memset failed.");
        return ERR_DM_FAILED;
    }
    if (memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), kvValue.anoyDeviceId.c_str(),
                 std::min(sizeof(deviceInfo.deviceId), kvValue.anoyDeviceId.length())) != DM_OK) {
        LOGE("copy deviceId data failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceListener::ConvertUdidHashToAnoyDeviceId(const std::string &pkgName,
    const std::string &udidHash, std::string &anoyDeviceId)
{
    LOGI("pkgName %{public}s, udidHash %{public}s.", pkgName.c_str(), GetAnonyString(udidHash).c_str());
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(pkgName, appId) != DM_OK) {
        LOGD("GetAppIdByPkgName failed");
        return ERR_DM_FAILED;
    }
    DmKVValue kvValue;
    int32_t ret = Crypto::ConvertUdidHashToAnoyDeviceId(appId, udidHash, kvValue);
    if (ret == DM_OK) {
        anoyDeviceId = kvValue.anoyDeviceId;
    }
    return ret;
}
#endif

void DeviceManagerServiceListener::OnDeviceTrustChange(const std::string &udid, const std::string &uuid,
    DmAuthForm authForm)
{
    LOGI("udid %{public}s, authForm %{public}d, uuid %{public}s.", GetAnonyString(udid).c_str(),
        authForm, GetAnonyString(uuid).c_str());
    std::shared_ptr<IpcNotifyDevTrustChangeReq> pReq = std::make_shared<IpcNotifyDevTrustChangeReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    int32_t userId = -1;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    userId = MultipleUserConnector::GetFirstForegroundUserId();
#endif
    std::vector<ProcessInfo> processInfoVec = GetNotifyProcessInfoByUserId(userId,
        DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE);
    for (const auto &item : processInfoVec) {
        pReq->SetPkgName(item.pkgName);
        pReq->SetUdid(udid);
        pReq->SetUuid(uuid);
        pReq->SetAuthForm(authForm);
        pReq->SetProcessInfo(item);
        ipcServerListener_.SendRequest(REMOTE_DEVICE_TRUST_CHANGE, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::SetDeviceScreenInfo(std::shared_ptr<IpcNotifyDeviceStateReq> pReq,
    const ProcessInfo &processInfo, const DmDeviceInfo &deviceInfo)
{
    LOGI("In");
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetProcessInfo(processInfo);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(processInfo.pkgName, appId) != DM_OK) {
        pReq->SetDeviceInfo(deviceInfo);
        return;
    }
    DmDeviceInfo dmDeviceInfo = deviceInfo;
    ConvertUdidHashToAnoyAndSave(processInfo.pkgName, dmDeviceInfo);
    pReq->SetDeviceInfo(dmDeviceInfo);
    return;
#endif
    pReq->SetDeviceInfo(deviceInfo);
}

void DeviceManagerServiceListener::OnDeviceScreenStateChange(const ProcessInfo &processInfo, DmDeviceInfo &devInfo)
{
    LOGI("In, pkgName = %{public}s", processInfo.pkgName.c_str());
    if (processInfo.pkgName == std::string(DM_PKG_NAME)) {
        std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
        std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
        int32_t userId = -1;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        userId = MultipleUserConnector::GetFirstForegroundUserId();
#endif
        std::vector<ProcessInfo> processInfoVec =
            GetNotifyProcessInfoByUserId(userId, DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE);
        for (const auto &item : processInfoVec) {
            SetDeviceScreenInfo(pReq, item, devInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_SCREEN_STATE_NOTIFY, pReq, pRsp);
        }
    } else {
        std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
        std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
        std::vector<ProcessInfo> processInfoVec =
            GetWhiteListSAProcessInfo(DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE);
        processInfoVec.push_back(processInfo);
        for (const auto &item : processInfoVec) {
            SetDeviceScreenInfo(pReq, item, devInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_SCREEN_STATE_NOTIFY, pReq, pRsp);
        }
    }
}

void DeviceManagerServiceListener::RemoveOnlinePkgName(const DmDeviceInfo &info)
{
    LOGI("udidHash: %{public}s.", GetAnonyString(info.deviceId).c_str());
    {
        std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
        for (auto item = alreadyOnlinePkgName_.begin(); item != alreadyOnlinePkgName_.end();) {
            if (std::string(item->second.deviceId) == std::string(info.deviceId)) {
                item = alreadyOnlinePkgName_.erase(item);
            } else {
                ++item;
            }
        }
    }
}

void DeviceManagerServiceListener::OnCredentialAuthStatus(const ProcessInfo &processInfo,
    const std::string &deviceList, uint16_t deviceTypeId, int32_t errcode)
{
    LOGI("In, pkgName = %{public}s", processInfo.pkgName.c_str());
    int32_t userId = -1;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    userId = MultipleUserConnector::GetFirstForegroundUserId();
#endif
    std::vector<ProcessInfo> processInfoVec =
        GetNotifyProcessInfoByUserId(userId, DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY);
    for (const auto &item : processInfoVec) {
        std::shared_ptr<IpcNotifyCredentialAuthStatusReq> pReq = std::make_shared<IpcNotifyCredentialAuthStatusReq>();
        std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
        pReq->SetDeviceList(deviceList);
        pReq->SetDeviceTypeId(deviceTypeId);
        pReq->SetErrCode(errcode);
        pReq->SetPkgName(item.pkgName);
        pReq->SetProcessInfo(item);
        ipcServerListener_.SendRequest(SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::OnAppUnintall(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
    for (auto it = alreadyOnlinePkgName_.begin(); it != alreadyOnlinePkgName_.end();) {
        if (it->first.find(pkgName) != std::string::npos) {
            it = alreadyOnlinePkgName_.erase(it);
        } else {
            ++it;
        }
    }
}

void DeviceManagerServiceListener::OnSinkBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId,
    int32_t result, int32_t status, std::string content)
{
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::make_shared<IpcNotifyBindResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    if (status < STATUS_DM_AUTH_FINISH && status > STATUS_DM_AUTH_DEFAULT) {
        status = STATUS_DM_AUTH_DEFAULT;
    }
    PeerTargetId returnTargetId = targetId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string deviceIdTemp = "";
    DmKVValue kvValue;
    if (ConvertUdidHashToAnoyDeviceId(processInfo.pkgName, targetId.deviceId, deviceIdTemp) == DM_OK &&
        KVAdapterManager::GetInstance().Get(deviceIdTemp, kvValue) == DM_OK) {
        returnTargetId.deviceId = deviceIdTemp;
    }
#endif
    pReq->SetPkgName(processInfo.pkgName);
    std::vector<ProcessInfo> processInfos = ipcServerListener_.GetAllProcessInfo();
    ProcessInfo processInfoTemp;
    for (auto item : processInfos) {
        if (item.pkgName == processInfo.pkgName) {
            processInfoTemp = item;
        }
    }
    if (processInfoTemp.pkgName.empty()) {
        LOGI("not register listener");
        return;
    }
    pReq->SetProcessInfo(processInfoTemp);
    pReq->SetPeerTargetId(returnTargetId);
    pReq->SetResult(result);
    pReq->SetStatus(status);
    pReq->SetContent(content);
    ipcServerListener_.SendRequest(SINK_BIND_TARGET_RESULT, pReq, pRsp);
}

std::vector<ProcessInfo> DeviceManagerServiceListener::GetWhiteListSAProcessInfo(
    DmCommonNotifyEvent dmCommonNotifyEvent)
{
    if (!IsDmCommonNotifyEventValid(dmCommonNotifyEvent)) {
        LOGE("Invalid dmCommonNotifyEvent: %{public}d.", dmCommonNotifyEvent);
        return {};
    }
    std::set<ProcessInfo> notifyProcessInfos;
    DeviceManagerServiceNotify::GetInstance().GetCallBack(dmCommonNotifyEvent, notifyProcessInfos);
    if (notifyProcessInfos.size() == 0) {
        LOGE("callback not exist dmCommonNotifyEvent: %{public}d", dmCommonNotifyEvent);
        return {};
    }
    std::unordered_set<std::string> notifyPkgnames = PermissionManager::GetInstance().GetWhiteListSystemSA();
    std::vector<ProcessInfo> processInfos;
    for (const auto &it : notifyPkgnames) {
        ProcessInfo processInfo;
        processInfo.pkgName = it;
        processInfo.userId = 0;
        if (notifyProcessInfos.find(processInfo) == notifyProcessInfos.end()) {
            continue;
        }
        processInfos.push_back(processInfo);
    }
    return processInfos;
}

std::vector<ProcessInfo> DeviceManagerServiceListener::GetNotifyProcessInfoByUserId(int32_t userId,
    DmCommonNotifyEvent dmCommonNotifyEvent)
{
    if (!IsDmCommonNotifyEventValid(dmCommonNotifyEvent)) {
        LOGE("Invalid dmCommonNotifyEvent: %{public}d.", dmCommonNotifyEvent);
        return {};
    }
    std::set<ProcessInfo> notifyProcessInfos;
    DeviceManagerServiceNotify::GetInstance().GetCallBack(dmCommonNotifyEvent, notifyProcessInfos);
    if (notifyProcessInfos.size() == 0) {
        LOGE("callback not exist dmCommonNotifyEvent: %{public}d", dmCommonNotifyEvent);
        return {};
    }
    std::vector<ProcessInfo> processInfos = ipcServerListener_.GetAllProcessInfo();
    std::set<std::string> systemSA = ipcServerListener_.GetSystemSA();
    std::vector<ProcessInfo> processInfosTemp;
    for (auto item : processInfos) {
        if (systemSA.find(item.pkgName) != systemSA.end()) {
            item.userId = 0;
            if (notifyProcessInfos.find(item) == notifyProcessInfos.end()) {
                continue;
            }
            processInfosTemp.push_back(item);
        } else if (item.userId == userId) {
            if (notifyProcessInfos.find(item) == notifyProcessInfos.end()) {
                continue;
            }
            processInfosTemp.push_back(item);
        }
    }
    return processInfosTemp;
}

ProcessInfo DeviceManagerServiceListener::DealBindProcessInfo(const ProcessInfo &processInfo)
{
    std::set<std::string> systemSA = ipcServerListener_.GetSystemSA();
    if (systemSA.find(processInfo.pkgName) == systemSA.end()) {
        return processInfo;
    }
    ProcessInfo bindProcessInfo = processInfo;
    bindProcessInfo.userId = 0;
    return bindProcessInfo;
}

void DeviceManagerServiceListener::ProcessDeviceOnline(const std::vector<ProcessInfo> &procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    for (const auto &it : procInfoVec) {
        std::string notifyPkgName = it.pkgName + "#" + std::to_string(it.userId) + "#" + std::string(info.deviceId);
        DmDeviceState notifyState = state;
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyPkgName) != alreadyOnlinePkgName_.end()) {
                notifyState = DmDeviceState::DEVICE_INFO_CHANGED;
            } else {
                alreadyOnlinePkgName_[notifyPkgName] = info;
            }
        }
        SetDeviceInfo(pReq, it, notifyState, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::ProcessDeviceOffline(const std::vector<ProcessInfo> &procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    RemoveNotExistProcess();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    for (const auto &it : procInfoVec) {
        std::string notifyPkgName = it.pkgName + "#" + std::to_string(it.userId) + "#" + std::string(info.deviceId);
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyPkgName) != alreadyOnlinePkgName_.end()) {
                alreadyOnlinePkgName_.erase(notifyPkgName);
            } else {
                continue;
            }
        }
        SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::ProcessDeviceInfoChange(const std::vector<ProcessInfo> &procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    for (const auto &it : procInfoVec) {
        SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::ProcessAppOnline(const std::vector<ProcessInfo> &procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    for (const auto &it : procInfoVec) {
        std::string notifyPkgName = it.pkgName + "#" + std::to_string(it.userId) + "#" + std::string(info.deviceId);
        DmDeviceState notifyState = state;
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyPkgName) != alreadyOnlinePkgName_.end()) {
                notifyState = DmDeviceState::DEVICE_INFO_CHANGED;
            } else {
                alreadyOnlinePkgName_[notifyPkgName] = info;
            }
        }
        LOGI("ProcessAppOnline notifyState = %{public}d", notifyState);
        SetDeviceInfo(pReq, it, notifyState, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::ProcessAppOffline(const std::vector<ProcessInfo> &procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    RemoveNotExistProcess();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    if (!SoftbusCache::GetInstance().CheckIsOnline(std::string(info.deviceId))) {
        for (const auto &it : procInfoVec) {
            std::string notifyPkgName = it.pkgName + "#" + std::to_string(it.userId) + "#" + std::string(info.deviceId);
            {
                std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
                if (alreadyOnlinePkgName_.find(notifyPkgName) != alreadyOnlinePkgName_.end()) {
                    alreadyOnlinePkgName_.erase(notifyPkgName);
                } else {
                    continue;
                }
            }
            SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    } else {
        std::string notifyPkgName = processInfo.pkgName + "#" + std::to_string(processInfo.userId) + "#" +
            std::string(info.deviceId);
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyPkgName) != alreadyOnlinePkgName_.end()) {
                alreadyOnlinePkgName_.erase(notifyPkgName);
            } else {
                return;
            }
        }
        SetDeviceInfo(pReq, processInfo, state, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::OnProcessRemove(const ProcessInfo &processInfo)
{
    std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
    std::string notifyPkgName = processInfo.pkgName + "#" + std::to_string(processInfo.userId);
    for (auto it = alreadyOnlinePkgName_.begin(); it != alreadyOnlinePkgName_.end();) {
        if (it->first.find(notifyPkgName) != std::string::npos) {
            it = alreadyOnlinePkgName_.erase(it);
        } else {
            ++it;
        }
    }
}

void DeviceManagerServiceListener::OnDevStateCallbackAdd(const ProcessInfo &processInfo,
    const std::vector<DmDeviceInfo> &deviceList)
{
    for (auto item : deviceList) {
        std::string notifyPkgName = processInfo.pkgName + "#" + std::to_string(processInfo.userId) + "#" +
            std::string(item.deviceId);
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyPkgName) != alreadyOnlinePkgName_.end()) {
                continue;
            }
            alreadyOnlinePkgName_[notifyPkgName] = item;
        }
        DmDeviceBasicInfo deviceBasicInfo;
        std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
        std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
        ConvertDeviceInfoToDeviceBasicInfo(processInfo.pkgName, item, deviceBasicInfo);
        SetDeviceInfo(pReq, processInfo, DmDeviceState::DEVICE_STATE_ONLINE, item, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::RemoveNotExistProcess()
{
    std::set<ProcessInfo> notifyProcessInfos;
    std::set<std::string> notifyPkgNames;
    DeviceManagerServiceNotify::GetInstance().GetCallBack(DmCommonNotifyEvent::REG_DEVICE_STATE, notifyProcessInfos);
    int32_t pkgNameIndex = 0;
    for (ProcessInfo processInfo : notifyProcessInfos) {
        notifyPkgNames.insert(processInfo.pkgName);
    }
    std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
    for (auto it = alreadyOnlinePkgName_.begin(); it != alreadyOnlinePkgName_.end();) {
        std::string pkgName = GetSubStr(it->first, "#", pkgNameIndex);
        if (find(notifyPkgNames.begin(), notifyPkgNames.end(), pkgName) == notifyPkgNames.end()) {
            it = alreadyOnlinePkgName_.erase(it);
            LOGI("pkgName %{public}s.", pkgName.c_str());
        } else {
            ++it;
        }
    }
}

void DeviceManagerServiceListener::OnGetDeviceProfileInfoListResult(const ProcessInfo &processInfo,
    const std::vector<DmDeviceProfileInfo> &deviceProfileInfos, int32_t code)
{
    LOGI("pkgName %{public}s.", processInfo.pkgName.c_str());
    std::shared_ptr<IpcNotifyGetDeviceProfileInfoListReq> pReq =
        std::make_shared<IpcNotifyGetDeviceProfileInfoListReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetDeviceProfileInfoList(deviceProfileInfos);
    pReq->SetResult(code);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(GET_DEVICE_PROFILE_INFO_LIST_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnGetDeviceIconInfoResult(const ProcessInfo &processInfo,
    const DmDeviceIconInfo &dmDeviceIconInfo, int32_t code)
{
    LOGI("pkgName %{public}s.", processInfo.pkgName.c_str());
    std::shared_ptr<IpcNotifyGetDeviceIconInfoReq> pReq = std::make_shared<IpcNotifyGetDeviceIconInfoReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetDmDeviceIconInfo(dmDeviceIconInfo);
    pReq->SetResult(code);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(GET_DEVICE_ICON_INFO_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnSetLocalDeviceNameResult(const ProcessInfo &processInfo,
    const std::string &deviceName, int32_t code)
{
    LOGI("pkgName %{public}s.", processInfo.pkgName.c_str());
    std::shared_ptr<IpcNotifySetLocalDeviceNameReq> pReq = std::make_shared<IpcNotifySetLocalDeviceNameReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetResult(code);
    pReq->SetProcessInfo(processInfo);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (code == DM_OK) {
        DeviceNameManager::GetInstance().ModifyUserDefinedName(deviceName);
    }
#endif
    ipcServerListener_.SendRequest(SET_LOCAL_DEVICE_NAME_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnSetRemoteDeviceNameResult(const ProcessInfo &processInfo,
    const std::string &deviceId, const std::string &deviceName, int32_t code)
{
    LOGI("pkgName %{public}s.", processInfo.pkgName.c_str());
    std::shared_ptr<IpcNotifySetRemoteDeviceNameReq> pReq = std::make_shared<IpcNotifySetRemoteDeviceNameReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetDeviceId(deviceId);
    pReq->SetResult(code);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SET_REMOTE_DEVICE_NAME_RESULT, pReq, pRsp);
}
} // namespace DistributedHardware
} // namespace OHOS
