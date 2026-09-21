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


#ifndef DM_IPC_CLIENT_STUB_H
#define DM_IPC_CLIENT_STUB_H

#include <stdint.h>
#include "ipc_def.h"
#include "dm_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IpcClientStub IpcClientStub;

DM_EXPORT IpcClientStub* DmIpcClientStubCreate(void);
DM_EXPORT void DmIpcClientStubDestroy(IpcClientStub* self);
DM_EXPORT void DmIpcClientStubInit(IpcClientStub* self);
DM_EXPORT void DmIpcClientStubDestroyInner(IpcClientStub* self);

DM_EXPORT int32_t DmIpcClientStubOnRemoteRequest(IpcClientStub* self, uint32_t code,
    void* data, void* reply, void* option);
DM_EXPORT int32_t DmIpcClientStubSendCmd(IpcClientStub* self, int32_t cmdCode, void* req, void* rsp);

struct IpcClientStub {
    void* remoteStubPtr;
};

void DmIpcClientStubInit(IpcClientStub* self);
void DmIpcClientStubDestroy(IpcClientStub* self);

#ifdef __cplusplus
}
#endif

#endif
