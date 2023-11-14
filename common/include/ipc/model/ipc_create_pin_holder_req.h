/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IPC_CREATE_PIN_HOLDER_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_CREATE_PIN_HOLDER_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcCreatePinHolderReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcCreatePinHolderReq);

public:
    /**
     * @tc.name: IpcCreatePinHolderReq::GetPeerTargetId
     * @tc.desc: Ipc Get Peer TargetId
     * @tc.type: FUNC
     */
    const PeerTargetId &GetPeerTargetId() const
    {
        return targetId_;
    }

    /**
     * @tc.name: IpcCreatePinHolderReq::SetPeerTargetId
     * @tc.desc: Ipc Set Peer TargetId
     * @tc.type: FUNC
     */
    void SetPeerTargetId(const PeerTargetId &targetId)
    {
        targetId_ = targetId;
    }

    /**
     * @tc.name: IpcCreatePinHolderReq::GetPayload
     * @tc.desc: Ipc Create Pin Holder request Get Payload
     * @tc.type: FUNC
     */
    const std::string GetPayload() const
    {
        return payload_;
    }

    /**
     * @tc.name: IpcCreatePinHolderReq::SetPayload
     * @tc.desc: Ipc Create Pin Holder request Set Payload
     * @tc.type: FUNC
     */
    void SetPayload(const std::string &payload)
    {
        payload_ = payload;
    }

    /**
     * @tc.name: IpcCreatePinHolderReq::GetDeviceId
     * @tc.desc: Ipc Create Pin Holder request Get DeviceId
     * @tc.type: FUNC
     */
    const std::string GetDeviceId() const
    {
        return deviceId_;
    }

    /**
     * @tc.name: IpcCreatePinHolderReq::SetDeviceId
     * @tc.desc: Ipc Create Pin Holder request Set DeviceId
     * @tc.type: FUNC
     */
    void SetDeviceId(const std::string &deviceId)
    {
        deviceId_ = deviceId;
    }

    /**
     * @tc.name: IpcCreatePinHolderReq::GetPinType
     * @tc.desc: Ipc Create Pin Holder request Get Pin Type
     * @tc.type: FUNC
     */
    int32_t GetPinType() const
    {
        return pinType_;
    }

    /**
     * @tc.name: IpcCreatePinHolderReq::SetPinType
     * @tc.desc: Ipc Create Pin Holder request Set Pin Type
     * @tc.type: FUNC
     */
    void SetPinType(int32_t pinType)
    {
        pinType_ = pinType;
    }

private:
    PeerTargetId targetId_;
    std::string payload_;
    std::string deviceId_;
    int32_t pinType_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_CREATE_PIN_HOLDER_REQ_H
