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

#ifndef OHOS_DM_CONSTANTS_H
#define OHOS_DM_CONSTANTS_H

#include <string>

#ifdef __LP64__
constexpr const char* LIB_LOAD_PATH = "/system/lib64/";
#else
constexpr const char* LIB_LOAD_PATH = "/system/lib/";
#endif

namespace OHOS {
namespace DistributedHardware {
constexpr const char* DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
constexpr const char* DM_SESSION_NAME = "ohos.distributedhardware.devicemanager.resident";

constexpr const char* DISCOVER_STATUS_KEY = "persist.distributed_hardware.device_manager.discover_status";
constexpr const char* DISCOVER_STATUS_ON = "1";
constexpr const char* DISCOVER_STATUS_OFF = "0";
constexpr const char* AUTH_LOAD_JSON_KEY = "devicemanager_auth_components";
constexpr const char* ADAPTER_LOAD_JSON_KEY = "devicemanager_adapter_components";
constexpr const char* AUTH_JSON_TYPE_KEY = "AUTHENTICATE";
constexpr const char* CPYPTO_JSON_TYPE_KEY = "CPYPTO";
constexpr const char* PROFILE_JSON_TYPE_KEY = "PROFILE";
constexpr const char* DECISION_JSON_TYPE_KEY = "DECISION";

const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_DDMP = 0;
const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_OSD = 1;
const int32_t MIN_PIN_TOKEN = 10000000;
const int32_t MAX_PIN_TOKEN = 90000000;
const int32_t MIN_PIN_CODE = 100000;
const int32_t MAX_PIN_CODE = 999999;
const int32_t DISCOVER_STATUS_LEN = 20;
const int32_t COMMON_CALLBACK_MAX_SIZE = 200;
const uint32_t MAX_LOAD_VALUE = 3;

const int32_t DM_AUTH_TYPE_MAX = 4;
const int32_t DM_AUTH_TYPE_MIN = 1;

enum {
    DM_OK = 0,
    ERR_DM_FAILED = -20000,
    ERR_DM_TIME_OUT = -20001,
    ERR_DM_NOT_INIT = -20002,
    ERR_DM_INIT_REPEATED = -20003,
    ERR_DM_INIT_FAILED = -20004,
    ERR_DM_POINT_NULL = -20005,
    ERR_DM_INPUT_PARAMETER_EMPTY = -20006,
    ERR_DM_NO_PERMISSION = -20007,
    ERR_DM_MALLOC_FAILED = -20008,
    ERR_DM_DISCOVERY_FAILED = -20009,
    ERR_DM_MAP_KEY_ALREADY_EXISTS = -20010,
    ERR_DM_PROFILE_EVENTS_FAILED = -20011,
    ERR_DM_IPC_WRITE_FAILED = -20012,
    ERR_DM_IPC_COPY_FAILED = -20013,
    ERR_DM_IPC_SEND_REQUEST_FAILED = -20014,
    ERR_DM_UNSUPPORTED_IPC_COMMAND = -20015,
    ERR_DM_IPC_RESPOND_FAILED = -20016,
    ERR_DM_DISCOVERY_REPEATED  = -20017,
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
};

constexpr const char* TARGET_PKG_NAME_KEY = "targetPkgName";
constexpr const char* TAG_REQUESTER = "REQUESTER";
constexpr const char* TAG_TOKEN = "TOKEN";
constexpr const char* TAG_HOST = "HOST";
constexpr const char* TAG_TARGET = "TARGET";
constexpr const char* TAG_VISIBILITY = "VISIBILITY";
constexpr const char* TAG_GROUPIDS = "GROUPIDLIST";
constexpr const char* TAG_REPLY = "REPLY";
constexpr const char* TAG_NET_ID = "NETID";
constexpr const char* TAG_GROUP_ID = "groupId";
constexpr const char* TAG_GROUP_NAME = "GROUPNAME";
constexpr const char* TAG_REQUEST_ID = "REQUESTID";
constexpr const char* TAG_DEVICE_ID = "DEVICEID";
constexpr const char* TAG_LOCAL_DEVICE_ID = "LOCALDEVICEID";
constexpr const char* TAG_DEVICE_TYPE = "DEVICETYPE";
constexpr const char* TAG_APP_NAME = "APPNAME";
constexpr const char* TAG_APP_DESCRIPTION = "APPDESC";
constexpr const char* TAG_APP_ICON = "APPICON";
constexpr const char* TAG_APP_THUMBNAIL = "APPTHUM";
constexpr const char* TAG_INDEX = "INDEX";
constexpr const char* TAG_SLICE_NUM = "SLICE";
constexpr const char* TAG_THUMBNAIL_SIZE = "THUMSIZE";
constexpr const char* TAG_AUTH_TYPE = "AUTHTYPE";
constexpr const char* TAG_CRYPTO_SUPPORT = "CRYPTOSUPPORT";
constexpr const char* TAG_CRYPTO_NAME = "CRYPTONAME";
constexpr const char* TAG_CRYPTO_VERSION = "CRYPTOVERSION";
constexpr const char* TAG_VER = "ITF_VER";
constexpr const char* TAG_TYPE = "MSG_TYPE";
constexpr const char* DM_ITF_VER = "1.1";
constexpr const char* APP_NAME_KEY = "appName";
constexpr const char* APP_DESCRIPTION_KEY = "appDescription";
constexpr const char* APP_ICON_KEY = "appIcon";
constexpr const char* APP_THUMBNAIL = "appThumbnail";
constexpr const char* CANCEL_DISPLAY_KEY = "cancelPinCodeDisplay";
const int32_t MSG_MAX_SIZE = 45 * 1024;
const int32_t AUTH_REPLY_ACCEPT = 0;
const int32_t ENCRYPT_TAG_LEN = 32;
const int32_t SERVICE_INIT_TRY_MAX_NUM = 200;
const int32_t SLEEP_TIME_MS = 50000; // 50ms

// pin
const int32_t DISPLAY_OWNER_SYSTEM = 0;
const int32_t DISPLAY_OWNER_OTHER = 1;
const int32_t BUSINESS_FA_MIRGRATION = 0;
const int32_t BUSINESS_RESOURCE_ACCESS = 1;

// json
constexpr const char* AUTH_TYPE = "authType";
constexpr const char* TOKEN = "token";
constexpr const char* PIN_TOKEN = "pinToken";
constexpr const char* PIN_CODE_KEY = "pinCode";
constexpr const char* NFC_CODE_KEY = "nfcCode";
constexpr const char* QR_CODE_KEY = "qrCode";
constexpr const char* TAG_AUTH_TOKEN = "authToken";
constexpr const char* VERIFY_FAILED = "verifyFailed";
const int32_t AUTH_TYPE_PIN = 1;
const int32_t AUTH_TYPE_SCAN = 2;
const int32_t AUTH_TYPE_TOUCH = 3;
const int32_t DEFAULT_PIN_CODE = 0;
const int32_t DEFAULT_PIN_TOKEN = 0;
const int32_t DEFAULT_PIN_CODE_LENGTH = 6;
const int32_t LOCAL_CREDENTIAL_DEAL_TYPE = 1;
const int32_t REMOTE_CREDENTIAL_DEAL_TYPE = 2;
const int32_t NONSYMMETRY_CREDENTIAL_TYPE = 2;
const int32_t SYMMETRY_CREDENTIAL_TYPE = 1;
const int32_t UNKNOWN_CREDENTIAL_TYPE = 0;
// Softbus
const int32_t SOFTBUS_CHECK_INTERVAL = 100000; // 100ms
const uint32_t SOFTBUS_SUBSCRIBE_ID_PREFIX_LEN = 16;
const int32_t SOFTBUS_SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t SOFTBUS_DISCOVER_DEVICE_INFO_MAX_SIZE = 20;
const int32_t AUTH_SESSION_SIDE_SERVER = 0;
const int32_t AUTH_SESSION_SIDE_CLIENT = 1;
const static char *DM_CAPABILITY_OSD = "osdCapability";

// HiChain
const int32_t DEVICE_UUID_LENGTH = 65;
const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1;
const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP = 256;
const int32_t GROUP_VISIBILITY_PUBLIC = -1;
const int64_t MIN_REQUEST_ID = 1000000000;
const int64_t MAX_REQUEST_ID = 9999999999;
const int32_t FIELD_EXPIRE_TIME_VALUE = 7;
const int32_t GROUP_VISIBILITY_IS_PUBLIC = -1;
const int32_t GROUP_VISIBILITY_IS_PRIVATE = 0;
constexpr const char* DEVICE_ID = "DEVICE_ID";
constexpr const char* WIFI_IP = "WIFI_IP";
constexpr const char* WIFI_PORT = "WIFI_PORT";
constexpr const char* BR_MAC = "BR_MAC";
constexpr const char* BLE_MAC = "BLE_MAC";
constexpr const char* ETH_IP = "ETH_IP";
constexpr const char* ETH_PORT = "ETH_PORT";

// ACE
const int32_t ACE_X = 50;
const int32_t ACE_Y = 300;
const int32_t ACE_WIDTH = 580;
const int32_t ACE_HEIGHT = 520;
constexpr const char* EVENT_CONFIRM = "EVENT_CONFIRM";
constexpr const char* EVENT_CANCEL = "EVENT_CANCEL";
constexpr const char* EVENT_INIT = "EVENT_INIT";
constexpr const char* EVENT_CONFIRM_CODE = "0";
constexpr const char* EVENT_CANCEL_CODE = "1";
constexpr const char* EVENT_INIT_CODE = "2";

// timer
constexpr const char* AUTHENTICATE_TIMEOUT_TASK = "deviceManagerTimer:authenticate";
constexpr const char* NEGOTIATE_TIMEOUT_TASK = "deviceManagerTimer:negotiate";
constexpr const char* CONFIRM_TIMEOUT_TASK = "deviceManagerTimer:confirm";
constexpr const char* INPUT_TIMEOUT_TASK = "deviceManagerTimer:input";
constexpr const char* ADD_TIMEOUT_TASK = "deviceManagerTimer:add";
constexpr const char* WAIT_NEGOTIATE_TIMEOUT_TASK = "deviceManagerTimer:waitNegotiate";
constexpr const char* WAIT_REQUEST_TIMEOUT_TASK = "deviceManagerTimer:waitRequest";
constexpr const char* STATE_TIMER_PREFIX = "deviceManagerTimer:stateTimer_";
const int32_t TIMER_PREFIX_LENGTH = 19;
const int32_t TIMER_DEFAULT = 0;
const int32_t NO_TIMER = -1;
const int32_t INIT_SIZE = 3;
const int32_t MAX_EVENT_NUMBER = 10;
const int32_t EXPAND_TWICE = 2;
const int32_t SEC_TO_MM = 1000;
const int32_t MAX_EVENTS = 5;

// credential
constexpr const char* FIELD_CREDENTIAL = "credential";
constexpr const char* FIELD_CREDENTIAL_VERSION = "version";
constexpr const char* FIELD_DEVICE_PK = "devicePk";
constexpr const char* FIELD_SERVER_PK = "serverPk";
constexpr const char* FIELD_PKINFO_SIGNATURE = "pkInfoSignature";
constexpr const char* FIELD_PKINFO = "pkInfo";
constexpr const char* FIELD_PROCESS_TYPE = "processType";
constexpr const char* FIELD_AUTH_TYPE = "authType";
constexpr const char* FIELD_CREDENTIAL_DATA = "credentialData";
constexpr const char* FIELD_CREDENTIAL_ID = "credentialId";
constexpr const char* FIELD_PEER_CREDENTIAL_INFO = "peerCredentialInfo";
const int32_t SAME_ACCOUNT_TYPE = 1;
const int32_t CROSS_ACCOUNT_TYPE = 2;
const int32_t PIN_CODE_NETWORK = 0;
const int32_t CREDENTIAL_NETWORK = 1;
const int32_t DELAY_TIME_MS = 10000; // 10ms
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H
