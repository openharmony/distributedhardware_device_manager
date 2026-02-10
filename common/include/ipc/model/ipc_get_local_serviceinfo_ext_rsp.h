/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_GET_LOCAL_SERVICEINFO_EXT_RSP_H
#define OHOS_DM_IPC_GET_LOCAL_SERVICEINFO_EXT_RSP_H

#include "ipc_rsp.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetLocalServiceInfoExtRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetLocalServiceInfoExtRsp);

public:
    /**
     * @tc.name: IpcGetLocalServiceInfoExtRsp::GetServiceInfo
     * @tc.desc: Get service info
     * @tc.type: FUNC
     */
    const DmRegisterServiceInfo &GetServiceInfo() const
    {
        return serviceInfo_;
    }
 
    /**
     * @tc.name: IpcGetLocalServiceInfoExtRsp::SetServiceInfo
     * @tc.desc: Set service info
     * @tc.type: FUNC
     */
    void SetServiceInfo(const DmRegisterServiceInfo &serviceInfo)
    {
        serviceInfo_ = serviceInfo;
    }

private:
    DmRegisterServiceInfo serviceInfo_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_LOCAL_SERVICEINFO_EXT_RSP_H
