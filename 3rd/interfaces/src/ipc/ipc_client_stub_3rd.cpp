/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ipc_client_stub_3rd.h"

#include "device_manager_notify_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"
#include "ipc_interface_code_3rd.h"
#include "ipc_object_stub.h"   // for IPCObjectStub
#include "message_option.h"    // for MessageOption
#include "message_parcel.h"    // for MessageParcel

namespace OHOS {
namespace DistributedHardware {
int32_t IpcClientStub3rd::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
        LOGI("ReadInterfaceToken fail!");
        return ERR_DM_IPC_READ_FAILED;
    }
    switch (code) {
        case ON_AUTH_RESULT_3RD:
            OnAuthResult(data, reply, option);
            break;
        default:
            LOGE("invalid request code.");
            break;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void IpcClientStub3rd::OnAuthResult(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    ProcessInfo3rd processInfo;
    DecodeProcessInfo(data, processInfo);
    int32_t result = data.ReadInt32();
    int32_t status = data.ReadInt32();
    std::string authContent = data.ReadString();
    DeviceManagerNotify3rd::GetInstance().OnAuthResult(processInfo, result, status, authContent);
    reply.WriteInt32(DM_OK);
}

void IpcClientStub3rd::DecodeProcessInfo(MessageParcel &parcel, ProcessInfo3rd &processInfo)
{
    processInfo.tokenId = parcel.ReadUint32();
    processInfo.uid = parcel.ReadUint32();
    processInfo.businessName = parcel.ReadString();
    processInfo.processName = parcel.ReadString();
    processInfo.userId = parcel.ReadInt32();
}
} // namespace DistributedHardware
} // namespace OHOS