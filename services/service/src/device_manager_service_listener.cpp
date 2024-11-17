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

#include <set>

#include "device_manager_service_listener.h"

#include "app_manager.h"
#include "device_manager_ipc_interface_code.h"
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
#include "ipc_notify_device_discovery_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_pin_holder_event_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_server_stub.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "datetime_ex.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#endif
#include "parameter.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
std::mutex DeviceManagerServiceListener::alreadyNotifyPkgNameLock_;
std::map<DmNotifyKey, DmDeviceInfo> DeviceManagerServiceListener::alreadyOnlinePkgName_ = {};
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
}

void DeviceManagerServiceListener::SetDeviceInfo(std::shared_ptr<IpcNotifyDeviceStateReq> pReq,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &deviceInfo,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGD("DeviceManagerServiceListener::SetDeviceInfo");
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetProcessInfo(processInfo);
    pReq->SetDeviceState(state);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(processInfo.pkgName, appId) != DM_OK) {
        pReq->SetDeviceInfo(deviceInfo);
        pReq->SetDeviceBasicInfo(deviceBasicInfo);
        return;
    }
    DmDeviceInfo dmDeviceInfo = deviceInfo;
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
    pReq->SetDeviceInfo(deviceInfo);
    pReq->SetDeviceBasicInfo(deviceBasicInfo);
}

void DeviceManagerServiceListener::ProcessDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
    const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("In");
    std::vector<ProcessInfo> processInfoVec = GetNotifyProcessInfoByUserId(processInfo.userId);
    switch (static_cast<int32_t>(state)) {
        case static_cast<int32_t>(DmDeviceState::DEVICE_STATE_ONLINE):
            ProcessDeviceOnline(processInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        case static_cast<int32_t>(DmDeviceState::DEVICE_STATE_OFFLINE):
            ProcessDeviceOffline(processInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        case static_cast<int32_t>(DmDeviceState::DEVICE_INFO_READY):
        case static_cast<int32_t>(DmDeviceState::DEVICE_INFO_CHANGED):
            ProcessDeviceInfoChange(processInfoVec, processInfo, state, info, deviceBasicInfo);
            break;
        default:
            break;
    }
}

void DeviceManagerServiceListener::ProcessAppStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
    const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("In");
    std::vector<ProcessInfo> processInfoVec = GetWhiteListSAProcessInfo();
    ProcessInfo bindProcessInfo = DealBindProcessInfo(processInfo);
    processInfoVec.push_back(bindProcessInfo);
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
    LOGI("udid %{public}s, uuid %{public}s, authForm %{public}d.", GetAnonyString(udid).c_str(),
        GetAnonyString(uuid).c_str(), authForm);
    std::shared_ptr<IpcNotifyDevTrustChangeReq> pReq = std::make_shared<IpcNotifyDevTrustChangeReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    int32_t userId = -1;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    userId = MultipleUserConnector::GetFirstForegroundUserId();
#endif
    std::vector<ProcessInfo> processInfoVec = GetNotifyProcessInfoByUserId(userId);
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
            GetNotifyProcessInfoByUserId(userId);
        for (const auto &item : processInfoVec) {
            SetDeviceScreenInfo(pReq, item, devInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_SCREEN_STATE_NOTIFY, pReq, pRsp);
        }
    } else {
        std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
        std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
        std::unordered_set<std::string> notifyPkgnames = PermissionManager::GetInstance().GetWhiteListSystemSA();
        for (const auto &item : notifyPkgnames) {
            ProcessInfo processInfo;
            processInfo.pkgName = item;
            processInfo.userId = 0;
            SetDeviceScreenInfo(pReq, processInfo, devInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_SCREEN_STATE_NOTIFY, pReq, pRsp);
        }
        SetDeviceScreenInfo(pReq, processInfo, devInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_SCREEN_STATE_NOTIFY, pReq, pRsp);
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
        GetNotifyProcessInfoByUserId(userId);
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
        if (it->first.processPkgName == pkgName) {
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

std::vector<ProcessInfo> DeviceManagerServiceListener::GetWhiteListSAProcessInfo()
{
    std::unordered_set<std::string> notifyPkgnames = PermissionManager::GetInstance().GetWhiteListSystemSA();
    std::vector<ProcessInfo> processInfos;
    for (const auto &it : notifyPkgnames) {
        ProcessInfo processInfo;
        processInfo.pkgName = it;
        processInfo.userId = 0;
        processInfos.push_back(processInfo);
    }
    return processInfos;
}

std::vector<ProcessInfo> DeviceManagerServiceListener::GetNotifyProcessInfoByUserId(int32_t userId)
{
    std::vector<ProcessInfo> processInfos = ipcServerListener_.GetAllProcessInfo();
    std::set<std::string> systemSA = ipcServerListener_.GetSystemSA();
    std::vector<ProcessInfo> processInfosTemp;
    for (auto item : processInfos) {
        if (systemSA.find(item.pkgName) != systemSA.end()) {
            item.userId = 0;
            processInfosTemp.push_back(item);
        } else if (item.userId == userId) {
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

void DeviceManagerServiceListener::ProcessDeviceOnline(const std::vector<ProcessInfo> procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    for (const auto &it : procInfoVec) {
        DmNotifyKey notifyKey;
        notifyKey.processPkgName = it.pkgName;
        notifyKey.processUserId = it.userId;
        notifyKey.notifyUserId = processInfo.userId;
        notifyKey.udid = info.deviceId;
        DmDeviceState notifyState = state;
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyKey) != alreadyOnlinePkgName_.end()) {
                notifyState = DmDeviceState::DEVICE_INFO_CHANGED;
            } else {
                alreadyOnlinePkgName_[notifyKey] = info;
            }
        }
        SetDeviceInfo(pReq, it, notifyState, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::ProcessDeviceOffline(const std::vector<ProcessInfo> procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    RemoveNotExistProcess(procInfoVec);
    if (!SoftbusCache::GetInstance().CheckIsOnline(std::string(info.deviceId))) {
        RemoveOnlinePkgName(info);
    }
    for (const auto &it : procInfoVec) {
        DmNotifyKey notifyKey;
        notifyKey.processPkgName = it.pkgName;
        notifyKey.processUserId = it.userId;
        notifyKey.notifyUserId = processInfo.userId;
        notifyKey.udid = info.deviceId;
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyKey) != alreadyOnlinePkgName_.end()) {
                alreadyOnlinePkgName_.erase(notifyKey);
            }
        }
        SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::ProcessDeviceInfoChange(const std::vector<ProcessInfo> procInfoVec,
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

void DeviceManagerServiceListener::ProcessAppOnline(const std::vector<ProcessInfo> procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    for (const auto &it : procInfoVec) {
        DmNotifyKey notifyKey;
        notifyKey.processPkgName = it.pkgName;
        notifyKey.processUserId = it.userId;
        notifyKey.notifyUserId = processInfo.userId;
        notifyKey.udid = info.deviceId;
        DmDeviceState notifyState = state;
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyKey) != alreadyOnlinePkgName_.end()) {
                notifyState = DmDeviceState::DEVICE_INFO_CHANGED;
            } else {
                alreadyOnlinePkgName_[notifyKey] = info;
            }
        }
        SetDeviceInfo(pReq, it, notifyState, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::ProcessAppOffline(const std::vector<ProcessInfo> procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId, static_cast<int32_t>(state),
        GetAnonyString(info.deviceId).c_str());
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    if (!SoftbusCache::GetInstance().CheckIsOnline(std::string(info.deviceId))) {
        RemoveOnlinePkgName(info);
        for (const auto &it : procInfoVec) {
            SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    } else {
        DmNotifyKey notifyKey;
        notifyKey.processPkgName = processInfo.pkgName;
        notifyKey.processUserId = processInfo.userId;
        notifyKey.notifyUserId = processInfo.userId;
        notifyKey.udid = info.deviceId;
        {
            std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
            if (alreadyOnlinePkgName_.find(notifyKey) != alreadyOnlinePkgName_.end()) {
                alreadyOnlinePkgName_.erase(notifyKey);
            }
        }
        SetDeviceInfo(pReq, processInfo, state, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::RemoveNotExistProcess(const std::vector<ProcessInfo> &procInfoVec)
{
    std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
    for (auto it = alreadyOnlinePkgName_.begin(); it != alreadyOnlinePkgName_.end();) {
        ProcessInfo processInfo;
        processInfo.pkgName = it->first.processPkgName;
        processInfo.userId = it->first.processUserId;
        if (find(procInfoVec.begin(), procInfoVec.end(), processInfo) != procInfoVec.end()) {
            it = alreadyOnlinePkgName_.erase(it);
        } else {
            ++it;
        }
    }
}

void DeviceManagerServiceListener::OnProcessRemove(const ProcessInfo &processInfo)
{
    std::lock_guard<std::mutex> autoLock(alreadyNotifyPkgNameLock_);
    for (auto it = alreadyOnlinePkgName_.begin(); it != alreadyOnlinePkgName_.end();) {
        if (processInfo.pkgName == it->first.processPkgName && processInfo.userId == it->first.processUserId) {
            it = alreadyOnlinePkgName_.erase(it);
        } else {
            ++it;
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
