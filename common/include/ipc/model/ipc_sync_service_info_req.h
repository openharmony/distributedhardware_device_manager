/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_SYNC_SERVICE_INFO_REQ_H
#define OHOS_DM_IPC_SYNC_SERVICE_INFO_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcSyncServiceInfoReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcSyncServiceInfoReq);

public:
    void SetLocalUserId(int32_t localUserId)
    {
        localUserId_ = localUserId;
    }

    int32_t GetLocalUserId() const
    {
        return localUserId_;
    }

    void SetNetworkId(const std::string &networkId)
    {
        networkId_ = networkId;
    }

    const std::string &GetNetworkId() const
    {
        return networkId_;
    }

    void SetServiceId(int64_t serviceId)
    {
        serviceId_ = serviceId;
    }

    int64_t GetServiceId() const
    {
        return serviceId_;
    }

private:
    int32_t localUserId_ = 0;
    std::string networkId_;
    int64_t serviceId_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SYNC_SERVICE_INFO_REQ_H
