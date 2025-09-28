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

DmAniInitCallback::DmAniInitCallback(taihe::string_view bundleName)
    : bundleName_(std::string(bundleName))
{
    std::lock_guard<std::mutex> autoLock(g_dmInitMutex);
    serviceDieCallback_ = nullptr;
}

void DmAniInitCallback::OnRemoteDied()
{
    LOGI("DmAniInitCallback::OnRemoteDied called.");
    auto &deviceManager = static_cast<OHOS::DistributedHardware::DeviceManager &>(
        OHOS::DistributedHardware::DeviceManager::GetInstance());
    deviceManager.UnInitDeviceManager(bundleName_);
}

void DmAniInitCallback::SetServiceDieCallback(std::shared_ptr<taihe::callback<void()>> callback)
{
    std::lock_guard<std::mutex> autoLock(g_dmInitMutex);
    serviceDieCallback_ = callback;
}

void DmAniInitCallback::ReleaseServiceDieCallback()
{
    std::lock_guard<std::mutex> autoLock(g_dmInitMutex);
    serviceDieCallback_ = nullptr;
}

DmAniDeviceNameChangeCallback::DmAniDeviceNameChangeCallback(std::string &bundleName,
    taihe::callback<void(ohos::distributedDeviceManager::DeviceNameChangeResult const&)> deviceNameChangeCallback)
    : bundleName_(bundleName), deviceNameChangeCallback_(
    std::make_shared<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceNameChangeResult const&)>>(deviceNameChangeCallback))
{
}

void DmAniDeviceNameChangeCallback::OnDeviceChanged(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("DmAniDeviceNameChangeCallback::OnDeviceChanged called.");
}

DmAniDeviceStateChangeResultCallback::DmAniDeviceStateChangeResultCallback(std::string &bundleName,
    taihe::callback<void(ohos::distributedDeviceManager::DeviceStateChangeResult const&)>
    deviceStateChangeDataCallback)
    : bundleName_(bundleName),
    deviceStateChangeDataCallback_(std::make_shared<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceStateChangeResult const&)>>(deviceStateChangeDataCallback))
{
}

void DmAniDeviceStateChangeResultCallback::OnDeviceChanged(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("DmAniDeviceStateChangeResultCallback::OnDeviceChanged called.");
}

DmAniDeviceManagerUiCallback::DmAniDeviceManagerUiCallback(
    taihe::callback<void(ohos::distributedDeviceManager::ReplyResult const&)> replyResultCallback,
    std::string &bundleName)
    : bundleName_(bundleName),
    replyResultCallback_(std::make_shared<taihe::callback<void(
    ohos::distributedDeviceManager::ReplyResult const&)>>(replyResultCallback))
{
}

void DmAniDeviceManagerUiCallback::OnCall(const std::string &paramJson)
{
    LOGI("DmAniDeviceManagerUiCallback::OnCall called.");
}

void DmAniDiscoveryCallback::SetSuccessCallback(JsDiscoverSuccessCallback discoverSuccessCallback)
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverSuccessCallback_ = discoverSuccessCallback;
}

void DmAniDiscoveryCallback::ResetSuccessCallback()
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverSuccessCallback_.reset();
}

void DmAniDiscoveryCallback::SetFailedCallback(JsDiscoverFailedCallback discoverFailedCallback)
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverFailedCallback_ = discoverFailedCallback;
}

void DmAniDiscoveryCallback::ResetFailedCallback()
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    discoverFailedCallback_.reset();
}

void DmAniDiscoveryCallback::OnDeviceFound(uint16_t subscribeId,
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    auto sharedThis = shared_from_this();
    SendEventToMainThread([subscribeId, deviceBasicInfo, sharedThis] {
        sharedThis->OnDeviceFoundInMainThread(subscribeId, deviceBasicInfo);
    });
}

void DmAniDiscoveryCallback::OnDeviceFoundInMainThread(uint16_t subscribeId,
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFound DmDeviceBasicInfo for subscribeId %{public}d", (int32_t)subscribeId);
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
    LOGI("OnDiscoveryFailed for subscribeId %{public}d", (int32_t)subscribeId);
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
    jsCallback_ = callback;
}

void DmAniBindTargetCallback::OnBindResult(const OHOS::DistributedHardware::PeerTargetId &targetId, int32_t result,
    int32_t status, std::string content)
{
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