/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "server_stub.h"
#include "server_init.h"

#include "securec.h"


#include "ohos_init.h"
#include "samgr_lite.h"


#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "dm_subscribe_info.h"

#include "message_def.h"


namespace {
    const int32_t WAIT_FOR_SERVER = 2;
    const int32_t STACK_SIZE = 0x1000;
    const int32_t QUEUE_SIZE = 32;
}

using namespace OHOS::DistributedHardware;

struct DefaultFeatureApi {
//    INHERIT_SERVER_IPROXY;
};

struct DeviceManagerSamgrService {
    INHERIT_SERVICE;
    INHERIT_IUNKNOWNENTRY(DefaultFeatureApi);
    Identity identity;
};


static const char *GetName(Service *service)
{
    (void)service;
    return DEVICE_MANAGER_SERVICE_NAME;
}

static BOOL Initialize(Service *service, Identity identity)
{
    if (service == NULL) {
        DMLOG(DM_LOG_WARN, "invalid param");
        return FALSE;
    }

    Server_Init();

    DeviceManagerSamgrService *mgrService = (DeviceManagerSamgrService *)service;
    mgrService->identity = identity;
    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *request)
{
    if ((service == NULL) || (request == NULL)) {
        DMLOG(DM_LOG_WARN, "invalid param");
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




static void DevMgrSvcInit(void)
{
    sleep(WAIT_FOR_SERVER);
    static DeviceManagerSamgrService service = {
        .GetName = GetName,
        .Initialize = Initialize,
        .MessageHandle = MessageHandle,
        .GetTaskConfig = GetTaskConfig,
        // SERVER_IPROXY_IMPL_BEGIN,
        // .Invoke = OnRemoteRequest,
        // IPROXY_END,
    };

    if (!SAMGR_GetInstance()->RegisterService((Service *)&service)) {
        DMLOG(DM_LOG_ERROR, "%s, RegisterService failed", DEVICE_MANAGER_SERVICE_NAME);
        return;
    }
    if (!SAMGR_GetInstance()->RegisterDefaultFeatureApi(DEVICE_MANAGER_SERVICE_NAME, GET_IUNKNOWN(service))) {
        DMLOG(DM_LOG_ERROR, "%s, RegisterDefaultFeatureApi failed", DEVICE_MANAGER_SERVICE_NAME);
        return;
    }
    DMLOG(DM_LOG_INFO, "%s, init success", DEVICE_MANAGER_SERVICE_NAME);
}
SYSEX_SERVICE_INIT(DevMgrSvcInit);
