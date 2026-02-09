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
 
#ifndef OHOS_DM_IPC_GET_TRUST_SERVICE_INFO_REQ_H
#define OHOS_DM_IPC_GET_TRUST_SERVICE_INFO_REQ_H
 
#include "ipc_req.h"
 
namespace OHOS {
namespace DistributedHardware {
class IpcGetTrustServiceInfoReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcGetTrustServiceInfoReq);
public:
    std::map<std::string, std::string> GetParamMap() const
    {
        return paramMap_;
    }
    void SetParamMap(const std::map<std::string, std::string> &paramMap)
    {
        paramMap_ = paramMap;
    }
private:
    std::map<std::string, std::string> paramMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_TRUST_SERVICE_INFO_REQ_H
