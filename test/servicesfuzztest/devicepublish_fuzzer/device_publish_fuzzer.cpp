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
#include <string>
#include <unistd.h>
#include <fuzzer/FuzzedDataProvider.h>

#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "device_publish_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10 * 1000;
}

class DevicePublishCallbackTest : public PublishCallback {
public:
    DevicePublishCallbackTest() : PublishCallback() {}
    virtual ~DevicePublishCallbackTest() {}
    void OnPublishResult(int32_t publishId, int32_t failedReason) override {}
};

void DevicePublishFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string bundleName = fdp.ConsumeRandomLengthString();
    DmPublishInfo publishInfo;
    publishInfo.publishId = fdp.ConsumeIntegral<int32_t>();
    publishInfo.mode = fdp.PickValueInArray<DmDiscoverMode>({
        DM_DISCOVER_MODE_PASSIVE,
        DM_DISCOVER_MODE_ACTIVE
    });
    publishInfo.freq = fdp.PickValueInArray<DmExchangeFreq>({
        DM_LOW,
        DM_MID,
        DM_HIGH,
        DM_SUPER_HIGH
    });
    int32_t publishId = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<PublishCallback> callback = std::make_shared<DevicePublishCallbackTest>();
    DeviceManager::GetInstance().PublishDeviceDiscovery(bundleName, publishInfo, callback);
    usleep(SLEEP_TIME_US);
    DeviceManager::GetInstance().UnPublishDeviceDiscovery(bundleName, publishId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DevicePublishFuzzTest(data, size);
    return 0;
}
