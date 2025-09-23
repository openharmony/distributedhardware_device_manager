/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "register_dev_state_callback_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceStatusCallbackFuzzTest : public DeviceStatusCallback {
public:
    virtual ~DeviceStatusCallbackFuzzTest() {}

    void OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo) override {}
};

void RegisterDevStateCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string bundleName = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DeviceStatusCallback> deviceStatusCallback = std::make_shared<DeviceStatusCallbackFuzzTest>();

    DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName, extra, deviceStatusCallback);
    DeviceManager::GetInstance().UnRegisterDevStateCallback(bundleName);
    DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::RegisterDevStateCallbackFuzzTest(data, size);
    return 0;
}
