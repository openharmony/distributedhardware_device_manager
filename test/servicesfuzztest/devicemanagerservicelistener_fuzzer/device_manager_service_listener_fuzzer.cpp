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
#include <cstring>
#include <fuzzer/FuzzedDataProvider.h>
#include "device_manager_service_listener_fuzzer.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceListenerFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t pkgNameSize = 128;
    int32_t maxPidSize = 99999;
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int32_t publishResult = fdp.ConsumeIntegral<int32_t>();
    std::string pkgName = fdp.ConsumeRandomLengthString(pkgNameSize);
    int32_t pid = fdp.ConsumeIntegralInRange<int32_t>(0, maxPidSize);
    ProcessInfo processInfo;
    processInfo.userId = pid;
    processInfo.pkgName = pkgName;

    DeviceManagerServiceListener listener;
    listener.OnServicePublishResult(processInfo, serviceId, publishResult);
}

void OnServiceFoundFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t pkgNameSize = 64;
    int32_t serviceInfoSize = 32;
    ProcessInfo processInfo;
    processInfo.userId = fdp.ConsumeIntegral<int32_t>();
    processInfo.pkgName = fdp.ConsumeRandomLengthString(pkgNameSize);
    int32_t discServiceId = fdp.ConsumeIntegral<int32_t>();

    DiscoveryServiceInfo discServiceInfo;
    discServiceInfo.pkgName = fdp.ConsumeRandomLengthString(serviceInfoSize);
    discServiceInfo.serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    discServiceInfo.serviceInfo.serviceType = fdp.ConsumeRandomLengthString(serviceInfoSize);
    discServiceInfo.serviceInfo.serviceName = fdp.ConsumeRandomLengthString(serviceInfoSize);
    discServiceInfo.serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(serviceInfoSize);

    DeviceManagerServiceListener listener;
    listener.OnServiceFound(processInfo, discServiceId, discServiceInfo);
}

void OnServiceDiscoveryResultFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t pkgNameSize = 64;
    ProcessInfo processInfo;
    processInfo.userId = fdp.ConsumeIntegral<int32_t>();
    processInfo.pkgName = fdp.ConsumeRandomLengthString(pkgNameSize);
    int32_t discServiceId = fdp.ConsumeIntegral<int32_t>();
    int32_t reason = fdp.ConsumeIntegral<int32_t>();

    DeviceManagerServiceListener listener;
    listener.OnServiceDiscoveryResult(processInfo, discServiceId, reason);
}

void OnServicePublishResultFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t pkgNameSize = 128;
    int32_t maxPidSize = 99999;
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int32_t publishResult = fdp.ConsumeIntegral<int32_t>();
    std::string pkgName = fdp.ConsumeRandomLengthString(pkgNameSize);
    int32_t pid = fdp.ConsumeIntegralInRange<int32_t>(0, maxPidSize);
    ProcessInfo processInfo;
    processInfo.userId = pid;
    processInfo.pkgName = pkgName;

    DeviceManagerServiceListener listener;
    listener.OnServicePublishResult(processInfo, serviceId, publishResult);
}
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const size_t minSize = sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t);
    if ((data == nullptr) || (size < minSize)) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::DeviceManagerServiceListenerFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceFoundFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceDiscoveryResultFuzzTest(fdp);
    OHOS::DistributedHardware::OnServicePublishResultFuzzTest(fdp);

    return 0;
}