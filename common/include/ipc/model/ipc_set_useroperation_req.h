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

#ifndef OHOS_DEVICE_MANAGER_IPC_GET_USER_OPERATION_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_GET_USER_OPERATION_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetOperationReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcGetOperationReq);

public:
    /**
     * @tc.name: IpcGetOperationReq::GetOperation
     * @tc.desc: Ipc Get Operation Request Get Operation
     * @tc.type: FUNC
     */
    int32_t GetOperation() const
    {
        return action_;
    }

    /**
     * @tc.name: IpcGetOperationReq::SetOperation
     * @tc.desc: Ipc Get Operation Request Set Operation
     * @tc.type: FUNC
     */
    void SetOperation(int32_t action)
    {
        action_ = action;
    }

    /**
     * @tc.name: IpcGetOperationReq::GetParams
     * @tc.desc: Ipc Get Params Request Get Params
     * @tc.type: FUNC
     */
    const std::string &GetParams() const
    {
        return params_;
    }

    /**
     * @tc.name: IpcGetOperationReq::SetParams
     * @tc.desc: Ipc Set Params Request Set Params
     * @tc.type: FUNC
     */
    void SetParams(const std::string &params)
    {
        params_ = params;
    }

private:
    int32_t action_ { 0 };
    std::string params_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_GET_USER_OPERATION_REQ_H
