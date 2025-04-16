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

#ifndef OHOS_DM_IPC_NOTIFY_SET_REMOTE_DEVICE_NAME_REQ_H
#define OHOS_DM_IPC_NOTIFY_SET_REMOTE_DEVICE_NAME_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifySetRemoteDeviceNameReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifySetRemoteDeviceNameReq);

public:
    /**
     * @tc.desc: Ipc get result
     * @tc.type: FUNC
     */
    int32_t GetResult() const
    {
        return result_;
    }

    /**
     * @tc.desc: Ipc set result
     * @tc.type: FUNC
     */
    void SetResult(int32_t result)
    {
        result_ = result;
    }
    /**
     * @tc.desc: Ipc get deviceId
     * @tc.type: FUNC
     */
    const std::string &GetDeviceId() const
    {
        return deviceId_;
    }

    /**
     * @tc.desc: Ipc set deviceId
     * @tc.type: FUNC
     */
    void SetDeviceId(const std::string &deviceId)
    {
        deviceId_ = deviceId;
    }

private:
    int32_t result_ = 0;
    std::string deviceId_ = "";
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_SET_REMOTE_DEVICE_NAME_REQ_H
