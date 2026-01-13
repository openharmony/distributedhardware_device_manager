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

#include "device_name_manager_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceNameManager);
std::string DeviceNameManager::GetLocalMarketName()
{
    return DmDeviceNameManager::dmDeviceNameManager_->GetLocalMarketName();
}

int32_t DeviceNameManager::GetLocalDisplayDeviceName(int32_t maxNamelength, std::string &displayName)
{
    return DmDeviceNameManager::dmDeviceNameManager_->GetLocalDisplayDeviceName(maxNamelength, displayName);
}

int32_t DeviceNameManager::InitDeviceNameWhenUserSwitch(int32_t curUserId, int32_t preUserId)
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenUserSwitch(curUserId, preUserId);
}
} // namespace DistributedHardware
} // namespace OHOS
