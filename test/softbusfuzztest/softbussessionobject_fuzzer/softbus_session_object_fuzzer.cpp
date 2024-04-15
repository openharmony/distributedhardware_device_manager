/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "softbus_session.h"
#include "softbus_connector.h"
#include "softbus_session_object_fuzzer.h"
namespace OHOS {
namespace DistributedHardware {

class SoftbusSessionCallbackTest : public ISoftbusSessionCallback {
public:
    SoftbusSessionCallbackTest() {}
    virtual ~SoftbusSessionCallbackTest() {}
    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) override
    {
        (void)sessionId;
        (void)sessionSide;
        (void)result;
    }
    void OnSessionClosed(int32_t sessionId) override
    {
        (void)sessionId;
    }
    void OnDataReceived(int32_t sessionId, std::string message) override
    {
        (void)sessionId;
        (void)message;
    }
    bool GetIsCryptoSupport() override
    {
        return true;
    }
    void OnUnbindSessionOpened(int32_t socket, PeerSocketInfo info) override
    {
        (void)socket;
        (void)info;
    }
    void OnAuthDeviceDataReceived(int32_t sessionId, std::string message) override
    {
        (void)sessionId;
        (void)message;
    }
    void BindSocketSuccess(int32_t socket) override
    {
        (void)socket;
    }
    void BindSocketFail() override {}
};

void SoftBusSessionFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    int32_t socket =  *(reinterpret_cast<const int*>(data));
    QoSEvent eventId = static_cast<QoSEvent>(1);
    uint32_t qosCount = 3;
    QosTV qos[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = 64 * 1024 },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = 19000},
        { .qos = QOS_TYPE_MIN_LATENCY, .value = 500 },
    };
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_UNKNOWN;

    std::shared_ptr<SoftbusSession> softbusSession = std::make_shared<SoftbusSession>();
    softbusSession->RegisterSessionCallback(std::make_shared<SoftbusSessionCallbackTest>());
    softbusSession->iSocketListener_.OnBytes(socket, data, size);
    softbusSession->iSocketListener_.OnShutdown(socket, reason);
    softbusSession->iSocketListener_.OnQos(socket, eventId, qos, qosCount);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftBusSessionFuzzTest(data, size);

    return 0;
}
