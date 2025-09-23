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
#include "pinholder_fuzzer.h"

#include "device_manager.h"
#include "device_manager_callback.h"
#include "dm_ability_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "pin_holder.h"
#include "json_object.h"
#include "parameter.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
class PinHolderCallbackFuzzTest : public PinHolderCallback {
public:
    virtual ~PinHolderCallbackFuzzTest() {}

    void OnPinHolderCreate(const std::string &deviceId, DmPinType pinType, const std::string &payload) override {}
    void OnPinHolderDestroy(DmPinType pinType, const std::string &payload) override {}
    void OnCreateResult(int32_t result) override {}
    void OnDestroyResult(int32_t result) override {}
    void OnPinHolderEvent(DmPinHolderEvent event, int32_t result, const std::string &content) override {}
};

void PinHolderFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint16_t) + sizeof(uint16_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string payload = fdp.ConsumeRandomLengthString();
    PeerTargetId peerTargetId;
    peerTargetId.deviceId = pkgName;
    peerTargetId.brMac = pkgName;
    peerTargetId.bleMac = pkgName;
    peerTargetId.wifiIp = pkgName;
    peerTargetId.wifiPort = fdp.ConsumeIntegral<uint16_t>();
    uint16_t tmp = fdp.ConsumeIntegral<uint16_t>();
    DmPinType pinType = static_cast<DmPinType>(tmp);

    std::shared_ptr<PinHolderCallback> pinHolderCallback = std::make_shared<PinHolderCallbackFuzzTest>();

    DeviceManager::GetInstance().RegisterPinHolderCallback(pkgName, pinHolderCallback);
    DeviceManager::GetInstance().CreatePinHolder(pkgName, peerTargetId, pinType, payload);
    DeviceManager::GetInstance().DestroyPinHolder(pkgName, peerTargetId, pinType, payload);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::PinHolderFuzzTest(data, size);

    return 0;
}
