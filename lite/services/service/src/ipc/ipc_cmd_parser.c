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


#include "device_manager_ipc_interface_code.h"
#include "device_manager_service.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "ipc_cmd_register.h"
#include "ipc_server_stub.h"

static int32_t DmIpcCmdParserOnIpcCmd(int32_t cmdCode, void* data, void* reply)
{
    LOGI("start ipcCode %d.", cmdCode);
    return DmIpcCmdRegisterOnIpcCmd(cmdCode, data, reply);
}

void DmIpcCmdParserRegisterAll(void)
{
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_ACCESS_CONTROL, DmIpcCmdParserOnIpcCmd);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SAME_ACCOUNT, DmIpcCmdParserOnIpcCmd);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SRC_ACCESS_CONTROL, DmIpcCmdParserOnIpcCmd);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SINK_ACCESS_CONTROL, DmIpcCmdParserOnIpcCmd);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SRC_SAME_ACCOUNT, DmIpcCmdParserOnIpcCmd);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SINK_SAME_ACCOUNT, DmIpcCmdParserOnIpcCmd);
}
