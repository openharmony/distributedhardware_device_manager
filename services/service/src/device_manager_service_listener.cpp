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

#include "device_manager_ipc_interface_code.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_destroy_pin_holder_req.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_bind_result_req.h"
#include "ipc_notify_credential_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_discovery_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_pin_holder_event_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_server_stub.h"

namespace OHOS {
namespace DistributedHardware {
std::mutex DeviceManagerServiceListener::dmListenerMapLock_;
std::mutex DeviceManagerServiceListener::udidHashMapLock_;
std::map<std::string, std::string> DeviceManagerServiceListener::dmListenerMap_ = {};
std::map<std::string, std::map<std::string, std::string>> DeviceManagerServiceListener::udidHashMap_ = {};
std::mutex DeviceManagerServiceListener::alreadyOnlineSetLock_;
std::unordered_set<std::string> DeviceManagerServiceListener::alreadyOnlineSet_ = {};

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
    LOGI("DeviceManagerServiceListener::SetDeviceInfo");
    pReq->SetPkgName(pkgName);
    pReq->SetDeviceState(state);
    pReq->SetDeviceInfo(deviceInfo);
    pReq->SetDeviceBasicInfo(deviceBasicInfo);
}

void DeviceManagerServiceListener::OnDeviceStateChange(const std::string &pkgName, const DmDeviceState &state,
                                                       const DmDeviceInfo &info)
{
    LOGI("OnDeviceStateChange, state = %{public}d", state);
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    DmDeviceBasicInfo deviceBasicInfo;
    ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);
    if (pkgName == std::string(DM_PKG_NAME)) {
        std::vector<std::string> PkgNameVec = ipcServerListener_.GetAllPkgName();
        if (state == DEVICE_STATE_OFFLINE) {
            {
                std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
                alreadyOnlineSet_.clear();
            }
        }
        for (const auto &it : PkgNameVec) {
            std::string notifyKey =  it + "_" + info.deviceId;
            DmDeviceState notifyState = state;
            {
                std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
                if (state == DEVICE_STATE_ONLINE && alreadyOnlineSet_.find(notifyKey) != alreadyOnlineSet_.end()) {
                    notifyState = DmDeviceState::DEVICE_INFO_CHANGED;
                } else if (state == DEVICE_STATE_ONLINE) {
                    alreadyOnlineSet_.insert(notifyKey);
                }
            }
            SetDeviceInfo(pReq, it, notifyState, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    } else {
        std::string notifyKey =  pkgName + "_" + info.deviceId;
        {
            std::lock_guard<std::mutex> autoLock(alreadyOnlineSetLock_);
            if (state == DEVICE_STATE_ONLINE) {
                alreadyOnlineSet_.insert(notifyKey);
            } else if (state == DEVICE_STATE_OFFLINE) {
                alreadyOnlineSet_.erase(notifyKey);
            }
        }
        SetDeviceInfo(pReq, pkgName, state, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    #if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::set<std::string> set = IpcServerStub::GetInstance().GetSaPkgname();
        for (const auto &item : set) {
            LOGI("Notify SA pkgname %s", item.c_str());
            SetDeviceInfo(pReq, item, state, info, deviceBasicInfo);
            ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
        }
    #endif
    }
}

void DeviceManagerServiceListener::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                                 const DmDeviceInfo &info)
{
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::make_shared<IpcNotifyDeviceFoundReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    DmDeviceBasicInfo devBasicInfo;
    ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, devBasicInfo);
    pReq->SetDeviceBasicInfo(devBasicInfo);
    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(subscribeId);
    pReq->SetDeviceInfo(info);
    ipcServerListener_.SendRequest(SERVER_DEVICE_FOUND, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                                 DmDeviceBasicInfo &info)
{
    std::shared_ptr<IpcNotifyDeviceDiscoveryReq> pReq = std::make_shared<IpcNotifyDeviceDiscoveryReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    std::string udIdHash = CalcDeviceId(pkgName, info.deviceId);
    if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, udIdHash.c_str(), udIdHash.length()) != DM_OK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceId data failed.");
    }
    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(subscribeId);
    pReq->SetDeviceBasicInfo(info);
    ipcServerListener_.SendRequest(SERVER_DEVICE_DISCOVERY, pReq, pRsp);
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

    pReq->SetPkgName(pkgName);
    pReq->SetDeviceId(deviceId);
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

    pReq->SetPkgName(pkgName);
    pReq->SetPeerTargetId(targetId);
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

    pReq->SetPkgName(pkgName);
    pReq->SetPeerTargetId(targetId);
    pReq->SetResult(result);
    pReq->SetContent(content);
    ipcServerListener_.SendRequest(UNBIND_TARGET_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::RegisterDmListener(const std::string &pkgName, const std::string &appId)
{
    std::lock_guard<std::mutex> autoLock(dmListenerMapLock_);
    dmListenerMap_[pkgName] = appId;
}

void DeviceManagerServiceListener::UnRegisterDmListener(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(dmListenerMapLock_);
    dmListenerMap_.erase(pkgName);
}

void DeviceManagerServiceListener::DeleteDeviceIdFromMap(const std::string &deviceId, const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(udidHashMapLock_);
    std::map<std::string, std::string> &udidMap = udidHashMap_[pkgName];
    auto iter = udidMap.find(deviceId);
    if (iter == udidMap.end()) {
        return;
    }
    udidMap.erase(deviceId);
}
void DeviceManagerServiceListener::SetUdidHashMap(const std::string &udidHash, const std::string &deviceId,
    const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(udidHashMapLock_);
    udidHashMap_[pkgName][deviceId] = udidHash;
}

std::string DeviceManagerServiceListener::GetDeviceId(const std::string &udidHash, const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(udidHashMapLock_);
    std::map<std::string, std::string> &udidMap = udidHashMap_[pkgName];
    for (auto iter = udidMap.begin(); iter != udidMap.end(); iter++) {
        if (udidHash == iter->second) {
            return iter->first;
        }
    }
    return "";
}

std::string DeviceManagerServiceListener::GetUdidHash(const std::string &deviceId, const std::string &pkgName)
{
    std::lock_guard<std::mutex> lock(udidHashMapLock_);
    return udidHashMap_[pkgName].count(deviceId) > 0 ?  udidHashMap_[pkgName][deviceId] : "";
}

std::string DeviceManagerServiceListener::GetAppId(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(dmListenerMapLock_);
    return dmListenerMap_.count(pkgName) > 0 ? dmListenerMap_[pkgName] : "";
}

std::string DeviceManagerServiceListener::CalcDeviceId(const std::string &pkgName, const std::string &udidHash)
{
    std::string appId = GetAppId(pkgName);
    LOGI("CalcDeviceId, appId : %{public}s, udidHash : %{public}s.", GetAnonyString(appId).c_str(),
        GetAnonyString(udidHash).c_str());
    if (appId.empty()) {
        return udidHash;
    }
    std::string deviceId = GetDeviceId(udidHash, pkgName);
    if (deviceId.empty()) {
        deviceId = Crypto::Sha256(appId + udidHash);
        SetUdidHashMap(udidHash, deviceId, pkgName);
        return deviceId;
    }
    return deviceId;
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
} // namespace DistributedHardware
} // namespace OHOS
