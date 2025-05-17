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

#include "ipc_server_listener.h"

#include "device_manager_ipc_interface_code.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "ipc_server_stub.h"

namespace OHOS {
namespace DistributedHardware {
int32_t IpcServerListener::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    if (rsp == nullptr || req == nullptr) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcServerListener::SendRequest cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    ProcessInfo processInfo = req->GetProcessInfo();
    if (processInfo.pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    sptr<IpcRemoteBroker> listener = IpcServerStub::GetInstance().GetDmListener(processInfo);
    if (listener == nullptr) {
        LOGE("cannot get listener for package:%{public}s.", processInfo.pkgName.c_str());
        return ERR_DM_POINT_NULL;
    }
    return listener->SendCmd(cmdCode, req, rsp);
}

std::vector<ProcessInfo> IpcServerListener::GetAllProcessInfo()
{
    return IpcServerStub::GetInstance().GetAllProcessInfo();
}

std::set<std::string> IpcServerListener::GetSystemSA()
{
    return IpcServerStub::GetInstance().GetSystemSA();
}
} // namespace DistributedHardware
} // namespace OHOS
