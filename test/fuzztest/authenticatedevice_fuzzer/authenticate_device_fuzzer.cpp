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
#include <string>
#include <unistd.h>

#include "device_manager.h"
#include "device_manager_callback.h"
#include "authenticate_device_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
class AuthenticateCallbackTest : public AuthenticateCallback {
public:
    virtual ~AuthenticateCallbackTest()
    {
    }
    virtual void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
                              int32_t reason) override {}
};

void AuthenticateDeviceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    int32_t authType = *(reinterpret_cast<const int32_t*>(data));
    DmDeviceInfo deviceInfo;
    std::string extraString(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();

    DeviceManager::GetInstance().AuthenticateDevice(pkgName, authType, deviceInfo, extraString, callback);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthenticateDeviceFuzzTest(data, size);
    return 0;
}
