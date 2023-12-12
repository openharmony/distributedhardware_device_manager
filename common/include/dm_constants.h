/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
    ERR_DM_FAILED = -20000,
    ERR_DM_TIME_OUT = -20001,
    ERR_DM_NOT_INIT = -20002,
    ERR_DM_INIT_FAILED = -20004,
    ERR_DM_POINT_NULL = -20005,
    ERR_DM_INPUT_PARA_INVALID = -20006,
    ERR_DM_NO_PERMISSION = -20007,
    ERR_DM_MALLOC_FAILED = -20008,
    ERR_DM_DISCOVERY_FAILED = -20009,
    ERR_DM_MAP_KEY_ALREADY_EXISTS = -20010,
    ERR_DM_IPC_WRITE_FAILED = -20012,
    ERR_DM_IPC_COPY_FAILED = -20013,
    ERR_DM_IPC_SEND_REQUEST_FAILED = -20014,
    ERR_DM_UNSUPPORTED_IPC_COMMAND = -20015,
    ERR_DM_IPC_RESPOND_FAILED = -20016,
    ERR_DM_DISCOVERY_REPEATED = -20017,
    ERR_DM_UNSUPPORTED_AUTH_TYPE = -20018,
    ERR_DM_AUTH_BUSINESS_BUSY = -20019,
    ERR_DM_AUTH_OPEN_SESSION_FAILED = -20020,
    ERR_DM_AUTH_PEER_REJECT = -20021,
    ERR_DM_AUTH_REJECT = -20022,
    ERR_DM_AUTH_FAILED = -20023,
    ERR_DM_AUTH_NOT_START = -20024,
    ERR_DM_AUTH_MESSAGE_INCOMPLETE = -20025,
    ERR_DM_CREATE_GROUP_FAILED = -20026,
    ERR_DM_IPC_READ_FAILED = -20027,
    ERR_DM_ENCRYPT_FAILED = -20028,
    ERR_DM_PUBLISH_FAILED = -20029,
    ERR_DM_PUBLISH_REPEATED = -20030,
    ERR_DM_STOP_DISCOVERY = -20031,

    // The following error codes are provided since OpenHarmony 4.1 Version.
    ERR_DM_ADAPTER_NOT_INIT = -20032,
    ERR_DM_UNSUPPORTED_METHOD = -20033,
    ERR_DM_BIND_COMMON_FAILED = -20034,
    ERR_DM_BIND_INPUT_PARA_INVALID = -20035,
    ERR_DM_BIND_PIN_CODE_ERROR = -20036,
    ERR_DM_BIND_USER_CANCEL = -20037,
    ERR_DM_BIND_TIMEOUT_FAILED = -20038,
    ERR_DM_BIND_DP_ERROR = -20039,
    ERR_DM_BIND_HICHAIN_ERROR = -20040,
    ERR_DM_BIND_SOFTBUS_ERROR = -20041,
    ERR_DM_BIND_USER_CANCEL_ERROR = -20042,
    ERR_DM_STOP_PUBLISH_LNN_FAILED = -20043,
    ERR_DM_REFRESH_LNN_FAILED = -20044,
    ERR_DM_STOP_REFRESH_LNN_FAILED = -20045,
    ERR_DM_START_ADVERTISING_FAILED = -20046,
    ERR_DM_STOP_ADVERTISING_FAILED = -20047,
    ERR_DM_ENABLE_DISCOVERY_LISTENER_FAILED = -20048,
    ERR_DM_DISABLE_DISCOVERY_LISTENER_FAILED = -20049,
    ERR_DM_START_DISCOVERING_FAILED = -20050,
    ERR_DM_STOP_DISCOVERING_FAILED = -20051,
    ERR_DM_SOFTBUS_SERVICE_NOT_INIT = -20052,
    ERR_DM_AUTH_CODE_INCORRECT = -20053,
    ERR_DM_META_TYPE_INVALID = -20054,
    ERR_DM_LOAD_CUSTOM_META_NODE = -20055,
    ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY = -20056,
    ERR_DM_ADD_GROUP_FAILED = -20057,
    ERR_DM_SYNC_DELETE_DEVICE_REPEATED = -20058,
    ERR_DM_BIND_PEER_UNSUPPORTED = -20059,
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
constexpr const char* DM_CAPABILITY_CASTPLUS = "castPlus";
constexpr const char* DM_CAPABILITY_SHARE = "share";
constexpr const char* DM_CAPABILITY_WEAR = "wear";
constexpr int32_t DM_STRING_LENGTH_MAX = 1024;
constexpr int32_t PKG_NAME_SIZE_MAX = 256;

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
const std::string FILTER_PARA_RANGE = "FILTER_RANGE";
const std::string FILTER_PARA_DEVICE_TYPE = "FILTER_DEVICE_TYPE";
const std::string FILTER_PARA_INCLUDE_TRUST = "FILTER_INCLUDE_TRUST";

// Connection address type
const std::string CONN_ADDR_TYPE_ID = "ID_TYPE";
const std::string CONN_ADDR_TYPE_BR = "BR_TYPE";
const std::string CONN_ADDR_TYPE_BLE = "BLE_TYPE";
const std::string CONN_ADDR_TYPE_WLAN_IP = "WLAN_IP_TYPE";

// Parameter Key
const std::string PARAM_KEY_META_TYPE = "META_TYPE";
const std::string PARAM_KEY_TARGET_ID = "TARGET_ID";
const std::string PARAM_KEY_BR_MAC = "BR_MAC";
const std::string PARAM_KEY_BLE_MAC = "BLE_MAC";
const std::string PARAM_KEY_WIFI_IP = "WIFI_IP";
const std::string PARAM_KEY_WIFI_PORT = "WIFI_PORT";
const std::string PARAM_KEY_AUTH_TOKEN = "AUTH_TOKEN";
const std::string PARAM_KEY_AUTH_TYPE = "AUTH_TYPE";
const std::string PARAM_KEY_PIN_CODE = "PIN_CODE";
const std::string PARAM_KEY_APP_OPER = "APP_OPER";
const std::string PARAM_KEY_APP_DESC = "APP_DESC";
const std::string PARAM_KEY_BLE_UDID_HASH = "BLE_UDID_HASH";
const std::string PARAM_KEY_CUSTOM_DATA = "CUSTOM_DATA";
const std::string PARAM_KEY_CONN_ADDR_TYPE = "CONN_ADDR_TYPE";
const std::string PARAM_KEY_PUBLISH_ID = "PUBLISH_ID";
const std::string PARAM_KEY_SUBSCRIBE_ID = "SUBSCRIBE_ID";
const std::string PARAM_KEY_TARGET_PKG_NAME = "TARGET_PKG_NAME";
const std::string PARAM_KEY_DISC_FREQ = "DISC_FREQ";
const std::string PARAM_KEY_DISC_MEDIUM = "DISC_MEDIUM";
const std::string PARAM_KEY_DISC_CAPABILITY = "DISC_CAPABILITY";
const std::string PARAM_KEY_AUTO_STOP_ADVERTISE = "AUTO_STOP_ADVERTISE";
const std::string PARAM_KEY_FILTER_OPTIONS = "FILTER_OPTIONS";
const std::string PARAM_KEY_BIND_EXTRA_DATA = "BIND_EXTRA_DATA";
const std::string DM_CONNECTION_DISCONNECTED = "DM_CONNECTION_DISCONNECTED";
const std::string BIND_LEVEL = "bindLevel";
const std::string TOKENID = "tokenId";
const std::string DM_BIND_RESULT_NETWORK_ID = "DM_BIND_RESULT_NETWORK_ID";
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H