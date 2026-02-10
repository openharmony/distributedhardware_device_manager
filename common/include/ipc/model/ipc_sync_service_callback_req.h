/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_SYNC_SERVICE_CALLBACK_REQ_H
#define OHOS_DM_IPC_SYNC_SERVICE_CALLBACK_REQ_H

#include "ipc_req.h"
// zl online&offline
namespace OHOS {
namespace DistributedHardware {
class IpcSyncServiceCallbackReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcSyncServiceCallbackReq);

public:
    /**
     * @tc.name: IpcSyncServiceCallbackReq::GetDmCommonNotifyEvent
     * @tc.desc: Get notifyEvent of the Ipc Request
     * @tc.type: FUNC
     */
    int32_t GetDmCommonNotifyEvent() const
    {
        return dmCommonNotifyEvent_;
    }

    /**
     * @tc.name: IpcSyncServiceCallbackReq::SetDmCommonNotifyEvent
     * @tc.desc: Set notifyEvent of the Ipc Request
     * @tc.type: FUNC
     */
    void SetDmCommonNotifyEvent(int32_t dmCommonNotifyEvent)
    {
        dmCommonNotifyEvent_ = dmCommonNotifyEvent;
    }

    /**
     * @tc.name: IpcSyncServiceCallbackReq::GetServiceId
     * @tc.desc: Get serviceId of the Ipc Request
     * @tc.type: FUNC
     */
    int64_t GetServiceId() const
    {
        return serviceId_;
    }

    /**
     * @tc.name: IpcSyncServiceCallbackReq::SetServiceId
     * @tc.desc: Set serviceId of the Ipc Request
     * @tc.type: FUNC
     */
    void SetServiceId(int64_t serviceId)
    {
        serviceId_ = serviceId;
    }
private:
    int32_t dmCommonNotifyEvent_ = 0;
    int64_t serviceId_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SYNC_SERVICE_CALLBACK_REQ_H
