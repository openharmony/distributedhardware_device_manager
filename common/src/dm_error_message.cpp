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

#include "dm_error_type.h"  // for ERR_DM_AUTH_BUSINESS_BUSY, ERR_DM_AUTH_FAILED

namespace OHOS {
namespace DistributedHardware {
typedef struct ERROR_INFO {
    int errCode;
    std::string errMsg;
} ERROR_INFO;

ERROR_INFO g_errorMessages[] = {
    {ERR_DM_FAILED, "dm process execution failed..."},
    {ERR_DM_TIME_OUT, "dm process execution timeout."},
    {ERR_DM_NOT_INIT, "dm service is not initialized, please try again later."},
    {ERR_DM_INIT_FAILED, "dm service initialize failed."},
    {ERR_DM_POINT_NULL, "dm service null pointer exception occurred."},
    {ERR_DM_INPUT_PARA_INVALID, "the function call input para is invalid."},
    {ERR_DM_NO_PERMISSION, "no permission for function call."},
    {ERR_DM_MALLOC_FAILED, "memory allocation failed."},
    {ERR_DM_DISCOVERY_FAILED, "device discovery failed."},
    {ERR_DM_MAP_KEY_ALREADY_EXISTS, "map key already exists."},
    {ERR_DM_IPC_WRITE_FAILED, "ipc write object failed."},
    {ERR_DM_IPC_COPY_FAILED, "ipc copy data failed."},
    {ERR_DM_IPC_SEND_REQUEST_FAILED, "ipc send request failed."},
    {ERR_DM_UNSUPPORTED_IPC_COMMAND, "ipc command not supported."},
    {ERR_DM_IPC_RESPOND_FAILED, "ipc process failed to receive response."},
    {ERR_DM_DISCOVERY_REPEATED, "repeat device discovery warning."},
    {ERR_DM_UNSUPPORTED_AUTH_TYPE, "auth type not supported."},
    {ERR_DM_AUTH_BUSINESS_BUSY, "authentication service is busy."},
    {ERR_DM_AUTH_OPEN_SESSION_FAILED, "open auth session failed."},
    {ERR_DM_AUTH_PEER_REJECT, "remote device refused to authorization."},
    {ERR_DM_AUTH_REJECT, "local device refused to authorization."},
    {ERR_DM_AUTH_FAILED, "authentication failed."},
    {ERR_DM_AUTH_NOT_START, "auth process not started."},
    {ERR_DM_AUTH_MESSAGE_INCOMPLETE, "authentication message is incomplete."},
    {ERR_DM_CREATE_GROUP_FAILED, "create group failed."},
    {ERR_DM_IPC_READ_FAILED, "ipc read object failed."},
    {ERR_DM_PUBLISH_FAILED, "device publish failed."},
    {ERR_DM_PUBLISH_REPEATED, "repeat device publish warning."},
    {DM_BIND_TRUST_TARGET, "bind to trusted target device."},
    {DM_ALREADY_AUTHED, "devices already authenticated."},
    {ERR_DM_BIND_USER_CANCEL, "user canceled the bind operation."},
    {ERR_DM_CAPABILITY_NEGOTIATE_FAILED, "capability negotiate failed."},
    {ERR_DM_SHOW_CONFIRM_FAILED, "show confirm dialog failed."},
    {ERR_DM_ANTI_DISTURB_MODE, "the sink is in anti-disturbance mode."},
    {ERR_DM_VERSION_INCOMPATIBLE, "the device version is incompatible."},
    {ERR_DM_BIND_PIN_CODE_ERROR, "bind failed due to pin code error."},
    {ERR_DM_BIND_USER_CANCEL_ERROR, "user canceled the pin code input."},
    {STOP_BIND, "stop bind operation."},
    {ERR_DM_SESSION_CLOSED, "the session is closed."},
    {ERR_DM_PARSE_MESSAGE_FAILED, "parse message failed."},
    {ERR_DM_PEER_CONFIRM_TIME_OUT, "peer confirm timeout."},
    {ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY, "user canceled the pin code display."},
    {ERR_DM_SKIP_AUTHENTICATE, "skip authenticate."},
};

std::string GetErrorString(int failedReason)
{
    std::string errorMessage = "undefined error code.";
    for (uint32_t i = 0; i < (sizeof(g_errorMessages) / sizeof(g_errorMessages[0])); i++) {
        if (failedReason == g_errorMessages[i].errCode) {
            errorMessage = g_errorMessages[i].errMsg;
            break;
        }
    }
    return errorMessage;
}
} // namespace DistributedHardware
} // namespace OHOS
