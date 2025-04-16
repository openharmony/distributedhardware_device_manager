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

#include <string>
#include <map>

#ifdef __LP64__
constexpr const char* DM_LIB_LOAD_PATH = "/system/lib64/";
#else
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* DM_LIB_LOAD_PATH = "/system/lib/";
#else
constexpr const char* DM_LIB_LOAD_PATH = "/usr/lib/";
#endif
#endif

namespace OHOS {
namespace DistributedHardware {
enum {
    DM_OK = 0,
    SOFTBUS_OK = 0,
    STOP_BIND = 1,

    /* Transfer to the other end device, not define specification error code */
    ERR_DM_TIME_OUT = -20001,
    ERR_DM_UNSUPPORTED_AUTH_TYPE = -20018,
    ERR_DM_AUTH_BUSINESS_BUSY = -20019,
    ERR_DM_AUTH_PEER_REJECT = -20021,
    ERR_DM_AUTH_REJECT = -20022,
    ERR_DM_CREATE_GROUP_FAILED = -20026,
    ERR_DM_BIND_USER_CANCEL = -20037,
    ERR_DM_BIND_USER_CANCEL_ERROR = -20042,
    ERR_DM_AUTH_CODE_INCORRECT = -20053,
    ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY = -20056,
    ERR_DM_SYNC_DELETE_DEVICE_REPEATED = -20058,
    ERR_DM_VERSION_INCOMPATIBLE = -20059,

    ERR_DM_FAILED = 96929744,
    ERR_DM_NOT_INIT = 96929746,
    ERR_DM_INIT_FAILED = 96929747,
    ERR_DM_POINT_NULL = 96929748,
    ERR_DM_INPUT_PARA_INVALID = 96929749,
    ERR_DM_NO_PERMISSION = 96929750,
    ERR_DM_MALLOC_FAILED = 96929751,
    ERR_DM_DISCOVERY_FAILED = 96929752,
    ERR_DM_MAP_KEY_ALREADY_EXISTS = 96929753,
    ERR_DM_IPC_WRITE_FAILED = 96929754,
    ERR_DM_IPC_COPY_FAILED = 96929755,
    ERR_DM_IPC_SEND_REQUEST_FAILED = 96929756,
    ERR_DM_UNSUPPORTED_IPC_COMMAND = 96929757,
    ERR_DM_IPC_RESPOND_FAILED = 96929758,
    ERR_DM_DISCOVERY_REPEATED = 96929759,
    ERR_DM_AUTH_OPEN_SESSION_FAILED = 96929762,
    ERR_DM_AUTH_FAILED = 96929765,
    ERR_DM_AUTH_NOT_START = 96929766,
    ERR_DM_AUTH_MESSAGE_INCOMPLETE = 96929767,
    ERR_DM_IPC_READ_FAILED = 96929769,
    ERR_DM_ENCRYPT_FAILED = 96929770,
    ERR_DM_PUBLISH_FAILED = 96929771,
    ERR_DM_PUBLISH_REPEATED = 96929772,
    ERR_DM_STOP_DISCOVERY = 96929773,
    ERR_DM_ADD_GROUP_FAILED = 96929774,

    // The following error codes are provided since OpenHarmony 4.1 Version.
    ERR_DM_ADAPTER_NOT_INIT = 96929775,
    ERR_DM_UNSUPPORTED_METHOD = 96929776,
    ERR_DM_BIND_COMMON_FAILED = 96929777,
    ERR_DM_BIND_INPUT_PARA_INVALID = 96929778,
    ERR_DM_BIND_PIN_CODE_ERROR = 96929779,
    ERR_DM_BIND_TIMEOUT_FAILED = 96929781,
    ERR_DM_BIND_DP_ERROR = 96929782,
    ERR_DM_BIND_HICHAIN_ERROR = 96929783,
    ERR_DM_BIND_SOFTBUS_ERROR = 96929784,
    ERR_DM_STOP_PUBLISH_LNN_FAILED = 96929786,
    ERR_DM_REFRESH_LNN_FAILED = 96929787,
    ERR_DM_STOP_REFRESH_LNN_FAILED = 96929788,
    ERR_DM_START_ADVERTISING_FAILED = 96929789,
    ERR_DM_STOP_ADVERTISING_FAILED = 96929790,
    ERR_DM_ENABLE_DISCOVERY_LISTENER_FAILED = 96929791,
    ERR_DM_DISABLE_DISCOVERY_LISTENER_FAILED = 96929792,
    ERR_DM_START_DISCOVERING_FAILED = 96929793,
    ERR_DM_STOP_DISCOVERING_FAILED = 96929794,
    ERR_DM_SOFTBUS_SERVICE_NOT_INIT = 96929795,
    ERR_DM_META_TYPE_INVALID = 96929797,
    ERR_DM_LOAD_CUSTOM_META_NODE = 96929798,
    ERR_DM_SOFTBUS_PUBLISH_SERVICE = 96929800,
    ERR_DM_BIND_PEER_UNSUPPORTED = 96929802,
    ERR_DM_HICHAIN_CREDENTIAL_REQUEST_FAILED = 96929803,
    ERR_DM_HICHAIN_CREDENTIAL_CHECK_FAILED = 96929804,
    ERR_DM_HICHAIN_CREDENTIAL_IMPORT_FAILED = 96929805,
    ERR_DM_HICHAIN_CREDENTIAL_DELETE_FAILED = 96929806,
    ERR_DM_HICHAIN_UNREGISTER_CALLBACK = 96929807,
    ERR_DM_HICHAIN_GET_REGISTER_INFO = 96929808,
    ERR_DM_HICHAIN_CREDENTIAL_EXISTS = 96929809,
    ERR_DM_HICHAIN_REGISTER_CALLBACK = 96929810,
    ERR_DM_HICHAIN_GROUP_CREATE_FAILED = 96929811,
    ERR_DM_JSON_PARSE_STRING = 96929812,
    ERR_DM_SOFTBUS_SEND_BROADCAST = 96929813,
    ERR_DM_SOFTBUS_DISCOVERY_DEVICE = 96929814,
    ERR_DM_SOFTBUS_STOP_DISCOVERY_DEVICE = 96929815,
    ERR_DM_INVALID_JSON_STRING = 96929816,
    ERR_DM_GET_DATA_SHA256_HASH = 96929817,
    ERR_DM_CHANNLE_OPEN_TIMEOUT = 96929818,
    ERR_DM_ADD_GOUTP_TIMEOUT = 96929819,
    ERR_DM_INPUT_TIMEOUT = 96929820,
    ERR_DM_MAX_SIZE_FAIL = 96929825,
    ERR_DM_HILINKSVC_RSP_PARSE_FAILD = 96929826,
    ERR_DM_HILINKSVC_REPLY_FAILED = 96929827,
    ERR_DM_HILINKSVC_ICON_URL_EMPTY = 96929828,
    ERR_DM_HILINKSVC_DISCONNECT = 96929829,
    ERR_DM_WISE_NEED_LOGIN = 96929830
};

constexpr const char* TAG_GROUP_ID = "groupId";
constexpr const char* TAG_GROUP_NAME = "GROUPNAME";
constexpr const char* TAG_REQUEST_ID = "REQUESTID";
constexpr const char* TAG_DEVICE_ID = "DEVICEID";
constexpr const char* TAG_AUTH_TYPE = "AUTHTYPE";
constexpr const char* TAG_CRYPTO_SUPPORT = "CRYPTOSUPPORT";
constexpr const char* TAG_VER = "ITF_VER";
constexpr const char* TAG_MSG_TYPE = "MSG_TYPE";
constexpr const char* DM_ITF_VER = "1.1";
constexpr const char* DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
constexpr const char* DM_SESSION_NAME = "ohos.distributedhardware.devicemanager.resident";
constexpr const char* DM_PIN_HOLDER_SESSION_NAME = "ohos.distributedhardware.devicemanager.pinholder";
constexpr const char* DM_UNBIND_SESSION_NAME = "ohos.distributedhardware.devicemanager.unbind";
constexpr const char* DM_CAPABILITY_OSD = "osdCapability";
constexpr const char* DM_CAPABILITY_APPROACH = "approach";
constexpr const char* DM_CAPABILITY_TOUCH = "touch";
constexpr const char* DM_CAPABILITY_CASTPLUS = "castPlus";
constexpr const char* DM_CAPABILITY_VIRTUAL_LINK = "virtualLink";
constexpr const char* DM_CAPABILITY_SHARE = "share";
constexpr const char* DM_CAPABILITY_WEAR = "wear";
constexpr const char* DM_CREDENTIAL_TYPE = "CREDENTIAL_TYPE";
constexpr const char* DM_CREDENTIAL_REQJSONSTR = "CREDENTIAL_REQJSONSTR";
constexpr const char* DM_CREDENTIAL_RETURNJSONSTR = "CREDENTIAL_RETURNJSONSTR";
constexpr const char* DEVICE_MANAGER_GROUPNAME = "DMSameAccountGroup";
constexpr const char* FIELD_CREDENTIAL_EXISTS = "isCredentialExists";
constexpr int32_t DM_STRING_LENGTH_MAX = 1024;
constexpr int32_t PKG_NAME_SIZE_MAX = 256;
constexpr const char* DM_TYPE_MINE = "MINE";
constexpr const char* DM_TYPE_OH = "OH";
constexpr const char* TAG_SESSION_HEARTBEAT = "session_heartbeat";

//The following constant are provided only for HiLink.
const static char *EXT_PART = "ext_part";

// Auth
constexpr const char* AUTH_TYPE = "authType";
constexpr const char* APP_OPERATION = "appOperation";
constexpr const char* CUSTOM_DESCRIPTION = "customDescription";
constexpr const char* TOKEN = "token";
constexpr const char* PIN_TOKEN = "pinToken";
constexpr const char* PIN_CODE_KEY = "pinCode";
constexpr int32_t CHECK_AUTH_ALWAYS_POS = 0;
constexpr const char AUTH_ALWAYS = '1';
constexpr const char AUTH_ONCE = '0';
constexpr const char* TAG_TARGET_DEVICE_NAME = "targetDeviceName";

// HiChain
constexpr int32_t SERVICE_INIT_TRY_MAX_NUM = 200;
constexpr int32_t DEVICE_UUID_LENGTH = 65;
constexpr int32_t DEVICE_NETWORKID_LENGTH = 100;
constexpr int32_t GROUP_TYPE_INVALID_GROUP = -1;
constexpr int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1;
constexpr int32_t GROUP_TYPE_PEER_TO_PEER_GROUP = 256;
constexpr int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP = 1282;
constexpr int32_t GROUP_VISIBILITY_PUBLIC = -1;
constexpr int64_t MIN_REQUEST_ID = 1000000000;
constexpr int64_t MAX_REQUEST_ID = 9999999999;
constexpr int32_t AUTH_DEVICE_REQ_NEGOTIATE = 600;
constexpr int32_t AUTH_DEVICE_RESP_NEGOTIATE = 700;
constexpr int32_t DEVICEID_LEN = 8;

// ACE
constexpr const char* EVENT_CONFIRM = "EVENT_CONFIRM";
constexpr const char* EVENT_CANCEL = "EVENT_CANCEL";
constexpr const char* EVENT_INIT = "EVENT_INIT";
constexpr const char* EVENT_CONFIRM_CODE = "0";
constexpr const char* EVENT_CANCEL_CODE = "1";
constexpr const char* EVENT_INIT_CODE = "2";

// Key of filter parameter
constexpr const char* FILTER_PARA_RANGE = "FILTER_RANGE";
constexpr const char* FILTER_PARA_DEVICE_TYPE = "FILTER_DEVICE_TYPE";
constexpr const char* FILTER_PARA_INCLUDE_TRUST = "FILTER_INCLUDE_TRUST";

// Connection address type
constexpr const char* CONN_ADDR_TYPE_ID = "ID_TYPE";
constexpr const char* CONN_ADDR_TYPE_BR = "BR_TYPE";
constexpr const char* CONN_ADDR_TYPE_BLE = "BLE_TYPE";
constexpr const char* CONN_ADDR_TYPE_WLAN_IP = "WLAN_IP_TYPE";
constexpr const char* CONN_ADDR_TYPE_ETH_IP = "ETH_IP_TYPE";

// Parameter Key
constexpr const char* PARAM_KEY_META_TYPE = "META_TYPE";
constexpr const char* PARAM_KEY_TARGET_ID = "TARGET_ID";
constexpr const char* PARAM_KEY_BR_MAC = "BR_MAC";
constexpr const char* PARAM_KEY_BLE_MAC = "BLE_MAC";
constexpr const char* PARAM_KEY_WIFI_IP = "WIFI_IP";
constexpr const char* PARAM_KEY_WIFI_PORT = "WIFI_PORT";
constexpr const char* PARAM_KEY_AUTH_TOKEN = "AUTH_TOKEN";
constexpr const char* PARAM_KEY_AUTH_TYPE = "AUTH_TYPE";
constexpr const char* PARAM_KEY_PIN_CODE = "PIN_CODE";
constexpr const char* PARAM_KEY_APP_OPER = "APP_OPER";
constexpr const char* PARAM_KEY_APP_DESC = "APP_DESC";
constexpr const char* PARAM_KEY_BLE_UDID_HASH = "BLE_UDID_HASH";
constexpr const char* PARAM_KEY_CUSTOM_DATA = "CUSTOM_DATA";
constexpr const char* PARAM_KEY_CONN_ADDR_TYPE = "CONN_ADDR_TYPE";
constexpr const char* PARAM_KEY_PUBLISH_ID = "PUBLISH_ID";
constexpr const char* PARAM_KEY_SUBSCRIBE_ID = "SUBSCRIBE_ID";
constexpr const char* PARAM_KEY_TARGET_PKG_NAME = "TARGET_PKG_NAME";
constexpr const char* PARAM_KEY_DISC_FREQ = "DISC_FREQ";
constexpr const char* PARAM_KEY_DISC_MEDIUM = "DISC_MEDIUM";
constexpr const char* PARAM_KEY_DISC_CAPABILITY = "DISC_CAPABILITY";
constexpr const char* PARAM_KEY_DISC_MODE = "DISC_MODE";
constexpr const char* PARAM_KEY_AUTO_STOP_ADVERTISE = "AUTO_STOP_ADVERTISE";
constexpr const char* PARAM_KEY_FILTER_OPTIONS = "FILTER_OPTIONS";
constexpr const char* PARAM_KEY_BIND_EXTRA_DATA = "BIND_EXTRA_DATA";
constexpr const char* PARAM_KEY_OS_TYPE = "OS_TYPE";
constexpr const char* PARAM_KEY_OS_VERSION = "OS_VERSION";
constexpr const char* DM_CONNECTION_DISCONNECTED = "DM_CONNECTION_DISCONNECTED";
constexpr const char* BIND_LEVEL = "bindLevel";
constexpr const char* TOKENID = "tokenId";
constexpr const char* DM_BIND_RESULT_NETWORK_ID = "DM_BIND_RESULT_NETWORK_ID";
constexpr const char* PARAM_KEY_POLICY_STRATEGY_FOR_BLE = "DM_POLICY_STRATEGY_FOR_BLE";
constexpr const char* PARAM_KEY_POLICY_TIME_OUT = "DM_POLICY_TIMEOUT";
constexpr const char* DEVICE_SCREEN_STATUS = "DEVICE_SCREEN_STATUS";
constexpr const char* PARAM_CLOSE_SESSION_DELAY_SECONDS = "DM_CLOSE_SESSION_DELAY_SECONDS";
constexpr const char* DM_AUTHENTICATION_TYPE = "DM_AUTHENTICATION_TYPE";

// screen state
constexpr int32_t DM_SCREEN_UNKNOWN = -1;
constexpr int32_t DM_SCREEN_ON = 0;
constexpr int32_t DM_SCREEN_OFF = 1;

// errCode map
const std::map<int32_t, int32_t> MAP_ERROR_CODE = {
    { ERR_DM_TIME_OUT, 96929745 }, { ERR_DM_UNSUPPORTED_AUTH_TYPE, 96929760 }, { ERR_DM_AUTH_BUSINESS_BUSY, 96929761 },
    { ERR_DM_AUTH_PEER_REJECT, 96929763 }, { ERR_DM_AUTH_REJECT, 96929764 }, { ERR_DM_CREATE_GROUP_FAILED, 96929768 },
    { ERR_DM_BIND_USER_CANCEL, 96929780 }, { ERR_DM_BIND_USER_CANCEL_ERROR, 96929785 },
    { ERR_DM_AUTH_CODE_INCORRECT, 96929796 }, { ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY, 96929799 },
    { ERR_DM_SYNC_DELETE_DEVICE_REPEATED, 96929801 }
};

// wise device
constexpr int32_t MAX_DEVICE_PROFILE_SIZE = 500;
constexpr int32_t MAX_CONTAINER_SIZE = 500;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H