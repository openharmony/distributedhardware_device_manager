/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <memory>
#include <vector>

#include "cj_lambda.h"
#include "ipc_skeleton.h"

#include "device_manager.h"
#include "dm_common_util.h"
#include "dm_constants.h"
#include "dm_error_message.h"
#include "dm_log.h"

namespace OHOS::DistributedHardware {

namespace {
std::map<std::string, DeviceManagerFfiImpl *> g_deviceManagerMap;
std::map<std::string, std::shared_ptr<DmFfiInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmFfiDeviceStatusCallback>> g_deviceStatusCallbackMap;
std::map<std::string, std::shared_ptr<DmFfiDiscoveryCallback>> g_DiscoveryCallbackMap;
std::map<std::string, std::shared_ptr<DmFfiPublishCallback>> g_publishCallbackMap;
std::map<std::string, std::shared_ptr<DmFfiAuthenticateCallback>> g_authCallbackMap;
std::map<std::string, std::shared_ptr<DmFfiBindTargetCallback>> g_bindCallbackMap;
std::map<std::string, std::shared_ptr<DmFfiDeviceManagerUiCallback>> g_dmUiCallbackMap;

std::mutex g_deviceManagerMapMutex;
std::mutex g_initCallbackMapMutex;
std::mutex g_deviceStatusCallbackMapMutex;
std::mutex g_discoveryCallbackMapMutex;
std::mutex g_publishCallbackMapMutex;
std::mutex g_authCallbackMapMutex;
std::mutex g_bindCallbackMapMutex;
std::mutex g_dmUiCallbackMapMutex;

const int32_t DM_AUTH_REQUEST_SUCCESS_STATUS = 7;
const int32_t DM_FFI_BUF_LENGTH = 256;

constexpr const char* DM_FFI_EVENT_DEVICE_STATE_CHANGE = "deviceStateChange";
constexpr const char* DM_FFI_EVENT_DEVICE_DISCOVER_SUCCESS = "discoverSuccess";
constexpr const char* DM_FFI_EVENT_DEVICE_DISCOVER_FAIL = "discoverFailure";
constexpr const char* DM_FFI_EVENT_DEVICE_NAME_CHANGE = "deviceNameChange";

enum ErrorCode {
    // OK
    ERR_OK = 0,
    // Permission verify failed.
    ERR_NO_PERMISSION = 201,
    // The caller is not a system application.
    ERR_NOT_SYSTEM_APP = 202,
    // Input parameter error.
    ERR_INVALID_PARAMS = 401,
    // Failed to execute the function.
    DM_ERR_FAILED = 11600101,
    // Failed to obtain the service.
    DM_ERR_OBTAIN_SERVICE = 11600102,
    // Authentication invalid.
    DM_ERR_AUTHENTICALTION_INVALID = 11600103,
    // Discovery invalid.
    DM_ERR_DISCOVERY_INVALID = 11600104,
    // Publish invalid.
    DM_ERR_PUBLISH_INVALID = 11600105,
};

inline int32_t StringCheck(const std::string &str)
{
    if (str.size() == 0 || str.size() >= DM_FFI_BUF_LENGTH) {
        return ERR_INVALID_PARAMS;
    }
    return ERR_OK;
}

int32_t TransformErrCode(const int32_t errCode)
{
    switch (errCode) {
        case ERR_DM_NO_PERMISSION:
            return ERR_NO_PERMISSION;
        case ERR_DM_DISCOVERY_REPEATED:
            return DM_ERR_DISCOVERY_INVALID;
        case ERR_DM_PUBLISH_REPEATED:
            return DM_ERR_PUBLISH_INVALID;
        case ERR_DM_AUTH_BUSINESS_BUSY:
            return DM_ERR_AUTHENTICALTION_INVALID;
        case ERR_DM_INPUT_PARA_INVALID:
        case ERR_DM_UNSUPPORTED_AUTH_TYPE:
            return ERR_INVALID_PARAMS;
        case ERR_DM_INIT_FAILED:
            return DM_ERR_OBTAIN_SERVICE;
        case ERR_NOT_SYSTEM_APP:
            return ERR_NOT_SYSTEM_APP;
        default:
            return DM_ERR_FAILED;
    }
    return 0;
}

inline void InsertIntItem(JsonObject &jsonObj, std::map<std::string, std::string> &jsonMap,
    const std::string &searchKey, const std::string &insertKey)
{
    if (IsInt32(jsonObj, searchKey)) {
        int32_t value = jsonObj[searchKey].Get<int32_t>();
        jsonMap.insert(std::pair<std::string, std::string>(insertKey, std::to_string(value)));
    }
}

inline void InsertStringItem(JsonObject &jsonObj, std::map<std::string, std::string> &jsonMap,
    const std::string &searchKey, const std::string &insertKey)
{
    if (IsString(jsonObj, searchKey)) {
        std::string value = jsonObj[searchKey].Get<std::string>();
        jsonMap.insert(std::pair<std::string, std::string>(insertKey, value));
    }
}

void InsertJsonParamesToMap(JsonObject &bindParamObj, std::map<std::string, std::string> &bindParamMap)
{
    LOGI("Insert map parames start");
    InsertIntItem(bindParamObj, bindParamMap, AUTH_TYPE, PARAM_KEY_AUTH_TYPE);
    InsertStringItem(bindParamObj, bindParamMap, APP_OPERATION, PARAM_KEY_APP_OPER);
    InsertStringItem(bindParamObj, bindParamMap, CUSTOM_DESCRIPTION, PARAM_KEY_APP_DESC);
    InsertStringItem(bindParamObj, bindParamMap, PARAM_KEY_TARGET_PKG_NAME, PARAM_KEY_TARGET_PKG_NAME);
    InsertStringItem(bindParamObj, bindParamMap, PARAM_KEY_META_TYPE, PARAM_KEY_META_TYPE);
    InsertStringItem(bindParamObj, bindParamMap, PARAM_KEY_PIN_CODE, PARAM_KEY_PIN_CODE);
    InsertStringItem(bindParamObj, bindParamMap, PARAM_KEY_AUTH_TOKEN, PARAM_KEY_AUTH_TOKEN);
    InsertIntItem(bindParamObj, bindParamMap, BIND_LEVEL, BIND_LEVEL);
}
} // namespace

DeviceManagerFfiImpl::DeviceManagerFfiImpl(const std::string &bundleName, int32_t *errCode) : bundleName_(bundleName)
{
    *errCode = StringCheck(bundleName);
    if (*errCode != 0) {
        LOGE("CreateDeviceManager for bundleName %{public}s failed, ret %{public}d.", bundleName_.c_str(), *errCode);
        return;
    }
    std::shared_ptr<DmFfiInitCallback> initCallback = std::make_shared<DmFfiInitCallback>(bundleName_);
    *errCode = DeviceManager::GetInstance().InitDeviceManager(bundleName_, initCallback);
    if (*errCode != 0) {
        *errCode = TransformErrCode(*errCode);
        LOGE("CreateDeviceManager for bundleName %{public}s failed, ret %{public}d.", bundleName_.c_str(), *errCode);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        CHECK_SIZE_VOID(g_initCallbackMap);
        g_initCallbackMap[bundleName_] = initCallback;
    }

    std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
    CHECK_SIZE_VOID(g_deviceManagerMap);
    g_deviceManagerMap[bundleName_] = this;
}

int32_t DeviceManagerFfiImpl::ReleaseDeviceManager()
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }
    int ret = DeviceManager::GetInstance().UnInitDeviceManager(bundleName_);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("ReleaseDeviceManager for bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        return ret;
    }
    ClearBundleCallbacks();
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::GetAvailableDeviceList(FfiDeviceBasicInfoArray &deviceInfoList)
{
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        return TransformErrCode(ret);
    }
    std::vector<DmDeviceBasicInfo> result;
    ret = DeviceManager::GetInstance().GetAvailableDeviceList(bundleName_, result);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("GetTrustedDeviceList for bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        return ret;
    }

    if (result.size() == 0) {
        return ERR_OK;
    }

    deviceInfoList.head = static_cast<FfiDeviceBasicInfo *>(malloc(sizeof(FfiDeviceBasicInfo) * result.size()));
    if (deviceInfoList.head == nullptr) {
        LOGE("Malloc failed");
        return DM_ERR_FAILED;
    }
    deviceInfoList.size = static_cast<int64_t>(result.size());
    for (decltype(result.size()) i = 0; i < result.size(); ++i) {
        ret = Transform2FfiDeviceBasicInfo(result[i], deviceInfoList.head[i]);
        if (ret != 0) {
            DeviceListFree(deviceInfoList, i);
            return ret;
        }
    }
    return ERR_OK;
}

void DeviceManagerFfiImpl::DeviceListFree(FfiDeviceBasicInfoArray &deviceInfoList, int64_t size)
{
    if (size == -1) {
        size = deviceInfoList.size;
    }
    for (int32_t i = 0; i < size; ++i) {
        FreeDeviceInfo(deviceInfoList.head[i]);
    }
    free(deviceInfoList.head);
    deviceInfoList.head = nullptr;
    deviceInfoList.size = 0;
}

int32_t DeviceManagerFfiImpl::Transform2FfiDeviceBasicInfo(const DmDeviceBasicInfo &in, FfiDeviceBasicInfo &out)
{
    out.deviceId = MallocCStr(in.deviceId);
    out.deviceName = MallocCStr(in.deviceName);
    out.deviceType = in.deviceTypeId;
    out.networkId = MallocCStr(in.networkId);
    if (out.deviceId == nullptr || out.deviceName == nullptr || out.networkId == nullptr) {
        FreeDeviceInfo(out);
        return DM_ERR_FAILED;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::GetLocalDeviceNetworkId(const char *&networkId)
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }

    std::string result;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceNetWorkId(bundleName_, result);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("GetLocalDeviceNetworkId for failed, ret %{public}d", ret);
        return ret;
    }
    LOGI("DeviceManager::GetLocalDeviceNetworkId networkId:%{public}s", GetAnonyString(result).c_str());

    networkId = MallocCStr(result.c_str());
    if (networkId == nullptr) {
        return DM_ERR_FAILED;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::GetLocalDeviceName(const char *&deviceName)
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }

    std::string result;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceName(bundleName_, result);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("GetLocalDeviceName for failed, ret %{public}d", ret);
        return ret;
    }
    LOGI("DeviceManager::GetLocalDeviceName deviceName:%{public}s", GetAnonyString(result).c_str());

    deviceName = MallocCStr(result.c_str());
    if (deviceName == nullptr) {
        return DM_ERR_FAILED;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::GetLocalDeviceType(int32_t &deviceType)
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }

    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceType(bundleName_, deviceType);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("GetLocalDeviceType for failed, ret %{public}d", ret);
        return ret;
    }
    LOGI("DeviceManager::GetLocalDeviceType deviceType:%{public}d", deviceType);
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::GetLocalDeviceId(const char *&deviceId)
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }

    std::string result;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceId(bundleName_, result);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("GetLocalDeviceId for failed, ret %{public}d", ret);
        return ret;
    }
    LOGI("DeviceManager::GetLocalDeviceId deviceId:%{public}s", GetAnonyString(result).c_str());

    deviceId = MallocCStr(result.c_str());
    if (deviceId == nullptr) {
        return DM_ERR_FAILED;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::GetDeviceName(const std::string &networkId, const char *&deviceName)
{
    int32_t ret = StringCheck(networkId);
    if (ret != 0) {
        return ret;
    }
    std::string result;
    ret = DeviceManager::GetInstance().GetDeviceName(bundleName_, networkId, result);
    LOGI("DeviceManager::GetDeviceName getinstance return.");
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("GetDeviceName for failed, ret %{public}d", ret);
        return ret;
    }
    LOGI("DeviceManager::GetDeviceName deviceName:%{public}s", GetAnonyString(result).c_str());

    deviceName = MallocCStr(result.c_str());
    if (deviceName == nullptr) {
        return DM_ERR_FAILED;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::GetDeviceType(const std::string &networkId, int32_t &deviceType)
{
    int32_t ret = StringCheck(networkId);
    if (ret != 0) {
        return ret;
    }
    ret = DeviceManager::GetInstance().GetDeviceType(bundleName_, networkId, deviceType);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("GetDeviceType for failed, ret %{public}d", ret);
        return ret;
    }
    LOGI("DeviceManager::GetDeviceType deviceType:%{public}d", deviceType);
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::StartDiscovering(const std::string &extra)
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }
    std::shared_ptr<DmFfiDiscoveryCallback> discoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        auto iter = g_DiscoveryCallbackMap.find(bundleName_);
        if (iter == g_DiscoveryCallbackMap.end()) {
            CHECK_SIZE_RETURN(g_DiscoveryCallbackMap, DM_ERR_FAILED);
            discoveryCallback = std::make_shared<DmFfiDiscoveryCallback>(bundleName_);
            g_DiscoveryCallbackMap[bundleName_] = discoveryCallback;
        } else {
            discoveryCallback = iter->second;
        }
    }
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(bundleName_, tokenId, extra, discoveryCallback);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("Discovery failed, bundleName %{public}s, ret %{public}d", bundleName_.c_str(), ret);
        discoveryCallback->OnDiscoveryFailed(static_cast<uint16_t>(0), ret);
        return ret;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::StopDiscovering()
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(tokenId, bundleName_);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("StopDeviceDiscovery for bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        return ret;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::BindDevice(const std::string &deviceId, const std::string &bindParam)
{
    std::shared_ptr<DmFfiAuthenticateCallback> bindDeviceCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
        auto iter = g_authCallbackMap.find(bundleName_);
        if (iter == g_authCallbackMap.end()) {
            CHECK_SIZE_RETURN(g_authCallbackMap, DM_ERR_FAILED);
            bindDeviceCallback = std::make_shared<DmFfiAuthenticateCallback>(bundleName_);
            g_authCallbackMap[bundleName_] = bindDeviceCallback;
        } else {
            bindDeviceCallback = iter->second;
        }
    }
    constexpr int32_t bindType = 1;
    int32_t ret = DeviceManager::GetInstance().BindDevice(bundleName_, bindType, deviceId,
        bindParam, bindDeviceCallback);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("BindDevice for bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        return ret;
    }
    return WaitForCallbackCv();
}

int32_t DeviceManagerFfiImpl::BindTarget(const std::string &deviceId,
    const std::string &bindParam, const bool isMetaType)
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }
    int32_t ret = StringCheck(deviceId);
    if (ret != 0) {
        return ret;
    }

    callbackFinished = false;
    if (isMetaType) {
        std::shared_ptr<DmFfiBindTargetCallback> bindTargetCallback = nullptr;
        {
            std::lock_guard<std::mutex> autoLock(g_bindCallbackMapMutex);
            auto iter = g_bindCallbackMap.find(bundleName_);
            if (iter == g_bindCallbackMap.end()) {
                CHECK_SIZE_RETURN(g_bindCallbackMap, DM_ERR_FAILED);
                bindTargetCallback = std::make_shared<DmFfiBindTargetCallback>(bundleName_);
                g_bindCallbackMap[bundleName_] = bindTargetCallback;
            } else {
                bindTargetCallback = iter->second;
            }
        }
        int32_t ret = BindTargetWarpper(deviceId, bindParam, bindTargetCallback);
        if (ret != 0) {
            ret = TransformErrCode(ret);
            LOGE("BindTarget for bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
            return ret;
        }
        return WaitForCallbackCv();
    }

    return BindDevice(deviceId, bindParam);
}

int32_t DeviceManagerFfiImpl::WaitForCallbackCv()
{
    std::unique_lock<std::mutex> autoLock(callbackFinishedMutex);
    callbackFinishedCv.wait(autoLock, [this] { return this->callbackFinished; });
    LOGI("WaitForCallbackCv got notified, errCode is %{public}d", errCode_.load());
    return errCode_.load();
}

int32_t DeviceManagerFfiImpl::UnbindTarget(const std::string &deviceId)
{
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        return ERR_NO_PERMISSION;
    }
    int32_t ret = StringCheck(deviceId);
    if (ret != 0) {
        return ret;
    }
    LOGI("UnBindDevice deviceId = %{public}s", GetAnonyString(deviceId).c_str());
    ret = DeviceManager::GetInstance().UnBindDevice(bundleName_, deviceId);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("UnBindDevice for bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        return ret;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::EventOn(const std::string &type, void *callback)
{
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        return TransformErrCode(ret);
    }

    LOGI("EventOn for bundleName %{public}s, eventType %{public}s ", bundleName_.c_str(), type.c_str());
    RegisterCallbackByType(type, callback);

    if (type == DM_FFI_EVENT_DEVICE_STATE_CHANGE || type == DM_FFI_EVENT_DEVICE_NAME_CHANGE) {
        return RegisterDevStatusCallback();
    } else if (type == DM_FFI_EVENT_DEVICE_DISCOVER_SUCCESS || type == DM_FFI_EVENT_DEVICE_DISCOVER_FAIL) {
        return RegisterDiscoveryCallback();
    }

    return ERR_INVALID_PARAMS;
}

int32_t DeviceManagerFfiImpl::EventOff(const std::string &type)
{
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        return ret;
    }

    LOGI("EventOff for bundleName %{public}s, eventType %{public}s ", bundleName_.c_str(), type.c_str());
    Off(type);
    if (type == DM_FFI_EVENT_DEVICE_STATE_CHANGE || type == DM_FFI_EVENT_DEVICE_NAME_CHANGE) {
        if (!deviceStateChangedCallback && !deviceNameChangedCallback) {
            return ReleaseDevStatusCallback();
        }
        return ERR_OK;
    } else if (type == DM_FFI_EVENT_DEVICE_DISCOVER_SUCCESS || type == DM_FFI_EVENT_DEVICE_DISCOVER_FAIL) {
        return ReleaseDiscoveryCallback();
    }
    return ERR_INVALID_PARAMS;
}


void DeviceManagerFfiImpl::OnDeviceStatusChange(int32_t action, const DmDeviceBasicInfo &deviceBasicInfo)
{
    std::lock_guard<std::mutex> autoLock(callbackLock);
    if (deviceStateChangedCallback) {
        auto ptr = static_cast<FfiDeviceBasicInfo *>(malloc(sizeof(FfiDeviceBasicInfo)));
        if (ptr == nullptr) {
            LOGE("OnDeviceStatusChange malloc FfiDeviceBasicInfo failed.");
            return;
        }
        int32_t ret = Transform2FfiDeviceBasicInfo(deviceBasicInfo, *ptr);
        if (ret != 0) {
            LOGE("OnDeviceStatusChange failed to transform DmDeviceBasicInfo.");
            free(ptr);
            ptr = nullptr;
            return;
        }
        deviceStateChangedCallback(action, ptr);
        FreeDeviceInfo(*ptr);
        free(ptr);
        ptr = nullptr;
    }
}

void DeviceManagerFfiImpl::OnDeviceNameChange(const std::string &deviceName)
{
    std::lock_guard<std::mutex> autoLock(callbackLock);
    if (deviceNameChangedCallback) {
        char *cDeviceName = MallocCStr(deviceName.c_str());
        if (cDeviceName == nullptr) {
            LOGE("OnDeviceNameChange malloc deviname failed.");
            return;
        }
        deviceNameChangedCallback(cDeviceName);
        free(cDeviceName);
        cDeviceName = nullptr;
    }
}

void DeviceManagerFfiImpl::OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo)
{
    std::lock_guard<std::mutex> autoLock(callbackLock);
    if (discoverSuccessCallback) {
        auto ptr = static_cast<FfiDeviceBasicInfo *>(malloc(sizeof(FfiDeviceBasicInfo)));
        if (ptr == nullptr) {
            LOGE("OnDeviceStatusChange malloc FfiDeviceBasicInfo failed.");
            return;
        }
        int32_t ret = Transform2FfiDeviceBasicInfo(deviceBasicInfo, *ptr);
        if (ret != 0) {
            LOGE("OnDeviceStatusChange failed to transform DmDeviceBasicInfo.");
            free(ptr);
            ptr = nullptr;
            return;
        }
        discoverSuccessCallback(ptr);
        FreeDeviceInfo(*ptr);
        free(ptr);
        ptr = nullptr;
    }
}

void DeviceManagerFfiImpl::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    std::lock_guard<std::mutex> autoLock(callbackLock);
    LOGI("OnDiscoveryFailed for subscribeId %{public}d", (int32_t)subscribeId);
    if (deviceDiscoverFailedCallback) {
        deviceDiscoverFailedCallback(failedReason);
    }
}

void DeviceManagerFfiImpl::OnPublishResult(int32_t publishId, int32_t publishResult)
{
    LOGI("OnPublishResult for publishId %{public}d, publishResult %{public}d", publishId, publishResult);
}

void DeviceManagerFfiImpl::OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
    int32_t reason)
{
    LOGI("OnAuthResult for status: %{public}d, reason: %{public}d", status, reason);
    if (reason == DM_OK && (status <= STATUS_DM_CLOSE_PIN_INPUT_UI && status >= STATUS_DM_SHOW_AUTHORIZE_UI)) {
        LOGI("update ui change, status: %{public}d, reason: %{public}d", status, reason);
        return;
    }

    if (status == DM_AUTH_REQUEST_SUCCESS_STATUS && reason == 0) {
        LOGI("OnAuthResult success");
        errCode_ = ERR_OK;
    } else {
        LOGI("OnAuthResult failed");
        errCode_ = reason;
    }

    std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
    g_authCallbackMap.erase(bundleName_);
    callbackFinished = true;
    callbackFinishedCv.notify_one();
}

void DeviceManagerFfiImpl::OnDmUiCall(const std::string &paramJson)
{
    LOGI("OnCall for paramJson");
}

DeviceManagerFfiImpl *DeviceManagerFfiImpl::GetDeviceManagerFfi(std::string &bundleName)
{
    std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
    auto iter = g_deviceManagerMap.find(bundleName);
    if (iter == g_deviceManagerMap.end()) {
        return nullptr;
    }
    return iter->second;
}

void DeviceManagerFfiImpl::ClearBundleCallbacks()
{
    LOGI("ClearBundleCallbacks start for bundleName %{public}s", bundleName_.c_str());
    {
        std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
        g_deviceManagerMap.erase(bundleName_);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        g_initCallbackMap.erase(bundleName_);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        g_deviceStatusCallbackMap.erase(bundleName_);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        g_DiscoveryCallbackMap.erase(bundleName_);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        g_publishCallbackMap.erase(bundleName_);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
        g_authCallbackMap.erase(bundleName_);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_bindCallbackMapMutex);
        g_bindCallbackMap.erase(bundleName_);
    }
    return;
}

int32_t DeviceManagerFfiImpl::BindTargetWarpper(const std::string &deviceId,
    const std::string &bindParam, std::shared_ptr<DmFfiBindTargetCallback> callback)
{
    if (bindParam.empty()) {
        return ERR_INVALID_PARAMS;
    }
    JsonObject bindParamObj(bindParam);
    if (bindParamObj.IsDiscarded()) {
        return ERR_INVALID_PARAMS;
    }
    PeerTargetId targetId;
    targetId.deviceId = deviceId;
    if (IsString(bindParamObj, PARAM_KEY_BR_MAC)) {
        targetId.brMac = bindParamObj[PARAM_KEY_BR_MAC].Get<std::string>();
    }
    if (IsString(bindParamObj, PARAM_KEY_BLE_MAC)) {
        targetId.bleMac = bindParamObj[PARAM_KEY_BLE_MAC].Get<std::string>();
    }
    if (IsString(bindParamObj, PARAM_KEY_WIFI_IP)) {
        targetId.wifiIp = bindParamObj[PARAM_KEY_WIFI_IP].Get<std::string>();
    }
    if (IsInt32(bindParamObj, PARAM_KEY_WIFI_PORT)) {
        targetId.wifiPort = (uint16_t)(bindParamObj[PARAM_KEY_WIFI_PORT].Get<int32_t>());
    }

    std::map<std::string, std::string> bindParamMap;
    InsertJsonParamesToMap(bindParamObj, bindParamMap);
    return DeviceManager::GetInstance().BindTarget(bundleName_, targetId, bindParamMap, callback);
}


int32_t DeviceManagerFfiImpl::RegisterDevStatusCallback()
{
    LOGI("RegisterDevStatusCallback start for bundleName %{public}s", bundleName_.c_str());
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        if (g_deviceStatusCallbackMap.find(bundleName_) != g_deviceStatusCallbackMap.end()) {
            LOGI("bundleName already register.");
            return ERR_OK;
        }
    }
    auto callback = std::make_shared<DmFfiDeviceStatusCallback>(bundleName_);
    int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName_, "", callback);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("RegisterDevStatusCallback failed for bundleName %{public}s", bundleName_.c_str());
        return ret;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        CHECK_SIZE_RETURN(g_deviceStatusCallbackMap, DM_ERR_FAILED);
        g_deviceStatusCallbackMap[bundleName_] = callback;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::RegisterDiscoveryCallback()
{
    auto discoveryCallback = std::make_shared<DmFfiDiscoveryCallback>(bundleName_);
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        CHECK_SIZE_RETURN(g_DiscoveryCallbackMap, DM_ERR_FAILED);
        g_DiscoveryCallbackMap[bundleName_] = discoveryCallback;
    }
    discoveryCallback->IncreaseRefCount();
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::RegisterPublishCallback()
{
    auto publishCallback = std::make_shared<DmFfiPublishCallback>(bundleName_);
    {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        CHECK_SIZE_RETURN(g_publishCallbackMap, DM_ERR_FAILED);
        g_publishCallbackMap[bundleName_] = publishCallback;
    }
    publishCallback->IncreaseRefCount();
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::RegisterReplyCallback()
{
    auto dmUiCallback = std::make_shared<DmFfiDeviceManagerUiCallback>(bundleName_);
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(bundleName_, dmUiCallback);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("RegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName_.c_str());
        return ret;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        CHECK_SIZE_RETURN(g_dmUiCallbackMap, DM_ERR_FAILED);
        g_dmUiCallbackMap[bundleName_] = dmUiCallback;
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::ReleaseDevStatusCallback()
{
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        auto iter = g_deviceStatusCallbackMap.find(bundleName_);
        if (iter == g_deviceStatusCallbackMap.end()) {
            LOGE("ReleaseDmCallback: cannot find statusCallback for bundleName %{public}s", bundleName_.c_str());
            return ERR_INVALID_PARAMS;
        }
    }
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName_);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("UnRegisterDevStatusCallback failed for bundleName %{public}s", bundleName_.c_str());
        return ret;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        g_deviceStatusCallbackMap.erase(bundleName_);
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::ReleaseDiscoveryCallback()
{
    LOGI("ReleaseDiscoveryCallback for bundleName %{public}s", bundleName_.c_str());
    std::shared_ptr<DmFfiDiscoveryCallback> DiscoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        auto iter = g_DiscoveryCallbackMap.find(bundleName_);
        if (iter == g_DiscoveryCallbackMap.end()) {
            return ERR_OK;
        }
        DiscoveryCallback = iter->second;
    }
    DiscoveryCallback->DecreaseRefCount();
    if (DiscoveryCallback->GetRefCount() == 0) {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        g_DiscoveryCallbackMap.erase(bundleName_);
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::ReleasePublishCallback()
{
    LOGI("ReleasePublishCallback for bundleName %{public}s", bundleName_.c_str());
    std::shared_ptr<DmFfiPublishCallback> publishCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        auto iter = g_publishCallbackMap.find(bundleName_);
        if (iter == g_publishCallbackMap.end()) {
            return ERR_OK;
        }
        publishCallback = iter->second;
    }
    publishCallback->DecreaseRefCount();
    if (publishCallback->GetRefCount() == 0) {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        g_publishCallbackMap.erase(bundleName_);
    }
    return ERR_OK;
}

int32_t DeviceManagerFfiImpl::ReleaseReplyCallback()
{
    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        auto iter = g_dmUiCallbackMap.find(bundleName_);
        if (iter == g_dmUiCallbackMap.end()) {
            LOGE("cannot find dmFaCallback for bundleName %{public}s", bundleName_.c_str());
            return ERR_INVALID_PARAMS;
        }
    }
    int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(bundleName_);
    if (ret != 0) {
        ret = TransformErrCode(ret);
        LOGE("UnRegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName_.c_str());
        return ret;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        g_dmUiCallbackMap.erase(bundleName_);
    }
    return ERR_OK;
}


void DeviceManagerFfiImpl::RegisterCallbackByType(const std::string &type, void *callback)
{
    std::lock_guard<std::mutex> autoLock(callbackLock);
    if (type == DM_FFI_EVENT_DEVICE_STATE_CHANGE) {
        deviceStateChangedCallback = CJLambda::Create(
            reinterpret_cast<void (*)(int32_t, FfiDeviceBasicInfo *)>(callback));
    } else if (type == DM_FFI_EVENT_DEVICE_DISCOVER_SUCCESS) {
        discoverSuccessCallback = CJLambda::Create(reinterpret_cast<void (*)(FfiDeviceBasicInfo *)>(callback));
    } else if (type == DM_FFI_EVENT_DEVICE_NAME_CHANGE) {
        deviceNameChangedCallback = CJLambda::Create(reinterpret_cast<void (*)(const char *)>(callback));
    } else if (type == DM_FFI_EVENT_DEVICE_DISCOVER_FAIL) {
        deviceDiscoverFailedCallback = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
    } else {
        LOGE("RegisterCallbackByType call with wrong type.");
    }
}

void DeviceManagerFfiImpl::Off(const std::string &type)
{
    std::lock_guard<std::mutex> autoLock(callbackLock);
    if (type == DM_FFI_EVENT_DEVICE_STATE_CHANGE) {
        deviceStateChangedCallback = nullptr;
    } else if (type == DM_FFI_EVENT_DEVICE_DISCOVER_SUCCESS) {
        discoverSuccessCallback = nullptr;
    } else if (type == DM_FFI_EVENT_DEVICE_NAME_CHANGE) {
        deviceNameChangedCallback = nullptr;
    } else if (type == DM_FFI_EVENT_DEVICE_DISCOVER_FAIL) {
        deviceDiscoverFailedCallback = nullptr;
    } else {
        LOGE("Off call with wrong type.");
    }
}
} // namespace OHOS::DistributedHardware
