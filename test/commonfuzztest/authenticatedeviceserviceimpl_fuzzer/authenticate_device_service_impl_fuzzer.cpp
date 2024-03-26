/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "device_manager_service_impl.h"
#include "authenticate_device_service_impl_fuzzer.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {
DmPublishInfo publishInfo = {
    .publishId = 1234,
    .mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE,
    .freq = DmExchangeFreq::DM_MID,
    .ranging = true,
};

PeerTargetId peerTargetId = {
    .deviceId = "deviceId",
    .brMac = "brMac",
    .bleMac = "bleMac",
    .wifiIp = "wifiIp",
    .wifiPort = 1,
};

std::vector<DmDeviceInfo> deviceList;
std::map<std::string, std::string> bindParam;
std::string returnJsonStr;
int32_t eventId = 1;
int32_t action = 1;
int32_t authType = -1;
uint16_t subscribeId = 123;

std::string reqJsonStr = R"(
{
    "userId":"4269DC28B639681698809A67EDAD08E39F207900038F91EFF95DD042FE2874E4"
}
)";

std::string credentialInfo = R"(
{
    "processType" : 1,
    "authType" : 1,
    "userId" : "123",
    "credentialData" :
    [
        {
            "credentialType" : 1,
            "credentialId" : "104",
            "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
            "serverPk" : "",
            "pkInfoSignature" : "",
            "pkInfo" : "",
            "peerDeviceId" : ""
        }
    ]
}
)";

std::string deleteInfo = R"(
{
    "processType" : 1,
    "authType" : 1,
    "userId" : "123"
}
)";

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

void AuthenticateDeviceServiceImplFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string str(reinterpret_cast<const char*>(data), size);
    AddPermission();
    DmSubscribeInfo subscribeInfo = {
        .subscribeId = 0,
        .mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE,
        .medium = DmExchangeMedium::DM_AUTO,
        .freq = DmExchangeFreq::DM_MID,
        .isSameAccount = true,
        .isWakeRemote = true,
    };
    if (strcpy_s(subscribeInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, "capability")) {
        return;
    }
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();

    deviceManagerServiceImpl->Initialize(listener);
    deviceManagerServiceImpl->RegisterUiStateCallback(str);
    deviceManagerServiceImpl->RegisterCredentialCallback(str);
    deviceManagerServiceImpl->StartDeviceDiscovery(str, subscribeInfo, str);
    deviceManagerServiceImpl->StopDeviceDiscovery(str, subscribeInfo.subscribeId);
    deviceManagerServiceImpl->StartDeviceDiscovery(str, subscribeId, str);
    deviceManagerServiceImpl->StopDeviceDiscovery(str, subscribeId);
    deviceManagerServiceImpl->PublishDeviceDiscovery(str, publishInfo);
    deviceManagerServiceImpl->UnPublishDeviceDiscovery(str, publishInfo.publishId);
    deviceManagerServiceImpl->AuthenticateDevice(str, authType, str, str);
    deviceManagerServiceImpl->UnAuthenticateDevice(str, str);
    deviceManagerServiceImpl->BindDevice(str, authType, str, str);
    deviceManagerServiceImpl->UnBindDevice(str, str);
    deviceManagerServiceImpl->SetUserOperation(str, action, str);
    deviceManagerServiceImpl->RequestCredential(reqJsonStr, returnJsonStr);
    deviceManagerServiceImpl->ImportCredential(str, credentialInfo);
    deviceManagerServiceImpl->DeleteCredential(str, deleteInfo);
    deviceManagerServiceImpl->MineRequestCredential(str, returnJsonStr);
    deviceManagerServiceImpl->CheckCredential(str, reqJsonStr, returnJsonStr);
    deviceManagerServiceImpl->ImportCredential(str, reqJsonStr, returnJsonStr);
    deviceManagerServiceImpl->DeleteCredential(str, reqJsonStr, returnJsonStr);
    deviceManagerServiceImpl->NotifyEvent(str, eventId, str);
    deviceManagerServiceImpl->GetGroupType(deviceList);
    deviceManagerServiceImpl->GetUdidHashByNetWorkId(str.c_str(), str);
    deviceManagerServiceImpl->ImportAuthCode(str, str);
    deviceManagerServiceImpl->ExportAuthCode(str);
    deviceManagerServiceImpl->BindTarget(str, peerTargetId, bindParam);
    deviceManagerServiceImpl->UnRegisterCredentialCallback(str);
    deviceManagerServiceImpl->UnRegisterUiStateCallback(str);
    deviceManagerServiceImpl->Release();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthenticateDeviceServiceImplFuzzTest(data, size);

    return 0;
}