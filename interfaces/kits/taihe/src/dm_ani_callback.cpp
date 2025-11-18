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

#include "dm_ani_callback.h"
#include "ani_utils.h"
#include "ani_dm_utils.h"
#include "device_manager.h"
#include "dm_log.h"
#include "event_handler.h"
#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"
#include <thread>

std::shared_ptr<OHOS::AppExecFwk::EventHandler> AsyncUtilBase::mainHandler_;

bool AsyncUtilBase::SendEventToMainThread(const std::function<void()> func)
{
    if (func == nullptr) {
        return false;
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            LOGE("GetMainEventRunner failed");
            return false;
        }
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    mainHandler_->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
    return true;
}

bool AsyncUtilBase::AsyncExecute(const std::function<void()> func)
{
    std::thread t(func);
    t.detach();
    return true;
}

DmAniInitCallback::DmAniInitCallback(ani_env* env, taihe::string_view bundleName)
    : env_(env), bundleName_(std::string(bundleName))
{
}

void DmAniInitCallback::OnRemoteDied()
{
    LOGI("OnRemoteDied called.");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([sharedThis] {
        sharedThis->OnRemoteDiedInMainthread();
    });
}

void DmAniInitCallback::OnRemoteDiedInMainthread()
{
    LOGI("OnRemoteDiedInMainthread called.");
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (serviceDieCallback_.has_value()) {
        ani_ref *nullResult = nullptr;
        env_->GetNull(nullResult);
        serviceDieCallback_.value()(reinterpret_cast<uintptr_t>(nullResult));
    }
}

void DmAniInitCallback::SetServiceDieCallback(taihe::callback<void(uintptr_t data)> callback)
{
    LOGI("SetServiceDieCallback");
    std::lock_guard<std::mutex> autoLock(jsCallbackMutex_);
    serviceDieCallback_ = callback;
}

void DmAniInitCallback::ReleaseServiceDieCallback()
{
    LOGI("ReleaseServiceDieCallback");
    std::lock_guard<std::mutex> autoLock(jsCallbackMutex_);
    serviceDieCallback_.reset();
}

DmAniDeviceNameChangeCallback::DmAniDeviceNameChangeCallback(const std::string &bundleName,
    taihe::callback<void(ohos::distributedDeviceManager::DeviceNameChangeResult const&)> deviceNameChangeCallback)
    : bundleName_(bundleName), deviceNameChangeCallback_(deviceNameChangeCallback)
{
}

void DmAniDeviceNameChangeCallback::OnDeviceChanged(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceChanged called.");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([deviceBasicInfo, sharedThis] {
        sharedThis->OnDeviceChangedInMainThread(deviceBasicInfo);
    });
}

void DmAniDeviceNameChangeCallback::OnDeviceChangedInMainThread(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceChangedInMainThread called.");
    ohos::distributedDeviceManager::DeviceNameChangeResult taiheInfo = {
        ::taihe::string(deviceBasicInfo.deviceName)
    };
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (deviceNameChangeCallback_.has_value()) {
        deviceNameChangeCallback_.value()(taiheInfo);
    }
}

DmAniDeviceStateChangeResultCallback::DmAniDeviceStateChangeResultCallback(std::string &bundleName,
    taihe::callback<void(ohos::distributedDeviceManager::DeviceStateChangeResult const&)>
    deviceStateChangeDataCallback)
    : bundleName_(bundleName),
    deviceStateChangeDataCallback_(deviceStateChangeDataCallback)
{
}

void DmAniDeviceStateChangeResultCallback::OnDeviceOnline(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceOnline called.");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([deviceBasicInfo, sharedThis] {
        sharedThis->OnDeviceStateChangeInMainthread(
            ohos::distributedDeviceManager::DeviceStateChange::key_t::UNKNOWN, deviceBasicInfo);
    });
}

void DmAniDeviceStateChangeResultCallback::OnDeviceReady(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceReady called.");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([deviceBasicInfo, sharedThis] {
        sharedThis->OnDeviceStateChangeInMainthread(
            ohos::distributedDeviceManager::DeviceStateChange::key_t::AVAILABLE, deviceBasicInfo);
    });
}

void DmAniDeviceStateChangeResultCallback::OnDeviceOffline(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceOffline called.");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([deviceBasicInfo, sharedThis] {
        sharedThis->OnDeviceStateChangeInMainthread(
            ohos::distributedDeviceManager::DeviceStateChange::key_t::UNAVAILABLE, deviceBasicInfo);
    });
}

void DmAniDeviceStateChangeResultCallback::OnDeviceStateChangeInMainthread(
    ::ohos::distributedDeviceManager::DeviceStateChange state,
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceStateChangeInMainthread called.");
    OHOS::DistributedHardware::DmDeviceType dmType =
        static_cast<OHOS::DistributedHardware::DmDeviceType>(deviceBasicInfo.deviceTypeId);
    ::ohos::distributedDeviceManager::DeviceBasicInfo taiheInfo = {
        ::taihe::string(deviceBasicInfo.deviceId),
        ::taihe::string(deviceBasicInfo.deviceName),
        ::taihe::string(ani_dmutils::GetDeviceTypeById(dmType)),
        ::taihe::optional<::taihe::string>::make(::taihe::string(deviceBasicInfo.networkId))
    };
    ohos::distributedDeviceManager::DeviceStateChangeResult taiheResult = { state, taiheInfo };
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (deviceStateChangeDataCallback_.has_value()) {
        deviceStateChangeDataCallback_.value()(taiheResult);
    }
}

DmAniDeviceManagerUiCallback::DmAniDeviceManagerUiCallback(
    taihe::callback<void(ohos::distributedDeviceManager::ReplyResult const&)> replyResultCallback,
    std::string &bundleName)
    : bundleName_(bundleName), replyResultCallback_(replyResultCallback)
{
}

void DmAniDeviceManagerUiCallback::OnCall(const std::string &paramJson)
{
    LOGI("OnCall.");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([paramJson, sharedThis] {
        sharedThis->OnCallInMainthread(paramJson);
    });
}

void DmAniDeviceManagerUiCallback::OnCallInMainthread(const std::string &paramJson)
{
    LOGI("OnCallInMainthread.");
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (replyResultCallback_.has_value()) {
        ohos::distributedDeviceManager::ReplyResult result = { ::taihe::string(paramJson) };
        replyResultCallback_.value()(result);
    }
}

void DmAniDiscoveryCallback::SetSuccessCallback(JsDiscoverSuccessCallback discoverSuccessCallback)
{
    LOGI("SetSuccessCallback.");
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverSuccessCallback_ = discoverSuccessCallback;
}

void DmAniDiscoveryCallback::ResetSuccessCallback()
{
    LOGI("ResetSuccessCallback.");
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverSuccessCallback_.reset();
}

void DmAniDiscoveryCallback::SetFailedCallback(JsDiscoverFailedCallback discoverFailedCallback)
{
    LOGI("SetFailedCallback.");
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverFailedCallback_ = discoverFailedCallback;
}

void DmAniDiscoveryCallback::ResetFailedCallback()
{
    LOGI("ResetFailedCallback.");
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverFailedCallback_.reset();
}

void DmAniDiscoveryCallback::OnDeviceFound(uint16_t subscribeId,
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFound.");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([subscribeId, deviceBasicInfo, sharedThis] {
        sharedThis->OnDeviceFoundInMainThread(subscribeId, deviceBasicInfo);
    });
}

void DmAniDiscoveryCallback::OnDeviceFoundInMainThread(uint16_t subscribeId,
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFoundInMainThread DmDeviceBasicInfo for subscribeId %{public}d", (int32_t)subscribeId);
    std::string deviceType = ani_dmutils::GetDeviceTypeById(
        static_cast<OHOS::DistributedHardware::DmDeviceType>(deviceBasicInfo.deviceTypeId));
    ::ohos::distributedDeviceManager::DeviceBasicInfo basicInfo = {
        ::taihe::string(deviceBasicInfo.deviceId),
        ::taihe::string(deviceBasicInfo.deviceName),
        ::taihe::string(deviceType),
        ::taihe::optional<::taihe::string>::make(deviceBasicInfo.networkId)
    };
    ohos::distributedDeviceManager::DiscoverySuccessResult taiheResult = {basicInfo};
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (discoverSuccessCallback_.has_value()) {
        discoverSuccessCallback_.value()(taiheResult);
    }
}

void DmAniDiscoveryCallback::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailed");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([subscribeId, failedReason, sharedThis] {
        sharedThis->OnDiscoveryFailedInMainThread(subscribeId, failedReason);
    });
}

void DmAniDiscoveryCallback::OnDiscoveryFailedInMainThread(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailedInMainThread for subscribeId %{public}d", (int32_t)subscribeId);
    std::string errCodeInfo = OHOS::DistributedHardware::GetErrorString((int)failedReason);
    ohos::distributedDeviceManager::DiscoveryFailureResult taiheResult = { failedReason };
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (discoverFailedCallback_.has_value()) {
        discoverFailedCallback_.value()(taiheResult);
    }
}

void DmAniDiscoveryCallback::OnDiscoverySuccess(uint16_t subscribeId)
{
}

int32_t DmAniDiscoveryCallback::GetRefCount()
{
    int count = 0;
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (discoverSuccessCallback_.has_value()) {
        count++;
    }
    if (discoverFailedCallback_.has_value()) {
        count++;
    }
    return count;
}

void DmAniBindTargetCallback::Release()
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    jsCallback_.reset();
}

void DmAniBindTargetCallback::SetTaiheCallback(
    ::taihe::callback<void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const&)> callback)
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    jsCallback_ = callback;
}

void DmAniBindTargetCallback::OnBindResult(const OHOS::DistributedHardware::PeerTargetId &targetId, int32_t result,
    int32_t status, std::string content)
{
    LOGI("OnBindResult");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([targetId, result, status, content, sharedThis] {
        sharedThis->OnBindResultInMainThread(targetId, result, status, content);
    });
}

void DmAniBindTargetCallback::OnBindResultInMainThread(const OHOS::DistributedHardware::PeerTargetId &targetId,
    int32_t result, int32_t status, std::string content)
{
    using namespace OHOS::DistributedHardware;
    using namespace ANI::distributedDeviceManager;

    LOGI("OnBindResultInMainThread");
    std::string deviceId = content;
    ani_ref err = nullptr;
    ::ohos::distributedDeviceManager::BindTargetResult data = {};
    if (status == DM_AUTH_REQUEST_SUCCESS_STATUS && result == 0) {
        LOGI("OnBindResult success");
        env_->GetNull(&err);
        data.deviceId = ::taihe::string(deviceId);
    } else {
        LOGI("OnBindResult failed");
        std::string errCodeInfo = OHOS::DistributedHardware::GetErrorString((int)result);
        err = ani_errorutils::ToBusinessError(env_, status, errCodeInfo);
    }
    if (result == DM_OK && (status <= STATUS_DM_CLOSE_PIN_INPUT_UI && status >= STATUS_DM_SHOW_AUTHORIZE_UI)) {
        LOGI("update ui change, status: %{public}d, reason: %{public}d", status, result);
    } else {
        LOGI("OnBindResult call js");
        std::lock_guard<std::mutex> lock(jsCallbackMutex_);
        if (jsCallback_.has_value()) {
            jsCallback_.value()(reinterpret_cast<uintptr_t>(err), data);
        }
    }
}

void DmAniAuthenticateCallback::Release()
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    jsCallback_.reset();
}

void DmAniAuthenticateCallback::SetTaiheCallback(
    ::taihe::callback<void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const& data)> callback)
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    jsCallback_ = callback;
}

void DmAniAuthenticateCallback::OnAuthResult(const std::string &deviceId, const std::string &token,
    int32_t status, int32_t reason)
{
    LOGI("OnAuthResult");
    auto sharedThis = shared_from_this();
    SendEventToMainThread([deviceId, token, status, reason, sharedThis] {
        sharedThis->OnAuthResultInMainThread(deviceId, token, status, reason);
    });
}

void DmAniAuthenticateCallback::OnAuthResultInMainThread(const std::string &deviceId, const std::string &token,
    int32_t status, int32_t reason)
{
    using namespace OHOS::DistributedHardware;
    using namespace ANI::distributedDeviceManager;

    LOGI("OnAuthResultInMainThread");
    ani_ref err = nullptr;
    ::ohos::distributedDeviceManager::BindTargetResult data = {};
    if (status == DM_AUTH_REQUEST_SUCCESS_STATUS && reason == 0) {
        LOGI("OnAuthResult success");
        env_->GetNull(&err);
        data.deviceId = ::taihe::string(deviceId);
    } else {
        LOGI("OnAuthResult failed");
        std::string errCodeInfo = OHOS::DistributedHardware::GetErrorString((int)reason);
        err = ani_errorutils::ToBusinessError(env_, status, errCodeInfo);
    }
    if (reason == DM_OK && (status <= STATUS_DM_CLOSE_PIN_INPUT_UI && status >= STATUS_DM_SHOW_AUTHORIZE_UI)) {
        LOGI("update ui change, status: %{public}d, reason: %{public}d", status, reason);
    } else {
        LOGI("OnAuthResult call js");
        {
            std::lock_guard<std::mutex> lock(jsCallbackMutex_);
            if (jsCallback_.has_value()) {
                jsCallback_.value()(reinterpret_cast<uintptr_t>(err), data);
            }
        }
        std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
        g_authCallbackMap.erase(bundleName_);
    }
}

void DmAniGetDeviceProfileInfoListCallback::OnResult(
    const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfos,
    int32_t code)
{
    LOGI("OnResult, %{public}d", code);
    ani_utils::AniExecuteFunc(vm_, [this, &deviceProfileInfos, code] (ani_env* currentEnv) {
        this->PromiseResult(currentEnv, deviceProfileInfos, code);
    });
}

void DmAniGetDeviceProfileInfoListCallback::PromiseResult(ani_env* currentEnv,
    const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfos,
    int32_t code)
{
    LOGI("PromiseResult");
    if (currentEnv == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (deferred_) {
        ani_object resolveResult = nullptr;
        if (code == OHOS::DistributedHardware::DM_OK) {
            resolveResult = ani_dmutils::DeviceProfileInfoArrayToAni(currentEnv, deviceProfileInfos);
        }
        LOGI("PromiseResult, AniPromiseCallback");
        ani_errorutils::AniPromiseCallback(currentEnv, deferred_, code, resolveResult);
        deferred_ = nullptr;
    }
}

void DmAniGetDeviceIconInfoCallback::OnResult(
    const OHOS::DistributedHardware::DmDeviceIconInfo &deviceIconInfo, int32_t code)
{
    LOGI("OnResult, %{public}d", code);
    ani_utils::AniExecuteFunc(vm_, [this, &deviceIconInfo, code] (ani_env* currentEnv) {
        this->PromiseResult(currentEnv, deviceIconInfo, code);
    });
}

void DmAniGetDeviceIconInfoCallback::PromiseResult(ani_env* currentEnv,
    const OHOS::DistributedHardware::DmDeviceIconInfo &deviceIconInfo, int32_t code)
{
    LOGI("PromiseResult");
    if (currentEnv == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (deferred_) {
        ani_object resolveResult = nullptr;
        if (code == OHOS::DistributedHardware::DM_OK) {
            resolveResult = ani_dmutils::DeviceIconInfoToAni(currentEnv, deviceIconInfo);
        }
        LOGI("DmAniGetDeviceIconInfoCallback AniPromiseCallback");
        ani_errorutils::AniPromiseCallback(currentEnv, deferred_, code, resolveResult);
        deferred_ = nullptr;
    }
}

void DmAniSetLocalDeviceNameCallback::OnResult(int32_t code)
{
    LOGI("OnResult, %{public}d", code);
    ani_utils::AniExecuteFunc(vm_, [this, code] (ani_env* currentEnv) {
        this->PromiseResult(currentEnv, code);
    });
}

void DmAniSetLocalDeviceNameCallback::PromiseResult(ani_env* currentEnv, int32_t code)
{
    LOGI("PromiseResult");
    if (currentEnv == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (deferred_) {
        ani_object resolveResult = nullptr;
        if (code == OHOS::DistributedHardware::DM_OK) {
            ani_utils::AniCreateInt(currentEnv, code, resolveResult);
        }
        ani_errorutils::AniPromiseCallback(currentEnv, deferred_, code, resolveResult);
        deferred_ = nullptr;
    }
}

void DmAniSetRemoteDeviceNameCallback::OnResult(int32_t code)
{
    LOGI("OnResult, %{public}d", code);
    ani_utils::AniExecuteFunc(vm_, [this, code] (ani_env* currentEnv) {
        this->PromiseResult(currentEnv, code);
    });
}

void DmAniSetRemoteDeviceNameCallback::PromiseResult(ani_env* currentEnv, int32_t code)
{
    LOGI("PromiseResult");
    if (currentEnv == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    if (deferred_) {
        ani_object resolveResult = nullptr;
        if (code == OHOS::DistributedHardware::DM_OK) {
            ani_utils::AniCreateInt(currentEnv, code, resolveResult);
        }
        ani_errorutils::AniPromiseCallback(currentEnv, deferred_, code, resolveResult);
        deferred_ = nullptr;
    }
}
