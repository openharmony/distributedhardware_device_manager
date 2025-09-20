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
#include <fuzzer/FuzzedDataProvider.h>
#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "get_uuid_by_network_id_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void GetUuidByNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string packName = fdp.ConsumeRandomLengthString();
    std::string netWorkId = fdp.ConsumeRandomLengthString();
    std::string uuid = fdp.ConsumeRandomLengthString();

    DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::GetUuidByNetworkIdFuzzTest(data, size);
    return 0;
}
