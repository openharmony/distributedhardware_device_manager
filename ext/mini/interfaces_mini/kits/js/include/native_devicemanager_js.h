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

#ifndef OHOS_DEVICE_MANAGER_NATIVE_DEVICEMANAGER_JS_H
#define OHOS_DEVICE_MANAGER_NATIVE_DEVICEMANAGER_JS_H

#include <memory>
#include <string>
#include "device_manager_callback.h"
#include "dm_native_event.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "nlohmann/json.hpp"
#include "dm_device_info.h"
#include "jsi.h"

namespace OHOS {
namespace ACELite {

#define DM_JSI_BUF_LENGTH (256)

struct AuthFuncParams {
    JSIValue handlerRef = JSI::CreateUndefined();
    JSIValue thisVarRef_ = JSI::CreateUndefined();
    const JSIValue *args = nullptr;
    uint8_t argsSize = 0;
};

struct AuthAsyncCallbackInfo {
    JSIValue thisVal_ = JSI::CreateUndefined();

    char bundleName[DM_JSI_BUF_LENGTH] = {0};
    
    JSIValue callback = JSI::CreateUndefined();
    int32_t authType = -1;
};

enum DmJSIDevStateChangeAction {
    ONLINE = 0,
    READY = 1,
    OFFLINE = 2,
    CHANGE = 3
};

class DmJSIInitCallback : public OHOS::DistributedHardware::DmInitCallback {
public:
    explicit DmJSIInitCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmJSIInitCallback() {}
    void OnRemoteDied() override;

private:
    std::string bundleName_;
};

class DmJSIDeviceStateCallback : public OHOS::DistributedHardware::DeviceStateCallback {
public:
    explicit DmJSIDeviceStateCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmJSIDeviceStateCallback() {};
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;

private:
    std::string bundleName_;
};

class DmJSIDiscoverCallback : public OHOS::DistributedHardware::DiscoverCallback {
public:
    explicit DmJSIDiscoverCallback(std::string &bundleName) : refCount_(0), bundleName_(bundleName) {}
    virtual ~DmJSIDiscoverCallback() {};
    void OnDeviceFound(uint16_t subscribeId, const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason) override;
    void OnDiscoverySuccess(uint16_t subscribeId) override;
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();
private:
    //std::atomic<int32_t> refCount_;
    int32_t refCount_ = 0;
    std::string bundleName_;
};

class DmJSIAuthenticateCallback : public OHOS::DistributedHardware::AuthenticateCallback {
public:
    explicit DmJSIAuthenticateCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmJSIAuthenticateCallback() {};
    void OnAuthResult(std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason) override;

private:
    std::string bundleName_;
};

class DmJSICheckAuthCallback : public OHOS::DistributedHardware::CheckAuthCallback {
public:
    explicit DmJSICheckAuthCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmJSICheckAuthCallback() {};
    void OnCheckAuthResult(std::string &deviceId, int32_t resultCode, int32_t flag) override;

private:
    std::string bundleName_;
};

class DmJSIDeviceManagerFaCallback : public OHOS::DistributedHardware::DeviceManagerFaCallback {
public:
    explicit DmJSIDeviceManagerFaCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmJSIDeviceManagerFaCallback() {};
    void OnCall(std::string &paramJson) override;

private:
    std::string bundleName_;
};

class GlobalBundleName {
public:
    std::string GetBundleName()
    {
        return bundleName_;
    }

    void SetBundleName(std::string bundleName)
    {
        bundleName_ = bundleName;
    }
private:
    std::string bundleName_;
};


class DeviceManagerModule final : public MemoryHeap,DmNativeEvent{
public:
     explicit DeviceManagerModule();
     virtual ~DeviceManagerModule();
    static JSIValue CreateDeviceManager(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue ReleaseDeviceManager(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue GetTrustedDeviceListSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue StartDeviceDiscoverSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue StopDeviceDiscoverSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue AuthenticateDevice(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue VerifyAuthInfo(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue JsOn(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue JsOff(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue SetUserOperationSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static JSIValue GetAuthenticationParamSync(const JSIValue thisVal, const JSIValue *args, uint8_t argsSize);
    static DeviceManagerModule *GetDeviceManagerJSI(std::string &bundleName);
    static void AuthRsultVerifyInfoAsyncWorkFunc(void *data);
    static void DmAuthParamToJsAuthParamy(const OHOS::DistributedHardware::DmAuthParam &authParam, JSIValue &paramResult);
    void OnDmfaCall(const std::string &paramJson);
    void OnVerifyResult(const std::string &deviceId, int32_t resultCode, int32_t flag);
    void OnAuthResult(const std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason);
    void OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason);
    void OnDeviceFound(uint16_t subscribeId, const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo);
    void OnDeviceStateChange(DmJSIDevStateChangeAction action,const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo);
    void OnRemoteDied();
    static void JsToDmAuthExtra(const JSIValue &param, nlohmann::json &jsonObj);
    static void JsToDmTokenInfo(const JSIValue &object,const std::string &fieldStr, nlohmann::json &jsonObj);
    static void JsToJsonObject(const JSIValue &object, const std::string &fieldStr, nlohmann::json &jsonObj);
    static void JsToDmBuffer(const JSIValue &object, const std::string &fieldStr, uint8_t **bufferPtr, int32_t &bufferLen);
    static void JsToDmAuthInfo(const JSIValue &object, std::string &extra);
    static void JsToDmAppImageInfoAndDmExtra(const JSIValue &object, OHOS::DistributedHardware::DmAppImageInfo& appImageInfo, std::string &extra, int32_t &authType);
    static void JsToDmDeviceInfo(const JSIValue &object, OHOS::DistributedHardware::DmDeviceInfo& info);
    static int32_t JsToDmSubscribeInfo(const JSIValue &object, OHOS::DistributedHardware::DmSubscribeInfo& info);
    static char *JsObjectToString(const JSIValue &object, const std::string &fieldStr);
    static bool JsObjectToBool(const JSIValue &object, const std::string &fieldStr);
    static int32_t JsObjectToInt(const JSIValue &object, const std::string &fieldStr);
    static void DmAuthParamToJsAuthParam(const OHOS::DistributedHardware::DmAuthParam &authParam, JSIValue &paramResult);
    static void CreateDmCallback(std::string &bundleName, std::string &eventType);
    static void ReleaseDmCallback(std::string &bundleName, std::string &eventType);
    static void DeviceInfoToJsArray(const std::vector<OHOS::DistributedHardware::DmDeviceInfo> &vecDevInfo, const int32_t idx,  JSIValue &arrayResult);

private:
    
    std::string bundleName_;
    static AuthAsyncCallbackInfo authAsyncCallbackInfo_;
    static AuthAsyncCallbackInfo verifyAsyncCallbackInfo_;	
};
void InitDeviceManagerModule(JSIValue exports);
}
}

#endif // OHOS_DEVICE_MANAGER_NATIVE_DEVICEMANAGER_JS_H
