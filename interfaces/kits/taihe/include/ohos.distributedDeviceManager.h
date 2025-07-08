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
#ifndef OHOS_DISTRIBUTEDDEVICEMANAGER_H
#define OHOS_DISTRIBUTEDDEVICEMANAGER_H

#include <functional>
#include <string>
#include "device_manager_callback.h"
#include "dm_device_info.h"
#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"
#include "taihe/callback.hpp"
#include "taihe/runtime.hpp"

namespace ANI::distributedDeviceManager {

enum DMBussinessErrorCode {
    ERR_NO_PERMISSION = 201,
    DM_ERR_FAILED = 11600101,
};

class DeviceManagerImpl {
public:
    DeviceManagerImpl() = default;
    explicit DeviceManagerImpl(std::shared_ptr<DeviceManagerImpl> impl);
    explicit DeviceManagerImpl(const std::string& bundleName);
    ~DeviceManagerImpl() = default;
    std::string GetLocalDeviceId();
    void UnbindTarget(taihe::string_view deviceId);
    double GetDeviceType(taihe::string_view networkId);
    std::string GetDeviceName(taihe::string_view networkId);
    std::string GetLocalDeviceNetworkId();

    void OnDeviceNameChange(taihe::callback_view<void(
        ohos::distributedDeviceManager::DeviceNameChangeResult const&)> onDeviceNameChangecb);
    void OnDiscoverFailure(taihe::callback_view<void(
        ohos::distributedDeviceManager::DiscoveryFailureResult const&)> onDiscoverFailurecb);
    void OnReplyResult(taihe::callback_view<void(
        ohos::distributedDeviceManager::ReplyResult const&)> onReplyResultcb);
    void OnDiscoverSuccess(taihe::callback_view<void(
        ohos::distributedDeviceManager::DiscoverySuccessResult const&)> onDiscoverSuccesscb);
    void OnDeviceStateChange(taihe::callback_view<void(
        ohos::distributedDeviceManager::DeviceStateChangeResult const&)> onDeviceStateChangecb);
    void OnServiceDie(taihe::callback_view<void()> onServiceDiecb);

    void OffDeviceNameChange(taihe::optional_view<taihe::callback<void(
        ohos::distributedDeviceManager::DeviceNameChangeResult const&)>> offDeviceNameChangecb);
    void OffDiscoverFailure(taihe::optional_view<taihe::callback<void(
        ohos::distributedDeviceManager::DiscoveryFailureResult const&)>> offDiscoverFailurecb);
    void OffReplyResult(taihe::optional_view<taihe::callback<void(
        ohos::distributedDeviceManager::ReplyResult const&)>> offReplyResultcb);
    void OffDiscoverSuccess(taihe::optional_view<taihe::callback<void(
        ohos::distributedDeviceManager::DiscoverySuccessResult const&)>> offDiscoverSuccesscb);
    void OffDeviceStateChange(taihe::optional_view<taihe::callback<void(
        ohos::distributedDeviceManager::DeviceStateChangeResult const&)>> offDeviceStateChangecb);
    void OffServiceDie(taihe::optional_view<taihe::callback<void()>> offServiceDiecb);

    friend ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName);

private:
    std::string bundleName_;
};

ohos::distributedDeviceManager::DeviceBasicInfo MakeDeviceBasicInfo(taihe::string_view deviceId,
    taihe::string_view deviceName, taihe::string_view deviceType,
    taihe::string_view networkId, taihe::string_view extraData);

ohos::distributedDeviceManager::DeviceNameChangeResult MakeDeviceNameChangeResult(taihe::string_view deviceName);
ohos::distributedDeviceManager::ReplyResult MakeReplyResult(taihe::string_view param);
ohos::distributedDeviceManager::DiscoveryFailureResult MakeDiscoveryFailureResult(int32_t reason);
ohos::distributedDeviceManager::DiscoverySuccessResult MakeDiscoverySuccessResult(
    ohos::distributedDeviceManager::DeviceBasicInfo const& device);
ohos::distributedDeviceManager::DeviceStateChangeResult MakeDeviceStateChangeResult(
    ohos::distributedDeviceManager::DeviceStateChange deviceStateChange,
    ohos::distributedDeviceManager::DeviceBasicInfo const& deviceBasicInfo);
ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName);
} // namespace ANI::distributedDeviceManager

#endif //OHOS_DISTRIBUTEDDEVICEMANAGER_H
