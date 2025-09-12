/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_NOTIFY_DEVICE_STATE_REQ_H
#define OHOS_DM_IPC_NOTIFY_DEVICE_STATE_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyDeviceStateReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyDeviceStateReq);

public:
    /**
     * @tc.name: IpcNotifyDeviceStateReq::GetDeviceState
     * @tc.desc: Ipc notification device status request Get Device State
     * @tc.type: FUNC
     */
    int32_t GetDeviceState() const
    {
        return deviceState_;
    }
    /**
     * @tc.name: IpcNotifyDeviceStateReq::SetDeviceState
     * @tc.desc: Ipc notification device status request Set Device State
     * @tc.type: FUNC
     */
    void SetDeviceState(int32_t deviceState)
    {
        deviceState_ = deviceState;
    }
    /**
     * @tc.name: IpcNotifyDeviceStateReq::GetDeviceInfo
     * @tc.desc: Ipc notification device status request Get Device Info
     * @tc.type: FUNC
     */
    const DmDeviceInfo &GetDeviceInfo() const
    {
        return dmDeviceInfo_;
    }
    /**
     * @tc.name: IpcNotifyDeviceStateReq::SetDeviceInfo
     * @tc.desc: Ipc notification device status request Set Device Info
     * @tc.type: FUNC
     */
    void SetDeviceInfo(const DmDeviceInfo &dmDeviceInfo)
    {
        dmDeviceInfo_ = dmDeviceInfo;
    }
    /**
     * @tc.name: IpcNotifyDeviceStateReq::GetDeviceBasicInfo
     * @tc.desc: Ipc notification device status request Get Device Basic Info
     * @tc.type: FUNC
     */
    const DmDeviceBasicInfo &GetDeviceBasicInfo() const
    {
        return dmDeviceBasicInfo_;
    }
    /**
     * @tc.name: IpcNotifyDeviceStateReq::SetDeviceBasicInfo
     * @tc.desc: Ipc notification device status request Set Device Basic Info
     * @tc.type: FUNC
     */
    void SetDeviceBasicInfo(const DmDeviceBasicInfo &dmDeviceBasicInfo)
    {
        dmDeviceBasicInfo_ = dmDeviceBasicInfo;
    }

    void SetServiceIds(const std::vector<int64_t> &serviceIds)
    {
        serviceIds_ = serviceIds;
    }

    const std::vector<int64_t> &GetServiceIds() const
    {
        return serviceIds_;
    }

private:
    int32_t deviceState_ { 0 };
    DmDeviceInfo dmDeviceInfo_;
    DmDeviceBasicInfo dmDeviceBasicInfo_;
    std::vector<int64_t> serviceIds_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_DEVICE_STATE_REQ_H
