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


#include "ipc_server_stub.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "device_manager_ipc_interface_code.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "dm_container.h"
#include "device_manager_service.h"

#include "samgr_lite.h"
#include "service.h"
#include "message.h"
#include "iproxy_server.h"
#include "iproxy_client.h"
#include "serializer.h"
#include "ipc_skeleton.h"

#define DM_SERVICE_NAME "dev_mgr_svc"
#define DM_IPC_BUF_SIZE 8192

static SvcIdentity g_dmSvcIdentity = {IPC_INVALID_HANDLE, 0, 0};
static IpcObjectStub g_dmStubObj;

static bool DmIsAsyncNotifyCmd(int32_t cmdCode)
{
    return cmdCode == DM_SERVER_DEVICE_FOUND ||
        cmdCode == DM_SERVER_DEVICE_DISCOVERY ||
        cmdCode == DM_SERVER_DISCOVER_FINISH ||
        cmdCode == DM_SERVER_PUBLISH_FINISH ||
        cmdCode == DM_SERVER_DEVICE_STATE_NOTIFY ||
        cmdCode == DM_SERVER_CREDENTIAL_RESULT ||
        cmdCode == DM_SERVER_DEVICE_FA_NOTIFY ||
        cmdCode == DM_SERVER_GET_DMFA_INFO ||
        cmdCode == DM_SERVER_DEVICE_SCREEN_STATE_NOTIFY ||
        cmdCode == DM_SERVER_SERVICE_STATE_NOTIFY ||
        cmdCode == DM_SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY ||
        cmdCode == DM_SERVICE_PUBLISH_RESULT ||
        cmdCode == DM_NOTIFY_SERVICE_FOUND ||
        cmdCode == DM_NOTIFY_SERVICE_DISCOVERY_RESULT ||
        cmdCode == DM_LEAVE_LNN_RESULT ||
        cmdCode == DM_SYNC_SERVICE_CALLBACK ||
        cmdCode == DM_SYNC_SERVICE_INFO_RESULT ||
        cmdCode == DM_GET_DEVICE_PROFILE_INFO_LIST_RESULT ||
        cmdCode == DM_GET_DEVICE_ICON_INFO_RESULT ||
        cmdCode == DM_SET_REMOTE_DEVICE_NAME_RESULT ||
        cmdCode == DM_SET_LOCAL_DEVICE_NAME_RESULT;
}

static int32_t DmServiceInvoke(IServerProxy* iProxy, int funcId, void* origin, IpcIo* req, IpcIo* reply)
{
    if (req == NULL || reply == NULL) {
        LOGE("DmServiceInvoke invalid para");
        return ERR_DM_FAILED;
    }
    int32_t cmdCode = funcId;
    if (cmdCode < 0 || cmdCode >= DM_IPC_MSG_BUTT) {
        LOGE("unsupported ipc cmd %d", cmdCode);
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    if (DmIsAsyncNotifyCmd(cmdCode)) {
        LOGI("async notify cmd %d from client, ignored on server", cmdCode);
        WriteInt32(reply, DM_OK);
        return DM_OK;
    }
    IpcServerStub* stub = IpcServerStubGetInstance();
    return DmIpcServerStubOnRemoteRequest(stub, cmdCode, req, reply, NULL);
}

static const char* DmGetName(Service* service)
{
    return DM_SERVICE_NAME;
}

static BOOL DmInitialize(Service* service, Identity identity)
{
    g_dmSvcIdentity.handle = identity.serviceId;
    g_dmSvcIdentity.token = identity.featureId;
    g_dmSvcIdentity.cookie = (uintptr_t)&g_dmStubObj;
    LOGI("DmService initialized, serviceId=%d", identity.serviceId);
    return TRUE;
}

static BOOL DmMessageHandle(Service* service, Request* request)
{
    return FALSE;
}

static TaskConfig DmGetTaskConfig(Service* service)
{
    TaskConfig config = {LEVEL_HIGH, PRI_BELOW_NORMAL, 0x4000, 20, SINGLE_TASK};
    return config;
}

typedef struct {
    INHERIT_SERVER_IPROXY;
} DefaultFeatureApi;

typedef struct {
    INHERIT_SERVICE;
    INHERIT_IUNKNOWNENTRY(DefaultFeatureApi);
    Identity identity;
} DmSamgrService;

static DmSamgrService g_dmSamgrService = {
    .GetName = DmGetName,
    .Initialize = DmInitialize,
    .MessageHandle = DmMessageHandle,
    .GetTaskConfig = DmGetTaskConfig,
    SERVER_IPROXY_IMPL_BEGIN,
    .Invoke = DmServiceInvoke,
    IPROXY_END,
};

static void DmSamgrInit(void)
{
    BOOL ret1 = SAMGR_GetInstance()->RegisterService((Service*)&g_dmSamgrService);
    LOGI("RegisterService ret=%d", ret1);
    BOOL ret2 = SAMGR_GetInstance()->RegisterDefaultFeatureApi(DM_SERVICE_NAME, GET_IUNKNOWN(g_dmSamgrService));
    LOGI("RegisterDefaultFeatureApi ret=%d name=%s", ret2, DM_SERVICE_NAME);
}

void DmIpcServiceInitLite(void)
{
    DmIpcServerStubOnStart(IpcServerStubGetInstance());
    DmLiteIpcCmdRegisterAll();
    DmSamgrInit();
    SAMGR_Bootstrap();
    DmServiceRegisterLite();
    LOGI("DM service registered with samgr_lite");
}
