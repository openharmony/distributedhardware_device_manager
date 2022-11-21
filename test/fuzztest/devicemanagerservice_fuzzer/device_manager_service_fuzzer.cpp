/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "device_manager_service.h"
#include "device_manager_service_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string extra(reinterpret_cast<const char*>(data), size);
    uint16_t subscribeId = 12;
    int32_t publishId = 14;
    DmSubscribeInfo subscribeInfo;
    subscribeInfo.subscribeId = 1;
    DmPublishInfo publishInfo;

    DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceManagerServiceFuzzTest(data, size);

    return 0;
}