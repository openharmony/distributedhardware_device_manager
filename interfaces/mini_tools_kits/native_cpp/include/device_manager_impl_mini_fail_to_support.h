/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_MINI_DM_DEVICE_MANAGER_IMPL_MINI_FAIL_TO_SUPPORT_H
#define OHOS_MINI_DM_DEVICE_MANAGER_IMPL_MINI_FAIL_TO_SUPPORT_H

#include "device_manager_mini.h"

#include <map>
#include <mutex>
#include <set>

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImplMiniFailToSupport : public DeviceManagerMini {
public:
    static DeviceManagerImplMiniFailToSupport &GetInstance();

public:
    virtual int32_t InitDeviceManager(const std::string &pkgName) override;
    virtual int32_t UnInitDeviceManager(const std::string &pkgName) override;
    int32_t OnDmServiceDied();
    virtual int32_t GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
        std::string &displayName) override;

private:
    DeviceManagerImplMiniFailToSupport() = default;
    ~DeviceManagerImplMiniFailToSupport() = default;
    DeviceManagerImplMiniFailToSupport(const DeviceManagerImplMiniFailToSupport &) = delete;
    DeviceManagerImplMiniFailToSupport &operator=(const DeviceManagerImplMiniFailToSupport &) = delete;
    DeviceManagerImplMiniFailToSupport(DeviceManagerImplMiniFailToSupport &&) = delete;
    DeviceManagerImplMiniFailToSupport &operator=(DeviceManagerImplMiniFailToSupport &&) = delete;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MINI_DM_DEVICE_MANAGER_IMPL_MINI_FAIL_TO_SUPPORT_H
