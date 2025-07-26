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

#ifndef OHOS_DM_CONSTANTS_H
#define OHOS_DM_CONSTANTS_H

#include <map>

#include "dm_error_type.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT

namespace OHOS {
namespace DistributedHardware {
DM_EXPORT extern const char* TAG_GROUP_ID;
DM_EXPORT extern const char* TAG_GROUP_NAME;
DM_EXPORT extern const char* TAG_REQUEST_ID;
DM_EXPORT extern const char* TAG_DEVICE_ID;
DM_EXPORT extern const char* TAG_AUTH_TYPE;
DM_EXPORT extern const char* TAG_CRYPTO_SUPPORT;
DM_EXPORT extern const char* TAG_VER;
DM_EXPORT extern const char* TAG_MSG_TYPE;
DM_EXPORT extern const char* DM_ITF_VER;
DM_EXPORT extern const char* DM_PKG_NAME;
DM_EXPORT extern const char* DM_SESSION_NAME;
DM_EXPORT extern const char* DM_PIN_HOLDER_SESSION_NAME;
DM_EXPORT extern const char* DM_SYNC_USERID_SESSION_NAME;
DM_EXPORT extern const char* DM_CAPABILITY_OSD;
DM_EXPORT extern const char* DM_CAPABILITY_APPROACH;
DM_EXPORT extern const char* DM_CAPABILITY_OH_APPROACH;
DM_EXPORT extern const char* DM_CAPABILITY_TOUCH;
DM_EXPORT extern const char* DM_CAPABILITY_CASTPLUS;
DM_EXPORT extern const char* DM_CAPABILITY_VIRTUAL_LINK;
DM_EXPORT extern const char* DM_CAPABILITY_SHARE;
DM_EXPORT extern const char* DM_CAPABILITY_WEAR;
DM_EXPORT extern const char* DM_CAPABILITY_OOP;
DM_EXPORT extern const char* DM_CREDENTIAL_TYPE;
DM_EXPORT extern const char* DM_CREDENTIAL_REQJSONSTR;
DM_EXPORT extern const char* DM_CREDENTIAL_RETURNJSONSTR;
DM_EXPORT extern const char* DEVICE_MANAGER_GROUPNAME;
DM_EXPORT extern const char* FIELD_CREDENTIAL_EXISTS;
DM_EXPORT extern const char* DM_TYPE_MINE;
DM_EXPORT extern const char* DM_TYPE_OH;
DM_EXPORT extern const char* TAG_SESSION_HEARTBEAT;
DM_EXPORT extern const char* TAG_BUNDLE_NAME;
DM_EXPORT extern const char* TAG_TOKENID;
DM_EXPORT extern const char* TAG_REMAINING_FROZEN_TIME;

//The following constant are provided only for HiLink.
DM_EXPORT extern const char *EXT_PART;

// Auth
DM_EXPORT extern const char* AUTH_TYPE;
DM_EXPORT extern const char* APP_OPERATION;
DM_EXPORT extern const char* CUSTOM_DESCRIPTION;
DM_EXPORT extern const char* TOKEN;
DM_EXPORT extern const char* PIN_TOKEN;
DM_EXPORT extern const char* PIN_CODE_KEY;
DM_EXPORT extern const int32_t CHECK_AUTH_ALWAYS_POS;
DM_EXPORT extern const char AUTH_ALWAYS;
DM_EXPORT extern const char AUTH_ONCE;
DM_EXPORT extern const char* TAG_TARGET_DEVICE_NAME;
DM_EXPORT extern const int32_t MIN_PINCODE_SIZE;
// HiChain
DM_EXPORT extern const int32_t SERVICE_INIT_TRY_MAX_NUM;
constexpr int32_t DEVICE_UUID_LENGTH = 65;
DM_EXPORT extern const int32_t DEVICE_NETWORKID_LENGTH;
DM_EXPORT extern const int32_t GROUP_TYPE_INVALID_GROUP;
DM_EXPORT extern const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
DM_EXPORT extern const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP;
DM_EXPORT extern const int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP;
DM_EXPORT extern const int32_t GROUP_VISIBILITY_PUBLIC;
DM_EXPORT extern const int64_t MIN_REQUEST_ID;
DM_EXPORT extern const int64_t MAX_REQUEST_ID;
DM_EXPORT extern const int32_t AUTH_DEVICE_REQ_NEGOTIATE;
DM_EXPORT extern const int32_t AUTH_DEVICE_RESP_NEGOTIATE;
DM_EXPORT extern const int32_t DEVICEID_LEN;

// Key of filter parameter
DM_EXPORT extern const char* FILTER_PARA_RANGE;
DM_EXPORT extern const char* FILTER_PARA_DEVICE_TYPE;
DM_EXPORT extern const char* FILTER_PARA_INCLUDE_TRUST;

// Connection address type
DM_EXPORT extern const char* CONN_ADDR_TYPE_ID;
DM_EXPORT extern const char* CONN_ADDR_TYPE_BR;
DM_EXPORT extern const char* CONN_ADDR_TYPE_BLE;
DM_EXPORT extern const char* CONN_ADDR_TYPE_USB;
DM_EXPORT extern const char* CONN_ADDR_TYPE_WLAN_IP;
DM_EXPORT extern const char* CONN_ADDR_TYPE_ETH_IP;
DM_EXPORT extern const char* CONN_ADDR_TYPE_NCM;

// Softbus connection address type int
DM_EXPORT extern const int32_t CONNECTION_ADDR_USB;

// Parameter Key
DM_EXPORT extern const char* PARAM_KEY_META_TYPE;
DM_EXPORT extern const char* PARAM_KEY_TARGET_ID;
DM_EXPORT extern const char* PARAM_KEY_BR_MAC;
DM_EXPORT extern const char* PARAM_KEY_BLE_MAC;
DM_EXPORT extern const char* PARAM_KEY_WIFI_IP;
DM_EXPORT extern const char* PARAM_KEY_WIFI_PORT;
DM_EXPORT extern const char* PARAM_KEY_USB_IP;
DM_EXPORT extern const char* PARAM_KEY_USB_PORT;
DM_EXPORT extern const char* PARAM_KEY_NCM_IP;
DM_EXPORT extern const char* PARAM_KEY_NCM_PORT;
DM_EXPORT extern const char* PARAM_KEY_ACCOUNT_HASH;
DM_EXPORT extern const char* PARAM_KEY_AUTH_TOKEN;
DM_EXPORT extern const char* PARAM_KEY_AUTH_TYPE;
DM_EXPORT extern const char* PARAM_KEY_PIN_CODE;
DM_EXPORT extern const char* PARAM_KEY_APP_OPER;
DM_EXPORT extern const char* PARAM_KEY_APP_DESC;
DM_EXPORT extern const char* PARAM_KEY_BLE_UDID_HASH;
constexpr const char* PARAM_KEY_CUSTOM_DATA = "CUSTOM_DATA";
DM_EXPORT extern const char* PARAM_KEY_CONN_ADDR_TYPE;
DM_EXPORT extern const char* PARAM_KEY_PUBLISH_ID;
DM_EXPORT extern const char* PARAM_KEY_SUBSCRIBE_ID;
DM_EXPORT extern const char* PARAM_KEY_TARGET_PKG_NAME;
DM_EXPORT extern const char* PARAM_KEY_PEER_BUNDLE_NAME;
DM_EXPORT extern const char* PARAM_KEY_DISC_FREQ;
DM_EXPORT extern const char* PARAM_KEY_DISC_MEDIUM;
DM_EXPORT extern const char* PARAM_KEY_DISC_CAPABILITY;
DM_EXPORT extern const char* PARAM_KEY_DISC_MODE;
DM_EXPORT extern const char* PARAM_KEY_AUTO_STOP_ADVERTISE;
DM_EXPORT extern const char* PARAM_KEY_FILTER_OPTIONS;
DM_EXPORT extern const char* PARAM_KEY_BIND_EXTRA_DATA;
DM_EXPORT extern const char* PARAM_KEY_OS_TYPE;
DM_EXPORT extern const char* PARAM_KEY_OS_VERSION;
DM_EXPORT extern const char* PARAM_KEY_IS_SHOW_TRUST_DIALOG;
DM_EXPORT extern const char* PARAM_KEY_UDID;
DM_EXPORT extern const char* PARAM_KEY_UUID;
DM_EXPORT extern const char* DM_CONNECTION_DISCONNECTED;
DM_EXPORT extern const char* BIND_LEVEL;
DM_EXPORT extern const char* TOKENID;
DM_EXPORT extern const char* DM_BIND_RESULT_NETWORK_ID;
DM_EXPORT extern const char* PARAM_KEY_POLICY_STRATEGY_FOR_BLE;
DM_EXPORT extern const char* PARAM_KEY_POLICY_TIME_OUT;
DM_EXPORT extern const char* DEVICE_SCREEN_STATUS;
DM_EXPORT extern const char* PROCESS_NAME;
DM_EXPORT extern const char* PARAM_CLOSE_SESSION_DELAY_SECONDS;
DM_EXPORT extern const char* DM_AUTHENTICATION_TYPE;

DM_EXPORT extern const char* PARAM_KEY_CONN_SESSIONTYPE;
DM_EXPORT extern const char* PARAM_KEY_HML_RELEASETIME;
DM_EXPORT extern const char* PARAM_KEY_HML_ENABLE_160M;
DM_EXPORT extern const char* PARAM_KEY_HML_ACTIONID;

DM_EXPORT extern const char* CONN_SESSION_TYPE_HML;
DM_EXPORT extern const char* CONN_SESSION_TYPE_BLE;
DM_EXPORT extern const char* UN_BIND_PARAM_UDID_KEY;
DM_EXPORT extern const char* PARAM_KEY_IS_PROXY_BIND;
DM_EXPORT extern const char* PARAM_KEY_IS_PROXY_UNBIND;
DM_EXPORT extern const char* PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT;
DM_EXPORT extern const char* PARAM_KEY_SUBJECT_PROXYED_SUBJECTS;
DM_EXPORT extern const char* DM_VAL_TRUE;
DM_EXPORT extern const char* DM_VAL_FALSE;
DM_EXPORT extern const char* APP_USER_DATA;
DM_EXPORT extern const char* BUNDLE_INFO;
DM_EXPORT extern const char* TITLE;
DM_EXPORT extern const char* DM_BUSINESS_ID;

// screen state
enum ScreenState {
    DM_SCREEN_UNKNOWN = -1,
    DM_SCREEN_ON = 0,
    DM_SCREEN_OFF = 1
};

// errCode map
DM_EXPORT extern const std::map<int32_t, int32_t> MAP_ERROR_CODE;
// wise device
DM_EXPORT extern const int32_t MAX_DEVICE_PROFILE_SIZE;
const int32_t DEVICE_NAME_MAX_BYTES = 100;

DM_EXPORT extern const char* ACL_IS_LNN_ACL_KEY;
DM_EXPORT extern const char* ACL_IS_LNN_ACL_VAL_TRUE;
DM_EXPORT extern const char* ACL_IS_LNN_ACL_VAL_FALSE;

extern const char* DM_VERSION_5_0_1;
extern const char* DM_VERSION_5_0_2;
extern const char* DM_VERSION_5_0_3;
extern const char* DM_VERSION_5_0_4;
extern const char* DM_VERSION_5_0_5;
extern const char* DM_VERSION_5_1_0;
extern const char* DM_VERSION_5_1_1;
extern const char* DM_VERSION_5_1_2;
extern const char* DM_VERSION_5_1_3;
extern const char* DM_CURRENT_VERSION;
extern const char* DM_ACL_AGING_VERSION;
extern const char* DM_VERSION_5_0_OLD_MAX;      // Estimated highest version number of the old version
extern const int32_t OLD_DM_HO_OSTYPE;
extern const int32_t NEW_DM_HO_OSTYPE;
extern const int32_t DM_OH_OSTYPE;
extern const char* PEER_UDID;
extern const char* PEER_OSTYPE;
extern const char* TIME_STAMP;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H
