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

#ifndef OHOS_DM_IPC_GET_TRUST_SERVICE_INFO_RSP_H
#define OHOS_DM_IPC_GET_TRUST_SERVICE_INFO_RSP_H

#include "ipc_req.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetTrustServiceInfoRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetTrustServiceInfoRsp);
public:
    std::vector<DmServiceInfo> GetServiceInfoVec() const
    {
        return serviceInfoVec_;
    }
    void SetServiceInfoVec(std::vector<DmServiceInfo> &serviceInfoVec)
    {
        serviceInfoVec_ = serviceInfoVec;
    }
private:
    std::vector<DmServiceInfo> serviceInfoVec_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_TRUST_SERVICE_INFO_RSP_H
