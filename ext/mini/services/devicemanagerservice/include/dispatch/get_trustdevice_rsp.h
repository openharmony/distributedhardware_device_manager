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

#ifndef OHOS_DEVICE_MANAGER_GET_TRUSTDEVICE_RSP_H
#define OHOS_DEVICE_MANAGER_GET_TRUSTDEVICE_RSP_H

#include <vector>

#include "dm_device_info.h"

#include "message_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class GetTrustdeviceRsp : public MessageRsp {
DECLARE_MESSAGE_MODEL(GetTrustdeviceRsp);
public:
    std::vector<DmDeviceInfo> GetDeviceVec() const
    {
        return deviceVec_;
    }

    void SetDeviceVec(std::vector<DmDeviceInfo>& deviceVec)
    {
        deviceVec_ = deviceVec;
    }
private:
    std::vector<DmDeviceInfo> deviceVec_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_GET_TRUSTDEVICE_RSP_H
