/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "ipc_cmd_register.h"

#include "device_manager_ipc_interface_code.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(IpcCmdRegister);

int32_t IpcCmdRegister::SetRequest(int32_t cmdCode, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                                   size_t buffLen)
{
    if (pBaseReq == nullptr) {
        LOGE("IpcCmdRegister::SetRequest pBaseReq is nullptr!");
        return ERR_DM_POINT_NULL;
    }
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcCmdRegister::SetRequest cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    auto setRequestMapIter = setIpcRequestFuncMap_.find(cmdCode);
    if (setRequestMapIter == setIpcRequestFuncMap_.end()) {
        LOGE("cmdCode:%{public}d not register SetRequestFunc", cmdCode);
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    return (setRequestMapIter->second)(pBaseReq, request, buffer, buffLen);
}

int32_t IpcCmdRegister::ReadResponse(int32_t cmdCode, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    auto readResponseMapIter = readResponseFuncMap_.find(cmdCode);
    if (readResponseMapIter == readResponseFuncMap_.end()) {
        LOGE("cmdCode:%{public}d not register ReadResponseFunc", cmdCode);
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    return (readResponseMapIter->second)(reply, pBaseRsp);
}

int32_t IpcCmdRegister::OnIpcCmd(int32_t cmdCode, IpcIo &reply)
{
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcCmdRegister::OnIpcCmd cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    auto onIpcCmdMapIter = onIpcCmdFuncMap_.find(cmdCode);
    if (onIpcCmdMapIter == onIpcCmdFuncMap_.end()) {
        LOGE("cmdCode:%{public}d not register OnIpcCmdFunc", cmdCode);
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    (onIpcCmdMapIter->second)(reply);
    return DM_OK;
}

int32_t IpcCmdRegister::OnIpcServerCmd(int32_t cmdCode, IpcIo &req, IpcIo &reply)
{
    auto onIpcServerCmdMapIter = onIpcServerCmdFuncMap_.find(cmdCode);
    if (onIpcServerCmdMapIter == onIpcServerCmdFuncMap_.end()) {
        LOGE("cmdCode:%{public}d not register OnIpcCmdFunc", cmdCode);
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    (onIpcServerCmdMapIter->second)(req, reply);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
