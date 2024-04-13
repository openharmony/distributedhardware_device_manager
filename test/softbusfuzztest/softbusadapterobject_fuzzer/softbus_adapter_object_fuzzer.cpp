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

#include "device_manager_service_listener.h"
#include "softbus_bus_center.h"
#include "softbus_adapter.h"
#include "softbus_connector.h"
#include "softbus_adapter_object_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusAdapterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    int32_t socket =  *(reinterpret_cast<const int*>(data));
    std::string str(reinterpret_cast<const char*>(data), size);
    uint32_t qosCount = 3;
    StreamData streamData;
    StreamData ext;
    StreamFrameInfo frameInfo;
    QoSEvent eventId = static_cast<QoSEvent>(1);
    QosTV qos[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = 64 * 1024 },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = 19000 },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = 500 },
    };
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_UNKNOWN;
    PeerSocketInfo info = {
        .name = const_cast<char*>(str.c_str()),
        .pkgName = const_cast<char*>(str.c_str()),
        .networkId = const_cast<char*>(str.c_str()),
    };

    SoftbusAdapter::GetInstance().iSocketListener_.OnBind(socket, info);
    SoftbusAdapter::GetInstance().iSocketListener_.OnShutdown(socket, reason);
    SoftbusAdapter::GetInstance().iSocketListener_.OnBytes(socket, data, size);
    SoftbusAdapter::GetInstance().iSocketListener_.OnMessage(socket, data, size);
    SoftbusAdapter::GetInstance().iSocketListener_.OnStream(socket, &streamData, &ext, &frameInfo);
    SoftbusAdapter::GetInstance().iSocketListener_.OnQos(socket, eventId, qos, qosCount);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftbusAdapterFuzzTest(data, size);

    return 0;
}
