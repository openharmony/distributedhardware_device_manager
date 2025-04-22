/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dm_device_state_manager_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {

std::string DmDeviceStateManager::GetUdidByNetWorkId(std::string networkId)
{
    return DmDmDeviceStateManager::dmDeviceStateManager->GetUdidByNetWorkId(networkId);
}

int32_t DmDeviceStateManager::ProcNotifyEvent(const int32_t eventId, const std::string &deviceId)
{
    return DmDmDeviceStateManager::dmDeviceStateManager->ProcNotifyEvent(eventId, deviceId);
}
void DmDeviceStateManager::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    DmDmDeviceStateManager::dmDeviceStateManager->HandleDeviceStatusChange(devState, devInfo);
}
} // namespace DistributedHardware
} // namespace OHOS