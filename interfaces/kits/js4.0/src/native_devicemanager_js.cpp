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

#include "native_devicemanager_js.h"

#include <securec.h>
#include <uv.h>
#include <map>
#include <mutex>

#include "device_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_device_profile_info.h"
#include "dm_log.h"
#include "dm_native_util.h"
#include "ipc_skeleton.h"
#include "js_native_api.h"
#include "tokenid_kit.h"
#include "json_object.h"

using namespace OHOS::DistributedHardware;

namespace {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr))

const std::string DM_NAPI_EVENT_DEVICE_STATE_CHANGE = "deviceStateChange";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVER_SUCCESS = "discoverSuccess";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL = "discoverFail";
const std::string DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS = "publishSuccess";
const std::string DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL = "publishFail";
const std::string DM_NAPI_EVENT_DEVICE_SERVICE_DIE = "serviceDie";
const std::string DEVICE_MANAGER_NAPI_CLASS_NAME = "DeviceManager";
const std::string DM_NAPI_EVENT_REPLY_RESULT = "replyResult";
const std::string DM_NAPI_EVENT_DEVICE_NAME_CHANGE = "deviceNameChange";

const int32_t DM_NAPI_ARGS_ZERO = 0;
const int32_t DM_NAPI_ARGS_ONE = 1;
const int32_t DM_NAPI_ARGS_TWO = 2;
const int32_t DM_NAPI_ARGS_THREE = 3;
const int32_t DM_AUTH_REQUEST_SUCCESS_STATUS = 7;
const int32_t DM_MAX_DEVICE_SIZE = 100;

napi_ref deviceStateChangeActionEnumConstructor_ = nullptr;
napi_ref g_strategyForHeartbeatEnumConstructor = nullptr;

std::map<std::string, DeviceManagerNapi *> g_deviceManagerMap;
std::map<std::string, std::shared_ptr<DmNapiInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceStatusCallback>> g_deviceStatusCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDiscoveryCallback>> g_DiscoveryCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiPublishCallback>> g_publishCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiAuthenticateCallback>> g_authCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiBindTargetCallback>> g_bindCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceManagerUiCallback>> g_dmUiCallbackMap;

std::mutex g_deviceManagerMapMutex;
std::mutex g_initCallbackMapMutex;
std::mutex g_deviceStatusCallbackMapMutex;
std::mutex g_discoveryCallbackMapMutex;
std::mutex g_publishCallbackMapMutex;
std::mutex g_authCallbackMapMutex;
std::mutex g_bindCallbackMapMutex;
std::mutex g_dmUiCallbackMapMutex;

void DeleteUvWork(uv_work_t *work)
{
    if (work == nullptr) {
        return;
    }
    delete work;
    work = nullptr;
    LOGI("delete work!");
}

void DeleteDmNapiStatusJsCallbackPtr(DmNapiStatusJsCallback *pJsCallbackPtr)
{
    if (pJsCallbackPtr == nullptr) {
        return;
    }
    delete pJsCallbackPtr;
    pJsCallbackPtr = nullptr;
    LOGI("delete DmNapiStatusJsCallback callbackPtr!");
}

void DeleteAsyncCallbackInfo(DeviceBasicInfoListAsyncCallbackInfo *pAsynCallbackInfo)
{
    if (pAsynCallbackInfo == nullptr) {
        return;
    }
    delete pAsynCallbackInfo;
    pAsynCallbackInfo = nullptr;
}

bool IsDeviceManagerNapiNull(napi_env env, napi_value thisVar, DeviceManagerNapi **pDeviceManagerWrapper)
{
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(pDeviceManagerWrapper));
    if (pDeviceManagerWrapper != nullptr && *pDeviceManagerWrapper != nullptr) {
        return false;
    }
    CreateBusinessError(env, ERR_DM_POINT_NULL);
    LOGE("DeviceManagerNapi object is nullptr!");
    return true;
}
} // namespace

thread_local napi_ref DeviceManagerNapi::sConstructor_ = nullptr;
AuthAsyncCallbackInfo DeviceManagerNapi::authAsyncCallbackInfo_;

void DmNapiInitCallback::OnRemoteDied()
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiInitCallback: OnRemoteDied, No memory");
        return;
    }

    DmDeviceBasicInfo info;
    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, 0, 0, info);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnRemoteDied uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnRemoteDied, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnEvent("serviceDie", 0, nullptr);
        }
        LOGI("OnRemoteDied, deviceManagerNapi bundleName %{public}s", callback->bundleName_.c_str());
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnRemoteDied work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStatusCallback::OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceStatusCallback: OnDeviceOnline, No memory");
        return;
    }

    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, 0, 0, deviceBasicInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceOnline uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceOnline, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::UNKNOWN, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceOnline work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStatusCallback::OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceStateCallback: OnDeviceReady, No memory");
        return;
    }

    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, 0, 0, deviceBasicInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceReady uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceReady, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::AVAILABLE, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceReady work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStatusCallback::OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceStatusCallback: OnDeviceOffline, No memory");
        return;
    }

    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, 0, 0, deviceBasicInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceOffline uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceOffline, deviceManagerNapi not find for bundleName %{public}s",
                callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::UNAVAILABLE, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceOffline work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStatusCallback::OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceStatusCallback: OnDeviceChanged, No memory");
        return;
    }

    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, 0, 0, deviceBasicInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceChanged uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceChanged, deviceManagerNapi not find for bundleName %{public}s",
                callback->bundleName_.c_str());
        } else {
            std::string deviceName = callback->deviceBasicInfo_.deviceName;
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::CHANGE, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceChanged work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDiscoveryCallback::OnDeviceFound(uint16_t subscribeId,
                                            const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFound for %{public}s, subscribeId %{public}d", bundleName_.c_str(), (int32_t)subscribeId);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDiscoveryCallback: OnDeviceFound, No memory");
        return;
    }

    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, subscribeId, 0, deviceBasicInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceFound uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceFound, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceFound(callback->subscribeId_, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceFound work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDiscoveryCallback::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailed for %{public}s, subscribeId %{public}d", bundleName_.c_str(), (int32_t)subscribeId);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDiscoveryCallback: OnDiscoveryFailed, No memory");
        return;
    }

    DmDeviceBasicInfo info;
    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, subscribeId,
        failedReason, info);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDiscoveryFailed uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDiscoveryFailed, deviceManagerNapi not find for bundleName %{public}s",
                callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDiscoveryFailed(callback->subscribeId_, callback->reason_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDiscoveryFailed work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDiscoveryCallback::OnDiscoverySuccess(uint16_t subscribeId)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        LOGE("OnDiscoverySuccess, deviceManagerNapi not find for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    LOGI("DiscoverySuccess for %{public}s, subscribeId %{public}d", bundleName_.c_str(), (int32_t)subscribeId);
}

void DmNapiDiscoveryCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmNapiDiscoveryCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmNapiDiscoveryCallback::GetRefCount()
{
    return refCount_;
}

void DmNapiPublishCallback::OnPublishResult(int32_t publishId, int32_t publishResult)
{
    LOGI("OnPublishResult for %{public}s, publishId %{public}d, publishResult %{public}d", bundleName_.c_str(),
        publishId, publishResult);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiPublishCallback: OnPublishResult, No memory");
        return;
    }

    DmNapiPublishJsCallback *jsCallback = new DmNapiPublishJsCallback(bundleName_, publishId, publishResult);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnPublishResult uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiPublishJsCallback *callback = reinterpret_cast<DmNapiPublishJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnPublishResult, deviceManagerNapi failed for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnPublishResult(callback->publishId_, callback->reason_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnPublishResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}

void DmNapiPublishCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmNapiPublishCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmNapiPublishCallback::GetRefCount()
{
    return refCount_;
}

void DmNapiAuthenticateCallback::OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
                                              int32_t reason)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("js4.0 DmNapiAuthenticateCallback::OnAuthResult, No memory");
        return;
    }

    DmNapiAuthJsCallback *jsCallback = new DmNapiAuthJsCallback(bundleName_, deviceId, token, status, reason);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnAuthResult uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiAuthJsCallback *callback = reinterpret_cast<DmNapiAuthJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnAuthResult, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnAuthResult(callback->deviceId_, callback->token_,
                callback->status_, callback->reason_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnAuthResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}

void DmNapiGetDeviceProfileInfoListCallback::OnResult(const std::vector<DmDeviceProfileInfo> &deviceProfileInfos,
    int32_t code)
{
    LOGI("In code:%{public}d, size:%{public}zu", code, deviceProfileInfos.size());
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        LOGE("get loop fail");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("OnResult, No memory");
        return;
    }
    auto *jsCallback = new DeviceProfileInfosAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("create jsCallback fail");
        DeleteUvWork(work);
        return;
    }
    jsCallback->env = env_;
    jsCallback->bundleName = bundleName_;
    jsCallback->deferred = deferred_;
    jsCallback->deviceProfileInfos = deviceProfileInfos;
    jsCallback->code = code;
    work->data = reinterpret_cast<void *>(jsCallback);
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
            LOGD("OnResult uv_queue_work_with_qos");
    },  [] (uv_work_t *work, int status) {
        DeviceProfileInfosAsyncCallbackInfo *callback =
            reinterpret_cast<DeviceProfileInfosAsyncCallbackInfo *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName);
        if (deviceManagerNapi == nullptr) {
            LOGE("deviceManagerNapi not find for bundleName %{public}s", callback->bundleName.c_str());
        } else {
            deviceManagerNapi->OnGetDeviceProfileInfoListCallbackResult(callback);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnBindResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}

DeviceManagerNapi::DeviceManagerNapi(napi_env env, napi_value thisVar) : DmNativeEvent(env, thisVar)
{
    env_ = env;
}

DeviceManagerNapi::~DeviceManagerNapi()
{
}

DeviceManagerNapi *DeviceManagerNapi::GetDeviceManagerNapi(std::string &bundleName)
{
    std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
    auto iter = g_deviceManagerMap.find(bundleName);
    if (iter == g_deviceManagerMap.end()) {
        return nullptr;
    }
    return iter->second;
}

void DeviceManagerNapi::OnDeviceStatusChange(DmNapiDevStatusChange action,
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "action", (int)action, result);

    napi_value device = nullptr;
    napi_create_object(env_, &device);
    DmDeviceBasicToJsObject(env_, deviceBasicInfo, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("deviceStateChange", DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFound DmDeviceBasicInfo for subscribeId %{public}d", (int32_t)subscribeId);
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value result = nullptr;
    napi_create_object(env_, &result);

    napi_value device = nullptr;
    napi_create_object(env_, &device);
    DmDeviceBasicToJsObject(env_, deviceBasicInfo, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("discoverSuccess", DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailed for subscribeId %{public}d", (int32_t)subscribeId);
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "reason", (int)failedReason, result);
    std::string errCodeInfo = OHOS::DistributedHardware::GetErrorString((int)failedReason);
    SetValueUtf8String(env_, "errInfo", errCodeInfo, result);
    OnEvent("discoverFail", DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnPublishResult(int32_t publishId, int32_t publishResult)
{
    LOGI("OnPublishResult for publishId %{public}d, publishResult %{public}d", publishId, publishResult);
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "publishId", publishId, result);
    if (publishResult == 0) {
        OnEvent("publishSuccess", DM_NAPI_ARGS_ONE, &result);
    } else {
        SetValueInt32(env_, "reason", publishResult, result);
        std::string errCodeInfo = OHOS::DistributedHardware::GetErrorString(publishResult);
        SetValueUtf8String(env_, "errInfo", errCodeInfo, result);
        OnEvent("publishFail", DM_NAPI_ARGS_ONE, &result);
    }
    NAPI_CALL_RETURN_VOID(env_, napi_close_handle_scope(env_, scope));
}

void DeviceManagerNapi::OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
                                     int32_t reason)
{
    LOGI("OnAuthResult for status: %{public}d, reason: %{public}d", status, reason);
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value thisVar = nullptr;
    napi_get_reference_value(env_, thisVarRef_, &thisVar);
    napi_value result[DM_NAPI_ARGS_TWO] = {0};

    if (status == DM_AUTH_REQUEST_SUCCESS_STATUS && reason == 0) {
        LOGI("OnAuthResult success");
        napi_get_undefined(env_, &result[0]);
        napi_create_object(env_, &result[1]);
        SetValueUtf8String(env_, "deviceId", deviceId, result[1]);
    } else {
        LOGI("OnAuthResult failed");
        napi_create_object(env_, &result[0]);
        SetValueInt32(env_, "code", status, result[0]);
        SetValueInt32(env_, "reason", reason, result[0]);
        std::string errCodeInfo = OHOS::DistributedHardware::GetErrorString((int)reason);
        SetValueUtf8String(env_, "errInfo", errCodeInfo, result[0]);
        napi_get_undefined(env_, &result[1]);
    }

    if (reason == DM_OK && (status <= STATUS_DM_CLOSE_PIN_INPUT_UI && status >= STATUS_DM_SHOW_AUTHORIZE_UI)) {
        LOGI("update ui change, status: %{public}d, reason: %{public}d", status, reason);
    } else {
        napi_value callResult = nullptr;
        napi_value handler = nullptr;
        napi_get_reference_value(env_, authAsyncCallbackInfo_.callback, &handler);
        if (handler != nullptr) {
            napi_call_function(env_, nullptr, handler, DM_NAPI_ARGS_TWO, &result[0], &callResult);
            napi_delete_reference(env_, authAsyncCallbackInfo_.callback);
            authAsyncCallbackInfo_.callback = nullptr;
        } else {
            LOGE("handler is nullptr");
        }
        {
            std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
            g_authCallbackMap.erase(bundleName_);
        }
    }
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnGetDeviceProfileInfoListCallbackResult(DeviceProfileInfosAsyncCallbackInfo *jsCallback)
{
    LOGI("In");
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    if (jsCallback->code != DM_OK) {
        napi_value error = CreateBusinessError(env_, jsCallback->code, false);
        napi_reject_deferred(env_, jsCallback->deferred, error);
        LOGE("jsCallback->code(%{public}d) != DM_OK", jsCallback->code);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_value devInfosJsObj;
    napi_create_array(env_, &devInfosJsObj);
    bool isArray = false;
    napi_is_array(env_, devInfosJsObj, &isArray);
    if (!isArray) {
        LOGE("napi_create_array failed");
        napi_value error = CreateBusinessError(env_, ERR_DM_POINT_NULL, false);
        napi_reject_deferred(env_, jsCallback->deferred, error);
        napi_close_handle_scope(env_, scope);
        return;
    }
    DmDeviceProfileInfoToJsArray(env_, jsCallback->deviceProfileInfos, devInfosJsObj);
    napi_resolve_deferred(env_, jsCallback->deferred, devInfosJsObj);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnGetDeviceIconInfoCallbackResult(DeviceIconInfoAsyncCallbackInfo *jsCallback)
{
    LOGI("In");
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    if (jsCallback->code != DM_OK) {
        napi_value error = CreateBusinessError(env_, jsCallback->code, false);
        napi_reject_deferred(env_, jsCallback->deferred, error);
        LOGE("jsCallback->code(%{public}d) != DM_OK", jsCallback->code);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_value deviceIconInfoJsObj;
    napi_create_object(env_, &deviceIconInfoJsObj);
    DmDeviceIconInfoToJs(env_, jsCallback->deviceIconInfo, deviceIconInfoJsObj);
    napi_resolve_deferred(env_, jsCallback->deferred, deviceIconInfoJsObj);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnSetLocalDeviceNameCallbackResult(SetLocalDeviceNameAsyncCallbackInfo *jsCallback)
{
    LOGI("In");
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    if (jsCallback->code != DM_OK) {
        napi_value error = CreateBusinessError(env_, jsCallback->code, false);
        napi_reject_deferred(env_, jsCallback->deferred, error);
        LOGE("jsCallback->code(%{public}d) != DM_OK", jsCallback->code);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_value value = nullptr;
    napi_create_int32(env_, jsCallback->code, &value);
    napi_resolve_deferred(env_, jsCallback->deferred, value);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnSetRemoteDeviceNameCallbackResult(SetRemoteDeviceNameAsyncCallbackInfo *jsCallback)
{
    LOGI("In");
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    if (jsCallback->code != DM_OK) {
        napi_value error = CreateBusinessError(env_, jsCallback->code, false);
        napi_reject_deferred(env_, jsCallback->deferred, error);
        LOGE("jsCallback->code(%{public}d) != DM_OK", jsCallback->code);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_value value = nullptr;
    napi_create_int32(env_, jsCallback->code, &value);
    napi_resolve_deferred(env_, jsCallback->deferred, value);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::CreateDmCallback(napi_env env, std::string &bundleName, std::string &eventType)
{
    LOGI("CreateDmCallback for bundleName %{public}s eventType %{public}s", bundleName.c_str(), eventType.c_str());
    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE || eventType == DM_NAPI_EVENT_DEVICE_NAME_CHANGE) {
        RegisterDevStatusCallback(env, bundleName);
        return;
    }
    if (eventType == DM_NAPI_EVENT_DEVICE_DISCOVER_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL) {
        auto callback = std::make_shared<DmNapiDiscoveryCallback>(env, bundleName);
        {
            std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
            g_DiscoveryCallbackMap[bundleName] = callback;
        }
        std::shared_ptr<DmNapiDiscoveryCallback> discoveryCallback = callback;
        discoveryCallback->IncreaseRefCount();
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL) {
        auto callback = std::make_shared<DmNapiPublishCallback>(env, bundleName);
        {
            std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
            g_publishCallbackMap[bundleName] = callback;
        }
        std::shared_ptr<DmNapiPublishCallback> publishCallback = callback;
        publishCallback->IncreaseRefCount();
        return;
    }

    if (eventType == DM_NAPI_EVENT_REPLY_RESULT) {
        auto callback = std::make_shared<DmNapiDeviceManagerUiCallback>(env, bundleName);
        int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(bundleName, callback);
        if (ret != 0) {
            LOGE("RegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName.c_str());
            return;
        }
        {
            std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
            g_dmUiCallbackMap[bundleName] = callback;
        }
        return;
    }
}

void DeviceManagerNapi::RegisterDevStatusCallback(napi_env env, std::string &bundleName)
{
    LOGI("RegisterDevStatusCallback start for bundleName %{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        if (g_deviceStatusCallbackMap.find(bundleName) != g_deviceStatusCallbackMap.end()) {
            LOGI("bundleName already register.");
            return;
        }
    }
    auto callback = std::make_shared<DmNapiDeviceStatusCallback>(env, bundleName);
    std::string extra = "";
    int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName, extra, callback);
    if (ret != 0) {
        LOGE("RegisterDevStatusCallback failed ret %{public}d", ret);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        g_deviceStatusCallbackMap[bundleName] = callback;
    }
    return;
}

void DeviceManagerNapi::CreateDmCallback(napi_env env, std::string &bundleName,
                                         std::string &eventType, std::string &extra)
{
    LOGI("CreateDmCallback for bundleName %{public}s eventType %{public}s extra = %{public}s",
         bundleName.c_str(), eventType.c_str(), extra.c_str());
    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE) {
        auto callback = std::make_shared<DmNapiDeviceStatusCallback>(env, bundleName);
        int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName, extra, callback);
        if (ret != 0) {
            LOGE("RegisterDevStatusCallback failed for bundleName %{public}s", bundleName.c_str());
            return;
        }
        {
            std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
            g_deviceStatusCallbackMap[bundleName] = callback;
        }
    }
}

void DeviceManagerNapi::ReleasePublishCallback(std::string &bundleName)
{
    LOGI("ReleasePublishCallback for bundleName %{public}s", bundleName.c_str());
    std::shared_ptr<DmNapiPublishCallback> publishCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        auto iter = g_publishCallbackMap.find(bundleName);
        if (iter == g_publishCallbackMap.end()) {
            return;
        }
        publishCallback = iter->second;
    }
    publishCallback->DecreaseRefCount();
    if (publishCallback->GetRefCount() == 0) {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        g_publishCallbackMap.erase(bundleName);
    }
    return;
}

void DeviceManagerNapi::ReleaseDiscoveryCallback(std::string &bundleName)
{
    LOGI("ReleaseDiscoveryCallback for bundleName %{public}s", bundleName.c_str());
    std::shared_ptr<DmNapiDiscoveryCallback> DiscoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        auto iter = g_DiscoveryCallbackMap.find(bundleName);
        if (iter == g_DiscoveryCallbackMap.end()) {
            return;
        }
        DiscoveryCallback = iter->second;
    }
    DiscoveryCallback->DecreaseRefCount();
    if (DiscoveryCallback->GetRefCount() == 0) {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        g_DiscoveryCallbackMap.erase(bundleName);
    }
    return;
}

void DeviceManagerNapi::ReleaseDmCallback(std::string &bundleName, std::string &eventType)
{
    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE) {
        {
            std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
            auto iter = g_deviceStatusCallbackMap.find(bundleName);
            if (iter == g_deviceStatusCallbackMap.end()) {
                LOGE("ReleaseDmCallback: cannot find statusCallback for bundleName %{public}s", bundleName.c_str());
                return;
            }
        }
        int32_t ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName);
        if (ret != 0) {
            LOGE("UnRegisterDevStatusCallback failed for bundleName %{public}s", bundleName.c_str());
            return;
        }
        {
            std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
            g_deviceStatusCallbackMap.erase(bundleName);
        }
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_DISCOVER_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL) {
        ReleaseDiscoveryCallback(bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL) {
        ReleasePublishCallback(bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_REPLY_RESULT) {
        {
            std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
            auto iter = g_dmUiCallbackMap.find(bundleName);
            if (iter == g_dmUiCallbackMap.end()) {
                LOGE("cannot find dmFaCallback for bundleName %{public}s", bundleName.c_str());
                return;
            }
        }
        int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(bundleName);
        if (ret != 0) {
            LOGE("UnRegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName.c_str());
            return;
        }
        {
            std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
            g_dmUiCallbackMap.erase(bundleName);
        }
        return;
    }
}

napi_value DeviceManagerNapi::SetUserOperationSync(napi_env env, napi_callback_info info)
{
    LOGI("SetUserOperationSync in");
    if (!IsSystemApp()) {
        LOGI("SetUserOperationSync not SystemApp");
        CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_number, "action", "number")) {
        return nullptr;
    }
    int32_t action = 0;
    napi_get_value_int32(env, argv[0], &action);

    std::string params;
    if (!JsToStringAndCheck(env, argv[1], "actionResult", params)) {
        return nullptr;
    }
    napi_value result = nullptr;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    int32_t ret = DeviceManager::GetInstance().SetUserOperation(deviceManagerWrapper->bundleName_, action, params);
    if (ret != 0) {
        LOGE("SetUserOperation for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    napi_get_undefined(env, &result);
    return result;
}

void DmNapiDeviceManagerUiCallback::OnCall(const std::string &paramJson)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceManagerUiCallback: OnCall, No memory");
        return;
    }

    DmNapiAuthJsCallback *jsCallback = new DmNapiAuthJsCallback(bundleName_, "", paramJson, 0, 0);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnCall uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiAuthJsCallback *callback = reinterpret_cast<DmNapiAuthJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnCall, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDmUiCall(callback->token_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnCall work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}

void DeviceManagerNapi::OnDmUiCall(const std::string &paramJson)
{
    LOGI("OnCall for paramJson");
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value result;
    napi_create_object(env_, &result);
    SetValueUtf8String(env_, "param", paramJson, result);
    OnEvent(DM_NAPI_EVENT_REPLY_RESULT, DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DmNapiBindTargetCallback::OnBindResult(const PeerTargetId &targetId, int32_t result, int32_t status,
    std::string content)
{
    (void)targetId;
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("js4.0 DmNapiBindTargetCallback::OnBindResult, No memory");
        return;
    }

    DmNapiAuthJsCallback *jsCallback = new DmNapiAuthJsCallback(bundleName_, content, "", status, result);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnBindResult uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiAuthJsCallback *callback = reinterpret_cast<DmNapiAuthJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnBindResult, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnAuthResult(callback->deviceId_, callback->token_,
                callback->status_, callback->reason_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnBindResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}

int32_t DeviceManagerNapi::DumpDeviceInfo(
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    CHECK_NULL_RETURN(deviceBasicInfoListAsyncCallbackInfo, ERR_DM_POINT_NULL);
    if (deviceBasicInfoListAsyncCallbackInfo->devList.size() > DM_MAX_DEVICE_SIZE) {
        LOGE("CallGetAvailableDeviceListStatus invalid devList size");
        return DM_ERR_FAILED;
    }
    for (unsigned int i = 0; i < deviceBasicInfoListAsyncCallbackInfo->devList.size(); i++) {
        LOGI("DeviceId:%{public}s deviceName:%{public}s deviceTypeId:%{public}d ",
             GetAnonyString(deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceId).c_str(),
             GetAnonyString(deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceName).c_str(),
             deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceTypeId);
    }
    return DM_OK;
}

void DeviceManagerNapi::CallGetAvailableDeviceListStatus(napi_env env, napi_status &status,
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    if (DumpDeviceInfo(deviceBasicInfoListAsyncCallbackInfo) != DM_OK) {
        return;
    }
    napi_value array[DM_NAPI_ARGS_TWO] = {0};
    bool isArray = false;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &array[1]));
    NAPI_CALL_RETURN_VOID(env, napi_is_array(env, array[1], &isArray));
    if (!isArray) {
        LOGE("napi_create_array fail");
    }
    if (deviceBasicInfoListAsyncCallbackInfo->status == 0) {
        if (deviceBasicInfoListAsyncCallbackInfo->devList.size() > 0) {
            for (size_t i = 0; i != deviceBasicInfoListAsyncCallbackInfo->devList.size(); ++i) {
                DeviceBasicInfoToJsArray(env, deviceBasicInfoListAsyncCallbackInfo->devList, i, array[1]);
            }
            LOGI("devList is OK");
        } else {
            LOGE("devList is null"); // CB come here
        }
    } else {
        array[0] = CreateBusinessError(env, deviceBasicInfoListAsyncCallbackInfo->ret, false);
    }
    if (deviceBasicInfoListAsyncCallbackInfo->deferred) {
        if (deviceBasicInfoListAsyncCallbackInfo->status == 0) {
            napi_resolve_deferred(env, deviceBasicInfoListAsyncCallbackInfo->deferred, array[1]);
        } else {
            napi_reject_deferred(env, deviceBasicInfoListAsyncCallbackInfo->deferred, array[0]);
        }
    } else {
        napi_value callResult = nullptr;
        napi_value handler = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, deviceBasicInfoListAsyncCallbackInfo->callback,
            &handler));
        if (handler != nullptr) {
            NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, handler, DM_NAPI_ARGS_TWO, &array[0],
                &callResult));
            NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, deviceBasicInfoListAsyncCallbackInfo->callback));
            deviceBasicInfoListAsyncCallbackInfo->callback = nullptr;
        } else {
            LOGE("handler is nullptr");
        }
    }
}

void DeviceManagerNapi::CallAsyncWork(napi_env env,
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "GetAvailableListInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            DeviceBasicInfoListAsyncCallbackInfo *devBasicInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceBasicInfoListAsyncCallbackInfo *>(data);
            int32_t ret = 0;
            ret = DeviceManager::GetInstance().GetAvailableDeviceList(devBasicInfoListAsyncCallbackInfo->bundleName,
                devBasicInfoListAsyncCallbackInfo->devList);
            if (ret != 0) {
                LOGE("CallAsyncWork for bundleName %{public}s failed, ret %{public}d",
                    devBasicInfoListAsyncCallbackInfo->bundleName.c_str(), ret);
                devBasicInfoListAsyncCallbackInfo->status = -1;
                devBasicInfoListAsyncCallbackInfo->ret = ret;
            } else {
                devBasicInfoListAsyncCallbackInfo->status = 0;
            }
            LOGI("CallAsyncWork status %{public}d , ret %{public}d", devBasicInfoListAsyncCallbackInfo->status,
                devBasicInfoListAsyncCallbackInfo->ret);
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceBasicInfoListAsyncCallbackInfo *dBasicInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceBasicInfoListAsyncCallbackInfo *>(data);
            CallGetAvailableDeviceListStatus(env, status, dBasicInfoListAsyncCallbackInfo);
            napi_delete_async_work(env, dBasicInfoListAsyncCallbackInfo->asyncWork);
            delete dBasicInfoListAsyncCallbackInfo;
            dBasicInfoListAsyncCallbackInfo = nullptr;
        },
        (void *)deviceBasicInfoListAsyncCallbackInfo, &deviceBasicInfoListAsyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, deviceBasicInfoListAsyncCallbackInfo->asyncWork, napi_qos_user_initiated);
}

napi_value DeviceManagerNapi::CallDeviceList(napi_env env, napi_callback_info info,
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    napi_value result = nullptr;
    std::string extra = "";
    deviceBasicInfoListAsyncCallbackInfo->extra = extra;
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[0], &eventHandleType);
    if (eventHandleType == napi_function) {
        LOGI("CallDeviceList for argc %{public}zu Type = %{public}d", argc, (int)eventHandleType);
        napi_create_reference(env, argv[0], 1, &deviceBasicInfoListAsyncCallbackInfo->callback);
        CallAsyncWork(env, deviceBasicInfoListAsyncCallbackInfo);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::GetAvailableDeviceListSync(napi_env env, napi_callback_info info)
{
    LOGI("In");
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    size_t argc = 0;
    bool isArray = false;
    napi_create_array(env, &result);
    napi_is_array(env, result, &isArray);
    if (!isArray) {
        LOGE("napi_create_array fail");
    }
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    std::vector<DmDeviceBasicInfo> devList;
    ret = DeviceManager::GetInstance().GetAvailableDeviceList(deviceManagerWrapper->bundleName_, devList);
    if (ret != 0) {
        LOGE("GetTrustedDeviceList for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGD("DeviceManager::GetAvailableDeviceListSync");
    if (devList.size() > 0) {
        for (size_t i = 0; i != devList.size(); ++i) {
            DeviceBasicInfoToJsArray(env, devList, (int32_t)i, result);
        }
    }
    return result;
}

napi_value DeviceManagerNapi::GetAvailableDeviceListPromise(napi_env env,
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    std::string extra = "";
    deviceBasicInfoListAsyncCallbackInfo->extra = extra;
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    deviceBasicInfoListAsyncCallbackInfo->deferred = deferred;
    CallAsyncWork(env, deviceBasicInfoListAsyncCallbackInfo);
    return promise;
}

napi_value DeviceManagerNapi::GetAvailableDeviceList(napi_env env, napi_callback_info info)
{
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    size_t argc = 0;
    std::vector<DmDeviceBasicInfo> devList;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    auto *deviceBasicInfoListAsyncCallbackInfo = new DeviceBasicInfoListAsyncCallbackInfo();
    if (deviceBasicInfoListAsyncCallbackInfo == nullptr) {
        return nullptr;
    }
    deviceBasicInfoListAsyncCallbackInfo->env = env;
    deviceBasicInfoListAsyncCallbackInfo->devList = devList;
    deviceBasicInfoListAsyncCallbackInfo->bundleName = deviceManagerWrapper->bundleName_;
    if (argc == DM_NAPI_ARGS_ZERO) {
        return GetAvailableDeviceListPromise(env, deviceBasicInfoListAsyncCallbackInfo);
    } else if (argc == DM_NAPI_ARGS_ONE) {
        GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
        if (!IsFunctionType(env, argv[0])) {
            DeleteAsyncCallbackInfo(deviceBasicInfoListAsyncCallbackInfo);
            return nullptr;
        }
        return CallDeviceList(env, info, deviceBasicInfoListAsyncCallbackInfo);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::GetLocalDeviceNetworkId(napi_env env, napi_callback_info info)
{
    LOGI("GetLocalDeviceNetworkId in");
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        CreateBusinessError(env, ERR_DM_NO_PERMISSION);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    std::string networkId;
    size_t argc = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_int32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceNetWorkId(deviceManagerWrapper->bundleName_, networkId);
    if (ret != 0) {
        LOGE("GetLocalDeviceNetworkId for failed, ret %{public}d", ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGI("DeviceManager::GetLocalDeviceNetworkId networkId:%{public}s", GetAnonyString(std::string(networkId)).c_str());
    napi_create_string_utf8(env, networkId.c_str(), networkId.size(), &result);
    return result;
}

napi_value DeviceManagerNapi::GetLocalDeviceId(napi_env env, napi_callback_info info)
{
    LOGI("GetLocalDeviceId in");
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        CreateBusinessError(env, ERR_DM_NO_PERMISSION);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    std::string deviceId;
    size_t argc = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_int32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceId(deviceManagerWrapper->bundleName_, deviceId);
    if (ret != 0) {
        LOGE("GetLocalDeviceId for failed, ret %{public}d", ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGI("DeviceManager::GetLocalDeviceId deviceId:%{public}s", GetAnonyString(std::string(deviceId)).c_str());
    napi_create_string_utf8(env, deviceId.c_str(), deviceId.size(), &result);
    return result;
}

napi_value DeviceManagerNapi::GetLocalDeviceName(napi_env env, napi_callback_info info)
{
    LOGI("GetLocalDeviceName in");
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        CreateBusinessError(env, ERR_DM_NO_PERMISSION);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    std::string deviceName;
    size_t argc = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceName(deviceManagerWrapper->bundleName_, deviceName);
    if (ret != 0) {
        LOGE("GetLocalDeviceName for failed, ret %{public}d", ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGI("DeviceManager::GetLocalDeviceName deviceName:%{public}s", GetAnonyString(std::string(deviceName)).c_str());
    napi_create_string_utf8(env, deviceName.c_str(), deviceName.size(), &result);
    return result;
}

napi_value DeviceManagerNapi::GetLocalDeviceType(napi_env env, napi_callback_info info)
{
    LOGI("GetLocalDeviceType in");
    if (DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        CreateBusinessError(env, ERR_DM_NO_PERMISSION);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    int32_t deviceType = 0;
    size_t argc = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceType(deviceManagerWrapper->bundleName_, deviceType);
    if (ret != 0) {
        LOGE("GetLocalDeviceType for failed, ret %{public}d", ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGI("DeviceManager::GetLocalDeviceType deviceType:%{public}d", deviceType);
    napi_create_int32(env, deviceType, &result);
    return result;
}

napi_value DeviceManagerNapi::GetDeviceName(napi_env env, napi_callback_info info)
{
    LOGI("GetDeviceName in");
    napi_value result = nullptr;
    std::string deviceName;
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE,  "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    std::string networkId;
    if (!JsToStringAndCheck(env, argv[0], "networkId", networkId)) {
        return nullptr;
    }

    int32_t ret = DeviceManager::GetInstance().GetDeviceName(deviceManagerWrapper->bundleName_, networkId, deviceName);
    if (ret != 0) {
        LOGE("GetDeviceName for failed, ret %{public}d", ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGI("DeviceManager::GetDeviceName deviceName:%{public}s", GetAnonyString(std::string(deviceName)).c_str());
    napi_create_string_utf8(env, deviceName.c_str(), deviceName.size(), &result);
    return result;
}

napi_value DeviceManagerNapi::GetDeviceType(napi_env env, napi_callback_info info)
{
    LOGI("GetDeviceType in");
    napi_value result = nullptr;
    int32_t deviceType;
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE,  "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    std::string networkId;
    if (!JsToStringAndCheck(env, argv[0], "networkId", networkId)) {
        return nullptr;
    }

    int32_t ret = DeviceManager::GetInstance().GetDeviceType(deviceManagerWrapper->bundleName_, networkId, deviceType);
    if (ret != 0) {
        LOGE("GetDeviceType for failed, ret %{public}d", ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGI("DeviceManager::GetDeviceType deviceType:%{public}d", deviceType);
    napi_create_int32(env, deviceType, &result);
    return result;
}

void DeviceManagerNapi::LockDiscoveryCallbackMutex(napi_env env, std::string &bundleName,
    std::map<std::string, std::string> discParam, std::string &extra, uint32_t subscribeId)
{
    std::shared_ptr<DmNapiDiscoveryCallback> discoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        auto iter = g_DiscoveryCallbackMap.find(bundleName);
        if (iter == g_DiscoveryCallbackMap.end()) {
            discoveryCallback = std::make_shared<DmNapiDiscoveryCallback>(env, bundleName);
            g_DiscoveryCallbackMap[bundleName] = discoveryCallback;
        } else {
            discoveryCallback = iter->second;
        }
    }
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID, std::to_string(tokenId)));
    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, extra));
    int32_t ret = DeviceManager::GetInstance().StartDiscovering(bundleName, discParam, filterOps, discoveryCallback);
    if (ret != 0) {
        LOGE("Discovery failed, bundleName %{public}s, ret %{public}d", bundleName.c_str(), ret);
        CreateBusinessError(env, ret);
        discoveryCallback->OnDiscoveryFailed(static_cast<uint16_t>(subscribeId), ret);
    }
    return;
}

napi_value DeviceManagerNapi::StartDeviceDiscover(napi_env env, napi_callback_info info)
{
    LOGI("StartDeviceDiscover in");
    std::string extra = "";
    std::map<std::string, std::string> discParam;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    size_t argcNum = 0;
    int32_t discoverTargetType = -1;
    uint32_t subscribeId = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcNum, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    if (argcNum == DM_NAPI_ARGS_ONE) {
        GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
        if (!JsToDiscoverTargetType(env, argv[DM_NAPI_ARGS_ZERO], discoverTargetType) || discoverTargetType != 1) {
            return nullptr;
        }
        JsToDiscoveryParam(env, argv[DM_NAPI_ARGS_ZERO], discParam);
    } else if (argcNum == DM_NAPI_ARGS_TWO) {
        GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
        if (!JsToDiscoverTargetType(env, argv[DM_NAPI_ARGS_ZERO], discoverTargetType) || discoverTargetType != 1) {
            return nullptr;
        }
        JsToDiscoveryParam(env, argv[DM_NAPI_ARGS_ZERO], discParam);
        napi_valuetype objectType = napi_undefined;
        napi_typeof(env, argv[DM_NAPI_ARGS_ONE], &objectType);
        if (!(CheckArgsType(env, objectType == napi_object, "filterOptions", "object or undefined"))) {
            return nullptr;
        }
        JsToDmDiscoveryExtra(env, argv[DM_NAPI_ARGS_ONE], extra);
    }
    LockDiscoveryCallbackMutex(env, deviceManagerWrapper->bundleName_, discParam, extra, subscribeId);
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::StopDeviceDiscover(napi_env env, napi_callback_info info)
{
    LOGI("StopDeviceDiscover in");
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    size_t argc = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (argc != 0) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(tokenId, deviceManagerWrapper->bundleName_);
    if (ret != 0) {
        LOGE("StopDeviceDiscovery for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
        return result;
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::PublishDeviceDiscoverySync(napi_env env, napi_callback_info info)
{
    LOGI("PublishDeviceDiscoverySync in");
    if (!IsSystemApp()) {
        LOGI("PublishDeviceDiscoverySync not SystemApp");
        CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE,  "Wrong number of arguments, required 1")) {
        return nullptr;
    }

    napi_value result = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_object, "publishInfo", "object")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    std::shared_ptr<DmNapiPublishCallback> publishCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        auto iter = g_publishCallbackMap.find(deviceManagerWrapper->bundleName_);
        if (iter == g_publishCallbackMap.end()) {
            publishCallback = std::make_shared<DmNapiPublishCallback>(env, deviceManagerWrapper->bundleName_);
            g_publishCallbackMap[deviceManagerWrapper->bundleName_] = publishCallback;
        } else {
            publishCallback = iter->second;
        }
    }
    DmPublishInfo publishInfo;
    JsToDmPublishInfo(env, argv[0], publishInfo);
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(deviceManagerWrapper->bundleName_, publishInfo,
        publishCallback);
    if (ret != 0) {
        LOGE("PublishDeviceDiscovery for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
        publishCallback->OnPublishResult(publishInfo.publishId, ret);
        return result;
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::UnPublishDeviceDiscoverySync(napi_env env, napi_callback_info info)
{
    LOGI("UnPublishDeviceDiscoverySync in");
    if (!IsSystemApp()) {
        LOGI("UnPublishDeviceDiscoverySync not SystemApp");
        CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE,  "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    napi_value result = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_number, "publishId", "number")) {
        return nullptr;
    }
    int32_t publishId = 0;
    napi_get_value_int32(env, argv[0], &publishId);

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(deviceManagerWrapper->bundleName_, publishId);
    if (ret != 0) {
        LOGE("UnPublishDeviceDiscovery bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
        return result;
    }

    napi_get_undefined(env, &result);
    return result;
}

void DeviceManagerNapi::BindDevOrTarget(DeviceManagerNapi *deviceManagerWrapper, const std::string &deviceId,
    napi_env env, napi_value &object)
{
    LOGI("Bind devices or target start");
    std::string bindParam;
    bool isMetaType = false;
    JsToBindParam(env, object, bindParam, authAsyncCallbackInfo_.authType, isMetaType);

    if (isMetaType) {
        std::shared_ptr<DmNapiBindTargetCallback> bindTargetCallback = nullptr;
        {
            std::lock_guard<std::mutex> autoLock(g_bindCallbackMapMutex);
            auto iter = g_bindCallbackMap.find(deviceManagerWrapper->bundleName_);
            if (iter == g_bindCallbackMap.end()) {
                bindTargetCallback = std::make_shared<DmNapiBindTargetCallback>(env, deviceManagerWrapper->bundleName_);
                g_bindCallbackMap[deviceManagerWrapper->bundleName_] = bindTargetCallback;
            } else {
                bindTargetCallback = iter->second;
            }
        }
        int32_t ret = BindTargetWarpper(deviceManagerWrapper->bundleName_, deviceId, bindParam, bindTargetCallback);
        if (ret != 0) {
            LOGE("BindTarget for bundleName %{public}s failed, ret %{public}d",
                deviceManagerWrapper->bundleName_.c_str(), ret);
            CreateBusinessError(env, ret);
        }
        return;
    }

    std::shared_ptr<DmNapiAuthenticateCallback> bindDeviceCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
        auto iter = g_authCallbackMap.find(deviceManagerWrapper->bundleName_);
        if (iter == g_authCallbackMap.end()) {
            bindDeviceCallback = std::make_shared<DmNapiAuthenticateCallback>(env, deviceManagerWrapper->bundleName_);
            g_authCallbackMap[deviceManagerWrapper->bundleName_] = bindDeviceCallback;
        } else {
            bindDeviceCallback = iter->second;
        }
    }
    int32_t ret = DeviceManager::GetInstance().BindDevice(deviceManagerWrapper->bundleName_,
        authAsyncCallbackInfo_.authType, deviceId, bindParam, bindDeviceCallback);
    if (ret != 0) {
        LOGE("BindDevice for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    return;
}

napi_value DeviceManagerNapi::BindTarget(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, DM_NAPI_ARGS_THREE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_THREE,  "Wrong number of arguments, required 3")) {
        return nullptr;
    }
    napi_valuetype bindPramType = napi_undefined;
    napi_typeof(env, argv[DM_NAPI_ARGS_ONE], &bindPramType);
    if (!CheckArgsType(env, bindPramType == napi_object, "bindParam", "object")) {
        return nullptr;
    }
    if (!IsFunctionType(env, argv[DM_NAPI_ARGS_TWO])) {
        return nullptr;
    }
    napi_value result = nullptr;
    authAsyncCallbackInfo_.env = env;
    napi_create_reference(env, argv[DM_NAPI_ARGS_TWO], 1, &authAsyncCallbackInfo_.callback);
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    std::string deviceId;
    if (!JsToStringAndCheck(env, argv[DM_NAPI_ARGS_ZERO], "deviceId", deviceId)) {
        return nullptr;
    }

    napi_value object = argv[DM_NAPI_ARGS_ONE];
    BindDevOrTarget(deviceManagerWrapper, deviceId, env, object);
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::UnBindTarget(napi_env env, napi_callback_info info)
{
    LOGI("UnBindDevice");
    napi_value result = nullptr;
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE,  "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    std::string deviceId;
    if (!JsToStringAndCheck(env, argv[0], "deviceId", deviceId)) {
        return nullptr;
    }

    LOGI("UnBindDevice deviceId = %{public}s", GetAnonyString(deviceId).c_str());
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    int32_t ret = DeviceManager::GetInstance().UnBindDevice(deviceManagerWrapper->bundleName_, deviceId);
    if (ret != 0) {
        LOGE("UnBindDevice for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::JsOnFrench(napi_env env, int32_t num, napi_value thisVar, napi_value argv[])
{
    std::string eventType;
    if (!JsToStringAndCheck(env, argv[0], "type", eventType)) {
        return nullptr;
    }

    napi_value result = nullptr;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    LOGI("JsOn for bundleName %{public}s, eventType %{public}s ", deviceManagerWrapper->bundleName_.c_str(),
        eventType.c_str());
    deviceManagerWrapper->On(eventType, argv[num + 1]);

    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE) {
        if (num == 1) {
            std::string extraString;
            if (!JsToStringAndCheck(env, argv[1], "extra", extraString)) {
                return nullptr;
            }
            LOGI("extra = %{public}s", extraString.c_str());
            CreateDmCallback(env, deviceManagerWrapper->bundleName_, eventType, extraString);
        } else {
            CreateDmCallback(env, deviceManagerWrapper->bundleName_, eventType);
        }
    } else {
        CreateDmCallback(env, deviceManagerWrapper->bundleName_, eventType);
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::JsOn(napi_env env, napi_callback_info info)
{
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (argc == DM_NAPI_ARGS_THREE) {
        LOGI("JsOn in argc == 3");
        GET_PARAMS(env, info, DM_NAPI_ARGS_THREE);
        if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_THREE, "Wrong number of arguments, required 3")) {
            return nullptr;
        }
        napi_valuetype eventValueType = napi_undefined;
        napi_typeof(env, argv[0], &eventValueType);
        if (!CheckArgsType(env, eventValueType == napi_string, "type", "string")) {
            return nullptr;
        }
        napi_valuetype valueType;
        napi_typeof(env, argv[1], &valueType);
        if (!CheckArgsType(env, (valueType == napi_string || valueType == napi_object),
            "extra", "string | object")) {
            return nullptr;
        }
        if (!IsFunctionType(env, argv[DM_NAPI_ARGS_TWO])) {
            return nullptr;
        }
        return JsOnFrench(env, 1, thisVar, argv);
    } else {
        GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
        if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
            return nullptr;
        }
        napi_valuetype eventValueType = napi_undefined;
        napi_typeof(env, argv[0], &eventValueType);
        if (!CheckArgsType(env, eventValueType == napi_string, "type", "string")) {
            return nullptr;
        }
        if (!IsFunctionType(env, argv[1])) {
            return nullptr;
        }
        return JsOnFrench(env, 0, thisVar, argv);
    }
}

napi_value DeviceManagerNapi::JsOffFrench(napi_env env, int32_t num, napi_value thisVar, napi_value argv[])
{
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    std::string eventType;
    if (!JsToStringAndCheck(env, argv[0], "type", eventType)) {
        return nullptr;
    }
    napi_value result = nullptr;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    LOGI("JsOff for bundleName %{public}s, eventType %{public}s ", deviceManagerWrapper->bundleName_.c_str(),
        eventType.c_str());
    deviceManagerWrapper->Off(eventType);
    ReleaseDmCallback(deviceManagerWrapper->bundleName_, eventType);

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::JsOff(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (argc == DM_NAPI_ARGS_THREE) {
        LOGI("JsOff in argc == 3");
        GET_PARAMS(env, info, DM_NAPI_ARGS_THREE);
        if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
            return nullptr;
        }
        napi_valuetype eventValueType = napi_undefined;
        napi_typeof(env, argv[0], &eventValueType);
        if (!CheckArgsType(env, eventValueType == napi_string, "type", "string")) {
            return nullptr;
        }
        napi_valuetype valueType;
        napi_typeof(env, argv[1], &valueType);
        if (!CheckArgsType(env, (valueType == napi_string || valueType == napi_object), "extra", "string or object")) {
            return nullptr;
        }
        if (argc > DM_NAPI_ARGS_ONE) {
            if (!IsFunctionType(env, argv[DM_NAPI_ARGS_TWO])) {
                return nullptr;
            }
        }
        return JsOffFrench(env, 1, thisVar, argv);
    } else {
        GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
        if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
            return nullptr;
        }
        napi_valuetype eventValueType = napi_undefined;
        napi_typeof(env, argv[0], &eventValueType);
        if (!CheckArgsType(env, eventValueType == napi_string, "type", "string")) {
            return nullptr;
        }
        if (argc > DM_NAPI_ARGS_ONE) {
            if (!IsFunctionType(env, argv[1])) {
                return nullptr;
            }
        }
        return JsOffFrench(env, 0, thisVar, argv);
    }
}

napi_value DeviceManagerNapi::GetDeviceProfileInfoListPromise(napi_env env,
    DeviceProfileInfosAsyncCallbackInfo *asyncCallback)
{
    LOGI("In");
    napi_value promise = 0;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallback->deferred = deferred;
    napi_value workName;
    napi_create_string_latin1(env, "GetDeviceProfileInfoListPromise", NAPI_AUTO_LENGTH, &workName);
    napi_create_async_work(env, nullptr, workName,
        [](napi_env env, void *data) {
            DeviceProfileInfosAsyncCallbackInfo *jsCallback =
                reinterpret_cast<DeviceProfileInfosAsyncCallbackInfo *>(data);
            std::shared_ptr<DmNapiGetDeviceProfileInfoListCallback> callback =
                std::make_shared<DmNapiGetDeviceProfileInfoListCallback>(jsCallback->env, jsCallback->bundleName,
                jsCallback->deferred);
            int32_t ret = DeviceManager::GetInstance().GetDeviceProfileInfoList(jsCallback->bundleName,
                jsCallback->filterOptions, callback);
            jsCallback->code = ret;
            if (ret != DM_OK) {
                LOGE("GetDeviceProfileInfoList failed, bundleName:%{public}s, ret=%{public}d",
                    jsCallback->bundleName.c_str(), ret);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceProfileInfosAsyncCallbackInfo *jsCallback =
                reinterpret_cast<DeviceProfileInfosAsyncCallbackInfo *>(data);
            if (jsCallback->code != DM_OK) {
                napi_value error = CreateBusinessError(env, jsCallback->code, false);
                napi_reject_deferred(env, jsCallback->deferred, error);
            }
            napi_delete_async_work(env, jsCallback->asyncWork);
            delete jsCallback;
            jsCallback = nullptr;
        },
        (void *)asyncCallback, &asyncCallback->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallback->asyncWork, napi_qos_user_initiated);
    return promise;
}

napi_value DeviceManagerNapi::JsGetDeviceProfileInfoList(napi_env env, napi_callback_info info)
{
    LOGI("In");
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        CreateBusinessError(env, ret);
        return nullptr;
    }

    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        LOGE("deviceManagerWrapper is NULL");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    napi_value argv[DM_NAPI_ARGS_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    DmDeviceProfileInfoFilterOptions filterOptions;
    JsToDmDeviceProfileInfoFilterOptions(env, argv[0], filterOptions);
    auto *jsCallback = new DeviceProfileInfosAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("jsCallback is nullptr");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }

    jsCallback->env = env;
    jsCallback->bundleName = deviceManagerWrapper->bundleName_;
    jsCallback->filterOptions = filterOptions;
    return GetDeviceProfileInfoListPromise(env, jsCallback);
}

napi_value DeviceManagerNapi::GetDeviceIconInfoPromise(napi_env env, DeviceIconInfoAsyncCallbackInfo *asyncCallback)
{
    LOGI("In");
    napi_value promise = 0;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallback->deferred = deferred;
    napi_value workName;
    napi_create_string_latin1(env, "GetDeviceIconInfoPromise", NAPI_AUTO_LENGTH, &workName);
    napi_create_async_work(env, nullptr, workName,
        [](napi_env env, void *data) {
            DeviceIconInfoAsyncCallbackInfo *jsCallback =
                reinterpret_cast<DeviceIconInfoAsyncCallbackInfo *>(data);
            std::shared_ptr<DmNapiGetDeviceIconInfoCallback> callback =
                std::make_shared<DmNapiGetDeviceIconInfoCallback>(jsCallback->env, jsCallback->bundleName,
            jsCallback->deferred);
            int32_t ret = DeviceManager::GetInstance().GetDeviceIconInfo(jsCallback->bundleName,
                jsCallback->filterOptions, callback);
            jsCallback->code = ret;
            if (ret != DM_OK) {
                LOGE("GetDeviceIconInfoPromise failed, bundleName:%{public}s, ret=%{public}d",
                    jsCallback->bundleName.c_str(), ret);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceIconInfoAsyncCallbackInfo *jsCallback =
                reinterpret_cast<DeviceIconInfoAsyncCallbackInfo *>(data);
            if (jsCallback->code != DM_OK) {
                napi_value error = CreateBusinessError(env, jsCallback->code, false);
                napi_reject_deferred(env, jsCallback->deferred, error);
            }
            napi_delete_async_work(env, jsCallback->asyncWork);
            delete jsCallback;
            jsCallback = nullptr;
        },
        (void *)asyncCallback, &asyncCallback->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallback->asyncWork, napi_qos_user_initiated);
    return promise;
}

napi_value DeviceManagerNapi::JsGetDeviceIconInfo(napi_env env, napi_callback_info info)
{
    LOGI("In");
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        CreateBusinessError(env, ret);
        return nullptr;
    }

    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        LOGE("deviceManagerWrapper is NULL");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    napi_value argv[DM_NAPI_ARGS_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    DmDeviceIconInfoFilterOptions filterOptions;
    JsToDmDeviceIconInfoFilterOptions(env, argv[0], filterOptions);
    auto *jsCallback = new DeviceIconInfoAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("jsCallback is nullptr");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }

    jsCallback->env = env;
    jsCallback->bundleName = deviceManagerWrapper->bundleName_;
    jsCallback->filterOptions = filterOptions;
    return GetDeviceIconInfoPromise(env, jsCallback);
}

napi_value DeviceManagerNapi::SetHeartbeatPolicy(napi_env env, napi_callback_info info)
{
    LOGI("in");
    size_t argsCount = 0;
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argsCount, nullptr, &thisArg, nullptr));
    if (!CheckArgsCount(env, argsCount >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
        return nullptr;
    }
    if (!IsSystemApp()) {
        LOGI("The caller is not SystemApp");
        CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_number, "policy", "number")) {
        return nullptr;
    }
    int32_t policy = 0;
    napi_get_value_int32(env, argv[0], &policy);

    napi_typeof(env, argv[DM_NAPI_ARGS_ONE], &valueType);
    if (!CheckArgsType(env, valueType == napi_number, "delayTime", "number")) {
        return nullptr;
    }
    int32_t delayTime = 0;
    napi_get_value_int32(env, argv[DM_NAPI_ARGS_ONE], &delayTime);

    napi_value result = nullptr;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    std::map<std::string, std::string> policyParam;
    policyParam[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = std::to_string(policy);
    policyParam[PARAM_KEY_POLICY_TIME_OUT] = std::to_string(delayTime);
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(deviceManagerWrapper->bundleName_, policyParam);
    if (ret != 0) {
        LOGE("bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::PutDeviceProfileInfoListPromise(napi_env env,
    PutDeviceProfileInfoListAsyncCallbackInfo *asyncCallback)
{
    LOGI("In");
    napi_value promise = 0;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallback->deferred = deferred;
    napi_value workName;
    napi_create_string_latin1(env, "PutDeviceProfileInfoListPromise", NAPI_AUTO_LENGTH, &workName);
    napi_create_async_work(env, nullptr, workName,
        [](napi_env env, void *data) {
            PutDeviceProfileInfoListAsyncCallbackInfo *jsCallback =
                reinterpret_cast<PutDeviceProfileInfoListAsyncCallbackInfo *>(data);
            jsCallback->code = DeviceManager::GetInstance().PutDeviceProfileInfoList(jsCallback->bundleName,
                jsCallback->deviceProfileInfos);
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            PutDeviceProfileInfoListAsyncCallbackInfo *jsCallback =
                reinterpret_cast<PutDeviceProfileInfoListAsyncCallbackInfo *>(data);
            if (jsCallback->code != DM_OK) {
                napi_value error = CreateBusinessError(env, jsCallback->code, false);
                napi_reject_deferred(env, jsCallback->deferred, error);
            } else {
                napi_value result = nullptr;
                napi_create_int32(env, DM_OK, &result);
                napi_resolve_deferred(env, jsCallback->deferred, result);
            }
            napi_delete_async_work(env, jsCallback->asyncWork);
            delete jsCallback;
            jsCallback = nullptr;
        },
        (void *)asyncCallback, &asyncCallback->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallback->asyncWork, napi_qos_user_initiated);
    return promise;
}

napi_value DeviceManagerNapi::JsPutDeviceProfileInfoList(napi_env env, napi_callback_info info)
{
    LOGI("In");
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        CreateBusinessError(env, ret);
        return nullptr;
    }

    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        LOGE("deviceManagerWrapper is NULL");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    std::vector<DmDeviceProfileInfo> deviceProfileInfos;
    if (!JsToDmDeviceProfileInfos(env, argv[0], deviceProfileInfos)) {
        LOGE("JsToDmDeviceProfileInfos fail");
        CreateBusinessError(env, ERR_INVALID_PARAMS);
        return nullptr;
    }
    auto *jsCallback = new PutDeviceProfileInfoListAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("jsCallback is nullptr");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    jsCallback->env = env;
    jsCallback->bundleName = deviceManagerWrapper->bundleName_;
    jsCallback->deviceProfileInfos = deviceProfileInfos;
    return PutDeviceProfileInfoListPromise(env, jsCallback);
}

napi_value DeviceManagerNapi::GetLocalDisplayDeviceNamePromise(napi_env env,
    GetLocalDisplayDeviceNameAsyncCallbackInfo *asyncCallback)
{
    LOGI("In");
    napi_value promise = 0;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallback->deferred = deferred;
    napi_value workName;
    napi_create_string_latin1(env, "GetLocalDisplayDeviceNamePromise", NAPI_AUTO_LENGTH, &workName);
    napi_create_async_work(env, nullptr, workName,
        [](napi_env env, void *data) {
            GetLocalDisplayDeviceNameAsyncCallbackInfo *jsCallback =
                reinterpret_cast<GetLocalDisplayDeviceNameAsyncCallbackInfo *>(data);
            jsCallback->code = DeviceManager::GetInstance().GetLocalDisplayDeviceName(jsCallback->bundleName,
                jsCallback->maxNameLength, jsCallback->displayName);
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            GetLocalDisplayDeviceNameAsyncCallbackInfo *jsCallback =
                reinterpret_cast<GetLocalDisplayDeviceNameAsyncCallbackInfo *>(data);
            if (jsCallback->code != DM_OK) {
                napi_value error = CreateBusinessError(env, jsCallback->code, false);
                napi_reject_deferred(env, jsCallback->deferred, error);
            } else {
                napi_value result = nullptr;
                napi_create_string_utf8(env, jsCallback->displayName.c_str(), NAPI_AUTO_LENGTH, &result);
                napi_resolve_deferred(env, jsCallback->deferred, result);
            }
            napi_delete_async_work(env, jsCallback->asyncWork);
            delete jsCallback;
            jsCallback = nullptr;
        },
        (void *)asyncCallback, &asyncCallback->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallback->asyncWork, napi_qos_user_initiated);
    return promise;
}

napi_value DeviceManagerNapi::JsGetLocalDisplayDeviceName(napi_env env, napi_callback_info info)
{
    LOGI("In");
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    int32_t maxNameLength = 0;
    if (argc >= DM_NAPI_ARGS_ONE) {
        napi_valuetype maxNameLengthType = napi_undefined;
        napi_typeof(env, argv[0], &maxNameLengthType);
        if (!CheckArgsType(env, maxNameLengthType == napi_number, "maxNameLength", "number")) {
            return nullptr;
        }
        napi_get_value_int32(env, argv[0], &maxNameLength);
    }

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        LOGE("deviceManagerWrapper is NULL");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    auto *jsCallback = new GetLocalDisplayDeviceNameAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("jsCallback is nullptr");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }

    jsCallback->env = env;
    jsCallback->bundleName = deviceManagerWrapper->bundleName_;
    jsCallback->maxNameLength = maxNameLength;
    return GetLocalDisplayDeviceNamePromise(env, jsCallback);
}

napi_value DeviceManagerNapi::SetLocalDeviceNamePromise(napi_env env,
    SetLocalDeviceNameAsyncCallbackInfo *asyncCallback)
{
    LOGI("In");
    napi_value promise = 0;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallback->deferred = deferred;
    napi_value workName;
    napi_create_string_latin1(env, "SetLocalDeviceNamePromise", NAPI_AUTO_LENGTH, &workName);
    napi_create_async_work(env, nullptr, workName,
        [](napi_env env, void *data) {
            SetLocalDeviceNameAsyncCallbackInfo *jsCallback =
                reinterpret_cast<SetLocalDeviceNameAsyncCallbackInfo *>(data);
            std::shared_ptr<DmNapiSetLocalDeviceNameCallback> callback =
                std::make_shared<DmNapiSetLocalDeviceNameCallback>(jsCallback->env, jsCallback->bundleName,
                jsCallback->deferred);
            int32_t ret = DeviceManager::GetInstance().SetLocalDeviceName(jsCallback->bundleName,
                jsCallback->deviceName, callback);
            jsCallback->code = ret;
            if (ret != DM_OK) {
                LOGE("SetLocalDeviceName failed, bundleName:%{public}s, ret=%{public}d",
                    jsCallback->bundleName.c_str(), ret);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            SetLocalDeviceNameAsyncCallbackInfo *jsCallback =
                reinterpret_cast<SetLocalDeviceNameAsyncCallbackInfo *>(data);
            if (jsCallback->code != DM_OK) {
                napi_value error = CreateBusinessError(env, jsCallback->code, false);
                napi_reject_deferred(env, jsCallback->deferred, error);
            }
            napi_delete_async_work(env, jsCallback->asyncWork);
            delete jsCallback;
            jsCallback = nullptr;
        },
        (void *)asyncCallback, &asyncCallback->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallback->asyncWork, napi_qos_user_initiated);
    return promise;
}

napi_value DeviceManagerNapi::SetRemoteDeviceNamePromise(napi_env env,
    SetRemoteDeviceNameAsyncCallbackInfo *asyncCallback)
{
    LOGI("In");
    napi_value promise = 0;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallback->deferred = deferred;
    napi_value workName;
    napi_create_string_latin1(env, "SetRemoteDeviceNamePromise", NAPI_AUTO_LENGTH, &workName);
    napi_create_async_work(env, nullptr, workName,
        [](napi_env env, void *data) {
            SetRemoteDeviceNameAsyncCallbackInfo *jsCallback =
                reinterpret_cast<SetRemoteDeviceNameAsyncCallbackInfo *>(data);
            std::shared_ptr<DmNapiSetRemoteDeviceNameCallback> callback =
                std::make_shared<DmNapiSetRemoteDeviceNameCallback>(jsCallback->env, jsCallback->bundleName,
                jsCallback->deferred);
            int32_t ret = DeviceManager::GetInstance().SetRemoteDeviceName(jsCallback->bundleName,
                jsCallback->deviceId, jsCallback->deviceName, callback);
            jsCallback->code = ret;
            if (ret != DM_OK) {
                LOGE("SetRemoteDeviceName failed, bundleName:%{public}s, ret=%{public}d",
                    jsCallback->bundleName.c_str(), ret);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            SetRemoteDeviceNameAsyncCallbackInfo *jsCallback =
                reinterpret_cast<SetRemoteDeviceNameAsyncCallbackInfo *>(data);
            if (jsCallback->code != DM_OK) {
                napi_value error = CreateBusinessError(env, jsCallback->code, false);
                napi_reject_deferred(env, jsCallback->deferred, error);
            }
            napi_delete_async_work(env, jsCallback->asyncWork);
            delete jsCallback;
            jsCallback = nullptr;
        },
        (void *)asyncCallback, &asyncCallback->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallback->asyncWork, napi_qos_user_initiated);
    return promise;
}

napi_value DeviceManagerNapi::JsSetLocalDeviceName(napi_env env, napi_callback_info info)
{
    LOGI("In");
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        LOGE("deviceManagerWrapper is NULL");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    std::string deviceName = "";
    if (!JsToStringAndCheck(env, argv[0], "deviceName", deviceName)) {
        return nullptr;
    }
    if (deviceName.size() > DEIVCE_NAME_MAX_BYTES) {
        LOGE("deviceName is too long");
        CreateBusinessError(env, ERR_DM_INIT_FAILED);
        return nullptr;
    }
    auto *jsCallback = new SetLocalDeviceNameAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("jsCallback is nullptr");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    jsCallback->env = env;
    jsCallback->bundleName = deviceManagerWrapper->bundleName_;
    jsCallback->deviceName = deviceName;
    return SetLocalDeviceNamePromise(env, jsCallback);
}

napi_value DeviceManagerNapi::JsSetRemoteDeviceName(napi_env env, napi_callback_info info)
{
    LOGI("In");
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        LOGE("deviceManagerWrapper is NULL");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }
    std::string deviceName = "";
    if (!JsToStringAndCheck(env, argv[0], "deviceName", deviceName)) {
        return nullptr;
    }
    if (deviceName.size() > DEIVCE_NAME_MAX_BYTES) {
        LOGE("deviceName is too long");
        CreateBusinessError(env, ERR_DM_INIT_FAILED);
        return nullptr;
    }
    std::string deviceId = "";
    if (!JsToStringAndCheck(env, argv[0], "deviceId", deviceName)) {
        return nullptr;
    }
    auto *jsCallback = new SetRemoteDeviceNameAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("jsCallback is nullptr");
        CreateBusinessError(env, ERR_DM_POINT_NULL);
        return nullptr;
    }

    jsCallback->env = env;
    jsCallback->bundleName = deviceManagerWrapper->bundleName_;
    jsCallback->deviceId = deviceId;
    jsCallback->deviceName = deviceName;
    return SetRemoteDeviceNamePromise(env, jsCallback);
}

napi_value DeviceManagerNapi::JsRestoreLocalDeivceName(napi_env env, napi_callback_info info)
{
    LOGI("In");
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    napi_value result = nullptr;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    int32_t ret = DeviceManager::GetInstance().RestoreLocalDeivceName(deviceManagerWrapper->bundleName_);
    if (ret != 0) {
        LOGE("bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    napi_get_undefined(env, &result);
    return result;
}

void DeviceManagerNapi::ClearBundleCallbacks(std::string &bundleName)
{
    LOGI("ClearBundleCallbacks start for bundleName %{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
        g_deviceManagerMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        g_initCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStatusCallbackMapMutex);
        g_deviceStatusCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        g_DiscoveryCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_publishCallbackMapMutex);
        g_publishCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
        g_authCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_bindCallbackMapMutex);
        g_bindCallbackMap.erase(bundleName);
    }
    return;
}

napi_value DeviceManagerNapi::ReleaseDeviceManager(napi_env env, napi_callback_info info)
{
    LOGI("ReleaseDeviceManager in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc == DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    napi_valuetype argvType = napi_undefined;
    napi_typeof(env, argv[0], &argvType);
    if (!CheckArgsType(env, argvType == napi_object, "DeviceManager", "object")) {
        return nullptr;
    }
    napi_value result = nullptr;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, argv[0], &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    LOGI("ReleaseDeviceManager for bundleName %{public}s", deviceManagerWrapper->bundleName_.c_str());
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(deviceManagerWrapper->bundleName_);
    if (ret != 0) {
        LOGE("ReleaseDeviceManager for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
        napi_create_uint32(env, static_cast<uint32_t>(ret), &result);
        return result;
    }
    ClearBundleCallbacks(deviceManagerWrapper->bundleName_);
    napi_get_undefined(env, &result);
    NAPI_CALL(env, napi_remove_wrap(env, argv[0], (void**)&deviceManagerWrapper));
    return result;
}

napi_value DeviceManagerNapi::CreateDeviceManager(napi_env env, napi_callback_info info)
{
    LOGI("In");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc == DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    std::string bundleName;
    if (!JsToStringAndCheck(env, argv[0], "bundleName", bundleName)) {
        return nullptr;
    }
    std::shared_ptr<DmNapiInitCallback> initCallback = std::make_shared<DmNapiInitCallback>(env, bundleName);
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(bundleName, initCallback);
    if (ret != 0) {
        LOGE("CreateDeviceManager for bundleName %{public}s failed, ret %{public}d.", bundleName.c_str(), ret);
        CreateBusinessError(env, ret);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        g_initCallbackMap[bundleName] = initCallback;
    }
    napi_value ctor = nullptr;
    napi_value napiName = nullptr;
    napi_value result = nullptr;
    napi_get_reference_value(env, sConstructor_, &ctor);
    napi_create_string_utf8(env, bundleName.c_str(), NAPI_AUTO_LENGTH, &napiName);
    napi_status status = napi_new_instance(env, ctor, DM_NAPI_ARGS_ONE, &napiName, &result);
    if (status != napi_ok) {
        LOGE("Create DeviceManagerNapi for bundleName %{public}s failed", bundleName.c_str());
    }
    return result;
}

napi_value DeviceManagerNapi::Constructor(napi_env env, napi_callback_info info)
{
    LOGI("In");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    std::string bundleName;
    if (!JsToStringAndCheck(env, argv[0], "bundleName", bundleName)) {
        return nullptr;
    }

    LOGI("Create for packageName:%{public}s", bundleName.c_str());
    DeviceManagerNapi *obj = new DeviceManagerNapi(env, thisVar);
    if (obj == nullptr) {
        return nullptr;
    }

    obj->bundleName_ = bundleName;
    std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
    g_deviceManagerMap[obj->bundleName_] = obj;
    napi_status status = napi_wrap(
        env, thisVar, reinterpret_cast<void *>(obj),
        [](napi_env env, void *data, void *hint) {
            (void)env;
            (void)hint;
            DeviceManagerNapi *deviceManager = reinterpret_cast<DeviceManagerNapi *>(data);
            delete deviceManager;
            deviceManager = nullptr;
            LOGI("delete deviceManager");
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        delete obj;
        obj = nullptr;
        LOGE("failed to wrap JS object");
        return nullptr;
    }
    return thisVar;
}

napi_value DeviceManagerNapi::Init(napi_env env, napi_value exports)
{
    napi_value dmClass = nullptr;
    napi_property_descriptor dmProperties[] = {
        DECLARE_NAPI_FUNCTION("getAvailableDeviceListSync", GetAvailableDeviceListSync),
        DECLARE_NAPI_FUNCTION("getAvailableDeviceList", GetAvailableDeviceList),
        DECLARE_NAPI_FUNCTION("getLocalDeviceNetworkId", GetLocalDeviceNetworkId),
        DECLARE_NAPI_FUNCTION("getLocalDeviceId", GetLocalDeviceId),
        DECLARE_NAPI_FUNCTION("getLocalDeviceName", GetLocalDeviceName),
        DECLARE_NAPI_FUNCTION("getLocalDeviceType", GetLocalDeviceType),
        DECLARE_NAPI_FUNCTION("getDeviceName", GetDeviceName),
        DECLARE_NAPI_FUNCTION("getDeviceType", GetDeviceType),
        DECLARE_NAPI_FUNCTION("startDiscovering", StartDeviceDiscover),
        DECLARE_NAPI_FUNCTION("stopDiscovering", StopDeviceDiscover),
        DECLARE_NAPI_FUNCTION("unbindTarget", UnBindTarget),
        DECLARE_NAPI_FUNCTION("bindTarget", BindTarget),
        DECLARE_NAPI_FUNCTION("replyUiAction", SetUserOperationSync),
        DECLARE_NAPI_FUNCTION("on", JsOn),
        DECLARE_NAPI_FUNCTION("off", JsOff),
        DECLARE_NAPI_FUNCTION("getDeviceProfileInfoList", JsGetDeviceProfileInfoList),
        DECLARE_NAPI_FUNCTION("getDeviceIconInfo", JsGetDeviceIconInfo),
        DECLARE_NAPI_FUNCTION("putDeviceProfileInfoList", JsPutDeviceProfileInfoList),
        DECLARE_NAPI_FUNCTION("getLocalDisplayDeviceName", JsGetLocalDisplayDeviceName),
        DECLARE_NAPI_FUNCTION("setHeartbeatPolicy", SetHeartbeatPolicy),
        DECLARE_NAPI_FUNCTION("setLocalDeviceName", JsSetLocalDeviceName),
        DECLARE_NAPI_FUNCTION("setRemoteDeviceName", JsSetRemoteDeviceName),
        DECLARE_NAPI_FUNCTION("restoreLocalDeivceName", JsRestoreLocalDeivceName)};

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createDeviceManager", CreateDeviceManager),
        DECLARE_NAPI_STATIC_FUNCTION("releaseDeviceManager", ReleaseDeviceManager),
    };

    LOGI("DeviceManagerNapi::Init() is called!");
    NAPI_CALL(env, napi_define_class(env, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Constructor,
                                     nullptr, sizeof(dmProperties) / sizeof(dmProperties[0]), dmProperties, &dmClass));
    NAPI_CALL(env, napi_create_reference(env, dmClass, 1, &sConstructor_));
    NAPI_CALL(env, napi_set_named_property(env, exports, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), dmClass));
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop));
    LOGI("All props and functions are configured..");
    return exports;
}

napi_value DeviceManagerNapi::EnumTypeConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value res = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &res, nullptr));
    return res;
}

napi_value DeviceManagerNapi::InitDeviceStatusChangeActionEnum(napi_env env, napi_value exports)
{
    napi_value device_state_online;
    napi_value device_state_ready;
    napi_value device_state_offline;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceState::DEVICE_STATE_ONLINE),
        &device_state_online);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceState::DEVICE_INFO_READY),
        &device_state_ready);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceState::DEVICE_STATE_OFFLINE),
        &device_state_offline);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("UNKNOWN", device_state_online),
        DECLARE_NAPI_STATIC_PROPERTY("AVAILABLE", device_state_ready),
        DECLARE_NAPI_STATIC_PROPERTY("UNAVAILABLE", device_state_offline),
    };

    napi_value result = nullptr;
    napi_define_class(env, "DeviceStateChange", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &deviceStateChangeActionEnumConstructor_);
    napi_set_named_property(env, exports, "DeviceStateChange", result);
    return exports;
}

napi_value DeviceManagerNapi::InitStrategyForHeartbeatEnum(napi_env env, napi_value exports)
{
    const uint32_t stop_heartbeat = 100;
    const uint32_t start_heartbeat = 101;

    napi_value start_heartbeat_value;
    napi_value stop_heartbeat_value;
    int32_t refCount = 1;

    napi_create_uint32(env, start_heartbeat, &start_heartbeat_value);
    napi_create_uint32(env, stop_heartbeat, &stop_heartbeat_value);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("START_HEARTBEAT", start_heartbeat_value),
        DECLARE_NAPI_STATIC_PROPERTY("TEMP_STOP_HEARTBEAT", stop_heartbeat_value),
    };

    napi_value result = nullptr;
    napi_define_class(env, "StrategyForHeartbeat", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &g_strategyForHeartbeatEnumConstructor);
    napi_set_named_property(env, exports, "StrategyForHeartbeat", result);
    return exports;
}

int32_t DeviceManagerNapi::BindTargetWarpper(const std::string &pkgName, const std::string &deviceId,
    const std::string &bindParam, std::shared_ptr<DmNapiBindTargetCallback> callback)
{
    if (bindParam.empty()) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    JsonObject bindParamObj(bindParam);
    if (bindParamObj.IsDiscarded()) {
        return ERR_DM_INPUT_PARA_INVALID;
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
    InsertMapParames(bindParamObj, bindParamMap);
    return DeviceManager::GetInstance().BindTarget(pkgName, targetId, bindParamMap, callback);
}

/*
 * Function registering all props and functions of ohos.distributedhardware
 */
static napi_value Export(napi_env env, napi_value exports)
{
    LOGI("Export() is called!");
    DeviceManagerNapi::Init(env, exports);
    DeviceManagerNapi::InitDeviceStatusChangeActionEnum(env, exports);
    DeviceManagerNapi::InitStrategyForHeartbeatEnum(env, exports);
    return exports;
}

/*
 * module define
 */
static napi_module g_dmModule = {.nm_version = 1,
                                 .nm_flags = 0,
                                 .nm_filename = nullptr,
                                 .nm_register_func = Export,
                                 .nm_modname = "distributedDeviceManager",
                                 .nm_priv = ((void *)0),
                                 .reserved = {0}};

/*
 * module register
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    LOGI("RegisterModule() is called!");
    napi_module_register(&g_dmModule);
}

void DmNapiGetDeviceIconInfoCallback::OnResult(const OHOS::DistributedHardware::DmDeviceIconInfo &deviceIconInfo,
    int32_t code)
{
    LOGI("In code:%{public}d", code);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        LOGE("get loop fail");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("OnResult, No memory");
        return;
    }
    auto *jsCallback = new DeviceIconInfoAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("create jsCallback fail");
        DeleteUvWork(work);
        return;
    }
    jsCallback->env = env_;
    jsCallback->bundleName = bundleName_;
    jsCallback->deferred = deferred_;
    jsCallback->deviceIconInfo = deviceIconInfo;
    jsCallback->code = code;
    work->data = reinterpret_cast<void *>(jsCallback);
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
            LOGD("OnResult uv_queue_work_with_qos");
    },  [] (uv_work_t *work, int status) {
        DeviceIconInfoAsyncCallbackInfo *callback =
            reinterpret_cast<DeviceIconInfoAsyncCallbackInfo *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName);
        if (deviceManagerNapi == nullptr) {
            LOGE("deviceManagerNapi not find for bundleName %{public}s", callback->bundleName.c_str());
        } else {
            deviceManagerNapi->OnGetDeviceIconInfoCallbackResult(callback);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnBindResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}

void DmNapiSetLocalDeviceNameCallback::OnResult(int32_t code)
{
    LOGI("In code:%{public}d", code);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        LOGE("get loop fail");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("OnResult, No memory");
        return;
    }
    auto *jsCallback = new SetLocalDeviceNameAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("create jsCallback fail");
        DeleteUvWork(work);
        return;
    }
    jsCallback->env = env_;
    jsCallback->bundleName = bundleName_;
    jsCallback->deferred = deferred_;
    jsCallback->code = code;
    work->data = reinterpret_cast<void *>(jsCallback);
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
            LOGD("OnResult uv_queue_work_with_qos");
    },  [] (uv_work_t *work, int status) {
        SetLocalDeviceNameAsyncCallbackInfo *callback =
            reinterpret_cast<SetLocalDeviceNameAsyncCallbackInfo *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName);
        if (deviceManagerNapi == nullptr) {
            LOGE("deviceManagerNapi not find for bundleName %{public}s", callback->bundleName.c_str());
        } else {
            deviceManagerNapi->OnSetLocalDeviceNameCallbackResult(callback);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnBindResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}

void DmNapiSetRemoteDeviceNameCallback::OnResult(int32_t code)
{
    LOGI("In code:%{public}d", code);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        LOGE("get loop fail");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("OnResult, No memory");
        return;
    }
    auto *jsCallback = new SetRemoteDeviceNameAsyncCallbackInfo();
    if (jsCallback == nullptr) {
        LOGE("create jsCallback fail");
        DeleteUvWork(work);
        return;
    }
    jsCallback->env = env_;
    jsCallback->bundleName = bundleName_;
    jsCallback->deferred = deferred_;
    jsCallback->code = code;
    work->data = reinterpret_cast<void *>(jsCallback);
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
            LOGD("OnResult uv_queue_work_with_qos");
    },  [] (uv_work_t *work, int status) {
        SetRemoteDeviceNameAsyncCallbackInfo *callback =
            reinterpret_cast<SetRemoteDeviceNameAsyncCallbackInfo *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName);
        if (deviceManagerNapi == nullptr) {
            LOGE("deviceManagerNapi not find for bundleName %{public}s", callback->bundleName.c_str());
        } else {
            deviceManagerNapi->OnSetRemoteDeviceNameCallbackResult(callback);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnBindResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        DeleteUvWork(work);
    }
}
