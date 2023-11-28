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

#ifndef OHOS_DEVICE_MANAGER_IPC_DESTROY_PIN_HOLDER_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_DESTROY_PIN_HOLDER_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcDestroyPinHolderReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcDestroyPinHolderReq);

public:
    /**
     * @tc.name: IpcDestroyPinHolderReq::GetPeerTargetId
     * @tc.desc: Ipc Get Peer TargetId
     * @tc.type: FUNC
     */
    const PeerTargetId &GetPeerTargetId() const
    {
        return targetId_;
    }

    /**
     * @tc.name: IpcDestroyPinHolderReq::SetPeerTargetId
     * @tc.desc: Ipc Set Peer TargetId
     * @tc.type: FUNC
     */
    void SetPeerTargetId(const PeerTargetId &targetId)
    {
        targetId_ = targetId;
    }

    /**
     * @tc.name: IpcDestroyPinHolderReq::GetPinType
     * @tc.desc: Ipc Destroy Pin Holder request Get Pin Type
     * @tc.type: FUNC
     */
    int32_t GetPinType() const
    {
        return pinType_;
    }

    /**
     * @tc.name: IpcDestroyPinHolderReq::SetPinType
     * @tc.desc: Ipc Destroy Pin Holder request Set Pin Type
     * @tc.type: FUNC
     */
    void SetPinType(int32_t pinType)
    {
        pinType_ = pinType;
    }

    /**
     * @tc.name: IpcDestroyPinHolderReq::GetPayload
     * @tc.desc: Ipc Destroy Pin Holder request Get Payload
     * @tc.type: FUNC
     */
    std::string GetPayload() const
    {
        return payload_;
    }

    /**
     * @tc.name: IpcDestroyPinHolderReq::SetPayload
     * @tc.desc: Ipc Destroy Pin Holder request Set Payload
     * @tc.type: FUNC
     */
    void SetPayload(const std::string payload)
    {
        payload_ = payload;
    }

private:
    PeerTargetId targetId_;
    int32_t pinType_;
    std::string payload_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_DESTROY_PIN_HOLDER_REQ_H
