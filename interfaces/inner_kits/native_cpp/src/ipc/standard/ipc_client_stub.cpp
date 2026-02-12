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

#include "device_manager_ipc_interface_code.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "ipc_cmd_register.h"
#include "ipc_object_stub.h"   // for IPCObjectStub
#include "message_option.h"    // for MessageOption
#include "message_parcel.h"    // for MessageParcel
namespace OHOS::DistributedHardware { class IpcReq; }
namespace OHOS::DistributedHardware { class IpcRsp; }

namespace OHOS {
namespace DistributedHardware {
int32_t IpcClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
        LOGI("ReadInterfaceToken fail!");
        return ERR_DM_IPC_READ_FAILED;
    }
    if (IpcCmdRegister::GetInstance().OnIpcCmd(static_cast<int32_t>(code), data, reply) == DM_OK) {
        return DM_OK;
    }
    LOGW("unsupported code: %{public}u", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t IpcClientStub::SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcClientStub::SetRequest cmdCode param invalid!");
        return IPCObjectStub::OnRemoteRequest(cmdCode, data, reply, option);
    }
    LOGI("SendCmd cmdCode: %{public}d", cmdCode);

    if (IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data) != DM_OK) {
        LOGE("set request cmd failed");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }

    LOGI("cmdCode = %{public}d, flags = %{public}d.", cmdCode, option.GetFlags());
    if (IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply) == DM_OK) {
        LOGE("on ipc cmd success");
        return DM_OK;
    }
    return IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
}
} // namespace DistributedHardware
} // namespace OHOS
