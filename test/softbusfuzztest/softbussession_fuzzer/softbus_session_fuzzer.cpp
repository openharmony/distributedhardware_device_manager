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
#include <fuzzer/FuzzedDataProvider.h>
#include "softbus_connector.h"
#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "softbus_session.h"
#include "softbus_session_fuzzer.h"

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
    void OnAuthDeviceDataReceived(int32_t sessionId, std::string message) override
    {
        (void)sessionId;
        (void)message;
    }
};

void SoftBusSessionFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int) + sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int result = fdp.ConsumeIntegral<int>();
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string str(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<SoftbusSession> softbusSession = std::make_shared<SoftbusSession>();

    softbusSession->RegisterSessionCallback(std::make_shared<SoftbusSessionCallbackTest>());
    softbusSession->OnSessionOpened(result, result);
    softbusSession->OpenAuthSession(str);
    softbusSession->CloseAuthSession(sessionId);
    softbusSession->GetPeerDeviceId(sessionId, str);
    softbusSession->SendData(sessionId, str);
    softbusSession->SendHeartbeatData(sessionId, str);
    softbusSession->OnSessionClosed(result);
    softbusSession->UnRegisterSessionCallback();
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
