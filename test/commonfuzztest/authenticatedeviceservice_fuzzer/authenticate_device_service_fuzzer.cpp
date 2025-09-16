/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <fuzzer/FuzzedDataProvider.h>
#include "device_manager_service.h"
#include "authenticate_device_service_fuzzer.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {

void AddPermission()
{
    const int32_t permsNum = 3;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    const int32_t indexTwo = 2;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[indexTwo] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "device_manager",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
void AuthenticateDeviceServiceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    AddPermission();
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string udid = fdp.ConsumeRandomLengthString();
    std::string network = fdp.ConsumeRandomLengthString();
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    int32_t level = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, extra);
    DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, extra);
    DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    DeviceManagerService::GetInstance().SetUserOperation(pkgName, authType, network);
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName, userId);
    DeviceManagerService::GetInstance().UnRegisterCallerAppId(pkgName, userId);
    DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, deviceId, udid);
    DeviceManagerService::GetInstance().CheckApiPermission(level);
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
