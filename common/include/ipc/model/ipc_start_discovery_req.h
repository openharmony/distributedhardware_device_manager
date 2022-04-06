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

#ifndef OHOS_DM_IPC_START_DISCOVERY_REQ_H
#define OHOS_DM_IPC_START_DISCOVERY_REQ_H

#include "dm_subscribe_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcStartDiscoveryReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcStartDiscoveryReq);

public:
    /**
     * @tc.name: IpcStartDiscoveryReq::GetSubscribeInfo
     * @tc.desc: Ipc Start Discovery Request Get SubscribeInfo
     * @tc.type: FUNC
     */
    const DmSubscribeInfo &GetSubscribeInfo() const
    {
        return subscribeInfo_;
    }

    /**
     * @tc.name: IpcStartDiscoveryReq::SetSubscribeInfo
     * @tc.desc: Ipc Start Discovery Request Set SubscribeInfo
     * @tc.type: FUNC
     */
    void SetSubscribeInfo(const DmSubscribeInfo &subscribeInfo)
    {
        subscribeInfo_ = subscribeInfo;
    }

    /**
     * @tc.name: IpcStartDiscoveryReq::GetExtra
     * @tc.desc: Ipc Start Discovery Request Get Extra
     * @tc.type: FUNC
     */
    const std::string &GetExtra() const
    {
        return extra_;
    }

    /**
     * @tc.name: IpcStartDiscoveryReq::SetExtra
     * @tc.desc: Ipc Start Discovery Request Set Extra
     * @tc.type: FUNC
     */
    void SetExtra(const std::string &extra)
    {
        extra_ = extra;
    }

private:
    std::string extra_;
    DmSubscribeInfo subscribeInfo_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_START_DISCOVERY_REQ_H
