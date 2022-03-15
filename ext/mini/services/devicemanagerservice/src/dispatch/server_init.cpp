/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <unistd.h>

#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "hichain_connector.h"
#include "server_stub.h"
#include "server_init.h"
#include "samgr_lite.h"
#include "softbus_adapter.h"

using namespace OHOS::DistributedHardware;

void Server_Init()
{
    const int32_t DM_SERVICE_INIT_DELAY = 2;

    sleep(DM_SERVICE_INIT_DELAY);
    SAMGR_Bootstrap();

    if (SoftbusAdapter::Init() != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "softbus adapter init failed");
        return;
    }
    if (HichainConnector::GetInstance().Init() != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "hichain connector init failed");
        return;
    }
 
    DMLOG(DM_LOG_INFO, "DM server Init success");
}
