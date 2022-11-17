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

#include <string>
#include <vector>
#include "device_manager_service.h"
#include "authenticate_device_service_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void AuthenticateDeviceServiceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string extra(reinterpret_cast<const char*>(data), size);
    int32_t authType = 123;
    DmAuthParam authParam;

    DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, pkgName, extra);
    DeviceManagerService::GetInstance().VerifyAuthentication(pkgName);
    DeviceManagerService::GetInstance().GetFaParam(pkgName, authParam);
    DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, extra);
    DeviceManagerService::GetInstance().SetUserOperation(pkgName, authType, extra);
    DeviceManagerService::GetInstance().RegisterDevStateCallback(pkgName, extra);
    DeviceManagerService::GetInstance().UnRegisterDevStateCallback(pkgName, extra);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthenticateDeviceServiceFuzzTest(data, size);

    return 0;
}