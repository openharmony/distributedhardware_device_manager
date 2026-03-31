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

#ifndef OHOS_DM_CONSTANTS_3RD_H
#define OHOS_DM_CONSTANTS_3RD_H

#include "dm_error_type_3rd.h"
namespace OHOS {
namespace DistributedHardware {
namespace {
// JSON keys for authentication messages
const char* TAG_DEVICE_ID_HASH = "deviceIdHash";
const char* TAG_USER_ID = "userId";
const char* TAG_ACCOUNT_ID_HASH = "accountIdHash";
const char* TAG_TOKEN_ID_HASH = "tokenIdHash";
const char* TAG_BUSINESS_NAME = "businessName";
const char* TAG_PROCESS_NAME = "processName";
const char* TAG_UID = "uid";
const char* TAG_VERSION = "version";
const char* TAG_PEER_BUSINESS_NAME = "peerBusinessName";
const char* TAG_PEER_PROCESS_NAME = "peerProcessName";
const char* TAG_DEVICE_ID = "deviceId";
const char* TAG_ACCOUNT_ID = "accountId";
const char* TAG_TOKEN_ID = "tokenId";
const char* TAG_REPLY = "reply";
const char* TAG_REASON = "reason";
const char* TAG_DATA = "data";
const char* TAG_SYNC = "sync";
const char* TAG_MSG_TYPE = "msgType";
const char* TAG_PROXY = "proxy";
const char* TAG_PROXY_TOKEN_ID = "proxyTokenId";
const char* TAG_PROXY_PROCESS_NAME = "proxyProcessName";
const char* TAG_PROXY_UID = "proxyUid";
const char* TAG_IS_PROXY_DELETE = "isProxyDelete";

const int32_t MIN_PINCODE_SIZE = 6;
const char* DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
const char* DM_VERSION_6_1_0 = "6.1.0";
const char* DM_CURRENT_VERSION = DM_VERSION_6_1_0;

const char* PARAM_KEY_IS_PROXY_BIND = "isProxyBind";
const char* PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT = "isCallingProxyAsSubject";
const char* PARAM_KEY_SUBJECT_PROXYED_SUBJECTS = "subjectProxyOnes";
const char* TAG_PROXY_CONTEXT_ID = "proxyContextId";
const char* TAG_DEVICE_VERSION = "deviceVersion";
const char* TAG_NETWORKID_ID = "networkId";
const char* TAG_BIND_LEVEL = "bindLevel";
constexpr int32_t DEVICE_UUID_LENGTH = 65;
const char* DM_VAL_TRUE = "true";
const char* DM_VAL_FALSE = "false";
const char* ACL_PREFIX = "acl_";
const char* TAG_SESSIONKEY = "sessionKey";
const char* TAG_SESSIONKEYS = "sessionKeys";

const char* TAG_BIND_CALLER_USERID = "bindCallerUserId";
const char* TAG_BIND_CALLER_TOKENID = "bindCallerTokenId";
const char* TAG_BIND_CALLER_UID = "bindCallerUid";
const char* TAG_BIND_CALLER_PROCESSNAME = "bindCallerProcessName";
const char* TAG_BIND_CALLER_IS_SYSTEM_SA = "bindCallerIsSystemSA";
const char* TAG_BIND_CALLER_BIND_LEVEL = "bindCallerBindLevel";
}
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_3RD_H
