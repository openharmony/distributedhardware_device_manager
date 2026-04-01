/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DM_SERVICE_LISTENER_3RD_H
#define OHOS_DM_SERVICE_LISTENER_3RD_H

#include "idevice_manager_service_listener_3rd.h"
#include  "device_manager_data_struct_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceListener3rd : public IDeviceManagerServiceListener3rd {
public:
    DeviceManagerServiceListener3rd() {};
    virtual ~DeviceManagerServiceListener3rd() {};

    void OnAuthResult(const ProcessInfo3rd &processInfo3rd, int32_t result, int32_t status,
        const std::string &authContent) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_LISTENER_H
