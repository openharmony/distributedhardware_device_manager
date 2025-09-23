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

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "init_device_manager_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    virtual ~DmInitCallbackTest() {}
    void OnRemoteDied() override {}
};

void InitDeviceManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string packName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();

    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
    callback = nullptr;
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::InitDeviceManagerFuzzTest(data, size);
    return 0;
}
