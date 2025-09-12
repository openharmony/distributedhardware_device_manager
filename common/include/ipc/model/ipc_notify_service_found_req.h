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

#ifndef OHOS_DM_IPC_NOTIFY_SERVICE_FOUND_REQ_H
#define OHOS_DM_IPC_NOTIFY_SERVICE_FOUND_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyServiceFoundReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyServiceFoundReq);

public:
    int32_t GetDiscServiceId() const
    {
        return discServiceId_;
    }

    void SetDiscServiceId(int32_t discServiceId)
    {
        discServiceId_ = discServiceId;
    }

    const DiscoveryServiceInfo &GetDiscServiceInfo() const
    {
        return discServiceInfo_;
    }

    void SetDiscServiceInfo(const DiscoveryServiceInfo &discServiceInfo)
    {
        discServiceInfo_ = discServiceInfo;
    }

private:
    int32_t discServiceId_ { 0 };
    DiscoveryServiceInfo discServiceInfo_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_SERVICE_FOUND_REQ_H
