/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <unordered_map>

#include "dp_inited_callback.h"
#include "dp_init_callback_fuzzer.h"
#include "dm_constants.h"
#include "json_object.h"


namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr uint32_t DATA_LEN = 10;
}
void DpInitedCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    DpInitedCallback dpInitedCallback;
    dpInitedCallback.PutAllTrustedDevices();
    dpInitedCallback.OnDpInited();
}

void DpInitedCallbackFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    DpInitedCallback dpInitedCallback;
    std::unordered_map<std::string, DmAuthForm> authFormMap;
    DmDeviceInfo deviceInfo;
    deviceInfo.extraData = "";
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;
    dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);

    deviceInfo.extraData = "extraInfo";
    dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);

    JsonObject extraJson;
    extraJson[PARAM_KEY_OS_VERSION] = 1;
    deviceInfo.extraData = extraJson.Dump();
    dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);

    extraJson[PARAM_KEY_OS_VERSION] = "os_version";
    extraJson[PARAM_KEY_OS_TYPE] = "os_type";
    deviceInfo.extraData = extraJson.Dump();
    dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);

    extraJson[PARAM_KEY_OS_TYPE] = DATA_LEN;
    deviceInfo.extraData = extraJson.Dump();
    dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DpInitedCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::DpInitedCallbackFirstFuzzTest(data, size);
    return 0;
}
