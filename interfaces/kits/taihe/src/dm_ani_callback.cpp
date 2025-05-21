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
#include "device_manager.h"
#include "dm_log.h"
#include "event_handler.h"

#define DH_LOG_TAG "DeviceManager"

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

DmAniDiscoveryFailedCallback::DmAniDiscoveryFailedCallback(std::string &bundleName,
    taihe::callback<void(int)> discoverFailedCallback)
    : refCount_(0), bundleName_(bundleName),
    discoverFailedCallback_(std::make_shared<taihe::callback<void(int)>>(discoverFailedCallback))
{
}

void DmAniDiscoveryFailedCallback::IncreaseRefCount()
{
    std::lock_guard<std::mutex> autoLock(g_dmDiscoveryMutex);
    refCount_++;
}

void DmAniDiscoveryFailedCallback::DecreaseRefCount()
{
    std::lock_guard<std::mutex> autoLock(g_dmDiscoveryMutex);
    refCount_--;
}

int32_t DmAniDiscoveryFailedCallback::GetRefCount()
{
    return refCount_;
}

DmAniDiscoverySuccessCallback::DmAniDiscoverySuccessCallback(std::string &bundleName,
    taihe::callback<void(ohos::distributedDeviceManager::DeviceBasicInfo const &)> discoverSuccessCallback)
    : refCount_(0), bundleName_(bundleName),
    discoverSuccessCallback_(std::make_shared<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceBasicInfo const &)>>(discoverSuccessCallback))
{
}

void DmAniDiscoverySuccessCallback::IncreaseRefCount()
{
    std::lock_guard<std::mutex> autoLock(g_dmDiscoveryMutex);
    refCount_++;
}

void DmAniDiscoverySuccessCallback::DecreaseRefCount()
{
    std::lock_guard<std::mutex> autoLock(g_dmDiscoveryMutex);
    refCount_--;
}

int32_t DmAniDiscoverySuccessCallback::GetRefCount()
{
    return refCount_;
}

DmAniDeviceNameChangeCallback::DmAniDeviceNameChangeCallback(std::string &bundleName,
    taihe::callback<void(taihe::string_view)> deviceNameChangeCallback)
    : bundleName_(bundleName), deviceNameChangeCallback_(
    std::make_shared<taihe::callback<void(taihe::string_view)>>(deviceNameChangeCallback))
{
}

void DmAniDeviceNameChangeCallback::OnDeviceChanged(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("DmAniDeviceNameChangeCallback::OnDeviceChanged called.");
}

DmAniDeviceStateChangeDataCallback::DmAniDeviceStateChangeDataCallback(std::string &bundleName,
    taihe::callback<void(ohos::distributedDeviceManager::DeviceStateChangeData const &)>
    deviceStateChangeDataCallback)
    : bundleName_(bundleName),
    deviceStateChangeDataCallback_(std::make_shared<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceStateChangeData const &)>>(deviceStateChangeDataCallback))
{
}

void DmAniDeviceStateChangeDataCallback::OnDeviceChanged(
    const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("DmAniDeviceStateChangeDataCallback::OnDeviceChanged called.");
}

DmAniDeviceManagerUiCallback::DmAniDeviceManagerUiCallback(
    taihe::callback<void(taihe::string_view)> replyResultCallback,
    std::string &bundleName)
    : bundleName_(bundleName),
    replyResultCallback_(std::make_shared<taihe::callback<void(taihe::string_view)>>(replyResultCallback))
{
}

void DmAniDeviceManagerUiCallback::OnCall(const std::string &paramJson)
{
    LOGI("DmAniDeviceManagerUiCallback::OnCall called.");
}