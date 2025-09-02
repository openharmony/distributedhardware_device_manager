/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <chrono>
#include <securec.h>
#include <string>
#include <fuzzer/FuzzedDataProvider.h>

#include "device_manager_impl.h"
#include "dm_constants.h"
#include "softbus_listener.h"
#include "softbus_listener_get_local_deviceinfo_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void GetLocalDeviceInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t subscribeId = fdp.ConsumeIntegral<int32_t>();
    int publishId = fdp.ConsumeIntegral<int>();
    PublishResult pResult = (PublishResult)1;
    RefreshResult rResult = (RefreshResult)1;
    DmDeviceInfo deviceInfo;
    NodeBasicInfoType type = NodeBasicInfoType::TYPE_DEVICE_NAME;

    DeviceInfo *device = nullptr;
    NodeBasicInfo *info = nullptr;

    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();

    softbusListener->GetLocalDeviceInfo(deviceInfo);
    softbusListener->OnSoftbusPublishResult(publishId, pResult);
    softbusListener->OnSoftbusDeviceOnline(info);
    softbusListener->OnSoftbusDeviceOffline(info);
    softbusListener->OnSoftbusDeviceInfoChanged(type, info);
    softbusListener->OnSoftbusDeviceFound(device);
    softbusListener->OnSoftbusDiscoveryResult(subscribeId, rResult);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::GetLocalDeviceInfoFuzzTest(data, size);

    return 0;
}
