/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "device_manager_service_listener.h"
#include "dm_auth_manager.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "hichain_connector.h"
#include "softbus_bus_center.h"
#include "softbus_connector.h"
#include "softbus_session.h"
#include "on_session_opened_fuzzer.h"

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

void OnSessionOpenedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int) + sizeof(int))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int sessionId = fdp.ConsumeIntegral<int>();
    int result = fdp.ConsumeIntegral<int>();

    std::shared_ptr<SoftbusSession> softbusSession = std::make_shared<SoftbusSession>();
    softbusSession->RegisterSessionCallback(std::make_shared<SoftbusSessionCallbackTest>());
    softbusSession->OnSessionOpened(sessionId, result);
    softbusSession->OnSessionClosed(sessionId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::OnSessionOpenedFuzzTest(data, size);

    return 0;
}
