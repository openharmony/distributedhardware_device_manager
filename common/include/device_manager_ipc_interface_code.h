/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IPC_INTERFACE_CODE_H
#define OHOS_DEVICE_MANAGER_IPC_INTERFACE_CODE_H

#include <cstdint>

/* SAID: 4802 */
namespace OHOS {
namespace DistributedHardware {
enum DMIpcCmdInterfaceCode {
    REGISTER_DEVICE_MANAGER_LISTENER = 0,
    UNREGISTER_DEVICE_MANAGER_LISTENER,
    GET_TRUST_DEVICE_LIST,
    GET_LOCAL_DEVICE_INFO,
    GET_UDID_BY_NETWORK,
    GET_UUID_BY_NETWORK,
    START_DEVICE_DISCOVER,
    STOP_DEVICE_DISCOVER,
    PUBLISH_DEVICE_DISCOVER,
    UNPUBLISH_DEVICE_DISCOVER,
    AUTHENTICATE_DEVICE,
    UNAUTHENTICATE_DEVICE,
    VERIFY_AUTHENTICATION,
    SERVER_DEVICE_STATE_NOTIFY,
    SERVER_DEVICE_FOUND,
    SERVER_DISCOVER_FINISH,
    SERVER_PUBLISH_FINISH,
    SERVER_AUTH_RESULT,
    SERVER_VERIFY_AUTH_RESULT,
    SERVER_GET_DMFA_INFO,
    SERVER_USER_AUTH_OPERATION,
    SERVER_DEVICE_FA_NOTIFY,
    SERVER_CREDENTIAL_RESULT,
    REGISTER_DEV_STATE_CALLBACK,
    UNREGISTER_DEV_STATE_CALLBACK,
    REQUEST_CREDENTIAL,
    IMPORT_CREDENTIAL,
    DELETE_CREDENTIAL,
    REGISTER_CREDENTIAL_CALLBACK,
    UNREGISTER_CREDENTIAL_CALLBACK,
    NOTIFY_EVENT,
    GET_DEVICE_INFO,
    GET_ENCRYPTED_UUID_BY_NETWOEKID,
    GENERATE_ENCRYPTED_UUID,
    CHECK_API_ACCESS_PRIMISSION,
    // Add ipc msg here
    IPC_MSG_BUTT
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
