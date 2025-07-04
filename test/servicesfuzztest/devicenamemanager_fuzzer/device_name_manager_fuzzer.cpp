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

#include "device_name_manager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>

#include "device_name_manager.h"
#include "dm_constants.h"
#include "dm_datashare_common_event.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
constexpr int32_t INT32_SIZE = 5;
}

std::shared_ptr<DeviceNameManager> deviceNameMgr_ = std::make_shared<DeviceNameManager>();
void DeviceNameManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < (sizeof(int32_t) * INT32_SIZE))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    deviceNameMgr_->InitDeviceNameWhenSoftBusReady();
    int32_t curUserId = fdp.ConsumeIntegral<int32_t>();
    deviceNameMgr_->InitDeviceNameWhenUserSwitch(curUserId, curUserId);
    deviceNameMgr_->InitDeviceNameWhenLogout();
    deviceNameMgr_->InitDeviceNameWhenLogin();
    deviceNameMgr_->InitDeviceNameWhenNickChange();
    deviceNameMgr_->InitDeviceNameWhenNameChange(curUserId);
    int32_t preUserId = fdp.ConsumeIntegral<int32_t>();
    deviceNameMgr_->RegisterDeviceNameChangeMonitor(curUserId, preUserId);
    deviceNameMgr_->UnRegisterDeviceNameChangeMonitor(curUserId);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    deviceNameMgr_->InitDeviceName(userId);

    deviceNameMgr_->InitDeviceName(-1);
    std::string prefixName(reinterpret_cast<const char*>(data), size);
    std::string subffixName(reinterpret_cast<const char*>(data), size);
    deviceNameMgr_->InitDeviceNameToSoftBus(prefixName, subffixName);

    int32_t maxNamelength = fdp.ConsumeIntegral<int32_t>();
    std::string displayName(reinterpret_cast<const char*>(data), size);
    deviceNameMgr_->GetLocalDisplayDeviceName(maxNamelength, displayName);
    deviceNameMgr_->ModifyUserDefinedName(displayName);
    deviceNameMgr_->RestoreLocalDeviceName();
    std::string nickName(reinterpret_cast<const char*>(data), size);
    std::string deviceName(reinterpret_cast<const char*>(data), size);
    deviceNameMgr_->InitDisplayDeviceNameToSettingsData(nickName, deviceName, userId);
    deviceNameMgr_->GetUserDefinedDeviceName(userId, deviceName);
    std::string str(reinterpret_cast<const char*>(data), size);
    int32_t maxNumBytes = fdp.ConsumeIntegral<int32_t>();
    deviceNameMgr_->SubstrByBytes(str, maxNumBytes);
    deviceNameMgr_->GetSystemLanguage();
    deviceNameMgr_->GetLocalMarketName();
    deviceNameMgr_->SetUserDefinedDeviceName(deviceName, userId);
    deviceNameMgr_->GetDisplayDeviceName(userId, deviceName);
    std::string state(reinterpret_cast<const char*>(data), size);
    deviceNameMgr_->SetDisplayDeviceNameState(state, userId);
}

void DeviceNameManagerFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string deviceName(reinterpret_cast<const char*>(data), size);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    deviceNameMgr_->SetDisplayDeviceName(deviceName, userId);
    deviceNameMgr_->SetDisplayDeviceName("", userId);
    deviceNameMgr_->GetDeviceName(deviceName);
    deviceNameMgr_->SetDeviceName(deviceName);
    deviceNameMgr_->SetDeviceName("");
    deviceNameMgr_->GetRemoteObj();
    std::string tableName(reinterpret_cast<const char*>(data), size);
    std::string key(reinterpret_cast<const char*>(data), size);
    std::string value(reinterpret_cast<const char*>(data), size);
    deviceNameMgr_->GetValue(tableName, userId, key, value);
    deviceNameMgr_->SetValue(tableName, userId, key, value);
    std::string proxyUri(reinterpret_cast<const char*>(data), size);
    deviceNameMgr_->CreateDataShareHelper(proxyUri);
    deviceNameMgr_->GetProxyUriStr(tableName, userId);
    deviceNameMgr_->MakeUri(proxyUri, key);
    deviceNameMgr_->MakeUri("", key);
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
    deviceNameMgr_->ReleaseDataShareHelper(helper);
}

void DeviceNameManagerSecondFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DeviceNameManager> deviceNameMgr = std::make_shared<DeviceNameManager>();
    deviceNameMgr->UnInit();
    deviceNameMgr->GetUserDefinedDeviceName();
    deviceNameMgr_->GetSystemRegion();
    DmDataShareCommonEventManager eventManager;
    eventManager.eventValidFlag_ = true;
    eventManager.UnsubscribeDataShareCommonEvent();
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceNameManagerFuzzTest(data, size);
    OHOS::DistributedHardware::DeviceNameManagerFirstFuzzTest(data, size);
    OHOS::DistributedHardware::DeviceNameManagerSecondFuzzTest(data, size);
    return 0;
}
