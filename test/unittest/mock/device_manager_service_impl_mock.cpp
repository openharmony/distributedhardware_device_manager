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

#include "device_manager_service_impl_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DeviceManagerServiceImpl::StopAuthenticateDevice(const std::string &pkgName)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->StopAuthenticateDevice(pkgName);
}

int32_t DeviceManagerServiceImpl::GetBindLevel(const std::string &pkgName, const std::string &localUdid,
    const std::string &udid, uint64_t &tokenId)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->GetBindLevel(pkgName, localUdid, udid, tokenId);
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->UnBindDevice(pkgName, udid, bindLevel);
}
} // namespace DistributedHardware
} // namespace OHOS