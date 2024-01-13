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

#include "dm_native_util.h"

#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "ipc_skeleton.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {

const std::string ERR_MESSAGE_NO_PERMISSION = "Permission verify failed.";
const std::string ERR_MESSAGE_NOT_SYSTEM_APP = "The caller is not a system application.";
const std::string ERR_MESSAGE_INVALID_PARAMS = "Input parameter error.";
const std::string ERR_MESSAGE_FAILED = "Failed to execute the function.";
const std::string ERR_MESSAGE_OBTAIN_SERVICE = "Failed to obtain the service.";
const std::string ERR_MESSAGE_AUTHENTICALTION_INVALID = "Authentication invalid.";
const std::string ERR_MESSAGE_DISCOVERY_INVALID = "Discovery invalid.";
const std::string ERR_MESSAGE_PUBLISH_INVALID = "Publish invalid.";

const int32_t DM_NAPI_DISCOVER_EXTRA_INIT_ONE = -1;
const int32_t DM_NAPI_DISCOVER_EXTRA_INIT_TWO = -2;
const int32_t DM_AUTH_DIRECTION_CLIENT = 1;

void DeviceBasicInfoToJsArray(const napi_env &env,
    const std::vector<DmDeviceBasicInfo> &vecDevInfo, const int32_t idx, napi_value &arrayResult)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    SetValueUtf8String(env, "deviceId", vecDevInfo[idx].deviceId, result);
    SetValueUtf8String(env, "networkId", vecDevInfo[idx].networkId, result);
    SetValueUtf8String(env, "deviceName", vecDevInfo[idx].deviceName, result);
    std::string deviceType = GetDeviceTypeById(static_cast<DmDeviceType>(vecDevInfo[idx].deviceTypeId));
    SetValueUtf8String(env, "deviceType", deviceType.c_str(), result);

    napi_status status = napi_set_element(env, arrayResult, idx, result);
    if (status != napi_ok) {
        LOGE("DmDeviceBasicInfo To JsArray set element error: %d", status);
    }
}

bool DmAuthParamDetection(const DmAuthParam &authParam)
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

void DmAuthParamToJsAuthParam(const napi_env &env, const DmAuthParam &authParam,
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

void SetValueInt32(const napi_env &env, const std::string &fieldStr, const int32_t intValue,
                   napi_value &result)
{
    napi_value value = nullptr;
    napi_create_int32(env, intValue, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

void SetValueUtf8String(const napi_env &env, const std::string &fieldStr, const std::string &str,
                                           napi_value &result)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

void JsObjectToString(const napi_env &env, const napi_value &object, const std::string &fieldStr,
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

std::string JsObjectToString(const napi_env &env, const napi_value &param)
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

void JsObjectToBool(const napi_env &env, const napi_value &object, const std::string &fieldStr,
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

void JsObjectToInt(const napi_env &env, const napi_value &object, const std::string &fieldStr,
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

void JsToDmPublishInfo(const napi_env &env, const napi_value &object, DmPublishInfo &info)
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

void JsToBindParam(const napi_env &env, const napi_value &object, std::string &bindParam,
    int32_t &bindType, bool &isMetaType)
{
    int32_t bindTypeTemp = -1;
    JsObjectToInt(env, object, "bindType", bindTypeTemp);
    bindType = bindTypeTemp;

    char appOperation[DM_NAPI_DESCRIPTION_BUF_LENGTH] = "";
    JsObjectToString(env, object, "appOperation", appOperation, sizeof(appOperation));
    char customDescription[DM_NAPI_DESCRIPTION_BUF_LENGTH] = "";
    JsObjectToString(env, object, "customDescription", customDescription, sizeof(customDescription));
    char targetPkgName[DM_NAPI_BUF_LENGTH] = "";
    JsObjectToString(env, object, "targetPkgName", targetPkgName, sizeof(targetPkgName));
    char metaType[DM_NAPI_BUF_LENGTH] = "";
    JsObjectToString(env, object, "metaType", metaType, sizeof(metaType));
    std::string metaTypeStr = metaType;
    isMetaType = !metaTypeStr.empty();

    char pinCode[DM_NAPI_BUF_LENGTH] = "";
    JsObjectToString(env, object, "pinCode", pinCode, sizeof(pinCode));
    char authToken[DM_NAPI_BUF_LENGTH] = "";
    JsObjectToString(env, object, "authToken", authToken, sizeof(authToken));
    char brMac[DM_NAPI_BUF_LENGTH] = "";
    JsObjectToString(env, object, "brMac", brMac, sizeof(brMac));
    char bleMac[DM_NAPI_BUF_LENGTH] = "";
    JsObjectToString(env, object, "bleMac", bleMac, sizeof(bleMac));
    char wifiIP[DM_NAPI_BUF_LENGTH] = "";
    JsObjectToString(env, object, "wifiIP", wifiIP, sizeof(wifiIP));

    int32_t wifiPort = -1;
    JsObjectToInt(env, object, "wifiPort", wifiPort);
    int32_t bindLevel = 0;
    JsObjectToInt(env, object, "bindLevel", bindLevel);

    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = bindType;
    jsonObj[APP_OPERATION] = std::string(appOperation);
    jsonObj[CUSTOM_DESCRIPTION] = std::string(customDescription);
    jsonObj[PARAM_KEY_TARGET_PKG_NAME] = std::string(targetPkgName);
    jsonObj[PARAM_KEY_META_TYPE] = metaTypeStr;
    jsonObj[PARAM_KEY_PIN_CODE] = std::string(pinCode);
    jsonObj[PARAM_KEY_AUTH_TOKEN] = std::string(authToken);
    jsonObj[PARAM_KEY_BR_MAC] = std::string(brMac);
    jsonObj[PARAM_KEY_BLE_MAC] = std::string(bleMac);
    jsonObj[PARAM_KEY_WIFI_IP] = std::string(wifiIP);
    jsonObj[PARAM_KEY_WIFI_PORT] = wifiPort;
    jsonObj[BIND_LEVEL] = bindLevel;
    jsonObj[TOKENID] = OHOS::IPCSkeleton::GetSelfTokenID();
    bindParam = jsonObj.dump();
}

void JsToDmAuthInfo(const napi_env &env, const napi_value &object, std::string &extra)
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

void JsToJsonObject(const napi_env &env, const napi_value &object, const std::string &fieldStr,
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

void JsToDmDiscoveryExtra(const napi_env &env, const napi_value &object, std::string &extra)
{
    nlohmann::json jsonObj;
    int32_t availableStatus = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    JsObjectToInt(env, object, "availableStatus", availableStatus);
    if (availableStatus != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["credible"] = availableStatus;
    }

    int32_t discoverDistance = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    JsObjectToInt(env, object, "discoverDistance", discoverDistance);
    if (discoverDistance != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["range"] = discoverDistance;
    }

    int32_t authenticationStatus = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    JsObjectToInt(env, object, "authenticationStatus", authenticationStatus);
    if (authenticationStatus != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["isTrusted"] = authenticationStatus;
    }

    int32_t authorizationType = DM_NAPI_DISCOVER_EXTRA_INIT_TWO;
    JsObjectToInt(env, object, "authorizationType", authorizationType);
    if (authorizationType != DM_NAPI_DISCOVER_EXTRA_INIT_TWO) {
        jsonObj["authForm"] = authorizationType;
    }

    int32_t deviceType = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    JsObjectToInt(env, object, "deviceType", deviceType);
    if (deviceType != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["deviceType"] = deviceType;
    }
    extra = jsonObj.dump();
    LOGI("JsToDmDiscoveryExtra, extra :%s", extra.c_str());
}

bool JsToDiscoverTargetType(napi_env env, const napi_value &object, int32_t &discoverTargetType)
{
    napi_valuetype objectType = napi_undefined;
    napi_typeof(env, object, &objectType);
    if (!(CheckArgsType(env, objectType == napi_object, "discoverParameter", "object or undefined"))) {
        return false;
    }
    bool hasProperty = false;
    napi_has_named_property(env, object, "discoverTargetType", &hasProperty);
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;
        napi_get_named_property(env, object, "discoverTargetType", &field);
        napi_typeof(env, field, &valueType);
        if (!CheckArgsType(env, valueType == napi_number, "discoverTargetType", "number")) {
            return false;
        }
        napi_get_value_int32(env, field, &discoverTargetType);
        return true;
    }
    LOGE("discoverTargetType is invalid.");
    return false;
}

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

bool IsSystemApp()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

std::string GetDeviceTypeById(DmDeviceType type)
{
    const static std::pair<DmDeviceType, std::string> mapArray[] = {
        {DEVICE_TYPE_UNKNOWN, DEVICE_TYPE_UNKNOWN_STRING},
        {DEVICE_TYPE_PHONE, DEVICE_TYPE_PHONE_STRING},
        {DEVICE_TYPE_PAD, DEVICE_TYPE_PAD_STRING},
        {DEVICE_TYPE_TV, DEVICE_TYPE_TV_STRING},
        {DEVICE_TYPE_CAR, DEVICE_TYPE_CAR_STRING},
        {DEVICE_TYPE_WATCH, DEVICE_TYPE_WATCH_STRING},
        {DEVICE_TYPE_WIFI_CAMERA, DEVICE_TYPE_WIFICAMERA_STRING},
        {DEVICE_TYPE_PC, DEVICE_TYPE_PC_STRING},
        {DEVICE_TYPE_SMART_DISPLAY, DEVICE_TYPE_SMART_DISPLAY_STRING},
        {DEVICE_TYPE_2IN1, DEVICE_TYPE_2IN1_STRING},
    };
    for (const auto& item : mapArray) {
        if (item.first == type) {
            return item.second;
        }
    }
    return DEVICE_TYPE_UNKNOWN_STRING;
}

bool IsFunctionType(napi_env env, napi_value value)
{
    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, value, &eventHandleType);
    return CheckArgsType(env, eventHandleType == napi_function, "callback", "function");
}

void InsertMapParames(nlohmann::json &bindParamObj, std::map<std::string, std::string> &bindParamMap)
{
    LOGI("Insert map parames start");
    if (IsInt32(bindParamObj, AUTH_TYPE)) {
        int32_t authType = bindParamObj[AUTH_TYPE].get<int32_t>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TYPE, std::to_string(authType)));
    }
    if (IsString(bindParamObj, APP_OPERATION)) {
        std::string appOperation = bindParamObj[APP_OPERATION].get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_APP_OPER, appOperation));
    }
    if (IsString(bindParamObj, CUSTOM_DESCRIPTION)) {
        std::string appDescription = bindParamObj[CUSTOM_DESCRIPTION].get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_APP_DESC, appDescription));
    }
    if (IsString(bindParamObj, PARAM_KEY_TARGET_PKG_NAME)) {
        std::string targetPkgName = bindParamObj[PARAM_KEY_TARGET_PKG_NAME].get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_TARGET_PKG_NAME, targetPkgName));
    }
    if (IsString(bindParamObj, PARAM_KEY_META_TYPE)) {
        std::string metaType = bindParamObj[PARAM_KEY_META_TYPE].get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_META_TYPE, metaType));
    }
    if (IsString(bindParamObj, PARAM_KEY_PIN_CODE)) {
        std::string pinCode = bindParamObj[PARAM_KEY_PIN_CODE].get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_PIN_CODE, pinCode));
    }
    if (IsString(bindParamObj, PARAM_KEY_AUTH_TOKEN)) {
        std::string authToken = bindParamObj[PARAM_KEY_AUTH_TOKEN].get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TOKEN, authToken));
    }
}
} // namespace DistributedHardware
} // namespace OHOS
