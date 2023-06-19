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

#ifndef OHOS_DM_IPC_NOTIFY_DEVICE_DISCOVERY_REQ_H
#define OHOS_DM_IPC_NOTIFY_DEVICE_DISCOVERY_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyDeviceDiscoveryReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyDeviceDiscoveryReq);

public:
    /**
     * @tc.name: IpcNotifyDeviceFoundReq::GetDeviceInfo
     * @tc.desc: Get DeviceInfo of the Ipc Notify Device Found Request
     * @tc.type: FUNC
     */
    const DmDeviceBasicInfo &GetDeviceBasicInfo() const
    {
        return dmDeviceBasicInfo_;
    }

    /**
     * @tc.name: IpcNotifyDeviceFoundReq::SetDeviceInfo
     * @tc.desc: Set DeviceInfo of the Ipc Notify Device Found Request
     * @tc.type: FUNC
     */
    void SetDeviceBasicInfo(const DmDeviceBasicInfo &dmDeviceBasicInfo)
    {
        dmDeviceBasicInfo_ = dmDeviceBasicInfo;
    }

    /**
     * @tc.name: IpcNotifyDeviceFoundReq::GetSubscribeId
     * @tc.desc: Get SubscribeId of the Ipc Notify Device Found Request
     * @tc.type: FUNC
     */
    uint16_t GetSubscribeId() const
    {
        return subscribeId_;
    }

    /**
     * @tc.name: IpcNotifyDeviceFoundReq::SetSubscribeId
     * @tc.desc: Set SubscribeId of the Ipc Notify Device Found Request
     * @tc.type: FUNC
     */
    void SetSubscribeId(uint16_t subscribeId)
    {
        subscribeId_ = subscribeId;
    }

private:
    uint16_t subscribeId_ { 0 };
    DmDeviceBasicInfo dmDeviceBasicInfo_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_DEVICE_DISCOVERY_REQ_H
