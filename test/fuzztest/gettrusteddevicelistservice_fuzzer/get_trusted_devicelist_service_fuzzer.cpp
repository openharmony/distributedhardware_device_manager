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
#include "get_trusted_devicelist_service_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void GetTrustedDeviceListFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string extra(reinterpret_cast<const char*>(data), size);
    std::vector<DmDeviceInfo> deviceList;

    DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, pkgName, pkgName);
    DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, pkgName, pkgName);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::GetTrustedDeviceListFuzzTest(data, size);

    return 0;
}