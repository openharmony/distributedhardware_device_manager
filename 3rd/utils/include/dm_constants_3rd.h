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

#include <cstdint>
#include "dm_error_type_3rd.h"
namespace OHOS {
namespace DistributedHardware {
// Tag
extern const char* TAG_DEVICE_ID_HASH;
extern const char* TAG_USER_ID;
extern const char* TAG_ACCOUNT_ID_HASH;
extern const char* TAG_TOKEN_ID_HASH;
extern const char* TAG_BUSINESS_NAME;
extern const char* TAG_PROCESS_NAME;
extern const char* TAG_UID;
extern const char* TAG_VERSION;
extern const char* TAG_PEER_BUSINESS_NAME;
extern const char* TAG_PEER_PROCESS_NAME;
extern const char* TAG_DEVICE_ID;
extern const char* TAG_ACCOUNT_ID;
extern const char* TAG_TOKEN_ID;
extern const char* TAG_REPLY;
extern const char* TAG_REASON;
extern const char* TAG_DATA;
extern const char* TAG_SYNC;
extern const char* TAG_MSG_TYPE;
extern const char* TAG_NETWORKID_ID;
extern const char* TAG_BIND_LEVEL;
extern const char* TAG_DEVICE_VERSION;
extern const char* TAG_SESSIONKEY;
extern const char* TAG_SESSIONKEYS;

// Auth
extern const int32_t MIN_PINCODE_SIZE;

// DM
extern const char* DM_PKG_NAME;
extern const char* DM_VAL_TRUE;
extern const char* DM_VAL_FALSE;

// ACL
extern const char* ACL_PREFIX;

// Version
extern const char* DM_VERSION_6_1_0;
extern const char* DM_CURRENT_VERSION;

// Proxy
extern const char* TAG_PROXY;
extern const char* TAG_PROXY_TOKEN_ID;
extern const char* TAG_PROXY_PROCESS_NAME;
extern const char* TAG_PROXY_UID;
extern const char* TAG_IS_PROXY_DELETE;
extern const char* TAG_PROXY_CONTEXT_ID;
extern const char* PARAM_KEY_IS_PROXY_BIND;
extern const char* PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT;
extern const char* PARAM_KEY_SUBJECT_PROXYED_SUBJECTS;

// Bind caller
extern const char* TAG_BIND_CALLER_USERID;
extern const char* TAG_BIND_CALLER_TOKENID;
extern const char* TAG_BIND_CALLER_UID;
extern const char* TAG_BIND_CALLER_PROCESSNAME;
extern const char* TAG_BIND_CALLER_IS_SYSTEM_SA;
extern const char* TAG_BIND_CALLER_BIND_LEVEL;

// Device
constexpr int32_t DEVICE_UUID_LENGTH = 65;
constexpr int32_t DM_POINT_TO_POINT = 256;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_3RD_H
