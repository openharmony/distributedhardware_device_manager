/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OHOS_DM_DEVICE_NAME_MANAGER_MOCK_H
#define OHOS_DM_DEVICE_NAME_MANAGER_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "device_name_manager.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeviceNameManager {
public:
    virtual ~DmDeviceNameManager() = default;

    virtual void DataShareReady() = 0;
    virtual void AccountSysReady(int32_t userId) = 0;
    virtual int32_t InitDeviceNameWhenSoftBusReady() = 0;
    virtual int32_t GetLocalDisplayDeviceName(int32_t maxNamelength, std::string &displayName) = 0;
    virtual std::string GetLocalDisplayDeviceName(const std::string &prefixName, const std::string &suffixName,
        int32_t maxNamelength) = 0;
    virtual int32_t InitDeviceNameWhenUserSwitch(int32_t curUserId, int32_t preUserId) = 0;
    virtual int32_t InitDeviceNameWhenLogout() = 0;
    virtual int32_t InitDeviceNameWhenLogin() = 0;
    virtual int32_t InitDeviceNameWhenNickChange() = 0;
    virtual int32_t InitDeviceNameWhenNameChange(int32_t userId) = 0;
    virtual int32_t ModifyUserDefinedName(const std::string &deviceName) = 0;
    virtual int32_t RestoreLocalDeviceName() = 0;
    virtual int32_t InitDeviceNameWhenLanguageOrRegionChanged() = 0;
    virtual std::string GetUserDefinedDeviceName() = 0;
    virtual int32_t GetUserDefinedDeviceName(int32_t userId, std::string &deviceName) = 0;
    virtual std::string GetLocalMarketName() = 0;

    static inline std::shared_ptr<DmDeviceNameManager> dmDeviceNameManager_ = nullptr;
};

class DeviceNameManagerMock : public DmDeviceNameManager {
public:
    MOCK_METHOD(void, DataShareReady, ());
    MOCK_METHOD(void, AccountSysReady, (int32_t));
    MOCK_METHOD(int32_t, InitDeviceNameWhenSoftBusReady, ());
    MOCK_METHOD(int32_t, GetLocalDisplayDeviceName, (int32_t, std::string &));
    MOCK_METHOD(std::string, GetLocalDisplayDeviceName, (const std::string &, const std::string &, int32_t));
    MOCK_METHOD(int32_t, InitDeviceNameWhenUserSwitch, (int32_t, int32_t));
    MOCK_METHOD(int32_t, InitDeviceNameWhenLogout, ());
    MOCK_METHOD(int32_t, InitDeviceNameWhenLogin, ());
    MOCK_METHOD(int32_t, InitDeviceNameWhenNickChange, ());
    MOCK_METHOD(int32_t, InitDeviceNameWhenNameChange, (int32_t));
    MOCK_METHOD(int32_t, ModifyUserDefinedName, (const std::string &));
    MOCK_METHOD(int32_t, RestoreLocalDeviceName, ());
    MOCK_METHOD(int32_t, InitDeviceNameWhenLanguageOrRegionChanged, ());
    MOCK_METHOD(std::string, GetUserDefinedDeviceName, ());
    MOCK_METHOD(int32_t, GetUserDefinedDeviceName, (int32_t, std::string &));
    MOCK_METHOD(std::string, GetLocalMarketName, ());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_NAME_MANAGER_MOCK_H
