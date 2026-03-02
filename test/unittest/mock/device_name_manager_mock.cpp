/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceNameManager);

void DeviceNameManager::DataShareReady()
{
    DmDeviceNameManager::dmDeviceNameManager_->DataShareReady();
}

void DeviceNameManager::AccountSysReady(int32_t userId)
{
    DmDeviceNameManager::dmDeviceNameManager_->AccountSysReady(userId);
}

int32_t DeviceNameManager::InitDeviceNameWhenSoftBusReady()
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenSoftBusReady();
}

int32_t DeviceNameManager::GetLocalDisplayDeviceName(int32_t maxNamelength, std::string &displayName)
{
    return DmDeviceNameManager::dmDeviceNameManager_->GetLocalDisplayDeviceName(maxNamelength, displayName);
}

std::string DeviceNameManager::GetLocalDisplayDeviceName(const std::string &prefixName, const std::string &suffixName,
    int32_t maxNamelength)
{
    return DmDeviceNameManager::dmDeviceNameManager_->GetLocalDisplayDeviceName(prefixName, suffixName, maxNamelength);
}

int32_t DeviceNameManager::InitDeviceNameWhenUserSwitch(int32_t curUserId, int32_t preUserId)
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenUserSwitch(curUserId, preUserId);
}

int32_t DeviceNameManager::InitDeviceNameWhenLogout()
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenLogout();
}

int32_t DeviceNameManager::InitDeviceNameWhenLogin()
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenLogin();
}

int32_t DeviceNameManager::InitDeviceNameWhenNickChange()
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenNickChange();
}

int32_t DeviceNameManager::InitDeviceNameWhenNameChange(int32_t userId)
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenNameChange(userId);
}

int32_t DeviceNameManager::ModifyUserDefinedName(const std::string &deviceName)
{
    return DmDeviceNameManager::dmDeviceNameManager_->ModifyUserDefinedName(deviceName);
}

int32_t DeviceNameManager::RestoreLocalDeviceName()
{
    return DmDeviceNameManager::dmDeviceNameManager_->RestoreLocalDeviceName();
}

int32_t DeviceNameManager::InitDeviceNameWhenLanguageOrRegionChanged()
{
    return DmDeviceNameManager::dmDeviceNameManager_->InitDeviceNameWhenLanguageOrRegionChanged();
}

std::string DeviceNameManager::GetUserDefinedDeviceName()
{
    return DmDeviceNameManager::dmDeviceNameManager_->GetUserDefinedDeviceName();
}

int32_t DeviceNameManager::GetUserDefinedDeviceName(int32_t userId, std::string &deviceName)
{
    return DmDeviceNameManager::dmDeviceNameManager_->GetUserDefinedDeviceName(userId, deviceName);
}

std::string DeviceNameManager::GetLocalMarketName()
{
    return DmDeviceNameManager::dmDeviceNameManager_->GetLocalMarketName();
}
} // namespace DistributedHardware
} // namespace OHOS
