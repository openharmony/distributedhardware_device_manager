/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_GET_AVAILABLE_DEVICE_RSP_H
#define OHOS_DM_IPC_GET_AVAILABLE_DEVICE_RSP_H

#include <vector>

#include "dm_device_info.h"
#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetAvailableDeviceRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetAvailableDeviceRsp);

public:
    /**
     * @tc.name: IpcGetAvailableDeviceRsp::GetDeviceVec
     * @tc.desc: Get DeviceVec of the Ipc Get Available Device Response
     * @tc.type: FUNC
     */
    std::vector<DmDeviceBasicInfo> GetDeviceVec() const
    {
        return deviceVec_;
    }

    /**
     * @tc.name: IpcGetAvailableDeviceRsp::SetDeviceVec
     * @tc.desc: Set DeviceVec of the Ipc Get Available Device Response
     * @tc.type: FUNC
     */
    void SetDeviceVec(std::vector<DmDeviceBasicInfo> &deviceVec)
    {
        deviceVec_ = deviceVec;
    }

private:
    std::vector<DmDeviceBasicInfo> deviceVec_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_AVAILABLE_DEVICE_RSP_H
