/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "auth_manager.h"
#include "device_manager_service_listener.h"
#include "on_data_received_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
// AuthSrcManager fuzz
void OnDataReceivedSrcFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector,
        listener, hiChainAuthConnector);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string message = fdp.ConsumeRandomLengthString();
    authManager->OnDataReceived(sessionId, message);
    authManager->OnSessionClosed(sessionId);
}

// AuthSinkManager fuzz
void OnDataReceivedSinkFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector,
        listener, hiChainAuthConnector);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string message = fdp.ConsumeRandomLengthString();
    authManager->OnDataReceived(sessionId, message);
    authManager->OnSessionClosed(sessionId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::OnDataReceivedSrcFuzzTest(fdp);
    OHOS::DistributedHardware::OnDataReceivedSinkFuzzTest(fdp);
    return 0;
}