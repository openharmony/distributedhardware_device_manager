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

#ifndef EXPORT
#define EXPORT __attribute__ ((visibility ("default")))
#endif // EXPORT

namespace OHOS {
namespace DistributedHardware {
EXPORT extern const char* TAG_GROUP_ID;
EXPORT extern const char* TAG_GROUP_NAME;
EXPORT extern const char* TAG_REQUEST_ID;
EXPORT extern const char* TAG_DEVICE_ID;
EXPORT extern const char* TAG_AUTH_TYPE;
EXPORT extern const char* TAG_CRYPTO_SUPPORT;
EXPORT extern const char* TAG_VER;
EXPORT extern const char* TAG_MSG_TYPE;
EXPORT extern const char* DM_ITF_VER;
EXPORT extern const char* DM_PKG_NAME;
EXPORT extern const char* DM_SESSION_NAME;
EXPORT extern const char* DM_PIN_HOLDER_SESSION_NAME;
EXPORT extern const char* DM_SYNC_USERID_SESSION_NAME;
EXPORT extern const char* DM_CAPABILITY_OSD;
EXPORT extern const char* DM_CAPABILITY_APPROACH;
EXPORT extern const char* DM_CAPABILITY_OH_APPROACH;
EXPORT extern const char* DM_CAPABILITY_TOUCH;
EXPORT extern const char* DM_CAPABILITY_CASTPLUS;
EXPORT extern const char* DM_CAPABILITY_VIRTUAL_LINK;
EXPORT extern const char* DM_CAPABILITY_SHARE;
EXPORT extern const char* DM_CAPABILITY_WEAR;
EXPORT extern const char* DM_CAPABILITY_OOP;
EXPORT extern const char* DM_CREDENTIAL_TYPE;
EXPORT extern const char* DM_CREDENTIAL_REQJSONSTR;
EXPORT extern const char* DM_CREDENTIAL_RETURNJSONSTR;
EXPORT extern const char* DEVICE_MANAGER_GROUPNAME;
EXPORT extern const char* FIELD_CREDENTIAL_EXISTS;
EXPORT extern const char* DM_TYPE_MINE;
EXPORT extern const char* DM_TYPE_OH;
EXPORT extern const char* TAG_SESSION_HEARTBEAT;

//The following constant are provided only for HiLink.
EXPORT extern const char *EXT_PART;

// Auth
EXPORT extern const char* AUTH_TYPE;
EXPORT extern const char* APP_OPERATION;
EXPORT extern const char* CUSTOM_DESCRIPTION;
EXPORT extern const char* TOKEN;
EXPORT extern const char* PIN_TOKEN;
EXPORT extern const char* PIN_CODE_KEY;
EXPORT extern const int32_t CHECK_AUTH_ALWAYS_POS;
EXPORT extern const char AUTH_ALWAYS;
EXPORT extern const char AUTH_ONCE;
EXPORT extern const char* TAG_TARGET_DEVICE_NAME;
EXPORT extern const int32_t INVALID_PINCODE;

// HiChain
EXPORT extern const int32_t SERVICE_INIT_TRY_MAX_NUM;
constexpr int32_t DEVICE_UUID_LENGTH = 65;
EXPORT extern const int32_t DEVICE_NETWORKID_LENGTH;
EXPORT extern const int32_t GROUP_TYPE_INVALID_GROUP;
EXPORT extern const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
EXPORT extern const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP;
EXPORT extern const int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP;
EXPORT extern const int32_t GROUP_VISIBILITY_PUBLIC;
EXPORT extern const int64_t MIN_REQUEST_ID;
EXPORT extern const int64_t MAX_REQUEST_ID;
EXPORT extern const int32_t AUTH_DEVICE_REQ_NEGOTIATE;
EXPORT extern const int32_t AUTH_DEVICE_RESP_NEGOTIATE;
EXPORT extern const int32_t DEVICEID_LEN;

// Key of filter parameter
EXPORT extern const char* FILTER_PARA_RANGE;
EXPORT extern const char* FILTER_PARA_DEVICE_TYPE;
EXPORT extern const char* FILTER_PARA_INCLUDE_TRUST;

// Connection address type
EXPORT extern const char* CONN_ADDR_TYPE_ID;
EXPORT extern const char* CONN_ADDR_TYPE_BR;
EXPORT extern const char* CONN_ADDR_TYPE_BLE;
EXPORT extern const char* CONN_ADDR_TYPE_USB;
EXPORT extern const char* CONN_ADDR_TYPE_WLAN_IP;
EXPORT extern const char* CONN_ADDR_TYPE_ETH_IP;

// Softbus connection address type int
EXPORT extern const int32_t CONNECTION_ADDR_USB;

// Parameter Key
EXPORT extern const char* PARAM_KEY_META_TYPE;
EXPORT extern const char* PARAM_KEY_TARGET_ID;
EXPORT extern const char* PARAM_KEY_BR_MAC;
EXPORT extern const char* PARAM_KEY_BLE_MAC;
EXPORT extern const char* PARAM_KEY_WIFI_IP;
EXPORT extern const char* PARAM_KEY_WIFI_PORT;
EXPORT extern const char* PARAM_KEY_USB_IP;
EXPORT extern const char* PARAM_KEY_USB_PORT;
EXPORT extern const char* PARAM_KEY_AUTH_TOKEN;
EXPORT extern const char* PARAM_KEY_AUTH_TYPE;
EXPORT extern const char* PARAM_KEY_PIN_CODE;
EXPORT extern const char* PARAM_KEY_APP_OPER;
EXPORT extern const char* PARAM_KEY_APP_DESC;
EXPORT extern const char* PARAM_KEY_BLE_UDID_HASH;
constexpr const char* PARAM_KEY_CUSTOM_DATA = "CUSTOM_DATA";
EXPORT extern const char* PARAM_KEY_CONN_ADDR_TYPE;
EXPORT extern const char* PARAM_KEY_PUBLISH_ID;
EXPORT extern const char* PARAM_KEY_SUBSCRIBE_ID;
EXPORT extern const char* PARAM_KEY_TARGET_PKG_NAME;
EXPORT extern const char* PARAM_KEY_PEER_BUNDLE_NAME;
EXPORT extern const char* PARAM_KEY_DISC_FREQ;
EXPORT extern const char* PARAM_KEY_DISC_MEDIUM;
EXPORT extern const char* PARAM_KEY_DISC_CAPABILITY;
EXPORT extern const char* PARAM_KEY_DISC_MODE;
EXPORT extern const char* PARAM_KEY_AUTO_STOP_ADVERTISE;
EXPORT extern const char* PARAM_KEY_FILTER_OPTIONS;
EXPORT extern const char* PARAM_KEY_BIND_EXTRA_DATA;
EXPORT extern const char* PARAM_KEY_OS_TYPE;
EXPORT extern const char* PARAM_KEY_OS_VERSION;
EXPORT extern const char* PARAM_KEY_IS_SHOW_TRUST_DIALOG;
EXPORT extern const char* PARAM_KEY_UDID;
EXPORT extern const char* PARAM_KEY_UUID;
EXPORT extern const char* DM_CONNECTION_DISCONNECTED;
EXPORT extern const char* BIND_LEVEL;
EXPORT extern const char* TOKENID;
EXPORT extern const char* DM_BIND_RESULT_NETWORK_ID;
EXPORT extern const char* PARAM_KEY_POLICY_STRATEGY_FOR_BLE;
EXPORT extern const char* PARAM_KEY_POLICY_TIME_OUT;
EXPORT extern const char* DEVICE_SCREEN_STATUS;
EXPORT extern const char* PROCESS_NAME;
EXPORT extern const char* PARAM_CLOSE_SESSION_DELAY_SECONDS;
EXPORT extern const char* DM_AUTHENTICATION_TYPE;

EXPORT extern const char* PARAM_KEY_CONN_SESSIONTYPE;
EXPORT extern const char* PARAM_KEY_HML_RELEASETIME;
EXPORT extern const char* PARAM_KEY_HML_ENABLE_160M;
EXPORT extern const char* PARAM_KEY_HML_ACTIONID;

EXPORT extern const char* CONN_SESSION_TYPE_HML;
EXPORT extern const char* CONN_SESSION_TYPE_BLE;
EXPORT extern const char* UN_BIND_PARAM_UDID_KEY;

// screen state
enum ScreenState {
    DM_SCREEN_UNKNOWN = -1,
    DM_SCREEN_ON = 0,
    DM_SCREEN_OFF = 1
};

// errCode map
EXPORT extern const std::map<int32_t, int32_t> MAP_ERROR_CODE;
// wise device
EXPORT extern const int32_t MAX_DEVICE_PROFILE_SIZE;
const int32_t DEIVCE_NAME_MAX_BYTES = 100;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H