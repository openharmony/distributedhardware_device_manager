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

#ifndef OHOS_DM_IPC_GET_DEVICE_NETWORK_ID_LIST_RSP_H
#define OHOS_DM_IPC_GET_DEVICE_NETWORK_ID_LIST_RSP_H

#include "dm_device_profile_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetDeviceNetworkIdListRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetDeviceNetworkIdListRsp);

public:

    const std::vector<std::string> &GetNetworkIds() const
    {
        return networkIds_;
    }

    void SetNetworkIds(const std::vector<std::string> &networkIds)
    {
        networkIds_ = networkIds;
    }
private:
    std::vector<std::string> networkIds_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_DEVICE_NETWORK_ID_LIST_RSP_H
