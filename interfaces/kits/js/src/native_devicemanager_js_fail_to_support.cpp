/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "native_devicemanager_js_fail_to_support.h"
#include "dm_error_type.h"

using namespace OHOS::DistributedHardware;

namespace {
#define DM_NOT_SUPPORT "801"
const std::string ERR_MESSAGE_INVALID_PARAMS = "Input parameter error.";
const std::string DEVICE_MANAGER_NAPI_CLASS_NAME = "DeviceManager";
const int32_t DM_NAPI_ARGS_ONE = 1;
const int32_t DM_NAPI_ARGS_TWO = 2;

#define DM_NAPI_CALL_NORETURN_BASE(theCall)       \
    do {                                         \
        if ((theCall) != napi_ok) {                \
        }                                       \
    } while (0)
#define DM_NAPI_CALL_NO_RETURN(theCall) DM_NAPI_CALL_NORETURN_BASE(theCall)

#define DM_NAPI_CALL_BASE(theCall, result)       \
    do {                                         \
        if ((theCall) != napi_ok) {              \
            return result;                      \
        }                                       \
    } while (0)
#define DM_NAPI_CALL(theCall, result) DM_NAPI_CALL_BASE(theCall, result)

#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    DM_NAPI_CALL(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr), nullptr)

enum class DMBussinessErrorCode : int32_t {
    ERR_INVALID_PARAMS = 401,
};

bool CheckArgsCountLite(napi_env env, bool assertion, const std::string &message)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + message;
        napi_throw_error(env, std::to_string(
            static_cast<int32_t>(DMBussinessErrorCode::ERR_INVALID_PARAMS)).c_str(), errMsg.c_str());
        return false;
    }
    return true;
}

bool CheckArgsTypeLite(napi_env env, bool assertion, const std::string &paramName, const std::string &type)
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

bool IsDeviceManagerNapiNullLite(napi_env env, napi_value thisVar,
    DeviceManagerNapiFailToSupport **pDeviceManagerWrapper)
{
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(pDeviceManagerWrapper));
    if (pDeviceManagerWrapper != nullptr && *pDeviceManagerWrapper != nullptr) {
        return false;
    }
    return true;
}
}

thread_local napi_ref DeviceManagerNapiFailToSupport::sConstructor_ = nullptr;
DeviceManagerNapiFailToSupport::DeviceManagerNapiFailToSupport(
    napi_env env, napi_value thisVar) : DmNativeEvent(env, thisVar)
{
    env_ = env;
}

DeviceManagerNapiFailToSupport::~DeviceManagerNapiFailToSupport()
{
}

napi_value DeviceManagerNapiFailToSupport::GetTrustedDeviceListSync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetTrustedDeviceList(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetLocalDeviceInfoSync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetLocalDeviceInfo(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::UnAuthenticateDevice(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::StartDeviceDiscoverSync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::StopDeviceDiscoverSync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::PublishDeviceDiscoverySync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::UnPublishDeviceDiscoverySync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::AuthenticateDevice(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::RequestCredential(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::ImportCredential(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::DeleteCredential(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsOn(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsOff(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::ReleaseDeviceManager(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    DM_NAPI_CALL(napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr), nullptr);
    DeviceManagerNapiFailToSupport *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNullLite(env, thisVar, &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    napi_get_undefined(env, &result);
    DM_NAPI_CALL(napi_remove_wrap(env, thisVar, (void**)&deviceManagerWrapper), nullptr);
    return result;
}

napi_value DeviceManagerNapiFailToSupport::SetUserOperationSync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetDeviceInfo(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::CreateDeviceManager(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    if (!CheckArgsCountLite(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2")) {
        return nullptr;
    }

    napi_valuetype bundleNameValueType = napi_undefined;
    napi_typeof(env, argv[0], &bundleNameValueType);
    if (!CheckArgsTypeLite(env, bundleNameValueType == napi_string, "bundleName", "string")) {
        return nullptr;
    }

    napi_valuetype funcValueType = napi_undefined;
    napi_typeof(env, argv[1], &funcValueType);
    if (!CheckArgsTypeLite(env, funcValueType == napi_function, "callback", "function")) {
        return nullptr;
    }

    auto *asCallbackInfo = new AsyncCallbackInfoLite();
    if (asCallbackInfo == nullptr) {
        return nullptr;
    }
    asCallbackInfo->env = env;
    napi_get_value_string_utf8(env, argv[0], asCallbackInfo->bundleName, DM_NAPI_BUF_LENGTH - 1,
                               &asCallbackInfo->bundleNameLen);

    napi_create_reference(env, argv[1], 1, &asCallbackInfo->callback);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    delete asCallbackInfo;
    asCallbackInfo = nullptr;
    return result;
}

napi_value DeviceManagerNapiFailToSupport::Constructor(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCountLite(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (!CheckArgsTypeLite(env, valueType == napi_string, "bundleName", "string")) {
        return nullptr;
    }

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], bundleName, sizeof(bundleName), &typeLen);

    DeviceManagerNapiFailToSupport *obj = new DeviceManagerNapiFailToSupport(env, thisVar);
    if (obj == nullptr) {
        return nullptr;
    }

    obj->bundleName_ = std::string(bundleName);
    napi_status status = napi_wrap(
        env, thisVar, reinterpret_cast<void *>(obj),
        [](napi_env env, void *data, void *hint) {
            (void)env;
            (void)hint;
            DeviceManagerNapiFailToSupport *deviceManager = reinterpret_cast<DeviceManagerNapiFailToSupport *>(data);
            delete deviceManager;
            deviceManager = nullptr;
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        delete obj;
        obj = nullptr;
        return nullptr;
    }
    return thisVar;
}

napi_value DeviceManagerNapiFailToSupport::Init(napi_env env, napi_value exports)
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

    DM_NAPI_CALL_NO_RETURN(napi_define_class(env, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Constructor,
                                     nullptr, sizeof(dmProperties) / sizeof(dmProperties[0]), dmProperties, &dmClass));
    DM_NAPI_CALL_NO_RETURN(napi_create_reference(env, dmClass, 1, &sConstructor_));
    DM_NAPI_CALL_NO_RETURN(napi_set_named_property(env, exports, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), dmClass));
    DM_NAPI_CALL_NO_RETURN(napi_define_properties(env, exports,
         sizeof(static_prop) / sizeof(static_prop[0]), static_prop));
    return exports;
}

/*
 * Function registering all props and functions of ohos.distributedhardware
 */
static napi_value Export(napi_env env, napi_value exports)
{
    DeviceManagerNapiFailToSupport::Init(env, exports);
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
    napi_module_register(&g_dmModule);
}
