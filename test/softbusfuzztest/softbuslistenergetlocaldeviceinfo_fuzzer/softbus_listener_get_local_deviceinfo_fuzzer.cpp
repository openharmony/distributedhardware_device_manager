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

#include <chrono>
#include <securec.h>
#include <string>

#include "device_manager_impl.h"
#include "dm_constants.h"
#include "softbus_listener.h"
#include "softbus_listener_get_local_deviceinfo_fuzzer.h"
#include "softbus_listener.cpp"

namespace OHOS {
namespace DistributedHardware {
void GetLocalDeviceInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::string str(reinterpret_cast<const char*>(data), size);
    int32_t subscribeId = *(reinterpret_cast<const int32_t*>(data));
    int32_t result = *(reinterpret_cast<const int32_t*>(data));
    int sessionId = *(reinterpret_cast<const int*>(data));
    int publishId = *(reinterpret_cast<const int*>(data));
    PublishResult pResult = (PublishResult)1;
    RefreshResult rResult = (RefreshResult)1;
    DmDeviceInfo deviceInfo;
    NodeBasicInfoType type = NodeBasicInfoType::TYPE_DEVICE_NAME;

    DeviceInfo device = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    NodeBasicInfo info = {
        .networkId = "123456",
        .deviceName = "123456",
        .deviceTypeId = 1
    };

    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();

    OnSessionOpened(sessionId, result);
    OnBytesReceived(sessionId, data, size);
    OnSessionClosed(sessionId);
    OnPinHolderSessionOpened(sessionId, result);
    OnPinHolderBytesReceived(sessionId, data, size);
    OnPinHolderSessionClosed(sessionId);
    
    softbusListener->GetLocalDeviceInfo(deviceInfo);
    softbusListener->OnSoftbusPublishResult(publishId, pResult);
    softbusListener->OnSoftbusDeviceOnline(&info);
    softbusListener->OnSoftbusDeviceOffline(&info);
    softbusListener->OnSoftbusDeviceInfoChanged(type, &info);
    softbusListener->OnSoftbusDeviceFound(&device);
    softbusListener->OnSoftbusDiscoveryResult(subscribeId, rResult);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::GetLocalDeviceInfoFuzzTest(data, size);

    return 0;
}