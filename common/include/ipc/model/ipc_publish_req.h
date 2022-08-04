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

#ifndef OHOS_DM_IPC_PUBLISH_REQ_H
#define OHOS_DM_IPC_PUBLISH_REQ_H

#include "dm_publish_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcPublishReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcPublishReq);

public:
    /**
     * @tc.name: IpcPublishReq::GetPublishInfo
     * @tc.desc: Ipc Publish Discovery Request Get PublishInfo
     * @tc.type: FUNC
     */
    const DmPublishInfo &GetPublishInfo() const
    {
        return publishInfo_;
    }

    /**
     * @tc.name: IpcPublishReq::SetPublishInfo
     * @tc.desc: Ipc Publish Discovery Request Set PublishInfo
     * @tc.type: FUNC
     */
    void SetPublishInfo(const DmPublishInfo &publishInfo)
    {
        publishInfo_ = publishInfo;
    }

private:
    DmPublishInfo publishInfo_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_PUBLISH_REQ_H
