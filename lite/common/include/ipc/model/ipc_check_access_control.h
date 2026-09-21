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


#ifndef DM_IPC_CHECK_ACL_H
#define DM_IPC_CHECK_ACL_H

#include "ipc_def.h"
#include "ipc_req.h"
#include "dm_device_info_c.h"

typedef struct {
    IpcReq base;
    DmAccessCaller caller;
    DmAccessCallee callee;
} IpcCheckAcl;

void DmIpcCheckAclInit(IpcCheckAcl* obj);
void DmIpcCheckAclDestroy(IpcCheckAcl* obj);
int DmIpcCheckAclSerialize(IpcCheckAcl* obj, void* parcel);
int DmIpcCheckAclDeserialize(void* parcel, IpcCheckAcl* obj);

#endif
