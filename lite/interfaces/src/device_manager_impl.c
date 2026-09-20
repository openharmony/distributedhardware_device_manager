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


#include "device_manager.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_container.h"
#include "dm_thread.h"
#include "dm_constants.h"
#include "ipc_client_manager.h"
#include "ipc_client_stub.h"
#include "serializer.h"
#include "ipc_skeleton.h"
#include "device_manager_ipc_interface_code.h"
#include "device_manager_notify.h"
#include "device_manager_callback.h"
#include "softbus_bus_center.h"
#include "samgr_lite.h"
#include "iproxy_client.h"
#include "cJSON.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "securec.h"

#define DM_IPC_BUF_SIZE 8192
const int DM_IPC_MAX_OBJECTS = 8;

static IpcClientManager* g_ipcClientMgr = NULL;

static int32_t DmClientOnRemoteRequest(uint32_t code, IpcIo *data, IpcIo *reply, MessageOption option)
{
    IpcClientStub* stub = DmIpcClientStubCreate();
    if (stub == NULL) {
        return 0;
    }
    int32_t ret = DmIpcClientStubOnRemoteRequest(stub, code, (void*)data, (void*)reply, (void*)&option);
    DmIpcClientStubDestroy(stub);
    return ret;
}

static IpcObjectStub g_clientIpcStub = {
    .func = DmClientOnRemoteRequest,
    .args = NULL,
    .isRemote = false,
};

static SvcIdentity g_clientSvcIdentity = {
    .handle = IPC_INVALID_HANDLE,
    .token = SERVICE_TYPE_ANONYMOUS,
    .cookie = (uintptr_t)&g_clientIpcStub
};

static int32_t DmImplSendRequest(int32_t cmdCode, IpcIo* req, IpcIo* reply)
{
    if (g_ipcClientMgr == NULL) {
        g_ipcClientMgr = DmIpcClientManagerCreate();
    }
    return DmIpcClientManagerSendRequest(g_ipcClientMgr, cmdCode, req, reply);
}

#define DM_SAMGR_SVC_NAME "dev_mgr_svc"
#define DM_CLIENT_RECOVERY_POLL_SEC 2

typedef struct {
    DmString pkgName;
    bool inited;
    bool listenerRegistered;
    bool devStateRegistered;
    DmInitCallback* initCallback;
} DmClientRegMirror;

static DmClientRegMirror g_regMirror;
static DmMutex g_mirrorLock = { PTHREAD_MUTEX_INITIALIZER };
static volatile bool g_dmAlive = true;
static uint32_t g_dmDeathCbId = 0;
static pthread_t g_recoveryTid;
static bool g_recoveryStarted = false;

static int32_t DmClientSendRegisterListener(const DmString* pkgName)
{
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), DM_IPC_MAX_OBJECTS);
    WriteString(&req, DmStringCstr(pkgName));
    WriteRemoteObject(&req, &g_clientSvcIdentity);
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_REGISTER_DEVICE_MANAGER_LISTENER, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}

static int32_t DmClientSendRegisterDevState(const DmString* pkgName)
{
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), DM_IPC_MAX_OBJECTS);
    WriteString(&req, DmStringCstr(pkgName));
    WriteRemoteObject(&req, &g_clientSvcIdentity);
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_REGISTER_DEV_STATE_CALLBACK, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}

static void DmClientOnServiceDied(void* args)
{
    (void)args;
    DmInitCallback* cb = NULL;
    DmMutexLock(&g_mirrorLock);
    g_dmAlive = false;
    cb = g_regMirror.initCallback;
    DmMutexUnlock(&g_mirrorLock);

    LOGE("[recover] DM service died, will auto re-register");
    if (cb != NULL && cb->vtable != NULL && cb->vtable->OnRemoteDied != NULL) {
        cb->vtable->OnRemoteDied(cb->ctx);
    } else {
        LOGW("[recover] no OnRemoteDied callback provided by caller");
    }
}

static int32_t DmClientArmServiceDeath(void)
{
    SvcIdentity dmId = SAMGR_GetRemoteIdentity(DM_SAMGR_SVC_NAME, NULL);
    if (dmId.handle == 0 || dmId.handle == (uint32_t)(-1)) {
        LOGE("[recover] DM service identity unavailable handle=%u", dmId.handle);
        return ERR_DM_FAILED;
    }
    uint32_t cbId = 0;
    int32_t ret = AddDeathRecipient(dmId, DmClientOnServiceDied, NULL, &cbId);
    if (ret != 0) {
        LOGE("[recover] AddDeathRecipient on DM failed ret=%d handle=%u", ret, dmId.handle);
        return ERR_DM_FAILED;
    }
    DmMutexLock(&g_mirrorLock);
    g_dmDeathCbId = cbId;
    DmMutexUnlock(&g_mirrorLock);
    LOGW("[recover] armed DM death recipient handle=%u cbId=%u", dmId.handle, cbId);
    return DM_OK;
}

static bool DmCheckAndRecoverService(const DmString* pkgName, bool replayDevState)
{
    SvcIdentity probe = SAMGR_GetRemoteIdentity(DM_SAMGR_SVC_NAME, NULL);
    if (probe.handle == 0 || probe.handle == (uint32_t)(-1)) {
        LOGW("[recover] DM service not back yet");
        return false;
    }
    LOGW("[recover] DM service back handle=%u, replaying registration", probe.handle);
    int32_t ret = DmClientSendRegisterListener(pkgName);
    if (ret != DM_OK) {
        LOGE("[recover] replay REGISTER_LISTENER failed ret=%d", ret);
        return false;
    }
    DmMutexLock(&g_mirrorLock);
    g_regMirror.listenerRegistered = true;
    DmMutexUnlock(&g_mirrorLock);
    if (replayDevState) {
        int32_t dsRet = DmClientSendRegisterDevState(pkgName);
        if (dsRet != DM_OK) {
            LOGE("[recover] replay REGISTER_DEV_STATE failed ret=%d", dsRet);
        }
    }
    if (DmClientArmServiceDeath() != DM_OK) {
        LOGE("[recover] re-arm death recipient failed");
        return false;
    }
    return true;
}

static void DmDoClientRecovery(void)
{
    sleep(DM_CLIENT_RECOVERY_POLL_SEC);

    DmMutexLock(&g_mirrorLock);
    bool needRecover = !g_dmAlive;
    bool inited = g_regMirror.inited;
    DmString pkgName = DmStringCreateEmpty();
    if (inited) {
        DmStringAssign(&pkgName, &g_regMirror.pkgName);
    }
    bool replayDevState = g_regMirror.devStateRegistered;
    DmMutexUnlock(&g_mirrorLock);

    if (needRecover && inited && DmCheckAndRecoverService(&pkgName, replayDevState)) {
        DmMutexLock(&g_mirrorLock);
        g_dmAlive = true;
        DmMutexUnlock(&g_mirrorLock);
        LOGW("[recover] registration restored, devState=%d", replayDevState ? 1 : 0);
    }
    DmStringDestroy(&pkgName);
}

static void* DmClientRecoveryThread(void* arg)
{
    (void)arg;
    while (g_dmAlive) {
        DmDoClientRecovery();
    }
    return NULL;
}

static void DmClientEnsureRecoveryThread(void)
{
    DmMutexLock(&g_mirrorLock);
    bool needStart = !g_recoveryStarted;
    if (needStart) {
        g_recoveryStarted = true;
    }
    DmMutexUnlock(&g_mirrorLock);

    if (!needStart) {
        return;
    }
    if (pthread_create(&g_recoveryTid, NULL, DmClientRecoveryThread, NULL) != 0) {
        DmMutexLock(&g_mirrorLock);
        g_recoveryStarted = false;
        DmMutexUnlock(&g_mirrorLock);
        LOGE("[recover] create recovery thread failed");
        return;
    }
    pthread_detach(g_recoveryTid);
    LOGW("[recover] recovery thread started");
}

int32_t DmImplInitDeviceManager(const DmString* pkgName, DmInitCallback* dmInitCallback)
{
    if (pkgName == NULL || dmInitCallback == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (g_ipcClientMgr == NULL) {
        g_ipcClientMgr = DmIpcClientManagerCreate();
    }
    DmIpcClientManagerInit(g_ipcClientMgr, pkgName);

    DmMutexLock(&g_mirrorLock);
    DmStringAssign(&g_regMirror.pkgName, pkgName);
    g_regMirror.initCallback = dmInitCallback;
    g_regMirror.inited = true;
    g_regMirror.listenerRegistered = false;
    DmMutexUnlock(&g_mirrorLock);

    int32_t result = DmClientSendRegisterListener(pkgName);
    if (result != DM_OK) {
        DmMutexLock(&g_mirrorLock);
        g_regMirror.inited = false;
        g_regMirror.initCallback = NULL;
        DmMutexUnlock(&g_mirrorLock);
        DmIpcClientManagerUnInit(g_ipcClientMgr, pkgName);
        return result;
    }

    DmMutexLock(&g_mirrorLock);
    g_regMirror.listenerRegistered = true;
    g_dmAlive = true;
    DmMutexUnlock(&g_mirrorLock);

    if (DmClientArmServiceDeath() != DM_OK) {
        LOGW("arm DM death recipient failed, auto-recovery unavailable");
    }
    DmClientEnsureRecoveryThread();
    return result;
}

int32_t DmImplUnInitDeviceManager(const DmString* pkgName)
{
    if (pkgName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    DmMutexLock(&g_mirrorLock);
    g_regMirror.inited = false;
    g_regMirror.listenerRegistered = false;
    g_regMirror.devStateRegistered = false;
    g_regMirror.initCallback = NULL;
    uint32_t cbId = g_dmDeathCbId;
    g_dmDeathCbId = 0;
    g_dmAlive = true;
    DmMutexUnlock(&g_mirrorLock);

    if (cbId != 0) {
        SvcIdentity dmId = SAMGR_GetRemoteIdentity(DM_SAMGR_SVC_NAME, NULL);
        if (dmId.handle != 0 && dmId.handle != (uint32_t)(-1)) {
            int32_t rmRet = RemoveDeathRecipient(dmId, cbId);
            LOGI("[recover] removed DM death recipient cbId=%u ret=%d", cbId, rmRet);
        }
    }

    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_UNREGISTER_DEVICE_MANAGER_LISTENER, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}

int32_t DmImplGetTrustedDeviceList(const DmString* pkgName, const DmString* extra, DmVec_DmDeviceInfo* deviceList)
{
    if (pkgName == NULL || deviceList == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_GET_TRUST_DEVICE_LIST, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    if (result != DM_OK) {
        return result;
    }
    int32_t count = 0;
    ReadInt32(&reply, &count);
    for (int32_t i = 0; i < count; i++) {
        DmDeviceInfo info;
        (void)memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        const char* deviceId = (const char*)ReadString(&reply, NULL);
        const char* deviceName = (const char*)ReadString(&reply, NULL);
        int32_t deviceTypeId = 0;
        ReadInt32(&reply, &deviceTypeId);
        const char* networkId = (const char*)ReadString(&reply, NULL);
        if (deviceId != NULL) {
            (void)strncpy_s(info.deviceId, sizeof(info.deviceId), deviceId, sizeof(info.deviceId) - 1);
        }
        if (deviceName != NULL) {
            (void)strncpy_s(info.deviceName, sizeof(info.deviceName), deviceName, sizeof(info.deviceName) - 1);
        }
        info.deviceTypeId = (uint16_t)deviceTypeId;
        if (networkId != NULL) {
            (void)strncpy_s(info.networkId, sizeof(info.networkId), networkId, sizeof(info.networkId) - 1);
        }
        int32_t authForm = 0;
        ReadInt32(&reply, &authForm);
        info.authForm = (DmAuthForm)authForm;
        DmVec_DmDeviceInfo_Push(deviceList, info);
    }
    return result;
}


int32_t DmImplGetLocalDeviceInfo(const DmString* pkgName, DmDeviceInfo* info)
{
    if (pkgName == NULL || info == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_GET_LOCAL_DEVICE_INFO, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    if (result != DM_OK) {
        return result;
    }
    (void)memset_s(info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    const char* deviceId = (const char*)ReadString(&reply, NULL);
    const char* deviceName = (const char*)ReadString(&reply, NULL);
    int32_t deviceTypeId = 0;
    ReadInt32(&reply, &deviceTypeId);
    const char* networkId = (const char*)ReadString(&reply, NULL);
    if (deviceId != NULL) {
        (void)strncpy_s(info->deviceId, sizeof(info->deviceId), deviceId, sizeof(info->deviceId) - 1);
    }
    if (deviceName != NULL) {
        (void)strncpy_s(info->deviceName, sizeof(info->deviceName), deviceName, sizeof(info->deviceName) - 1);
    }
    info->deviceTypeId = (uint16_t)deviceTypeId;
    if (networkId != NULL) {
        (void)strncpy_s(info->networkId, sizeof(info->networkId), networkId, sizeof(info->networkId) - 1);
    }
    return result;
}

int32_t DmImplGetDeviceInfoByNetworkId(const DmString* pkgName, const DmString* networkId, DmDeviceInfo* info)
{
    if (pkgName == NULL || networkId == NULL || info == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    WriteString(&req, DmStringCstr(networkId));
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_GET_DEVICE_INFO, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    if (result != DM_OK) {
        return result;
    }
    (void)memset_s(info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    const char* deviceId = (const char*)ReadString(&reply, NULL);
    const char* netId = (const char*)ReadString(&reply, NULL);
    const char* deviceName = (const char*)ReadString(&reply, NULL);
    int32_t deviceTypeId = 0;
    ReadInt32(&reply, &deviceTypeId);
    if (deviceId != NULL) {
        (void)strncpy_s(info->deviceId, sizeof(info->deviceId), deviceId, sizeof(info->deviceId) - 1);
    }
    if (netId != NULL) {
        (void)strncpy_s(info->networkId, sizeof(info->networkId), netId, sizeof(info->networkId) - 1);
    }
    if (deviceName != NULL) {
        (void)strncpy_s(info->deviceName, sizeof(info->deviceName), deviceName, sizeof(info->deviceName) - 1);
    }
    info->deviceTypeId = (uint16_t)deviceTypeId;
    return result;
}


int32_t DmImplPublishDeviceDiscovery(const DmString* pkgName, const DmPublishInfo* publishInfo,
    PublishCallback* callback)
{
    if (pkgName == NULL || publishInfo == NULL || callback == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify* notify = DmDeviceManagerNotifyGetInstance();
    DmNotifyRegisterPublishCallback(notify, DmStringCstr(pkgName), publishInfo->publishId, callback);
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    WriteInt32(&req, publishInfo->publishId);
    WriteInt32(&req, (int32_t)publishInfo->mode);
    WriteInt32(&req, (int32_t)publishInfo->freq);
    WriteInt32(&req, (int32_t)publishInfo->medium);
    WriteInt32(&req, (int32_t)publishInfo->ranging);
    WriteString(&req, publishInfo->capability);
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_PUBLISH_DEVICE_DISCOVER, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}

int32_t DmImplUnPublishDeviceDiscovery(const DmString* pkgName, int32_t publishId)
{
    if (pkgName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    WriteInt32(&req, publishId);
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_UNPUBLISH_DEVICE_DISCOVER, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}


int32_t DmImplStartDeviceDiscovery(const DmString* pkgName, const DmSubscribeInfo* subscribeInfo,
    const DmString* extra, DiscoveryCallback* callback)
{
    if (pkgName == NULL || subscribeInfo == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify* notify = DmDeviceManagerNotifyGetInstance();
    DmNotifyRegisterDiscoveryCallback(notify, DmStringCstr(pkgName), subscribeInfo->subscribeId, callback);
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    WriteInt32(&req, subscribeInfo->subscribeId);
    WriteInt32(&req, (int32_t)subscribeInfo->mode);
    WriteInt32(&req, (int32_t)subscribeInfo->medium);
    WriteInt32(&req, (int32_t)subscribeInfo->freq);
    WriteString(&req, subscribeInfo->capability);
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_START_DISCOVERING, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}

int32_t DmImplStopDeviceDiscovery(const DmString* pkgName, uint16_t subscribeId)
{
    if (pkgName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    WriteInt32(&req, (int32_t)subscribeId);
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_STOP_DISCOVERING, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}

int32_t DmImplRegisterDevStateCallback(const DmString* pkgName, const DmString* extra, DeviceStateCallback* callback)
{
    if (pkgName == NULL || callback == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    /* Save callback locally for notification dispatch */
    DeviceManagerNotify* notify = DmDeviceManagerNotifyGetInstance();
    DmNotifyRegisterDeviceStateCallback(notify, DmStringCstr(pkgName), callback);
    /* Send IPC to register with service (service will [BACKFILL] currently online devices) */
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), DM_IPC_MAX_OBJECTS);
    WriteString(&req, DmStringCstr(pkgName));
    WriteRemoteObject(&req, &g_clientSvcIdentity);
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_REGISTER_DEV_STATE_CALLBACK, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    if (result == DM_OK) {
        DmMutexLock(&g_mirrorLock);
        g_regMirror.devStateRegistered = true;
        DmMutexUnlock(&g_mirrorLock);
        LOGW("[recover] devState marked for auto re-register pkgName=%s", DmStringCstr(pkgName));
    }
    return result;
}

int32_t DmImplUnRegisterDevStateCallback(const DmString* pkgName)
{
    if (pkgName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    DmMutexLock(&g_mirrorLock);
    g_regMirror.devStateRegistered = false;
    DmMutexUnlock(&g_mirrorLock);

    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_UNREGISTER_DEV_STATE_CALLBACK, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}

int32_t DmImplSetLocalDeviceName(const DmString* pkgName, const DmString* deviceName)
{
    if (pkgName == NULL || deviceName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (DmStringEmpty(deviceName)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char buf[DM_IPC_BUF_SIZE];
    IpcIo req;
    IpcIoInit(&req, buf, sizeof(buf), 0);
    WriteString(&req, DmStringCstr(pkgName));
    WriteString(&req, DmStringCstr(deviceName));
    char rbuf[DM_IPC_BUF_SIZE];
    IpcIo reply;
    IpcIoInit(&reply, rbuf, sizeof(rbuf), 0);
    int32_t ret = DmImplSendRequest(DM_SET_LOCAL_DEVICE_NAME, &req, &reply);
    if (ret != DM_OK) {
        return ret;
    }
    int32_t result = 0;
    ReadInt32(&reply, &result);
    return result;
}
