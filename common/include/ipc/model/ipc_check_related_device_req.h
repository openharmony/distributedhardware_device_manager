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

#ifndef OHOS_DM_IPC_CHECK_RELATED_DEVICE_REQ_H
#define OHOS_DM_IPC_CHECK_RELATED_DEVICE_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcCheckRelatedDeviceReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcCheckRelatedDeviceReq);

public:
    const std::string GetUdid() const
    {
        return udid_;
    }

    const std::string GetBundleName() const
    {
        return bundleName_;
    }

    void SetUdid(const std::string &udid)
    {
        udid_ = udid;
    }

    void SetBundleName(const std::string &bundleName)
    {
        bundleName_ = bundleName;
    }

private:
    std::string udid_;
    std::string bundleName_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_CHECK_RELATED_DEVICE_REQ_H
