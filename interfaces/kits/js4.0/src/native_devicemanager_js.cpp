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

#include <securec.h>
#include <uv.h>
#include <mutex>

#include "device_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "ipc_skeleton.h"
#include "js_native_api.h"
#include "tokenid_kit.h"
#include "nlohmann/json.hpp"

using namespace OHOS::DistributedHardware;

namespace {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr))

const std::string DM_NAPI_EVENT_DEVICE_STATUS_CHANGE = "deviceStatusChange";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVERY_SUCCESS = "discoverySuccess";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVERY_FAIL = "discoveryFail";
const std::string DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS = "publishSuccess";
const std::string DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL = "publishFail";
const std::string DM_NAPI_EVENT_DEVICE_SERVICE_DIE = "serviceDie";
const std::string DEVICE_MANAGER_NAPI_CLASS_NAME = "DeviceManager";
const std::string DM_NAPI_EVENT_UI_STATE_CHANGE = "uiStateChange";
const std::string DM_NAPI_EVENT_DEVICE_NAME_CHANGE = "deviceNameChange";


const int32_t DM_NAPI_ARGS_ZERO = 0;
const int32_t DM_NAPI_ARGS_ONE = 1;
const int32_t DM_NAPI_ARGS_TWO = 2;
const int32_t DM_NAPI_ARGS_THREE = 3;
const int32_t DM_NAPI_SUB_ID_MAX = 65535;
const int32_t DM_AUTH_DIRECTION_CLIENT = 1;
const int32_t DM_AUTH_REQUEST_SUCCESS_STATUS = 7;

const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_DDMP = 0;
const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_OSD = 1;

napi_ref deviceTypeEnumConstructor_ = nullptr;
napi_ref deviceStateChangeActionEnumConstructor_ = nullptr;
napi_ref discoverModeEnumConstructor_ = nullptr;
napi_ref exchangeMediumEnumConstructor_ = nullptr;
napi_ref exchangeFreqEnumConstructor_ = nullptr;
napi_ref subscribeCapEnumConstructor_ = nullptr;

std::map<std::string, DeviceManagerNapi *> g_deviceManagerMap;
std::map<std::string, std::shared_ptr<DmNapiInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceStatusCallback>> g_deviceStatusCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDiscoveryCallback>> g_DiscoveryCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiPublishCallback>> g_publishCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiAuthenticateCallback>> g_authCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiVerifyAuthCallback>> g_verifyAuthCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceManagerUiCallback>> g_dmUiCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiCredentialCallback>> g_creCallbackMap;

std::mutex g_initCallbackMapMutex;
std::mutex g_deviceManagerMapMutex;

enum DMBussinessErrorCode {
    // Permission verify failed.
    ERR_NO_PERMISSION = 201,
    //The caller is not a system application.
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
        napi_throw_error(env, std::to_string(ERR_INVALID_PARAMS).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

bool CheckArgsCount(napi_env env, bool assertion, const std::string &message)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + message;
        napi_throw_error(env, std::to_string(ERR_INVALID_PARAMS).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

bool CheckArgsType(napi_env env, bool assertion, const std::string &paramName, const std::string &type)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + "The type of " + paramName +
                " must be " + type;
        napi_throw_error(env, std::to_string(ERR_INVALID_PARAMS).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

napi_value CreateErrorForCall(napi_env env, int32_t code, const std::string &errMsg, bool isAsync = true)
{
    LOGI("CreateErrorForCall code:%d, message:%s", code, errMsg.c_str());
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
            error = CreateErrorForCall(env, ERR_NO_PERMISSION, ERR_MESSAGE_NO_PERMISSION, isAsync);
            break;
        case ERR_DM_DISCOVERY_REPEATED:
            error = CreateErrorForCall(env, DM_ERR_DISCOVERY_INVALID, ERR_MESSAGE_DISCOVERY_INVALID, isAsync);
            break;
        case ERR_DM_PUBLISH_REPEATED:
            error = CreateErrorForCall(env, DM_ERR_PUBLISH_INVALID, ERR_MESSAGE_PUBLISH_INVALID, isAsync);
            break;
        case ERR_DM_AUTH_BUSINESS_BUSY:
            error = CreateErrorForCall(env, DM_ERR_AUTHENTICALTION_INVALID,
                ERR_MESSAGE_AUTHENTICALTION_INVALID, isAsync);
            break;
        case ERR_DM_INPUT_PARA_INVALID:
        case ERR_DM_UNSUPPORTED_AUTH_TYPE:
            error = CreateErrorForCall(env, ERR_INVALID_PARAMS, ERR_MESSAGE_INVALID_PARAMS, isAsync);
            break;
        case ERR_DM_INIT_FAILED:
            error = CreateErrorForCall(env, DM_ERR_OBTAIN_SERVICE, ERR_MESSAGE_OBTAIN_SERVICE, isAsync);
            break;
        case ERR_NOT_SYSTEM_APP:
            error = CreateErrorForCall(env, ERR_NOT_SYSTEM_APP, ERR_MESSAGE_NOT_SYSTEM_APP, isAsync);
            break;
        default:
            error = CreateErrorForCall(env, DM_ERR_FAILED, ERR_MESSAGE_FAILED, isAsync);
            break;
    }
    return error;
}

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
AuthAsyncCallbackInfo DeviceManagerNapi::verifyAsyncCallbackInfo_;
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

    DmDeviceBasicInfo info;
    DmNapiStatusJsCallback *jsCallback = new DmNapiStatusJsCallback(bundleName_, 0, 0, info);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnRemoteDied, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnEvent("serviceDie", 0, nullptr);
        }
        LOGI("OnRemoteDied, deviceManagerNapi bundleName %s", callback->bundleName_.c_str());
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    });
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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceOnline, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::UNKNOWN, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    });
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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceReady, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::AVAILABLE, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    });
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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceOffline, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::UNAVAILABLE, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    });
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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceChanged, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceStatusChange(DmNapiDevStatusChange::CHANGE, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    });
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceChanged work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDiscoveryCallback::OnDeviceFound(uint16_t subscribeId,
                                            const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFound for %s, subscribeId %d", bundleName_.c_str(), (int32_t)subscribeId);
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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDeviceFound, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDeviceFound(callback->subscribeId_, callback->deviceBasicInfo_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    });
    if (ret != 0) {
        LOGE("Failed to execute OnDeviceFound work queue");
        DeleteDmNapiStatusJsCallbackPtr(jsCallback);
        DeleteUvWork(work);
    }
}

void DmNapiDiscoveryCallback::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailed for %s, subscribeId %d", bundleName_.c_str(), (int32_t)subscribeId);

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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiStatusJsCallback *callback = reinterpret_cast<DmNapiStatusJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnDiscoveryFailed, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDiscoveryFailed(callback->subscribeId_, callback->reason_);
        }
        DeleteDmNapiStatusJsCallbackPtr(callback);
        DeleteUvWork(work);
    });
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
        LOGE("OnDiscoverySuccess, deviceManagerNapi not find for bundleName %s", bundleName_.c_str());
        return;
    }
    LOGI("DiscoverySuccess for %s, subscribeId %d", bundleName_.c_str(), (int32_t)subscribeId);
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
    LOGI("OnPublishResult for %s, publishId %d, publishResult %d", bundleName_.c_str(), publishId, publishResult);
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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiPublishJsCallback *callback = reinterpret_cast<DmNapiPublishJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnPublishResult, deviceManagerNapi failed for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnPublishResult(callback->publishId_, callback->reason_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    });
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
        LOGE("DmNapiAuthenticateCallback: OnAuthResult, No memory");
        return;
    }

    DmNapiAuthJsCallback *jsCallback = new DmNapiAuthJsCallback(bundleName_, deviceId, token, status, reason);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiAuthJsCallback *callback = reinterpret_cast<DmNapiAuthJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnAuthResult, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnAuthResult(callback->deviceId_, callback->token_,
                callback->status_, callback->reason_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    });
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
        LOGE("DmNapiAuthenticateCallback: OnAuthResult, No memory");
        return;
    }

    DmNapiCredentialJsCallback *jsCallback = new DmNapiCredentialJsCallback(bundleName_, action, credentialResult);
    if (jsCallback == nullptr) {
        delete work;
        work = nullptr;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiCredentialJsCallback *callback = reinterpret_cast<DmNapiCredentialJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnCredentialResult, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnCredentialResult(callback->action_, callback->credentialResult_);
        }
        delete callback;
        callback = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        LOGE("Failed to execute OnCredentialResult work queue");
        delete jsCallback;
        jsCallback = nullptr;
        delete work;
        work = nullptr;
    }
}

void DmNapiVerifyAuthCallback::OnVerifyAuthResult(const std::string &deviceId, int32_t resultCode, int32_t flag)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("DmNapiVerifyAuthCallback: OnVerifyAuthResult, No memory");
        return;
    }

    DmNapiVerifyJsCallback *jsCallback = new DmNapiVerifyJsCallback(bundleName_, deviceId, resultCode, flag);
    if (jsCallback == nullptr) {
        DeleteUvWork(work);
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallback);

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiVerifyJsCallback *callback = reinterpret_cast<DmNapiVerifyJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi =  DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnVerifyAuthResult, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnVerifyResult(callback->deviceId_, callback->resultCode_, callback->flag_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    });
    if (ret != 0) {
        LOGE("Failed to execute OnVerifyAuthResult work queue");
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
    SetValueUtf8String(env_, "deviceId", deviceBasicInfo.deviceId, device);
    SetValueUtf8String(env_, "networkId", deviceBasicInfo.networkId, device);
    SetValueUtf8String(env_, "deviceName", deviceBasicInfo.deviceName, device);
    SetValueInt32(env_, "deviceType", (int)deviceBasicInfo.deviceTypeId, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("deviceStatusChange", DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFound DmDeviceBasicInfo for subscribeId %d", (int32_t)subscribeId);
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "subscribeId", (int)subscribeId, result);

    napi_value device = nullptr;
    napi_create_object(env_, &device);
    SetValueUtf8String(env_, "deviceId", deviceBasicInfo.deviceId, device);
    SetValueUtf8String(env_, "networkId", deviceBasicInfo.networkId, device);
    SetValueUtf8String(env_, "deviceName", deviceBasicInfo.deviceName, device);
    SetValueInt32(env_, "deviceType", (int)deviceBasicInfo.deviceTypeId, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("discoverSuccess", DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailed for subscribeId %d", (int32_t)subscribeId);
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
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
    LOGI("OnPublishResult for publishId %d, publishResult %d", publishId, publishResult);
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

void DeviceManagerNapi::OnCredentialResult(int32_t &action, const std::string &credentialResult)
{
    LOGI("OnCredentialResult for action: %d, credentialResult: %s", action, credentialResult.c_str());
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        LOGE("scope is nullptr");
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
    LOGI("OnAuthResult for status: %d, reason: %d", status, reason);
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

    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_get_reference_value(env_, authAsyncCallbackInfo_.callback, &handler);
    if (handler != nullptr) {
        napi_call_function(env_, nullptr, handler, DM_NAPI_ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env_, authAsyncCallbackInfo_.callback);
    } else {
        LOGE("handler is nullptr");
    }
    napi_close_handle_scope(env_, scope);
    g_authCallbackMap.erase(bundleName_);
}

void DeviceManagerNapi::OnVerifyResult(const std::string &deviceId, int32_t resultCode, int32_t flag)
{
    LOGI("OnVerifyResult for resultCode: %d, flag: %d", resultCode, flag);
    napi_handle_scope scope;
    napi_open_handle_scope(env_, &scope);
    napi_value thisVar = nullptr;
    napi_get_reference_value(env_, thisVarRef_, &thisVar);
    napi_value result[DM_NAPI_ARGS_TWO] = {0};
    if (resultCode == 0) {
        napi_get_undefined(env_, &result[0]);
        napi_create_object(env_, &result[1]);
        SetValueUtf8String(env_, "deviceId", deviceId, result[1]);
        SetValueInt32(env_, "level", flag, result[1]);
    } else {
        napi_create_object(env_, &result[0]);
        SetValueInt32(env_, "code", resultCode, result[0]);
        napi_get_undefined(env_, &result[1]);
    }

    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_get_reference_value(env_, verifyAsyncCallbackInfo_.callback, &handler);
    if (handler != nullptr) {
        napi_call_function(env_, nullptr, handler, DM_NAPI_ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env_, verifyAsyncCallbackInfo_.callback);
    } else {
        LOGE("handler is nullptr");
    }
    napi_close_handle_scope(env_, scope);
    g_verifyAuthCallbackMap.erase(bundleName_);
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

void DeviceManagerNapi::DeviceBasicInfoToJsArray(const napi_env &env,
    const std::vector<DmDeviceBasicInfo> &vecDevInfo, const int32_t idx, napi_value &arrayResult)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    SetValueUtf8String(env, "deviceId", vecDevInfo[idx].deviceId, result);
    SetValueUtf8String(env, "networkId", vecDevInfo[idx].networkId, result);
    SetValueUtf8String(env, "deviceName", vecDevInfo[idx].deviceName, result);
    SetValueInt32(env, "deviceType", vecDevInfo[idx].deviceTypeId, result);

    napi_status status = napi_set_element(env, arrayResult, idx, result);
    if (status != napi_ok) {
        LOGE("DmDeviceBasicInfo To JsArray set element error: %d", status);
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
    SetValueInt32(env, "pinToken", stoi(authParam.authToken), extraInfo);

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
        LOGE("devicemanager napi js to str no property: %s", fieldStr.c_str());
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
        LOGE("devicemanager napi js to int no property: %s", fieldStr.c_str());
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
        LOGE("devicemanager napi js to bool no property: %s", fieldStr.c_str());
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

    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    jsonObj[APP_OPERATION] = appOperationStr;
    jsonObj[CUSTOM_DESCRIPTION] = customDescriptionStr;
    JsToJsonObject(env, object, "extraInfo", jsonObj);
    extra = jsonObj.dump();
    LOGI("appOperationLen %d, customDescriptionLen %d.", appOperationStr.size(), customDescriptionStr.size());
}

void DeviceManagerNapi::JsToBindParam(const napi_env &env, const napi_value &object, std::string &bindParam,
    int32_t &bindType)
{
    LOGI("JsToBindParam in.");
    int32_t bindTypeTemp = -1;
    JsObjectToInt(env, object, "bindType", bindTypeTemp);
    bindType = bindTypeTemp;

    char appOperation[DM_NAPI_DESCRIPTION_BUF_LENGTH] = "";
    JsObjectToString(env, object, "appOperation", appOperation, sizeof(appOperation));
    std::string appOperationStr = appOperation;

    char customDescription[DM_NAPI_DESCRIPTION_BUF_LENGTH] = "";
    JsObjectToString(env, object, "customDescription", customDescription, sizeof(customDescription));
    std::string customDescriptionStr = customDescription;

    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = bindType;
    jsonObj[APP_OPERATION] = appOperationStr;
    jsonObj[CUSTOM_DESCRIPTION] = customDescriptionStr;
    JsToJsonObject(env, object, "bindParam", jsonObj);
    bindParam = jsonObj.dump();
    LOGI("appOperationLen %d, customDescriptionLen %d.", appOperationStr.size(), customDescriptionStr.size());
}

void DeviceManagerNapi::JsToDmBuffer(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                                     uint8_t **bufferPtr, int32_t &bufferLen)
{
    LOGI("JsToDmBuffer in.");
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (!hasProperty) {
        LOGE("devicemanager napi js to str no property: %s", fieldStr.c_str());
        return;
    }

    napi_value field = nullptr;
    napi_get_named_property(env, object, fieldStr.c_str(), &field);
    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    uint8_t *data = nullptr;
    napi_get_typedarray_info(env, field, &type, &length, reinterpret_cast<void **>(&data), &buffer, &offset);
    if (type != napi_uint8_array || length == 0 || data == nullptr) {
        LOGE("Invalid AppIconInfo");
        return;
    }
    *bufferPtr = static_cast<uint8_t *>(calloc(sizeof(uint8_t), length));
    if (*bufferPtr == nullptr) {
        LOGE("low memory, calloc return nullptr, length is %d, filed %s", length, fieldStr.c_str());
        return;
    }
    if (memcpy_s(*bufferPtr, length, data, length) != 0) {
        LOGE("memcpy_s failed, filed %s", fieldStr.c_str());
        free(*bufferPtr);
        *bufferPtr = nullptr;
        return;
    }
    bufferLen = static_cast<int32_t>(length);
}

void DeviceManagerNapi::JsToJsonObject(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                                       nlohmann::json &jsonObj)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (!hasProperty) {
        LOGE("devicemanager napi js to str no property: %s", fieldStr.c_str());
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
                LOGI("Property name = %s, string, value = %s", strProName.c_str(), stringValue.c_str());
                jsonObj[strProName] = stringValue;
                break;
            case napi_boolean:
                napi_get_value_bool(env, jsProValue, &boolValue);
                LOGI("Property name = %s, boolean, value = %d.", strProName.c_str(), boolValue);
                jsonObj[strProName] = boolValue;
                break;
            case napi_number:
                if (napi_get_value_int32(env, jsProValue, &numberValue) != napi_ok) {
                    LOGE("Property name = %s, Property int32_t parse error", strProName.c_str());
                } else {
                    jsonObj[strProName] = numberValue;
                    LOGI("Property name = %s, number, value = %d.", strProName.c_str(), numberValue);
                }
                break;
            default:
                LOGE("Property name = %s, value type not support.", strProName.c_str());
                break;
        }
    }
}

void DeviceManagerNapi::JsToDmAuthInfo(const napi_env &env, const napi_value &object, std::string &extra)
{
    LOGI("%s called.", __func__);
    int32_t authType = -1;
    int32_t token = -1;

    JsObjectToInt(env, object, "authType", authType);
    JsObjectToInt(env, object, "token", token);
    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    jsonObj[PIN_TOKEN] = token;
    JsToJsonObject(env, object, "extraInfo", jsonObj);
    extra = jsonObj.dump();
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
    LOGI("JsToDmDiscoveryExtra, extra :%s, typeLen : %d", extra.c_str(), typeLen);
}

bool DeviceManagerNapi::IsSystemApp()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

void DeviceManagerNapi::CreateDmCallback(napi_env env, std::string &bundleName, std::string &eventType)
{
    LOGI("CreateDmCallback for bundleName %s eventType %s", bundleName.c_str(), eventType.c_str());
    if (eventType == DM_NAPI_EVENT_DEVICE_STATUS_CHANGE) {
        auto callback = std::make_shared<DmNapiDeviceStatusCallback>(env, bundleName);
        std::string extra = "";
        int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName, extra, callback);
        if (ret != 0) {
            LOGE("RegisterDevStatusCallback failed for bundleName %s", bundleName.c_str());
            return;
        }
        g_deviceStatusCallbackMap.erase(bundleName);
        g_deviceStatusCallbackMap[bundleName] = callback;
        return;
    }
    if (eventType == DM_NAPI_EVENT_DEVICE_DISCOVERY_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_DISCOVERY_FAIL) {
        auto callback = std::make_shared<DmNapiDiscoveryCallback>(env, bundleName);
        g_DiscoveryCallbackMap.erase(bundleName);
        g_DiscoveryCallbackMap[bundleName] = callback;
        std::shared_ptr<DmNapiDiscoveryCallback> discoveryCallback = callback;
        discoveryCallback->IncreaseRefCount();
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL) {
        auto callback = std::make_shared<DmNapiPublishCallback>(env, bundleName);
        g_publishCallbackMap.erase(bundleName);
        g_publishCallbackMap[bundleName] = callback;
        std::shared_ptr<DmNapiPublishCallback> publishCallback = callback;
        publishCallback->IncreaseRefCount();
        return;
    }

    if (eventType == DM_NAPI_EVENT_UI_STATE_CHANGE) {
        auto callback = std::make_shared<DmNapiDeviceManagerUiCallback>(env, bundleName);
        int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(bundleName, callback);
        if (ret != 0) {
            LOGE("RegisterDeviceManagerFaCallback failed for bundleName %s", bundleName.c_str());
            return;
        }
        g_dmUiCallbackMap.erase(bundleName);
        g_dmUiCallbackMap[bundleName] = callback;
    }
}

void DeviceManagerNapi::CreateDmCallback(napi_env env, std::string &bundleName,
                                         std::string &eventType, std::string &extra)
{
    LOGI("CreateDmCallback for bundleName %s eventType %s extra = %s",
         bundleName.c_str(), eventType.c_str(), extra.c_str());
    if (eventType == DM_NAPI_EVENT_DEVICE_STATUS_CHANGE) {
        auto callback = std::make_shared<DmNapiDeviceStatusCallback>(env, bundleName);
        int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName, extra, callback);
        if (ret != 0) {
            LOGE("RegisterDevStatusCallback failed for bundleName %s", bundleName.c_str());
            return;
        }
        g_deviceStatusCallbackMap.erase(bundleName);
        g_deviceStatusCallbackMap[bundleName] = callback;
    }
}

void DeviceManagerNapi::ReleasePublishCallback(std::string &bundleName)
{
    std::shared_ptr<DmNapiPublishCallback> publishCallback = nullptr;
    auto iter = g_publishCallbackMap.find(bundleName);
    if (iter == g_publishCallbackMap.end()) {
        return;
    }

    publishCallback = iter->second;
    publishCallback->DecreaseRefCount();
    if (publishCallback->GetRefCount() == 0) {
        g_publishCallbackMap.erase(bundleName);
    }
    return;
}

void DeviceManagerNapi::ReleaseDmCallback(std::string &bundleName, std::string &eventType)
{
    if (eventType == DM_NAPI_EVENT_DEVICE_STATUS_CHANGE) {
        auto iter = g_deviceStatusCallbackMap.find(bundleName);
        if (iter == g_deviceStatusCallbackMap.end()) {
            LOGE("ReleaseDmCallback: cannot find statusCallback for bundleName %s", bundleName.c_str());
            return;
        }
        int32_t ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName);
        if (ret != 0) {
            LOGE("UnRegisterDevStatusCallback failed for bundleName %s", bundleName.c_str());
            return;
        }
        g_deviceStatusCallbackMap.erase(bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_DISCOVERY_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_DISCOVERY_FAIL) {
        std::shared_ptr<DmNapiDiscoveryCallback> DiscoveryCallback = nullptr;
        auto iter = g_DiscoveryCallbackMap.find(bundleName);
        if (iter == g_DiscoveryCallbackMap.end()) {
            return;
        }

        DiscoveryCallback = iter->second;
        DiscoveryCallback->DecreaseRefCount();
        if (DiscoveryCallback->GetRefCount() == 0) {
            g_DiscoveryCallbackMap.erase(bundleName);
        }
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_SUCCESS || eventType == DM_NAPI_EVENT_DEVICE_PUBLISH_FAIL) {
        ReleasePublishCallback(bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_UI_STATE_CHANGE) {
        auto iter = g_dmUiCallbackMap.find(bundleName);
        if (iter == g_dmUiCallbackMap.end()) {
            LOGE("cannot find dmFaCallback for bundleName %s", bundleName.c_str());
            return;
        }
        int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(bundleName);
        if (ret != 0) {
            LOGE("RegisterDevStateCallback failed for bundleName %s", bundleName.c_str());
            return;
        }
        g_dmUiCallbackMap.erase(bundleName);
        return;
    }
}

napi_value DeviceManagerNapi::GetAuthenticationParamSync(napi_env env, napi_callback_info info)
{
    LOGI("yangwei4.0 GetAuthenticationParamSync");
    LOGI("GetAuthenticationParamSync in");
    if (!IsSystemApp()) {
        LOGI("GetAuthenticationParamSync not SystemApp");
        CreateBusinessError(env, ERR_NOT_SYSTEM_APP);
        return nullptr;
    }
    size_t argc = 0;
    napi_value thisVar = nullptr;
    napi_value resultParam = nullptr;
    napi_value result = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNull(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    DmAuthParam authParam;
    int32_t ret = DeviceManager::GetInstance().GetFaParam(deviceManagerWrapper->bundleName_, authParam);
    if (ret != 0) {
        LOGE("GetAuthenticationParam for %s failed, ret %d", deviceManagerWrapper->bundleName_.c_str(), ret);
        napi_get_undefined(env, &resultParam);
        return resultParam;
    }
    napi_create_object(env, &resultParam);
    DmAuthParamToJsAuthParam(env, authParam, resultParam);
    return resultParam;
}

napi_value DeviceManagerNapi::SetUserOperationSync(napi_env env, napi_callback_info info)
{
    LOGI("yangwei4.0 SetUserOperationSync");
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
        LOGE("SetUserOperation for bundleName %s failed, ret %d", deviceManagerWrapper->bundleName_.c_str(), ret);
        CreateBusinessError(env, ret);
    }
    napi_get_undefined(env, &result);
    return result;
}

void DeviceManagerNapi::CallGetAvailableDeviceListStatusSync(napi_env env, napi_status &status,
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    for (unsigned int i = 0; i < deviceBasicInfoListAsyncCallbackInfo->devList.size(); i++) {
        LOGI("DeviceManager::GetAvailableDeviceList deviceId:%s deviceName:%s deviceTypeId:%d ",
             GetAnonyString(deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceId).c_str(),
             deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceName,
             deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceTypeId);
    }

    napi_value array[DM_NAPI_ARGS_TWO] = {0};
    bool isArray = false;
    napi_create_array(env, &array[1]);
    napi_is_array(env, array[1], &isArray);
    if (!isArray) {
        LOGE("napi_create_array fail");
    }
    if (deviceBasicInfoListAsyncCallbackInfo->status == 0) {
        if (deviceBasicInfoListAsyncCallbackInfo->devList.size() > 0) {
            for (unsigned int i = 0; i != deviceBasicInfoListAsyncCallbackInfo->devList.size(); ++i) {
                DeviceBasicInfoToJsArray(env, deviceBasicInfoListAsyncCallbackInfo->devList, (int32_t)i, array[1]);
            }
            LOGI("devList is OK");
        } else {
            LOGE("devList is empty");
        }
        napi_resolve_deferred(env, deviceBasicInfoListAsyncCallbackInfo->deferred, array[1]);
    } else {
        array[0] = CreateBusinessError(env, deviceBasicInfoListAsyncCallbackInfo->ret, false);
        napi_reject_deferred(env, deviceBasicInfoListAsyncCallbackInfo->deferred, array[0]);
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

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        DmNapiAuthJsCallback *callback = reinterpret_cast<DmNapiAuthJsCallback *>(work->data);
        DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(callback->bundleName_);
        if (deviceManagerNapi == nullptr) {
            LOGE("OnCall, deviceManagerNapi not find for bundleName %s", callback->bundleName_.c_str());
        } else {
            deviceManagerNapi->OnDmUiCall(callback->token_);
        }
        delete callback;
        callback = nullptr;
        DeleteUvWork(work);
    });
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
    OnEvent(DM_NAPI_EVENT_UI_STATE_CHANGE, DM_NAPI_ARGS_ONE, &result);
    napi_close_handle_scope(env_, scope);
}

void DeviceManagerNapi::CallGetAvailableDeviceListStatus(napi_env env, napi_status &status,
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    for (unsigned int i = 0; i < deviceBasicInfoListAsyncCallbackInfo->devList.size(); i++) {
        LOGI("DeviceManager::GetAvailableDeviceList deviceId:%s deviceName:%s deviceTypeId:%d ",
             GetAnonyString(deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceId).c_str(),
             deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceName,
             deviceBasicInfoListAsyncCallbackInfo->devList[i].deviceTypeId);
    }
    napi_value callResult = nullptr;
    napi_value handler = nullptr;
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
            LOGE("devList is null"); //CB come here
        }
    } else {
        array[0] = CreateBusinessError(env, deviceBasicInfoListAsyncCallbackInfo->ret, false);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, deviceBasicInfoListAsyncCallbackInfo->callback, &handler));
    if (handler != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, handler, DM_NAPI_ARGS_TWO, &array[0], &callResult));
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, deviceBasicInfoListAsyncCallbackInfo->callback));
    } else {
        LOGE("handler is nullptr");
    }
}

void DeviceManagerNapi::CallRequestCreInfoStatus(napi_env env, napi_status &status,
                                                 CredentialAsyncCallbackInfo *creAsyncCallbackInfo)
{
    LOGI("DeviceManager::RequestCredential Info:%s", creAsyncCallbackInfo->returnJsonStr.c_str());
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
    } else {
        LOGE("handler is nullptr");
    }
}

void DeviceManagerNapi::CallAsyncWorkSync(napi_env env,
    DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "GetTrustListInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            (void)env;
            DeviceBasicInfoListAsyncCallbackInfo *devBasicInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceBasicInfoListAsyncCallbackInfo *>(data);
            int32_t ret = 0;
            ret = DeviceManager::GetInstance().GetAvailableDeviceList(devBasicInfoListAsyncCallbackInfo->bundleName,
                devBasicInfoListAsyncCallbackInfo->devList);
            if (ret != 0) {
                LOGE("CallAsyncWorkSync for bundleName %s failed, ret %d",
                     devBasicInfoListAsyncCallbackInfo->bundleName.c_str(), ret);
                     devBasicInfoListAsyncCallbackInfo->status = -1;
                     devBasicInfoListAsyncCallbackInfo->ret = ret;
            } else {
                devBasicInfoListAsyncCallbackInfo->status = 0;
            }
            LOGI("CallAsyncWorkSync status %d", devBasicInfoListAsyncCallbackInfo->status);
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            DeviceBasicInfoListAsyncCallbackInfo *dBasicInfoListAsyncCallbackInfo =
                reinterpret_cast<DeviceBasicInfoListAsyncCallbackInfo *>(data);
            CallGetAvailableDeviceListStatusSync(env, status, dBasicInfoListAsyncCallbackInfo);
            napi_delete_async_work(env, dBasicInfoListAsyncCallbackInfo->asyncWork);
            delete dBasicInfoListAsyncCallbackInfo;
        },
        (void *)deviceBasicInfoListAsyncCallbackInfo, &deviceBasicInfoListAsyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, deviceBasicInfoListAsyncCallbackInfo->asyncWork);
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
                LOGE("CallAsyncWork for bundleName %s failed, ret %d",
                    devBasicInfoListAsyncCallbackInfo->bundleName.c_str(), ret);
                devBasicInfoListAsyncCallbackInfo->status = -1;
                devBasicInfoListAsyncCallbackInfo->ret = ret;
            } else {
                devBasicInfoListAsyncCallbackInfo->status = 0;
            }
            LOGI("CallAsyncWork status %d , ret %d", devBasicInfoListAsyncCallbackInfo->status,
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
    napi_queue_async_work(env, deviceBasicInfoListAsyncCallbackInfo->asyncWork);
}

void DeviceManagerNapi::AsyncTaskCallback(napi_env env, void *data)
{
    CredentialAsyncCallbackInfo *creAsyncCallbackInfo = reinterpret_cast<CredentialAsyncCallbackInfo *>(data);
    int32_t ret = DeviceManager::GetInstance().RequestCredential(creAsyncCallbackInfo->bundleName,
        creAsyncCallbackInfo->reqInfo, creAsyncCallbackInfo->returnJsonStr);
    if (ret != 0) {
        LOGE("CallCredentialAsyncWork for bundleName %s failed, ret %d",
            creAsyncCallbackInfo->bundleName.c_str(), ret);
        creAsyncCallbackInfo->status = -1;
        creAsyncCallbackInfo->ret = ret;
    } else {
        creAsyncCallbackInfo->status = 0;
    }
    LOGI("CallCredentialAsyncWork status %d", creAsyncCallbackInfo->status);
}

void DeviceManagerNapi::AsyncAfterTaskCallback(napi_env env, napi_status status, void *data)
{
    (void)status;
    CredentialAsyncCallbackInfo *creAsyncCallbackInfo = reinterpret_cast<CredentialAsyncCallbackInfo *>(data);
    CallRequestCreInfoStatus(env, status, creAsyncCallbackInfo);
    napi_delete_async_work(env, creAsyncCallbackInfo->asyncWork);
    delete creAsyncCallbackInfo;
}
