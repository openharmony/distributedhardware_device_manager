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

#ifndef OHOS_DM_IPC_NOTIFY_CREDENTIAL_AUTH_STATUS_REQ_H
#define OHOS_DM_IPC_NOTIFY_CREDENTIAL_AUTH_STATUS_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyCredentialAuthStatusReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyCredentialAuthStatusReq);

public:
    /**
     * @tc.name: IpcNotifyCredentialAuthStatusReq::GetProofInfo
     * @tc.desc: Get proofInfo of the Ipc Notify Credential Auth Status Request
     * @tc.type: FUNC
     */
    std::string GetProofInfo() const
    {
        return proofInfo_;
    }

    /**
     * @tc.name: IpcNotifyCredentialAuthStatusReq::SetProofInfo
     * @tc.desc: Set proofInfo of the Ipc Notify Credential Auth Status Request
     * @tc.type: FUNC
     */
    void SetProofInfo(const std::string &proofInfo)
    {
        proofInfo_ = proofInfo;
    }

    /**
     * @tc.name: IpcNotifyCredentialAuthStatusReq::GetDeviceTypeId
     * @tc.desc: Get DeviceType of the Ipc Notify Credential Auth Status Request
     * @tc.type: FUNC
     */
    uint16_t GetDeviceTypeId() const
    {
        return deviceTypeId_;
    }

    /**
     * @tc.name: IpcNotifyCredentialAuthStatusReq::SetDeviceTypeId
     * @tc.desc: Set DeviceType of the Ipc Notify Credential Auth Status Request
     * @tc.type: FUNC
     */
    void SetDeviceTypeId(uint16_t deviceTypeId)
    {
        deviceTypeId_ = deviceTypeId;
    }

    /**
     * @tc.name: IpcNotifyCredentialAuthStatusReq::GetErrCode
     * @tc.desc: Get ErrCode of the Ipc Notify Credential Auth Status Request
     * @tc.type: FUNC
     */
    int32_t GetErrCode() const
    {
        return errcode_;
    }

    /**
     * @tc.name: IpcNotifyCredentialAuthStatusReq::SetErrCode
     * @tc.desc: Set ErrCode of the Ipc Notify Credential Auth Status Request
     * @tc.type: FUNC
     */
    void SetErrCode(int32_t errcode)
    {
        errcode_ = errcode;
    }

private:
    std::string proofInfo_;
    uint16_t deviceTypeId_ = 0;
    int32_t errcode_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_CREDENTIAL_AUTH_STATUS_REQ_H
