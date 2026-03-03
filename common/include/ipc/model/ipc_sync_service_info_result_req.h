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

#ifndef OHOS_DM_IPC_SYNC_SERVICE_INFO_RESULT_REQ_H
#define OHOS_DM_IPC_SYNC_SERVICE_INFO_RESULT_REQ_H

#include "ipc_req.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class IpcSyncServiceInfoResultReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcSyncServiceInfoResultReq);
public:
    void SetServiceSyncInfo(const ServiceSyncInfo &serviceSyncInfo)
    {
        serviceSyncInfo_ = serviceSyncInfo;
    }

    const ServiceSyncInfo &GetServiceSyncInfo() const
    {
        return serviceSyncInfo_;
    }

    void SetResult(int32_t result)
    {
        result_ = result;
    }

    int32_t GetResult()
    {
        return result_;
    }

    void SetContent(const std::string &content)
    {
        content_ = content;
    }

    const std::string &GetContent() const
    {
        return content_;
    }

    void SetServiceId(int64_t serviceId)
    {
        serviceId_ = serviceId;
    }

    int64_t GetServiceId()
    {
        return serviceId_;
    }

private:
    ServiceSyncInfo serviceSyncInfo_;
    int32_t result_ = 0;
    std::string content_;
    int64_t serviceId_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SYNC_SERVICE_INFO_RESULT_REQ_H
