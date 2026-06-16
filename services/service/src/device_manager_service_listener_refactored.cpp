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

#include <algorithm>
#include <cstdlib>
#include <set>
#include <sstream>
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
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_bind_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_server_stub.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "datetime_ex.h"
#include "device_name_manager.h"
#include "device_manager_service.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#endif
#include "parameter.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {

// ============================================================================
// DeviceListenerUtils: Helper functions namespace
// ============================================================================

namespace DeviceListenerUtils {

std::string MakeNotifyKey(const ProcessInfo &processInfo, const std::string &deviceId)
{
    return processInfo.pkgName + "#" + std::to_string(processInfo.userId) + "#" +
        std::to_string(processInfo.tokenId) + "#" + deviceId;
}

std::string MakeNotifyPrefix(const ProcessInfo &processInfo)
{
    return processInfo.pkgName + "#" + std::to_string(processInfo.userId) + "#" +
        std::to_string(processInfo.tokenId);
}

bool StartsWith(const std::string &value, const std::string &prefix)
{
    return value.compare(0, prefix.length(), prefix) == 0;
}

ProcessInfo FindExactProcessInfo(const std::vector<ProcessInfo> &processInfos, const ProcessInfo &target)
{
    for (const auto &item : processInfos) {
        if (item == target) {
            return item;
        }
    }
    return {};
}

bool ParseNotifyKey(const std::string &notifyKey, ProcessInfo &processInfo)
{
    std::istringstream stream(notifyKey);
    std::string userId;
    std::string tokenId;
    if (!std::getline(stream, processInfo.pkgName, '#') ||
        !std::getline(stream, userId, '#') ||
        !std::getline(stream, tokenId, '#')) {
        return false;
    }
    processInfo.userId = std::stoi(userId);
    processInfo.tokenId = std::stoul(tokenId);
    return true;
}

void HandleExtraData(const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo)
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

} // namespace DeviceListenerUtils

// ============================================================================
// DmListenerStateCache: Singleton for encapsulated static members
// ============================================================================

class DmListenerStateCache {
public:
    static DmListenerStateCache& GetInstance()
    {
        static DmListenerStateCache instance;
        return instance;
    }
    
    // Online state management
    bool IsAlreadyOnline(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(onlineLock_);
        return onlineMap_.find(key) != onlineMap_.end();
    }
    
    void MarkOnline(const std::string& key, const DmDeviceInfo& info)
    {
        std::lock_guard<std::mutex> lock(onlineLock_);
        onlineMap_[key] = info;
    }
    
    void RemoveOnline(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(onlineLock_);
        if (onlineMap_.find(key) != onlineMap_.end()) {
            onlineMap_.erase(key);
        }
    }
    
    // DbReady state management
    bool IsDbReady(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(dbReadyLock_);
        return dbReadyMap_.find(key) != dbReadyMap_.end();
    }
    
    void MarkDbReady(const std::string& key, const DmDeviceInfo& info)
    {
        std::lock_guard<std::mutex> lock(dbReadyLock_);
        dbReadyMap_[key] = info;
    }
    
    void ClearDbReady(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(dbReadyLock_);
        if (dbReadyMap_.find(key) != dbReadyMap_.end()) {
            dbReadyMap_.erase(key);
        }
    }
    
    // High priority check
    bool IsHighPriority(const std::string& pkgName)
    {
        return highPriorityPkgSet_.find(pkgName) != highPriorityPkgSet_.end();
    }
    
private:
    DmListenerStateCache() = default;
    
    std::mutex onlineLock_;
    std::map<std::string, DmDeviceInfo> onlineMap_;
    
    std::mutex dbReadyLock_;
    std::map<std::string, DmDeviceInfo> dbReadyMap_;
    
    std::unordered_set<std::string> highPriorityPkgSet_ = {
        "ohos.deviceprofile",
        "ohos.distributeddata.service"
    };
};

// ============================================================================
// DeviceManagerServiceListener: Main class (refactored methods)
// ============================================================================

void DeviceManagerServiceListener::ConvertDeviceInfoToDeviceBasicInfo(const std::string &pkgName,
    const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo)
{
    (void)pkgName;
    if (memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo)) != DM_OK) {
        LOGE("memset_s failed.");
        return;
    }
    if (memcpy_s(deviceBasicInfo.deviceName, sizeof(deviceBasicInfo.deviceName), info.deviceName,
                 std::min(sizeof(deviceBasicInfo.deviceName), sizeof(info.deviceName))) != DM_OK) {
        LOGE("copy deviceName data failed.");
        return;
    }
    if (memcpy_s(deviceBasicInfo.networkId, sizeof(deviceBasicInfo.networkId), info.networkId,
                 std::min(sizeof(deviceBasicInfo.networkId), sizeof(info.networkId))) != DM_OK) {
        LOGE("copy networkId data failed.");
        return;
    }
    if (memcpy_s(deviceBasicInfo.deviceId, sizeof(deviceBasicInfo.deviceId), info.deviceId,
                 std::min(sizeof(deviceBasicInfo.deviceId), sizeof(info.deviceId))) != DM_OK) {
        LOGE("copy deviceId data failed.");
        return;
    }
    deviceBasicInfo.deviceTypeId = info.deviceTypeId;
    DeviceListenerUtils::HandleExtraData(info, deviceBasicInfo);
}

void DeviceManagerServiceListener::OnDeviceStateChange(const ProcessInfo &processInfo,
    const DmDeviceState &state, const DmDeviceInfo &info, const bool isOnline)
{
    DmDeviceBasicInfo deviceBasicInfo;
    ConvertDeviceInfoToDeviceBasicInfo(processInfo.pkgName, info, deviceBasicInfo);
    if (processInfo.pkgName == std::string(DM_PKG_NAME)) {
        ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);
    } else {
        ProcessAppStateChange(processInfo, state, info, deviceBasicInfo, isOnline);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    KVAdapterManager::GetInstance().DeleteAgedEntry();
#endif
}

void DeviceManagerServiceListener::ProcessDeviceOnline(const std::vector<ProcessInfo> &procInfoVec,
    const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
    const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("userId %{public}d, state %{public}d, udidhash %{public}s.", processInfo.userId,
        static_cast<int32_t>(state), GetAnonyString(info.deviceId).c_str());
    
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    
    for (const auto &it : procInfoVec) {
        std::string notifyPkgName = DeviceListenerUtils::MakeNotifyKey(it, std::string(info.deviceId));
        DmDeviceState notifyState = state;
        
        // Use singleton instead of static member
        if (DmListenerStateCache::GetInstance().IsAlreadyOnline(notifyPkgName)) {
            notifyState = DmDeviceState::DEVICE_INFO_CHANGED;
        } else {
            DmListenerStateCache::GetInstance().MarkOnline(notifyPkgName, info);
        }
        
        SetDeviceInfo(pReq, it, notifyState, info, deviceBasicInfo);
        ipcServerListener_.SendRequest(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
    }
}

void DeviceManagerServiceListener::OnDeviceFound(const ProcessInfo &processInfo, uint16_t subscribeId,
    const DmDeviceInfo &info)
{
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::make_shared<IpcNotifyDeviceFoundReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    DmDeviceInfo deviceInfo = info;
    
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ConvertUdidHashToAnoyAndSave(processInfo.pkgName, deviceInfo, processInfo.userId);
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

// ============================================================================
// Refactoring: Eliminate duplicate code with NotifyBindResultCommon
// ============================================================================

void DeviceManagerServiceListener::NotifyBindResultCommon(
    const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result,
    int32_t status, const std::string &content, bool hasStatus)
{
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::make_shared<IpcNotifyBindResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    
    if (hasStatus && status < STATUS_DM_AUTH_FINISH && status > STATUS_DM_AUTH_DEFAULT) {
        status = STATUS_DM_AUTH_DEFAULT;
    }
    
    PeerTargetId returnTargetId = targetId;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string deviceIdTemp = "";
    DmKVValue kvValue;
    if (ConvertUdidHashToAnoyDeviceId(processInfo.pkgName, targetId.deviceId, deviceIdTemp,
        processInfo.userId) == DM_OK && KVAdapterManager::GetInstance().Get(deviceIdTemp, kvValue) == DM_OK) {
        returnTargetId.deviceId = deviceIdTemp;
    }
#endif
    
    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetPeerTargetId(returnTargetId);
    pReq->SetResult(result);
    if (hasStatus) {
        pReq->SetStatus(status);
    }
    pReq->SetContent(content);
    pReq->SetProcessInfo(processInfo);
    
    ipcServerListener_.SendRequest(
        hasStatus ? BIND_TARGET_RESULT : UNBIND_TARGET_RESULT, pReq, pRsp);
}

void DeviceManagerServiceListener::OnBindResult(const ProcessInfo &processInfo,
    const PeerTargetId &targetId, int32_t result, int32_t status, std::string content)
{
    NotifyBindResultCommon(processInfo, targetId, result, status, content, true);
}

void DeviceManagerServiceListener::OnUnbindResult(const ProcessInfo &processInfo,
    const PeerTargetId &targetId, int32_t result, std::string content)
{
    NotifyBindResultCommon(processInfo, targetId, result, 0, content, false);
}

void DeviceManagerServiceListener::OnPinHolderCreate(const ProcessInfo &processInfo,
    const std::string &deviceId, DmPinType pinType, const std::string &payload)
{
    LOGI("%{public}s", processInfo.pkgName.c_str());
    std::shared_ptr<IpcCreatePinHolderReq> pReq = std::make_shared<IpcCreatePinHolderReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(processInfo.pkgName);
    pReq->SetDeviceId(deviceId);
    pReq->SetPinType(pinType);
    pReq->SetPayload(payload);
    pReq->SetProcessInfo(processInfo);
    ipcServerListener_.SendRequest(SERVER_CREATE_PIN_HOLDER, pReq, pRsp);
}

void DeviceManagerServiceListener::OnDeviceTrustChange(const std::string &udid,
    const std::string &uuid, DmAuthForm authForm)
{
    LOGI("udid %{public}s, authForm %{public}d, uuid %{public}s.",
        GetAnonyString(udid).c_str(), authForm, GetAnonyString(uuid).c_str());
    
    int32_t userId = -1;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    userId = MultipleUserConnector::GetFirstForegroundUserId();
#endif
    
    std::vector<ProcessInfo> processInfoVec = GetNotifyProcessInfoByUserId(userId,
        DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE);
    
    for (const auto &item : processInfoVec) {
        std::shared_ptr<IpcNotifyDevTrustChangeReq> pReq = std::make_shared<IpcNotifyDevTrustChangeReq>();
        std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
        pReq->SetPkgName(item.pkgName);
        pReq->SetUdid(udid);
        pReq->SetUuid(uuid);
        pReq->SetAuthForm(authForm);
        pReq->SetProcessInfo(item);
        ipcServerListener_.SendRequest(REMOTE_DEVICE_TRUST_CHANGE, pReq, pRsp);
    }
}

// ============================================================================
// Conditional compilation: Standard system only methods
// ============================================================================

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))

int32_t DeviceManagerServiceListener::FillUdidAndUuidToDeviceInfo(const std::string &pkgName,
    DmDeviceInfo &dmDeviceInfo)
{
    // Use singleton for high priority check
    if (!DmListenerStateCache::GetInstance().IsHighPriority(pkgName)) {
        return DM_OK;
    }
    
    std::string udid = "";
    std::string uuid = "";
    if (SoftbusCache::GetInstance().GetUdidFromCache(dmDeviceInfo.networkId, udid) != DM_OK) {
        LOGE("GetUdidFromCache fail, networkId:%{public}s", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    if (SoftbusCache::GetInstance().GetUuidFromCache(dmDeviceInfo.networkId, uuid) != DM_OK) {
        LOGE("GetUuidFromCache fail, networkId:%{public}s", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    
    std::string extraData = dmDeviceInfo.extraData;
    if (extraData.empty()) {
        LOGE("extraData is empty, networkId:%{public}s", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    
    JsonObject extraJson(extraData);
    if (extraJson.IsDiscarded()) {
        LOGE("parse extraData fail, networkId:%{public}s", GetAnonyString(dmDeviceInfo.networkId).c_str());
        return ERR_DM_FAILED;
    }
    
    extraJson[PARAM_KEY_UDID] = udid;
    extraJson[PARAM_KEY_UUID] = uuid;
    dmDeviceInfo.extraData = ToString(extraJson);
    return DM_OK;
}

int32_t DeviceManagerServiceListener::ConvertUdidHashToAnoyAndSave(const std::string &pkgName,
    DmDeviceInfo &deviceInfo, const int32_t userId)
{
    std::string appId = "";
    if (AppManager::GetInstance().GetAppIdByPkgName(pkgName, appId, userId) != DM_OK) {
        LOGD("GetAppIdByPkgName failed");
        return ERR_DM_FAILED;
    }
    
    DmKVValue kvValue;
    int32_t ret = Crypto::ConvertUdidHashToAnoyAndSave(appId, std::string(deviceInfo.deviceId), kvValue);
    if (ret != DM_OK) {
        return ERR_DM_FAILED;
    }
    
    if (memset_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN) != DM_OK) {
        LOGE("memset failed.");
        return ERR_DM_FAILED;
    }
    
    if (memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), kvValue.anoyDeviceId.c_str(),
                 std::min(sizeof(deviceInfo.deviceId), kvValue.anoyDeviceId.length())) != DM_OK) {
        LOGE("copy deviceId data failed.");
        return ERR_DM_FAILED;
    }
    
    return DM_OK;
}

// ============================================================================
// Refactoring: Eliminate service info duplicate code
// ============================================================================

int32_t DeviceManagerServiceListener::NotifyServiceStateChange(
    const DmRegisterServiceState &registerServiceState, const DmServiceInfo &serviceInfo,
    DmServiceState serviceState)
{
    std::shared_ptr<IpcNotifyServiceStateReq> pReq = std::make_shared<IpcNotifyServiceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();
    
    pReq->SetDmRegisterServiceState(registerServiceState);
    pReq->SetDmServiceInfo(serviceInfo);
    pReq->SetServiceState(serviceState);
    
    std::vector<ProcessInfo> processInfos = ipcServerListener_.GetAllProcessInfo();
    ProcessInfo targetProcessInfo { registerServiceState.userId, registerServiceState.pkgName };
    targetProcessInfo.tokenId = registerServiceState.tokenId;
    
    ProcessInfo processInfoTemp = DeviceListenerUtils::FindExactProcessInfo(processInfos, targetProcessInfo);
    if (processInfoTemp.pkgName.empty()) {
        LOGI("not register listener");
        return ERR_DM_FAILED;
    }
    
    pReq->SetPkgName(processInfoTemp.pkgName);
    pReq->SetProcessInfo(processInfoTemp);
    
    int32_t ret = ipcServerListener_.SendRequest(SERVER_SERVICE_STATE_NOTIFY, pReq, pRsp);
    if (ret != DM_OK) {
        LOGE("SERVER_SERVICE_STATE_NOTIFY request failed.");
        return ret;
    }
    
    return DM_OK;
}

int32_t DeviceManagerServiceListener::OnServiceInfoOnline(
    const DmRegisterServiceState &registerServiceState, const DmServiceInfo &serviceInfo)
{
    return NotifyServiceStateChange(registerServiceState, serviceInfo, DmServiceState::SERVICE_STATE_ONLINE);
}

int32_t DeviceManagerServiceListener::OnServiceInfoOffline(
    const DmRegisterServiceState &registerServiceState, const DmServiceInfo &serviceInfo)
{
    return NotifyServiceStateChange(registerServiceState, serviceInfo, DmServiceState::SERVICE_STATE_OFFLINE);
}

int32_t DeviceManagerServiceListener::OnServiceInfoChange(
    const DmRegisterServiceState &registerServiceState, const DmServiceInfo &serviceInfo)
{
    return NotifyServiceStateChange(registerServiceState, serviceInfo, DmServiceState::SERVICE_INFO_CHANGED);
}

#endif // !(defined(__LITEOS_M__) || defined(LITE_DEVICE))

} // namespace DistributedHardware
} // namespace OHOS
