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
IMPLEMENT_SINGLE_INSTANCE(IpcCmdRegister);
constexpr int32_t DM_OK = 0;
constexpr int32_t ERR_DM_INPUT_PARA_INVALID = -20006;
constexpr int32_t ERR_DM_UNSUPPORTED_IPC_COMMAND = -20015;
constexpr int32_t ERR_DM_POINT_NULL = -20005;

int32_t IpcCmdRegister::SetRequest(int32_t cmdCode, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    int32_t ret = DM_OK;
    if (pBaseReq == nullptr) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcCmdRegister::SetRequest cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }

    if (setIpcRequestFuncMap_.count(cmdCode) == 0) {
        LOGE("cmdCode:%{public}d not register SetRequestFunc", cmdCode);
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }

    auto setRequestMapIter = setIpcRequestFuncMap_.find(cmdCode);
    if (setRequestMapIter != setIpcRequestFuncMap_.end()) {
        SetIpcRequestFunc ptr = setRequestMapIter->second;
        if (ptr == nullptr) {
            LOGE("IpcCmdRegister::SetRequest setRequestMapIter->second is null");
            return ERR_DM_POINT_NULL;
        }
        ret = (setRequestMapIter->second)(pBaseReq, data);
    }
    return ret;
}

int32_t IpcCmdRegister::ReadResponse(int32_t cmdCode, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcCmdRegister::ReadResponse cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    auto readResponseMapIter = readResponseFuncMap_.find(cmdCode);
    if (readResponseMapIter == readResponseFuncMap_.end()) {
        LOGE("cmdCode:%{public}d not register ReadResponseFunc", cmdCode);
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    if (readResponseMapIter->second == nullptr) {
        return ERR_DM_POINT_NULL;
    }
    return (readResponseMapIter->second)(reply, pBaseRsp);
}

int32_t IpcCmdRegister::OnIpcCmd(int32_t cmdCode, MessageParcel &data, MessageParcel &reply)
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
    if (onIpcCmdMapIter->second ==  nullptr) {
        return ERR_DM_POINT_NULL;
    }
    return (onIpcCmdMapIter->second)(data, reply);
}
} // namespace DistributedHardware
} // namespace OHOS
