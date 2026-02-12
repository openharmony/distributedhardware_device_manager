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

#include <utility>         // for pair

#include "device_manager_ipc_interface_code.h"
#include "dm_log.h"
#include "ipc_def.h"
namespace OHOS { class MessageParcel; }

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(IpcCmdRegister);
constexpr int32_t ERR_DM_INPUT_PARA_INVALID = 96929749;
constexpr int32_t ERR_DM_UNSUPPORTED_IPC_COMMAND = 96929757;
constexpr int32_t ERR_DM_POINT_NULL = 96929748;
int32_t IpcCmdRegister::SetRequest(int32_t cmdCode, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcCmdRegister::SetRequest cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    SetIpcRequestFunc ptr = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(setIpcRequestFuncMapLock_);
        auto setRequestMapIter = setIpcRequestFuncMap_.find(cmdCode);
        if (setRequestMapIter == setIpcRequestFuncMap_.end()) {
            LOGE("cmdCode:%{public}d not register SetRequestFunc", cmdCode);
            return ERR_DM_UNSUPPORTED_IPC_COMMAND;
        }
        ptr = setRequestMapIter->second;
        if (ptr == nullptr) {
            LOGE("IpcCmdRegister::SetRequest setRequestMapIter->second is null");
            return ERR_DM_POINT_NULL;
        }
    }
    return (ptr)(pBaseReq, data);
}

int32_t IpcCmdRegister::ReadResponse(int32_t cmdCode, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcCmdRegister::ReadResponse cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    ReadResponseFunc ptr = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(readResponseFuncMapLock_);
        auto readResponseMapIter = readResponseFuncMap_.find(cmdCode);
        if (readResponseMapIter == readResponseFuncMap_.end()) {
            LOGE("cmdCode:%{public}d not register ReadResponseFunc", cmdCode);
            return ERR_DM_UNSUPPORTED_IPC_COMMAND;
        }
        ptr = readResponseMapIter->second;
        if (ptr == nullptr) {
            LOGE("IpcCmdRegister::ReadResponse readResponseMapIter->second is null");
            return ERR_DM_POINT_NULL;
        }
    }
    return (ptr)(reply, pBaseRsp);
}

int32_t IpcCmdRegister::OnIpcCmd(int32_t cmdCode, MessageParcel &data, MessageParcel &reply)
{
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcCmdRegister::OnIpcCmd cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    OnIpcCmdFunc ptr = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(onIpcCmdFuncMapLock_);
        auto onIpcCmdMapIter = onIpcCmdFuncMap_.find(cmdCode);
        if (onIpcCmdMapIter == onIpcCmdFuncMap_.end()) {
            LOGE("cmdCode:%{public}d not register OnIpcCmdFunc", cmdCode);
            return ERR_DM_UNSUPPORTED_IPC_COMMAND;
        }
        ptr = onIpcCmdMapIter->second;
        if (ptr == nullptr) {
            LOGE("IpcCmdRegister::OnIpcCmd onIpcCmdMapIter->second is null");
            return ERR_DM_POINT_NULL;
        }
    }
    return (ptr)(data, reply);
}
} // namespace DistributedHardware
} // namespace OHOS
