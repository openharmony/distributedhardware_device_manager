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

int32_t DeviceManagerNotify::RegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId,
    std::shared_ptr<ServiceInfoStateCallback> callback)
{
    return DmDeviceManagerNotify::dmDeviceManagerNotify->RegisterServiceStateCallback(pkgName, serviceId, callback);
}

int32_t DeviceManagerNotify::UnRegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId)
{
    return DmDeviceManagerNotify::dmDeviceManagerNotify->UnRegisterServiceStateCallback(pkgName, serviceId);
}

void DeviceManagerNotify::RegisterServicePublishCallback(const std::string &pkgName, int64_t serviceId,
    std::shared_ptr<ServicePublishCallback> callback)
{
    DmDeviceManagerNotify::dmDeviceManagerNotify->RegisterServicePublishCallback(pkgName, serviceId, callback);
}

void DeviceManagerNotify::UnRegisterServicePublishCallback(const std::string &pkgName, int64_t serviceId)
{
    DmDeviceManagerNotify::dmDeviceManagerNotify->UnRegisterServicePublishCallback(pkgName, serviceId);
}

void DeviceManagerNotify::RegisterServiceDiscoveryCallback(const std::string &pkgName, const std::string &serviceType,
    std::shared_ptr<ServiceDiscoveryCallback> callback)
{
    DmDeviceManagerNotify::dmDeviceManagerNotify->RegisterServiceDiscoveryCallback(pkgName, serviceType, callback);
}

void DeviceManagerNotify::UnRegisterServiceDiscoveryCallback(const std::string &pkgName, const std::string &serviceType)
{
    DmDeviceManagerNotify::dmDeviceManagerNotify->UnRegisterServiceDiscoveryCallback(pkgName, serviceType);
}

void DeviceManagerNotify::RegisterSyncServiceInfoCallback(const std::string &pkgName, int32_t localUserId,
    const std::string &networkId, std::shared_ptr<SyncServiceInfoCallback> callback, int64_t serviceId)
{
    DmDeviceManagerNotify::dmDeviceManagerNotify->RegisterSyncServiceInfoCallback(pkgName, localUserId,
        networkId, callback, serviceId);
}

void DeviceManagerNotify::UnRegisterSyncServiceInfoCallback(const std::string &pkgName, int32_t localUserId,
    const std::string &networkId, int64_t serviceId)
{
    DmDeviceManagerNotify::dmDeviceManagerNotify->UnRegisterSyncServiceInfoCallback(pkgName, localUserId,
        networkId, serviceId);
}
} // namespace DistributedHardware
} // namespace OHOS