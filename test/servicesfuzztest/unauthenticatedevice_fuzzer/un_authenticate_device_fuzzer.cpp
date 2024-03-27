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

#include "dm_constants.h"
#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "un_authenticate_device_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void UnAuthenticateDeviceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) || (size > DM_MAX_DEVICE_ID_LEN))) {
        return;
    }

    std::string bundleName(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    DmDeviceInfo deviceInfo;
    if (memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size) != DM_OK) {
        return;
    }
    if (memcpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size) != DM_OK) {
        return;
    }
    if (memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size) != DM_OK) {
        return;
    }
    deviceInfo.deviceTypeId = *(reinterpret_cast<const uint16_t *>(data));
    deviceInfo.range = *(reinterpret_cast<const int32_t *>(data));
    deviceInfo.networkType = *(reinterpret_cast<const int32_t *>(data));
    std::string extraData(reinterpret_cast<const char*>(data), size);
    deviceInfo.extraData = extraData;
    deviceInfo.authForm = *(reinterpret_cast<const DmAuthForm*>(data));

    DeviceManager::GetInstance().UnAuthenticateDevice(bundleName, deviceInfo);
    DeviceManager::GetInstance().UnBindDevice(bundleName, deviceId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::UnAuthenticateDeviceFuzzTest(data, size);
    return 0;
}
