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

#ifndef OHOS_DM_IPC_UNPUBLISH_REQ_H
#define OHOS_DM_IPC_UNPUBLISH_REQ_H

#include <cstdint>
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcUnPublishReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcUnPublishReq);

public:
    /**
     * @tc.name: IpcUnPublishReq::GetPublishId
     * @tc.desc: Ipc UnPublish discovery request get publish id
     * @tc.type: FUNC
     */
    int32_t GetPublishId() const
    {
        return publishId_;
    }

    /**
     * @tc.name: IpcUnPublishReq::SetPublishId
     * @tc.desc: Ipc UnPublish discovery request set publish id
     * @tc.type: FUNC
     */
    void SetPublishId(int32_t publishId)
    {
        publishId_ = publishId;
    }

private:
    int32_t publishId_ { 0 };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_UNPUBLISH_REQ_H
