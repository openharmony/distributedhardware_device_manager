/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DM_ERROR_TYPE_H
#define DM_ERROR_TYPE_H

namespace OHOS {
namespace DistributedHardware {
enum {
    DM_OK = 0,
    SOFTBUS_OK = 0,
    STOP_BIND = 1,
    DM_ALREADY_AUTHED = 2,
    DM_BIND_TRUST_TARGET = 969298343,
    /* Transfer to the other end device, not define specification error code */
    ERR_DM_NOT_SYSTEM_APP = 202,
    ERR_DM_TIME_OUT = -20001,
    ERR_DM_PEER_CONFIRM_TIME_OUT = -20002,
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
    ERR_DM_SAVE_SESSION_KEY_FAILED = 96929821,
    ERR_DM_CRYPTO_OPT_FAILED = 96929822,
    ERR_DM_CRYPTO_PARA_INVALID = 96929823,
    ERR_DM_SECURITY_FUNC_FAILED = 96929824,
    ERR_DM_CALLBACK_REGISTER_FAILED = 96929825,
    ERR_DM_HILINKSVC_RSP_PARSE_FAILD = 96929826,
    ERR_DM_HILINKSVC_REPLY_FAILED = 96929827,
    ERR_DM_HILINKSVC_ICON_URL_EMPTY = 96929828,
    ERR_DM_HILINKSVC_DISCONNECT = 96929829,
    ERR_DM_WISE_NEED_LOGIN = 96929830,
    ERR_DM_NAME_EMPTY = 96929831,
    ERR_DM_HICHAIN_PROOFMISMATCH = 96929832,
    ERR_DM_PROCESS_SESSION_KEY_FAILED = 96929833,
    ERR_DM_HILINKSVC_SCAS_CHECK_FAILED = 96929834,
    ERR_DM_FIND_NETWORKID_LIST_EMPTY = 96929835,
    ERR_DM_GET_SESSION_KEY_FAILED = 96929836,
    ERR_DM_QUADRUPLE_NOT_SAME = 96929837,
    ERR_DM_NEXT_STATE_INVALID = 96929838,
    ERR_DM_LOGIC_SESSION_CREATE_FAILED = 96929839,
    ERR_DM_SESSION_CLOSED = 96929840,
    ERR_DM_GET_LOCAL_USERID_FAILED = 969298341,
    ERR_DM_CAPABILITY_NEGOTIATE_FAILED = 969298342,
    ERR_DM_BINDTARGET_SCREEN_LOCK = 969298344,
    ERR_DM_NO_REPLAY = 969298345,
    ERR_DM_GET_TOKENID_FAILED = 969298346,
    ERR_DM_SHOW_CONFIRM_FAILED = 969298347,
    ERR_DM_PARSE_MESSAGE_FAILED = 969298348,
    ERR_DM_GET_BMS_FAILED = 969298349,
    ERR_DM_DESERIAL_CERT_FAILED = 969298350,
    ERR_DM_VERIFY_CERT_FAILED = 969298351,
    ERR_DM_GET_PARAM_FAILED = 969298352,
    ERR_DM_VERIFY_SAME_ACCOUNT_FAILED = 969298353,
    ERR_DM_GET_BUNDLE_NAME_FAILED = 969298354,
    ERR_DM_DEVICE_FROZEN = 969298355,
    ERR_DM_SOCKET_IN_USED = 969298356,
	ERR_DM_ANTI_DISTURB_MODE = 969298357,
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DM_ERROR_TYPE_H