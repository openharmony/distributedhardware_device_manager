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

#ifndef OHOS_DM_IPC_GET_PRODUCT_INFO_REQ_H
#define OHOS_DM_IPC_GET_PRODUCT_INFO_REQ_H

#include "dm_device_profile_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetProductInfoReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcGetProductInfoReq);

public:
    /**
     * @tc.name: IpcBindTargetReq::GetFirstParam
     * @tc.desc: Ipc get first string parameter
     * @tc.type: FUNC
     */
    const DmProductInfoFilterOptions &GetFilterOptions() const
    {
        return filterOptions_;
    }

    /**
     * @tc.name: IpcBindTargetReq::SetFirstParam
     * @tc.desc: Ipc set first string parameter
     * @tc.type: FUNC
     */
    void SetFilterOptions(const DmProductInfoFilterOptions &filterOptions)
    {
        filterOptions_ = filterOptions;
    }

private:
    DmProductInfoFilterOptions filterOptions_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_PRODUCT_INFO_REQ_H
