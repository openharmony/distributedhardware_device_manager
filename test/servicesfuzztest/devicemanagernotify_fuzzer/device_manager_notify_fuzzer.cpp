/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <string>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <fuzzer/FuzzedDataProvider.h>
#include "ipc_client_manager.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_rsp.h"
#include "ipc_def.h"
#include "device_manager_notify.h"
#include "dm_device_info.h"
#include "ipc_client_stub.h"
#include "ipc_register_listener_req.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "dm_constants.h"
#include "system_ability_definition.h"
#include "device_manager_notify_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerNotifyUnRegisterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    uint16_t subscribeId = 33;
    int32_t publishId = 123;

    DeviceManagerNotify::GetInstance().UnRegisterDeathRecipientCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(pkgName, subscribeId);
    DeviceManagerNotify::GetInstance().UnRegisterPublishCallback(pkgName, publishId);
    DeviceManagerNotify::GetInstance().UnRegisterAuthenticateCallback(pkgName, deviceId);
    DeviceManagerNotify::GetInstance().OnUiCall(pkgName, deviceId);
}

void DeviceManagerNotifyDeviceStatusFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string token(reinterpret_cast<const char*>(data), size);
    uint16_t subscribeId = 12;
    int32_t failedReason = 231;
    uint32_t status = 3;
    uint32_t reason = 14;
    DmDeviceInfo deviceInfo;
    DmDeviceBasicInfo deviceBasicInfo;

    DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, deviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, deviceBasicInfo);
    DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, deviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, deviceBasicInfo);
    DeviceManagerNotify::GetInstance().OnDeviceChanged(pkgName, deviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceReady(pkgName, deviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceReady(pkgName, deviceBasicInfo);
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, deviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, deviceBasicInfo);
    DeviceManagerNotify::GetInstance().OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    DeviceManagerNotify::GetInstance().OnDiscoverySuccess(pkgName, subscribeId);
    DeviceManagerNotify::GetInstance().OnAuthResult(pkgName, deviceId, token, status, reason);
}

void DeviceManagerNotifyOnPublishResultFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    int32_t publishId = fdp.ConsumeIntegral<int32_t>();
    int32_t publishResult = fdp.ConsumeIntegral<int32_t>();

    DeviceManagerNotify::GetInstance().OnPublishResult(pkgName, publishId, publishResult);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceManagerNotifyUnRegisterFuzzTest(data, size);
    OHOS::DistributedHardware::DeviceManagerNotifyDeviceStatusFuzzTest(data, size);
    OHOS::DistributedHardware::DeviceManagerNotifyOnPublishResultFuzzTest(data, size);

    return 0;
}
