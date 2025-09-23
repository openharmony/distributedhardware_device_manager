/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <fuzzer/FuzzedDataProvider.h>

#include "softbus_connector.h"
#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "softbus_session.h"
#include "softbus_connector_publish_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void SoftBusConnectorPublishFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(uint16_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();

    softbusConnector->HaveDeviceInMap(deviceId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftBusConnectorPublishFuzzTest(data, size);

    return 0;
}
