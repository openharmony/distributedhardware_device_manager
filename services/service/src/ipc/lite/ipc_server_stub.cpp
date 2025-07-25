/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dm_constants.h"
#include "dm_log.h"
#include "dm_subscribe_info.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_server_listenermgr.h"
#include "iproxy_server.h"
#include "ipc_skeleton.h"
#include "ohos_init.h"
#include "samgr_lite.h"
#include "securec.h"

namespace {
const int32_t WAIT_FOR_SERVER = 2;
const int32_t STACK_SIZE = 0x1000;
const int32_t QUEUE_SIZE = 32;
const int32_t MALLOC_MAX_LEN = 2 * 1024 * 1024;
} // namespace

using namespace OHOS::DistributedHardware;

struct DefaultFeatureApi {
    INHERIT_SERVER_IPROXY;
};

struct DeviceManagerSamgrService {
    INHERIT_SERVICE;
    INHERIT_IUNKNOWNENTRY(DefaultFeatureApi);
    Identity identity;
};

static void DeathCb(void *arg)
{
    if (arg == nullptr) {
        LOGE("package name is NULL.");
        return;
    }
    CommonSvcId svcId = {0};
    std::string pkgName = reinterpret_cast<const char*>(arg);
    if (IpcServerListenermgr::GetInstance().GetListenerByPkgName(pkgName, &svcId) != DM_OK) {
        LOGE("not found client by package name.");
        free(arg);
        return;
    }
    IpcServerListenermgr::GetInstance().UnregisterListener(pkgName);
    free(arg);
    SvcIdentity sid = {0};
    sid.handle = svcId.handle;
    sid.token = svcId.token;
    sid.cookie = svcId.cookie;
    ReleaseSvc(sid);
}

int32_t RegisterDeviceManagerListener(IpcIo *req, IpcIo *reply)
{
    LOGI("register service listener.");
    size_t len = 0;
    uint8_t *name = ReadString(req, &len);
    SvcIdentity svc;
    bool ret = ReadRemoteObject(req, &svc);
    if (!ret || name == NULL || len == 0) {
        LOGE("get para failed");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    CommonSvcId svcId = {0};
    svcId.handle = svc.handle;
    svcId.token = svc.token;
    svcId.cookie = svc.cookie;

    if (len == 0 || len > MALLOC_MAX_LEN) {
        LOGE("malloc length invalid!");
        return ERR_DM_MALLOC_FAILED;
    }
    char *pkgName = new char[len+1];
    if (pkgName == nullptr) {
        LOGE("malloc failed!");
        return ERR_DM_MALLOC_FAILED;
    }
    if (strcpy_s(pkgName, len + 1, reinterpret_cast<const char*>(name)) != DM_OK) {
        LOGE("strcpy_s failed!");
        delete[] pkgName;
        return ERR_DM_IPC_COPY_FAILED;
    }
    uint32_t cbId = 0;
    AddDeathRecipient(svc, DeathCb, pkgName, &cbId);
    svcId.cbId = cbId;
    std::string strPkgName = reinterpret_cast<const char*>(name);
    delete[] pkgName;
    return IpcServerListenermgr::GetInstance().RegisterListener(strPkgName, &svcId);
}

int32_t UnRegisterDeviceManagerListener(IpcIo *req, IpcIo *reply)
{
    LOGI("unregister service listener.");
    size_t len = 0;
    std::string pkgName = reinterpret_cast<const char*>(ReadString(req, &len));
    if (pkgName == "" || len == 0) {
        LOGE("get para failed");
        return ERR_DM_FAILED;
    }
    CommonSvcId svcId;
    if (IpcServerListenermgr::GetInstance().GetListenerByPkgName(pkgName, &svcId) != DM_OK) {
        LOGE("not found listener by package name.");
        return ERR_DM_FAILED;
    }
    int32_t ret = IpcServerListenermgr::GetInstance().UnregisterListener(pkgName);
    if (ret == DM_OK) {
        SvcIdentity sid;
        sid.handle = svcId.handle;
        sid.token = svcId.token;
        sid.cookie = svcId.cookie;
        ReleaseSvc(sid);
    }
    return ret;
}

static const char *GetName(Service *service)
{
    (void)service;
    return DEVICE_MANAGER_SERVICE_NAME;
}

static BOOL Initialize(Service *service, Identity identity)
{
    if (service == nullptr) {
        LOGW("invalid param");
        return FALSE;
    }

    DeviceManagerSamgrService *mgrService = reinterpret_cast<DeviceManagerSamgrService *>(service);
    mgrService->identity = identity;
    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *request)
{
    if ((service == nullptr) || (request == nullptr)) {
        LOGW("invalid param");
        return FALSE;
    }
    return TRUE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    TaskConfig config = {LEVEL_HIGH, PRI_BELOW_NORMAL, STACK_SIZE, QUEUE_SIZE, SHARED_TASK};
    return config;
}

static int32_t OnRemoteRequestLite(IServerProxy *iProxy, int32_t funcId, void *origin, IpcIo *req, IpcIo *reply)
{
    LOGI("Receive funcId:%{public}d", funcId);
    (void)origin;
    return IpcCmdRegister::GetInstance().OnIpcServerCmd(funcId, *req, *reply);
}

static void HOS_SystemInit(void)
{
    SAMGR_Bootstrap();
    return;
}

int32_t IpcServerStubInit(void)
{
    HOS_SystemInit();
    return DM_OK;
}

static void DevMgrSvcInit(void)
{
    sleep(WAIT_FOR_SERVER);
    static DeviceManagerSamgrService service = {
        .GetName = GetName,
        .Initialize = Initialize,
        .MessageHandle = MessageHandle,
        .GetTaskConfig = GetTaskConfig,
        SERVER_IPROXY_IMPL_BEGIN,
        .Invoke = OnRemoteRequestLite,
        IPROXY_END,
    };

    if (!SAMGR_GetInstance()->RegisterService((Service *)&service)) {
        LOGE("%{public}s, RegisterService failed", DEVICE_MANAGER_SERVICE_NAME);
        return;
    }
    if (!SAMGR_GetInstance()->RegisterDefaultFeatureApi(DEVICE_MANAGER_SERVICE_NAME, GET_IUNKNOWN(service))) {
        LOGE("%{public}s, RegisterDefaultFeatureApi failed", DEVICE_MANAGER_SERVICE_NAME);
        return;
    }
    LOGI("%{public}s, init success", DEVICE_MANAGER_SERVICE_NAME);
}

SYSEX_SERVICE_INIT(DevMgrSvcInit);
