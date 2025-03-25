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

namespace OHOS {
namespace DistributedHardware {
__attribute__ ((visibility ("default")))extern const char* TAG_GROUP_ID;
__attribute__ ((visibility ("default")))extern const char* TAG_GROUP_NAME;
__attribute__ ((visibility ("default")))extern const char* TAG_REQUEST_ID;
__attribute__ ((visibility ("default")))extern const char* TAG_DEVICE_ID;
__attribute__ ((visibility ("default")))extern const char* TAG_AUTH_TYPE;
__attribute__ ((visibility ("default")))extern const char* TAG_CRYPTO_SUPPORT;
__attribute__ ((visibility ("default")))extern const char* TAG_VER;
__attribute__ ((visibility ("default")))extern const char* TAG_MSG_TYPE;
__attribute__ ((visibility ("default")))extern const char* DM_ITF_VER;
__attribute__ ((visibility ("default")))extern const char* DM_PKG_NAME;
__attribute__ ((visibility ("default")))extern const char* DM_SESSION_NAME;
__attribute__ ((visibility ("default")))extern const char* DM_PIN_HOLDER_SESSION_NAME;
__attribute__ ((visibility ("default")))extern const char* DM_SYNC_USERID_SESSION_NAME;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_OSD;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_APPROACH;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_TOUCH;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_CASTPLUS;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_VIRTUAL_LINK;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_SHARE;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_WEAR;
__attribute__ ((visibility ("default")))extern const char* DM_CAPABILITY_OOP;
__attribute__ ((visibility ("default")))extern const char* DM_CREDENTIAL_TYPE;
__attribute__ ((visibility ("default")))extern const char* DM_CREDENTIAL_REQJSONSTR;
__attribute__ ((visibility ("default")))extern const char* DM_CREDENTIAL_RETURNJSONSTR;
__attribute__ ((visibility ("default")))extern const char* DEVICE_MANAGER_GROUPNAME;
__attribute__ ((visibility ("default")))extern const char* FIELD_CREDENTIAL_EXISTS;
__attribute__ ((visibility ("default")))extern const char* DM_TYPE_MINE;
__attribute__ ((visibility ("default")))extern const char* DM_TYPE_OH;
__attribute__ ((visibility ("default")))extern const char* TAG_SESSION_HEARTBEAT;

//The following constant are provided only for HiLink.
__attribute__ ((visibility ("default")))extern const char *EXT_PART;

// Auth
__attribute__ ((visibility ("default")))extern const char* AUTH_TYPE;
__attribute__ ((visibility ("default")))extern const char* APP_OPERATION;
__attribute__ ((visibility ("default")))extern const char* CUSTOM_DESCRIPTION;
__attribute__ ((visibility ("default")))extern const char* TOKEN;
__attribute__ ((visibility ("default")))extern const char* PIN_TOKEN;
__attribute__ ((visibility ("default")))extern const char* PIN_CODE_KEY;
__attribute__ ((visibility ("default")))extern const int32_t CHECK_AUTH_ALWAYS_POS;
__attribute__ ((visibility ("default")))extern const char AUTH_ALWAYS;
__attribute__ ((visibility ("default")))extern const char AUTH_ONCE;
__attribute__ ((visibility ("default")))extern const char* TAG_TARGET_DEVICE_NAME;
__attribute__ ((visibility ("default")))extern const int32_t INVALID_PINCODE;

// HiChain
__attribute__ ((visibility ("default")))extern const int32_t SERVICE_INIT_TRY_MAX_NUM;
constexpr int32_t DEVICE_UUID_LENGTH = 65;
__attribute__ ((visibility ("default")))extern const int32_t DEVICE_NETWORKID_LENGTH;
__attribute__ ((visibility ("default")))extern const int32_t GROUP_TYPE_INVALID_GROUP;
__attribute__ ((visibility ("default")))extern const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
__attribute__ ((visibility ("default")))extern const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP;
__attribute__ ((visibility ("default")))extern const int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP;
__attribute__ ((visibility ("default")))extern const int32_t GROUP_VISIBILITY_PUBLIC;
__attribute__ ((visibility ("default")))extern const int64_t MIN_REQUEST_ID;
__attribute__ ((visibility ("default")))extern const int64_t MAX_REQUEST_ID;
__attribute__ ((visibility ("default")))extern const int32_t AUTH_DEVICE_REQ_NEGOTIATE;
__attribute__ ((visibility ("default")))extern const int32_t AUTH_DEVICE_RESP_NEGOTIATE;
__attribute__ ((visibility ("default")))extern const int32_t DEVICEID_LEN;

// Key of filter parameter
__attribute__ ((visibility ("default")))extern const char* FILTER_PARA_RANGE;
__attribute__ ((visibility ("default")))extern const char* FILTER_PARA_DEVICE_TYPE;
__attribute__ ((visibility ("default")))extern const char* FILTER_PARA_INCLUDE_TRUST;

// Connection address type
__attribute__ ((visibility ("default")))extern const char* CONN_ADDR_TYPE_ID;
__attribute__ ((visibility ("default")))extern const char* CONN_ADDR_TYPE_BR;
__attribute__ ((visibility ("default")))extern const char* CONN_ADDR_TYPE_BLE;
__attribute__ ((visibility ("default")))extern const char* CONN_ADDR_TYPE_USB;
__attribute__ ((visibility ("default")))extern const char* CONN_ADDR_TYPE_WLAN_IP;
__attribute__ ((visibility ("default")))extern const char* CONN_ADDR_TYPE_ETH_IP;

// Softbus connection address type int
__attribute__ ((visibility ("default")))extern const int32_t CONNECTION_ADDR_USB;

// Parameter Key
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_META_TYPE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_TARGET_ID;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_BR_MAC;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_BLE_MAC;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_WIFI_IP;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_WIFI_PORT;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_USB_IP;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_USB_PORT;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_AUTH_TOKEN;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_AUTH_TYPE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_PIN_CODE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_APP_OPER;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_APP_DESC;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_BLE_UDID_HASH;
constexpr const char* PARAM_KEY_CUSTOM_DATA = "CUSTOM_DATA";
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_CONN_ADDR_TYPE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_PUBLISH_ID;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_SUBSCRIBE_ID;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_TARGET_PKG_NAME;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_PEER_BUNDLE_NAME;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_DISC_FREQ;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_DISC_MEDIUM;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_DISC_CAPABILITY;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_DISC_MODE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_AUTO_STOP_ADVERTISE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_FILTER_OPTIONS;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_BIND_EXTRA_DATA;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_OS_TYPE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_OS_VERSION;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_IS_SHOW_TRUST_DIALOG;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_UDID;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_UUID;
__attribute__ ((visibility ("default")))extern const char* DM_CONNECTION_DISCONNECTED;
__attribute__ ((visibility ("default")))extern const char* BIND_LEVEL;
__attribute__ ((visibility ("default")))extern const char* TOKENID;
__attribute__ ((visibility ("default")))extern const char* DM_BIND_RESULT_NETWORK_ID;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_POLICY_STRATEGY_FOR_BLE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_POLICY_TIME_OUT;
__attribute__ ((visibility ("default")))extern const char* DEVICE_SCREEN_STATUS;
__attribute__ ((visibility ("default")))extern const char* PROCESS_NAME;
__attribute__ ((visibility ("default")))extern const char* PARAM_CLOSE_SESSION_DELAY_SECONDS;
__attribute__ ((visibility ("default")))extern const char* DM_AUTHENTICATION_TYPE;

__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_CONN_SESSIONTYPE;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_HML_RELEASETIME;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_HML_ENABLE_160M;
__attribute__ ((visibility ("default")))extern const char* PARAM_KEY_HML_ACTIONID;

__attribute__ ((visibility ("default")))extern const char* CONN_SESSION_TYPE_HML;
__attribute__ ((visibility ("default")))extern const char* CONN_SESSION_TYPE_BLE;
// screen state
enum ScreenState {
    DM_SCREEN_UNKNOWN = -1,
    DM_SCREEN_ON = 0,
    DM_SCREEN_OFF = 1
};

// errCode map
__attribute__ ((visibility ("default")))extern const std::map<int32_t, int32_t> MAP_ERROR_CODE;
// wise device
__attribute__ ((visibility ("default")))extern const int32_t MAX_DEVICE_PROFILE_SIZE;
const int32_t DEIVCE_NAME_MAX_BYTES = 100;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H