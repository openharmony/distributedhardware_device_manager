/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dm_device_profile_info.h"
#include "dm_native_event.h"
#include "dm_subscribe_info.h"
#include "dm_publish_info.h"
#include "dm_anonymous.h"
#include "dm_error_message.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "json_object.h"
#define DM_NAPI_BUF_LENGTH (256)

struct AsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t bundleNameLen = 0;

    napi_ref callback = nullptr;
    int32_t status = -1;
    int32_t ret = 0;
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

struct DeviceProfileInfosAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::string bundleName;
    napi_deferred deferred = nullptr;
    int32_t code = -1;
    OHOS::DistributedHardware::DmDeviceProfileInfoFilterOptions filterOptions;
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> deviceProfileInfos;
};

struct GetLocalDisplayDeviceNameAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::string bundleName;
    int32_t maxNameLength = 0;
    napi_deferred deferred = nullptr;
    int32_t code = -1;
    std::string displayName;
};

struct GetDeviceNetworkIdListAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::string bundleName;
    napi_deferred deferred = nullptr;
    int32_t code = -1;
    OHOS::DistributedHardware::NetworkIdQueryFilter filterOptions;
    std::vector<std::string> deviceNetworkIds;
};

struct DeviceIconInfoAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::string bundleName;
    napi_deferred deferred = nullptr;
    int32_t code = -1;
    OHOS::DistributedHardware::DmDeviceIconInfoFilterOptions filterOptions;
    OHOS::DistributedHardware::DmDeviceIconInfo deviceIconInfo;
};

struct PutDeviceProfileInfoListAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::string bundleName;
    napi_deferred deferred = nullptr;
    int32_t code = -1;
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> deviceProfileInfos;
};

struct SetLocalDeviceNameAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::string bundleName = "";
    std::string deviceName = "";
    napi_deferred deferred = nullptr;
    int32_t code = -1;
};

struct SetRemoteDeviceNameAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::string bundleName = "";
    std::string deviceName = "";
    std::string deviceId = "";
    napi_deferred deferred = nullptr;
    int32_t code = -1;
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

class DmNapiGetDeviceProfileInfoListCallback : public OHOS::DistributedHardware::GetDeviceProfileInfoListCallback {
public:
    explicit DmNapiGetDeviceProfileInfoListCallback(napi_env env, const std::string &bundleName,
        const napi_deferred &deferred) : env_(env), bundleName_(bundleName), deferred_(deferred)
    {}
    ~DmNapiGetDeviceProfileInfoListCallback() override {};

    void OnResult(const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfos,
        int32_t code) override;

private:
    napi_env env_;
    std::string bundleName_;
    napi_deferred deferred_ = nullptr;
};

class DmNapiGetDeviceIconInfoCallback : public OHOS::DistributedHardware::GetDeviceIconInfoCallback {
public:
    explicit DmNapiGetDeviceIconInfoCallback(napi_env env, const std::string &bundleName,
        const napi_deferred &deferred) : env_(env), bundleName_(bundleName), deferred_(deferred)
    {
    }
    ~DmNapiGetDeviceIconInfoCallback() override {};

    void OnResult(const OHOS::DistributedHardware::DmDeviceIconInfo &deviceIconInfo, int32_t code) override;

private:
    napi_env env_;
    std::string bundleName_ = "";
    napi_deferred deferred_ = nullptr;
};

class DmNapiSetLocalDeviceNameCallback : public OHOS::DistributedHardware::SetLocalDeviceNameCallback {
public:
    explicit DmNapiSetLocalDeviceNameCallback(napi_env env, const std::string &bundleName,
        const napi_deferred &deferred) : env_(env), bundleName_(bundleName), deferred_(deferred)
    {}
    ~DmNapiSetLocalDeviceNameCallback() override {};

    void OnResult(int32_t code) override;

private:
    napi_env env_;
    std::string bundleName_;
    napi_deferred deferred_ = nullptr;
};

class DmNapiSetRemoteDeviceNameCallback : public OHOS::DistributedHardware::SetRemoteDeviceNameCallback {
public:
    explicit DmNapiSetRemoteDeviceNameCallback(napi_env env, const std::string &bundleName,
        const napi_deferred &deferred) : env_(env), bundleName_(bundleName), deferred_(deferred)
    {}
    ~DmNapiSetRemoteDeviceNameCallback() override {};

    void OnResult(int32_t code) override;

private:
    napi_env env_;
    std::string bundleName_;
    napi_deferred deferred_ = nullptr;
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
    static napi_value JsOn(napi_env env, napi_callback_info info);
    static napi_value JsOff(napi_env env, napi_callback_info info);
    static napi_value JsGetDeviceProfileInfoList(napi_env env, napi_callback_info info);
    static napi_value JsGetDeviceIconInfo(napi_env env, napi_callback_info info);
    static napi_value JsPutDeviceProfileInfoList(napi_env env, napi_callback_info info);
    static napi_value JsGetLocalDisplayDeviceName(napi_env env, napi_callback_info info);
    static napi_value JsGetDeviceNetworkIdList(napi_env env, napi_callback_info info);
    static napi_value JsSetLocalDeviceName(napi_env env, napi_callback_info info);
    static napi_value JsSetRemoteDeviceName(napi_env env, napi_callback_info info);
    static napi_value JsRestoreLocalDeviceName(napi_env env, napi_callback_info info);
    static DeviceManagerNapi *GetDeviceManagerNapi(std::string &bundleName);
    static void CreateDmCallback(napi_env env, std::string &bundleName, std::string &eventType);
    static void CreateDmCallback(napi_env env, std::string &bundleName, std::string &eventType, std::string &extra);
    static void ReleaseDmCallback(std::string &bundleName, std::string &eventType);
    void OnDeviceStatusChange(DmNapiDevStatusChange action,
                             const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo);
    void OnDeviceFound(uint16_t subscribeId, const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo);
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason);
    void OnPublishResult(int32_t publishId, int32_t publishResult);
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status, int32_t reason);
    void OnDmUiCall(const std::string &paramJson);
    void OnGetDeviceProfileInfoListCallbackResult(DeviceProfileInfosAsyncCallbackInfo *jsCallback);
    void OnGetDeviceIconInfoCallbackResult(DeviceIconInfoAsyncCallbackInfo *jsCallback);
    static napi_value InitStrategyForHeartbeatEnum(napi_env env, napi_value exports);
    static napi_value SetHeartbeatPolicy(napi_env env, napi_callback_info info);
    void OnSetLocalDeviceNameCallbackResult(SetLocalDeviceNameAsyncCallbackInfo *jsCallback);
    void OnSetRemoteDeviceNameCallbackResult(SetRemoteDeviceNameAsyncCallbackInfo *jsCallback);

private:
    static void ReleasePublishCallback(std::string &bundleName);
    static void ReleaseDiscoveryCallback(std::string &bundleName);
    static void LockDiscoveryCallbackMutex(napi_env env, std::string &bundleName,
        std::map<std::string, std::string> discParam, std::string &extra, uint32_t subscribeId);
    static void ClearBundleCallbacks(std::string &bundleName);
    static napi_value JsOffFrench(napi_env env, int32_t num, napi_value thisVar, napi_value argv[]);
    static napi_value JsOnFrench(napi_env env, int32_t num, napi_value thisVar, napi_value argv[]);
    static void CallAsyncWork(napi_env env, DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo);
    static void CallGetAvailableDeviceListStatus(napi_env env, napi_status &status,
        DeviceBasicInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo);
    static napi_value CallDeviceList(napi_env env, napi_callback_info info,
        DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo);
    static napi_value GetAvailableDeviceListPromise(napi_env env,
        DeviceBasicInfoListAsyncCallbackInfo *deviceInfoListAsyncCallbackInfo);
    static void BindDevOrTarget(DeviceManagerNapi *deviceManagerWrapper, const std::string &deviceId, napi_env env,
        napi_value &object);
    static int32_t BindTargetWarpper(const std::string &pkgName, const std::string &deviceId,
        const std::string &bindParam, std::shared_ptr<DmNapiBindTargetCallback> callback);
    static void RegisterDevStatusCallback(napi_env env, std::string &bundleName);
    static int32_t DumpDeviceInfo(DeviceBasicInfoListAsyncCallbackInfo *deviceBasicInfoListAsyncCallbackInfo);
    static napi_value GetDeviceProfileInfoListPromise(napi_env env, DeviceProfileInfosAsyncCallbackInfo *jsCallback);
    static napi_value GetDeviceIconInfoPromise(napi_env env, DeviceIconInfoAsyncCallbackInfo *jsCallback);
    static napi_value PutDeviceProfileInfoListPromise(napi_env env,
        PutDeviceProfileInfoListAsyncCallbackInfo *jsCallback);
    static napi_value GetLocalDisplayDeviceNamePromise(napi_env env,
        GetLocalDisplayDeviceNameAsyncCallbackInfo *jsCallback);
    static napi_value GetDeviceNetworkIdListPromise(napi_env env,
        GetDeviceNetworkIdListAsyncCallbackInfo *jsCallback);
    static napi_value SetLocalDeviceNamePromise(napi_env env, SetLocalDeviceNameAsyncCallbackInfo *jsCallback);
    static napi_value SetRemoteDeviceNamePromise(napi_env env, SetRemoteDeviceNameAsyncCallbackInfo *jsCallback);

private:
    napi_env env_;
    static thread_local napi_ref sConstructor_;
    std::string bundleName_;
    static AuthAsyncCallbackInfo authAsyncCallbackInfo_;
};
#endif // OHOS_DM_NATIVE_DEVICEMANAGER_JS_H
