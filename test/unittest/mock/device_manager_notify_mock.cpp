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

#include "device_manager_notify_mock.h"
#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify);
int32_t DeviceManagerNotify::RegisterGetDeviceProfileInfoListCallback(const std::string &pkgName,
    std::shared_ptr<GetDeviceProfileInfoListCallback> callback)
{
    return DmDeviceManagerNotify::dmDeviceManagerNotify->RegisterGetDeviceProfileInfoListCallback(pkgName, callback);
}

int32_t DeviceManagerNotify::RegisterGetDeviceIconInfoCallback(const std::string &pkgName, const std::string &uk,
    std::shared_ptr<GetDeviceIconInfoCallback> callback)
{
    return DmDeviceManagerNotify::dmDeviceManagerNotify->RegisterGetDeviceIconInfoCallback(pkgName, uk, callback);
}

int32_t DeviceManagerNotify::RegisterServiceStateCallback(const std::string &key,
    std::shared_ptr<ServiceInfoStateCallback> callback)
{
    return DmDeviceManagerNotify::dmDeviceManagerNotify->RegisterServiceStateCallback(key, callback);
}

int32_t DeviceManagerNotify::UnRegisterServiceStateCallback(const std::string &key)
{
    return DmDeviceManagerNotify::dmDeviceManagerNotify->UnRegisterServiceStateCallback(key);
}
} // namespace DistributedHardware
} // namespace OHOS