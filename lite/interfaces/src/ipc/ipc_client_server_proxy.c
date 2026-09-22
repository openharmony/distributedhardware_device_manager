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


#include "ipc_client_server_proxy.h"
#include "ipc_client_manager.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "serializer.h"
#include <stdlib.h>

DM_EXPORT IpcClientServerProxy* DmIpcClientServerProxyCreate(void* impl)
{
    (void)impl;
    IpcClientServerProxy* proxy = (IpcClientServerProxy*)calloc(1, sizeof(IpcClientServerProxy));
    return proxy;
}

DM_EXPORT void DmIpcClientServerProxyDestroy(IpcClientServerProxy* self)
{
    if (self != NULL) {
        free(self);
    }
}

DM_EXPORT void DmIpcClientServerProxyInit(IpcClientServerProxy* self)
{
    (void)self;
}

DM_EXPORT void DmIpcClientServerProxyDestroyInner(IpcClientServerProxy* self)
{
    (void)self;
}

DM_EXPORT int32_t DmIpcClientServerProxySendCmd(IpcClientServerProxy* self, int32_t cmdCode,
    void* req, void* rsp)
{
    (void)self;
    (void)cmdCode;
    (void)req;
    (void)rsp;
    return DM_OK;
}
