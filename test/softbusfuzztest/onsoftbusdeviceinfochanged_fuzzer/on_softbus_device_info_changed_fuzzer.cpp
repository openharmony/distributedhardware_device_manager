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
#include "on_softbus_device_info_changed_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void OnSoftbusDeviceInfoChangedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint16_t) || (size > DM_MAX_DEVICE_ID_LEN))) {
        return;
    }

    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();

    int infoType = 1234;
    NodeBasicInfoType type = static_cast<NodeBasicInfoType>(infoType);
    NodeBasicInfo info;
    if (memcpy_s(info.networkId, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size) != DM_OK) {
        return;
    }
    if (memcpy_s(info.deviceName, DM_MAX_DEVICE_NAME_LEN, (reinterpret_cast<const char *>(data)), size) != DM_OK) {
        return;
    }
    info.deviceTypeId = *(reinterpret_cast<const uint16_t *>(data));
    softbusListener->OnSoftbusDeviceInfoChanged(type, &info);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::OnSoftbusDeviceInfoChangedFuzzTest(data, size);

    return 0;
}
