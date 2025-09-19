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

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <unistd.h>

#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "register_device_manager_fa_callback_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10 * 1000;
}

class DeviceManagerFaCallbackTest : public DeviceManagerUiCallback {
public:
    DeviceManagerFaCallbackTest() : DeviceManagerUiCallback() {}
    ~DeviceManagerFaCallbackTest() override {}
    void OnCall(const std::string &paramJson) override {}
};

void RegisterDeviceManagerFaCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string packageName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();

    DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packageName, callback);
    usleep(SLEEP_TIME_US);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::RegisterDeviceManagerFaCallbackFuzzTest(data, size);
    return 0;
}
