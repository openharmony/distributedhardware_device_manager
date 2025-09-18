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

#include "device_manager_notify_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#include "ipc_client_manager.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_rsp.h"
#include "ipc_def.h"
#include "ipc_client_stub.h"
#include "ipc_register_listener_req.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "device_manager_callback.h"
#include "device_manager_notify.h"
#include "dm_device_info.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
class ServiceDiscoveryCallbackTest : public ServiceDiscoveryCallback {
public:
    ServiceDiscoveryCallbackTest() = default;
    virtual ~ServiceDiscoveryCallbackTest() = default;
    void OnServiceFound(const DiscoveryServiceInfo &service) override {}
    void OnServiceDiscoveryResult(int32_t resReason) override {}
};

class MockServicePublishCallback : public ServicePublishCallback {
public:
    void OnServicePublishResult(int64_t serviceId, int32_t reason) override {}
};

class ServiceInfoStateCallbackTest : public ServiceInfoStateCallback {
public:
    ServiceInfoStateCallbackTest() = default;
    virtual ~ServiceInfoStateCallbackTest() = default;
    void OnServiceOnline(int64_t serviceId) override {}
    void OnServiceOffline(int64_t serviceId) override {}
};

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
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    int32_t publishId = *(reinterpret_cast<const int32_t*>(data));
    int32_t publishResult = *(reinterpret_cast<const int32_t*>(data));

    DeviceManagerNotify::GetInstance().OnPublishResult(pkgName, publishId, publishResult);
}

void OnServiceOnlineFuzzTest(const uint8_t* data, size_t size)
{
    constexpr size_t minRequiredSize = sizeof(int64_t) + 1;
    if ((data == nullptr) || (size < minRequiredSize)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::vector<int64_t> serviceIds;
    while (fdp.remaining_bytes() >= sizeof(int64_t)) {
        serviceIds.push_back(fdp.ConsumeIntegral<int64_t>());
    }

    DeviceManagerNotify::GetInstance().OnServiceOnline(serviceIds);
}

void ServiceInfoOnlineFuzzTest(const uint8_t* data, size_t size)
{
    constexpr size_t minRequiredSize = sizeof(int64_t) + 1;
    if ((data == nullptr) || (size < minRequiredSize)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    std::vector<std::pair<std::shared_ptr<ServiceInfoStateCallback>, int64_t>> callbackInfo;
    while (fdp.remaining_bytes() >= sizeof(int64_t)) {
        int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
        callbackInfo.emplace_back(callback, serviceId);
    }

    DeviceManagerNotify::GetInstance().ServiceInfoOnline(callbackInfo);
}

void RegisterServiceStateCallbackFuzzTest(const uint8_t* data, size_t size)
{
    constexpr size_t minRequiredSize = 2;
    if ((data == nullptr) || (size < minRequiredSize)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    std::string key = fdp.ConsumeRandomLengthString(size);
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterServiceStateCallback(key, callback);
}

void UnRegisterServiceStateCallbackFuzzTest(const uint8_t* data, size_t size)
{
    constexpr size_t minRequiredSize = 2;
    if ((data == nullptr) || (size < minRequiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string key = fdp.ConsumeRandomLengthString(size);

    DeviceManagerNotify::GetInstance().UnRegisterServiceStateCallback(key);
}

void RegisterServicePublishCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    auto callback = std::make_shared<MockServicePublishCallback>();
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(serviceId, callback);
}

void UnRegisterServicePublishCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    DeviceManagerNotify::GetInstance().UnRegisterServicePublishCallback(serviceId);
}

void OnServicePublishResultFuzzTest(const uint8_t* data, size_t size)
{
    const size_t minSize = sizeof(int64_t) + sizeof(int32_t);
    if ((data == nullptr) || (size < minSize)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int32_t publishResult = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerNotify::GetInstance().OnServicePublishResult(serviceId, publishResult);
}

void RegisterServiceDiscoveryCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(discoveryServiceId, callback);
}

void UnRegisterServiceDiscoveryCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerNotify::GetInstance().UnRegisterServiceDiscoveryCallback(discoveryServiceId);
}

void OnServiceFoundFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = 64;
    if ((data == nullptr) || (size < maxStringLength)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    DiscoveryServiceInfo service;
    service.serviceInfo.serviceName = fdp.ConsumeRandomLengthString(maxStringLength);
    service.serviceInfo.serviceType = fdp.ConsumeRandomLengthString(maxStringLength);
    service.serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();

    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(discoveryServiceId, callback);

    DeviceManagerNotify::GetInstance().OnServiceFound(discoveryServiceId, service);
    DeviceManagerNotify::GetInstance().OnServiceFound(-1, service);
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(discoveryServiceId, nullptr);
    DeviceManagerNotify::GetInstance().OnServiceFound(discoveryServiceId, service);
}

void OnServiceDiscoveryResultFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = sizeof(int32_t) + sizeof(int32_t);
    if ((data == nullptr) || (size < maxStringLength)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    int32_t resReason = fdp.ConsumeIntegral<int32_t>();

    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(discoveryServiceId, callback);
    DeviceManagerNotify::GetInstance().OnServiceDiscoveryResult(discoveryServiceId, resReason);
    DeviceManagerNotify::GetInstance().OnServiceDiscoveryResult(-1, resReason);
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(discoveryServiceId, nullptr);
    DeviceManagerNotify::GetInstance().OnServiceDiscoveryResult(discoveryServiceId, resReason);
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
    OHOS::DistributedHardware::OnServiceOnlineFuzzTest(data, size);
    OHOS::DistributedHardware::ServiceInfoOnlineFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterServiceStateCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::UnRegisterServiceStateCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterServicePublishCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::UnRegisterServicePublishCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::OnServicePublishResultFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterServiceDiscoveryCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::UnRegisterServiceDiscoveryCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::OnServiceFoundFuzzTest(data, size);
    OHOS::DistributedHardware::OnServiceDiscoveryResultFuzzTest(data, size);

    return 0;
}
