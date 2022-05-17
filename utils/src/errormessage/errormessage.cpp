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

#include "errormessage/errormessage.h"

namespace OHOS {
namespace DistributedHardware {
ErrorMessage::ErrorMessage()
{
    LOGI("ErrorMessage construct.");
}

ErrorMessage::~ErrorMessage()
{
    LOGI("ErrorMessage Destructor.");
}

std::string ErrorMessage::GetErrorString(int failedReason)
{
    LOGI("Enter PermissionManager::GetErrorString");
    std::string tmpStr;
    switch (failedReason) {
        case ERR_DM_FAILED:
            tmpStr = DM_FAILED_INFO;
            break;
        case ERR_DM_TIME_OUT:
            tmpStr = DM_TIME_OUT_INFO;
            break;
        case ERR_DM_NOT_INIT:
            tmpStr = DM_NOT_INIT_INFO;
            break;
        case ERR_DM_INIT_REPEATED:
            tmpStr = DM_INIT_REPEATED_INFO;
            break;
        case ERR_DM_INIT_FAILED:
            tmpStr = DM_INIT_FAILED_INFO;
            break;
        case ERR_DM_UNINIT_FAILED:
            tmpStr = DM_UNINIT_FAILED_INFO;
            break;
        case ERR_DM_POINT_NULL:
            tmpStr = DM_POINT_NULL_INFO;
            break;
        case ERR_DM_INPUT_PARAMETER_EMPTY:
            tmpStr = DM_INPUT_PARAMETER_EMPTY_INFO;
            break;
        case ERR_DM_NO_PERMISSION:
            tmpStr = DM_NO_PERMISSION_INFO;
            break;
        case ERR_DM_MALLOC_FAILED:
            tmpStr = DM_MALLOC_FAILED_INFO;
            break;
        case ERR_DM_DISCOVERY_FAILED:
            tmpStr = DM_DISCOVERY_FAILED_INFO;
            break;
        case ERR_DM_MAP_KEY_ALREADY_EXISTS:
            tmpStr = DM_MAP_KEY_ALREADY_EXISTS_INFO;
            break;
        case DM_PROFILE_EVENTS_FAILED:
            tmpStr = DM_PROFILE_EVENTS_FAILED_INFO;
            break;
        case ERR_DM_IPC_WRITE_FAILED:
            tmpStr = DM_IPC_WRITE_FAILED_INFO;
            break;
        case ERR_DM_IPC_COPY_FAILED:
            tmpStr = DM_IPC_COPY_FAILED_INFO;
            break;
        case ERR_DM_IPC_SEND_REQUEST_FAILED:
            tmpStr = DM_IPC_SEND_REQUEST_FAILED_INFO;
            break;
        case ERR_DM_UNSUPPORTED_IPC_COMMAND:
            tmpStr = DM_UNSUPPORTED_IPC_COMMAND_INFO;
            break;
        case ERR_DM_IPC_RESPOND_FAILED:
            tmpStr = DM_IPC_RESPOND_FAILED_INFO;
            break;
        case ERR_DM_IPC_WRITE_TOKEN_FAILED:
            tmpStr = DM_IPC_WRITE_TOKEN_INFO;
            break;
        case ERR_DM_DISCOVERY_REPEATED:
            tmpStr = DM_DISCOVERY_REPEATED_INFO;
            break;
        case ERR_DM_UNSUPPORTED_AUTH_TYPE:
            tmpStr = DM_UNSUPPORTED_AUTH_TYPE_INFO;
            break;
        case ERR_DM_AUTH_OPEN_SESSION_FAILED:
            tmpStr = DM_AUTH_OPEN_SESSION_FAILED_INFO;
            break;
        case ERR_DM_AUTH_PEER_REJECT:
            tmpStr = DM_AUTH_PEER_REJECT_INFO;
            break;
        case ERR_DM_AUTH_REJECT:
            tmpStr = DM_AUTH_REJECT_INFO;
            break;
        case ERR_DM_AUTH_FAILED:
            tmpStr = DM_AUTH_FAILED_INFO;
            break;
        case ERR_DM_AUTH_NOT_START:
            tmpStr = DM_AUTH_NOT_START_INFO;
            break;
        case ERR_DM_AUTH_MESSAGE_INCOMPLETE:
            tmpStr = DM_AUTH_MESSAGE_INCOMPLETE_INFO;
            break;
        case ERR_DM_CREATE_GROUP_FAILED:
            tmpStr = DM_CREATE_GROUP_FAILED_INFO;
            break;
        default:
            tmpStr = DM_ERROR_CODE_DEFAULT_INFO;
            break;
    }
    return tmpStr;
}
} // namespace DistributedHardware
} // namespace OHOS
