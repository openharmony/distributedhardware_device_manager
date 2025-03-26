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
extern const char* TAG_GROUP_ID;
extern const char* TAG_GROUP_NAME;
extern const char* TAG_REQUEST_ID;
extern const char* TAG_DEVICE_ID;
extern const char* TAG_AUTH_TYPE;
extern const char* TAG_CRYPTO_SUPPORT;
extern const char* TAG_VER;
extern const char* TAG_MSG_TYPE;
extern const char* DM_ITF_VER;
extern const char* DM_PKG_NAME;
extern const char* DM_SESSION_NAME;
extern const char* DM_PIN_HOLDER_SESSION_NAME;
extern const char* DM_SYNC_USERID_SESSION_NAME;
extern const char* DM_CAPABILITY_OSD;
extern const char* DM_CAPABILITY_APPROACH;
extern const char* DM_CAPABILITY_TOUCH;
extern const char* DM_CAPABILITY_CASTPLUS;
extern const char* DM_CAPABILITY_VIRTUAL_LINK;
extern const char* DM_CAPABILITY_SHARE;
extern const char* DM_CAPABILITY_WEAR;
extern const char* DM_CAPABILITY_OOP;
extern const char* DM_CREDENTIAL_TYPE;
extern const char* DM_CREDENTIAL_REQJSONSTR;
extern const char* DM_CREDENTIAL_RETURNJSONSTR;
extern const char* DEVICE_MANAGER_GROUPNAME;
extern const char* FIELD_CREDENTIAL_EXISTS;
extern const char* DM_TYPE_MINE;
extern const char* DM_TYPE_OH;
extern const char* TAG_SESSION_HEARTBEAT;

//The following constant are provided only for HiLink.
extern const char *EXT_PART;

// Auth
extern const char* AUTH_TYPE;
extern const char* APP_OPERATION;
extern const char* CUSTOM_DESCRIPTION;
extern const char* TOKEN;
extern const char* PIN_TOKEN;
extern const char* PIN_CODE_KEY;
extern const int32_t CHECK_AUTH_ALWAYS_POS;
extern const char AUTH_ALWAYS;
extern const char AUTH_ONCE;
extern const char* TAG_TARGET_DEVICE_NAME;
extern const int32_t INVALID_PINCODE;

// HiChain
extern const int32_t SERVICE_INIT_TRY_MAX_NUM;
constexpr int32_t DEVICE_UUID_LENGTH = 65;
extern const int32_t DEVICE_NETWORKID_LENGTH;
extern const int32_t GROUP_TYPE_INVALID_GROUP;
extern const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
extern const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP;
extern const int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP;
extern const int32_t GROUP_VISIBILITY_PUBLIC;
extern const int64_t MIN_REQUEST_ID;
extern const int64_t MAX_REQUEST_ID;
extern const int32_t AUTH_DEVICE_REQ_NEGOTIATE;
extern const int32_t AUTH_DEVICE_RESP_NEGOTIATE;
extern const int32_t DEVICEID_LEN;

// Key of filter parameter
extern const char* FILTER_PARA_RANGE;
extern const char* FILTER_PARA_DEVICE_TYPE;
extern const char* FILTER_PARA_INCLUDE_TRUST;

// Connection address type
extern const char* CONN_ADDR_TYPE_ID;
extern const char* CONN_ADDR_TYPE_BR;
extern const char* CONN_ADDR_TYPE_BLE;
extern const char* CONN_ADDR_TYPE_USB;
extern const char* CONN_ADDR_TYPE_WLAN_IP;
extern const char* CONN_ADDR_TYPE_ETH_IP;

// Softbus connection address type int
extern const int32_t CONNECTION_ADDR_USB;

// Parameter Key
extern const char* PARAM_KEY_META_TYPE;
extern const char* PARAM_KEY_TARGET_ID;
extern const char* PARAM_KEY_BR_MAC;
extern const char* PARAM_KEY_BLE_MAC;
extern const char* PARAM_KEY_WIFI_IP;
extern const char* PARAM_KEY_WIFI_PORT;
extern const char* PARAM_KEY_USB_IP;
extern const char* PARAM_KEY_USB_PORT;
extern const char* PARAM_KEY_AUTH_TOKEN;
extern const char* PARAM_KEY_AUTH_TYPE;
extern const char* PARAM_KEY_PIN_CODE;
extern const char* PARAM_KEY_APP_OPER;
extern const char* PARAM_KEY_APP_DESC;
extern const char* PARAM_KEY_BLE_UDID_HASH;
constexpr const char* PARAM_KEY_CUSTOM_DATA = "CUSTOM_DATA";
extern const char* PARAM_KEY_CONN_ADDR_TYPE;
extern const char* PARAM_KEY_PUBLISH_ID;
extern const char* PARAM_KEY_SUBSCRIBE_ID;
extern const char* PARAM_KEY_TARGET_PKG_NAME;
extern const char* PARAM_KEY_PEER_BUNDLE_NAME;
extern const char* PARAM_KEY_DISC_FREQ;
extern const char* PARAM_KEY_DISC_MEDIUM;
extern const char* PARAM_KEY_DISC_CAPABILITY;
extern const char* PARAM_KEY_DISC_MODE;
extern const char* PARAM_KEY_AUTO_STOP_ADVERTISE;
extern const char* PARAM_KEY_FILTER_OPTIONS;
extern const char* PARAM_KEY_BIND_EXTRA_DATA;
extern const char* PARAM_KEY_OS_TYPE;
extern const char* PARAM_KEY_OS_VERSION;
extern const char* PARAM_KEY_IS_SHOW_TRUST_DIALOG;
extern const char* PARAM_KEY_UDID;
extern const char* PARAM_KEY_UUID;
extern const char* DM_CONNECTION_DISCONNECTED;
extern const char* BIND_LEVEL;
extern const char* TOKENID;
extern const char* DM_BIND_RESULT_NETWORK_ID;
extern const char* PARAM_KEY_POLICY_STRATEGY_FOR_BLE;
extern const char* PARAM_KEY_POLICY_TIME_OUT;
extern const char* DEVICE_SCREEN_STATUS;
extern const char* PROCESS_NAME;
extern const char* PARAM_CLOSE_SESSION_DELAY_SECONDS;
extern const char* DM_AUTHENTICATION_TYPE;

extern const char* PARAM_KEY_CONN_SESSIONTYPE;
extern const char* PARAM_KEY_HML_RELEASETIME;
extern const char* PARAM_KEY_HML_ENABLE_160M;
extern const char* PARAM_KEY_HML_ACTIONID;

extern const char* CONN_SESSION_TYPE_HML;
extern const char* CONN_SESSION_TYPE_BLE;
extern const char* UN_BIND_PARAM_UDID_KEY;
// screen state
enum ScreenState {
    DM_SCREEN_UNKNOWN = -1,
    DM_SCREEN_ON = 0,
    DM_SCREEN_OFF = 1
};

// errCode map
extern const std::map<int32_t, int32_t> MAP_ERROR_CODE;
// wise device
extern const int32_t MAX_DEVICE_PROFILE_SIZE;
const int32_t DEIVCE_NAME_MAX_BYTES = 100;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H