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


#include "dm_lite_client_notify.h"
#include "ipc_skeleton.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_constants.h"
#include "securec.h"
#include "dm_crypto.h"
#include "dm_device_info_c.h"
#include "softbus_bus_center.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DM_IPC_NOTIFY_BUF_SIZE 4096

typedef struct {
    int32_t handle;
    pid_t pid;
} DmClientDeathCtx;

static DmLiteClientRegistry g_clientRegistry = {
    .lock = { PTHREAD_MUTEX_INITIALIZER }
};

static void DmLiteRegistryLock(void)
{
    DmMutexLock(&g_clientRegistry.lock);
}

static void DmLiteRegistryUnlock(void)
{
    DmMutexUnlock(&g_clientRegistry.lock);
}

static int32_t DmLiteRegistryFindByHandle(int32_t handle)
{
    for (int32_t i = 0; i < g_clientRegistry.count; i++) {
        if (g_clientRegistry.clients[i].handle == handle) {
            return i;
        }
    }
    return -1;
}

static void DmLiteRegistryRemoveSlotLocked(int32_t index, DmClientDeathCtx** outCtx)
{
    if (index < 0 || index >= g_clientRegistry.count) {
        return;
    }
    if (outCtx != NULL) {
        *outCtx = g_clientRegistry.deathCtxs[index];
    }
    for (int32_t j = index; j < g_clientRegistry.count - 1; j++) {
        g_clientRegistry.clients[j] = g_clientRegistry.clients[j + 1];
        g_clientRegistry.deathCbIds[j] = g_clientRegistry.deathCbIds[j + 1];
        g_clientRegistry.deathCtxs[j] = g_clientRegistry.deathCtxs[j + 1];
        g_clientRegistry.pids[j] = g_clientRegistry.pids[j + 1];
        DmStringAssign(&g_clientRegistry.pkgNames[j], &g_clientRegistry.pkgNames[j + 1]);
    }
    int32_t last = g_clientRegistry.count - 1;
    DmStringDestroy(&g_clientRegistry.pkgNames[last]);
    (void)memset_s(&g_clientRegistry.clients[last], sizeof(SvcIdentity), 0, sizeof(SvcIdentity));
    g_clientRegistry.deathCbIds[last] = 0;
    g_clientRegistry.deathCtxs[last] = NULL;
    g_clientRegistry.pids[last] = 0;
    g_clientRegistry.count--;
}

static int32_t DmLiteRegistryFindOwnerByCtxLocked(void* ctx)
{
    for (int32_t i = 0; i < g_clientRegistry.count; i++) {
        if (g_clientRegistry.deathCtxs[i] == ctx) {
            return i;
        }
    }
    return -1;
}

void DmClientOnRemoteDead(void* args)
{
    if (args == NULL) {
        return;
    }

    DmClientDeathCtx* owned = NULL;
    int32_t handle = 0;
    pid_t pid = 0;
    bool cleaned = false;

    DmLiteRegistryLock();
    int32_t index = DmLiteRegistryFindOwnerByCtxLocked(args);
    if (index < 0) {
        DmLiteRegistryUnlock();
        return;
    }
    handle = g_clientRegistry.clients[index].handle;
    pid = g_clientRegistry.pids[index];
    DmLiteRegistryRemoveSlotLocked(index, &owned);
    int32_t remain = g_clientRegistry.count;
    DmLiteRegistryUnlock();

    if (owned != NULL) {
        free(owned);
        cleaned = true;
    }
    LOGE("[death] client crashed, cleaned handle=%d pid=%d remain=%d freed=%d",
         handle, (int)pid, remain, cleaned ? 1 : 0);
    if (remain == 0) {
        LOGW("[death] no listener left after cleanup");
    }
}

void DmLiteClientNotifyInit(void)
{
    DmLiteRegistryLock();
    g_clientRegistry.count = 0;
    DmLiteRegistryUnlock();
}

static int32_t DmAddDeathRecipientLocked(const DmString* pkgName, int32_t handle,
    pid_t callerPid, DmClientDeathCtx* ctx, bool* isNew)
{
    SvcIdentity target;
    DmLiteRegistryLock();
    int32_t cur = DmLiteRegistryFindByHandle(handle);
    if (cur < 0) {
        DmLiteRegistryUnlock();
        LOGE("[death] client vanished during register handle=%d", handle);
        return ERR_DM_FAILED;
    }
    target = g_clientRegistry.clients[cur];
    uint32_t cbId = 0;
    int32_t addRet = AddDeathRecipient(target, DmClientOnRemoteDead, (void*)ctx, &cbId);
    if (addRet != 0) {
        DmClientDeathCtx* removed = NULL;
        DmLiteRegistryRemoveSlotLocked(cur, &removed);
        DmLiteRegistryUnlock();
        if (removed != NULL) {
            free(removed);
        }
        LOGE("AddDeathRecipient failed ret=%d handle=%d pkgName=%s", addRet, handle, DmStringCstr(pkgName));
        return ERR_DM_FAILED;
    }
    g_clientRegistry.deathCbIds[cur] = cbId;
    int32_t totalCount = g_clientRegistry.count;
    DmLiteRegistryUnlock();
    LOGW("register client pkgName=%s handle=%d pid=%d cbId=%u count=%d",
         DmStringCstr(pkgName), handle, (int)callerPid, cbId, totalCount);
    return DM_OK;
}

static int32_t DmAllocClientSlot(const DmString* pkgName, const SvcIdentity* svc,
    pid_t callerPid, DmClientDeathCtx** outCtx)
{
    int32_t handle = svc->handle;
    DmLiteRegistryLock();
    int32_t index = DmLiteRegistryFindByHandle(handle);
    if (index >= 0) {
        g_clientRegistry.clients[index] = *svc;
        g_clientRegistry.pids[index] = callerPid;
        DmStringAssign(&g_clientRegistry.pkgNames[index], pkgName);
        DmLiteRegistryUnlock();
        return -1;
    }
    if (g_clientRegistry.count >= DM_MAX_REGISTERED_CLIENTS) {
        DmLiteRegistryUnlock();
        LOGE("client registry full, reject pkgName=%s", DmStringCstr(pkgName));
        return ERR_DM_FAILED;
    }
    DmClientDeathCtx* ctx = (DmClientDeathCtx*)calloc(1, sizeof(DmClientDeathCtx));
    if (ctx == NULL) {
        DmLiteRegistryUnlock();
        LOGE("alloc death ctx failed pkgName=%s", DmStringCstr(pkgName));
        return ERR_DM_FAILED;
    }
    ctx->handle = handle;
    ctx->pid = callerPid;
    index = g_clientRegistry.count;
    g_clientRegistry.clients[index] = *svc;
    g_clientRegistry.deathCbIds[index] = 0;
    g_clientRegistry.deathCtxs[index] = (void*)ctx;
    g_clientRegistry.pids[index] = callerPid;
    DmStringAssign(&g_clientRegistry.pkgNames[index], pkgName);
    g_clientRegistry.count++;
    DmLiteRegistryUnlock();
    *outCtx = ctx;
    return DM_OK;
}

int32_t DmLiteClientNotifyRegister(const DmString* pkgName, const SvcIdentity* svc)
{
    if (pkgName == NULL || svc == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (svc->handle == IPC_INVALID_HANDLE) {
        LOGE("register rejected, invalid handle pkgName=%s", DmStringCstr(pkgName));
        return ERR_DM_INPUT_PARA_INVALID;
    }
    pid_t callerPid = GetCallingPid();
    DmClientDeathCtx* ctx = NULL;
    int32_t slotRet = DmAllocClientSlot(pkgName, svc, callerPid, &ctx);
    if (slotRet == -1) {
        return DM_OK;
    }
    if (slotRet != DM_OK) {
        return slotRet;
    }
    int32_t ret = DmAddDeathRecipientLocked(pkgName, svc->handle, callerPid, ctx, NULL);
    if (ret != DM_OK) {
        return ret;
    }
    DmLiteClientNotifyOnlineDevicesToNewClient(svc->handle);
    return DM_OK;
}

static void DmWriteUdidHashToIpc(IpcIo* ipcReq, const char* networkId)
{
    char peerUdid[UDID_BUF_LEN] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME, networkId, NODE_KEY_UDID,
        (uint8_t*)peerUdid, sizeof(peerUdid)) == 0 && peerUdid[0] != '\0') {
        DmString udidStr = DmStringCreate(peerUdid);
        char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (DmGetUdidHashBuf(&udidStr, (unsigned char*)udidHash) == DM_OK) {
            WriteString(ipcReq, udidHash);
        } else {
            WriteString(ipcReq, "");
        }
        DmStringDestroy(&udidStr);
    } else {
        WriteString(ipcReq, "");
    }
}

static void DmDispatchOnlineNotifications(SvcIdentity* svc, int32_t count)
{
    (void)count;
    NodeBasicInfo* infoList = NULL;
    int32_t infoNum = 0;
    const char* pkgName = DM_PKG_NAME_LITE;
    int32_t ret = GetAllNodeDeviceInfo(pkgName, &infoList, &infoNum);
    if (ret != 0 || infoNum <= 0 || infoList == NULL) {
        if (infoList != NULL) {
            FreeNodeInfo(infoList);
        }
        return;
    }
    LOGW("[BACKFILL] %d online devices to client handle=%d", infoNum, svc->handle);
    for (int32_t i = 0; i < infoNum; i++) {
        NodeBasicInfo* info = &infoList[i];
        uint8_t* buf = (uint8_t*)malloc(DM_IPC_NOTIFY_BUF_SIZE);
        if (buf == NULL) {
            continue;
        }
        IpcIo ipcReq;
        IpcIoInit(&ipcReq, buf, DM_IPC_NOTIFY_BUF_SIZE, 0);
        WriteInt32(&ipcReq, DEVICE_STATE_ONLINE);
        WriteInt32(&ipcReq, 1);
        WriteString(&ipcReq, info->networkId);
        WriteString(&ipcReq, info->deviceName);
        WriteInt32(&ipcReq, info->deviceTypeId);
        DmWriteUdidHashToIpc(&ipcReq, info->networkId);
        uint8_t replyBuf[256];
        IpcIo ipcReply;
        IpcIoInit(&ipcReply, replyBuf, sizeof(replyBuf), 0);
        MessageOption option;
        MessageOptionInit(&option);
        option.flags = TF_OP_ASYNC;
        int32_t sendRet = SendRequest(*svc, DM_SERVER_DEVICE_STATE_NOTIFY, &ipcReq, &ipcReply, option, NULL);
        free(buf);
    }
    FreeNodeInfo(infoList);
}

void DmLiteClientNotifyOnlineDevicesToNewClient(int32_t handle)
{
    SvcIdentity svc;
    (void)memset_s(&svc, sizeof(SvcIdentity), 0, sizeof(SvcIdentity));

    DmLiteRegistryLock();
    int32_t index = DmLiteRegistryFindByHandle(handle);
    if (index < 0) {
        DmLiteRegistryUnlock();
        return;
    }
    svc = g_clientRegistry.clients[index];
    DmLiteRegistryUnlock();

    if (svc.handle == IPC_INVALID_HANDLE) {
        return;
    }

    DmDispatchOnlineNotifications(&svc, 0);
}

int32_t DmLiteClientNotifyUnregister(const DmString* pkgName)
{
    if (pkgName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SvcIdentity targets[DM_MAX_REGISTERED_CLIENTS];
    uint32_t cbIds[DM_MAX_REGISTERED_CLIENTS];
    DmClientDeathCtx* ctxList[DM_MAX_REGISTERED_CLIENTS];
    int32_t matched = 0;
    (void)memset_s(targets, sizeof(targets), 0, sizeof(targets));
    (void)memset_s(cbIds, sizeof(cbIds), 0, sizeof(cbIds));
    (void)memset_s(ctxList, sizeof(ctxList), 0, sizeof(ctxList));

    DmLiteRegistryLock();
    for (int32_t i = g_clientRegistry.count - 1; i >= 0; i--) {
        if (strcmp(DmStringCstr(&g_clientRegistry.pkgNames[i]), DmStringCstr(pkgName)) != 0) {
            continue;
        }
        if (matched < DM_MAX_REGISTERED_CLIENTS) {
            targets[matched] = g_clientRegistry.clients[i];
            cbIds[matched] = g_clientRegistry.deathCbIds[i];
            DmClientDeathCtx* ctx = NULL;
            DmLiteRegistryRemoveSlotLocked(i, &ctx);
            ctxList[matched] = ctx;
            matched++;
        }
    }
    int32_t remain = g_clientRegistry.count;
    DmLiteRegistryUnlock();

    if (matched == 0) {
        return ERR_DM_FAILED;
    }
    for (int32_t i = 0; i < matched; i++) {
        int32_t rmRet = RemoveDeathRecipient(targets[i], cbIds[i]);
        if (ctxList[i] != NULL) {
            free(ctxList[i]);
        }
    }
    return DM_OK;
}

static void DmLiteNotifyAllClients(int32_t cmdCode, IpcIo* ipcReq)
{
    SvcIdentity snapshot[DM_MAX_REGISTERED_CLIENTS];
    int32_t snapshotCount = 0;
    (void)memset_s(snapshot, sizeof(snapshot), 0, sizeof(snapshot));

    DmLiteRegistryLock();
    for (int32_t i = 0; i < g_clientRegistry.count && snapshotCount < DM_MAX_REGISTERED_CLIENTS; i++) {
        if (g_clientRegistry.clients[i].handle == IPC_INVALID_HANDLE) {
            continue;
        }
        snapshot[snapshotCount++] = g_clientRegistry.clients[i];
    }
    DmLiteRegistryUnlock();

    bool notified = false;
    for (int32_t i = 0; i < snapshotCount; i++) {
        uint8_t replyBuf[256];
        IpcIo ipcReply;
        IpcIoInit(&ipcReply, replyBuf, sizeof(replyBuf), 0);
        MessageOption option;
        MessageOptionInit(&option);
        option.flags = TF_OP_ASYNC;
        int32_t ret = SendRequest(snapshot[i], cmdCode, ipcReq, &ipcReply, option, NULL);
        if (ret != DM_OK) {
            LOGW("notify client %d failed %d", snapshot[i].handle, ret);
        } else {
            LOGI("notified client handle=%d cmd=%d", snapshot[i].handle, cmdCode);
            notified = true;
        }
    }
    if (!notified) {
        LOGW("no client with valid handle, IPC notify skipped cmd=%d", cmdCode);
    }
}

void DmLiteClientNotifyDeviceState(DmDeviceState state, const DmDeviceInfo* info, bool isOnline)
{
    uint8_t* buf = (uint8_t*)malloc(DM_IPC_NOTIFY_BUF_SIZE);
    if (buf == NULL) {
        return;
    }
    IpcIo ipcReq;
    IpcIoInit(&ipcReq, buf, DM_IPC_NOTIFY_BUF_SIZE, 0);
    WriteInt32(&ipcReq, state);
    WriteInt32(&ipcReq, isOnline ? 1 : 0);
    if (info != NULL) {
        WriteString(&ipcReq, info->networkId);
        WriteString(&ipcReq, info->deviceName);
        WriteInt32(&ipcReq, info->deviceTypeId);
        WriteString(&ipcReq, info->deviceId);
    }
    DmLiteNotifyAllClients(DM_SERVER_DEVICE_STATE_NOTIFY, &ipcReq);
    free(buf);
}

void DmLiteClientNotifyDeviceFound(uint16_t subscribeId, const DmDeviceInfo* info)
{
    uint8_t* buf = (uint8_t*)malloc(DM_IPC_NOTIFY_BUF_SIZE);
    if (buf == NULL) {
        return;
    }
    IpcIo ipcReq;
    IpcIoInit(&ipcReq, buf, DM_IPC_NOTIFY_BUF_SIZE, 0);
    WriteInt32(&ipcReq, subscribeId);
    if (info != NULL) {
        WriteString(&ipcReq, info->deviceId);
        WriteString(&ipcReq, info->networkId);
        WriteString(&ipcReq, info->deviceName);
        WriteInt32(&ipcReq, info->deviceTypeId);
        const char* extra = (info->extraData.data != NULL) ? info->extraData.data : "";
        WriteString(&ipcReq, extra);
    }
    DmLiteNotifyAllClients(DM_SERVER_DEVICE_FOUND, &ipcReq);
    free(buf);
}

void DmLiteClientNotifyPublishResult(int32_t publishId, int32_t result)
{
    uint8_t* buf = (uint8_t*)malloc(DM_IPC_NOTIFY_BUF_SIZE);
    if (buf == NULL) {
        return;
    }
    IpcIo ipcReq;
    IpcIoInit(&ipcReq, buf, DM_IPC_NOTIFY_BUF_SIZE, 0);
    WriteInt32(&ipcReq, publishId);
    WriteInt32(&ipcReq, result);
    DmLiteNotifyAllClients(DM_SERVER_PUBLISH_FINISH, &ipcReq);
    free(buf);
}

void DmLiteClientNotifyDiscoverResult(const char* pkgName, uint16_t subscribeId, int32_t result)
{
    uint8_t* buf = (uint8_t*)malloc(DM_IPC_NOTIFY_BUF_SIZE);
    if (buf == NULL) {
        return;
    }
    IpcIo ipcReq;
    IpcIoInit(&ipcReq, buf, DM_IPC_NOTIFY_BUF_SIZE, 0);
    WriteString(&ipcReq, pkgName ? pkgName : "");
    WriteInt32(&ipcReq, (int32_t)subscribeId);
    WriteInt32(&ipcReq, result);
    DmLiteNotifyAllClients(DM_SERVER_DISCOVER_FINISH, &ipcReq);
    free(buf);
}
