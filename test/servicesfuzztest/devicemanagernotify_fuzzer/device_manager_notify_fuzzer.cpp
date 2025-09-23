/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
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
void DeviceManagerNotifyUnRegisterFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    uint16_t subscribeId = fdp.ConsumeIntegral<uint16_t>();
    int32_t publishId = fdp.ConsumeIntegral<int32_t>();

    DeviceManagerNotify::GetInstance().UnRegisterDeathRecipientCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(pkgName, subscribeId);
    DeviceManagerNotify::GetInstance().UnRegisterPublishCallback(pkgName, publishId);
    DeviceManagerNotify::GetInstance().UnRegisterAuthenticateCallback(pkgName, deviceId);
    DeviceManagerNotify::GetInstance().OnUiCall(pkgName, deviceId);
}

void DeviceManagerNotifyDeviceStatusFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string token = fdp.ConsumeRandomLengthString();
    uint16_t subscribeId = fdp.ConsumeIntegral<uint16_t>();
    int32_t failedReason = fdp.ConsumeIntegral<int32_t>();
    uint32_t status = fdp.ConsumeIntegral<uint32_t>();
    uint32_t reason = fdp.ConsumeIntegral<uint32_t>();
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceTypeId = fdp.ConsumeIntegral<uint16_t>();
    deviceInfo.networkType = fdp.ConsumeIntegral<int32_t>();
    deviceInfo.range = fdp.ConsumeIntegral<int32_t>();
    deviceInfo.extraData = fdp.ConsumeRandomLengthString();
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

void DeviceManagerNotifyOnPublishResultFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
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
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t))) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::DeviceManagerNotifyUnRegisterFuzzTest(fdp);
    OHOS::DistributedHardware::DeviceManagerNotifyDeviceStatusFuzzTest(fdp);
    OHOS::DistributedHardware::DeviceManagerNotifyOnPublishResultFuzzTest(fdp);

    return 0;
}
