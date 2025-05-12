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

#ifndef OHOS_DISTRIBUTED_DEVICE_MANAGER_ANI_CALLBACK_H
#define OHOS_DISTRIBUTED_DEVICE_MANAGER_ANI_CALLBACK_H

#include <atomic>
#include <string>
#include "device_manager_callback.h"
#include "dm_device_info.h"
#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "taihe/callback.hpp"

class DmAniInitCallback : public OHOS::DistributedHardware::DmInitCallback
{
public:
    explicit DmAniInitCallback(taihe::string_view bundleName) : bundleName_(std::string(bundleName))
    {
    }
    ~DmAniInitCallback() override {}
    void OnRemoteDied() override;

private:
    std::string bundleName_;
};

class DmAniDiscoverySuccessCallback : public OHOS::DistributedHardware::DiscoveryCallback
{
public:
    explicit DmAniDiscoverySuccessCallback(std::string &bundleName,
        taihe::callback_view<void(ohos::distributedDeviceManager::DeviceBasicInfo const &)> discoverSuccessCallback)
    : refCount_(0), bundleName_(bundleName),
        discoverSuccessCallback_(std::make_shared<taihe::callback<void(
            ohos::distributedDeviceManager::DeviceBasicInfo const &)>>(discoverSuccessCallback))
    {
    }
    ~DmAniDiscoverySuccessCallback() override {};
    void OnDeviceFound(uint16_t subscribeId,
        const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override{}
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override{}
    void OnDiscoverySuccess(uint16_t subscribeId) override{}
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();

private:
    std::atomic<int32_t> refCount_;
    std::string bundleName_;
    std::shared_ptr<taihe::callback_view<void(
        ohos::distributedDeviceManager::DeviceBasicInfo const &)>> discoverSuccessCallback_;
};

class DmAniDiscoveryFailedCallback : public OHOS::DistributedHardware::DiscoveryCallback
{
public:
    explicit DmAniDiscoveryFailedCallback(std::string &bundleName,
        taihe::callback_view<void(int)> discoverFailedCallback)
    : refCount_(0), bundleName_(bundleName),
        discoverFailedCallback_(std::make_shared<taihe::callback<void(int)>>(discoverFailedCallback))
    {
    }
    ~DmAniDiscoveryFailedCallback() override {};
    void OnDeviceFound(uint16_t subscribeId,
        const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override{}
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override{}
    void OnDiscoverySuccess(uint16_t subscribeId) override{}
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();

private:
    std::atomic<int32_t> refCount_;
    std::string bundleName_;
    std::shared_ptr<taihe::callback<void(int)>> discoverFailedCallback_;
};

class DmAniDeviceNameChangeCallback : public OHOS::DistributedHardware::DeviceStatusCallback
{
public:
    explicit DmAniDeviceNameChangeCallback(std::string &bundleName,
        taihe::callback_view<void(taihe::string_view)> deviceNameChangeCallback)
    : bundleName_(bundleName),
        deviceNameChangeCallback_(
            std::make_shared<taihe::callback<void(taihe::string_view)>>(deviceNameChangeCallback))
    {
    }
    ~DmAniDeviceNameChangeCallback() override {};
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}

private:
    std::string bundleName_;
    std::shared_ptr<taihe::callback<void(taihe::string_view)>> deviceNameChangeCallback_;
};

class DmAniDeviceStateChangeDataCallback : public OHOS::DistributedHardware::DeviceStatusCallback
{
public:
    explicit DmAniDeviceStateChangeDataCallback(std::string &bundleName,
        taihe::callback_view<void(
            ohos::distributedDeviceManager::DeviceStateChangeData const &)> deviceStateChangeDataCallback)
    : bundleName_(bundleName),
        deviceStateChangeDataCallback_(
            std::make_shared<taihe::callback_view<void(
                ohos::distributedDeviceManager::DeviceStateChangeData const &)>>(
                    deviceStateChangeDataCallback))
    {
    }
    ~DmAniDeviceStateChangeDataCallback() override {};
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {}

private:
    std::string bundleName_;
    std::shared_ptr<taihe::callback_view<void(
        ohos::distributedDeviceManager::DeviceStateChangeData const &)>>
        deviceStateChangeDataCallback_;
};

class DmAniDeviceManagerUiCallback : public OHOS::DistributedHardware::DeviceManagerUiCallback
{
public:
    explicit DmAniDeviceManagerUiCallback(taihe::callback_view<void(taihe::string_view)> replyResultCallback,
    std::string &bundleName)
    : bundleName_(bundleName),
        replyResultCallback_(std::make_shared<taihe::callback<void(taihe::string_view)>>(replyResultCallback))
    {
    }
    ~DmAniDeviceManagerUiCallback() override {};
    void OnCall(const std::string &paramJson) override {}

private:
    std::string bundleName_;
    std::shared_ptr<taihe::callback<void(taihe::string_view)>> replyResultCallback_;
};

#endif
