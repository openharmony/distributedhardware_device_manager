/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "native_devicemanager_js.h"
#include "jsi.h"
#include <securec.h>
#include "nlohmann/json.hpp"
#include "device_manager.h"
#include "device_manager_log.h"
#include "constants.h"
#include "jsi_types.h"
#include "js_async_work.h"

using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace ACELite {

const std::string DM_JSI_EVENT_DEVICE_STATE_CHANGE = "deviceStateChange";
const std::string DM_JSI_EVENT_DEVICE_FOUND = "deviceFound";
const std::string DM_JSI_EVENT_DEVICE_DISCOVER_FAIL = "discoverFail";
const std::string DM_JSI_EVENT_DMFA_CALLBACK = "dmFaCallback";
const std::string DM_JSI_EVENT_DEVICE_SERVICE_DIE = "serviceDie";

const std::string DEVICE_MANAGER_JSI_CLASS_NAME = "DeviceManager";
//int32->uint8_t
const uint8_t DM_JSI_ARGS_ONE = 1;
const uint8_t DM_JSI_ARGS_TWO = 2;
const uint8_t DM_JSI_ARGS_THREE = 3;
const int32_t DM_JSI_SUB_ID_MAX = 65535;

const int32_t DM_AUTH_TYPE_PINCODE = 1;
const int32_t DM_AUTH_DIRECTION_CLIENT = 1;

const int32_t DM_JSI_SUBSCRIBE_CAPABILITY_DDMP = 0;
const int32_t DM_JSI_SUBSCRIBE_CAPABILITY_OSD = 1;
const char *DM_CAPABILITY_OSD = "osdCapability";

std::map<std::string, DeviceManagerModule *> g_deviceManagerMap;
std::map<std::string, std::shared_ptr<DmJSIInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmJSIDeviceStateCallback>> g_deviceStateCallbackMap;
std::map<std::string, std::shared_ptr<DmJSIDiscoverCallback>> g_discoverCallbackMap;
std::map<std::string, std::shared_ptr<DmJSIAuthenticateCallback>> g_authCallbackMap;
std::map<std::string, std::shared_ptr<DmJSICheckAuthCallback>> g_checkAuthCallbackMap;
std::map<std::string, std::shared_ptr<DmJSIDeviceManagerFaCallback>> g_dmfaCallbackMap;

AuthAsyncCallbackInfo DeviceManagerModule::authAsyncCallbackInfo_;
AuthAsyncCallbackInfo DeviceManagerModule::verifyAsyncCallbackInfo_;
//add new 
GlobalBundleName globalBundleName_;


DeviceManagerModule *DeviceManagerModule::GetDeviceManagerJSI(std::string &bundleName) 
{
    auto iter = g_deviceManagerMap.find(bundleName);
    if (iter == g_deviceManagerMap.end()) {
        return nullptr;
    }
    return iter->second;
}


DeviceManagerModule::DeviceManagerModule() : DmNativeEvent()
{
     DMLOG(DM_LOG_INFO, "new DeviceManagerModule is success");
}

DeviceManagerModule::~DeviceManagerModule()
{

}

void DmJSIInitCallback::OnRemoteDied()
{ 
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnRemoteDied, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnRemoteDied();
}

void DeviceManagerModule::OnRemoteDied()
{
   OnEvent("serviceDie", 0, nullptr);
}


void DmJSIDeviceStateCallback::OnDeviceOnline(const DmDeviceInfo &deviceInfo)
{
   DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceOnline, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnDeviceStateChange(DmJSIDevStateChangeAction::ONLINE, deviceInfo);
}

void DmJSIDeviceStateCallback::OnDeviceReady(const DmDeviceInfo &deviceInfo)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceOnline, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnDeviceStateChange(DmJSIDevStateChangeAction::READY, deviceInfo);
}

void DmJSIDeviceStateCallback::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceOffline, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnDeviceStateChange(DmJSIDevStateChangeAction::OFFLINE, deviceInfo);
}

void DmJSIDeviceStateCallback::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceChanged, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnDeviceStateChange(DmJSIDevStateChangeAction::CHANGE, deviceInfo);
}

void DmJSIDiscoverCallback::OnDeviceFound(uint16_t subscribeId,  const DmDeviceInfo &deviceInfo)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceFound, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }

    DMLOG(DM_LOG_INFO, "OnDeviceFound for %s, subscribeId %d", bundleName_.c_str(), (int32_t)subscribeId);
    deviceManagerJSI->OnDeviceFound(subscribeId, deviceInfo);
}

void DmJSIDiscoverCallback::OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDiscoverFailed, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }

    deviceManagerJSI->OnDiscoverFailed(subscribeId, failedReason);
}

void DmJSIDiscoverCallback::OnDiscoverySuccess(uint16_t subscribeId)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDiscoverySuccess, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    DMLOG(DM_LOG_INFO, "DiscoverySuccess for %s, subscribeId %d", bundleName_.c_str(), (int32_t)subscribeId);
}


void DmJSIAuthenticateCallback::OnAuthResult(std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnAuthResult, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnAuthResult(deviceId, pinToken, status, reason);
}

void DmJSICheckAuthCallback::OnCheckAuthResult(std::string &deviceId, int32_t resultCode, int32_t flag)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnCheckAuthResult, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnVerifyResult(deviceId, resultCode, flag);
}

void DmJSIDeviceManagerFaCallback::OnCall(std::string &paramJson)
{
    DeviceManagerModule *deviceManagerJSI = DeviceManagerModule::GetDeviceManagerJSI(bundleName_);
    if (deviceManagerJSI == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnCall, deviceManagerJSI not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerJSI->OnDmfaCall(paramJson);
}

void DmJSIDiscoverCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmJSIDiscoverCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmJSIDiscoverCallback::GetRefCount()
{
    return refCount_;
}

void DeviceManagerModule::OnDeviceStateChange(DmJSIDevStateChangeAction action, const DmDeviceInfo &deviceInfo)
{   
   JSIValue result = JSI::CreateObject();
   JSI::SetNumberProperty(result, "action", (double)action);
   
   JSIValue device = JSI::CreateObject();
   JSI::SetStringProperty(device, "deviceId", deviceInfo.deviceId);
   JSI::SetStringProperty(device, "deviceName", deviceInfo.deviceName);
   JSI::SetNumberProperty(device, "deviceTypeId", (double)deviceInfo.deviceTypeId);

   JSIValue param[2] = {result, device};
   OnEvent("deviceStateChange", DM_JSI_ARGS_TWO, param);
   JSI::ReleaseValueList(result, device, ARGS_END);
}

void DeviceManagerModule::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "OnDeviceFound for subscribeId %d", (int32_t)subscribeId);
    JSIValue result = JSI::CreateObject();
    JSI::SetNumberProperty(result, "subscribeId", (double)subscribeId);
    
    JSIValue device = JSI::CreateObject();
    JSI::SetStringProperty(device, "deviceId", deviceInfo.deviceId);
    JSI::SetStringProperty(device, "deviceName", deviceInfo.deviceName);
    JSI::SetNumberProperty(device, "deviceTypeId", (double)deviceInfo.deviceTypeId);
    DMLOG(DM_LOG_INFO, "OnDeviceFound subscribeId %ld ", subscribeId);    
    DMLOG(DM_LOG_INFO, "OnDeviceFound deviceId %s ", deviceInfo.deviceId);    
    DMLOG(DM_LOG_INFO, "OnDeviceFound deviceName %s ", deviceInfo.deviceName);    
    DMLOG(DM_LOG_INFO, "OnDeviceFound deviceTypeId %x ", deviceInfo.deviceTypeId);    
   
    JSIValue param[2] = {result, device};
    OnEvent("deviceFound", DM_JSI_ARGS_TWO, param);
    JSI::ReleaseValueList(result, device, ARGS_END);
}

void DeviceManagerModule::OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason)
{
    DMLOG(DM_LOG_INFO, "OnDiscoverFailed for subscribeId %d", (int32_t)subscribeId);
    JSIValue result = JSI::CreateObject();
    JSI::SetNumberProperty(result, "subscribeId", (double)subscribeId);
    JSIValue reason = JSI::CreateObject();
    JSI::SetNumberProperty(reason, "reason", (double)failedReason);

    JSIValue param[2] = {result, reason};
    OnEvent("discoverFail", DM_JSI_ARGS_TWO, param);
    JSI::ReleaseValueList(result, reason, ARGS_END);
}

void DeviceManagerModule::OnDmfaCall(const std::string &paramJson)
{
    DMLOG(DM_LOG_INFO, "OnCall for paramJson");
    JSIValue result = JSI::CreateObject();
    JSI::SetStringProperty(result, "param", paramJson.c_str());

    JSIValue param[1] = {result};
    OnEvent("dmFaCallback", DM_JSI_ARGS_ONE, param);
    JSI::ReleaseValueList(result, ARGS_END);
}

void DeviceManagerModule::OnAuthResult(const std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason)
{
    DMLOG(DM_LOG_INFO, "OnAuthResult for status: %d, reason: %d", status, reason);
    JSIValue thisVar = authAsyncCallbackInfo_.thisVal_;
    JSIValue success = JSI::GetNamedProperty(authAsyncCallbackInfo_.callback, CB_SUCCESS);
    JSIValue fail = JSI::GetNamedProperty(authAsyncCallbackInfo_.callback, CB_FAIL);

    JSIValue errOne = JSI::CreateObject();
    JSIValue errTwo = JSI::CreateObject();
    JSIValue successOne = JSI::CreateObject();
    JSIValue successTwo = JSI::CreateObject();

    if (status == 0) {
        DMLOG(DM_LOG_INFO, "OnAuthResult success");
        JSI::SetStringProperty(successOne, "deviceId", deviceId.c_str());
        JSI::SetNumberProperty(successTwo, "pinTone", (double)pinToken);
        JSIValue param[2] = {successOne, successTwo};
        AuthFuncParams* params =new AuthFuncParams();
        params->handlerRef = success;
        params->thisVarRef_ = thisVar; 
        params->args = param;
        params->argsSize = DM_JSI_ARGS_TWO;
        DMLOG(DM_LOG_INFO, "OnAuthResult SuccessCallBack in.");
        JsAsyncWork::DispatchAsyncWork(AuthRsultVerifyInfoAsyncWorkFunc, reinterpret_cast<void *>(params));
    } else {
        DMLOG(DM_LOG_INFO, "OnAuthResult failed");
        JSI::SetNumberProperty(errOne, "code", (double)status);
        JSI::SetNumberProperty(errTwo, "reason", (double)reason);
        JSIValue param[2] = {errOne, errTwo};
        AuthFuncParams* params =new AuthFuncParams();
        params->handlerRef = fail;
        params->thisVarRef_ = thisVar; 
        params->args = param;
        params->argsSize = DM_JSI_ARGS_TWO;
        DMLOG(DM_LOG_INFO, "OnAuthResult FailCallBack in.");
        JsAsyncWork::DispatchAsyncWork(AuthRsultVerifyInfoAsyncWorkFunc, reinterpret_cast<void *>(params));
        
    }
    g_authCallbackMap.erase(bundleName_);
    JSI::ReleaseValueList(thisVar, success, fail, errOne, errTwo, successOne, successTwo, authAsyncCallbackInfo_.thisVal_, authAsyncCallbackInfo_.callback, ARGS_END);
}

void DeviceManagerModule::OnVerifyResult(const std::string &deviceId, int32_t resultCode, int32_t flag)
{
    DMLOG(DM_LOG_INFO, "OnVerifyResult for resultCode: %d, flag: %d", resultCode, flag);
    JSIValue thisVar = verifyAsyncCallbackInfo_.thisVal_;
    JSIValue success = JSI::GetNamedProperty(verifyAsyncCallbackInfo_.callback, CB_SUCCESS);
    JSIValue fail = JSI::GetNamedProperty(verifyAsyncCallbackInfo_.callback, CB_FAIL);

    JSIValue successOne = JSI::CreateObject();
    JSIValue successTwo = JSI::CreateObject();
    JSIValue errOne = JSI::CreateObject();

    if (resultCode == 0) {
        DMLOG(DM_LOG_INFO, "OnVerifyResult success");
        JSI::SetStringProperty(successOne, "deviceId", deviceId.c_str());
        JSI::SetNumberProperty(successTwo, "level", (double)flag);
        JSIValue param[2] = {successOne, successTwo};
        AuthFuncParams* params =new AuthFuncParams();
        params->handlerRef = success;
        params->thisVarRef_ = thisVar; 
        params->args = param;
        params->argsSize = DM_JSI_ARGS_TWO;
        DMLOG(DM_LOG_INFO, "OnVerifyResult SuccessCallBack in.");
        JsAsyncWork::DispatchAsyncWork(AuthRsultVerifyInfoAsyncWorkFunc, reinterpret_cast<void *>(params));

    } else {
        DMLOG(DM_LOG_INFO, "OnVerifyResult failed");
        JSI::SetNumberProperty(errOne, "code", (double)resultCode);
        JSIValue param[1] = {errOne};
        AuthFuncParams* params =new AuthFuncParams();
        params->handlerRef = fail;
        params->thisVarRef_ = thisVar; 
        params->args = param;
        params->argsSize = DM_JSI_ARGS_ONE;
        DMLOG(DM_LOG_INFO, "OnVerifyResult FailCallBack in.");
        JsAsyncWork::DispatchAsyncWork(AuthRsultVerifyInfoAsyncWorkFunc, reinterpret_cast<void *>(params));    
    }

    g_checkAuthCallbackMap.erase(bundleName_);
    JSI::ReleaseValueList(thisVar, success, fail, successOne, successTwo, errOne, verifyAsyncCallbackInfo_.thisVal_, verifyAsyncCallbackInfo_.callback, ARGS_END);
}

void DeviceManagerModule::DeviceInfoToJsArray(const std::vector<DmDeviceInfo> &vecDevInfo, const int32_t idx,  JSIValue &arrayResult)
{  
    bool status = false;
    JSIValue result = JSI::CreateObject();
    char *deviceId = const_cast<char *>(vecDevInfo[idx].deviceId);
    char *deviceName = const_cast<char *>(vecDevInfo[idx].deviceName);
    
    JSI::SetStringProperty(result, "deviceId", deviceId);
    JSI::SetStringProperty(result, "deviceName", deviceName);
    JSI::SetNumberProperty(result, "deviceTypeId", (double)vecDevInfo[idx].deviceTypeId);
     
    status = JSI::SetPropertyByIndex(arrayResult,idx,result);
    if (status == false) {
        DMLOG(DM_LOG_ERROR, "DmDeviceInfo To JsArray set element error");
    }
    JSI::ReleaseValue(result);	
}

void DeviceManagerModule::DmAuthParamToJsAuthParamy(const DmAuthParam &authParam, JSIValue &paramResult)
{
    DMLOG(DM_LOG_INFO, "DmAuthParamToJsAuthParamy in");	
    JSI::SetNumberProperty(paramResult,"authType",(double)authParam.authType);
    
    JSIValue extraInfo = JSI::CreateObject();
    JSI::SetNumberProperty(extraInfo,"direction",(double)authParam.direction);
    JSI::SetNumberProperty(extraInfo,"pinToken",(double)authParam.pinToken);
    if (authParam.direction == DM_AUTH_DIRECTION_CLIENT) {
        JSI::SetNamedProperty(paramResult,"extraInfo",extraInfo);
        return;
    }
    JSI::SetStringProperty(extraInfo, "packageName",authParam.packageName.c_str());
    JSI::SetStringProperty(extraInfo, "appName",authParam.appName.c_str());
    JSI::SetStringProperty(extraInfo, "appDescription",authParam.appDescription.c_str());	
    JSI::SetNumberProperty(extraInfo,"business",(double)authParam.business);
    JSI::SetNumberProperty(extraInfo,"pincode",(double)authParam.pincode);
    JSI::SetNamedProperty(paramResult,"extraInfo",extraInfo);
    DMLOG(DM_LOG_INFO, "DeviceManagerModule::DmAuthParamToJsAuthParamy, packageName: %s", authParam.packageName.c_str());
	DMLOG(DM_LOG_INFO, "DeviceManagerModule::DmAuthParamToJsAuthParamy, appName: %s", authParam.appName.c_str());
	DMLOG(DM_LOG_INFO, "DeviceManagerModule::DmAuthParamToJsAuthParamy, appDescription: %s", authParam.appDescription.c_str());
	DMLOG(DM_LOG_INFO, "DeviceManagerModule::DmAuthParamToJsAuthParamy, business: %d",authParam.business);
	DMLOG(DM_LOG_INFO, "DeviceManagerModule::DmAuthParamToJsAuthParamy, pincode: %d", authParam.pincode);
    DMLOG(DM_LOG_INFO, "DeviceManagerModule::DmAuthParamToJsAuthParamy, pinToken: %d", authParam.pinToken);
    
    size_t appIconLen = (size_t)authParam.imageinfo.GetAppIconLen();
    if (appIconLen > 0) {
        uint8_t *appIcon = nullptr;
        JSIValue appIconBuffer = JSI::CreateArrayBuffer(appIconLen, appIcon);
        if (appIcon != nullptr &&
            memcpy_s(appIcon, appIconLen, reinterpret_cast<const void*>(authParam.imageinfo.GetAppIcon()),
                appIconLen) == 0) { 
            JSIValue appIconArray = JSI::CreateTypedArray(TypedArrayType::JSI_UINT8_ARRAY, appIconLen, appIconBuffer, 0);          
            JSI::SetNamedProperty(paramResult,"appIcon",appIconArray);
        }
    }

    size_t appThumbnailLen = (size_t)authParam.imageinfo.GetAppThumbnailLen();
    if (appThumbnailLen > 0) {
        uint8_t *appThumbnail = nullptr;
        JSIValue appThumbnailBuffer = JSI::CreateArrayBuffer(appThumbnailLen, appThumbnail);
        if (appThumbnail != nullptr && memcpy_s(appThumbnail, appThumbnailLen, reinterpret_cast<const void*>(authParam.imageinfo.GetAppThumbnail()), appThumbnailLen) == 0) {
            JSIValue appThumbnailArray = JSI::CreateTypedArray(TypedArrayType::JSI_UINT8_ARRAY, appThumbnailLen, appThumbnailBuffer, 0);
            JSI::SetNamedProperty(paramResult,"appThumbnail", appThumbnailArray);
        }
    }
}

int32_t DeviceManagerModule::JsObjectToInt(const JSIValue &object,
    const std::string &fieldStr)
{
    double result = JSI::GetNumberProperty(object,fieldStr.c_str());
    return (int32_t)result;	
}

bool DeviceManagerModule::JsObjectToBool(const JSIValue &object,
    const std::string &fieldStr)
{
    bool result = JSI::GetBooleanProperty(object,fieldStr.c_str());
    return result;	
}

char *DeviceManagerModule::JsObjectToString(const JSIValue &object,
    const std::string &fieldStr)
{
    char* str = JSI::GetStringProperty(object, fieldStr.c_str());
    return str;	
}

int32_t DeviceManagerModule::JsToDmSubscribeInfo(const JSIValue &object,DmSubscribeInfo &info)
{
    int32_t subscribeId = -1;
    subscribeId = JsObjectToInt(object,"subscribeId");
    if (subscribeId < 0 || subscribeId > DM_JSI_SUB_ID_MAX) {
        DMLOG(DM_LOG_ERROR, "DeviceManagerModule::JsToDmSubscribeInfo, subscribeId error, subscribeId: %d ", subscribeId);
        return -1;
    }
    info.subscribeId = (uint16_t)subscribeId;

    int32_t mode = -1;
    mode = JsObjectToInt(object, "mode");
    info.mode = (DmDiscoverMode)mode;

    int32_t medium = -1;
    medium = JsObjectToInt(object, "medium");
    info.medium = (DmExchangeMedium)medium;

    int32_t freq = -1;
    freq = JsObjectToInt(object, "freq");
    info.freq = (DmExchangeFreq)freq;

    info.isSameAccount = JsObjectToBool(object, "isSameAccount");
    info.isWakeRemote = JsObjectToBool(object, "isWakeRemote");

    int32_t capability = -1;
    capability = JsObjectToInt(object, "capability");
    if (capability == DM_JSI_SUBSCRIBE_CAPABILITY_DDMP || capability == DM_JSI_SUBSCRIBE_CAPABILITY_OSD) {
        (void)strncpy_s(info.capability, sizeof(info.capability), DM_CAPABILITY_OSD, strlen(DM_CAPABILITY_OSD));
    }
    return 0;
}

void DeviceManagerModule::JsToDmDeviceInfo( const JSIValue &object,
    DmDeviceInfo &info)
{   
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, JsObjectToString(object, "deviceId"));
    strcpy_s(info.deviceName, DM_MAX_DEVICE_NAME_LEN, JsObjectToString(object, "deviceName"));
    int32_t deviceTypeId = -1;
    deviceTypeId = (int32_t)JsObjectToInt(object, "deviceTypeId");
    info.deviceTypeId = (DMDeviceType)deviceTypeId;
}

void DeviceManagerModule::JsToDmAppImageInfoAndDmExtra(const JSIValue &object,
    DmAppImageInfo& appImageInfo, std::string &extra, int32_t &authType)
{
    DMLOG(DM_LOG_INFO, "JsToDmAppImageInfoAndDmExtra in.");
    int32_t authTypeTemp = -1;
    authTypeTemp = (int32_t)JsObjectToInt(object, "authType");
    authType = authTypeTemp;

    uint8_t *appIconBufferPtr = nullptr;
    int32_t appIconBufferLen = 0;
    JsToDmBuffer(object, "appIcon", &appIconBufferPtr, appIconBufferLen);

    uint8_t *appThumbnailBufferPtr = nullptr;
    int32_t appThumbnailBufferLen = 0;
    JsToDmBuffer(object, "appThumbnail", &appThumbnailBufferPtr, appThumbnailBufferLen);

    appImageInfo.Reset(appIconBufferPtr, appIconBufferLen, appThumbnailBufferPtr, appThumbnailBufferLen);
    if (appIconBufferPtr != nullptr) {
        free(appIconBufferPtr);
        appIconBufferPtr = nullptr;
    }
    if (appThumbnailBufferPtr != nullptr) {
        free(appThumbnailBufferPtr);
        appThumbnailBufferPtr = nullptr;
    }

    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    std::string extraInfo = "extraInfo";

    JsToJsonObject(object, "extraInfo", jsonObj);
    extra = jsonObj.dump();
    DMLOG(DM_LOG_INFO, "appIconLen %d, appThumbnailLen %d", appIconBufferLen, appThumbnailBufferLen);
}

void DeviceManagerModule::JsToDmBuffer(const JSIValue &object,
    const std::string &fieldStr, uint8_t **bufferPtr, int32_t &bufferLen)
{
    DMLOG(DM_LOG_INFO, "JsToDmBuffer in.");

    JSIValue field = JSI::GetNamedProperty(object, fieldStr.c_str());
    if (field == JSI::CreateUndefined() || field == JSI::CreateNull()){
        DMLOG(DM_LOG_ERROR, "devicemanager JSI js to str no property: %s", fieldStr.c_str());
        return;
    }

    OHOS::ACELite::TypedArrayType type = TypedArrayType::JSI_UINT8_ARRAY;
    size_t length = 0;
    JSIValue buffer = nullptr;
    size_t offset = 0;
    uint8_t *data = nullptr;
    data = JSI::GetTypedArrayInfo(field, type, length, buffer, offset);

    if (type != TypedArrayType::JSI_UINT8_ARRAY || length == 0 || data == nullptr) {
        DMLOG(DM_LOG_ERROR, "Invaild AppIconInfo");
        return;
    }
    *bufferPtr = (uint8_t*)calloc(sizeof(uint8_t), length);
    if (*bufferPtr == nullptr) {
        DMLOG(DM_LOG_ERROR, "low memory, calloc return nullptr, length is %zu,  filed %s", length, fieldStr.c_str());
        return;
    }
    if (memcpy_s(*bufferPtr, length, data, length) != 0) {
        DMLOG(DM_LOG_ERROR, "memcpy_s failed, filed %s", fieldStr.c_str());
        free(*bufferPtr);
        *bufferPtr = nullptr;
        return;
    }
    bufferLen = length;
}

void DeviceManagerModule::JsToJsonObject(const JSIValue &object,
    const std::string &fieldStr, nlohmann::json &jsonObj)
{
    DMLOG(DM_LOG_INFO, "JsToJsonObject in.");
    JSIValue jsonField = JSI::GetNamedProperty(object, fieldStr.c_str());
    if (jsonField == JSI::CreateUndefined() || jsonField == JSI::CreateNull()){
        DMLOG(DM_LOG_ERROR, "devicemanager JSI js to str no property: %s", fieldStr.c_str());
        return;
    }

    JSIValue jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    jsProNameList = JSI::GetObjectKeys(jsonField);
    jsProCount = JSI::GetArrayLength(jsProNameList);
    DMLOG(DM_LOG_INFO, "Property size=%d.", jsProCount);

    JSIValue jsProName = nullptr;
    JSIValue jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        jsProName = JSI::GetPropertyByIndex(jsProNameList, index);
       
        std::string strProName = JSI::ValueToString(jsProName);
        jsProValue = JSI::GetNamedProperty(jsonField, strProName.c_str());
        
        if (JSI::ValueIsString(jsProValue)){
            std::string natValue = JSI::ValueToString(jsProValue);
            DMLOG(DM_LOG_INFO, "Property name=%s, string, value=%s", strProName.c_str(), natValue.c_str());
            jsonObj[strProName] = natValue; 
        }

        if (JSI::ValueIsBoolean(jsProValue)){
            bool elementValue = JSI::ValueToBoolean(jsProValue);
            DMLOG(DM_LOG_INFO, "Property name=%s, boolean, value=%d.", strProName.c_str(), elementValue);
            jsonObj[strProName] = elementValue;
        }

        if (JSI::ValueIsNumber(jsProValue)){
            int32_t elementValue = 0;
            elementValue = (int32_t)JSI::ValueToNumber(jsProValue);
            jsonObj[strProName] = elementValue;
            DMLOG(DM_LOG_INFO, "Property name=%s, number, value=%d.", strProName.c_str(), elementValue);
        }
    }
}

void DeviceManagerModule::JsToDmAuthInfo(const JSIValue &object, std::string &extra)
{
    DMLOG(DM_LOG_INFO, "%s called.", __func__);
    int32_t authType = -1;
    int32_t token = -1;

    authType = JsObjectToInt(object, "authType");
    token = JsObjectToInt(object, "token");

    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    if (authType == DM_AUTH_TYPE_PINCODE) {
        jsonObj[PIN_TOKEN] = token;
    } else {
        jsonObj[TOKEN] = token;
    }
    JsToJsonObject(object, "extraInfo", jsonObj);
    extra = jsonObj.dump();
}

void DeviceManagerModule::CreateDmCallback(std::string &bundleName, std::string &eventType)
{
    DMLOG(DM_LOG_ERROR, "CreateDmCallback for bunderName %s eventType %s", bundleName.c_str(), eventType.c_str());
    if (eventType == DM_JSI_EVENT_DEVICE_STATE_CHANGE) {
        auto iter = g_deviceStateCallbackMap.find(bundleName);
        if (iter == g_deviceStateCallbackMap.end()) {
            auto callback = std::make_shared<DmJSIDeviceStateCallback>(bundleName);
            std::string extra = "";
            int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDevStateCallback(bundleName, extra, callback);
            if (ret != 0) {
                DMLOG(DM_LOG_ERROR, "RegisterDevStateCallback failed for bunderName %s", bundleName.c_str());
                return;
            }
            g_deviceStateCallbackMap[bundleName] = callback;
        }
        return;
    }

    if (eventType == DM_JSI_EVENT_DEVICE_FOUND || eventType == DM_JSI_EVENT_DEVICE_DISCOVER_FAIL) {
        std::shared_ptr<DmJSIDiscoverCallback> discoverCallback = nullptr;
        auto iter = g_discoverCallbackMap.find(bundleName);
        if (iter == g_discoverCallbackMap.end()) {
            auto callback = std::make_shared<DmJSIDiscoverCallback>(bundleName);
            g_discoverCallbackMap[bundleName] = callback;
            discoverCallback = callback;
        } else {
            discoverCallback = iter->second;
        }

        discoverCallback->IncreaseRefCount();
        return;
    }

    if (eventType == DM_JSI_EVENT_DMFA_CALLBACK) {
        auto iter = g_dmfaCallbackMap.find(bundleName);
        if (iter == g_dmfaCallbackMap.end()) {
            auto callback = std::make_shared<DmJSIDeviceManagerFaCallback>(bundleName);
            int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(bundleName, callback);
            if (ret != 0) {
                DMLOG(DM_LOG_ERROR, "RegisterDeviceManagerFaCallback failed for bunderName %s", bundleName.c_str());
                return;
            }
            g_dmfaCallbackMap[bundleName] = callback;
        }
        return;
    }
}

void DeviceManagerModule::ReleaseDmCallback(std::string &bundleName, std::string &eventType)
{
    if (eventType == DM_JSI_EVENT_DEVICE_STATE_CHANGE) {
        auto iter = g_deviceStateCallbackMap.find(bundleName);
        if (iter == g_deviceStateCallbackMap.end()) {
            DMLOG(DM_LOG_ERROR, "ReleaseDmCallback: cannot find stateCallback for bunderName %s", bundleName.c_str());
            return;
        }
        int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDevStateCallback(bundleName);
        if (ret != 0) {
            DMLOG(DM_LOG_ERROR, "RegisterDevStateCallback failed for bunderName %s", bundleName.c_str());
            return;
        }
        g_deviceStateCallbackMap.erase(bundleName);
        return;
    }

    if (eventType == DM_JSI_EVENT_DEVICE_FOUND || eventType == DM_JSI_EVENT_DEVICE_DISCOVER_FAIL) {
        std::shared_ptr<DmJSIDiscoverCallback> discoverCallback = nullptr;
        auto iter = g_discoverCallbackMap.find(bundleName);
        if (iter == g_discoverCallbackMap.end()) {
            return;
        }

        discoverCallback = iter->second;
        discoverCallback->DecreaseRefCount();
        if (discoverCallback->GetRefCount() == 0) {
            g_discoverCallbackMap.erase(bundleName);
        }
        return;
    }

    if (eventType == DM_JSI_EVENT_DMFA_CALLBACK) {
        auto iter = g_dmfaCallbackMap.find(bundleName);
        if (iter == g_dmfaCallbackMap.end()) {
            DMLOG(DM_LOG_ERROR, "cannot find dmFaCallback for bunderName %s", bundleName.c_str());
            return;
        }
        int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(bundleName);
        if (ret != 0) {
            DMLOG(DM_LOG_ERROR, "RegisterDevStateCallback failed for bunderName %s", bundleName.c_str());
            return;
        }
        g_dmfaCallbackMap.erase(bundleName);
        return;
    }
}

void DeviceManagerModule::AuthRsultVerifyInfoAsyncWorkFunc(void *data)
{
    DMLOG(DM_LOG_INFO, "AuthRsultVerifyInfoAsyncWorkFunc in ............");
    AuthFuncParams* params = reinterpret_cast<AuthFuncParams *>(data);
    JSI::CallFunction(params->handlerRef, params->thisVarRef_, params->args, params->argsSize);	
}


JSIValue DeviceManagerModule::SetUserOperationSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "SetUserOperationSync in");
    if (argsSize < 1) {
           DMLOG(DM_LOG_ERROR, "1 argument is required.");
           return JSI::CreateNull();
       }
    
    if (!JSI::ValueIsNumber(args[0])){
        DMLOG(DM_LOG_ERROR, "a Number is required.");
        return JSI::CreateNull();
    }
    
    std::string bundleName = globalBundleName_.GetBundleName();
    int32_t action = 0;
    action = static_cast<int32_t>(JSI::ValueToNumber(args[0]));

    DMLOG(DM_LOG_INFO, "SetUserOperation action %d", action);

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().SetUserOperation(bundleName, action);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "SetUserOperation for bunderName %s failed, ret %d",
        bundleName.c_str(), ret);
        return JSI::CreateNull();
    }

    return JSI::CreateNull();
}

JSIValue DeviceManagerModule::GetAuthenticationParamSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "GetAuthenticationParamSync in");
    std::string bundleName = globalBundleName_.GetBundleName();
    JSIValue resultParam = JSI::CreateObject();
    DmAuthParam authParam;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetAuthenticationParam(bundleName, authParam);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "GetAuthenticationParam for %s failed, ret %d",
            bundleName.c_str(), ret);
        return JSI::CreateNull();
    }
    DmAuthParamToJsAuthParamy(authParam,resultParam);
    return resultParam;
}
  
JSIValue DeviceManagerModule::GetTrustedDeviceListSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "GetTrustedDeviceList in");
    JSIValue array = JSI::CreateNull();
    std::string extra = "";
    std::vector<DmDeviceInfo> devList;
    std::string bundleName = globalBundleName_.GetBundleName();

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(bundleName, extra, devList);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "GetTrustedDeviceList for bunderName %s failed, ret %d",
            bundleName.c_str(), ret);
        return array;
    }
    if (devList.size() > 0) {
        bool isArray = false;
        array = JSI::CreateArray(devList.size());   
        isArray = JSI::ValueIsArray(array);
        if (isArray == false) {
            DMLOG(DM_LOG_ERROR, "JSI_create_array fail");
        }

        for (size_t i = 0; i != devList.size(); ++i) {
            DeviceInfoToJsArray(devList, i, array);
        }
    } else {
        DMLOG(DM_LOG_ERROR, "devList is null");
    }

    return array;
}

JSIValue DeviceManagerModule::StartDeviceDiscoverSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "StartDeviceDiscoverSync in");
    std::string bundleName = globalBundleName_.GetBundleName();
    
    if (argsSize < 1){
        DMLOG(DM_LOG_ERROR, "1 argument is required.");
        return JSI::CreateNull();
    }

    if (!JSI::ValueIsObject(args[0])){
        DMLOG(DM_LOG_ERROR, "a object is required.");
        return JSI::CreateNull();
    }
    
    std::shared_ptr<DmJSIDiscoverCallback> discoverCallback = nullptr;
    auto iter = g_discoverCallbackMap.find(bundleName);
    if (iter == g_discoverCallbackMap.end()) {
        discoverCallback = std::make_shared<DmJSIDiscoverCallback>(bundleName);
        g_discoverCallbackMap[bundleName] = discoverCallback;
    } else {
        discoverCallback = iter->second;
    }
    DmSubscribeInfo subInfo;
    int32_t res = JsToDmSubscribeInfo(args[0], subInfo);
    if (res != 0){
        DMLOG(DM_LOG_ERROR, "Wrong subscribeId.");
        return JSI::CreateNull();
    }

    DMLOG(DM_LOG_INFO, "subInfo %d , %d, %d, %d, %d , %d, %s",  
                        subInfo.subscribeId, 
                        subInfo.mode,
                        subInfo.medium,
                        subInfo.freq,
                        subInfo.isSameAccount,
                        subInfo.isWakeRemote,
                        subInfo.capability);

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().StartDeviceDiscovery(bundleName,
        subInfo, discoverCallback);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "StartDeviceDiscovery for bunderName %s failed, ret %d",
            bundleName.c_str(), ret);
        return JSI::CreateNull();
    }
    return JSI::CreateNull();	
}

JSIValue DeviceManagerModule::StopDeviceDiscoverSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "StopDeviceDiscoverSync in");
    std::string bundleName = globalBundleName_.GetBundleName();

    if (argsSize < 1){
       DMLOG(DM_LOG_ERROR, "1 argument is required.");
       return JSI::CreateNull();
    }

    if (!JSI::ValueIsNumber(args[0])){
       DMLOG(DM_LOG_ERROR, "a Number is required.");
       return JSI::CreateNull();
    }

    uint16_t subscribeId = 0;
    subscribeId = static_cast<uint16_t>(JSI::ValueToNumber(args[0]));
    DMLOG(DM_LOG_INFO, "subscribeId %d ", subscribeId); 

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().StopDeviceDiscovery(bundleName, subscribeId);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "StopDeviceDiscovery for bunderName %s failed, ret %d",
            bundleName.c_str(), ret);
        return JSI::CreateNull();
    }

    return JSI::CreateNull();
}

JSIValue DeviceManagerModule::AuthenticateDevice(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "AuthenticateDevice in");
    std::string bundleName = globalBundleName_.GetBundleName();
    if (argsSize < 3){
        DMLOG(DM_LOG_ERROR, "3 argument is required.");
        return JSI::CreateNull();
    }
    
    if (!JSI::ValueIsObject(args[0])){
        DMLOG(DM_LOG_ERROR, "a object is required.");
        return JSI::CreateNull();
    }

    if (!JSI::ValueIsObject(args[1])){
        DMLOG(DM_LOG_ERROR, "a object is required.");
        return JSI::CreateNull();
    }
    
    
    authAsyncCallbackInfo_.thisVal_ = JSI::AcquireValue(thisVal);
    authAsyncCallbackInfo_.callback = JSI::AcquireValue(args[2]);
   
    std::shared_ptr<DmJSIAuthenticateCallback> authCallback = nullptr;
    auto iter = g_authCallbackMap.find(bundleName);
    if (iter == g_authCallbackMap.end()) {
        authCallback = std::make_shared<DmJSIAuthenticateCallback>(bundleName);
        g_authCallbackMap[bundleName] = authCallback;
    } else {
        authCallback = iter->second;
    }
    DmDeviceInfo deviceInfo;
    JsToDmDeviceInfo(args[0], deviceInfo);

    DMLOG(DM_LOG_INFO, "deviceInfo %s , %s, %d",  
                        deviceInfo.deviceId, 
                        deviceInfo.deviceName,
                        deviceInfo.deviceTypeId);

    DmAppImageInfo appImageInfo(nullptr, 0, nullptr, 0);
    std::string extra;
    JsToDmAppImageInfoAndDmExtra(args[1], appImageInfo, extra, authAsyncCallbackInfo_.authType);

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().AuthenticateDevice(bundleName, deviceInfo,
        appImageInfo, extra, authCallback);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "AuthenticateDevice for bunderName %s failed, ret %d",
            bundleName.c_str(), ret);
    }

    return JSI::CreateUndefined();
}

JSIValue DeviceManagerModule::VerifyAuthInfo(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "VerifyAuthInfo in");
    std::string bundleName = globalBundleName_.GetBundleName();
    if (argsSize < 2){
        DMLOG(DM_LOG_ERROR, "2 argument is required.");
        return JSI::CreateNull();
    }
    
    if (!JSI::ValueIsObject(args[0])){
        DMLOG(DM_LOG_ERROR, "a object is required.");
        return JSI::CreateNull();
    }
    
    verifyAsyncCallbackInfo_.thisVal_ = JSI::AcquireValue(thisVal);
    verifyAsyncCallbackInfo_.callback = JSI::AcquireValue(args[1]);
  
    std::shared_ptr<DmJSICheckAuthCallback> verifyCallback = nullptr;
    auto iter = g_checkAuthCallbackMap.find(bundleName);
    if (iter == g_checkAuthCallbackMap.end()) {
        verifyCallback = std::make_shared<DmJSICheckAuthCallback>(bundleName);
        g_checkAuthCallbackMap[bundleName] = verifyCallback;
    } else {
        verifyCallback = iter->second;
    }
    std::string authParam;
    JsToDmAuthInfo(args[0], authParam);
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().CheckAuthentication(bundleName,
        authParam, verifyCallback);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "VerifyAuthInfo for bunderName %s failed, ret %d",
            bundleName.c_str(), ret);
    }  
    return JSI::CreateUndefined();
}

JSIValue DeviceManagerModule::JsOn(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "JsOn in");
       std::string bundleName = globalBundleName_.GetBundleName();
    if (argsSize < 2){
        DMLOG(DM_LOG_ERROR, "2 argument is required.");
        return JSI::CreateNull();
    }
    if (!JSI::ValueIsString(args[0])){
        DMLOG(DM_LOG_ERROR, "a string is required.");
        return JSI::CreateNull();		
    }
    if (!JSI::ValueIsFunction(args[1])){
        DMLOG(DM_LOG_ERROR, "a FUNC is required.");
        return JSI::CreateNull();
    }
    std::string eventType = JSI::ValueToString(args[0]);
    
    DMLOG(DM_LOG_INFO, "JsOn for bunderName %s, eventType %s ", bundleName.c_str(),
        eventType.c_str());	
     std::shared_ptr<DmNativeEvent> DmNativeEventobj = std::make_shared<DmNativeEvent>(thisVal);      
    DmNativeEventobj->On(eventType, args[1], thisVal);  
    CreateDmCallback(bundleName, eventType);
   
    return JSI::CreateUndefined();
}

JSIValue DeviceManagerModule::JsOff(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "JsOff in");
    std::string bundleName = globalBundleName_.GetBundleName();
    if (argsSize < 2){
        DMLOG(DM_LOG_ERROR, "2 argument is required.");
        return JSI::CreateNull();
    }
       if (!JSI::ValueIsString(args[0])){
        DMLOG(DM_LOG_ERROR, "a string is required.");
        return JSI::CreateNull();
    }
       if (!JSI::ValueIsFunction(args[1])){
        DMLOG(DM_LOG_ERROR, "a FUNC is required.");
        return JSI::CreateNull();
    }
    
    std::string eventType = JSI::ValueToString(args[0]);

    DMLOG(DM_LOG_INFO, "JsOff for bunderName %s, eventType %s ", bundleName.c_str(),
        eventType.c_str());
     
    std::shared_ptr<DmNativeEvent> DmNativeEventobj = std::make_shared<DmNativeEvent>(thisVal);   
    DmNativeEventobj->Off(eventType);  
    ReleaseDmCallback(bundleName, eventType);
   
    return JSI::CreateUndefined();
}

JSIValue DeviceManagerModule::ReleaseDeviceManager(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    DMLOG(DM_LOG_INFO, "ReleaseDeviceManager in");
    std::string bundleName = globalBundleName_.GetBundleName();
    DMLOG(DM_LOG_INFO, "ReleaseDeviceManager for bunderName %s", bundleName.c_str());
    
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnInitDeviceManager(bundleName);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "ReleaseDeviceManager for bunderName %s failed, ret %d",
            bundleName.c_str(), ret);
        JSIValue result = JSI::CreateNumber((double)ret);
        return result;
    }
    
    g_deviceManagerMap.erase(bundleName);
    g_initCallbackMap.erase(bundleName);
    g_deviceStateCallbackMap.erase(bundleName);
    g_discoverCallbackMap.erase(bundleName);
    g_authCallbackMap.erase(bundleName);
    g_checkAuthCallbackMap.erase(bundleName);
    
    return JSI::CreateUndefined();
}

JSIValue DeviceManagerModule::CreateDeviceManager(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize)
{
    if (argsSize < 1) {
        DMLOG(DM_LOG_ERROR,"1 argument is required.");
        return JSI::CreateBoolean(false);
    }
 
    if (!JSI::ValueIsString(args[0])) {
        DMLOG(DM_LOG_ERROR,"a string is required.");
        return JSI::CreateBoolean(false);
    }
    std::string bundleName = JSI::ValueToString(args[0]);;
    globalBundleName_.SetBundleName(bundleName);

    DMLOG(DM_LOG_INFO, "CreateDeviceManager for bunderName %s", bundleName.c_str());

    if (DeviceManagerModule::GetDeviceManagerJSI(bundleName) != nullptr) {
        DMLOG(DM_LOG_INFO, "CreateDeviceManager repeat for bunderName %s", bundleName.c_str());
        return JSI::CreateBoolean(true);
    }

    DeviceManagerModule *obj = new DeviceManagerModule();
    obj->bundleName_ = bundleName;
    g_deviceManagerMap[bundleName] = obj;
    
    std::shared_ptr<DmJSIInitCallback> initCallback = std::make_shared<DmJSIInitCallback>(bundleName);
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(bundleName, initCallback) != 0) {
        DMLOG(DM_LOG_ERROR, "InitDeviceManager for bunderName %s failed", bundleName.c_str());
        return JSI::CreateBoolean(false);
    }

    return JSI::CreateBoolean(true);
}

void InitDeviceManagerModule(JSIValue exports)
{    
    JSI::SetModuleAPI(exports, "createDeviceManager", DeviceManagerModule::CreateDeviceManager);
    JSI::SetModuleAPI(exports, "getTrustedDeviceListSync", DeviceManagerModule::GetTrustedDeviceListSync);
    JSI::SetModuleAPI(exports, "release", DeviceManagerModule::ReleaseDeviceManager);
    JSI::SetModuleAPI(exports, "startDeviceDiscovery", DeviceManagerModule::StartDeviceDiscoverSync);
    JSI::SetModuleAPI(exports, "stopDeviceDiscovery", DeviceManagerModule::StopDeviceDiscoverSync);
    JSI::SetModuleAPI(exports, "authenticateDevice", DeviceManagerModule::AuthenticateDevice);
    JSI::SetModuleAPI(exports, "verifyAuthInfo", DeviceManagerModule::VerifyAuthInfo);
    JSI::SetModuleAPI(exports, "setUserOperation", DeviceManagerModule::SetUserOperationSync);
    JSI::SetModuleAPI(exports, "getAuthenticationParam", DeviceManagerModule::GetAuthenticationParamSync);
    JSI::SetModuleAPI(exports, "on", DeviceManagerModule::JsOn);
    JSI::SetModuleAPI(exports, "off", DeviceManagerModule::JsOff);
}
}
}