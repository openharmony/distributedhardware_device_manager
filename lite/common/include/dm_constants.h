/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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


#ifndef DM_CONSTANTS_H
#define DM_CONSTANTS_H

#include "dm_error_type.h"
#include "dm_container.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

#define DM_TAG_GROUP_ID "groupId"
#define DM_TAG_GROUP_NAME "GROUPNAME"
#define DM_TAG_REQUEST_ID "REQUESTID"
#define DM_TAG_DEVICE_ID "DEVICEID"
#define DM_TAG_AUTH_TYPE "AUTHTYPE"
#define DM_TAG_CRYPTO_SUPPORT "CRYPTOSUPPORT"
#define DM_TAG_VER "ITF_VER"
#define DM_TAG_MSG_TYPE "MSG_TYPE"
#define DM_ITF_VER "1.1"
#define DM_PKG_NAME "ohos.distributedhardware.devicemanager"
#define DM_PKG_NAME_LITE "ohos.distributedhardware.devicemanager"
#define DM_ALL_PKGNAME "all"
#define DM_SYNC_USERID_SESSION_NAME "ohos.distributedhardware.devicemanager.syncuserid"

#define DM_CAPABILITY_OSD "osdCapability"
#define DM_CAPABILITY_APPROACH "approach"
#define DM_CAPABILITY_OH_APPROACH "oh_approach"
#define DM_CAPABILITY_TOUCH "touch"
#define DM_CAPABILITY_CASTPLUS "castPlus"
#define DM_CAPABILITY_VIRTUAL_LINK "virtualLink"
#define DM_CAPABILITY_SHARE "share"
#define DM_CAPABILITY_WEAR "wear"
#define DM_CAPABILITY_OOP "oop"
#define DM_CREDENTIAL_TYPE "CREDENTIAL_TYPE"
#define DM_CREDENTIAL_REQJSONSTR "CREDENTIAL_REQJSONSTR"
#define DM_CREDENTIAL_RETURNJSONSTR "CREDENTIAL_RETURNJSONSTR"
#define DM_DEVICE_MANAGER_GROUPNAME "DMSameAccountGroup"
#define DM_FIELD_CREDENTIAL_EXISTS "isCredentialExists"
#define DM_TYPE_MINE "MINE"
#define DM_TYPE_OH "OH"
#define DM_TAG_SESSION_HEARTBEAT "session_heartbeat"
#define DM_TAG_BUNDLE_NAME "bundleName"
#define DM_TAG_TOKENID "tokenId"
#define DM_TAG_REMAINING_FROZEN_TIME "remainingFrozenTime"
#define DM_TAG_SERVICE_ID_EXT "serviceId"
#define DM_EXT_PART "ext_part"

#define DM_AUTH_TYPE "authType"
#define DM_APP_OPERATION "appOperation"
#define DM_CUSTOM_DESCRIPTION "customDescription"
#define DM_TOKEN "token"
#define DM_CHECK_AUTH_ALWAYS_POS 0
#define DM_AUTH_ALWAYS '1'
#define DM_AUTH_ONCE '0'
#define DM_TAG_TARGET_DEVICE_NAME "targetDeviceName"

#define DM_SERVICE_INIT_TRY_MAX_NUM 200
#define DM_DEVICE_UUID_LENGTH 65
#define DM_DEVICE_NETWORKID_LENGTH 100
#define DM_GROUP_TYPE_INVALID_GROUP (-1)
#define DM_GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP 1
#define DM_GROUP_TYPE_PEER_TO_PEER_GROUP 256
#define DM_GROUP_TYPE_ACROSS_ACCOUNT_GROUP 1282
#define DM_GROUP_VISIBILITY_PUBLIC (-1)
#define DM_MIN_REQUEST_ID 1000000000LL
#define DM_MAX_REQUEST_ID 9999999999LL
#define DM_DEVICEID_LEN 8

#define DM_FILTER_PARA_RANGE "FILTER_RANGE"
#define DM_FILTER_PARA_DEVICE_TYPE "FILTER_DEVICE_TYPE"
#define DM_FILTER_PARA_INCLUDE_TRUST "FILTER_INCLUDE_TRUST"

#define DM_CONN_ADDR_TYPE_ID "ID_TYPE"
#define DM_CONN_ADDR_TYPE_BR "BR_TYPE"
#define DM_CONN_ADDR_TYPE_BLE "BLE_TYPE"
#define DM_CONN_ADDR_TYPE_USB "USB_TYPE"
#define DM_CONN_ADDR_TYPE_WLAN_IP "WLAN_IP_TYPE"
#define DM_CONN_ADDR_TYPE_ETH_IP "ETH_IP_TYPE"
#define DM_CONN_ADDR_TYPE_NCM "NCM_TYPE"

#define DM_CONNECTION_ADDR_USB 5

#define DM_PARAM_KEY_META_TYPE "META_TYPE"
#define DM_PARAM_KEY_TARGET_ID "TARGET_ID"
#define DM_PARAM_KEY_BR_MAC "BR_MAC"
#define DM_PARAM_KEY_BLE_MAC "BLE_MAC"
#define DM_PARAM_KEY_WIFI_IP "WIFI_IP"
#define DM_PARAM_KEY_WIFI_PORT "WIFI_PORT"
#define DM_PARAM_KEY_USB_IP "USB_IP"
#define DM_PARAM_KEY_USB_PORT "USB_PORT"
#define DM_PARAM_KEY_NCM_IP "NCM_IP"
#define DM_PARAM_KEY_NCM_PORT "NCM_PORT"
#define DM_PARAM_KEY_ACCOUNT_HASH "ACCOUNT_HASH"
#define DM_PARAM_KEY_AUTH_TYPE "AUTH_TYPE"
#define DM_PARAM_KEY_APP_OPER "APP_OPER"
#define DM_PARAM_KEY_APP_DESC "APP_DESC"
#define DM_PARAM_KEY_BLE_UDID_HASH "BLE_UDID_HASH"
#define DM_PARAM_KEY_CUSTOM_DATA "CUSTOM_DATA"
#define DM_PARAM_KEY_CONN_ADDR_TYPE "CONN_ADDR_TYPE"
#define DM_PARAM_KEY_PUBLISH_ID "PUBLISH_ID"
#define DM_PARAM_KEY_SUBSCRIBE_ID "SUBSCRIBE_ID"
#define DM_PARAM_KEY_TARGET_PKG_NAME "TARGET_PKG_NAME"
#define DM_PARAM_KEY_PEER_BUNDLE_NAME "PEER_BUNDLE_NAME"
#define DM_PARAM_KEY_DISC_FREQ "DISC_FREQ"
#define DM_PARAM_KEY_DISC_MEDIUM "DISC_MEDIUM"
#define DM_PARAM_KEY_DISC_CAPABILITY "DISC_CAPABILITY"
#define DM_PARAM_KEY_DISC_MODE "DISC_MODE"
#define DM_PARAM_KEY_AUTO_STOP_ADVERTISE "AUTO_STOP_ADVERTISE"
#define DM_PARAM_KEY_FILTER_OPTIONS "FILTER_OPTIONS"
#define DM_PARAM_KEY_OS_TYPE "OS_TYPE"
#define DM_PARAM_KEY_OS_VERSION "OS_VERSION"
#define DM_PARAM_KEY_IS_SHOW_TRUST_DIALOG "isShowTrustDialog"
#define DM_PARAM_KEY_SCREEN_ID "screenId"
#define DM_PARAM_KEY_SCREEN_TYPE "screenType"
#define DM_PARAM_KEY_UDID "udid"
#define DM_PARAM_KEY_UUID "uuid"
#define DM_CONNECTION_DISCONNECTED "DM_CONNECTION_DISCONNECTED"
#define DM_TOKENID "tokenId"
#define DM_PARAM_KEY_POLICY_STRATEGY_FOR_BLE "DM_POLICY_STRATEGY_FOR_BLE"
#define DM_PARAM_KEY_POLICY_TIME_OUT "DM_POLICY_TIMEOUT"
#define DM_DEVICE_SCREEN_STATUS "DEVICE_SCREEN_STATUS"
#define DM_PROCESS_NAME "PROCESS_NAME"
#define DM_PARAM_CLOSE_SESSION_DELAY_SECONDS "DM_CLOSE_SESSION_DELAY_SECONDS"

#define DM_PARAM_KEY_CONN_SESSIONTYPE "connSessionType"
#define DM_PARAM_KEY_HML_RELEASETIME "hmlReleaseTime"
#define DM_PARAM_KEY_HML_ENABLE_160M "hmlEnable160M"
#define DM_PARAM_KEY_HML_ACTIONID "hmlActionId"

#define DM_CONN_SESSION_TYPE_HML "HML"
#define DM_CONN_SESSION_TYPE_BLE "BLE"
#define DM_PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT "isCallingProxyAsSubject"
#define DM_PARAM_KEY_SUBJECT_PROXYED_SUBJECTS "subjectProxyOnes"
#define DM_VAL_TRUE "true"
#define DM_VAL_FALSE "false"
#define DM_APP_USER_DATA "appUserData"
#define DM_SERVICE_USER_DATA "serviceUserData"
#define DM_BUNDLE_INFO "bundleInfo"
#define DM_TITLE "title"
#define DM_BUSINESS_ID "business_id"
#define DM_PARAM_KEY_PEER_PKG_NAME "PEER_PKG_NAME"
#define DM_PARAM_KEY_SUBJECT_SERVICE_ONES "subjectServiceOnes"
#define DM_PARAM_KEY_LOCAL_PKGNAME "local_pkgName"
#define DM_PARAM_KEY_LOCAL_TOKENID "local_tokenId"
#define DM_PARAM_KEY_PEER_SERVICEID "peer_serviceId"
#define DM_PARAM_KEY_LOCAL_USER_ID "localUserId"
#define DM_PARAM_KEY_IS_PROXY_QUERY "isProxyQuery"
#define DM_PARAM_KEY_PKG_NAME "pkgName"
#define DM_PARAM_KEY_TOKEN_ID "tokenId"
#define DM_PARAM_KEY_SERVICE_CODE "serviceCode"

typedef enum DmScreenState {
    DM_SCREEN_UNKNOWN = -1,
    DM_SCREEN_ON = 0,
    DM_SCREEN_OFF = 1
} DmScreenState;

extern DmMap_int_int g_dmMapErrorCode;
int DmMapErrorCodeInit(void);

#define DM_MAX_CONTAINER_SIZE 10000u
#define DM_MAX_DEVICE_PROFILE_SIZE 500
#define DM_DEVICE_NAME_MAX_BYTES 100
#define DM_DEFAULT_DELAY_CLOSE_TIME_US 500000

#define DM_ACL_IS_LNN_ACL_KEY "IsLnnAcl"
#define DM_ACL_IS_LNN_ACL_VAL_TRUE "true"
#define DM_ACL_IS_LNN_ACL_VAL_FALSE "false"
#define DM_SERVICE_ID_KEY "serviceId"

#define DM_ACL_LIFE_CYCLE_DAYS "ACL_LIFE_CYCLE_DAYS"
#define DM_ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED (-1)
#define DM_ACL_LIFE_CYCLE_DAYS_MIN 1
#define DM_ACL_LIFE_CYCLE_DAYS_MAX 3650
#define DM_SECONDS_PER_DAY 86400

#define DM_VERSION_5_0_1 "5.0.1"
#define DM_VERSION_5_0_2 "5.0.2"
#define DM_VERSION_5_0_3 "5.0.3"
#define DM_VERSION_5_0_4 "5.0.4"
#define DM_VERSION_5_0_5 "5.0.5"
#define DM_VERSION_5_1_0 "5.1.0"
#define DM_VERSION_5_1_1 "5.1.1"
#define DM_VERSION_5_1_2 "5.1.2"
#define DM_VERSION_5_1_3 "5.1.3"
#define DM_VERSION_5_1_4 "5.1.4"
#define DM_VERSION_5_1_5 "5.1.5"
#define DM_CURRENT_VERSION DM_VERSION_5_1_5
#define DM_ACL_AGING_VERSION DM_VERSION_5_1_0
#define DM_VERSION_5_0_OLD_MAX "5.0.99"
#define DM_OLD_DM_HO_OSTYPE (-1)
#define DM_NEW_DM_HO_OSTYPE 11
#define DM_OH_OSTYPE 10
#define DM_PEER_UDID "peer_udid"
#define DM_PEER_OSTYPE "peer_ostype"
#define DM_TIME_STAMP "time_stamp"
#define DM_ACCOUNT_CONSTRAINT "constraint.distributed.transmission"

#ifndef FIELD_CONFIRMATION
#define FIELD_CONFIRMATION "confirmation"
#endif
#ifndef DM_REQUEST_REJECTED
#define DM_REQUEST_REJECTED 0
#endif
#ifndef DM_REQUEST_REJECTED_STR
#define DM_REQUEST_REJECTED_STR "0"
#endif
#ifndef DM_REQUEST_ACCEPTED
#define DM_REQUEST_ACCEPTED 1
#endif
#ifndef DM_REQUEST_ACCEPTED_STR
#define DM_REQUEST_ACCEPTED_STR "1"
#endif
#ifndef FIELD_PIN_CODE
#define FIELD_PIN_CODE "pinCode"
#endif
#ifndef FIELD_CRED_ID
#define FIELD_CRED_ID "credId"
#endif
#ifndef FIELD_SERVICE_PKG_NAME
#define FIELD_SERVICE_PKG_NAME "servicePkgName"
#endif
#endif
