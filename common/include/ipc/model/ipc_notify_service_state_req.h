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

#ifndef OHOS_DM_IPC_NOTIFY_SERVICE_STATE_REQ_H
#define OHOS_DM_IPC_NOTIFY_SERVICE_STATE_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyServiceStateReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyServiceStateReq);

public:
    int32_t GetServiceState() const
    {
        return serviceState_;
    }

    void SetServiceState(int32_t serviceState)
    {
        serviceState_ = serviceState;
    }

    const DmServiceInfo &GetDmServiceInfo() const
    {
        return dmServiceInfo_;
    }

    void SetDmServiceInfo(const DmServiceInfo &dmServiceInfo)
    {
        dmServiceInfo_ = dmServiceInfo;
    }

    DmRegisterServiceState GetDmRegisterServiceState()
    {
        return registerServiceState_;
    }

    void SetDmRegisterServiceState(const DmRegisterServiceState &registerServiceState)
    {
        registerServiceState_ = registerServiceState;
    }

private:
    int32_t serviceState_ { 0 };
    DmServiceInfo dmServiceInfo_;
    DmRegisterServiceState registerServiceState_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_SERVICE_STATE_REQ_H
