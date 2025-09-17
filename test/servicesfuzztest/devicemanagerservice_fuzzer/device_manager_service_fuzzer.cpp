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
void DeviceManagerServiceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
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
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceManagerServiceFuzzTest(data, size);

    return 0;
}
