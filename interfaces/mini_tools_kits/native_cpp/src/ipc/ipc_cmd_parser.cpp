/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "device_manager_ipc_interface_code.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_cmd_register.h"
#include "ipc_get_local_display_device_name_req.h"
#include "ipc_get_local_display_device_name_rsp.h"
#include "ipc_def.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "message_parcel.h"
#include "parcel.h"

namespace OHOS {
namespace DistributedHardware {

ON_IPC_SET_REQUEST(GET_LOCAL_DISPLAY_DEVICE_NAME, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalDisplayDeviceNameReq> pReq =
        std::static_pointer_cast<IpcGetLocalDisplayDeviceNameReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pReq->GetMaxNameLength())) {
        LOGE("write maxNameLength failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DISPLAY_DEVICE_NAME, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalDisplayDeviceNameRsp> pRsp =
        std::static_pointer_cast<IpcGetLocalDisplayDeviceNameRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetDisplayName(reply.ReadString());
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
