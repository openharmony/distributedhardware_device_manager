/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_ANI_CALLBACK_H
#define OHOS_DM_ANI_CALLBACK_H

#include <atomic>
#include <string>
#include "ani_error_utils.h"
#include "device_manager_callback.h"
#include "device_manager_impl.h"
#include "dm_device_info.h"
#include "dm_error_message.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "event_handler.h"
#include "event_runner.h"
#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "taihe/callback.hpp"

const int32_t DM_AUTH_REQUEST_SUCCESS_STATUS = 7;

class DmAniAuthenticateCallback;
namespace ANI::distributedDeviceManager {
extern std::mutex g_authCallbackMapMutex;
extern std::map<std::string, std::shared_ptr<DmAniAuthenticateCallback>> g_authCallbackMap;
}

class AsyncUtilBase {
public:
    AsyncUtilBase() = default;
    virtual ~AsyncUtilBase() {}
    static bool AsyncExecue(const std::function<void()> func);
    bool SendEventToMainThread(const std::function<void()> func);

protected:
    std::mutex jsCallbackMutex_;
    static std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_;
};

class DmAniInitCallback : public OHOS::DistributedHardware::DmInitCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniInitCallback> {
public:
    explicit DmAniInitCallback(ani_env* env, taihe::string_view bundleName);
    ~DmAniInitCallback() override {}
    void OnRemoteDied() override;
    void OnRemoteDiedInMainthread();
    void SetServiceDieCallback(taihe::callback<void(uintptr_t data)> callback);
    void ReleaseServiceDieCallback();

private:
    ani_env* env_;
    std::string bundleName_;
    std::optional<taihe::callback<void(uintptr_t data)>> serviceDieCallback_;
    std::mutex g_dmInitMutex;
};

using JsDiscoverSuccessCallback = taihe::callback<void(ohos::distributedDeviceManager::DiscoverySuccessResult const&)>;
using JsDiscoverFailedCallback = taihe::callback<void(ohos::distributedDeviceManager::DiscoveryFailureResult const&)>;

class DmAniDiscoveryCallback : public OHOS::DistributedHardware::DiscoveryCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniDiscoveryCallback> {
public:
    explicit DmAniDiscoveryCallback(std::string &bundleName): bundleName_(bundleName) {};
    ~DmAniDiscoveryCallback() override {};
    void SetSuccessCallback(JsDiscoverSuccessCallback discoverSuccessCallback);
    void ResetSuccessCallback();
    void SetFailedCallback(JsDiscoverFailedCallback discoverFailedCallback);
    void ResetFailedCallback();
    void OnDeviceFound(uint16_t subscribeId,
        const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override;
    void OnDiscoverySuccess(uint16_t subscribeId) override;
    void OnDeviceFoundInMainThread(uint16_t subscribeId,
        const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo);
    void OnDiscoveryFailedInMainThread(uint16_t subscribeId, int32_t failedReason);
    int32_t GetRefCount();

private:
    std::string bundleName_;
    std::optional<taihe::callback<void(
        ohos::distributedDeviceManager::DiscoverySuccessResult const&)>> discoverSuccessCallback_;
    std::optional<taihe::callback<void(
        ohos::distributedDeviceManager::DiscoveryFailureResult const&)>> discoverFailedCallback_;
};

class DmAniDeviceNameChangeCallback : public OHOS::DistributedHardware::DeviceStatusCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniDeviceNameChangeCallback> {
public:
    explicit DmAniDeviceNameChangeCallback(std::string &bundleName,
        taihe::callback<void(ohos::distributedDeviceManager::DeviceNameChangeResult const&)> deviceNameChangeCallback);
    ~DmAniDeviceNameChangeCallback() override {}
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceChangedInMainThread(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo);

private:
    std::string bundleName_;
    std::optional<taihe::callback<void(
        ohos::distributedDeviceManager::DeviceNameChangeResult const&)>> deviceNameChangeCallback_;
};

class DmAniDeviceStateChangeResultCallback : public OHOS::DistributedHardware::DeviceStatusCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniDeviceStateChangeResultCallback> {
public:
    explicit DmAniDeviceStateChangeResultCallback(std::string &bundleName,
        taihe::callback<void(ohos::distributedDeviceManager::DeviceStateChangeResult const&)>
        deviceStateChangeDataCallback);
    ~DmAniDeviceStateChangeResultCallback() override {}
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceStateChangeInMainthread(
        ::ohos::distributedDeviceManager::DeviceStateChange state,
        const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo);
private:
    std::string bundleName_;
    std::optional<taihe::callback<void(ohos::distributedDeviceManager::DeviceStateChangeResult const&)>>
        deviceStateChangeDataCallback_;
};

class DmAniBindTargetCallback : public OHOS::DistributedHardware::BindTargetCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniBindTargetCallback> {
public:
    explicit DmAniBindTargetCallback(ani_env* env, std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    void Release();
    void SetTaiheCallback(
        ::taihe::callback<void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const&)> callback);
    void OnBindResult(const OHOS::DistributedHardware::PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) override;
    void OnBindResultInMainThread(const OHOS::DistributedHardware::PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content);

private:
    ani_env* env_;
    std::string bundleName_;
    std::optional<::taihe::callback<
        void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const& data)>> jsCallback_;
};

class DmAniAuthenticateCallback : public OHOS::DistributedHardware::AuthenticateCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniAuthenticateCallback> {
public:
    explicit DmAniAuthenticateCallback(ani_env* env, std::string &bundleName) : env_(env), bundleName_(bundleName)
    {
    }
    void Release();
    void SetTaiheCallback(::taihe::callback<
        void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const& data)> callback);
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status, int32_t reason) override;
    void OnAuthResultInMainThread(const std::string &deviceId, const std::string &token,
        int32_t status, int32_t reason);
private:
    ani_env* env_;
    std::string bundleName_;
    std::optional<::taihe::callback<
        void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const& data)>> jsCallback_;
};

class DmAniDeviceManagerUiCallback : public OHOS::DistributedHardware::DeviceManagerUiCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniDeviceManagerUiCallback> {
public:
    explicit DmAniDeviceManagerUiCallback(taihe::callback<void(
        ohos::distributedDeviceManager::ReplyResult const&)> replyResultCallback,
        std::string &bundleName);
    ~DmAniDeviceManagerUiCallback() override {}
    void OnCall(const std::string &paramJson) override;
    void OnCallInMainthread(const std::string &paramJson);
private:
    std::string bundleName_;
    std::optional<taihe::callback<void(ohos::distributedDeviceManager::ReplyResult const&)>> replyResultCallback_;
};

class DmAniGetDeviceProfileInfoListCallback : public OHOS::DistributedHardware::GetDeviceProfileInfoListCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniGetDeviceProfileInfoListCallback> {
public:
    explicit DmAniGetDeviceProfileInfoListCallback(ani_vm* vm, const ani_resolver &deferred)
        :vm_(vm), deferred_(deferred)
    {}
    ~DmAniGetDeviceProfileInfoListCallback() override {};

    void OnResult(const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfos,
        int32_t code) override;
    void PromiseResult(ani_env* currentEnv,
        const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfos, int32_t code);
private:
    ani_vm* vm_ = nullptr;
    ani_resolver deferred_ = nullptr;
};

class DmAniGetDeviceIconInfoCallback : public OHOS::DistributedHardware::GetDeviceIconInfoCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniGetDeviceIconInfoCallback> {
public:
    explicit DmAniGetDeviceIconInfoCallback(ani_vm* vm, const ani_resolver &deferred)
        :vm_(vm), deferred_(deferred)
    {
    }
    ~DmAniGetDeviceIconInfoCallback() override {};

    void OnResult(const OHOS::DistributedHardware::DmDeviceIconInfo &deviceIconInfo, int32_t code) override;
    void PromiseResult(ani_env* currentEnv,
        const OHOS::DistributedHardware::DmDeviceIconInfo &deviceIconInfo, int32_t code);
private:
    ani_vm* vm_ = nullptr;
    ani_resolver deferred_ = nullptr;
};

class DmAniSetLocalDeviceNameCallback : public OHOS::DistributedHardware::SetLocalDeviceNameCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniSetLocalDeviceNameCallback> {
public:
    explicit DmAniSetLocalDeviceNameCallback(ani_vm* vm, const ani_resolver &deferred)
        :vm_(vm), deferred_(deferred)
    {
    }
    ~DmAniSetLocalDeviceNameCallback() override {};

    void OnResult(int32_t code) override;
    void PromiseResult(ani_env* currentEnv, int32_t code);
private:
    ani_vm* vm_ = nullptr;
    ani_resolver deferred_ = nullptr;
};

class DmAniSetRemoteDeviceNameCallback : public OHOS::DistributedHardware::SetRemoteDeviceNameCallback,
    public AsyncUtilBase,
    public std::enable_shared_from_this<DmAniSetRemoteDeviceNameCallback> {
public:
    explicit DmAniSetRemoteDeviceNameCallback(ani_vm* vm, const ani_resolver &deferred)
        :vm_(vm), deferred_(deferred)
    {
    }
    ~DmAniSetRemoteDeviceNameCallback() override {};

    void OnResult(int32_t code) override;
    void PromiseResult(ani_env* currentEnv, int32_t code);

private:
    ani_vm* vm_ = nullptr;
    ani_resolver deferred_ = nullptr;
};

#endif //OHOS_DM_ANI_CALLBACK_H