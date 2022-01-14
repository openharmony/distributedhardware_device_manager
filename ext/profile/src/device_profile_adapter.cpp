/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "device_profile_adapter.h"

#include "dm_constants.h"
#include "dm_device_state_manager.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<ProfileConnector> DeviceProfileAdapter::profileConnector_ = std::make_shared<ProfileConnector>();

DeviceProfileAdapter::DeviceProfileAdapter()
{
}

DeviceProfileAdapter::~DeviceProfileAdapter()
{
}

int32_t DeviceProfileAdapter::RegisterProfileListener(const std::string &pkgName, const std::string &deviceId,
                                                      std::shared_ptr<DmDeviceStateManager> callback)
{
    LOGI("DeviceProfileAdapter::RegisterProfileListener");
    deviceProfileAdapterCallback_ = callback;
    profileConnector_->RegisterProfileCallback(pkgName, deviceId, std::shared_ptr<DeviceProfileAdapter>(this));
    return DM_OK;
}

int32_t DeviceProfileAdapter::UnRegisterProfileListener(const std::string &pkgName)
{
    LOGI("DeviceProfileAdapter::RegisterProfileListener");
    deviceProfileAdapterCallback_ = nullptr;
    profileConnector_->UnRegisterProfileCallback(pkgName);
    return DM_OK;
}

int32_t DeviceProfileAdapter::OnProfileClientDeviceReady(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DeviceProfileAdapter::OnProfileClientDeviceReady");
    if (deviceProfileAdapterCallback_ != nullptr) {
        deviceProfileAdapterCallback_->OnProfileReady(pkgName, deviceId);
    } else {
        LOGI("deviceProfileAdapterCallback_ is nullptr");
    }
    return DM_OK;
}

void DeviceProfileAdapter::OnProfileChanged(const std::string &pkgName, const std::string &deviceId)
{
    OnProfileClientDeviceReady(pkgName, deviceId);
}

void DeviceProfileAdapter::OnProfileComplete(const std::string &pkgName, const std::string &deviceId)
{
    OnProfileClientDeviceReady(pkgName, deviceId);
}

extern "C" IProfileAdapter *CreateDeviceProfileObject(void)
{
    return new DeviceProfileAdapter;
}
} // namespace DistributedHardware
} // namespace OHOS
