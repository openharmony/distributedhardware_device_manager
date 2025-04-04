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

#include "ipc_client_proxy.h"

#include "device_manager_ipc_interface_code.h"
#include "dm_error_type.h"
#include "dm_log.h"
namespace OHOS::DistributedHardware { class IpcReq; }
namespace OHOS::DistributedHardware { class IpcRsp; }

namespace OHOS {
namespace DistributedHardware {
int32_t IpcClientProxy::Init(const std::string &pkgName)
{
    if (ipcClientManager_ == nullptr) {
        return ERR_DM_POINT_NULL;
    }
    return ipcClientManager_->Init(pkgName);
}

int32_t IpcClientProxy::UnInit(const std::string &pkgName)
{
    if (ipcClientManager_ == nullptr) {
        return ERR_DM_POINT_NULL;
    }
    return ipcClientManager_->UnInit(pkgName);
}

int32_t IpcClientProxy::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    if (ipcClientManager_ == nullptr || req == nullptr || rsp == nullptr) {
        LOGE("req,rsp or ipc client is null");
        return ERR_DM_POINT_NULL;
    }
    return ipcClientManager_->SendRequest(cmdCode, req, rsp);
}

int32_t IpcClientProxy::OnDmServiceDied()
{
    if (ipcClientManager_ == nullptr) {
        LOGE("IpcClientProxy::ipcClientManager_ is null");
        return ERR_DM_POINT_NULL;
    }
    return ipcClientManager_->OnDmServiceDied();
}
} // namespace DistributedHardware
} // namespace OHOS
