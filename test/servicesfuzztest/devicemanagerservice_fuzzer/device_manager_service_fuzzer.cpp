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

#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>
#include "device_manager_service.h"
#include "device_manager_service_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t MAX_STRING_LENGTH = 256;
enum class EmptyParamCase : int32_t {
    BOTH_EMPTY = 0,
    PKG_ONLY = 1,
    NETWORK_ONLY = 2,
    BOTH_NON_EMPTY = 3,
};
}

void DeviceManagerServiceFuzzTest(FuzzedDataProvider &fdp)
{
    int sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string inputStr = fdp.ConsumeRandomLengthString();
    std::string retStr = fdp.ConsumeRandomLengthString();
    DmPinType pinType = DmPinType::QR_CODE;
    uint16_t subscribeId = 12;
    int32_t publishId = 14;
    int64_t accessControlId = 1;
    DmDeviceInfo info;
    info.extraData = fdp.ConsumeRandomLengthString();
    PeerTargetId targetId;
    DmSubscribeInfo subscribeInfo;
    subscribeInfo.subscribeId = 1;
    DmPublishInfo publishInfo;
    std::map<std::string, std::string> parametricMap;
    uint32_t pinSize = fdp.ConsumeIntegral<uint32_t>();
    std::vector<uint8_t> pinData = fdp.ConsumeBytes<uint8_t>(pinSize);
    const void *pinDataPtr = pinData.data();

    DeviceManagerService::GetInstance().PublishDeviceDiscovery(inputStr, publishInfo);
    DeviceManagerService::GetInstance().RequestCredential(inputStr, inputStr);
    DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(inputStr, publishId);
    DeviceManagerService::GetInstance().GetDeviceInfo(inputStr, info);
    DeviceManagerService::GetInstance().GetLocalDeviceInfo(info);
    DeviceManagerService::GetInstance().GetDeviceSecurityLevel(inputStr, inputStr, publishId);
    DeviceManagerService::GetInstance().ImportAuthCode(inputStr, inputStr);
    DeviceManagerService::GetInstance().ExportAuthCode(inputStr);
    DeviceManagerService::GetInstance().StartDiscovering(inputStr, parametricMap, parametricMap);
    DeviceManagerService::GetInstance().StopDiscovering(inputStr, parametricMap);
    DeviceManagerService::GetInstance().EnableDiscoveryListener(inputStr, parametricMap, parametricMap);
    DeviceManagerService::GetInstance().DisableDiscoveryListener(inputStr, parametricMap);
    DeviceManagerService::GetInstance().StartAdvertising(inputStr, parametricMap);
    DeviceManagerService::GetInstance().StopAdvertising(inputStr, parametricMap);
    DeviceManagerService::GetInstance().BindTarget(inputStr, targetId, parametricMap);
    DeviceManagerService::GetInstance().UnbindTarget(inputStr, targetId, parametricMap);
    DeviceManagerService::GetInstance().RegisterPinHolderCallback(inputStr);
    DeviceManagerService::GetInstance().CreatePinHolder(inputStr, targetId, pinType, inputStr);
    DeviceManagerService::GetInstance().DestroyPinHolder(inputStr, targetId, pinType, inputStr);
    DeviceManagerService::GetInstance().OnPinHolderSessionOpened(sessionId, sessionId);
    DeviceManagerService::GetInstance().OnPinHolderBytesReceived(sessionId, pinDataPtr, pinSize);
    DeviceManagerService::GetInstance().OnPinHolderSessionClosed(sessionId);
    DeviceManagerService::GetInstance().ImportCredential(inputStr, inputStr);
    DeviceManagerService::GetInstance().DeleteCredential(inputStr, inputStr);
    DeviceManagerService::GetInstance().CheckCredential(inputStr, inputStr, inputStr);
    DeviceManagerService::GetInstance().ImportCredential(inputStr, inputStr, inputStr);
    DeviceManagerService::GetInstance().DeleteCredential(inputStr, inputStr, inputStr);
    DeviceManagerService::GetInstance().DpAclAdd(inputStr, accessControlId);
    DeviceManagerService::GetInstance().GetLocalDeviceName(retStr);
}

void StartDiscoveryServiceFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t maxStringLength = 64;
    std::string pkgName = fdp.ConsumeRandomLengthString(maxStringLength);
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = fdp.ConsumeRandomLengthString(maxStringLength);

    DeviceManagerService::GetInstance().StartDiscoveryService(pkgName, discParam);
    DeviceManagerService::GetInstance().StartDiscoveryService("", discParam);
}

void StopServiceDiscoveryFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t maxStringLength = 64;
    std::string pkgName = fdp.ConsumeRandomLengthString(maxStringLength);
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = fdp.ConsumeRandomLengthString(maxStringLength);

    DeviceManagerService::GetInstance().StopDiscoveryService(pkgName, discParam);
    DeviceManagerService::GetInstance().StopDiscoveryService("", discParam);
}

void DeviceManagerServiceTwoFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int64_t internalServiceId = fdp.ConsumeIntegral<int64_t>();
    DmPublishServiceParam publishServiceParam;
    DmRegisterServiceInfo serviceRegInfo;
    serviceRegInfo.serviceType = fdp.ConsumeIntegral<int32_t>();
    serviceRegInfo.serviceName = fdp.ConsumeRandomLengthString();
    serviceRegInfo.serviceDisplayName = fdp.ConsumeRandomLengthString();
    int64_t regServiceId = 0;
    int64_t publishServiceId = 0;
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerService::GetInstance().RegisterServiceInfo(serviceRegInfo, regServiceId);
    DeviceManagerService::GetInstance().UnRegisterServiceInfo(regServiceId);
    DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceId, publishServiceParam);
    DeviceManagerService::GetInstance().StopPublishService(pkgName, publishServiceId);
}

void GetOsTypeByNetworkIdFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string networkId = fdp.ConsumeRandomLengthString();
    int32_t osType = 0;
    DeviceManagerService::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
}

void GetOsTypeByNetworkIdWithEmptyParamsFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t choice = fdp.ConsumeIntegralInRange<int32_t>(static_cast<int32_t>(EmptyParamCase::BOTH_EMPTY),
        static_cast<int32_t>(EmptyParamCase::BOTH_NON_EMPTY));
    std::string pkgName;
    std::string networkId;
    int32_t osType = 0;
    
    switch (choice) {
        case static_cast<int32_t>(EmptyParamCase::BOTH_EMPTY):
            pkgName = "";
            networkId = "";
            break;
        case static_cast<int32_t>(EmptyParamCase::PKG_ONLY):
            pkgName = fdp.ConsumeRandomLengthString();
            networkId = "";
            break;
        case static_cast<int32_t>(EmptyParamCase::NETWORK_ONLY):
            pkgName = "";
            networkId = fdp.ConsumeRandomLengthString();
            break;
        case static_cast<int32_t>(EmptyParamCase::BOTH_NON_EMPTY):
            pkgName = fdp.ConsumeRandomLengthString();
            networkId = fdp.ConsumeRandomLengthString();
            break;
        default:
            break;
    }
    DeviceManagerService::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
}

void GetOsTypeByNetworkIdWithSpecialCharsFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string networkId = fdp.ConsumeRandomLengthString();
    int32_t osType = 0;
    
    std::string specialChars = "!@#$%^&*(){}[]|\\:;\"'<>,.?/~`";
    int32_t insertPos = fdp.ConsumeIntegralInRange<int32_t>(0, static_cast<int32_t>(networkId.length()));
    if (insertPos <= static_cast<int32_t>(networkId.length())) {
        networkId.insert(insertPos, specialChars);
    }
    
    DeviceManagerService::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::DeviceManagerServiceFuzzTest(fdp);
    OHOS::DistributedHardware::StartDiscoveryServiceFuzzTest(fdp);
    OHOS::DistributedHardware::StopServiceDiscoveryFuzzTest(fdp);
    OHOS::DistributedHardware::DeviceManagerServiceTwoFuzzTest(fdp);
    OHOS::DistributedHardware::GetOsTypeByNetworkIdFuzzTest(fdp);
    OHOS::DistributedHardware::GetOsTypeByNetworkIdWithEmptyParamsFuzzTest(fdp);
    OHOS::DistributedHardware::GetOsTypeByNetworkIdWithSpecialCharsFuzzTest(fdp);
    
    return 0;
}
