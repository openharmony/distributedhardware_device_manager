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

#include "dm_discovery_manager.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "device_manager_service_listener.h"
#include "ipc_server_listener.h"
#include "softbus_bus_center.h"
#include "softbus_connector.h"
#include "dm_discovery_manager_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {

void DmDiscoveryManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint16_t))) {
        return;
    }

    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmDiscoveryManager> dmDiscoveryManager =
        std::make_shared<DmDiscoveryManager>(softbusConnector, listener);
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    DmSubscribeInfo subscribeInfo;
    std::string extra(reinterpret_cast<const char*>(data), size);
    uint16_t subscribeId = *(reinterpret_cast<const uint16_t*>(data));

    dmDiscoveryManager->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    dmDiscoveryManager->StopDeviceDiscovery(pkgName, subscribeId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmDiscoveryManagerFuzzTest(data, size);

    return 0;
}
