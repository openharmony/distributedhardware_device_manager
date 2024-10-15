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

#ifndef OHOS_DM_IPC_NOTIFY_CANDIDATERESTRICT_STATUS_REQ_H
#define OHOS_DM_IPC_NOTIFY_CANDIDATERESTRICT_STATUS_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyHichainProofExceptionStatusReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyHichainProofExceptionStatusReq);

public:
    /**
     * @tc.name: IpcNotifyHichainProofExceptionStatusReq::GetPublishId
     * @tc.desc: Get deviceId of the Ipc Notify Candidate Restrict Status Request
     * @tc.type: FUNC
     */
    std::string GetDeviceId() const
    {
        return deviceId_;
    }

    /**
     * @tc.name: IpcNotifyHichainProofExceptionStatusReq::SetPublishId
     * @tc.desc: Set deviceId of the Ipc Notify Candidate Restrict Status Request
     * @tc.type: FUNC
     */
    void SetDeviceId(const std::string &deviceId)
    {
        deviceId_ = deviceId;
    }

    /**
     * @tc.name: IpcNotifyHichainProofExceptionStatusReq::GetResult
     * @tc.desc: Get DeviceType of the Ipc Notify Candidate Restrict Status Request
     * @tc.type: FUNC
     */
    uint16_t GetDeviceTypeId() const
    {
        return deviceTypeId_;
    }

    /**
     * @tc.name: IpcNotifyHichainProofExceptionStatusReq::SetResult
     * @tc.desc: Set DeviceType of the Ipc Notify Candidate Restrict Status Request
     * @tc.type: FUNC
     */
    void SetDeviceTypeId(uint16_t deviceTypeId)
    {
        deviceTypeId_ = deviceTypeId;
    }

    /**
     * @tc.name: IpcNotifyHichainProofExceptionStatusReq::GetErrCode
     * @tc.desc: Get ErrCode of the Ipc Notify Candidate Restrict Status Request
     * @tc.type: FUNC
     */
    int32_t GetErrCode() const
    {
        return errcode_;
    }

    /**
     * @tc.name: IpcNotifyHichainProofExceptionStatusReq::SetErrCode
     * @tc.desc: Set ErrCode of the Ipc Notify Candidate Restrict Status Request
     * @tc.type: FUNC
     */
    void SetErrCode(int32_t errcode)
    {
        errcode_ = errcode;
    }

private:
    std::string deviceId_;
    uint16_t deviceTypeId_ = 0;
    int32_t errcode_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_CANDIDATERESTRICT_STATUS_REQ_H
