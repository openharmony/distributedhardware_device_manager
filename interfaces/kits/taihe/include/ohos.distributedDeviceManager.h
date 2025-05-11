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
#ifndef OHOS_DISTRIBUTED_DEVICE_MANAGER_H
#define OHOS_DISTRIBUTED_DEVICE_MANAGER_H

#include <string>
#include <functional>
#include "taihe/callback.hpp"
#include "dm_device_info.h"
#include "device_manager_callback.h"

namespace ANI::distributedDeviceManager {

enum DMBussinessErrorCode {
    ERR_NO_PERMISSION = 201,
    DM_ERR_FAILED = 11600101,
};

class DeviceManagerImpl {
public:
    DeviceManagerImpl() = default;
    explicit DeviceManagerImpl(std::shared_ptr<DeviceManagerImpl> impl);
    explicit DeviceManagerImpl(const std::string& bundleName) ;
    ~DeviceManagerImpl() = default;
    std::string getLocalDeviceId();
    void unbindTarget(taihe::string_view deviceId);
    double getDeviceType(taihe::string_view networkId);
    std::string getDeviceName(taihe::string_view networkId);
    std::string getLocalDeviceNetworkId();

    void ondeviceNameChange(taihe::callback_view<taihe::string(taihe::string_view)> f);
    void ondiscoverFailure(taihe::callback_view<void(int32_t)> f);
    void onreplyResult(taihe::callback_view<taihe::string(taihe::string_view)> f);
    void onserviceDie(taihe::callback_view<taihe::string(taihe::string_view)> f);
    void ondiscoverSuccess(taihe::callback_view<ohos::distributedDeviceManager::DeviceBasicInfo(
        ohos::distributedDeviceManager::DeviceBasicInfo const&)> f);
    void ondeviceStateChange(taihe::callback_view<ohos::distributedDeviceManager::DeviceStatusStructer(
        ohos::distributedDeviceManager::DeviceStatusStructer const&)> f);

    void offdeviceNameChange(taihe::callback_view<taihe::string(taihe::string_view)> f);
    void offdiscoverFailure(taihe::callback_view<void(int32_t)> f);
    void offreplyResult(taihe::callback_view<taihe::string(taihe::string_view)> f);
    void offserviceDie(taihe::callback_view<taihe::string(taihe::string_view)> f);
    void offdiscoverSuccess(taihe::callback_view<ohos::distributedDeviceManager::DeviceBasicInfo(
        ohos::distributedDeviceManager::DeviceBasicInfo const&)> f);
    void offdeviceStateChange(taihe::callback_view<ohos::distributedDeviceManager::DeviceStatusStructer(
        ohos::distributedDeviceManager::DeviceStatusStructer const&)> f);

    friend ohos::distributedDeviceManager::DeviceManager createDeviceManager(taihe::string_view bundleName);

private:
    std::shared_ptr<ohos::distributedDeviceManager::DeviceManager> deviceManager_;
    std::string bundleName_;
};

ohos::distributedDeviceManager::DeviceManager createDeviceManager(taihe::string_view bundleName);

class DmAniInitCallback : public OHOS::DistributedHardware::DmInitCallback {
public:
    explicit DmAniInitCallback(taihe::string_view bundleName) : bundleName_(std::string(bundleName))
    {
    }
    ~DmAniInitCallback() override {}
    void OnRemoteDied() override;

private:
    std::string bundleName_;
};
}

#endif
