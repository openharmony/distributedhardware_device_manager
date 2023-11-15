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

#ifndef OHOS_DM_IPC_NOTIFY_BIND_RESULT_REQ_H
#define OHOS_DM_IPC_NOTIFY_BIND_RESULT_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyBindResultReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyBindResultReq);

public:
    /**
     * @tc.name: IpcNotifyBindResultReq::GetFirstParam
     * @tc.desc: Ipc get first string parameter
     * @tc.type: FUNC
     */
    const PeerTargetId &GetPeerTargetId() const
    {
        return targetId_;
    }

    /**
     * @tc.name: IpcNotifyBindResultReq::SetFirstParam
     * @tc.desc: Ipc set first string parameter
     * @tc.type: FUNC
     */
    void SetPeerTargetId(const PeerTargetId &targetId)
    {
        targetId_ = targetId;
    }

    /**
     * @tc.name: IpcNotifyBindResultReq::GetResult
     * @tc.desc: Ipc get result
     * @tc.type: FUNC
     */
    int32_t GetResult() const
    {
        return result_;
    }

    /**
     * @tc.name: IpcNotifyBindResultReq::SetResult
     * @tc.desc: Ipc set result
     * @tc.type: FUNC
     */
    void SetResult(int32_t result)
    {
        result_ = result;
    }

    /**
     * @tc.name: IpcNotifyBindResultReq::GetStatus
     * @tc.desc: Ipc get status
     * @tc.type: FUNC
     */
    int32_t GetStatus() const
    {
        return status_;
    }

    /**
     * @tc.name: IpcNotifyBindResultReq::SetStatus
     * @tc.desc: Ipc set status
     * @tc.type: FUNC
     */
    void SetStatus(int32_t status)
    {
        status_ = status;
    }

    /**
     * @tc.name: IpcNotifyBindResultReq::GetSecondParam
     * @tc.desc: Ipc get content
     * @tc.type: FUNC
     */
    const std::string &GetContent() const
    {
        return content_;
    }

    /**
     * @tc.name: IpcNotifyBindResultReq::SetContent
     * @tc.desc: Ipc set content
     * @tc.type: FUNC
     */
    void SetContent(const std::string &content)
    {
        content_ = content;
    }

private:
    PeerTargetId targetId_;
    int32_t result_;
    int32_t status_;
    std::string content_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_BIND_RESULT_REQ_H
