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
void DeviceManagerServiceFuzzTest(FuzzedDataProvider &fdp)
{
    int sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string inputStr = fdp.ConsumeRandomLengthString();
    std::string retStr = fdp.ConsumeRandomLengthString();
    DmPinType pinType = DmPinType::QR_CODE;
    uint16_t subscribeId = 12;
    int32_t publishId = 14;
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
    DeviceManagerService::GetInstance().DpAclAdd(inputStr);
    DeviceManagerService::GetInstance().GetLocalDeviceName(retStr);
}

void StartServiceDiscoveryFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t maxStringLength = 32;
    std::string pkgName = fdp.ConsumeRandomLengthString(maxStringLength);
    DiscoveryServiceParam discParam;
    discParam.serviceType = fdp.ConsumeRandomLengthString(maxStringLength);
    discParam.discoveryServiceId = fdp.ConsumeIntegral<uint32_t>();

    DeviceManagerService::GetInstance().StartServiceDiscovery(pkgName, discParam);
    DeviceManagerService::GetInstance().StartServiceDiscovery("", discParam);
    discParam.discoveryServiceId = 0;
    DeviceManagerService::GetInstance().StartServiceDiscovery(pkgName, discParam);
}

void StopServiceDiscoveryFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t maxStringLength = 32;
    std::string pkgName = fdp.ConsumeRandomLengthString(maxStringLength);
    int32_t discServiceId = fdp.ConsumeIntegral<int32_t>();

    DeviceManagerService::GetInstance().StopServiceDiscovery(pkgName, discServiceId);
    DeviceManagerService::GetInstance().StopServiceDiscovery("", discServiceId);
    discServiceId = 0;
    DeviceManagerService::GetInstance().StopServiceDiscovery(pkgName, discServiceId);
}

void DeviceManagerServiceTwoFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int64_t internalServiceId = fdp.ConsumeIntegral<int64_t>();
    PublishServiceParam publishServiceParam;
    publishServiceParam.serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    publishServiceParam.serviceInfo.serviceType = fdp.ConsumeIntegral<int32_t>();
    publishServiceParam.serviceInfo.serviceName = fdp.ConsumeRandomLengthString();
    publishServiceParam.serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString();
    ServiceRegInfo serviceRegInfo;
    serviceRegInfo.serviceInfo.serviceId = fdp.ConsumeIntegral<int32_t>();
    serviceRegInfo.serviceInfo.serviceType = fdp.ConsumeIntegral<int32_t>();
    serviceRegInfo.serviceInfo.serviceName = fdp.ConsumeRandomLengthString();
    serviceRegInfo.serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString();
    int32_t regServiceId = 0;
    int64_t publishServiceId = 0;
    int64_t fuzzServiceId = 0;
    PublishServiceParam fuzzPublishServiceParam;
    fuzzPublishServiceParam.regServiceId = fdp.ConsumeIntegral<int32_t>();
    fuzzPublishServiceParam.serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    fuzzPublishServiceParam.serviceInfo.serviceType = fdp.ConsumeRandomLengthString();
    fuzzPublishServiceParam.serviceInfo.serviceName = fdp.ConsumeRandomLengthString();
    fuzzPublishServiceParam.serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString();
    int32_t fuzzUserId2 = fdp.ConsumeIntegral<int32_t>();
    ServiceInfoProfile fuzzServiceInfoProfile;
    fuzzServiceInfoProfile.serviceId = fdp.ConsumeIntegral<int64_t>();
    fuzzServiceInfoProfile.userId = fuzzUserId2;
    fuzzServiceInfoProfile.serviceName = fdp.ConsumeRandomLengthString();
    fuzzServiceInfoProfile.serviceType = fdp.ConsumeRandomLengthString();
    int32_t fuzzRegServiceId = 0;
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerService::GetInstance().RegisterServiceInfo(serviceRegInfo, regServiceId);
    DeviceManagerService::GetInstance().UnRegisterServiceInfo(regServiceId);
    DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceParam, publishServiceId);
    DeviceManagerService::GetInstance().StopPublishService(publishServiceId);
    DeviceManagerService::GetInstance().GenerateServiceId(fuzzServiceId);
    DeviceManagerService::GetInstance().ConvertServiceInfoProfileByRegInfo(serviceRegInfo,
        fuzzServiceInfoProfile, fdp.ConsumeIntegral<int64_t>());
    DeviceManagerService::GetInstance().GenerateRegServiceId(fuzzRegServiceId);
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
    OHOS::DistributedHardware::StartServiceDiscoveryFuzzTest(fdp);
    OHOS::DistributedHardware::StopServiceDiscoveryFuzzTest(fdp);
    OHOS::DistributedHardware::DeviceManagerServiceTwoFuzzTest(fdp);

    return 0;
}
