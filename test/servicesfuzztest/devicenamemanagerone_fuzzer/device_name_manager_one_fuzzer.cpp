/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <string>
#include <vector>
#include <fuzzer/FuzzedDataProvider.h>
#include "device_name_manager.h"
#include "device_name_manager_one_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceNameManagerOnePrivateFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t maxNamelength = fdp.ConsumeIntegral<int32_t>();
    std::string deviceName = fdp.ConsumeRandomLengthString();
    std::string prefix = fdp.ConsumeRandomLengthString();
    std::string suffix = fdp.ConsumeRandomLengthString();
    std::string state = fdp.ConsumeRandomLengthString();
    std::string tableName = fdp.ConsumeRandomLengthString();
    std::string key = fdp.ConsumeRandomLengthString();
    std::string proxyUri = fdp.ConsumeRemainingBytesAsString();
    std::string value;
    
    DeviceNameManager::GetInstance().RegisterDeviceNameChangeMonitor(fdp.ConsumeIntegral<int32_t>(),
        fdp.ConsumeIntegral<int32_t>());
    DeviceNameManager::GetInstance().UnRegisterDeviceNameChangeMonitor(userId);
    DeviceNameManager::GetInstance().InitDeviceName(userId);
    DeviceNameManager::GetInstance().InitDeviceNameToSoftBus(prefix, suffix);
    DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(
        prefix, suffix, maxNamelength);
    DeviceNameManager::GetInstance().SubstrByBytes(fdp.ConsumeRandomLengthString(),
        fdp.ConsumeIntegral<int32_t>());
    DeviceNameManager::GetInstance().GetSystemLanguage();
    DeviceNameManager::GetInstance().GetSystemRegion();
    DeviceNameManager::GetInstance().GetLocalMarketName();
    DeviceNameManager::GetInstance().InitDisplayDeviceNameToSettingsData(fdp.ConsumeRandomLengthString(),
        fdp.ConsumeRandomLengthString(), userId);
    DeviceNameManager::GetInstance().GetUserDefinedDeviceName(userId, deviceName);
    DeviceNameManager::GetInstance().SetUserDefinedDeviceName(fdp.ConsumeRandomLengthString(), userId);
    DeviceNameManager::GetInstance().GetDisplayDeviceName(userId, deviceName);
    DeviceNameManager::GetInstance().SetDisplayDeviceName(fdp.ConsumeRandomLengthString(), userId);
    DeviceNameManager::GetInstance().SetDisplayDeviceNameState(state, userId);
    DeviceNameManager::GetInstance().GetDeviceName(deviceName);
    DeviceNameManager::GetInstance().SetDeviceName(fdp.ConsumeRandomLengthString());
    DeviceNameManager::GetInstance().GetValue(tableName, userId, key, value);
    DeviceNameManager::GetInstance().SetValue(tableName, userId, key, fdp.ConsumeRandomLengthString());
    DeviceNameManager::GetInstance().GetProxyUriStr(tableName, userId);
    auto helper = DeviceNameManager::GetInstance().CreateDataShareHelper(proxyUri);
    DeviceNameManager::GetInstance().MakeUri(proxyUri, key);
    DeviceNameManager::GetInstance().ReleaseDataShareHelper(helper);
}
    
void DeviceNameManagerOneFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t maxNamelength = fdp.ConsumeIntegral<int32_t>();
    std::string displayName;
    std::string deviceName = fdp.ConsumeRandomLengthString();
    int32_t curUserId = fdp.ConsumeIntegral<int32_t>();
    int32_t preUserId = fdp.ConsumeIntegral<int32_t>();
    std::string prefix = fdp.ConsumeRandomLengthString();
    std::string suffix = fdp.ConsumeRandomLengthString();
    std::string state = fdp.ConsumeRandomLengthString();
    std::string tableName = fdp.ConsumeRandomLengthString();
    std::string key = fdp.ConsumeRandomLengthString();
    std::string proxyUri = fdp.ConsumeRemainingBytesAsString();

    DeviceNameManager::GetInstance().AccountSysReady(userId);
    DeviceNameManager::GetInstance().DataShareReady();
    DeviceNameManager::GetInstance().InitDeviceNameWhenSoftBusReady();
    DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(maxNamelength, displayName);
    DeviceNameManager::GetInstance().ModifyUserDefinedName(deviceName);
    DeviceNameManager::GetInstance().RestoreLocalDeviceName();
    DeviceNameManager::GetInstance().InitDeviceNameWhenUserSwitch(curUserId, preUserId);
    DeviceNameManager::GetInstance().InitDeviceNameWhenNameChange(userId);
    DeviceNameManager::GetInstance().InitDeviceNameWhenLogout();
    DeviceNameManager::GetInstance().InitDeviceNameWhenLogin();
    DeviceNameManager::GetInstance().InitDeviceNameWhenNickChange();
    DeviceNameManager::GetInstance().InitDeviceNameWhenLanguageOrRegionChanged();

    OHOS::DistributedHardware::DeviceNameManagerOnePrivateFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceNameManagerOneFuzzTest(data, size);

    return 0;
}
