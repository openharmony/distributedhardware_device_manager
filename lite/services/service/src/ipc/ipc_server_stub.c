/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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


#include "ipc_server_stub.h"
#include <stdlib.h>
#include <string.h>
#include "dm_log.h"
#include "dm_error_type.h"
#include "permission_manager.h"


DM_IMPLEMENT_SINGLE_INSTANCE(IpcServerStub)

void DmIpcServerStubOnStart(IpcServerStub* stub)
{
    CHECK_NULL_VOID(stub);
    stub->runningState = DM_STATE_RUNNING;
}

int32_t DmIpcServerStubOnRemoteRequest(IpcServerStub* stub, uint32_t code, void* data, void* reply, void* option)
{
    CHECK_NULL_RETURN(stub, ERR_DM_FAILED);
    return DmIpcCmdRegisterOnRemoteRequest((int32_t)code, data, reply);
}

void DmIpcServerStubAddSystemSa(IpcServerStub* stub, const DmString* pkgName)
{
    CHECK_NULL_VOID(stub);
    if (DmPermissionCheckSystemSa(pkgName)) {
        DmSetDmStringInsert(&stub->systemSASet, pkgName);
    }
}
