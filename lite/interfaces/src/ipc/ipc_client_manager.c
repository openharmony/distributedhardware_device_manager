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


#include "ipc_client_manager.h"
#include "ipc_client_server_proxy.h"
#include "ipc_client_stub.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_container.h"
#include "dm_thread.h"
#include "dm_constants.h"
#include "dm_device_info_c.h"
#include "device_manager_ipc_interface_code.h"
#include "serializer.h"
#include "ipc_skeleton.h"
#include "samgr_lite.h"
#include "iunknown.h"
#include "iproxy_client.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "securec.h"

#define DM_SAMGR_SERVICE_NAME "dev_mgr_svc"
#define DM_MAX_IPC_DATA_LEN 2048

typedef struct {
    IpcIo* reply;
} DmIpcOwner;

static IpcClientManager g_ipcClientManager;
static bool g_isInited = false;

static int DmIpcNotify(IOwner owner, int code, IpcIo* reply)
{
    (void)code;
    if (owner == NULL || reply == NULL) {
        return EC_SUCCESS;
    }
    DmIpcOwner* data = (DmIpcOwner*)owner;
    if (data->reply == NULL || data->reply->bufferBase == NULL) {
        return EC_SUCCESS;
    }
    size_t dataLen = reply->bufferLeft;
    size_t destCap = (size_t)(data->reply->bufferCur - data->reply->bufferBase) + data->reply->bufferLeft;
    if (dataLen > destCap) {
        dataLen = destCap;
    }
    if (dataLen > 0) {
        (void)memcpy_s(data->reply->bufferBase, destCap, reply->bufferBase, dataLen);
    }
    data->reply->bufferCur = data->reply->bufferBase;
    data->reply->bufferLeft = dataLen;
    return EC_SUCCESS;
}

static int32_t DmIpcClientInvoke(uint32_t code, IpcIo* req, IpcIo* reply)
{
    IUnknown* iUnknown = SAMGR_GetInstance()->GetDefaultFeatureApi(DM_SAMGR_SERVICE_NAME);
    if (iUnknown == NULL) {
        LOGE("GetDefaultFeatureApi failed");
        return ERR_DM_FAILED;
    }
    IClientProxy* proxy = (IClientProxy*)iUnknown;
    if (proxy->Invoke == NULL) {
        LOGE("Invoke is NULL");
        return ERR_DM_FAILED;
    }
    DmIpcOwner owner = { .reply = reply };
    return proxy->Invoke(proxy, (int)code, req, (IOwner)&owner, DmIpcNotify);
}

DM_EXPORT IpcClientManager* DmIpcClientManagerCreate(void)
{
    LOGI("create");
    return &g_ipcClientManager;
}

DM_EXPORT void DmIpcClientManagerDestroy(IpcClientManager* self)
{
    (void)self;
    g_isInited = false;
}

DM_EXPORT int32_t DmIpcClientManagerInit(IpcClientManager* self, const DmString* pkgName)
{
    (void)self;
    if (pkgName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("init pkgName=%s", DmStringCstr(pkgName));
    g_isInited = true;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerUnInit(IpcClientManager* self, const DmString* pkgName)
{
    (void)self;
    (void)pkgName;
    LOGI("un_init");
    g_isInited = false;
    return DM_OK;
}

DM_EXPORT void DmIpcClientManagerDestroyInner(IpcClientManager* self)
{
    (void)self;
    g_isInited = false;
}


DM_EXPORT int32_t DmIpcClientManagerSendRequest(IpcClientManager* self, int32_t cmdCode, void* req, void* rsp)
{
    (void)self;
    if (!g_isInited) {
        LOGE("not inited");
        return ERR_DM_FAILED;
    }
    IpcIo* reqIo = (IpcIo*)req;
    IpcIo* rspIo = (IpcIo*)rsp;
    if (reqIo == NULL || rspIo == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t ret = DmIpcClientInvoke((uint32_t)cmdCode, reqIo, rspIo);
    if (ret != 0) {
        LOGE("invoke cmd %d failed %d", cmdCode, ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerOnDmServiceDied(IpcClientManager* self)
{
    (void)self;
    LOGI("service died");
    g_isInited = false;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerConnectSa(IpcClientManager* self)
{
    (void)self;
    IUnknown* iUnknown = SAMGR_GetInstance()->GetDefaultFeatureApi(DM_SAMGR_SERVICE_NAME);
    if (iUnknown == NULL) {
        LOGE("connect_sa failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT void DmIpcClientManagerSubscribeSaChangeListener(IpcClientManager* self)
{
    (void)self;
}

DM_EXPORT void DmIpcClientManagerUnSubscribeSaChangeListener(IpcClientManager* self)
{
    (void)self;
}

DM_EXPORT int32_t DmIpcClientManagerSubscribeSystemAbility(IpcClientManager* self)
{
    (void)self;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerUnsubscribeSystemAbility(IpcClientManager* self)
{
    (void)self;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerRegisterListener(IpcClientManager* self, const DmString* pkgName,
    IpcClientStub* listener)
{
    (void)self;
    (void)pkgName;
    (void)listener;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerUnregisterListener(IpcClientManager* self, const DmString* pkgName)
{
    (void)self;
    (void)pkgName;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerGetSaProxy(IpcClientManager* self)
{
    (void)self;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerSendRegisterListenerCmd(IpcClientManager* self, const DmString* pkgName,
    IpcClientStub* listener)
{
    (void)self;
    (void)pkgName;
    (void)listener;
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientManagerSendUnregisterListenerCmd(IpcClientManager* self, const DmString* pkgName)
{
    (void)self;
    (void)pkgName;
    return DM_OK;
}
