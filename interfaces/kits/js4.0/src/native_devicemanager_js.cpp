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
    if (eventType == DM_NAPI_EVENT_DEVICE_NAME_CHANGE) {

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