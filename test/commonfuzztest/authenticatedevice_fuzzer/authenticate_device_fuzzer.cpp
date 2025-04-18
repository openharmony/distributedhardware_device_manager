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

#include <cstddef>
#include <cstdint>
#include <string>
#include <unistd.h>

#include "device_manager.h"
#include "device_manager_callback.h"
#include "device_manager_impl.h"
#include "dm_crypto.h"
#include "accesstoken_kit.h"
#include "authenticate_device_fuzzer.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
class AuthenticateCallbackTest : public AuthenticateCallback {
public:
    virtual ~AuthenticateCallbackTest() {}
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
        int32_t reason) override {}
};

class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    virtual ~DmInitCallbackTest() {}
    void OnRemoteDied() override {}
};

class DeviceStateCallbackTest : public DeviceStateCallback {
public:
    DeviceStateCallbackTest() : DeviceStateCallback() {}
    virtual ~DeviceStateCallbackTest() {}
    void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceReady(const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override {}
};

class DeviceStatusCallbackTest : public DeviceStatusCallback {
public:
    virtual ~DeviceStatusCallbackTest() { }
    void OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo) override {}
};

class DeviceDiscoveryCallbackTest : public DiscoveryCallback {
public:
    DeviceDiscoveryCallbackTest() : DiscoveryCallback() {}
    virtual ~DeviceDiscoveryCallbackTest() {}
    void OnDiscoverySuccess(uint16_t subscribeId) override {}
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override {}
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo) override{}
};

class DevicePublishCallbackTest : public PublishCallback {
public:
    DevicePublishCallbackTest() : PublishCallback() {}
    virtual ~DevicePublishCallbackTest() {}
    void OnPublishResult(int32_t publishId, int32_t failedReason) override {}
};

class DeviceManagerUiCallbackTest : public DeviceManagerUiCallback {
public:
    virtual ~DeviceManagerUiCallbackTest() {}
    void OnCall(const std::string &paramJson) override {}
};

std::string g_returnStr;
std::string g_reqJsonStr = R"(
{
    "userId":"4269DC28B639681698809A67EDAD08E39F207900038F91EFF95DD042FE2874E4"
}
)";

std::string g_credentialInfo = R"(
{
    "processType" : 1,
    "g_authType" : 1,
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

std::string g_deleteInfo = R"(
{
    "processType" : 1,
    "g_authType" : 1,
    "userId" : "123"
}
)";

DmDeviceInfo g_deviceInfo = {
    .deviceId = "123456789101112131415",
    .deviceName = "deviceName",
    .deviceTypeId = 1
};

DmSubscribeInfo g_subscribeInfo = {
    .subscribeId = 0,
    .mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE,
    .medium = DmExchangeMedium::DM_AUTO,
    .freq = DmExchangeFreq::DM_MID,
    .isSameAccount = true,
    .isWakeRemote = true,
};

DmPublishInfo g_publishInfo = {
    .publishId = 1234,
    .mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE,
    .freq = DmExchangeFreq::DM_MID,
    .ranging = true,
};

PeerTargetId g_targetId = {
    .deviceId = "deviceId",
    .brMac = "brMac",
    .bleMac = "bleMac",
    .wifiIp = "wifiIp",
};

DmDeviceInfo g_getDeviceInfo;
DmPinType g_pinType = DmPinType::SUPER_SONIC;

uint64_t g_tokenId = 1;
int32_t g_authType = -1;
int32_t g_action = 2;
int32_t g_eventId = 1;
int32_t g_bindType = 1;
int32_t g_securityLevel = 1;
int64_t g_accessControlId = 1;

bool g_isRefresh = false;

std::vector<DmDeviceInfo> g_deviceList;
std::vector<DmDeviceBasicInfo> g_deviceBasic;

std::shared_ptr<DmInitCallback> g_initcallback = std::make_shared<DmInitCallbackTest>();
std::shared_ptr<DeviceStateCallback> g_stateCallback = std::make_shared<DeviceStateCallbackTest>();
std::shared_ptr<AuthenticateCallback> g_callbackk = std::make_shared<AuthenticateCallbackTest>();
std::shared_ptr<DeviceStatusCallback> g_statusCallback = std::make_shared<DeviceStatusCallbackTest>();
std::shared_ptr<DiscoveryCallback> g_discoveryCallback = std::make_shared<DeviceDiscoveryCallbackTest>();
std::shared_ptr<PublishCallback> g_publishCallback = std::make_shared<DevicePublishCallbackTest>();
std::shared_ptr<DeviceManagerUiCallback> g_Uicallback = std::make_shared<DeviceManagerUiCallbackTest>();

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

void AuthenticateDeviceFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AddPermission();
    std::string str(reinterpret_cast<const char*>(data), size);

    DeviceManagerImpl::GetInstance().ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());

    DeviceManager::GetInstance().InitDeviceManager(str, g_initcallback);
    DeviceManager::GetInstance().RegisterDevStateCallback(str, str, g_stateCallback);
    DeviceManager::GetInstance().RegisterDevStatusCallback(str, str, g_statusCallback);
    DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(str, g_Uicallback);
    std::string emptyStr = "";
    DeviceManager::GetInstance().AuthenticateDevice(str, g_authType, g_deviceInfo, emptyStr, g_callbackk);
    DeviceManager::GetInstance().UnAuthenticateDevice(str, g_deviceInfo);
    std::string pkgName = "pkgName";
    DeviceManager::GetInstance().StartDeviceDiscovery(pkgName, g_subscribeInfo, pkgName, g_discoveryCallback);
    DeviceManager::GetInstance().StopDeviceDiscovery(str, g_subscribeInfo.subscribeId);
    DeviceManager::GetInstance().StartDeviceDiscovery(pkgName, g_tokenId, pkgName, g_discoveryCallback);
    DeviceManager::GetInstance().StopDeviceDiscovery(g_tokenId, str);
    DeviceManager::GetInstance().PublishDeviceDiscovery(str, g_publishInfo, g_publishCallback);
    DeviceManager::GetInstance().UnPublishDeviceDiscovery(str, g_publishInfo.publishId);
    DeviceManager::GetInstance().UnInitDeviceManager(str);
}

void AuthenticateDeviceSecondFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AddPermission();
    std::string str(reinterpret_cast<const char*>(data), size);

    DeviceManagerImpl::GetInstance().ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());

    DeviceManager::GetInstance().GetTrustedDeviceList(str, str, g_deviceList);
    DeviceManager::GetInstance().GetTrustedDeviceList(str, str, g_isRefresh, g_deviceList);
    DeviceManager::GetInstance().GetAvailableDeviceList(str, g_deviceBasic);
    DeviceManager::GetInstance().GetDeviceInfo(str, str, g_getDeviceInfo);
    DeviceManager::GetInstance().GetLocalDeviceInfo(str, g_getDeviceInfo);
    DeviceManager::GetInstance().GetUdidByNetworkId(str, str, g_returnStr);
    DeviceManager::GetInstance().GetUuidByNetworkId(str, str, g_returnStr);
    DeviceManager::GetInstance().DpAclAdd(g_accessControlId, str, g_bindType);
    DeviceManager::GetInstance().CreatePinHolder(str, g_targetId, g_pinType, str);
    DeviceManager::GetInstance().DestroyPinHolder(str, g_targetId, g_pinType, str);
    DeviceManager::GetInstance().CheckAccessToTarget(g_tokenId, str);
    DeviceManager::GetInstance().IsSameAccount(str);
}

void AuthenticateDeviceThirdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AddPermission();
    std::string str = Crypto::Sha256(data, size);

    DeviceManagerImpl::GetInstance().ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());

    DeviceManager::GetInstance().SetUserOperation(str, g_action, str);
    DeviceManager::GetInstance().RequestCredential(str, g_returnStr);
    DeviceManager::GetInstance().RequestCredential(str, g_reqJsonStr, g_returnStr);
    DeviceManager::GetInstance().ImportCredential(str, g_credentialInfo);
    DeviceManager::GetInstance().DeleteCredential(str, g_deleteInfo);
    DeviceManager::GetInstance().CheckCredential(str, g_reqJsonStr, g_returnStr);
    DeviceManager::GetInstance().ImportCredential(str, g_reqJsonStr, g_returnStr);
    DeviceManager::GetInstance().DeleteCredential(str, g_reqJsonStr, g_returnStr);
    DeviceManager::GetInstance().NotifyEvent(str, g_eventId, str);
    std::string emptyStr = "";
    DeviceManager::GetInstance().BindDevice(str, g_authType, str, emptyStr, g_callbackk);
    DeviceManager::GetInstance().UnBindDevice(str, g_deviceInfo.deviceId);
    DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(str);
    DeviceManager::GetInstance().UnRegisterDevStateCallback(str);
    DeviceManager::GetInstance().UnRegisterDevStatusCallback(str);
    std::map<std::string, std::string> authParam;
    authParam[DM_AUTHENTICATION_TYPE] = str;
    DeviceManager::GetInstance().RegisterAuthenticationType(str, authParam);
}

void AuthenticateDeviceFourthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AddPermission();
    std::string str(reinterpret_cast<const char*>(data), size);

    DeviceManagerImpl::GetInstance().ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());
    std::string emptyStr = "";
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    int32_t indexTwo = 2;
    int32_t numOne = 1;
    int32_t numOneTwoTimes = 11;
    int32_t numOneThreeTimes = 111;
    int32_t numOneSixTimes = 111111;

    DeviceManagerImpl::GetInstance().ConvertDeviceInfoToDeviceBasicInfo(info, deviceBasicInfo);
    DeviceManagerImpl::GetInstance().GetTrustedDeviceList(str, emptyStr, g_deviceList);
    DeviceManagerImpl::GetInstance().GetTrustedDeviceList(str, emptyStr, false, g_deviceList);
    DeviceManagerImpl::GetInstance().GetDeviceInfo(str, emptyStr, info);
    DeviceManagerImpl::GetInstance().RegisterDevStatusCallback(str, emptyStr, nullptr);
    DeviceManagerImpl::GetInstance().UnRegisterDevStateCallback(str);
    DeviceManagerImpl::GetInstance().UnRegisterDevStatusCallback(emptyStr);
    DeviceManagerImpl::GetInstance().StartDeviceDiscovery(str, g_subscribeInfo, emptyStr, nullptr);
    DeviceManagerImpl::GetInstance().StartDeviceDiscovery(str, numOneSixTimes, emptyStr, nullptr);
    DeviceManagerImpl::GetInstance().StopDeviceDiscovery(str, numOneTwoTimes);
    DeviceManagerImpl::GetInstance().StopDeviceDiscovery(numOneThreeTimes, "");
    DeviceManagerImpl::GetInstance().PublishDeviceDiscovery(str, g_publishInfo, nullptr);
    DeviceManagerImpl::GetInstance().UnPublishDeviceDiscovery(str, numOneTwoTimes);
    DeviceManagerImpl::GetInstance().AuthenticateDevice(str, numOne, info, emptyStr, nullptr);
    DeviceManagerImpl::GetInstance().RegisterDeviceManagerFaCallback(str, nullptr);
    DeviceManagerImpl::GetInstance().UnRegisterDeviceManagerFaCallback(str);
    DeviceManagerImpl::GetInstance().VerifyAuthentication(str, emptyStr, nullptr);
    PeerTargetId targetId;
    std::map<std::string, std::string> discoverParam;
    DeviceManagerImpl::GetInstance().BindTarget(str, targetId, discoverParam, nullptr);
    DeviceManagerImpl::GetInstance().UnbindTarget(str, targetId, discoverParam, nullptr);
    DeviceManagerImpl::GetInstance().GetTrustedDeviceList(str, discoverParam, false, g_deviceList);
    DeviceManagerImpl::GetInstance().RegisterDevStateCallback(str, discoverParam, nullptr);
    DeviceManagerImpl::GetInstance().AddDiscoveryCallback("test", discoverParam, nullptr);
    DeviceManagerImpl::GetInstance().RemoveDiscoveryCallback("test");
    DeviceManagerImpl::GetInstance().AddPublishCallback("test");
    DeviceManagerImpl::GetInstance().RemovePublishCallback("test");
    DeviceManagerImpl::GetInstance().RegisterPinHolderCallback(str, nullptr);
    DeviceManagerImpl::GetInstance().GetDeviceSecurityLevel(str, emptyStr, indexTwo);
    DeviceManagerImpl::GetInstance().IsSameAccount(emptyStr);
}

void AuthenticateDeviceFifthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AddPermission();
    std::string str(reinterpret_cast<const char*>(data), size);

    DeviceManagerImpl::GetInstance().ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());
    std::string emptyStr = "";
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    int32_t indexTwo = 2;
    DmAuthParam dmFaParam;
    DeviceManagerImpl::GetInstance().GetFaParam(str, dmFaParam);
    DeviceManagerImpl::GetInstance().SetUserOperation(str, 1, emptyStr);
    DeviceManagerImpl::GetInstance().GetUdidByNetworkId(str, emptyStr, g_returnStr);
    DeviceManagerImpl::GetInstance().GetUuidByNetworkId(str, emptyStr, g_returnStr);
    DeviceManagerImpl::GetInstance().RegisterDevStateCallback(str, emptyStr);
    DeviceManagerImpl::GetInstance().UnRegisterDevStateCallback(str, emptyStr);
    DeviceManagerImpl::GetInstance().RegisterUiStateCallback(str);
    DeviceManagerImpl::GetInstance().UnRegisterUiStateCallback(str);
    DeviceManagerImpl::GetInstance().RequestCredential(str, g_reqJsonStr, g_returnStr);
    DeviceManagerImpl::GetInstance().ImportCredential(str, emptyStr);
    DeviceManagerImpl::GetInstance().DeleteCredential(str, emptyStr);
    DeviceManagerImpl::GetInstance().RegisterCredentialCallback(str, nullptr);
    DeviceManagerImpl::GetInstance().UnRegisterCredentialCallback(str);
    DeviceManagerImpl::GetInstance().NotifyEvent(str, 1, emptyStr);
    DeviceManagerImpl::GetInstance().RequestCredential(str, g_returnStr);
    DeviceManagerImpl::GetInstance().CheckCredential(str, g_reqJsonStr, g_returnStr);
    DeviceManagerImpl::GetInstance().GetEncryptedUuidByNetworkId(str, emptyStr, g_returnStr);
    DeviceManagerImpl::GetInstance().GenerateEncryptedUuid(str, emptyStr, emptyStr, g_returnStr);
    DeviceManagerImpl::GetInstance().BindDevice(str, 1, emptyStr, emptyStr, nullptr);
    DeviceManagerImpl::GetInstance().UnBindDevice(str, emptyStr);
    DeviceManagerImpl::GetInstance().GetNetworkTypeByNetworkId(str, emptyStr, indexTwo);
    DeviceManagerImpl::GetInstance().ImportAuthCode(emptyStr, emptyStr);
    DeviceManagerImpl::GetInstance().ExportAuthCode(g_returnStr);
    std::map<std::string, std::string> discoverParam;
    DeviceManagerImpl::GetInstance().StartDiscovering(str, discoverParam, discoverParam, nullptr);
    DeviceManagerImpl::GetInstance().StopDiscovering(str, discoverParam);
    DeviceManagerImpl::GetInstance().RegisterDiscoveryCallback(str, discoverParam, discoverParam, nullptr);
    DeviceManagerImpl::GetInstance().UnRegisterDiscoveryCallback(str);
    DeviceManagerImpl::GetInstance().StartAdvertising(str, discoverParam, nullptr);
    DeviceManagerImpl::GetInstance().StopAdvertising(str, discoverParam);
    DeviceManagerImpl::GetInstance().SetDnPolicy(str, discoverParam);
    DeviceManagerImpl::GetInstance().RegisterDeviceScreenStatusCallback(emptyStr, nullptr);
    DeviceManagerImpl::GetInstance().UnRegisterDeviceScreenStatusCallback(emptyStr);
    DeviceManagerImpl::GetInstance().GetDeviceScreenStatus(emptyStr, emptyStr, indexTwo);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthenticateDeviceFirstFuzzTest(data, size);
    OHOS::DistributedHardware::AuthenticateDeviceSecondFuzzTest(data, size);
    OHOS::DistributedHardware::AuthenticateDeviceThirdFuzzTest(data, size);
    OHOS::DistributedHardware::AuthenticateDeviceFourthFuzzTest(data, size);
    OHOS::DistributedHardware::AuthenticateDeviceFifthFuzzTest(data, size);
    return 0;
}
