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
#define DM_NOT_SUPPORT "801"

#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr))

#define GET_PARAMS_SYSTEM(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    DM_NAPI_CALL(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr), nullptr)

const std::string DEVICE_MANAGER_NAPI_CLASS_NAME = "DeviceManager";
const std::string ERR_MESSAGE_INVALID_PARAMS = "Input parameter error.";
std::mutex g_deviceManagerMapMutex;

const int32_t MAX_CONTAINER_SIZE_LITE = 10000;
const int32_t DM_NAPI_ARGS_ONE = 1;
std::map<std::string, DeviceManagerNapiFailToSupport *> g_deviceManagerMap;
} // namespace

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

bool CheckArgsValLite(napi_env env, bool assertion, const std::string &param, const std::string &msg)
{
    if (!(assertion)) {
        std::string errMsg = ERR_MESSAGE_INVALID_PARAMS + "The value of " + param + ": " + msg;
        napi_throw_error(env, std::to_string(ERR_INVALID_PARAMS).c_str(), errMsg.c_str());
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

bool JsToStringAndCheckLite(napi_env env, napi_value value, const std::string &valueName, std::string &strValue)
{
    napi_valuetype deviceIdType = napi_undefined;
    napi_typeof(env, value, &deviceIdType);
    if (!CheckArgsTypeLite(env, deviceIdType == napi_string, valueName, "string")) {
        return false;
    }
    size_t valueLen = 0;
    napi_status stat = napi_get_value_string_utf8(env, value, nullptr, 0, &valueLen);
    if (stat != napi_ok) {
        return false;
    }
    if (!CheckArgsValLite(env, valueLen > 0, valueName, "len == 0")) {
        return false;
    }
    if (!CheckArgsValLite(env, valueLen < DM_NAPI_BUF_LENGTH, valueName, "len >= MAXLEN")) {
        return false;
    }
    char temp[DM_NAPI_BUF_LENGTH] = {0};
    napi_status status = napi_get_value_string_utf8(env, value, temp, valueLen + 1, &valueLen);
    if (status != napi_ok) {
        return false;
    }
    strValue = temp;
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

thread_local napi_ref DeviceManagerNapiFailToSupport::sConstructor_ = nullptr;
DeviceManagerNapiFailToSupport::DeviceManagerNapiFailToSupport(
    napi_env env, napi_value thisVar) : DmNativeEvent(env, thisVar)
{
    env_ = env;
}

DeviceManagerNapiFailToSupport::~DeviceManagerNapiFailToSupport()
{
}

napi_value DeviceManagerNapiFailToSupport::SetUserOperationSync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetAvailableDeviceListSync(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetAvailableDeviceList(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetLocalDeviceNetworkId(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetLocalDeviceId(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetLocalDeviceName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetLocalDeviceType(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetDeviceName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetDeviceType(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::StartDeviceDiscover(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::StopDeviceDiscover(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::BindTarget(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::UnBindTarget(napi_env env, napi_callback_info info)
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

napi_value DeviceManagerNapiFailToSupport::JsGetDeviceNetworkIdList(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsGetDeviceProfileInfoList(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsGetDeviceIconInfo(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::SetHeartbeatPolicy(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsPutDeviceProfileInfoList(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsGetLocalDisplayDeviceName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsSetLocalDeviceName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsSetRemoteDeviceName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::JsRestoreLocalDeviceName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::GetIdentificationByDeviceIds(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_throw_error(env, DM_NOT_SUPPORT, "device not support");
    return nullptr;
}

napi_value DeviceManagerNapiFailToSupport::ReleaseDeviceManager(napi_env env, napi_callback_info info)
{
    LOGI("in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCountLite(env, argc == DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    napi_valuetype argvType = napi_undefined;
    napi_typeof(env, argv[0], &argvType);
    if (!CheckArgsTypeLite(env, argvType == napi_object, "DeviceManager", "object")) {
        return nullptr;
    }
    napi_value result = nullptr;
    DeviceManagerNapiFailToSupport *deviceManagerWrapper = nullptr;
    if (IsDeviceManagerNapiNullLite(env, argv[0], &deviceManagerWrapper)) {
        napi_create_uint32(env, ERR_DM_POINT_NULL, &result);
        return result;
    }
    LOGI("bundleName %{public}s", deviceManagerWrapper->bundleName_.c_str());
    napi_get_undefined(env, &result);
    NAPI_CALL(env, napi_remove_wrap(env, argv[0], (void**)&deviceManagerWrapper));
    return result;
}

napi_value DeviceManagerNapiFailToSupport::CreateDeviceManager(napi_env env, napi_callback_info info)
{
    LOGI("In");
    GET_PARAMS_SYSTEM(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCountLite(env, argc == DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    std::string bundleName;
    if (!JsToStringAndCheckLite(env, argv[0], "bundleName", bundleName)) {
        return nullptr;
    }
    napi_value ctor = nullptr;
    napi_value napiName = nullptr;
    napi_value result = nullptr;
    napi_get_reference_value(env, sConstructor_, &ctor);
    napi_create_string_utf8(env, bundleName.c_str(), NAPI_AUTO_LENGTH, &napiName);
    napi_status status = napi_new_instance(env, ctor, DM_NAPI_ARGS_ONE, &napiName, &result);
    if (status != napi_ok) {
        LOGE("Create DeviceManagerNapiFailToSupport for bundleName %{public}s failed", bundleName.c_str());
    }
    return result;
}

napi_value DeviceManagerNapiFailToSupport::Constructor(napi_env env, napi_callback_info info)
{
    LOGI("In");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    if (!CheckArgsCountLite(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1")) {
        return nullptr;
    }
    std::string bundleName;
    if (!JsToStringAndCheckLite(env, argv[0], "bundleName", bundleName)) {
        return nullptr;
    }

    LOGI("Create for packageName:%{public}s", bundleName.c_str());
    DeviceManagerNapiFailToSupport *obj = new DeviceManagerNapiFailToSupport(env, thisVar);
    if (obj == nullptr) {
        return nullptr;
    }

    obj->bundleName_ = bundleName;
    std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
    if (g_deviceManagerMap.size() >= MAX_CONTAINER_SIZE_LITE) {
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
            DeviceManagerNapiFailToSupport *deviceManager = reinterpret_cast<DeviceManagerNapiFailToSupport *>(data);
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

napi_value DeviceManagerNapiFailToSupport::Init(napi_env env, napi_value exports)
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
        DECLARE_NAPI_FUNCTION("setLocalDeviceName", JsSetLocalDeviceName),
        DECLARE_NAPI_FUNCTION("setRemoteDeviceName", JsSetRemoteDeviceName),
        DECLARE_NAPI_FUNCTION("restoreLocalDeviceName", JsRestoreLocalDeviceName),
        DECLARE_NAPI_FUNCTION("restoreLocalDeivceName", JsRestoreLocalDeviceName),
        DECLARE_NAPI_FUNCTION("getDeviceNetworkIdList", JsGetDeviceNetworkIdList),
        DECLARE_NAPI_FUNCTION("getIdentificationByDeviceIds", GetIdentificationByDeviceIds),
        DECLARE_NAPI_FUNCTION("setHeartbeatPolicy", SetHeartbeatPolicy)};

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createDeviceManager", CreateDeviceManager),
        DECLARE_NAPI_STATIC_FUNCTION("releaseDeviceManager", ReleaseDeviceManager),
    };

    LOGI("DeviceManagerNapiFailToSupport::Init() is called!");
    NAPI_CALL(env, napi_define_class(env, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Constructor,
                                     nullptr, sizeof(dmProperties) / sizeof(dmProperties[0]), dmProperties, &dmClass));
    NAPI_CALL(env, napi_create_reference(env, dmClass, 1, &sConstructor_));
    NAPI_CALL(env, napi_set_named_property(env, exports, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), dmClass));
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop));
    LOGI("All props and functions are configured..");
    return exports;
}

static napi_value Export(napi_env env, napi_value exports)
{
    LOGI("Export() is called!");
    DeviceManagerNapiFailToSupport::Init(env, exports);
    return exports;
}

static napi_module g_dmModule = {.nm_version = 1,
                                 .nm_flags = 0,
                                 .nm_filename = nullptr,
                                 .nm_register_func = Export,
                                 .nm_modname = "distributedDeviceManager",
                                 .nm_priv = ((void *)0),
                                 .reserved = {0}};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    LOGI("RegisterModule() is called!");
    napi_module_register(&g_dmModule);
}
