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

#include "ipc_client_stub.h"

#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(IpcClientStub);

static int32_t ClientIpcInterfaceMsgHandle(uint32_t code, IpcIo *data, IpcIo *reply, MessageOption option)
{
    if (data== nullptr) {
        LOGE("invalid param");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    int32_t errCode = IpcCmdRegister::GetInstance().OnIpcCmd(code, *data);
    LOGI("receive ipc transact code:%{public}u, retCode = %{public}d", code, errCode);
    return errCode;
}

int32_t IpcClientStub::Init()
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (bInit) {
        return DM_OK;
    }

    objectStub_.func = ClientIpcInterfaceMsgHandle;
    objectStub_.args = nullptr;
    objectStub_.isRemote = false;
    clientIdentity_.handle = IPC_INVALID_HANDLE;
    clientIdentity_.token = 0;
    clientIdentity_.cookie = (uintptr_t)&objectStub_;

    bInit = true;
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
