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


#ifndef DM_IPC_CLIENT_MANAGER_H
#define DM_IPC_CLIENT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "ipc_client.h"
#include "ipc_client_stub.h"
#include "ipc_def.h"
#include "dm_container.h"
#include "dm_thread.h"
#include "dm_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IpcClientManager IpcClientManager;

DM_EXPORT IpcClientManager* DmIpcClientManagerCreate(void);
DM_EXPORT void DmIpcClientManagerDestroy(IpcClientManager* self);

DM_EXPORT int32_t DmIpcClientManagerInit(IpcClientManager* self, const DmString* pkgName);
DM_EXPORT int32_t DmIpcClientManagerUnInit(IpcClientManager* self, const DmString* pkgName);
DM_EXPORT void DmIpcClientManagerDestroyInner(IpcClientManager* self);
DM_EXPORT int32_t DmIpcClientManagerSendRequest(IpcClientManager* self, int32_t cmdCode, void* req, void* rsp);
DM_EXPORT int32_t DmIpcClientManagerOnDmServiceDied(IpcClientManager* self);
DM_EXPORT int32_t DmIpcClientManagerConnectSa(IpcClientManager* self);
DM_EXPORT void DmIpcClientManagerSubscribeSaChangeListener(IpcClientManager* self);
DM_EXPORT void DmIpcClientManagerUnSubscribeSaChangeListener(IpcClientManager* self);
DM_EXPORT int32_t DmIpcClientManagerSubscribeSystemAbility(IpcClientManager* self);
DM_EXPORT int32_t DmIpcClientManagerUnsubscribeSystemAbility(IpcClientManager* self);
DM_EXPORT int32_t DmIpcClientManagerRegisterListener(IpcClientManager* self, const DmString* pkgName,
    IpcClientStub* listener);
DM_EXPORT int32_t DmIpcClientManagerUnregisterListener(IpcClientManager* self, const DmString* pkgName);
DM_EXPORT int32_t DmIpcClientManagerGetSaProxy(IpcClientManager* self);
DM_EXPORT int32_t DmIpcClientManagerSendRegisterListenerCmd(IpcClientManager* self, const DmString* pkgName,
    IpcClientStub* listener);
DM_EXPORT int32_t DmIpcClientManagerSendUnregisterListenerCmd(IpcClientManager* self, const DmString* pkgName);

struct IpcClientManager {
    IpcClient base;
    DmMutex lock;
    DmHmap_DmString_DmVoidPtr dmListener;
    void* dmInterface;
    void* dmRecipient;
    bool isSubscribeDMSAChangeListener;
    void* saListenerCallback;
};


#ifdef __cplusplus
}
#endif

#endif
