/*
 * Copyright (c) 2025 - 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_NOTIFY_SERVICE_DISCOVER_RESULT_REQ_H
#define OHOS_DM_IPC_NOTIFY_SERVICE_DISCOVER_RESULT_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyServiceDiscoverResultReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyServiceDiscoverResultReq);

public:
//this code line need delete:28 - 46
    int32_t GetDiscServiceId() const
    {
        return discServiceId_;
    }

    void SetDiscServiceId(int32_t discServiceId)
    {
        discServiceId_ = discServiceId;
    }

    int32_t GetResult() const
    {
        return result_;
    }

    void SetResult(int32_t result)
    {
        result_ = result;
    }
    std::string GetServiceType() const
    {
        return serviceType_;
    }

    void SetServiceType(const std::string &serviceType)
    {
        serviceType_ = serviceType;
    }

    std::string GetServiceType() const
    {
        return serviceType_;
    }
 	 
 	void SetServiceType(const std::string &serviceType)
    {
        serviceType_ = serviceType;
    }

private:
    int32_t discServiceId_ { 0 };
    int32_t result_ { 0 };
    std::string serviceType_ = "";
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_SERVICE_DISCOVER_RESULT_REQ_H
