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

#ifndef OHOS_DEVICE_MANAGER_IPC_GET_INFO_BY_NETWORK_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_GET_INFO_BY_NETWORK_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetInfoByNetWorkReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcGetInfoByNetWorkReq);

public:
    /**
     * @tc.name: IpcGetInfoByNetWorkReq::GetNetWorkId
     * @tc.desc: Ipc Get Info By NetWork request Get NetWorkId
     * @tc.type: FUNC
     */
    const std::string GetNetWorkId() const
    {
        return netWorkId_;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkReq::SetNetWorkId
     * @tc.desc: Ipc Get Info By NetWork request Set NetWorkId
     * @tc.type: FUNC
     */
    void SetNetWorkId(const std::string &netWorkId)
    {
        netWorkId_ = netWorkId;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkReq::GetUdid
     * @tc.desc: Ipc Get Info By NetWork request Get udid
     * @tc.type: FUNC
     */
    const std::string GetUdid() const
    {
        return udid_;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkReq::SetUdid
     * @tc.desc: Ipc Get Info By NetWork request Set udid
     * @tc.type: FUNC
     */
    void SetUdid(const std::string &udid)
    {
        udid_ = udid;
    }

private:
    std::string netWorkId_;
    std::string udid_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_GET_INFO_BY_NETWORK_REQ_H
