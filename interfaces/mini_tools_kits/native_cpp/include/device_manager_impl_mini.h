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

#ifndef OHOS_MINI_DM_DEVICE_MANAGER_IMPL_MINI_H
#define OHOS_MINI_DM_DEVICE_MANAGER_IMPL_MINI_H

#include "device_manager_mini.h"

#include <map>
#include <mutex>
#include <set>

#include "ipc_client_manager_mini.h"
#include "ipc_client_proxy.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImplMini : public DeviceManagerMini {
public:
    static DeviceManagerImplMini &GetInstance();

public:
    /**
     * @tc.name: DeviceManagerImpl::InitDeviceManager
     * @tc.desc: Initialize DeviceManager
     * @tc.type: FUNC
     */
    virtual int32_t InitDeviceManager(const std::string &pkgName) override;
    /**
     * @tc.name: DeviceManagerImpl::UnInitDeviceManager
     * @tc.desc: UnInitialize DeviceManager
     * @tc.type: FUNC
     */
    virtual int32_t UnInitDeviceManager(const std::string &pkgName) override;
    int32_t OnDmServiceDied();
    virtual int32_t GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
        std::string &displayName) override;

private:
    DeviceManagerImplMini() = default;
    ~DeviceManagerImplMini() = default;
    DeviceManagerImplMini(const DeviceManagerImplMini &) = delete;
    DeviceManagerImplMini &operator=(const DeviceManagerImplMini &) = delete;
    DeviceManagerImplMini(DeviceManagerImplMini &&) = delete;
    DeviceManagerImplMini &operator=(DeviceManagerImplMini &&) = delete;

private:
    std::shared_ptr<IpcClientProxy> ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManagerMini>());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MINI_DM_DEVICE_MANAGER_IMPL_MINI_H
