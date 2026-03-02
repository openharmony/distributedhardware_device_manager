/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "device_manager_service_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);
bool DeviceManagerService::IsDMServiceImplReady()
{
    return DmDeviceManagerService::dmDeviceManagerService->IsDMServiceImplReady();
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, std::vector<DmDeviceInfo> &deviceList)
{
    return DmDeviceManagerService::dmDeviceManagerService->GetTrustedDeviceList(pkgName, deviceList);
}

bool DeviceManagerService::IsDMServiceAdapterSoLoaded()
{
    return DmDeviceManagerService::dmDeviceManagerService->IsDMServiceAdapterSoLoaded();
}

bool DeviceManagerService::IsDMServiceAdapterResidentLoad()
{
    return DmDeviceManagerService::dmDeviceManagerService->IsDMServiceAdapterResidentLoad();
}

int32_t OpenAuthSessionWithPara(int64_t serviceId)
{
    return DmDeviceManagerService::dmDeviceManagerService->OpenAuthSessionWithPara(serviceId);
}
} // namespace DistributedHardware
} // namespace OHOS
