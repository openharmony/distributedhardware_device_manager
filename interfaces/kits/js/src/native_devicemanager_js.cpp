/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <uv.h>
#include "device_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
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
const std::string DM_NAPI_EVENT_DEVICE_FOUND = "deviceFound";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVERY_FAIL = "discoveryFail";
const std::string DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS = "publishSuccess";
const std::string DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL = "publishFail";
const std::string DM_NAPI_EVENT_DEVICE_SERVICE_DIE = "serviceDie";
const std::string DEVICE_MANAGER_NAPI_CLASS_NAME = "DeviceManager";
const std::string DM_NAPI_EVENT_UI_STATE_CHANGE = "uiStateChange";

const int32_t DM_NAPI_ARGS_ZERO = 0;
const int32_t DM_NAPI_ARGS_ONE = 1;
const int32_t DM_NAPI_ARGS_TWO = 2;
const int32_t DM_NAPI_ARGS_THREE = 3;
const int32_t DM_NAPI_SUB_ID_MAX = 65535;
const int32_t DM_AUTH_DIRECTION_CLIENT = 1;
const int32_t DM_AUTH_REQUEST_SUCCESS_STATUS = 7;

const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_DDMP = 0;
const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_OSD = 1;
constexpr int32_t DM_MAX_DEVICE_SIZE = 100;

napi_ref deviceTypeEnumConstructor_ = nullptr;
napi_ref deviceStateChangeActionEnumConstructor_ = nullptr;
napi_ref discoverModeEnumConstructor_ = nullptr;
napi_ref exchangeMediumEnumConstructor_ = nullptr;
napi_ref exchangeFreqEnumConstructor_ = nullptr;
napi_ref subscribeCapEnumConstructor_ = nullptr;

std::map<std::string, DeviceManagerNapi *> g_deviceManagerMap;
std::map<std::string, std::shared_ptr<DmNapiInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceStateCallback>> g_deviceStateCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDiscoveryCallback>> g_DiscoveryCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiPublishCallback>> g_publishCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiAuthenticateCallback>> g_authCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceManagerUiCallback>> g_dmUiCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiCredentialCallback>> g_creCallbackMap;

std::mutex g_deviceManagerMapMutex;
std::mutex g_initCallbackMapMutex;
std::mutex g_deviceStateCallbackMapMutex;
std::mutex g_discoveryCallbackMapMutex;
std::mutex g_publishCallbackMapMutex;
std::mutex g_authCallbackMapMutex;
std::mutex g_dmUiCallbackMapMutex;

enum class DMBussinessErrorCode : int32_t {
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

const std::string ERR_MESSAGE_NO_PERMISSION = "Permission verify failed.";
const std::string ERR_MESSAGE_NOT_SYSTEM_APP = "The caller is not a system application.";
const std::string ERR_MESSAGE_INVALID_PARAMS = "Input parameter error.";
const std::string ERR_MESSAGE_FAILED = "Failed to execute the function.";
const std::string ERR_MESSAGE_OBTAIN_SERVICE = "Failed to obtain the service.";
const std::string ERR_MESSAGE_AUTHENTICALTION_INVALID = "Authentication invalid.";
const std::string ERR_MESSAGE_DISCOVERY_INVALID = "Discovery invalid.";
const std::string ERR_MESSAGE_PUBLISH_INVALID = "Publish invalid.";

napi_value GenerateBusinessError(napi_env env, int32_t err, const std::string &msg)
{
    napi_value businessError = nullptr;
    NAPI_CALL(env, napi_create_object(env, &businessError));
    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, err, &errorCode));
    napi_value errorMessage = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, msg.c_str(), NAPI_AUTO_LENGTH, &errorMessage));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "code", errorCode));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "message", errorMessage));

    return businessError;
}

bool CheckArgsVal(napi_env env, bool assertion, const std::string &param, const std::string &msg)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + "The value of " + param + ": " + msg;
        napi_throw_error(env, std::to_string(
            static_cast<int32_t>(DMBussinessErrorCode::ERR_INVALID_PARAMS)).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

bool CheckArgsCount(napi_env env, bool assertion, const std::string &message)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + message;
        napi_throw_error(env, std::to_string(
            static_cast<int32_t>(DMBussinessErrorCode::ERR_INVALID_PARAMS)).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

bool CheckArgsType(napi_env env, bool assertion, const std::string &paramName, const std::string &type)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + "The type of " + paramName +
                " must be " + type;
        napi_throw_error(env, std::to_string(
            static_cast<int32_t>(DMBussinessErrorCode::ERR_INVALID_PARAMS)).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

napi_value CreateErrorForCall(napi_env env, int32_t code, const std::string &errMsg, bool isAsync = true)
{
    LOGI("CreateErrorForCall code:%{public}d, message:%{public}s", code, errMsg.c_str());
    napi_value error = nullptr;
    if (isAsync) {
        napi_throw_error(env, std::to_string(code).c_str(), errMsg.c_str());
    } else {
        error = GenerateBusinessError(env, code, errMsg);
    }
    return error;
}

napi_value CreateBusinessError(napi_env env, int32_t errCode, bool isAsync = true)
{
    napi_value error = nullptr;
    switch (errCode) {
        case ERR_DM_NO_PERMISSION:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NO_PERMISSION),
                ERR_MESSAGE_NO_PERMISSION, isAsync);
            break;
        case ERR_DM_DISCOVERY_REPEATED:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::DM_ERR_DISCOVERY_INVALID),
                ERR_MESSAGE_DISCOVERY_INVALID, isAsync);
            break;
        case ERR_DM_PUBLISH_REPEATED:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::DM_ERR_PUBLISH_INVALID),
                ERR_MESSAGE_PUBLISH_INVALID, isAsync);
            break;
        case ERR_DM_AUTH_BUSINESS_BUSY:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::DM_ERR_AUTHENTICALTION_INVALID),
                ERR_MESSAGE_AUTHENTICALTION_INVALID, isAsync);
            break;
        case ERR_DM_INPUT_PARA_INVALID:
        case ERR_DM_UNSUPPORTED_AUTH_TYPE:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_INVALID_PARAMS),
                ERR_MESSAGE_INVALID_PARAMS, isAsync);
            break;
        case ERR_DM_INIT_FAILED:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::DM_ERR_OBTAIN_SERVICE),
                ERR_MESSAGE_OBTAIN_SERVICE, isAsync);
            break;
        case ERR_DM_NOT_SYSTEM_APP:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP),
                ERR_MESSAGE_NOT_SYSTEM_APP, isAsync);
            break;
        default:
            error = CreateErrorForCall(env, static_cast<int32_t>(DMBussinessErrorCode::DM_ERR_FAILED),
                ERR_MESSAGE_FAILED, isAsync);
            break;
    }
    return error;
}

void DeleteUvWork(uv_work_t *&work)
{
    if (work == nullptr) {
        return;
    }
    delete work;
    work = nullptr;
    LOGI("delete work!");
}

void DeleteDmNapiStateJsCallbackPtr(DmNapiStateJsCallback *&pJsCallbackPtr)
{
    if (pJsCallbackPtr == nullptr) {
        return;
    }
    delete pJsCallbackPtr;
    pJsCallbackPtr = nullptr;
    LOGI("delete DmNapiStateJsCallback callbackPtr!");
}

void DeleteAsyncCallbackInfo(DeviceInfoListAsyncCallbackInfo *&pAsynCallbackInfo)
{
    if (pAsynCallbackInfo == nullptr) {
        return;
    }
    delete pAsynCallbackInfo;
    pAsynCallbackInfo = nullptr;
}

bool IsJSObjectType(napi_env env, napi_value value, const std::string &param)
{
    napi_valuetype authparamType = napi_undefined;
    napi_typeof(env, value, &authparamType);
    return CheckArgsType(env, authparamType == napi_object, param, "object");
}

bool IsFunctionType(napi_env env, napi_value value)
{
    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, value, &eventHandleType);
    return CheckArgsType(env, eventHandleType == napi_function, "callback", "function");
}

bool IsDeviceManagerNapiNull(napi_env env, napi_value thisVar, DeviceManagerNapi **pDeviceManagerWrapper)
{
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(pDeviceManagerWrapper));
    if (pDeviceManagerWrapper != nullptr && *pDeviceManagerWrapper != nullptr) {
        return false;
    }
    CreateBusinessError(env, ERR_DM_POINT_NULL);
    LOGE(" DeviceManagerNapi object is nullptr!");
    return true;
}
} // namespace

thread_local napi_ref DeviceManagerNapi::sConstructor_ = nullptr;
AuthAsyncCallbackInfo DeviceManagerNapi::authAsyncCallbackInfo_;
CredentialAsyncCallbackInfo DeviceManagerNapi::creAsyncCallbackInfo_;
std::mutex DeviceManagerNapi::creMapLocks_;

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

    DmDeviceInfo info;
    DmNapiStateJsCallback *jsCallback = new DmNapiStateJsCallback(bundleName_, 0, 0, info);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnRemoteDied uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnRemoteDied, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnEvent("serviceDie", 0, nullptr);
        }
        LOGI("OnRemoteDied, deviceManagerNapi bundleName %{public}s", callback->bundleName_.c_str());
        DeleteDmNapiStateJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnRemoteDied work queue");
        DeleteDmNapiStateJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStateCallback::OnDeviceOnline(const DmDeviceInfo &deviceInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceStateCallback: OnDeviceOnline, No memory");
        return;
    }

    DmNapiStateJsCallback *jsCallback = new DmNapiStateJsCallback(bundleName_, 0, 0, deviceInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceOnline uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceOnline, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::ONLINE, callback->deviceInfo_);
        }
        DeleteDmNapiStateJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceOnline work queue");
        DeleteDmNapiStateJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStateCallback::OnDeviceReady(const DmDeviceInfo &deviceInfo)
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

    DmNapiStateJsCallback *jsCallback = new DmNapiStateJsCallback(bundleName_, 0, 0, deviceInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceReady uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceReady, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::READY, callback->deviceInfo_);
        }
        DeleteDmNapiStateJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceReady work queue");
        DeleteDmNapiStateJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStateCallback::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceStateCallback: OnDeviceOffline, No memory");
        return;
    }

    DmNapiStateJsCallback *jsCallback = new DmNapiStateJsCallback(bundleName_, 0, 0, deviceInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceOffline uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceOffline, deviceManagerNapi not find for bundleName %{public}s",
                callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::OFFLINE, callback->deviceInfo_);
        }
        DeleteDmNapiStateJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceOffline work queue");
        DeleteDmNapiStateJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDeviceStateCallback::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiDeviceStateCallback: OnDeviceChanged, No memory");
        return;
    }

    DmNapiStateJsCallback *jsCallback = new DmNapiStateJsCallback(bundleName_, 0, 0, deviceInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceChanged uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceChanged, deviceManagerNapi not find for bundleName %{public}s",
                callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::CHANGE, callback->deviceInfo_);
        }
        DeleteDmNapiStateJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceChanged work queue");
        DeleteDmNapiStateJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDiscoveryCallback::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
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

    DmNapiStateJsCallback *jsCallback = new DmNapiStateJsCallback(bundleName_, subscribeId, 0, deviceInfo);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDeviceFound uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceFound, deviceManagerNapi not find for bundleName %{public}s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceFound(callback->subscribeId_, callback->deviceInfo_);
        }
        DeleteDmNapiStateJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceFound work queue");
        DeleteDmNapiStateJsCallbackPtr(jsCallback);
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

    DmDeviceInfo info;
    DmNapiStateJsCallback *jsCallback = new DmNapiStateJsCallback(bundleName_, subscribeId,
        failedReason, info);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnDiscoveryFailed uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDiscoveryFailed, deviceManagerNapi not find for bundleName %{public}s",
                callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDiscoveryFailed(callback->subscribeId_, callback->reason_);
        }
        DeleteDmNapiStateJsCallbackPtr(callback);
        DeleteUvWork(work);
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnDiscoveryFailed work queue");
        DeleteDmNapiStateJsCallbackPtr(jsCallback);
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
        LOGE("js DmNapiAuthenticateCallback::OnAuthResult, No memory");
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

void DmNapiCredentialCallback::OnCredentialResult(int32_t &action, const std::string &credentialResult)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("js DmNapiAuthenticateCallback::OnAuthResult, No memory");
        return;
    }

    DmNapiCredentialJsCallback *jsCallback = new DmNapiCredentialJsCallback(bundleName_, action, credentialResult);
    if (jsCallback == nullptr) {
        delete work;
        work = nullptr;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {
        LOGD("OnCredentialResult uv_queue_work_with_qos");
    }, [] (uv_work_t *work, int status) {
        DmNapiCredentialJsCallback *callback = reinterpret_cast<DmNapiCredentialJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnCredentialResult, deviceManagerNapi not find for bundleName %{public}s",
                callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnCredentialResult(callback->action_, callback->credentialResult_);
        }
        delete callback;
        callback = nullptr;
        delete work;
        work = nullptr;
    }, uv_qos_user_initiated);
    if (ret != 0) {
        LOGE("Failed to execute OnCredentialResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        delete work;
        work = nullptr;
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

void DeviceManagerNapi::OnDeviceStateChange(DmNapiDevStateChangeAction action,
                                            const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo)
{
    napi_handle_scope scope;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "action", (int)action, result);

    napi_value device = nullptr;
    napi_create_object(env_, &device);
    SetValueUtf8String(env_, "deviceId", deviceInfo.deviceId, device);
    SetValueUtf8String(env_, "networkId", deviceInfo.networkId, device);
    SetValueUtf8String(env_, "deviceName", deviceInfo.deviceName, device);
    SetValueInt32(env_, "deviceType", (int)deviceInfo.deviceTypeId, device);
    SetValueInt32(env_, "authForm", (int)deviceInfo.authForm, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("deviceStateChange", DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceFound for subscribeId %{public}d, range : %{public}d", (int32_t)subscribeId, deviceInfo.range);
    napi_handle_scope scope;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "subscribeId", (int)subscribeId, result);

    napi_value device = nullptr;
    napi_create_object(env_, &device);
    SetValueUtf8String(env_, "deviceId", deviceInfo.deviceId, device);
    SetValueUtf8String(env_, "networkId", deviceInfo.networkId, device);
    SetValueUtf8String(env_, "deviceName", deviceInfo.deviceName, device);
    SetValueInt32(env_, "deviceType", (int)deviceInfo.deviceTypeId, device);
    SetValueInt32(env_, "authForm", (int)deviceInfo.authForm, device);
    SetValueInt32(env_, "range", deviceInfo.range, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("deviceFound", DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailed for subscribeId %{public}d", (int32_t)subscribeId);
    napi_handle_scope scope;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "subscribeId", (int)subscribeId, result);
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
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }
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

void DeviceManagerNapi::OnCredentialResult(int32_t &action, const std::string &credentialResult)
{
    LOGI("OnCredentialResult for action: %{public}d", action);
    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueUtf8String(env_, "resultInfo", credentialResult, result);

    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_get_reference_value(env_, creAsyncCallbackInfo_.callback, &handler);
    if (handler != nullptr) {
        napi_call_function(env_, nullptr, handler, DM_NAPI_ARGS_ONE, &result, &callResult);
        napi_delete_reference(env_, creAsyncCallbackInfo_.callback);
        creAsyncCallbackInfo_.callback = nullptr;
    } else {
        LOGE("handler is nullptr");
    }
    napi_close_handle_scope(env_, scope);
    DeviceManager::GetInstance().UnRegisterCredentialCallback(bundleName_);
    {
        std::lock_guard<std::mutex> autoLock(creMapLocks_);
        g_creCallbackMap.erase(bundleName_);
    }
}

void DeviceManagerNapi::OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
                                     int32_t reason)
{
    LOGI("OnAuthResult for status: %{public}d, reason: %{public}d", status, reason);
    napi_handle_scope scope;
    napi_status openHandleStatus = napi_open_handle_scope(env_, &scope);
    if (openHandleStatus != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }
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

    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_get_reference_value(env_, authAsyncCallbackInfo_.callback, &handler);
    if (handler != nullptr) {
        if (reason == DM_OK && (status <= STATUS_DM_CLOSE_PIN_INPUT_UI && status >= STATUS_DM_SHOW_AUTHORIZE_UI)) {
            LOGI("update ui change, status: %{public}d, reason: %{public}d", status, reason);
        } else {
            napi_call_function(env_, nullptr, handler, DM_NAPI_ARGS_TWO, &result[0], &callResult);
            napi_delete_reference(env_, authAsyncCallbackInfo_.callback);
            authAsyncCallbackInfo_.callback = nullptr;
        }
    } else {
        LOGE("handler is nullptr");
    }
    napi_close_handle_scope(env_, scope);
    std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
    g_authCallbackMap.erase(bundleName_);
}

void DeviceManagerNapi::SetValueUtf8String(const napi_env &env, const std::string &fieldStr, const std::string &str,
                                           napi_value &result)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

void DeviceManagerNapi::SetValueInt32(const napi_env &env, const std::string &fieldStr, const int32_t intValue,
                                      napi_value &result)
{
    napi_value value = nullptr;
    napi_create_int32(env, intValue, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

void DeviceManagerNapi::DeviceInfoToJsArray(const napi_env &env, const std::vector<DmDeviceInfo> &vecDevInfo,
                                            const int32_t idx, napi_value &arrayResult)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    SetValueUtf8String(env, "deviceId", vecDevInfo[idx].deviceId, result);
    SetValueUtf8String(env, "networkId", vecDevInfo[idx].networkId, result);
    SetValueUtf8String(env, "deviceName", vecDevInfo[idx].deviceName, result);
    SetValueInt32(env, "deviceType", (int)vecDevInfo[idx].deviceTypeId, result);
    SetValueInt32(env, "authForm", (int)vecDevInfo[idx].authForm, result);

    napi_status status = napi_set_element(env, arrayResult, idx, result);
    if (status != napi_ok) {
        LOGE("DmDeviceInfo To JsArray set element error: %{public}d", status);
    }
}

bool DeviceManagerNapi::DmAuthParamDetection(const DmAuthParam &authParam)
{
    LOGI("DeviceManagerNapi::DmAuthParamDetection");
    const uint32_t maxIntValueLen = 10;
    const std::string maxAuthToken = "2147483647";
    if (authParam.authToken.length() > maxIntValueLen) {
        LOGE("The authToken is illegal");
        return false;
    } else {
        if (!IsNumberString(authParam.authToken)) {
            LOGE("The authToken is Error");
            return false;
        } else {
            if (authParam.authToken > maxAuthToken) {
                LOGE("The authToken is Cross the border");
                return false;
            }
        }
    }
    return true;
}

void DeviceManagerNapi::DmAuthParamToJsAuthParam(const napi_env &env, const DmAuthParam &authParam,
                                                 napi_value &paramResult)
{
    LOGI("DeviceManagerNapi::DmAuthParamToJsAuthParam");
    if (!DmAuthParamDetection(authParam)) {
        LOGE("The authToken is Error");
        return;
    }
    napi_value extraInfo = nullptr;
    napi_create_object(env, &extraInfo);
    SetValueInt32(env, "direction", authParam.direction, extraInfo);
    SetValueInt32(env, "authType", authParam.authType, paramResult);
    SetValueInt32(env, "pinToken", atoi(authParam.authToken.c_str()), extraInfo);

    if (authParam.direction == DM_AUTH_DIRECTION_CLIENT) {
        napi_set_named_property(env, paramResult, "extraInfo", extraInfo);
        return;
    }

    SetValueUtf8String(env, "packageName", authParam.packageName, extraInfo);
    SetValueUtf8String(env, "appName", authParam.appName, extraInfo);
    SetValueUtf8String(env, "appDescription", authParam.appDescription, extraInfo);
    SetValueInt32(env, "business", authParam.business, extraInfo);
    SetValueInt32(env, "pinCode", authParam.pincode, extraInfo);
    napi_set_named_property(env, paramResult, "extraInfo", extraInfo);

    size_t appIconLen = static_cast<size_t>(authParam.imageinfo.GetAppIconLen());
    if (appIconLen > 0) {
        void *appIcon = nullptr;
        napi_value appIconBuffer = nullptr;
        napi_create_arraybuffer(env, appIconLen, &appIcon, &appIconBuffer);
        if (appIcon != nullptr &&
            memcpy_s(appIcon, appIconLen, reinterpret_cast<const void *>(authParam.imageinfo.GetAppIcon()),
                     appIconLen) == 0) {
            napi_value appIconArray = nullptr;
            napi_create_typedarray(env, napi_uint8_array, appIconLen, appIconBuffer, 0, &appIconArray);
            napi_set_named_property(env, paramResult, "appIcon", appIconArray);
        }
    }

    size_t appThumbnailLen = static_cast<size_t>(authParam.imageinfo.GetAppThumbnailLen());
    if (appThumbnailLen > 0) {
        void *appThumbnail = nullptr;
        napi_value appThumbnailBuffer = nullptr;
        napi_create_arraybuffer(env, appThumbnailLen, &appThumbnail, &appThumbnailBuffer);
        if (appThumbnail != nullptr &&
            memcpy_s(appThumbnail, appThumbnailLen,
                     reinterpret_cast<const void *>(authParam.imageinfo.GetAppThumbnail()), appThumbnailLen) == 0) {
            napi_value appThumbnailArray = nullptr;
            napi_create_typedarray(env, napi_uint8_array, appThumbnailLen, appThumbnailBuffer, 0, &appThumbnailArray);
            napi_set_named_property(env, paramResult, "appThumbnail", appThumbnailArray);
        }
    }
    return;
}

void DeviceManagerNapi::JsObjectToString(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                                         char *dest, const int32_t destLen)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        if (!CheckArgsType(env, valueType == napi_string, fieldStr.c_str(), "string")) {
            return;
        }
        size_t result = 0;
        NAPI_CALL_RETURN_VOID(env, napi_get_value_string_utf8(env, field, dest, destLen, &result));
    } else {
        LOGE("devicemanager napi js to str no property: %{public}s", fieldStr.c_str());
    }
}

std::string DeviceManagerNapi::JsObjectToString(const napi_env &env, const napi_value &param)
{
    LOGI("JsObjectToString in.");
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return "";
    }
    if (size == 0) {
        return "";
    }
    char *buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return "";
    }
    int32_t ret = memset_s(buf, (size + 1), 0, (size + 1));
    if (ret != 0) {
        LOGE("devicemanager memset_s error.");
        delete[] buf;
        buf = nullptr;
        return "";
    }
    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;

    std::string value;
    if (rev) {
        value = buf;
    } else {
        value = "";
    }
    delete[] buf;
    buf = nullptr;
    return value;
}

void DeviceManagerNapi::JsObjectToInt(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                                      int32_t &fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        if (!CheckArgsType(env, valueType == napi_number, fieldStr.c_str(), "number")) {
            return;
        }
        napi_get_value_int32(env, field, &fieldRef);
    } else {
        LOGE("devicemanager napi js to int no property: %{public}s", fieldStr.c_str());
    }
}

void DeviceManagerNapi::JsObjectToBool(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                                       bool &fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        if (!CheckArgsType(env, valueType == napi_boolean, fieldStr.c_str(), "bool")) {
            return;
        }
        napi_get_value_bool(env, field, &fieldRef);
    } else {
        LOGE("devicemanager napi js to bool no property: %{public}s", fieldStr.c_str());
    }
}

void DeviceManagerNapi::JsToDmPublishInfo(const napi_env &env, const napi_value &object, DmPublishInfo &info)
{
    int32_t publishId = -1;
    JsObjectToInt(env, object, "publishId", publishId);
    info.publishId = publishId;

    int32_t mode = -1;
    JsObjectToInt(env, object, "mode", mode);
    info.mode = static_cast<DmDiscoverMode>(mode);

    int32_t freq = -1;
    JsObjectToInt(env, object, "freq", freq);
    info.freq = static_cast<DmExchangeFreq>(freq);

    JsObjectToBool(env, object, "ranging", info.ranging);
    return;
}

int32_t DeviceManagerNapi::JsToDmSubscribeInfo(const napi_env &env, const napi_value &object, DmSubscribeInfo &info)
{
    int32_t subscribeId = -1;
    JsObjectToInt(env, object, "subscribeId", subscribeId);
    if (subscribeId < 0 || subscribeId > DM_NAPI_SUB_ID_MAX) {
        LOGE("DeviceManagerNapi::JsToDmSubscribeInfo, subscribeId error, subscribeId: %{public}d ", subscribeId);
        return -1;
    }

    info.subscribeId = static_cast<uint16_t>(subscribeId);

    int32_t mode = -1;
    JsObjectToInt(env, object, "mode", mode);
    info.mode = static_cast<DmDiscoverMode>(mode);

    int32_t medium = -1;
    JsObjectToInt(env, object, "medium", medium);
    info.medium = static_cast<DmExchangeMedium>(medium);

    int32_t freq = -1;
    JsObjectToInt(env, object, "freq", freq);
    info.freq = static_cast<DmExchangeFreq>(freq);

    JsObjectToBool(env, object, "isSameAccount", info.isSameAccount);
    JsObjectToBool(env, object, "isWakeRemote", info.isWakeRemote);

    int32_t capability = -1;
    JsObjectToInt(env, object, "capability", capability);
    if (capability == DM_NAPI_SUBSCRIBE_CAPABILITY_DDMP || capability == DM_NAPI_SUBSCRIBE_CAPABILITY_OSD) {
        (void)strncpy_s(info.capability, sizeof(info.capability), DM_CAPABILITY_OSD, strlen(DM_CAPABILITY_OSD));
    }
    return 0;
}

void DeviceManagerNapi::JsToDmDeviceInfo(const napi_env &env, const napi_value &object, DmDeviceInfo &info)
{
    JsObjectToString(env, object, "deviceId", info.deviceId, sizeof(info.deviceId));
    JsObjectToString(env, object, "deviceName", info.deviceName, sizeof(info.deviceName));
    JsObjectToString(env, object, "networkId", info.networkId, sizeof(info.networkId));
    int32_t deviceType = -1;
    JsObjectToInt(env, object, "deviceType", deviceType);
    info.deviceTypeId = static_cast<DmDeviceType>(deviceType);
    JsObjectToInt(env, object, "range", info.range);
}

void DeviceManagerNapi::JsToDmExtra(const napi_env &env, const napi_value &object, std::string &extra,
                                    int32_t &authType)
{
    LOGI("JsToDmExtra in.");
    int32_t authTypeTemp = -1;
    JsObjectToInt(env, object, "authType", authTypeTemp);
    authType = authTypeTemp;

    char appOperation[DM_NAPI_DESCRIPTION_BUF_LENGTH] = "";
    JsObjectToString(env, object, "appOperation", appOperation, sizeof(appOperation));
    std::string appOperationStr = appOperation;

    char customDescription[DM_NAPI_DESCRIPTION_BUF_LENGTH] = "";
    JsObjectToString(env, object, "customDescription", customDescription, sizeof(customDescription));
    std::string customDescriptionStr = customDescription;

    int32_t bindLevel = 0;
    JsObjectToInt(env, object, "bindLevel", bindLevel);

    JsonObject jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    jsonObj[APP_OPERATION] = appOperationStr;
    jsonObj[CUSTOM_DESCRIPTION] = customDescriptionStr;
    jsonObj[BIND_LEVEL] = bindLevel;
    JsToJsonObject(env, object, "extraInfo", jsonObj);
    extra = SafetyDump(jsonObj);
    LOGI("appOperationLen %{public}zu, customDescriptionLen %{public}zu", appOperationStr.size(),
        customDescriptionStr.size());
}

void DeviceManagerNapi::JsToJsonObject(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                                       JsonObject &jsonObj)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (!hasProperty) {
        LOGE("devicemanager napi js to str no property: %{public}s", fieldStr.c_str());
        return;
    }

    napi_value jsonField = nullptr;
    napi_get_named_property(env, object, fieldStr.c_str(), &jsonField);
    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    napi_get_property_names(env, jsonField, &jsProNameList);
    napi_get_array_length(env, jsProNameList, &jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        napi_get_element(env, jsProNameList, index, &jsProName);
        std::string strProName = JsObjectToString(env, jsProName);
        napi_get_named_property(env, jsonField, strProName.c_str(), &jsProValue);
        napi_typeof(env, jsProValue, &jsValueType);
        int32_t numberValue = 0;
        bool boolValue = false;
        std::string stringValue = "";
        switch (jsValueType) {
            case napi_string:
                stringValue = JsObjectToString(env, jsProValue);
                LOGI("Property name = %{public}s, string, value = %{public}s", strProName.c_str(), stringValue.c_str());
                jsonObj[strProName] = stringValue;
                break;
            case napi_boolean:
                napi_get_value_bool(env, jsProValue, &boolValue);
                LOGI("Property name = %{public}s, boolean, value = %{public}d.", strProName.c_str(), boolValue);
                jsonObj[strProName] = boolValue;
                break;
            case napi_number:
                if (napi_get_value_int32(env, jsProValue, &numberValue) != napi_ok) {
                    LOGE("Property name = %{public}s, Property int32_t parse error", strProName.c_str());
                } else {
                    jsonObj[strProName] = numberValue;
                    LOGI("Property name = %{public}s, number, value = %{public}d.", strProName.c_str(), numberValue);
                }
                break;
            default:
                LOGE("Property name = %{public}s, value type not support.", strProName.c_str());
                break;
        }
    }
}

void DeviceManagerNapi::JsToDmAuthInfo(const napi_env &env, const napi_value &object, std::string &extra)
{
    LOGI("%{public}s called.", __func__);
    int32_t authType = -1;
    int32_t token = -1;

    JsObjectToInt(env, object, "authType", authType);
    JsObjectToInt(env, object, "token", token);
    JsonObject jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    jsonObj[PIN_TOKEN] = token;
    JsToJsonObject(env, object, "extraInfo", jsonObj);
    extra = SafetyDump(jsonObj);
}

void DeviceManagerNapi::JsToDmDiscoveryExtra(const napi_env &env, const napi_value &object, std::string &extra)
{
    napi_valuetype valueType1 = napi_undefined;
    napi_typeof(env, object, &valueType1);
    if (valueType1 == napi_undefined) {
        extra = "";
        return;
    }
    char filterOption[DM_NAPI_BUF_LENGTH] = {0};
    size_t typeLen = 0;
    NAPI_CALL_RETURN_VOID(env, napi_get_value_string_utf8(env, object, nullptr, 0, &typeLen));
    if (!CheckArgsVal(env, typeLen > 0, "extra", "typeLen == 0")) {
        return;
    }

    if (!CheckArgsVal(env, typeLen < DM_NAPI_BUF_LENGTH, "extra", "typeLen >= BUF_MAX_LENGTH")) {
        return;
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_value_string_utf8(env, object, filterOption, typeLen + 1, &typeLen));
    extra = filterOption;
    LOGI("JsToDmDiscoveryExtra, typeLen : %{public}zu", typeLen);
}

bool DeviceManagerNapi::IsSystemApp()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

void DeviceManagerNapi::DmDeviceInfotoJsDeviceInfo(const napi_env &env, const DmDeviceInfo &vecDevInfo,
                                                   napi_value &result)
{
    napi_create_object(env, &result);

    SetValueUtf8String(env, "deviceId", vecDevInfo.deviceId, result);
    SetValueUtf8String(env, "networkId", vecDevInfo.networkId, result);
    SetValueUtf8String(env, "deviceName", vecDevInfo.deviceName, result);
    SetValueInt32(env, "deviceType", (int)vecDevInfo.deviceTypeId, result);
}

void DeviceManagerNapi::RegisterDevStateCallback(napi_env env, std::string &bundleName)
{
    LOGI("RegisterDevStateCallback start bundleName %{public}s", bundleName.c_str());
    auto callback = std::make_shared<DmNapiDeviceStateCallback>(env, bundleName);
    std::string extra = "";
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(bundleName, extra, callback);
    if (ret != 0) {
        LOGE("RegisterDevStateCallback failed for bundleName %{public}s", bundleName.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateCallbackMapMutex);
        if (g_deviceStateCallbackMap.size() >= MAX_CONTAINER_SIZE) {
            LOGE("g_deviceStateCallbackMap map size is more than max size");
            return;
        }
        g_deviceStateCallbackMap[bundleName] = callback;
    }
    return;
}

void DeviceManagerNapi::CreateDmCallback(napi_env env, std::string &bundleName, std::string &eventType)
{
    LOGI("CreateDmCallback for bundleName %{public}s eventType %{public}s", bundleName.c_str(), eventType.c_str());
    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE) {
        RegisterDevStateCallback(env, bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_FOUND || eventType == DM_NAPI_EVENT_DEVICE_DISCOVERY_FAIL) {
        auto callback = std::make_shared<DmNapiDiscoveryCallback>(env, bundleName);
        {
            std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
            if (g_DiscoveryCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_DiscoveryCallbackMap map size is more than max size");
                return;
            }
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
            if (g_publishCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_publishCallbackMap map size is more than max size");
                return;
            }
            g_publishCallbackMap[bundleName] = callback;
        }
        std::shared_ptr<DmNapiPublishCallback> publishCallback = callback;
        publishCallback->IncreaseRefCount();
        return;
    }

    if (eventType == DM_NAPI_EVENT_UI_STATE_CHANGE) {
        auto callback = std::make_shared<DmNapiDeviceManagerUiCallback>(env, bundleName);
        if (DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(bundleName, callback) != 0) {
            LOGE("RegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName.c_str());
            return;
        }
        {
            std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
            if (g_dmUiCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_dmUiCallbackMap map size is more than max size");
                return;
            }
            g_dmUiCallbackMap[bundleName] = callback;
        }
    }
}

void DeviceManagerNapi::CreateDmCallback(napi_env env, std::string &bundleName,
                                         std::string &eventType, std::string &extra)
{
    LOGI("CreateDmCallback for bundleName %{public}s eventType %{public}s",
         bundleName.c_str(), eventType.c_str());
    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE) {
        auto callback = std::make_shared<DmNapiDeviceStateCallback>(env, bundleName);
        int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(bundleName, extra, callback);
        if (ret != 0) {
            LOGE("RegisterDevStateCallback failed for bundleName %{public}s", bundleName.c_str());
            return;
        }
        {
            std::lock_guard<std::mutex> autoLock(g_deviceStateCallbackMapMutex);
            if (g_deviceStateCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_deviceStateCallbackMap map size is more than max size");
                return;
            }
            g_deviceStateCallbackMap[bundleName] = callback;
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
            std::lock_guard<std::mutex> autoLock(g_deviceStateCallbackMapMutex);
            auto iter = g_deviceStateCallbackMap.find(bundleName);
            if (iter == g_deviceStateCallbackMap.end()) {
                LOGE("ReleaseDmCallback: cannot find stateCallback for bundleName %{public}s", bundleName.c_str());
                return;
            }
        }
        int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(bundleName);
        if (ret != 0) {
            LOGE("UnRegisterDevStateCallback failed for bundleName %{public}s", bundleName.c_str());
            return;
        }
        {
            std::lock_guard<std::mutex> autoLock(g_deviceStateCallbackMapMutex);
            g_deviceStateCallbackMap.erase(bundleName);
        }
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_FOUND || eventType == DM_NAPI_EVENT_DEVICE_DISCOVERY_FAIL) {
        ReleaseDiscoveryCallback(bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL) {
        ReleasePublishCallback(bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_UI_STATE_CHANGE) {
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
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        g_dmUiCallbackMap.erase(bundleName);
        return;
    }
}

napi_value DeviceManagerNapi::SetUserOperationSync(napi_env env, napi_callback_info info)
{
    LOGI("SetUserOperationSync in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_number, "action", "number")) {
        return nullptr;
    }

    napi_valuetype strType;
    napi_typeof(env, argv[1], &strType);
    NAPI_ASSERT(env, strType == napi_string, "Wrong argument type, string expected.");

    int32_t action = 0;
    napi_get_value_int32(env, argv[0], &action);

    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &typeLen);
    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    NAPI_ASSERT(env, typeLen < DM_NAPI_BUF_LENGTH, "typeLen >= MAXLEN");
    char type[DM_NAPI_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[1], type, typeLen + 1, &typeLen);

    std::string params = type;
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

void DeviceManagerNapi::CallGetTrustedDeviceListStatusSync(napi_env env, napi_status &status,
    DeviceInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo)
{
    CHECK_NULL_VOID(deviceInfoListAsyncCallbackInfo);
    if (deviceInfoListAsyncCallbackInfo->devList.size() > DM_MAX_DEVICE_SIZE) {
        LOGE("CallGetTrustedDeviceListStatusSync invalid devList size");
        return;
    }
    for (unsigned int i = 0; i < deviceInfoListAsyncCallbackInfo->devList.size(); i++) {
        LOGI("DeviceManager::GetTrustedDeviceList deviceId:%{public}s deviceName:%{public}s deviceTypeId:%{public}d ",
             GetAnonyString(deviceInfoListAsyncCallbackInfo->devList[i].deviceId).c_str(),
             GetAnonyString(deviceInfoListAsyncCallbackInfo->devList[i].deviceName).c_str(),
             deviceInfoListAsyncCallbackInfo->devList[i].deviceTypeId);
    }

    napi_value array[DM_NAPI_ARGS_TWO] = {0};
    if (deviceInfoListAsyncCallbackInfo->status == 0) {
        bool isArray = false;
        napi_create_array(env, &array[1]);
        napi_is_array(env, array[1], &isArray);
        if (!isArray) {
            LOGE("napi_create_array fail");
        }
        if (deviceInfoListAsyncCallbackInfo->devList.size() > 0) {
            for (unsigned int i = 0; i != deviceInfoListAsyncCallbackInfo->devList.size(); ++i) {
                DeviceInfoToJsArray(env, deviceInfoListAsyncCallbackInfo->devList, (int32_t)i, array[1]);
            }
            LOGI("devList is OK");
        } else {
            LOGE("devList is null");
        }
        napi_resolve_deferred(env, deviceInfoListAsyncCallbackInfo->deferred, array[1]);
    } else {
        array[0] = CreateBusinessError(env, deviceInfoListAsyncCallbackInfo->ret, false);
        napi_reject_deferred(env, deviceInfoListAsyncCallbackInfo->deferred, array[0]);
    }
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
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }
    napi_value result;
    napi_create_object(env_, &result);
    SetValueUtf8String(env_, "param", paramJson, result);
    OnEvent(DM_NAPI_EVENT_UI_STATE_CHANGE, DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::CallGetTrustedDeviceListStatus(napi_env env, napi_status &status,
                                                       DeviceInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo)
{
    CHECK_NULL_VOID(deviceInfoListAsyncCallbackInfo);
    if (deviceInfoListAsyncCallbackInfo->devList.size() > DM_MAX_DEVICE_SIZE) {
        LOGE("CallGetTrustedDeviceListStatus invalid devList size");
        return;
    }
    for (unsigned int i = 0; i < deviceInfoListAsyncCallbackInfo->devList.size(); i++) {
        LOGI("DeviceManager::GetTrustedDeviceList deviceId:%{public}s deviceName:%{public}s deviceTypeId:%{public}d ",
             GetAnonyString(deviceInfoListAsyncCallbackInfo->devList[i].deviceId).c_str(),
             GetAnonyString(deviceInfoListAsyncCallbackInfo->devList[i].deviceName).c_str(),
             deviceInfoListAsyncCallbackInfo->devList[i].deviceTypeId);
    }
    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_value array[DM_NAPI_ARGS_TWO] = {0};

    if (deviceInfoListAsyncCallbackInfo->status == 0) {
        if (deviceInfoListAsyncCallbackInfo->devList.size() > 0) {
            bool isArray = false;
            NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &array[1]));
            NAPI_CALL_RETURN_VOID(env, napi_is_array(env, array[1], &isArray));
            if (!isArray) {
                LOGE("napi_create_array fail");
            }
            for (size_t i = 0; i != deviceInfoListAsyncCallbackInfo->devList.size(); ++i) {
                DeviceInfoToJsArray(env, deviceInfoListAsyncCallbackInfo->devList, i, array[1]);
            }
            LOGI("devList is OK");
        } else {
            LOGE("devList is null");
        }
    } else {
        array[0] = CreateBusinessError(env, deviceInfoListAsyncCallbackInfo->ret, false);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, deviceInfoListAsyncCallbackInfo->callback, &handler));
    if (handler != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, handler, DM_NAPI_ARGS_TWO, &array[0], &callResult));
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, deviceInfoListAsyncCallbackInfo->callback));
        deviceInfoListAsyncCallbackInfo->callback = nullptr;
    } else {
        LOGE("handler is nullptr");
    }
}

void DeviceManagerNapi::CallRequestCreInfoStatus(napi_env env, napi_status &status,
                                                 CredentialAsyncCallbackInfo *creAsyncCallbackInfo)
{
    LOGI("DeviceManager::RequestCredential");
    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_value result = nullptr;
    napi_create_object(env, &result);

    if (creAsyncCallbackInfo->status == 0) {
        if (creAsyncCallbackInfo->returnJsonStr == "") {
            LOGE("creAsyncCallbackInfo returnJsonStr is null");
        }
        SetValueUtf8String(env, "registerInfo", creAsyncCallbackInfo->returnJsonStr, result);
    } else {
        result = CreateBusinessError(env, creAsyncCallbackInfo->ret, false);
    }

    napi_get_reference_value(env, creAsyncCallbackInfo->callback, &handler);
    if (handler != nullptr) {
        napi_call_function(env, nullptr, handler, DM_NAPI_ARGS_ONE, &result, &callResult);
        napi_delete_reference(env, creAsyncCallbackInfo->callback);
        creAsyncCallbackInfo->callback = nullptr;
    } else {
        LOGE("handler is nullptr");
    }
}

void DeviceManagerNapi::CallGetLocalDeviceInfoSync(napi_env env, napi_status &status,
                                                   DeviceInfoAsyncCallbackInfo *deviceInfoAsyncCallbackInfo)
{
    napi_value result[DM_NAPI_ARGS_TWO] = {0};

    LOGI("DeviceManager::CallGetLocalDeviceInfoSync deviceId:%{public}s deviceName:%{public}s deviceTypeId:%{public}d ",
         GetAnonyString(deviceInfoAsyncCallbackInfo->deviceInfo.deviceId).c_str(),
         GetAnonyString(deviceInfoAsyncCallbackInfo->deviceInfo.deviceName).c_str(),
         deviceInfoAsyncCallbackInfo->deviceInfo.deviceTypeId);

    if (deviceInfoAsyncCallbackInfo->status == 0) {
        DmDeviceInfotoJsDeviceInfo(env, deviceInfoAsyncCallbackInfo->deviceInfo, result[1]);
        napi_resolve_deferred(env, deviceInfoAsyncCallbackInfo->deferred, result[1]);
    } else {
        result[0] = CreateBusinessError(env, deviceInfoAsyncCallbackInfo->ret, false);
        napi_reject_deferred(env, deviceInfoAsyncCallbackInfo->deferred, result[0]);
    }
}

void DeviceManagerNapi::CallGetLocalDeviceInfo(napi_env env, napi_status &status,
                                               DeviceInfoAsyncCallbackInfo *deviceInfoAsyncCallbackInfo)
{
    napi_value result[DM_NAPI_ARGS_TWO] = {0};
    LOGI("DeviceManager::CallGetLocalDeviceInfo deviceId:%{public}s deviceName:%{public}s deviceTypeId:%{public}d ",
         GetAnonyString(deviceInfoAsyncCallbackInfo->deviceInfo.deviceId).c_str(),
         GetAnonyString(deviceInfoAsyncCallbackInfo->deviceInfo.deviceName).c_str(),
         deviceInfoAsyncCallbackInfo->deviceInfo.deviceTypeId);
    napi_value callResult = nullptr;
    napi_value handler = nullptr;

    if (deviceInfoAsyncCallbackInfo->status == 0) {
        DmDeviceInfotoJsDeviceInfo(env, deviceInfoAsyncCallbackInfo->deviceInfo, result[1]);
    } else {
        result[0] = CreateBusinessError(env, deviceInfoAsyncCallbackInfo->ret, false);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, deviceInfoAsyncCallbackInfo->callback, &handler));
    if (handler != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, handler, DM_NAPI_ARGS_TWO,
            &result[0], &callResult));
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, deviceInfoAsyncCallbackInfo->callback));
        deviceInfoAsyncCallbackInfo->callback = nullptr;
    } else {
        LOGE("handler is nullptr");
    }
}

void DeviceManagerNapi::CallAsyncWorkSync(napi_env env, DeviceInfoAsyncCallbackInfo *deviceInfoAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "GetLocalDeviceInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            (void)env;
            DeviceInfoAsyncCallbackInfo *devInfoAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoAsyncCallbackInfo *>(data);
            int32_t ret = 0;
            ret = DeviceManager::GetInstance().GetLocalDeviceInfo(devInfoAsyncCallbackInfo->bundleName,
                                                                  devInfoAsyncCallbackInfo->deviceInfo);
            if (ret != 0) {
                LOGE("CallAsyncWorkSync for bundleName %{public}s failed, ret %{public}d",
                     devInfoAsyncCallbackInfo->bundleName.c_str(), ret);
                devInfoAsyncCallbackInfo->status = -1;
                devInfoAsyncCallbackInfo->ret = ret;
            } else {
                devInfoAsyncCallbackInfo->status = 0;
                LOGI("CallAsyncWorkSync status %{public}d", devInfoAsyncCallbackInfo->status);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceInfoAsyncCallbackInfo *dInfoAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoAsyncCallbackInfo *>(data);
            CallGetLocalDeviceInfoSync(env, status, dInfoAsyncCallbackInfo);
            napi_delete_async_work(env, dInfoAsyncCallbackInfo->asyncWork);
            delete dInfoAsyncCallbackInfo;
        },
        (void *)deviceInfoAsyncCallbackInfo, &deviceInfoAsyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, deviceInfoAsyncCallbackInfo->asyncWork, napi_qos_user_initiated);
}

void DeviceManagerNapi::CallAsyncWork(napi_env env, DeviceInfoAsyncCallbackInfo *deviceInfoAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "GetLocalDeviceInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            DeviceInfoAsyncCallbackInfo *devInfoAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoAsyncCallbackInfo *>(data);
            int32_t ret = 0;
            ret = DeviceManager::GetInstance().GetLocalDeviceInfo(devInfoAsyncCallbackInfo->bundleName,
                                                                  devInfoAsyncCallbackInfo->deviceInfo);
            if (ret != 0) {
                LOGE("CallAsyncWork for bundleName %{public}s failed, ret %{public}d",
                     devInfoAsyncCallbackInfo->bundleName.c_str(), ret);
                devInfoAsyncCallbackInfo->status = -1;
                devInfoAsyncCallbackInfo->ret = ret;
            } else {
                devInfoAsyncCallbackInfo->status = 0;
                LOGI("CallAsyncWork status %{public}d", devInfoAsyncCallbackInfo->status);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceInfoAsyncCallbackInfo *dInfoAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoAsyncCallbackInfo *>(data);
            CallGetLocalDeviceInfo(env, status, dInfoAsyncCallbackInfo);
            napi_delete_async_work(env, dInfoAsyncCallbackInfo->asyncWork);
            delete dInfoAsyncCallbackInfo;
        },
        (void *)deviceInfoAsyncCallbackInfo, &deviceInfoAsyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, deviceInfoAsyncCallbackInfo->asyncWork, napi_qos_user_initiated);
}

void DeviceManagerNapi::CallAsyncWorkSync(napi_env env,
                                          DeviceInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "GetTrustListInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            (void)env;
            DeviceInfoListAsyncCallbackInfo *devInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoListAsyncCallbackInfo *>(data);
            int32_t ret = 0;
            ret = DeviceManager::GetInstance().GetTrustedDeviceList(devInfoListAsyncCallbackInfo->bundleName,
                                                                    devInfoListAsyncCallbackInfo->extra,
                                                                    devInfoListAsyncCallbackInfo->devList);
            if (ret != 0) {
                LOGE("CallAsyncWorkSync for bundleName %{public}s failed, ret %{public}d",
                     devInfoListAsyncCallbackInfo->bundleName.c_str(), ret);
                     devInfoListAsyncCallbackInfo->status = -1;
                     devInfoListAsyncCallbackInfo->ret = ret;
            } else {
                devInfoListAsyncCallbackInfo->status = 0;
            }
            LOGI("CallAsyncWorkSync status %{public}d", devInfoListAsyncCallbackInfo->status);
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceInfoListAsyncCallbackInfo *dInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoListAsyncCallbackInfo *>(data);
            CallGetTrustedDeviceListStatusSync(env, status, dInfoListAsyncCallbackInfo);
            napi_delete_async_work(env, dInfoListAsyncCallbackInfo->asyncWork);
            delete dInfoListAsyncCallbackInfo;
        },
        (void *)deviceInfoListAsyncCallbackInfo, &deviceInfoListAsyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, deviceInfoListAsyncCallbackInfo->asyncWork, napi_qos_user_initiated);
}

void DeviceManagerNapi::CallAsyncWork(napi_env env, DeviceInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "GetTrustListInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            DeviceInfoListAsyncCallbackInfo *devInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoListAsyncCallbackInfo *>(data);
            int32_t ret = 0;
            ret = DeviceManager::GetInstance().GetTrustedDeviceList(devInfoListAsyncCallbackInfo->bundleName,
                                                                    devInfoListAsyncCallbackInfo->extra,
                                                                    devInfoListAsyncCallbackInfo->devList);
            if (ret != 0) {
                LOGE("CallAsyncWork for bundleName %{public}s failed, ret %{public}d",
                    devInfoListAsyncCallbackInfo->bundleName.c_str(), ret);
                devInfoListAsyncCallbackInfo->status = -1;
                devInfoListAsyncCallbackInfo->ret = ret;
            } else {
                devInfoListAsyncCallbackInfo->status = 0;
            }
            LOGI("CallAsyncWork status %{public}d", devInfoListAsyncCallbackInfo->status);
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceInfoListAsyncCallbackInfo *dInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceInfoListAsyncCallbackInfo *>(data);
            CallGetTrustedDeviceListStatus(env, status, dInfoListAsyncCallbackInfo);
            napi_delete_async_work(env, dInfoListAsyncCallbackInfo->asyncWork);
            delete dInfoListAsyncCallbackInfo;
            dInfoListAsyncCallbackInfo = nullptr;
        },
        (void *)deviceInfoListAsyncCallbackInfo, &deviceInfoListAsyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, deviceInfoListAsyncCallbackInfo->asyncWork, napi_qos_user_initiated);
}

void DeviceManagerNapi::AsyncTaskCallback(napi_env env, void *data)
{
    CredentialAsyncCallbackInfo *creAsyncCallbackInfo = reinterpret_cast<CredentialAsyncCallbackInfo *>(data);
    int32_t ret = DeviceManager::GetInstance().RequestCredential(creAsyncCallbackInfo->bundleName,
        creAsyncCallbackInfo->reqInfo, creAsyncCallbackInfo->returnJsonStr);
    if (ret != 0) {
        LOGE("CallCredentialAsyncWork for bundleName %{public}s failed, ret %{public}d",
            creAsyncCallbackInfo->bundleName.c_str(), ret);
        creAsyncCallbackInfo->status = -1;
        creAsyncCallbackInfo->ret = ret;
    } else {
        creAsyncCallbackInfo->status = 0;
    }
    LOGI("CallCredentialAsyncWork status %{public}d", creAsyncCallbackInfo->status);
}

void DeviceManagerNapi::AsyncAfterTaskCallback(napi_env env, napi_status status, void *data)
{
    (void)status;
    CredentialAsyncCallbackInfo *creAsyncCallbackInfo = reinterpret_cast<CredentialAsyncCallbackInfo *>(data);
    CallRequestCreInfoStatus(env, status, creAsyncCallbackInfo);
    napi_delete_async_work(env, creAsyncCallbackInfo->asyncWork);
    delete creAsyncCallbackInfo;
}

void DeviceManagerNapi::CallCredentialAsyncWork(napi_env env, CredentialAsyncCallbackInfo *creAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "RequestCreInfo", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env, nullptr, resourceName, AsyncTaskCallback, AsyncAfterTaskCallback,
        (void *)creAsyncCallbackInfo, &creAsyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, creAsyncCallbackInfo->asyncWork, napi_qos_user_initiated);
}

napi_value DeviceManagerNapi::CallDeviceList(napi_env env, napi_callback_info info,
                                             DeviceInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo)
{
    napi_value result = nullptr;
    std::string extra = "";
    deviceInfoListAsyncCallbackInfo->extra = extra;
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[0], &eventHandleType);
    if (eventHandleType == napi_function) {
        LOGI("CallDeviceList for argc %{public}zu Type = %{public}d", argc, (int)eventHandleType);
        napi_create_reference(env, argv[0], 1, &deviceInfoListAsyncCallbackInfo->callback);
        CallAsyncWork(env, deviceInfoListAsyncCallbackInfo);
        napi_get_undefined(env, &result);
        return result;
    } else {
        LOGI("CallDeviceList for argc %{public}zu Type = %{public}d", argc, (int)eventHandleType);
        napi_deferred deferred;
        napi_value promise = 0;
        napi_create_promise(env, &deferred, &promise);
        deviceInfoListAsyncCallbackInfo->deferred = deferred;
        char extraString[20];
        JsObjectToString(env, argv[0], "extra", extraString, sizeof(extraString));
        deviceInfoListAsyncCallbackInfo->extra = extraString;
        CallAsyncWorkSync(env, deviceInfoListAsyncCallbackInfo);
        return promise;
    }
}

napi_value DeviceManagerNapi::GetTrustedDeviceListSync(napi_env env, napi_callback_info info)
{
    LOGI("GetTrustedDeviceListSync in");
    if (!CheckPermissions(env)) {
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
    std::vector<OHOS::DistributedHardware::DmDeviceInfo> devList;
    int32_t ret = 0;
    if (argc == DM_NAPI_ARGS_ZERO) {
        std::string extra = "";
        ret = DeviceManager::GetInstance().GetTrustedDeviceList(deviceManagerWrapper->bundleName_, extra, devList);
    } else if (argc == DM_NAPI_ARGS_ONE) {
        GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
        napi_valuetype valueType;
        napi_typeof(env, argv[0], &valueType);
        if (!CheckArgsType(env, valueType == napi_boolean, "refreshList", "bool")) {
            return nullptr;
        }
        bool isRefresh = false;
        napi_get_value_bool(env, argv[0], &isRefresh);
        std::string extra = "";
        ret = DeviceManager::GetInstance().GetTrustedDeviceList(deviceManagerWrapper->bundleName_, extra, isRefresh,
            devList);
    }
    if (ret != 0) {
        LOGE("GetTrustedDeviceList for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
        return result;
    }
    if (devList.size() > 0) {
        for (size_t i = 0; i != devList.size(); ++i) {
            DeviceInfoToJsArray(env, devList, (int32_t)i, result);
        }
    }
    return result;
}

napi_value DeviceManagerNapi::GetTrustedDeviceListPromise(napi_env env,
    DeviceInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo)
{
    std::string extra = "";
    deviceInfoListAsyncCallbackInfo->extra = extra;
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    deviceInfoListAsyncCallbackInfo->deferred = deferred;
    CallAsyncWorkSync(env, deviceInfoListAsyncCallbackInfo);
    return promise;
}

napi_value DeviceManagerNapi::GetTrustedDeviceListByFilter(napi_env env, napi_callback_info info,
    DeviceInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_string, "extra", "string")) {
        DeleteAsyncCallbackInfo(deviceInfoListAsyncCallbackInfo);
        return nullptr;
    }

    if (!IsFunctionType(env, argv[1])) {
        DeleteAsyncCallbackInfo(deviceInfoListAsyncCallbackInfo);
        return result;
    }
    char extra[DM_NAPI_BUF_LENGTH];
    JsObjectToString(env, argv[0], "extra", extra, sizeof(extra));
    deviceInfoListAsyncCallbackInfo->extra = extra;
    napi_create_reference(env, argv[1], 1, &deviceInfoListAsyncCallbackInfo->callback);
    CallAsyncWork(env, deviceInfoListAsyncCallbackInfo);
    return result;
}

napi_value DeviceManagerNapi::GetTrustedDeviceList(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    size_t argc = 0;
    std::vector<DmDeviceInfo> devList;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    auto *deviceInfoListAsyncCallbackInfo = new DeviceInfoListAsyncCallbackInfo();
    if (deviceInfoListAsyncCallbackInfo == nullptr) {
        return nullptr;
    }
    deviceInfoListAsyncCallbackInfo->env = env;
    deviceInfoListAsyncCallbackInfo->devList = devList;
    deviceInfoListAsyncCallbackInfo->bundleName = deviceManagerWrapper->bundleName_;
    if (argc == 0) {
        return GetTrustedDeviceListPromise(env, deviceInfoListAsyncCallbackInfo);
    } else if (argc == 1) {
        GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
        if (!IsFunctionType(env, argv[0])) {
            DeleteAsyncCallbackInfo(deviceInfoListAsyncCallbackInfo);
            return nullptr;
        }
        return CallDeviceList(env, info, deviceInfoListAsyncCallbackInfo);
    } else if (argc == DM_NAPI_ARGS_TWO) {
        return GetTrustedDeviceListByFilter(env, info, deviceInfoListAsyncCallbackInfo);
    } else {
        DeleteAsyncCallbackInfo(deviceInfoListAsyncCallbackInfo);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::GetLocalDeviceInfoSync(napi_env env, napi_callback_info info)
{
    LOGI("GetLocalDeviceInfoSync in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    DmDeviceInfo deviceInfo;
    size_t argc = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    ret = DeviceManager::GetInstance().GetLocalDeviceInfo(deviceManagerWrapper->bundleName_, deviceInfo);
    if (ret != 0) {
        LOGE("GetLocalDeviceInfoSync for failed, ret %{public}d", ret);
        CreateBusinessError(env, ret);
        return result;
    }
    LOGI("DeviceManager::GetLocalDeviceInfoSync deviceId:%{public}s deviceName:%{public}s deviceTypeId:%{public}d ",
         GetAnonyString(std::string(deviceInfo.deviceId)).c_str(),
         GetAnonyString(std::string(deviceInfo.deviceName)).c_str(),
         deviceInfo.deviceTypeId);
    DmDeviceInfotoJsDeviceInfo(env, deviceInfo, result);
    return result;
}

napi_value DeviceManagerNapi::GetLocalDeviceInfo(napi_env env, napi_callback_info info)
{
    LOGI("GetLocalDeviceInfo in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    if (DeviceManager::GetInstance().CheckAPIAccessPermission() != 0) {
        CreateBusinessError(env, ERR_DM_NO_PERMISSION);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    size_t argc = 0;
    DmDeviceInfo deviceInfo;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    auto *deviceInfoAsyncCallbackInfo = new DeviceInfoAsyncCallbackInfo();
    if (deviceInfoAsyncCallbackInfo == nullptr) {
        return nullptr;
    }
    deviceInfoAsyncCallbackInfo->env = env;
    deviceInfoAsyncCallbackInfo->deviceInfo = deviceInfo;
    deviceInfoAsyncCallbackInfo->bundleName = deviceManagerWrapper->bundleName_;
    if (argc == 0) {
        std::string extra = "";
        deviceInfoAsyncCallbackInfo->extra = extra;
        napi_deferred deferred;
        napi_value promise = 0;
        napi_create_promise(env, &deferred, &promise);
        deviceInfoAsyncCallbackInfo->deferred = deferred;
        CallAsyncWorkSync(env, deviceInfoAsyncCallbackInfo);
        return promise;
    } else if (argc == 1) {
        GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
        if (!IsFunctionType(env, argv[0])) {
            delete deviceInfoAsyncCallbackInfo;
            deviceInfoAsyncCallbackInfo = nullptr;
            return nullptr;
        }
        deviceInfoAsyncCallbackInfo->extra = "";
        napi_create_reference(env, argv[0], 1, &deviceInfoAsyncCallbackInfo->callback);
        CallAsyncWork(env, deviceInfoAsyncCallbackInfo);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::UnAuthenticateDevice(napi_env env, napi_callback_info info)
{
    LOGI("UnAuthenticateDevice");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    napi_value result = nullptr;
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE,  "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    napi_valuetype deviceInfoType = napi_undefined;
    napi_typeof(env, argv[0], &deviceInfoType);
    if (!CheckArgsType(env, deviceInfoType == napi_object, "deviceInfo", "object")) {
        return nullptr;
    }

    DmDeviceInfo deviceInfo;
    JsToDmDeviceInfo(env, argv[0], deviceInfo);
    LOGI("UnAuthenticateDevice deviceId = %{public}s", GetAnonyString(deviceInfo.deviceId).c_str());
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(deviceManagerWrapper->bundleName_, deviceInfo);
    if (ret != 0) {
        LOGE("UnAuthenticateDevice for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }

    napi_create_int32(env, ret, &result);
    return result;
}

bool DeviceManagerNapi::StartArgCheck(napi_env env, napi_value &argv,
    OHOS::DistributedHardware::DmSubscribeInfo &subInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv, &valueType);
    if (!CheckArgsType(env, valueType == napi_object, "subscribeInfo", "object")) {
        return false;
    }
    int32_t res = JsToDmSubscribeInfo(env, argv, subInfo);
    if (!CheckArgsVal(env, res == 0, "subscribeId", "Wrong subscribeId")) {
        return false;
    }
    return true;
}

bool DeviceManagerNapi::CheckPermissions(napi_env env)
{
    LOGI("CheckPermissions in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return false;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return false;
    }
    return true;
}

void DeviceManagerNapi::LockDiscoveryCallbackMutex(napi_env env, DmSubscribeInfo subInfo, std::string &bundleName,
    std::string &extra)
{
    std::shared_ptr<DmNapiDiscoveryCallback> discoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        auto iter = g_DiscoveryCallbackMap.find(bundleName);
        if (iter == g_DiscoveryCallbackMap.end()) {
            discoveryCallback = std::make_shared<DmNapiDiscoveryCallback>(env, bundleName);
            if (g_DiscoveryCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_DiscoveryCallbackMap map size is more than max size");
                return;
            }
            g_DiscoveryCallbackMap[bundleName] = discoveryCallback;
        } else {
            discoveryCallback = iter->second;
        }
    }
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(bundleName, subInfo, extra, discoveryCallback);
    if (ret != 0) {
        LOGE("Discovery failed, bundleName %{public}s, ret %{public}d", bundleName.c_str(), ret);
        CreateBusinessError(env, ret);
        discoveryCallback->OnDiscoveryFailed(subInfo.subscribeId, ret);
    }
    return;
}

napi_value DeviceManagerNapi::StartDeviceDiscoverSync(napi_env env, napi_callback_info info)
{
    LOGI("StartDeviceDiscoverSync in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    std::string extra = "";
    DmSubscribeInfo subInfo;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;
    size_t argcNum = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcNum, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    if (argcNum >= DM_NAPI_ARGS_ONE) {
        GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
        if (!StartArgCheck(env, argv[0], subInfo)) {
            return nullptr;
        }
        if (argcNum == DM_NAPI_ARGS_TWO) {
            napi_valuetype valueType1 = napi_undefined;
            napi_typeof(env, argv[1], &valueType1);
            if (!(CheckArgsType(env, (valueType1 == napi_undefined || valueType1 == napi_string), "filterOptions",
                "string or undefined"))) {
                return nullptr;
            }
            JsToDmDiscoveryExtra(env, argv[1], extra);
        }
    }
    LockDiscoveryCallbackMutex(env, subInfo, deviceManagerWrapper->bundleName_, extra);
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::StopDeviceDiscoverSync(napi_env env, napi_callback_info info)
{
    LOGI("StopDeviceDiscoverSync in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE,  "Wrong number of arguments, required 1")) {
        return nullptr;
    }

    napi_value result = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_number, "subscribeId", "number")) {
        return nullptr;
    }

    int32_t subscribeId = 0;
    napi_get_value_int32(env, argv[0], &subscribeId);
    if (!CheckArgsVal(env, subscribeId <= DM_NAPI_SUB_ID_MAX, "subscribeId", "Wrong argument. subscribeId Too Big")) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(deviceManagerWrapper->bundleName_,
                                                                   static_cast<int16_t>(subscribeId));
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
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
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
            if (g_publishCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_publishCallbackMap map size is more than max size");
                return nullptr;
            }
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
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
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

napi_value DeviceManagerNapi::AuthenticateDevice(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_THREE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_THREE,  "Wrong number of arguments, required 3")) {
        return nullptr;
    }
    napi_value result = nullptr;
    if (!IsJSObjectType(env, argv[0], "deviceInfo") ||
        !IsJSObjectType(env, argv[DM_NAPI_ARGS_ONE], "authParam") ||
        !IsFunctionType(env, argv[DM_NAPI_ARGS_TWO])) {
        return nullptr;
    }

    authAsyncCallbackInfo_.env = env;
    napi_create_reference(env, argv[DM_NAPI_ARGS_TWO], 1, &authAsyncCallbackInfo_.callback);
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }

    std::shared_ptr<DmNapiAuthenticateCallback> authCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
        auto iter = g_authCallbackMap.find(deviceManagerWrapper->bundleName_);
        if (iter == g_authCallbackMap.end()) {
            authCallback = std::make_shared<DmNapiAuthenticateCallback>(env, deviceManagerWrapper->bundleName_);
            if (g_authCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_authCallbackMap map size is more than max size");
                return nullptr;
            }
            g_authCallbackMap[deviceManagerWrapper->bundleName_] = authCallback;
        } else {
            authCallback = iter->second;
        }
    }
    DmDeviceInfo deviceInfo;
    JsToDmDeviceInfo(env, argv[0], deviceInfo);
    std::string extraString;
    JsToDmExtra(env, argv[DM_NAPI_ARGS_ONE], extraString, authAsyncCallbackInfo_.authType);
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(deviceManagerWrapper->bundleName_,
        authAsyncCallbackInfo_.authType, deviceInfo, extraString, authCallback);
    if (ret != 0) {
        LOGE("AuthDevice failed bundleName %{public}s, ret %{public}d", deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::RequestCredential(napi_env env, napi_callback_info info)
{
    LOGI("RequestCredential function has been discarded");
    return nullptr;
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);

    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
        return nullptr;
    }

    napi_value result = nullptr;
    napi_valuetype requestInfoValueType = napi_undefined;
    napi_typeof(env, argv[0], &requestInfoValueType);
    if (!CheckArgsType(env, requestInfoValueType == napi_string, "requestInfo", "string")) {
        return nullptr;
    }

    napi_valuetype funcValueType = napi_undefined;
    napi_typeof(env, argv[1], &funcValueType);
    if (!CheckArgsType(env, funcValueType == napi_function, "callback", "function")) {
        return nullptr;
    }

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));
    if (deviceManagerWrapper == nullptr) {
        LOGE(" DeviceManagerNapi object is nullptr!");
        return result;
    }

    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);
    NAPI_ASSERT(env, typeLen < DM_NAPI_BUF_LENGTH, "typeLen >= MAXLEN");
    char type[DM_NAPI_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    auto *creAsyncCallbackInfo = new CredentialAsyncCallbackInfo();
    creAsyncCallbackInfo->env = env;
    creAsyncCallbackInfo->bundleName = deviceManagerWrapper->bundleName_;
    creAsyncCallbackInfo->reqInfo = type;

    napi_create_reference(env, argv[1], 1, &creAsyncCallbackInfo->callback);
    CallCredentialAsyncWork(env, creAsyncCallbackInfo);
    napi_get_undefined(env, &result);
    return result;
}

int32_t DeviceManagerNapi::RegisterCredentialCallback(napi_env env, const std::string &pkgName)
{
    std::shared_ptr<DmNapiCredentialCallback> creCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(creMapLocks_);
        auto iter = g_creCallbackMap.find(pkgName);
        if (iter == g_creCallbackMap.end()) {
            creCallback = std::make_shared<DmNapiCredentialCallback>(env, pkgName);
            if (g_creCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_creCallbackMap map size is more than max size");
                return ERR_DM_FAILED;
            }
            g_creCallbackMap[pkgName] = creCallback;
        } else {
            creCallback = iter->second;
        }
    }
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(pkgName,
        creCallback);
    if (ret != 0) {
        LOGE("RegisterCredentialCallback for bundleName %{public}s failed, ret %{public}d", pkgName.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    return ret;
}

napi_value DeviceManagerNapi::ImportCredential(napi_env env, napi_callback_info info)
{
    LOGI("ImportCredential start.");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
        return nullptr;
    }
    napi_value result = nullptr;
    napi_valuetype importInfoValueType = napi_undefined;
    napi_typeof(env, argv[0], &importInfoValueType);
    if (!CheckArgsType(env, importInfoValueType == napi_string, "credentialInfo", "string")) {
        return nullptr;
    }
    if (!IsFunctionType(env, argv[1])) {
        return nullptr;
    }

    creAsyncCallbackInfo_.env = env;
    napi_create_reference(env, argv[1], 1, &creAsyncCallbackInfo_.callback);

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));
    if (deviceManagerWrapper == nullptr) {
        LOGE(" DeviceManagerNapi object is nullptr!");
        return result;
    }
    if (RegisterCredentialCallback(env, deviceManagerWrapper->bundleName_) != 0) {
        LOGE("RegisterCredentialCallback failed!");
        return result;
    }

    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);
    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    NAPI_ASSERT(env, typeLen < DM_NAPI_CREDENTIAL_BUF_LENGTH, "typeLen >= MAXLEN");
    char type[DM_NAPI_CREDENTIAL_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);
    std::string credentialInfo = type;
    int32_t ret = DeviceManager::GetInstance().ImportCredential(deviceManagerWrapper->bundleName_, credentialInfo);
    if (ret != 0) {
        LOGE("ImportCredential for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::DeleteCredential(napi_env env, napi_callback_info info)
{
    LOGE("DeleteCredential function has been discarded");
    return nullptr;
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
        return nullptr;
    }

    napi_value result = nullptr;
    napi_valuetype queryInfoValueType = napi_undefined;
    napi_typeof(env, argv[0], &queryInfoValueType);
    if (!CheckArgsType(env, queryInfoValueType == napi_string, "queryInfo", "string")) {
        return nullptr;
    }
    if (!IsFunctionType(env, argv[1])) {
        return nullptr;
    }

    creAsyncCallbackInfo_.env = env;
    napi_create_reference(env, argv[1], 1, &creAsyncCallbackInfo_.callback);
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));
    if (deviceManagerWrapper == nullptr) {
        LOGE(" DeviceManagerNapi object is nullptr!");
        return result;
    }
    if (RegisterCredentialCallback(env, deviceManagerWrapper->bundleName_) != 0) {
        LOGE("RegisterCredentialCallback failed!");
        return result;
    }

    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);
    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    NAPI_ASSERT(env, typeLen < DM_NAPI_CREDENTIAL_BUF_LENGTH, "typeLen >= MAXLEN");
    char type[DM_NAPI_CREDENTIAL_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);
    std::string queryInfo = type;
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(deviceManagerWrapper->bundleName_, queryInfo);
    if (ret != 0) {
        LOGE("DeleteCredential for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::JsOnFrench(napi_env env, int32_t num, napi_value thisVar, napi_value argv[])
{
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);

    if (!CheckArgsVal(env, typeLen > 0, "type", "typeLen == 0")) {
        return nullptr;
    }
    if (!CheckArgsVal(env, typeLen < DM_NAPI_BUF_LENGTH, "type", "typeLen >= MAXLEN")) {
        return nullptr;
    }
    char type[DM_NAPI_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    std::string eventType = type;
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
            size_t extraLen = 0;
            napi_get_value_string_utf8(env, argv[1], nullptr, 0, &extraLen);
            if (!CheckArgsVal(env, extraLen < DM_NAPI_BUF_LENGTH, "extra", "extraLen >= MAXLEN")) {
                return nullptr;
            }
            char extra[DM_NAPI_BUF_LENGTH] = {0};
            napi_get_value_string_utf8(env, argv[1], extra, extraLen + 1, &extraLen);
            std::string extraString = extra;
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
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    if (argc == DM_NAPI_ARGS_THREE) {
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
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);
    if (!CheckArgsVal(env, typeLen > 0, "type", "typeLen == 0")) {
        return nullptr;
    }
    if (!CheckArgsVal(env, typeLen < DM_NAPI_BUF_LENGTH, "type", "typeLen >= MAXLEN")) {
        return nullptr;
    }
    char type[DM_NAPI_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    napi_value result = nullptr;
    std::string eventType = type;
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
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
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
        std::lock_guard<std::mutex> autoLock(g_deviceStateCallbackMapMutex);
        g_deviceStateCallbackMap.erase(bundleName);
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
        std::lock_guard<std::mutex> autoLock(creMapLocks_);
        g_creCallbackMap.erase(bundleName);
    }
}
napi_value DeviceManagerNapi::ReleaseDeviceManager(napi_env env, napi_callback_info info)
{
    LOGI("ReleaseDeviceManager in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != 0) {
        CreateBusinessError(env, ret);
        return nullptr;
    }
    size_t argc = 0;
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    LOGI("ReleaseDeviceManager for bundleName %{public}s", deviceManagerWrapper->bundleName_.c_str());
    ret = DeviceManager::GetInstance().UnInitDeviceManager(deviceManagerWrapper->bundleName_);
    if (ret != 0) {
        LOGE("ReleaseDeviceManager for bundleName %{public}s failed, ret %{public}d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
        napi_create_uint32(env, static_cast<uint32_t>(ret), &result);
        return result;
    }
    ClearBundleCallbacks(deviceManagerWrapper->bundleName_);
    napi_get_undefined(env, &result);
    NAPI_CALL(env, napi_remove_wrap(env, thisVar, (void**)&deviceManagerWrapper));
    return result;
}

void DeviceManagerNapi::HandleCreateDmCallBackCompletedCB(napi_env env, napi_status status, void *data)
{
    (void)status;
    AsyncCallbackInfo *asyncCallbackInfo = reinterpret_cast<AsyncCallbackInfo *>(data);
    napi_value result[DM_NAPI_ARGS_TWO] = {0};
    if (asyncCallbackInfo->status == 0) {
        napi_value ctor = nullptr;
        napi_value argv = nullptr;
        napi_get_reference_value(env, sConstructor_, &ctor);
        napi_create_string_utf8(env, asyncCallbackInfo->bundleName, NAPI_AUTO_LENGTH, &argv);
        napi_status ret = napi_new_instance(env, ctor, DM_NAPI_ARGS_ONE, &argv, &result[1]);
        if (ret != napi_ok) {
                LOGE("Create DeviceManagerNapi for bundleName %{public}s failed", asyncCallbackInfo->bundleName);
        } else {
                LOGI("InitDeviceManager for bundleName %{public}s success", asyncCallbackInfo->bundleName);
                napi_get_undefined(env, &result[0]);
        }
    } else {
        LOGI("InitDeviceManager for bundleName %{public}s failed", asyncCallbackInfo->bundleName);
        result[0] = CreateBusinessError(env, asyncCallbackInfo->ret, false);
    }
    napi_value callback = nullptr;
    napi_value callResult = nullptr;
    napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
    if (callback != nullptr) {
        napi_call_function(env, nullptr, callback, DM_NAPI_ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env, asyncCallbackInfo->callback);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

void DeviceManagerNapi::HandleCreateDmCallBack(const napi_env &env, AsyncCallbackInfo *asCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "createDeviceManagerCallback", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            (void)env;
            AsyncCallbackInfo *asyCallbackInfo = reinterpret_cast<AsyncCallbackInfo *>(data);
            std::string bundleName = std::string(asyCallbackInfo->bundleName);
            std::shared_ptr<DmNapiInitCallback> initCallback = std::make_shared<DmNapiInitCallback>(env, bundleName);
            int32_t ret = DeviceManager::GetInstance().InitDeviceManager(bundleName, initCallback);
            if (ret != 0) {
                asyCallbackInfo->status = 1;
                asyCallbackInfo->ret = ret;
                return;
            }
            std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
            if (g_initCallbackMap.size() >= MAX_CONTAINER_SIZE) {
                LOGE("g_initCallbackMap map size is more than max size");
                asyCallbackInfo->status = 1;
                asyCallbackInfo->ret = ERR_DM_NOT_INIT;
                return;
            }
            g_initCallbackMap[bundleName] = initCallback;
            asyCallbackInfo->status = 0;
        }, HandleCreateDmCallBackCompletedCB, (void *)asCallbackInfo, &asCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asCallbackInfo->asyncWork, napi_qos_user_initiated);
}

void DeviceManagerNapi::CallGetDeviceInfo(napi_env env, NetworkIdAsyncCallbackInfo *networkIdAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "GetLocalDeviceInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        GetDeviceInfoCB,
        CompleteGetDeviceInfoCB,
        (void *)networkIdAsyncCallbackInfo, &networkIdAsyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, networkIdAsyncCallbackInfo->asyncWork, napi_qos_user_initiated);
}

void DeviceManagerNapi::GetDeviceInfoCB(napi_env env, void *data)
{
    if (!data) {
        LOGE("Invalid async callback data");
        return;
    }
    (void)env;
    NetworkIdAsyncCallbackInfo *networkIdAsyncCallbackInfo = reinterpret_cast<NetworkIdAsyncCallbackInfo *>(data);
    int32_t ret = DeviceManager::GetInstance().GetDeviceInfo(networkIdAsyncCallbackInfo->bundleName,
                                                             networkIdAsyncCallbackInfo->networkId,
                                                             networkIdAsyncCallbackInfo->deviceInfo);
    if (ret != 0) {
        LOGE("GetDeviceInfoCB for bundleName %{public}s networkId %{public}s failed, ret %{public}d",
             networkIdAsyncCallbackInfo->bundleName.c_str(),
             GetAnonyString(networkIdAsyncCallbackInfo->networkId).c_str(), ret);
        networkIdAsyncCallbackInfo->status = -1;
        networkIdAsyncCallbackInfo->ret = ret;
    } else {
        networkIdAsyncCallbackInfo->status = 0;
        LOGI("GetDeviceInfoCB status %{public}d", networkIdAsyncCallbackInfo->status);
    }
}

void DeviceManagerNapi::CompleteGetDeviceInfoCB(napi_env env, napi_status status, void *data)
{
    if (!data) {
        LOGE("Invalid async callback data");
        return;
    }
    (void)status;
    NetworkIdAsyncCallbackInfo *networkIdAsyncCallbackInfo = reinterpret_cast<NetworkIdAsyncCallbackInfo *>(data);
    if (networkIdAsyncCallbackInfo->deferred != nullptr) {
        CallGetDeviceInfoPromise(env, status, networkIdAsyncCallbackInfo);    // promise
    } else {
        CallGetDeviceInfoCB(env, status, networkIdAsyncCallbackInfo);         // callback
    }
    napi_delete_async_work(env, networkIdAsyncCallbackInfo->asyncWork);
    delete networkIdAsyncCallbackInfo;
}

// promise function
void DeviceManagerNapi::CallGetDeviceInfoPromise(napi_env env, napi_status &status,
                                                 NetworkIdAsyncCallbackInfo *networkIdAsyncCallbackInfo)
{
    napi_value result[DM_NAPI_ARGS_TWO] = {0};

    LOGI("DeviceManager::CallGetDeviceInfoSync deviceName:%{public}s deviceTypeId:%{public}d ",
         GetAnonyString(networkIdAsyncCallbackInfo->deviceInfo.deviceName).c_str(),
         networkIdAsyncCallbackInfo->deviceInfo.deviceTypeId);

    if (networkIdAsyncCallbackInfo->status == 0) {
        DeviceInfotoJsByNetworkId(env, networkIdAsyncCallbackInfo->deviceInfo, result[1]);
        napi_resolve_deferred(env, networkIdAsyncCallbackInfo->deferred, result[1]);
    } else {
        result[0] = CreateBusinessError(env, networkIdAsyncCallbackInfo->ret, false);
        napi_reject_deferred(env, networkIdAsyncCallbackInfo->deferred, result[0]);
    }
}

// callback function
void DeviceManagerNapi::CallGetDeviceInfoCB(napi_env env, napi_status &status,
                                            NetworkIdAsyncCallbackInfo *networkIdAsyncCallbackInfo)
{
    napi_value result[DM_NAPI_ARGS_TWO] = {0};
    LOGI("DeviceManager::CallGetDeviceInfo deviceName:%{public}s deviceTypeId:%{public}d ",
         GetAnonyString(networkIdAsyncCallbackInfo->deviceInfo.deviceName).c_str(),
         networkIdAsyncCallbackInfo->deviceInfo.deviceTypeId);
    napi_value callResult = nullptr;
    napi_value handler = nullptr;

    if (networkIdAsyncCallbackInfo->status == 0) {
        DeviceInfotoJsByNetworkId(env, networkIdAsyncCallbackInfo->deviceInfo, result[1]);
    } else {
        result[0] = CreateBusinessError(env, networkIdAsyncCallbackInfo->ret, false);
    }

    napi_get_reference_value(env, networkIdAsyncCallbackInfo->callback, &handler);
    if (handler != nullptr) {
        napi_call_function(env, nullptr, handler, DM_NAPI_ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env, networkIdAsyncCallbackInfo->callback);
        networkIdAsyncCallbackInfo->callback = nullptr;
    } else {
        LOGE("handler is nullptr");
    }
}

void DeviceManagerNapi::DeviceInfotoJsByNetworkId(const napi_env &env, const DmDeviceInfo &nidDevInfo,
                                                  napi_value &result)
{
    napi_create_object(env, &result);

    SetValueUtf8String(env, "deviceName", nidDevInfo.deviceName, result);
    SetValueInt32(env, "deviceType", (int)nidDevInfo.deviceTypeId, result);
}

bool DeviceManagerNapi::JsToStringAndCheck(napi_env env, napi_value value, const std::string &valueName,
                                           std::string &strValue)
{
    napi_valuetype deviceIdType = napi_undefined;
    napi_typeof(env, value, &deviceIdType);
    if (!CheckArgsType(env, deviceIdType == napi_string, valueName, "string")) {
        return false;
    }
    size_t valueLen = 0;
    napi_get_value_string_utf8(env, value, nullptr, 0, &valueLen);
    if (!CheckArgsVal(env, valueLen > 0, valueName, "len == 0")) {
        return false;
    }
    if (!CheckArgsVal(env, valueLen < DM_NAPI_BUF_LENGTH, valueName, "len >= MAXLEN")) {
        return false;
    }
    char temp[DM_NAPI_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, value, temp, valueLen + 1, &valueLen);
    strValue = temp;
    return true;
}

napi_value DeviceManagerNapi::GetDeviceInfo(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    napi_value result = nullptr;
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_value thisVar = nullptr;
    DmDeviceInfo deviceInfo;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, ((argc >= DM_NAPI_ARGS_ONE) && (argc <= DM_NAPI_ARGS_TWO)), "requires 1 or 2 parameter");

    std::string networkId;
    if (!JsToStringAndCheck(env, argv[0], "networkId", networkId)) {
        return nullptr;
    }
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    auto *networkIdAsyncCallbackInfo = new NetworkIdAsyncCallbackInfo();
    networkIdAsyncCallbackInfo->env = env;
    networkIdAsyncCallbackInfo->deviceInfo = deviceInfo;
    networkIdAsyncCallbackInfo->bundleName = deviceManagerWrapper->bundleName_;
    networkIdAsyncCallbackInfo->networkId = networkId;

    if (argc == DM_NAPI_ARGS_ONE) {    // promise
        napi_deferred deferred;
        napi_value promise = 0;
        napi_create_promise(env, &deferred, &promise);
        networkIdAsyncCallbackInfo->deferred = deferred;
        CallGetDeviceInfo(env, networkIdAsyncCallbackInfo);
        return promise;
    } else if (argc == DM_NAPI_ARGS_TWO) {    // callback
        if (!IsFunctionType(env, argv[1])) {
            delete networkIdAsyncCallbackInfo;
            networkIdAsyncCallbackInfo = nullptr;
            return nullptr;
        }
        napi_create_reference(env, argv[1], 1, &networkIdAsyncCallbackInfo->callback);
        CallGetDeviceInfo(env, networkIdAsyncCallbackInfo);
    }
    napi_get_undefined(env, &result);
    return result;
}
napi_value DeviceManagerNapi::CreateDeviceManager(napi_env env, napi_callback_info info)
{
    LOGI("CreateDeviceManager in");
    if (!IsSystemApp()) {
        CreateBusinessError(env, static_cast<int32_t>(DMBussinessErrorCode::ERR_NOT_SYSTEM_APP));
        return nullptr;
    }
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);

    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
        return nullptr;
    }

    napi_valuetype bundleNameValueType = napi_undefined;
    napi_typeof(env, argv[0], &bundleNameValueType);
    if (!CheckArgsType(env, bundleNameValueType == napi_string, "bundleName", "string")) {
        return nullptr;
    }

    napi_valuetype funcValueType = napi_undefined;
    napi_typeof(env, argv[1], &funcValueType);
    if (!CheckArgsType(env, funcValueType == napi_function, "callback", "function")) {
        return nullptr;
    }

    auto *asCallbackInfo = new AsyncCallbackInfo();
    if (asCallbackInfo == nullptr) {
        return nullptr;
    }
    asCallbackInfo->env = env;
    napi_get_value_string_utf8(env, argv[0], asCallbackInfo->bundleName, DM_NAPI_BUF_LENGTH - 1,
                               &asCallbackInfo->bundleNameLen);

    napi_create_reference(env, argv[1], 1, &asCallbackInfo->callback);

    HandleCreateDmCallBack(env, asCallbackInfo);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::Constructor(napi_env env, napi_callback_info info)
{
    LOGI("DeviceManagerNapi Constructor in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCount(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsType(env, valueType == napi_string, "bundleName", "string")) {
        return nullptr;
    }

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], bundleName, sizeof(bundleName), &typeLen);

    LOGI("create DeviceManagerNapi for packageName:%{public}s", GetAnonyString(bundleName).c_str());
    DeviceManagerNapi *obj = new DeviceManagerNapi(env, thisVar);
    if (obj == nullptr) {
        return nullptr;
    }

    obj->bundleName_ = std::string(bundleName);
    std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
    if (g_deviceManagerMap.size() >= MAX_CONTAINER_SIZE) {
        LOGE("g_deviceManagerMap map size is more than max size");
        delete obj;
        obj = nullptr;
        return nullptr;
    }
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
        DECLARE_NAPI_FUNCTION("release", ReleaseDeviceManager),
        DECLARE_NAPI_FUNCTION("getTrustedDeviceListSync", GetTrustedDeviceListSync),
        DECLARE_NAPI_FUNCTION("getTrustedDeviceList", GetTrustedDeviceList),
        DECLARE_NAPI_FUNCTION("startDeviceDiscovery", StartDeviceDiscoverSync),
        DECLARE_NAPI_FUNCTION("stopDeviceDiscovery", StopDeviceDiscoverSync),
        DECLARE_NAPI_FUNCTION("publishDeviceDiscovery", PublishDeviceDiscoverySync),
        DECLARE_NAPI_FUNCTION("unPublishDeviceDiscovery", UnPublishDeviceDiscoverySync),
        DECLARE_NAPI_FUNCTION("getLocalDeviceInfoSync", GetLocalDeviceInfoSync),
        DECLARE_NAPI_FUNCTION("getLocalDeviceInfo", GetLocalDeviceInfo),
        DECLARE_NAPI_FUNCTION("getDeviceInfo", GetDeviceInfo),
        DECLARE_NAPI_FUNCTION("unAuthenticateDevice", UnAuthenticateDevice),
        DECLARE_NAPI_FUNCTION("authenticateDevice", AuthenticateDevice),
        DECLARE_NAPI_FUNCTION("setUserOperation", SetUserOperationSync),
        DECLARE_NAPI_FUNCTION("requestCredentialRegisterInfo", RequestCredential),
        DECLARE_NAPI_FUNCTION("importCredential", ImportCredential),
        DECLARE_NAPI_FUNCTION("deleteCredential", DeleteCredential),
        DECLARE_NAPI_FUNCTION("on", JsOn),
        DECLARE_NAPI_FUNCTION("off", JsOff)};

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createDeviceManager", CreateDeviceManager),
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

napi_value DeviceManagerNapi::InitDeviceTypeEnum(napi_env env, napi_value exports)
{
    napi_value unknown_type;
    napi_value speaker;
    napi_value phone;
    napi_value tablet;
    napi_value wearable;
    napi_value car;
    napi_value tv;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceType::DEVICE_TYPE_UNKNOWN),
        &unknown_type);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceType::DEVICE_TYPE_AUDIO),
        &speaker);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceType::DEVICE_TYPE_PHONE),
        &phone);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceType::DEVICE_TYPE_PAD),
        &tablet);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceType::DEVICE_TYPE_WATCH),
        &wearable);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceType::DEVICE_TYPE_CAR),
        &car);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceType::DEVICE_TYPE_TV),
        &tv);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("UNKNOWN_TYPE", unknown_type),
        DECLARE_NAPI_STATIC_PROPERTY("SPEAKER", speaker),
        DECLARE_NAPI_STATIC_PROPERTY("PHONE", phone),
        DECLARE_NAPI_STATIC_PROPERTY("TABLET", tablet),
        DECLARE_NAPI_STATIC_PROPERTY("WEARABLE", wearable),
        DECLARE_NAPI_STATIC_PROPERTY("CAR", car),
        DECLARE_NAPI_STATIC_PROPERTY("TV", tv),
    };

    napi_value result = nullptr;
    napi_define_class(env, "DeviceType", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &deviceTypeEnumConstructor_);
    napi_set_named_property(env, exports, "DeviceType", result);
    return exports;
}

napi_value DeviceManagerNapi::InitDeviceStateChangeActionEnum(napi_env env, napi_value exports)
{
    napi_value device_state_online;
    napi_value device_state_ready;
    napi_value device_state_offline;
    napi_value device_state_change;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceState::DEVICE_STATE_ONLINE),
        &device_state_online);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceState::DEVICE_INFO_READY),
        &device_state_ready);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceState::DEVICE_STATE_OFFLINE),
        &device_state_offline);
    napi_create_uint32(env, static_cast<uint32_t>(DmDeviceState::DEVICE_INFO_CHANGED),
        &device_state_change);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("ONLINE", device_state_online),
        DECLARE_NAPI_STATIC_PROPERTY("READY", device_state_ready),
        DECLARE_NAPI_STATIC_PROPERTY("OFFLINE", device_state_offline),
        DECLARE_NAPI_STATIC_PROPERTY("CHANGE", device_state_change),
    };

    napi_value result = nullptr;
    napi_define_class(env, "DeviceStateChangeAction", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &deviceStateChangeActionEnumConstructor_);
    napi_set_named_property(env, exports, "DeviceStateChangeAction", result);
    return exports;
}

napi_value DeviceManagerNapi::InitDiscoverModeEnum(napi_env env, napi_value exports)
{
    napi_value discover_mode_passive;
    napi_value discover_mode_active;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(DmDiscoverMode::DM_DISCOVER_MODE_PASSIVE),
        &discover_mode_passive);
    napi_create_uint32(env, static_cast<uint32_t>(DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE),
        &discover_mode_active);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("DISCOVER_MODE_PASSIVE", discover_mode_passive),
        DECLARE_NAPI_STATIC_PROPERTY("DISCOVER_MODE_ACTIVE", discover_mode_active),
    };

    napi_value result = nullptr;
    napi_define_class(env, "DiscoverMode", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &discoverModeEnumConstructor_);
    napi_set_named_property(env, exports, "DiscoverMode", result);
    return exports;
}

napi_value DeviceManagerNapi::InitExchangeMediumEnum(napi_env env, napi_value exports)
{
    napi_value medium_auto;
    napi_value medium_ble;
    napi_value medium_coap;
    napi_value medium_usb;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeMedium::DM_AUTO),
        &medium_auto);
    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeMedium::DM_BLE),
        &medium_ble);
    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeMedium::DM_COAP),
        &medium_coap);
    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeMedium::DM_USB),
        &medium_usb);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("AUTO", medium_auto),
        DECLARE_NAPI_STATIC_PROPERTY("BLE", medium_ble),
        DECLARE_NAPI_STATIC_PROPERTY("COAP", medium_coap),
        DECLARE_NAPI_STATIC_PROPERTY("USB", medium_usb),
    };

    napi_value result = nullptr;
    napi_define_class(env, "ExchangeMedium", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &exchangeMediumEnumConstructor_);
    napi_set_named_property(env, exports, "ExchangeMedium", result);
    return exports;
}

napi_value DeviceManagerNapi::InitExchangeFreqEnum(napi_env env, napi_value exports)
{
    napi_value low;
    napi_value mid;
    napi_value high;
    napi_value super_high;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeFreq::DM_LOW),
        &low);
    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeFreq::DM_MID),
        &mid);
    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeFreq::DM_HIGH),
        &high);
    napi_create_uint32(env, static_cast<uint32_t>(DmExchangeFreq::DM_SUPER_HIGH),
        &super_high);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("LOW", low),
        DECLARE_NAPI_STATIC_PROPERTY("MID", mid),
        DECLARE_NAPI_STATIC_PROPERTY("HIGH", high),
        DECLARE_NAPI_STATIC_PROPERTY("SUPER_HIGH", super_high),
    };

    napi_value result = nullptr;
    napi_define_class(env, "ExchangeFreq", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &exchangeFreqEnumConstructor_);
    napi_set_named_property(env, exports, "ExchangeFreq", result);
    return exports;
}

napi_value DeviceManagerNapi::InitSubscribeCapEnum(napi_env env, napi_value exports)
{
    napi_value subscribe_capability_ddmp;
    napi_value subscribe_capability_osd;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(DM_NAPI_SUBSCRIBE_CAPABILITY_DDMP),
        &subscribe_capability_ddmp);
    napi_create_uint32(env, static_cast<uint32_t>(DM_NAPI_SUBSCRIBE_CAPABILITY_OSD),
        &subscribe_capability_osd);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("SUBSCRIBE_CAPABILITY_DDMP", subscribe_capability_ddmp),
        DECLARE_NAPI_STATIC_PROPERTY("SUBSCRIBE_CAPABILITY_OSD", subscribe_capability_osd),
    };

    napi_value result = nullptr;
    napi_define_class(env, "SubscribeCap", NAPI_AUTO_LENGTH, EnumTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &subscribeCapEnumConstructor_);
    napi_set_named_property(env, exports, "SubscribeCap", result);
    return exports;
}

/*
 * Function registering all props and functions of ohos.distributedhardware
 */
static napi_value Export(napi_env env, napi_value exports)
{
    LOGI("Export() is called!");
    DeviceManagerNapi::Init(env, exports);
    DeviceManagerNapi::InitDeviceTypeEnum(env, exports);
    DeviceManagerNapi::InitDeviceStateChangeActionEnum(env, exports);
    DeviceManagerNapi::InitDiscoverModeEnum(env, exports);
    DeviceManagerNapi::InitExchangeMediumEnum(env, exports);
    DeviceManagerNapi::InitExchangeFreqEnum(env, exports);
    DeviceManagerNapi::InitSubscribeCapEnum(env, exports);
    return exports;
}

/*
 * module define
 */
static napi_module g_dmModule = {.nm_version = 1,
                                 .nm_flags = 0,
                                 .nm_filename = nullptr,
                                 .nm_register_func = Export,
                                 .nm_modname = "distributedHardware.deviceManager",
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
