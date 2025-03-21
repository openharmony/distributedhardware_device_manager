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

#ifndef OHOS_MINI_DM_DEVICE_MANAGER_MINI_H
#define OHOS_MINI_DM_DEVICE_MANAGER_MINI_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerMini {
public:
    static DeviceManagerMini &GetInstance();
public:
    /**
     * @brief Initialize DeviceManager.
     * @param pkgName        package name.
     * @return Returns 0 if success.
     */
    virtual int32_t InitDeviceManager(const std::string &pkgName) = 0;
    /**
     * @brief UnInitialize DeviceManager.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnInitDeviceManager(const std::string &pkgName) = 0;
    virtual int32_t GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
        std::string &displayName) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MINI_DM_DEVICE_MANAGER_MINI_H
