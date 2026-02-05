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

#ifndef OHOS_DM_IPC_BIND_TARGET_REQ_H
#define OHOS_DM_IPC_BIND_TARGET_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcBindTargetReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcBindTargetReq);

public:
    /**
     * @tc.name: IpcBindTargetReq::GetFirstParam
     * @tc.desc: Ipc get first string parameter
     * @tc.type: FUNC
     */
    const PeerTargetId &GetPeerTargetId() const
    {
        return targetId_;
    }

    /**
     * @tc.name: IpcBindTargetReq::SetFirstParam
     * @tc.desc: Ipc set first string parameter
     * @tc.type: FUNC
     */
    void SetPeerTargetId(const PeerTargetId &targetId)
    {
        targetId_ = targetId;
    }

    /**
     * @tc.name: IpcBindTargetReq::GetBindParam
     * @tc.desc: Ipc get bind parameter
     * @tc.type: FUNC
     */
    const std::string &GetBindParam() const
    {
        return bindParam_;
    }

    /**
     * @tc.name: IpcBindTargetReq::SetBindParam
     * @tc.desc: Ipc set bind parameter
     * @tc.type: FUNC
     */
    void SetBindParam(const std::string &bindParam)
    {
        bindParam_ = bindParam;
    }

    void SetNetWorkId(const std::string &netWorkId)
 	{
 	    netWorkId_ = netWorkId;
 	}

 	const std::string &GetNetWorkId() const
 	{
 	    return netWorkId_;
 	}

 	void SetServiceId(int64_t serviceId)
 	{
 	    serviceId_ = serviceId;
 	}

 	const int64_t &GetServiceId() const
 	{
 	    return serviceId_;
 	}
private:
    PeerTargetId targetId_;
    std::string bindParam_;
    std::string netWorkId_;
 	int64_t serviceId_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_BIND_TARGET_REQ_H
