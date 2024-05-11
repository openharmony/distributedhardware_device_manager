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
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string extra(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string udid;
    std::string network;
    std::string device;
    int32_t authType = 1;
    DmAuthParam authParam;
    std::vector<DmDeviceBasicInfo> deviceBasicInfoList;
    int32_t deviceType = 0;
    int32_t level = *(reinterpret_cast<const int32_t*>(data));
    DeviceManagerService::GetInstance().Init();
    DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, extra);
    DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, extra);
    DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    DeviceManagerService::GetInstance().SetUserOperation(pkgName, authType, network);
    DeviceManagerService::GetInstance().RegisterDeviceManagerListener(pkgName);
    DeviceManagerService::GetInstance().UnRegisterDeviceManagerListener(pkgName);
    DeviceManagerService::GetInstance().GetAvailableDeviceList(pkgName, deviceBasicInfoList);
    DeviceManagerService::GetInstance().GetLocalDeviceNetworkId(network);
    DeviceManagerService::GetInstance().GetLocalDeviceId(pkgName, device);
    DeviceManagerService::GetInstance().GetLocalDeviceType(deviceType);
    DeviceManagerService::GetInstance().GetLocalDeviceName(pkgName);
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