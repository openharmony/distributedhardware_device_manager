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
 
#ifndef OHOS_DM_IPC_NOTIFY_SERVICE_STATE_REQ_H
#define OHOS_DM_IPC_NOTIFY_SERVICE_STATE_REQ_H
 
#include "dm_device_info.h"
#include "ipc_req.h"
 
namespace OHOS {
namespace DistributedHardware {
class IpcNotifyServiceStateReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyServiceStateReq);
 
public:
    /**
     * @tc.name: IpcNotifyServiceStateReq::GetServiceState
     * @tc.desc: Ipc notification Service status request Get Device State
     * @tc.type: FUNC
     */
    int32_t GetServiceState() const
    {
        return serviceState_;
    }
    /**
     * @tc.name: IpcNotifyServiceStateReq::SetServiceState
     * @tc.desc: Ipc notification Service status request Set Service State
     * @tc.type: FUNC
     */
    void SetServiceState(int32_t serviceState)
    {
        serviceState_ = serviceState;
    }
    /**
     * @tc.name: IpcNotifyServiceStateReq::GetDmServiceInfo
     * @tc.desc: Ipc notification Service status request Get Service Info
     * @tc.type: FUNC
     */
    const DmServiceInfo &GetDmServiceInfo() const
    {
        return dmServiceInfo_;
    }
    /**
     * @tc.name: IpcNotifyServiceStateReq::SetDmServiceInfo
     * @tc.desc: Ipc notification Service status request Set Service Info
     * @tc.type: FUNC
     */
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