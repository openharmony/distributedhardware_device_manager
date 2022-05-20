/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dm_error_message.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct ERROR_INFO
{
    int errCode;
    std::string errMsg;
}ERROR_INFO;

static ERROR_INFO g_errorMessages[] =
{
    {ERR_DM_FAILED, DM_FAILED_INFO},
    {ERR_DM_TIME_OUT, DM_TIME_OUT_INFO},
    {ERR_DM_NOT_INIT, DM_NOT_INIT_INFO},
    {ERR_DM_INIT_REPEATED, DM_INIT_REPEATED_INFO},
    {ERR_DM_INIT_FAILED, DM_INIT_FAILED_INFO},
    {ERR_DM_UNINIT_FAILED, DM_UNINIT_FAILED_INFO},
    {ERR_DM_POINT_NULL, DM_POINT_NULL_INFO},
    {ERR_DM_INPUT_PARAMETER_EMPTY, DM_INPUT_PARAMETER_EMPTY_INFO},
    {ERR_DM_NO_PERMISSION, DM_NO_PERMISSION_INFO},
    {ERR_DM_MALLOC_FAILED, DM_MALLOC_FAILED_INFO},
    {ERR_DM_DISCOVERY_FAILED, DM_DISCOVERY_FAILED_INFO},
    {ERR_DM_MAP_KEY_ALREADY_EXISTS, DM_MAP_KEY_ALREADY_EXISTS_INFO},
    {DM_PROFILE_EVENTS_FAILED, DM_PROFILE_EVENTS_FAILED_INFO},
    {ERR_DM_IPC_WRITE_FAILED, DM_IPC_WRITE_FAILED_INFO},
    {ERR_DM_IPC_COPY_FAILED, DM_IPC_COPY_FAILED_INFO},
    {ERR_DM_IPC_SEND_REQUEST_FAILED, DM_IPC_SEND_REQUEST_FAILED_INFO},
    {ERR_DM_UNSUPPORTED_IPC_COMMAND, DM_UNSUPPORTED_IPC_COMMAND_INFO},
    {ERR_DM_IPC_RESPOND_FAILED, DM_IPC_RESPOND_FAILED_INFO},
    {ERR_DM_IPC_WRITE_TOKEN_FAILED, DM_IPC_WRITE_TOKEN_INFO},
    {ERR_DM_DISCOVERY_REPEATED, DM_DISCOVERY_REPEATED_INFO},
    {ERR_DM_UNSUPPORTED_AUTH_TYPE, DM_UNSUPPORTED_AUTH_TYPE_INFO},
    {ERR_DM_AUTH_BUSINESS_BUSY, DM_AUTH_BUSINESS_BUSY_INFO},
    {ERR_DM_AUTH_OPEN_SESSION_FAILED, DM_AUTH_OPEN_SESSION_FAILED_INFO},
    {ERR_DM_AUTH_PEER_REJECT, DM_AUTH_PEER_REJECT_INFO},
    {ERR_DM_AUTH_REJECT, DM_AUTH_REJECT_INFO},
    {ERR_DM_AUTH_FAILED, DM_AUTH_FAILED_INFO},
    {ERR_DM_AUTH_NOT_START, DM_AUTH_NOT_START_INFO},
    {ERR_DM_AUTH_MESSAGE_INCOMPLETE, DM_AUTH_MESSAGE_INCOMPLETE_INFO},
    {ERR_DM_CREATE_GROUP_FAILED, DM_CREATE_GROUP_FAILED_INFO},
};

std::string GetErrorString(int failedReason)
{
    std::string g_errorMessage = "dm process execution failed.";
    for (int i =0; i<sizeof(g_errorMessages);i++)
    {
        if(failedReason == g_errorMessages[i].errCode) {
            g_errorMessage = g_errorMessages[i].errMsg;
        }
    }
    return g_errorMessage;
}
} // namespace DistributedHardware
} // namespace OHOS
