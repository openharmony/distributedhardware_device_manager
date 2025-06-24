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

#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
const char* TAG_GROUP_ID = "groupId";
const char* TAG_GROUP_NAME = "GROUPNAME";
const char* TAG_REQUEST_ID = "REQUESTID";
const char* TAG_DEVICE_ID = "DEVICEID";
const char* TAG_AUTH_TYPE = "AUTHTYPE";
const char* TAG_CRYPTO_SUPPORT = "CRYPTOSUPPORT";
const char* TAG_VER = "ITF_VER";
const char* TAG_MSG_TYPE = "MSG_TYPE";
const char* DM_ITF_VER = "1.1";
const char* DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
const char* DM_SESSION_NAME = "ohos.distributedhardware.devicemanager.resident";
const char* DM_PIN_HOLDER_SESSION_NAME = "ohos.distributedhardware.devicemanager.pinholder";
const char* DM_SYNC_USERID_SESSION_NAME = "ohos.distributedhardware.devicemanager.syncuserid";
const char* DM_CAPABILITY_OSD = "osdCapability";
const char* DM_CAPABILITY_APPROACH = "approach";
const char* DM_CAPABILITY_OH_APPROACH = "oh_approach";
const char* DM_CAPABILITY_TOUCH = "touch";
const char* DM_CAPABILITY_CASTPLUS = "castPlus";
const char* DM_CAPABILITY_VIRTUAL_LINK = "virtualLink";
const char* DM_CAPABILITY_SHARE = "share";
const char* DM_CAPABILITY_WEAR = "wear";
const char* DM_CAPABILITY_OOP = "oop";
const char* DM_CREDENTIAL_TYPE = "CREDENTIAL_TYPE";
const char* DM_CREDENTIAL_REQJSONSTR = "CREDENTIAL_REQJSONSTR";
const char* DM_CREDENTIAL_RETURNJSONSTR = "CREDENTIAL_RETURNJSONSTR";
const char* DEVICE_MANAGER_GROUPNAME = "DMSameAccountGroup";
const char* FIELD_CREDENTIAL_EXISTS = "isCredentialExists";
const char* DM_TYPE_MINE = "MINE";
const char* DM_TYPE_OH = "OH";
const char* TAG_SESSION_HEARTBEAT = "session_heartbeat";

//The following constant are provided only for HiLink.
const char *EXT_PART = "ext_part";

// Auth
const char* AUTH_TYPE = "authType";
const char* APP_OPERATION = "appOperation";
const char* CUSTOM_DESCRIPTION = "customDescription";
const char* TOKEN = "token";
const char* PIN_TOKEN = "pinToken";
const char* PIN_CODE_KEY = "pinCode";
const int32_t CHECK_AUTH_ALWAYS_POS = 0;
const char AUTH_ALWAYS = '1';
const char AUTH_ONCE = '0';
const char* TAG_TARGET_DEVICE_NAME = "targetDeviceName";
const int32_t MIN_PINCODE_SIZE = 6;

// HiChain
const int32_t SERVICE_INIT_TRY_MAX_NUM = 200;
const int32_t DEVICE_NETWORKID_LENGTH = 100;
const int32_t GROUP_TYPE_INVALID_GROUP = -1;
const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1;
const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP = 256;
const int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP = 1282;
const int32_t GROUP_VISIBILITY_PUBLIC = -1;
const int64_t MIN_REQUEST_ID = 1000000000;
const int64_t MAX_REQUEST_ID = 9999999999;
const int32_t AUTH_DEVICE_REQ_NEGOTIATE = 600;
const int32_t AUTH_DEVICE_RESP_NEGOTIATE = 700;
const int32_t DEVICEID_LEN = 8;

// Key of filter parameter
const char* FILTER_PARA_RANGE = "FILTER_RANGE";
const char* FILTER_PARA_DEVICE_TYPE = "FILTER_DEVICE_TYPE";
const char* FILTER_PARA_INCLUDE_TRUST = "FILTER_INCLUDE_TRUST";

// Connection address type
const char* CONN_ADDR_TYPE_ID = "ID_TYPE";
const char* CONN_ADDR_TYPE_BR = "BR_TYPE";
const char* CONN_ADDR_TYPE_BLE = "BLE_TYPE";
const char* CONN_ADDR_TYPE_USB = "USB_TYPE";
const char* CONN_ADDR_TYPE_WLAN_IP = "WLAN_IP_TYPE";
const char* CONN_ADDR_TYPE_ETH_IP = "ETH_IP_TYPE";
const char* CONN_ADDR_TYPE_NCM = "NCM_TYPE";

// Softbus connection address type int
const int32_t CONNECTION_ADDR_USB = 5;

// Parameter Key
const char* PARAM_KEY_META_TYPE = "META_TYPE";
const char* PARAM_KEY_TARGET_ID = "TARGET_ID";
const char* PARAM_KEY_BR_MAC = "BR_MAC";
const char* PARAM_KEY_BLE_MAC = "BLE_MAC";
const char* PARAM_KEY_WIFI_IP = "WIFI_IP";
const char* PARAM_KEY_WIFI_PORT = "WIFI_PORT";
const char* PARAM_KEY_USB_IP = "USB_IP";
const char* PARAM_KEY_USB_PORT = "USB_PORT";
const char* PARAM_KEY_NCM_IP = "NCM_IP";
const char* PARAM_KEY_NCM_PORT = "NCM_PORT";
const char* PARAM_KEY_AUTH_TOKEN = "AUTH_TOKEN";
const char* PARAM_KEY_AUTH_TYPE = "AUTH_TYPE";
const char* PARAM_KEY_PIN_CODE = "PIN_CODE";
const char* PARAM_KEY_APP_OPER = "APP_OPER";
const char* PARAM_KEY_APP_DESC = "APP_DESC";
const char* PARAM_KEY_BLE_UDID_HASH = "BLE_UDID_HASH";
const char* PARAM_KEY_CONN_ADDR_TYPE = "CONN_ADDR_TYPE";
const char* PARAM_KEY_PUBLISH_ID = "PUBLISH_ID";
const char* PARAM_KEY_SUBSCRIBE_ID = "SUBSCRIBE_ID";
const char* PARAM_KEY_TARGET_PKG_NAME = "TARGET_PKG_NAME";
const char* PARAM_KEY_PEER_BUNDLE_NAME = "PEER_BUNDLE_NAME";
const char* PARAM_KEY_DISC_FREQ = "DISC_FREQ";
const char* PARAM_KEY_DISC_MEDIUM = "DISC_MEDIUM";
const char* PARAM_KEY_DISC_CAPABILITY = "DISC_CAPABILITY";
const char* PARAM_KEY_DISC_MODE = "DISC_MODE";
const char* PARAM_KEY_AUTO_STOP_ADVERTISE = "AUTO_STOP_ADVERTISE";
const char* PARAM_KEY_FILTER_OPTIONS = "FILTER_OPTIONS";
const char* PARAM_KEY_BIND_EXTRA_DATA = "BIND_EXTRA_DATA";
const char* PARAM_KEY_OS_TYPE = "OS_TYPE";
const char* PARAM_KEY_OS_VERSION = "OS_VERSION";
const char* PARAM_KEY_IS_SHOW_TRUST_DIALOG = "isShowTrustDialog";
const char* PARAM_KEY_UDID = "udid";
const char* PARAM_KEY_UUID = "uuid";
const char* DM_CONNECTION_DISCONNECTED = "DM_CONNECTION_DISCONNECTED";
const char* BIND_LEVEL = "bindLevel";
const char* TOKENID = "tokenId";
const char* DM_BIND_RESULT_NETWORK_ID = "DM_BIND_RESULT_NETWORK_ID";
const char* PARAM_KEY_POLICY_STRATEGY_FOR_BLE = "DM_POLICY_STRATEGY_FOR_BLE";
const char* PARAM_KEY_POLICY_TIME_OUT = "DM_POLICY_TIMEOUT";
const char* DEVICE_SCREEN_STATUS = "DEVICE_SCREEN_STATUS";
const char* PROCESS_NAME = "PROCESS_NAME";
const char* PARAM_CLOSE_SESSION_DELAY_SECONDS = "DM_CLOSE_SESSION_DELAY_SECONDS";
const char* DM_AUTHENTICATION_TYPE = "DM_AUTHENTICATION_TYPE";

const char* PARAM_KEY_CONN_SESSIONTYPE = "connSessionType";
const char* PARAM_KEY_HML_RELEASETIME = "hmlReleaseTime";
const char* PARAM_KEY_HML_ENABLE_160M = "hmlEnable160M";
const char* PARAM_KEY_HML_ACTIONID = "hmlActionId";

const char* CONN_SESSION_TYPE_HML = "HML";
const char* CONN_SESSION_TYPE_BLE = "BLE";
const char* UN_BIND_PARAM_UDID_KEY = "udidKey";

const char* DM_BUSINESS_ID = "business_id";

// errCode map
const std::map<int32_t, int32_t> MAP_ERROR_CODE = {
    { ERR_DM_TIME_OUT, 96929745 }, { ERR_DM_UNSUPPORTED_AUTH_TYPE, 96929760 }, { ERR_DM_AUTH_BUSINESS_BUSY, 96929761 },
    { ERR_DM_AUTH_PEER_REJECT, 96929763 }, { ERR_DM_AUTH_REJECT, 96929764 }, { ERR_DM_CREATE_GROUP_FAILED, 96929768 },
    { ERR_DM_BIND_USER_CANCEL, 96929780 }, { ERR_DM_BIND_USER_CANCEL_ERROR, 96929785 },
    { ERR_DM_AUTH_CODE_INCORRECT, 96929796 }, { ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY, 96929799 },
    { ERR_DM_SYNC_DELETE_DEVICE_REPEATED, 96929801 }
};

// wise device
const int32_t MAX_DEVICE_PROFILE_SIZE = 500;
const char* ACL_IS_LNN_ACL_KEY = "IsLnnAcl";
const char* ACL_IS_LNN_ACL_VAL_TRUE = "true";
const char* ACL_IS_LNN_ACL_VAL_FALSE = "false";

const char* DM_VERSION_5_0_1 = "5.0.1";
const char* DM_VERSION_5_0_2 = "5.0.2";
const char* DM_VERSION_5_0_3 = "5.0.3";
const char* DM_VERSION_5_0_4 = "5.0.4";
const char* DM_VERSION_5_0_5 = "5.0.5";
const char* DM_VERSION_5_1_0 = "5.1.0";
const char* DM_VERSION_5_1_1 = "5.1.1";
const char* DM_CURRENT_VERSION = DM_VERSION_5_1_1;
const char* DM_ACL_AGING_VERSION = DM_VERSION_5_1_0;
const char* DM_VERSION_5_0_OLD_MAX = "5.0.99";       // Estimated highest version number of the old version
const int32_t DM_HO_OSTYPE = -1;
const int32_t DM_OH_OSTYPE = 10;
const char* PEER_UDID = "peer_udid";
const char* PEER_OSTYPE = "peer_ostype";
const char* TIME_STAMP = "time_stamp";
} // namespace DistributedHardware
} // namespace OHOS