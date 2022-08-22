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
#include "device_manager_service_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string extra(reinterpret_cast<const char*>(data), size);
    int32_t authType = *(reinterpret_cast<const int32_t*>(data));
    uint16_t subscribeId = *(reinterpret_cast<const uint16_t*>(data));
    int32_t publishId = *(reinterpret_cast<const int32_t*>(data));
    DmSubscribeInfo subscribeInfo;
    std::vector<DmDeviceInfo> deviceList;
    DmPublishInfo publishInfo;
    DmAuthParam authParam;

    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, pkgName, pkgName);
    ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, pkgName, pkgName);
    ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, pkgName, extra);
    ret = DeviceManagerService::GetInstance().VerifyAuthentication(pkgName);
    ret = DeviceManagerService::GetInstance().GetFaParam(pkgName, authParam);
    ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, extra);
    ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, authType);
    ret = DeviceManagerService::GetInstance().RegisterDevStateCallback(pkgName, extra);
    ret = DeviceManagerService::GetInstance().UnRegisterDevStateCallback(pkgName, extra);
    ret = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
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