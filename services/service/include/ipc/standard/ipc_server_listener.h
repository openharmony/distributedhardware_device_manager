/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_SERVER_LISTENER_H
#define OHOS_DM_IPC_SERVER_LISTENER_H

#include <set>
#include "ipc_req.h"
#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class IpcServerListener {
public:
    IpcServerListener() = default;
    virtual ~IpcServerListener() = default;

public:
    /**
     * @tc.name: IpcServerListener::SendRequest
     * @tc.desc: Send Request of the Ipc Server Listener
     * @tc.type: FUNC
     */
    int32_t SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp);

    /**
     * @tc.name: IpcServerListener::GetAllProcessInfo
     * @tc.desc: Get All PkgName from stub
     * @tc.type: FUNC
     */
    std::vector<ProcessInfo> GetAllProcessInfo();
    std::set<std::string> GetSystemSA();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SERVER_LISTENER_H
