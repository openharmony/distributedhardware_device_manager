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