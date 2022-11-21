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

#include "softbus_connector.h"
#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_publish_manager.h"
#include "dm_subscribe_info.h"
#include "softbus_discovery_callback.h"
#include "softbus_publish_callback.h"
#include "softbus_session.h"
#include "softbus_state_callback.h"
#include "softbus_connector_register_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusStateCallbackFuzzTest : public ISoftbusStateCallback {
public:
    virtual ~SoftbusStateCallbackFuzzTest() {}

    void OnDeviceOnline(const std::string &pkgName, const DmDeviceInfo &info) override {}
    void OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &info) override {}
    void OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &info) override {}
    void OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &info) override {}
};

class SoftbusDiscoveryCallbackFuzzTest : public ISoftbusDiscoveryCallback {
public:
    virtual ~SoftbusDiscoveryCallbackFuzzTest() {}

    void OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info) override {}
    void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId) override {}
    void OnDiscoveryFailed(const std::string &pkgName, int32_t subscribeId, int32_t failedReason) override {}
};

class SoftbusPublishCallbackFuzzTest : public ISoftbusPublishCallback {
public:
    virtual ~SoftbusPublishCallbackFuzzTest() {}

    void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult) override {}
};

void SoftBusConnectorRegisterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<ISoftbusPublishCallback> pubishCallback = std::make_shared<SoftbusPublishCallbackFuzzTest>();
    std::shared_ptr<ISoftbusDiscoveryCallback> discoveryCallback = std::make_shared<SoftbusDiscoveryCallbackFuzzTest>();
    std::shared_ptr<ISoftbusStateCallback> stateCallback = std::make_shared<SoftbusStateCallbackFuzzTest>();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();

    softbusConnector->RegisterSoftbusStateCallback(pkgName, stateCallback);
    softbusConnector->UnRegisterSoftbusStateCallback(pkgName);
    softbusConnector->RegisterSoftbusDiscoveryCallback(pkgName, discoveryCallback);
    softbusConnector->UnRegisterSoftbusDiscoveryCallback(pkgName);
    softbusConnector->RegisterSoftbusPublishCallback(pkgName, pubishCallback);
    softbusConnector->UnRegisterSoftbusPublishCallback(pkgName);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftBusConnectorRegisterFuzzTest(data, size);

    return 0;
}
