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

void OnServiceOnlineFuzzTest(FuzzedDataProvider &fdp)
{
    std::vector<int64_t> serviceIds;
    while (fdp.remaining_bytes() >= sizeof(int64_t)) {
        serviceIds.push_back(fdp.ConsumeIntegral<int64_t>());
    }

    DeviceManagerNotify::GetInstance().OnServiceOnline(serviceIds);
}

void ServiceInfoOnlineFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    std::vector<std::pair<std::shared_ptr<ServiceInfoStateCallback>, int64_t>> callbackInfo;
    while (fdp.remaining_bytes() >= sizeof(int64_t)) {
        int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
        callbackInfo.emplace_back(callback, serviceId);
    }

    DeviceManagerNotify::GetInstance().ServiceInfoOnline(callbackInfo);
}

void RegisterServiceStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string key = fdp.ConsumeRandomLengthString(size);
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterServiceStateCallback(key, callback);
}

void UnRegisterServiceStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string key = fdp.ConsumeRandomLengthString(size);

    DeviceManagerNotify::GetInstance().UnRegisterServiceStateCallback(key);
}

void RegisterServicePublishCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    auto callback = std::make_shared<MockServicePublishCallback>();
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(serviceId, callback);
}

void UnRegisterServicePublishCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    DeviceManagerNotify::GetInstance().UnRegisterServicePublishCallback(serviceId);
}

void OnServicePublishResultFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int32_t publishResult = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerNotify::GetInstance().OnServicePublishResult(serviceId, publishResult);
}

void RegisterServiceDiscoveryCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(discoveryServiceId, callback);
}

void UnRegisterServiceDiscoveryCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerNotify::GetInstance().UnRegisterServiceDiscoveryCallback(discoveryServiceId);
}

void OnServiceFoundFuzzTest(FuzzedDataProvider &fdp)
{
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

void OnServiceDiscoveryResultFuzzTest(FuzzedDataProvider &fdp)
{
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
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t))) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::DeviceManagerNotifyUnRegisterFuzzTest(fdp);
    OHOS::DistributedHardware::DeviceManagerNotifyDeviceStatusFuzzTest(fdp);
    OHOS::DistributedHardware::DeviceManagerNotifyOnPublishResultFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceOnlineFuzzTest(fdp);
    OHOS::DistributedHardware::ServiceInfoOnlineFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterServiceStateCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterServiceStateCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterServicePublishCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterServicePublishCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::OnServicePublishResultFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterServiceDiscoveryCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterServiceDiscoveryCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceFoundFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceDiscoveryResultFuzzTest(fdp);

    return 0;
}
