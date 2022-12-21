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
};

constexpr const char* TAG_REPLY = "REPLY";
constexpr const char* TAG_NET_ID = "NETID";
constexpr const char* TAG_TARGET = "TARGET";
constexpr const char* TAG_APP_NAME = "APPNAME";
constexpr const char* TAG_GROUPIDS = "GROUPIDLIST";
constexpr const char* TAG_APP_DESCRIPTION = "APPDESC";
constexpr const char* TAG_LOCAL_DEVICE_ID = "LOCALDEVICEID";
constexpr const char* TAG_INDEX = "INDEX";
constexpr const char* TAG_SLICE_NUM = "SLICE";
constexpr const char* TAG_TOKEN = "TOKEN";
constexpr const char* TAG_GROUP_ID = "groupId";
constexpr const char* TAG_GROUP_NAME = "GROUPNAME";
constexpr const char* TAG_REQUEST_ID = "REQUESTID";
constexpr const char* TAG_DEVICE_ID = "DEVICEID";
constexpr const char* TAG_AUTH_TYPE = "AUTHTYPE";
constexpr const char* TAG_CRYPTO_SUPPORT = "CRYPTOSUPPORT";
constexpr const char* TAG_CRYPTO_NAME = "CRYPTONAME";
constexpr const char* TAG_CRYPTO_VERSION = "CRYPTOVERSION";
constexpr const char* TAG_IDENTICAL_ACCOUNT = "IDENTICALACCOUNT";
constexpr const char* TAG_VER = "ITF_VER";
constexpr const char* TAG_MSG_TYPE = "MSG_TYPE";
constexpr const char* DM_ITF_VER = "1.1";
constexpr const char* APP_THUMBNAIL = "appThumbnail";
constexpr const char* DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
constexpr const char* DM_SESSION_NAME = "ohos.distributedhardware.devicemanager.resident";
const static char *DM_CAPABILITY_OSD = "osdCapability";

// Auth
constexpr const char* AUTH_TYPE = "authType";
constexpr const char* TOKEN = "token";
constexpr const char* PIN_TOKEN = "pinToken";
constexpr const char* PIN_CODE_KEY = "pinCode";
constexpr const char* NFC_CODE_KEY = "nfcCode";
constexpr const char* QR_CODE_KEY = "qrCode";
constexpr const char* TAG_AUTH_TOKEN = "authToken";

// HiChain
const int32_t AUTH_TYPE_PIN = 1;
const int32_t SERVICE_INIT_TRY_MAX_NUM = 200;
const int32_t DEVICE_UUID_LENGTH = 65;
const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1;
const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP = 256;
const int32_t GROUP_VISIBILITY_PUBLIC = -1;
const int64_t MIN_REQUEST_ID = 1000000000;
const int64_t MAX_REQUEST_ID = 9999999999;

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

constexpr uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CONSTANTS_H