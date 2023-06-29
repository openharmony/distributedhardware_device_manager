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

#ifndef OHOS_DM_IPC_START_DISCOVER_REQ_H
#define OHOS_DM_IPC_START_DISCOVER_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcStartDevDiscoveryByIdReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcStartDevDiscoveryByIdReq);

public:
    /**
     * @tc.name: IpcStartDiscoveryReq::GetSubscribeInfo
     * @tc.desc: Ipc Start Discovery Request Get SubscribeInfo
     * @tc.type: FUNC
     */
    const uint16_t &GetSubscribeId() const
    {
        return subscribeId_;
    }

    /**
     * @tc.name: IpcStartDiscoveryReq::SetSubscribeInfo
     * @tc.desc: Ipc Start Discovery Request Set SubscribeInfo
     * @tc.type: FUNC
     */
    void SetSubscribeId(const uint16_t &subscribeId)
    {
        subscribeId_ = subscribeId;
    }

    /**
     * @tc.name: IpcStartDiscoveryReq::GetExtra
     * @tc.desc: Ipc Start Discovery Request Get Extra
     * @tc.type: FUNC
     */
    const std::string &GetFilterOption() const
    {
        return filterOptions_;
    }

    /**
     * @tc.name: IpcStartDiscoveryReq::SetExtra
     * @tc.desc: Ipc Start Discovery Request Set Extra
     * @tc.type: FUNC
     */
    void SetFilterOption(const std::string &filterOptions)
    {
        filterOptions_ = filterOptions;
    }

private:
    std::string filterOptions_;
    uint16_t subscribeId_ { 0 };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_START_DISCOVER_REQ_H
