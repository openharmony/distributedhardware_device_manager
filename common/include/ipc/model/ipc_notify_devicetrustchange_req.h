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

#ifndef OHOS_DM_IPC_NOTIFY_DEVICETRUSTCHANGE_REQ_H
#define OHOS_DM_IPC_NOTIFY_DEVICETRUSTCHANGE_REQ_H
#include "ipc_req.h"
namespace OHOS {
namespace DistributedHardware {
class IpcNotifyDevTrustChangeReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyDevTrustChangeReq);

public:
    int32_t GetAuthForm() const
    {
        return authForm_;
    }
    void SetAuthForm(int32_t authForm)
    {
        authForm_ = authForm;
    }
    const std::string &GetUdid() const
    {
        return udid_;
    }
    void SetUdid(const std::string &udid)
    {
        udid_ = udid;
    }
    const std::string &GetUuid() const
    {
        return uuid_;
    }
    void SetUuid(const std::string &uuid)
    {
        uuid_ = uuid;
    }
private:
    int32_t authForm_;
    std::string udid_;
    std::string uuid_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_DEVICETRUSTCHANGE_REQ_H