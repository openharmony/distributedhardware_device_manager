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

#include "ipc_server_client_proxy_3rd.h"
#include "ipc_utils_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {

bool IpcServerClientProxy3rd::EncodeProcessInfo(const ProcessInfo3rd &processInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteUint32(processInfo.tokenId));
    bRet = (bRet && parcel.WriteUint32(processInfo.uid));
    bRet = (bRet && parcel.WriteString(processInfo.businessName));
    bRet = (bRet && parcel.WriteString(processInfo.processName));
    bRet = (bRet && parcel.WriteInt32(processInfo.userId));
    return bRet;
}

int32_t IpcServerClientProxy3rd::SendCmd(int32_t cmdCode, const ProcessInfo3rd &processInfo, int32_t result,
    int32_t status, const std::string &authContent)
{
    LOGI("code: %{public}d", cmdCode);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("remote service null");
        return ERR_DM_POINT_NULL;
    }
    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    if (!EncodeProcessInfo(processInfo, data)) {
        LOGE("write processInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    WRITE_HELPER_RET(data, Int32, result, ERR_DM_IPC_WRITE_FAILED);
    WRITE_HELPER_RET(data, Int32, status, ERR_DM_IPC_WRITE_FAILED);
    WRITE_HELPER_RET(data, String, authContent, ERR_DM_IPC_WRITE_FAILED);
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (remote->SendRequest(cmdCode, data, reply, option) != DM_OK) {
        LOGE("SendRequest fail");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = DM_OK;
    READ_HELPER_RET(reply, Int32, ret, ERR_DM_IPC_READ_FAILED);
    if (ret != DM_OK) {
        LOGE("SendRequest failed, ret: %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
