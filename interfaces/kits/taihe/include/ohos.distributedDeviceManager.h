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
    explicit DeviceManagerImpl(const std::string& bundleName) ;
    ~DeviceManagerImpl() = default;
    std::string GetLocalDeviceId();
    void UnbindTarget(taihe::string_view deviceId);
    double GetDeviceType(taihe::string_view networkId);
    std::string GetDeviceName(taihe::string_view networkId);
    std::string GetLocalDeviceNetworkId();

    void OnDeviceNameChange(taihe::callback_view<void(taihe::string_view)> onDeviceNameChangecb);
    void OnDiscoverFailure(taihe::callback_view<void(int32_t)> onDiscoverFailurecb);
    void OnreplyResult(taihe::callback_view<void(taihe::string_view)> onreplyResultcb);
    void OnDiscoverSuccess(taihe::callback_view<void(
        ohos::distributedDeviceManager::DeviceBasicInfo const &)> onDiscoverSuccesscb);
    void OnDeviceStateChange(taihe::callback_view<void(
        ohos::distributedDeviceManager::DeviceStateChangeData const&)> onDeviceStateChangecb);

    void OffDeviceNameChange(taihe::optional_view<taihe::callback<void(taihe::string_view)>> offDeviceNameChangecb);
    void OffDiscoverFailure(taihe::optional_view<taihe::callback<void(int32_t)>> offDiscoverFailurecb);
    void OffreplyResult(taihe::optional_view<taihe::callback<void(taihe::string_view)>> offreplyResultcb);
    void OffDiscoverSuccess(taihe::optional_view<taihe::callback<void(
        ohos::distributedDeviceManager::DeviceBasicInfo const&)>> offDiscoverSuccesscb);
    void OffDeviceStateChange(taihe::optional_view<taihe::callback<void(
        ohos::distributedDeviceManager::DeviceStateChangeData const&)>> offDeviceStateChangecb);

    friend ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName);

private:
    std::string bundleName_;
};

ohos::distributedDeviceManager::DeviceBasicInfo MakeDeviceBasicInfo(taihe::string_view deviceId,
    taihe::string_view deviceName, taihe::string_view deviceType,
    taihe::string_view networkId, taihe::string_view extraData);
ohos::distributedDeviceManager::DeviceStateChangeData MakeDeviceStateChangeData(
    ohos::distributedDeviceManager::DeviceStateChange deviceStateChange,
    ohos::distributedDeviceManager::DeviceBasicInfo const &deviceBasicInfo);
ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName);
}// namespace ANI::distributedDeviceManager

#endif
