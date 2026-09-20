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


#ifndef DM_IPC_SERVER_STUB_H
#define DM_IPC_SERVER_STUB_H

#include <stdint.h>
#include "dm_container.h"
#include "dm_device_info_c.h"
#include "dm_thread.h"
#include "ipc/ipc_def.h"
#include "ipc_cmd_register.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DmServiceRunningState {
    DM_STATE_NOT_START = 0,
    DM_STATE_RUNNING = 1
} DmServiceRunningState;

typedef struct IpcServerStub {
    DmServiceRunningState runningState;
    DmMutex stubMutex;
    DmMap_int_DmVoidPtr clientProxyMap;
    DmMutex clientProxyMapMutex;
    DmHmap_DmString_DmVoidPtr pkgNameMap;
    DmMutex pkgNameMapMutex;
    DmSetDmString systemSASet;
} IpcServerStub;

DM_DECLARE_SINGLE_INSTANCE(IpcServerStub);

void DmIpcServerStubOnStart(IpcServerStub* stub);
int32_t DmIpcServerStubOnRemoteRequest(IpcServerStub* stub, uint32_t code, void* data, void* reply, void* option);
void DmIpcServerStubAddSystemSa(IpcServerStub* stub, const DmString* pkgName);

void DmServiceRegisterLite(void);
void DmLiteIpcCmdRegisterAll(void);
void DmIpcServiceInitLite(void);

#ifdef __cplusplus
}
#endif

#endif
