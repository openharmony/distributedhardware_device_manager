/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <chrono>
#include <fuzzer/FuzzedDataProvider.h>
#include <securec.h>
#include <string>

#include "device_manager_impl.h"
#include "dm_constants.h"
#include "softbus_listener.h"
#include "refresh_softbus_lnn_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void RefreshSoftbusLNNFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint16_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgNameStr = fdp.ConsumeRandomLengthString();
    const char *pkgName = pkgNameStr.c_str();
    DmSubscribeInfo dmSubInfo;
    dmSubInfo.subscribeId = fdp.ConsumeIntegral<uint16_t>();
    dmSubInfo.mode = DM_DISCOVER_MODE_ACTIVE;
    dmSubInfo.medium = DM_USB;
    dmSubInfo.isSameAccount = fdp.ConsumeBool();
    dmSubInfo.isWakeRemote = fdp.ConsumeBool();
    std::string customData = fdp.ConsumeRandomLengthString();

    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    softbusListener->RefreshSoftbusLNN(pkgName, dmSubInfo, customData);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::RefreshSoftbusLNNFuzzTest(data, size);

    return 0;
}
