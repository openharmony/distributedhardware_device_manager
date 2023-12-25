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

#ifndef OHOS_DM_NATIVE_DEVICEMANAGER_JS_H
#define OHOS_DM_NATIVE_DEVICEMANAGER_JS_H

#include <memory>
#include <string>
#include <mutex>

#include "device_manager_callback.h"
#include "dm_app_image_info.h"
#include "dm_device_info.h"
#include "dm_native_event.h"
#include "dm_subscribe_info.h"
#include "dm_publish_info.h"
#include "dm_anonymous.h"
#include "dm_error_message.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "nlohmann/json.hpp"
#define DM_NAPI_BUF_LENGTH (256)
#define DM_NAPI_CREDENTIAL_BUF_LENGTH (6000)
#define DM_NAPI_DESCRIPTION_BUF_LENGTH (16384)

struct AsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t bundleNameLen = 0;

    napi_ref callback = nullptr;
    int32_t status = -1;
    int32_t ret = 0;
};

struct CredentialAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;

    std::string bundleName;
    std::string reqInfo;
    std::string returnJsonStr;
    int32_t status = -1;
    int32_t ret = 0;
    napi_ref callback = nullptr;
};

struct DeviceBasicInfoListAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;

    std::string bundleName;
    size_t bundleNameLen = 0;
    std::vector<OHOS::DistributedHardware::DmDeviceBasicInfo> devList;
    std::string extra;
    // OHOS::DistributedHardware::DmFilterOptions filter;
    napi_ref callback = nullptr;
    napi_value thisVar = nullptr;
    napi_deferred deferred = nullptr;
    int32_t status = -1;
    int32_t ret = 0;
};

struct AuthAsyncCallbackInfo {
    napi_env env = nullptr;

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t bundleNameLen = 0;

    napi_ref callback = nullptr;
    int32_t authType = -1;
};

struct DmNapiStatusJsCallback {
    std::string bundleName_;
    uint16_t subscribeId_;
    int32_t reason_;
    OHOS::DistributedHardware::DmDeviceBasicInfo deviceBasicInfo_;

    DmNapiStatusJsCallback(std::string bundleName, uint16_t subscribeId, int32_t reason,
        OHOS::DistributedHardware::DmDeviceBasicInfo deviceBasicInfo_)
        : bundleName_(bundleName), subscribeId_(subscribeId), reason_(reason), deviceBasicInfo_(deviceBasicInfo_) {}
};

struct DmNapiPublishJsCallback {
    std::string bundleName_;
    int32_t publishId_;
    int32_t reason_;

    DmNapiPublishJsCallback(std::string bundleName, int32_t publishId, int32_t reason)
        : bundleName_(bundleName), publishId_(publishId), reason_(reason) {}
};

struct DmNapiCredentialJsCallback {
    std::string bundleName_;
    int32_t action_;
    std::string credentialResult_;

    DmNapiCredentialJsCallback(std::string bundleName, int32_t action, std::string credentialResult)
        : bundleName_(bundleName), action_(action), credentialResult_(credentialResult) {}
};

struct DmNapiAuthJsCallback {
    std::string bundleName_;
    std::string deviceId_;
    std::string token_;
    int32_t status_;
    int32_t reason_;

    DmNapiAuthJsCallback(std::string bundleName, std::string deviceId, std::string token, int32_t status,
        int32_t reason)
        : bundleName_(bundleName), deviceId_(deviceId), token_(token), status_(status), reason_(reason) {}
};

enum DmNapiDevStatusChange { UNKNOWN = 0, AVAILABLE = 1, UNAVAILABLE = 2, CHANGE = 3};

class DmNapiInitCallback : public OHOS::DistributedHardware::DmInitCallback {
public:
    explicit DmNapiInitCallback(napi_env env, std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    ~DmNapiInitCallback() override {}
    void OnRemoteDied() override;

private:
    napi_env env_;
    std::string bundleName_;
};

class DmNapiDeviceStatusCallback : public OHOS::DistributedHardware::DeviceStatusCallback {
public:
    explicit DmNapiDeviceStatusCallback(napi_env env, std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    ~DmNapiDeviceStatusCallback() override {};
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
private:
    napi_env env_;
    std::string bundleName_;
};

class DmNapiDiscoveryCallback : public OHOS::DistributedHardware::DiscoveryCallback {
public:
    explicit DmNapiDiscoveryCallback(napi_env env, std::string &bundleName)
        : env_(env), refCount_(0), bundleName_(bundleName)
    {
    }
    ~DmNapiDiscoveryCallback() override {};
    void OnDeviceFound(uint16_t subscribeId,
        const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override;
    void OnDiscoverySuccess(uint16_t subscribeId) override;
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();

private:
    napi_env env_;
    std::atomic<int32_t> refCount_;
    std::string bundleName_;
};

class DmNapiPublishCallback : public OHOS::DistributedHardware::PublishCallback {
public:
    explicit DmNapiPublishCallback(napi_env env, std::string &bundleName)
        : env_(env), refCount_(0), bundleName_(bundleName)
    {
    }
    ~DmNapiPublishCallback() override {};
    void OnPublishResult(int32_t publishId, int32_t publishResult) override;
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();

private:
    napi_env env_;
    std::atomic<int32_t> refCount_;
    std::string bundleName_;
};

class DmNapiDeviceManagerUiCallback : public OHOS::DistributedHardware::DeviceManagerUiCallback {
public:
    explicit DmNapiDeviceManagerUiCallback(napi_env env, std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    ~DmNapiDeviceManagerUiCallback() override {};
    void OnCall(const std::string &paramJson) override;

private:
    napi_env env_;
    std::string bundleName_;
};

class DmNapiCredentialCallback : public OHOS::DistributedHardware::CredentialCallback {
public:
    explicit DmNapiCredentialCallback(napi_env env, const std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    ~DmNapiCredentialCallback() override {};
    void OnCredentialResult(int32_t &action, const std::string &credentialResult) override;

private:
    napi_env env_;
    std::string bundleName_;
};

class DmNapiAuthenticateCallback : public OHOS::DistributedHardware::AuthenticateCallback {
public:
    explicit DmNapiAuthenticateCallback(napi_env env, std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    ~DmNapiAuthenticateCallback() override {};
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status, int32_t reason) override;

private:
    napi_env env_;
    std::string bundleName_;
};

class DmNapiBindTargetCallback : public OHOS::DistributedHardware::BindTargetCallback {
public:
    explicit DmNapiBindTargetCallback(napi_env env, std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    ~DmNapiBindTargetCallback() override {};
    void OnBindResult(const OHOS::DistributedHardware::PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) override;

private:
    napi_env env_;
    std::string bundleName_;
};

class DeviceManagerNapi : public DmNativeEvent {
public:
    explicit DeviceManagerNapi(napi_env env, napi_value thisVar);
    ~DeviceManagerNapi() override;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value EnumTypeConstructor(napi_env env, napi_callback_info info);
    static napi_value InitDeviceStatusChangeActionEnum(napi_env env, napi_value exports);
    static napi_value CreateDeviceManager(napi_env env, napi_callback_info info);
    static napi_value ReleaseDeviceManager(napi_env env, napi_callback_info info);
    static napi_value SetUserOperationSync(napi_env env, napi_callback_info info);
    static napi_value GetAvailableDeviceListSync(napi_env env, napi_callback_info info);
    static napi_value GetAvailableDeviceList(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceNetworkId(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceId(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceName(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceType(napi_env env, napi_callback_info info);
    static napi_value GetDeviceName(napi_env env, napi_callback_info info);
    static napi_value GetDeviceType(napi_env env, napi_callback_info info);
    static napi_value StartDeviceDiscover(napi_env env, napi_callback_info info);
    static napi_value StopDeviceDiscover(napi_env env, napi_callback_info info);
    static napi_value PublishDeviceDiscoverySync(napi_env env, napi_callback_info info);
    static napi_value UnPublishDeviceDiscoverySync(napi_env env, napi_callback_info info);
    static napi_value BindTarget(napi_env env, napi_callback_info info);
    static napi_value UnBindTarget(napi_env env, napi_callback_info info);
    static napi_value RequestCredential(napi_env env, napi_callback_info info);
    static napi_value ImportCredential(napi_env env, napi_callback_info info);
    static napi_value DeleteCredential(napi_env env, napi_callback_info info);
    static napi_value JsOn(napi_env env, napi_callback_info info);
    static napi_value JsOff(napi_env env, napi_callback_info info);
    static DeviceManagerNapi *GetDeviceManagerNapi(std::string &bundleName);
    static void CreateDmCallback(napi_env env, std::string &bundleName, std::string &eventType);
    static void CreateDmCallback(napi_env env, std::string &bundleName, std::string &eventType, std::string &extra);
    static void ReleaseDmCallback(std::string &bundleName, std::string &eventType);
    static void DeviceBasicInfoToJsArray(const napi_env &env,
                                    const std::vector<OHOS::DistributedHardware::DmDeviceBasicInfo> &vecDevInfo,
                                    const int32_t idx, napi_value &arrayResult);
    static bool DmAuthParamDetection(const OHOS::DistributedHardware::DmAuthParam &authParam);
    static void DmAuthParamToJsAuthParam(const napi_env &env, const OHOS::DistributedHardware::DmAuthParam &authParam,
                                         napi_value &paramResult);
    static void SetValueInt32(const napi_env &env, const std::string &fieldStr, const int32_t intValue,
                              napi_value &result);
    static void SetValueUtf8String(const napi_env &env, const std::string &fieldStr, const std::string &str,
                                   napi_value &result);
    static void JsObjectToString(const napi_env &env, const napi_value &object, const std::string &fieldStr, char *dest,
                                 const int32_t destLen);
    static void JsObjectToBool(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                               bool &fieldRef);
    static void JsObjectToInt(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                              int32_t &fieldRef);
    static std::string JsObjectToString(const napi_env &env, const napi_value &param);
    static void JsToDmPublishInfo(const napi_env &env, const napi_value &object,
                                  OHOS::DistributedHardware::DmPublishInfo &info);
    static void JsToBindParam(const napi_env &env, const napi_value &object, std::string &bindParam, int32_t &bindType,
        bool &isMetaType);
    static void JsToDmAuthInfo(const napi_env &env, const napi_value &object, std::string &extra);
    static void JsToJsonObject(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                               nlohmann::json &jsonObj);
    static void JsToDmDiscoveryExtra(const napi_env &env, const napi_value &object, std::string &extra);
    static bool JsToDiscoverTargetType(napi_env env, const napi_value &object, int32_t &discoverTargetType);
    static int32_t RegisterCredentialCallback(napi_env env, const std::string &pkgName);
    static void AsyncAfterTaskCallback(napi_env env, napi_status status, void *data);
    static void AsyncTaskCallback(napi_env env, void *data);
    void OnDeviceStatusChange(DmNapiDevStatusChange action,
                             const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo);
    void OnDeviceFound(uint16_t subscribeId, const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo);
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason);
    void OnPublishResult(int32_t publishId, int32_t publishResult);
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status, int32_t reason);
    void OnCredentialResult(int32_t &action, const std::string &credentialResult);
    void OnDmUiCall(const std::string &paramJson);

private:
    static void ReleasePublishCallback(std::string &bundleName);
    static napi_value JsOffFrench(napi_env env, int32_t num, napi_value thisVar, napi_value argv[]);
    static napi_value JsOnFrench(napi_env env, int32_t num, napi_value thisVar, napi_value argv[]);
    static void CallAsyncWorkSync(napi_env env,
        DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo);
    static void CallAsyncWork(napi_env env, DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo);
    static void CallCredentialAsyncWork(napi_env env, CredentialAsyncCallbackInfo *creAsyncCallbackInfo);
    static void CallRequestCreInfoStatus(napi_env env, napi_status &status,
        CredentialAsyncCallbackInfo *creAsyncCallbackInfo);
    static void CallGetAvailableDeviceListStatusSync(napi_env env, napi_status &status,
        DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo);
    static void CallGetAvailableDeviceListStatus(napi_env env, napi_status &status,
        DeviceBasicInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo);
    static napi_value CallDeviceList(napi_env env, napi_callback_info info,
        DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo);
    static napi_value GetAvailableDeviceListPromise(napi_env env,
        DeviceBasicInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo);
    static napi_value GetAvailableDeviceListByFilter(napi_env env, napi_callback_info info,
        DeviceBasicInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo);
    static void BindDevOrTarget(DeviceManagerNapi *deviceManagerWrapper, const std::string &deviceId, napi_env env,
        napi_value &object);
    static bool IsSystemApp();
    static std::string GetDeviceTypeById(OHOS::DistributedHardware::DmDeviceType type);
    static int32_t BindTargetWarpper(const std::string &pkgName, const std::string &deviceId,
        const std::string &bindParam, std::shared_ptr<DmNapiBindTargetCallback> callback);
    static void InsertMapParames(nlohmann::json &bindParamObj, std::map<std::string, std::string> &bindParamMap);

private:
    napi_env env_;
    static thread_local napi_ref sConstructor_;
    std::string bundleName_;
    static std::mutex creMapLocks_;
    static AuthAsyncCallbackInfo authAsyncCallbackInfo_;
    static CredentialAsyncCallbackInfo creAsyncCallbackInfo_;
};
#endif // OHOS_DM_NATIVE_DEVICEMANAGER_JS_H
