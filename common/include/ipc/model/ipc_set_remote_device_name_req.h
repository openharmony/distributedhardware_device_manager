/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IPC_SET_REMOTE_DEVICE_NAME_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_SET_REMOTE_DEVICE_NAME_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcSetRemoteDeviceNameReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcSetRemoteDeviceNameReq);

public:
    /**
     * @tc.name: IpcSetRemoteDeviceNameReq::GetDeviceId
     * @tc.desc: Ipc Get DeviceId Request Get DeviceId
     * @tc.type: FUNC
     */
    const std::string &GetDeviceId() const
    {
        return deviceId_;
    }

    /**
     * @tc.name: IpcSetRemoteDeviceNameReq::SetDeviceId
     * @tc.desc: Ipc Set DeviceId Request Set DeviceId
     * @tc.type: FUNC
     */
    void SetDeviceId(const std::string &deviceId)
    {
        deviceId_ = deviceId;
    }

    /**
     * @tc.name: IpcSetRemoteDeviceNameReq::GetDeviceName
     * @tc.desc: Ipc Get DeviceName Request Get DeviceName
     * @tc.type: FUNC
     */
    const std::string &GetDeviceName() const
    {
        return deviceName_;
    }

    /**
     * @tc.name: IpcSetRemoteDeviceNameReq::SetDeviceName
     * @tc.desc: Ipc Set DeviceName Request Set DeviceName
     * @tc.type: FUNC
     */
    void SetDeviceName(const std::string &deviceName)
    {
        deviceName_ = deviceName;
    }

private:
    std::string deviceId_;
    std::string deviceName_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_SET_REMOTE_DEVICE_NAME_REQ_H