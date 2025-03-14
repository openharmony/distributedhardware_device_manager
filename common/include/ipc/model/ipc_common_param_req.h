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

#ifndef OHOS_DM_IPC_COMMON_PARAM_REQ_H
#define OHOS_DM_IPC_COMMON_PARAM_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcCommonParamReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcCommonParamReq);

public:
    /**
     * @tc.name: IpcCommonParamReq::GetFirstParam
     * @tc.desc: Ipc get first string parameter
     * @tc.type: FUNC
     */
    const std::string &GetFirstParam() const
    {
        return firstParam_;
    }

    /**
     * @tc.name: IpcCommonParamReq::SetFirstParam
     * @tc.desc: Ipc set first string parameter
     * @tc.type: FUNC
     */
    void SetFirstParam(const std::string &firstParam)
    {
        firstParam_ = firstParam;
    }

    /**
     * @tc.name: IpcCommonParamReq::GetSecondParam
     * @tc.desc: Ipc get second string parameter
     * @tc.type: FUNC
     */
    const std::string &GetSecondParam() const
    {
        return secondParam_;
    }

    /**
     * @tc.name: IpcCommonParamReq::SetSecondParam
     * @tc.desc: Ipc set second string parameter
     * @tc.type: FUNC
     */
    void SetSecondParam(const std::string &secondParam)
    {
        secondParam_ = secondParam;
    }

    int64_t GetInt32Param() const
    {
        return int32Param_;
    }

    void SetInt32Param(int64_t param)
    {
        int32Param_ = param;
    }
private:
    std::string firstParam_;
    std::string secondParam_;
    int64_t int32Param_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_COMMON_PARAM_REQ_H
