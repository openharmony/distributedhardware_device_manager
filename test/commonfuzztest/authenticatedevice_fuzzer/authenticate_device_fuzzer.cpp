/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "accesstoken_kit.h"
#include "authenticate_device_fuzzer.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {
class AuthenticateCallbackTest : public AuthenticateCallback {
public:
    virtual ~AuthenticateCallbackTest()
    {
    }
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
        int32_t reason) override {}
};

class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    virtual ~DmInitCallbackTest() {}
    void OnRemoteDied() override {}
};

void AuthenticateDeviceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

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

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    int32_t authType = 1;
    DmDeviceInfo deviceInfo;
    if (strcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, "123456789101112131415")) {
        return;
    }
    if (strcpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, "deviceName")) {
        return;
    }
    deviceInfo.deviceTypeId = 1;

    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    std::string extraString(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    std::string deviceId(reinterpret_cast<const char*>(data), size);

    DeviceManager::GetInstance().InitDeviceManager(pkgName, initcallback);
    DeviceManager::GetInstance().AuthenticateDevice(pkgName, authType, deviceInfo, extraString, callback);
    DeviceManager::GetInstance().BindDevice(pkgName, authType, deviceId, extraString, callback);

    DeviceManager::GetInstance().UnAuthenticateDevice(pkgName, deviceInfo);
    DeviceManager::GetInstance().UnBindDevice(pkgName, deviceInfo.deviceId);
    DeviceManager::GetInstance().UnInitDeviceManager(pkgName);
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
