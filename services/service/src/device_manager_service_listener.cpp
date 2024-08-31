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

#include "device_manager_service_listener.h"

#include "app_manager.h"
#include "device_manager_ipc_interface_code.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "ipc_create_pin_holder_req.h"
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
#endif
#include "parameter.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
std::mutex DeviceManagerServiceListener::alreadyOnlineSetLock_;
std::unordered_set<std::string> DeviceManagerServiceListener::alreadyOnlineSet_ = {};
const int32_t LAST_APP_ONLINE_NUMS = 8;
void DeviceManagerServiceListener::ConvertDeviceInfoToDeviceBasicInfo(const std::string &pkgName,
    const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo)
{
    (void)memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    if (memcpy_s(deviceBasicInfo.deviceName, sizeof(deviceBasicInfo.deviceName), info.deviceName,
                 std::min(sizeof(deviceBasicInfo.deviceName), sizeof(info.deviceName))) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
    }

    if (memcpy_s(deviceBasicInfo.networkId, sizeof(deviceBasicInfo.networkId), info.networkId,
        std::min(sizeof(deviceBasicInfo.networkId), sizeof(info.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed.");
    }
    if (memcpy_s(deviceBasicInfo.deviceId, sizeof(deviceBasicInfo.deviceId), info.deviceId,
        std::min(sizeof(deviceBasicInfo.deviceId), sizeof(info.deviceId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceId data failed.");
    }
    deviceBasicInfo.deviceTypeId = info.deviceTypeId;
}

void DeviceManagerServiceListener::SetDeviceInfo(std::shared_ptr<IpcNotifyDeviceStateReq> pReq,
    const std::string &pkgName, const DmDeviceState &state, const DmDeviceInfo &deviceInfo,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGD("DeviceManagerServiceListener::SetDeviceInfo");
    pReq->SetPkgName(pkgName);
    pReq->SetDeviceState(state);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(pkgName, appId) != DM_OK) {
        pReq->SetDeviceInfo(deviceInfo);
        pReq->SetDeviceBasicInfo(deviceBasicInfo);
        return;
    }
    DmDeviceInfo dmDeviceInfo = deviceInfo;
    ConfuseUdidHash(pkgName, dmDeviceInfo);
    DmDeviceBasicInfo dmDeviceBasicInfo = deviceBasicInfo;
    (void)memset_s(dmDeviceBasicInfo.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
    if (memcpy_s(dmDeviceBasicInfo.deviceId, sizeof(dmDeviceBasicInfo.deviceId), dmDeviceInfo.deviceId,
        std::min(sizeof(dmDeviceBasicInfo.deviceId), sizeof(dmDeviceInfo.deviceId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceId data failed.");
    }
    pReq->SetDeviceInfo(dmDeviceInfo);
    pReq->SetDeviceBasicInfo(dmDeviceBasicInfo);
    return;
#endif
    pReq->SetDeviceInfo(deviceInfo);
    pReq->SetDeviceBasicInfo(deviceBasicInfo);
}

std::string DeviceManagerServiceListener::ComposeOnlineKey(const std::string &pkgName, const std::string &devId)
{
    return pkgName + "_" + devId;
}

void DeviceManagerServiceListener::ProcessDeviceStateChange(const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("In");
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    std::vector<std::string> PkgNameVec = ipcServerListener_.GetAllPkgName();
    if (state == DEVICE_STATE_OFFLINE) {
        for (const auto &it : PkgNameVec) {
            std::string notifyKey = ComposeOnlineKey(it, std::string(info.deviceId));
            {
                std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
                alreadyOnlineSet_.erase(notifyKey);
            }
            SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    }
    if (state == DEVICE_STATE_ONLINE) {
        for (const auto &it : PkgNameVec) {
            std::string notifyKey = ComposeOnlineKey(it, std::string(info.deviceId));
            DmDeviceState notifyState = state;
            {
                std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
                if (alreadyOnlineSet_.find(notifyKey) != alreadyOnlineSet_.end()) {
                    notifyState = DmDeviceState::DEVICE_INFO_CHANGED;
                } else {
                    alreadyOnlineSet_.insert(notifyKey);
                }
            }
            SetDeviceInfo(pReq, it, notifyState, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    }
    if (state == DEVICE_INFO_READY || state == DEVICE_INFO_CHANGED) {
        for (const auto &it : PkgNameVec) {
            SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    }
}

void DeviceManagerServiceListener::ProcessAppStateChange(const std::string &pkgName, const DmDeviceState &state,
    const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("In");
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    std::unordered_set<std::string> notifyPkgnames = PermissionManager::GetInstance().GetSystemSA();
    notifyPkgnames.insert(pkgName);
    if (state == DEVICE_STATE_ONLINE) {
        for (const auto &it : notifyPkgnames) {
            std::string notifyKey =  it + "_" + info.deviceId;
            {
                std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
                if (alreadyOnlineSet_.find(notifyKey) != alreadyOnlineSet_.end()) {
                    continue;
                }
                alreadyOnlineSet_.insert(notifyKey);
            }
            SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    }
    if (state == DEVICE_STATE_OFFLINE) {
        if (alreadyOnlineSet_.size() == LAST_APP_ONLINE_NUMS) {
            {
                std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
                alreadyOnlineSet_.clear();
            }
            for (const auto &it : notifyPkgnames) {
                SetDeviceInfo(pReq, it, state, info, deviceBasicInfo);
                ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
            }
        } else {
            std::string notifyKey =  pkgName + "_" + info.deviceId;
            {
                std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
                if (alreadyOnlineSet_.find(notifyKey) != alreadyOnlineSet_.end()) {
                    alreadyOnlineSet_.erase(notifyKey);
                }
            }
            SetDeviceInfo(pReq, pkgName, state, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    }
    if (state == DEVICE_INFO_READY || state == DEVICE_INFO_CHANGED) {
        SetDeviceInfo(pReq, pkgName, state, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::OnDeviceStateChange(const std::string &pkgName, const DmDeviceState &state,
                                                       const DmDeviceInfo &info)
{
    LOGI("OnDeviceStateChange, state = %{public}d", state);
    DmDeviceBasicInfo deviceBasicInfo;
    ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);
    if (pkgName == std::string(DM_PKG_NAME)) {
        ProcessDeviceStateChange(state, info, deviceBasicInfo);
    } else {
        ProcessAppStateChange(pkgName, state, info, deviceBasicInfo);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    KVAdapterManager::GetInstance().DeleteAgedEntry();
#endif
}

void DeviceManagerServiceListener::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                                 const DmDeviceInfo &info)
{
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::make_shared<IpcNotifyDeviceFoundReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    DmDeviceInfo deviceInfo = info;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ConfuseUdidHash(pkgName, deviceInfo);
#endif
    DmDeviceBasicInfo devBasicInfo;
    ConvertDeviceInfoToDeviceBasicInfo(pkgName, deviceInfo, devBasicInfo);
    pReq->SetDeviceBasicInfo(devBasicInfo);
    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(subscribeId);
    pReq->SetDeviceInfo(deviceInfo);
    ipcServerListener_.SendRequest(SERVER_DEVICE_FOUND, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                                 DmDeviceBasicInfo &info)
{
    (void)pkgName;
    (void)subscribeId;
    (void)info;
}

void DeviceManagerServiceListener::OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId,
                                                     int32_t failedReason)
{
    LOGI("DeviceManagerServiceListener::OnDiscoveryFailed");
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::make_shared<IpcNotifyDiscoverResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(subscribeId);
    pReq->SetResult(failedReason);
    ipcServerListener_.SendRequest(SERVER_DISCOVER_FINISH, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId)
{
    LOGI("DeviceManagerServiceListener::OnDiscoverySuccess");
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::make_shared<IpcNotifyDiscoverResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId((uint16_t)subscribeId);
    pReq->SetResult(DM_OK);
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

void DeviceManagerServiceListener::OnAuthResult(const std::string &pkgName, const std::string &deviceId,
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
    if (ConvertUdidHashToAnoy(pkgName, deviceId, deviceIdTemp) == DM_OK) {
        pReq->SetDeviceId(deviceIdTemp);
    }
#endif
    pReq->SetPkgName(pkgName);
    pReq->SetToken(token);
    pReq->SetStatus(status);
    pReq->SetReason(reason);
    ipcServerListener_.SendRequest(SERVER_AUTH_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnUiCall(std::string &pkgName, std::string &paramJson)
{
    LOGI("OnUiCall in");
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::make_shared<IpcNotifyDMFAResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetJsonParam(paramJson);
    ipcServerListener_.SendRequest(SERVER_DEVICE_FA_NOTIFY, pReq, pRsp);
}

void DeviceManagerServiceListener::OnCredentialResult(const std::string &pkgName, int32_t action,
    const std::string &resultInfo)
{
    LOGI("call OnCredentialResult for %{public}s, action %{public}d", pkgName.c_str(), action);
    std::shared_ptr<IpcNotifyCredentialReq> pReq = std::make_shared<IpcNotifyCredentialReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetCredentialAction(action);
    pReq->SetCredentialResult(resultInfo);
    ipcServerListener_.SendRequest(SERVER_CREDENTIAL_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnBindResult(const std::string &pkgName, const PeerTargetId &targetId,
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
    if (ConvertUdidHashToAnoy(pkgName, targetId.deviceId, deviceIdTemp) == DM_OK &&
        KVAdapterManager::GetInstance().Get(deviceIdTemp, kvValue) == DM_OK) {
        returnTargetId.deviceId = deviceIdTemp;
    }
#endif
    pReq->SetPkgName(pkgName);
    pReq->SetPeerTargetId(returnTargetId);
    pReq->SetResult(result);
    pReq->SetStatus(status);
    pReq->SetContent(content);
    ipcServerListener_.SendRequest(BIND_TARGET_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnUnbindResult(const std::string &pkgName, const PeerTargetId &targetId,
    int32_t result, std::string content)
{
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::make_shared<IpcNotifyBindResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    PeerTargetId returnTargetId = targetId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string deviceIdTemp = "";
    DmKVValue kvValue;
    if (ConvertUdidHashToAnoy(pkgName, targetId.deviceId, deviceIdTemp) == DM_OK &&
        KVAdapterManager::GetInstance().Get(deviceIdTemp, kvValue) == DM_OK) {
        returnTargetId.deviceId = deviceIdTemp;
    }
#endif
    pReq->SetPkgName(pkgName);
    pReq->SetPeerTargetId(returnTargetId);
    pReq->SetResult(result);
    pReq->SetContent(content);
    ipcServerListener_.SendRequest(UNBIND_TARGET_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnPinHolderCreate(const std::string &pkgName, const std::string &deviceId,
    DmPinType pinType, const std::string &payload)
{
    LOGI("DeviceManagerServiceListener::OnPinHolderCreate : %{public}s", pkgName.c_str());
    std::shared_ptr<IpcCreatePinHolderReq> pReq = std::make_shared<IpcCreatePinHolderReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetDeviceId(deviceId);
    pReq->SetPinType(pinType);
    pReq->SetPayload(payload);
    ipcServerListener_.SendRequest(SERVER_CREATE_PIN_HOLDER, pReq, pRsp);
}

void DeviceManagerServiceListener::OnPinHolderDestroy(const std::string &pkgName, DmPinType pinType,
    const std::string &payload)
{
    LOGI("DeviceManagerServiceListener::OnPinHolderDestroy : %{public}s", pkgName.c_str());
    std::shared_ptr<IpcDestroyPinHolderReq> pReq = std::make_shared<IpcDestroyPinHolderReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetPinType(pinType);
    pReq->SetPayload(payload);
    ipcServerListener_.SendRequest(SERVER_DESTROY_PIN_HOLDER, pReq, pRsp);
}

void DeviceManagerServiceListener::OnCreateResult(const std::string &pkgName, int32_t result)
{
    LOGI("DeviceManagerServiceListener::OnCreateResult : %{public}d", result);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::make_shared<IpcNotifyPublishResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetResult(result);
    ipcServerListener_.SendRequest(SERVER_CREATE_PIN_HOLDER_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDestroyResult(const std::string &pkgName, int32_t result)
{
    LOGI("DeviceManagerServiceListener::OnDestroyResult : %{public}d", result);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::make_shared<IpcNotifyPublishResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetResult(result);
    ipcServerListener_.SendRequest(SERVER_DESTROY_PIN_HOLDER_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnPinHolderEvent(const std::string &pkgName, DmPinHolderEvent event,
    int32_t result, const std::string &content)
{
    LOGI("OnPinHolderEvent pkgName: %{public}s, event: %{public}d, result: %{public}d",
        pkgName.c_str(), event, result);
    std::shared_ptr<IpcNotifyPinHolderEventReq> pReq = std::make_shared<IpcNotifyPinHolderEventReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetPinHolderEvent(event);
    pReq->SetResult(result);
    pReq->SetContent(content);
    ipcServerListener_.SendRequest(SERVER_ON_PIN_HOLDER_EVENT, pReq, pRsp);
}
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
int32_t DeviceManagerServiceListener::ConfuseUdidHash(const std::string &pkgName, DmDeviceInfo &deviceInfo)
{
    LOGI("ConfuseUdidHash pkgName %{public}s.", pkgName.c_str());
    std::string deviceIdTemp = "";
    if (ConvertUdidHashToAnoy(pkgName, std::string(deviceInfo.deviceId), deviceIdTemp) != DM_OK) {
        LOGE("ConvertUdidHashToAnoy failed.");
        return ERR_DM_FAILED;
    }
    std::string appId = "";
    AppManager::GetInstance().GetAppIdByPkgName(pkgName, appId);
    if (appId.empty()) {
        LOGE("GetAppIdByPkgName failed.");
        return ERR_DM_FAILED;
    }

    DmKVValue kvValue;
    kvValue.udidHash = std::string(deviceInfo.deviceId);
    kvValue.appID = appId;
    kvValue.lastModifyTime = GetSecondsSince1970ToNow();
    (void)memset_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
    if (memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), deviceIdTemp.c_str(),
        std::min(sizeof(deviceInfo.deviceId), deviceIdTemp.length())) != DM_OK) {
        LOGE("ConfuseUdidHash copy deviceId data failed.");
        return ERR_DM_FAILED;
    }
    KVAdapterManager::GetInstance().Put(deviceIdTemp, kvValue);
    return DM_OK;
}

int32_t DeviceManagerServiceListener::ConvertUdidHashToAnoy(const std::string &pkgName,
    const std::string &udidHash, std::string &result)
{
    LOGI("pkgName %{public}s, udidHash %{public}s.", pkgName.c_str(), GetAnonyString(udidHash).c_str());
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(pkgName, appId) != DM_OK) {
        LOGE("GetAppIdByPkgName failed");
        return ERR_DM_FAILED;
    }
    std::string udidTemp = udidHash + appId;
    char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(udidTemp, reinterpret_cast<uint8_t *>(deviceIdHash)) != DM_OK) {
        LOGE("get deviceIdHash by udidTemp: %{public}s failed.", GetAnonyString(udidTemp).c_str());
        return ERR_DM_FAILED;
    }
    result = std::string(deviceIdHash);
    return DM_OK;
}
#endif

void DeviceManagerServiceListener::OnDeviceTrustChange(const std::string &deviceId, DmAuthForm authForm)
{
    LOGI("DeviceId %{public}s, authForm %{public}d.", GetAnonyString(deviceId).c_str(), authForm);
    std::shared_ptr<IpcNotifyDevTrustChangeReq> pReq = std::make_shared<IpcNotifyDevTrustChangeReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    std::vector<std::string> PkgNameVec = ipcServerListener_.GetAllPkgName();
    for (const auto &it : PkgNameVec) {
        pReq->SetPkgName(it);
        pReq->SetDeviceId(deviceId);
        pReq->SetAuthForm(authForm);
        ipcServerListener_.SendRequest(REMOTE_DEVICE_TRUST_CHANGE, pReq, pRsp);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
