/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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


#ifndef DM_DEVICE_INFO_C_H
#define DM_DEVICE_INFO_C_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM_MAX_DEVICE_ID_LEN 97
#define DM_MAX_DEVICE_NAME_LEN 129
#define DM_MAX_DEVICE_CAPABILITY_LEN 65

typedef enum DmNotifyEvent {
    DM_NOTIFY_EVENT_START = 0,
    DM_NOTIFY_EVENT_ONDEVICEREADY,
    DM_NOTIFY_EVENT_ON_PINHOLDER_EVENT,
    DM_NOTIFY_EVENT_BUTT
} DmNotifyEvent;

typedef enum DmDeviceType {
    DEVICE_TYPE_UNKNOWN = 0x00,
    DEVICE_TYPE_WIFI_CAMERA = 0x08,
    DEVICE_TYPE_AUDIO = 0x0A,
    DEVICE_TYPE_PC = 0x0C,
    DEVICE_TYPE_PHONE = 0x0E,
    DEVICE_TYPE_PAD = 0x11,
    DEVICE_TYPE_WATCH = 0x6D,
    DEVICE_TYPE_CAR = 0x83,
    DEVICE_TYPE_TV = 0x9C,
    DEVICE_TYPE_SMART_DISPLAY = 0xA02,
    DEVICE_TYPE_2IN1 = 0xA2F,
    THIRD_TV = 0x2E,
    DEVICE_TYPE_GLASSES = 0xA31
} DmDeviceType;

typedef enum DmDeviceState {
    DEVICE_STATE_UNKNOWN = -1,
    DEVICE_STATE_ONLINE = 0,
    DEVICE_INFO_READY = 1,
    DEVICE_STATE_OFFLINE = 2,
    DEVICE_INFO_CHANGED = 3,
} DmDeviceState;

typedef enum DmAuthForm {
    DM_AUTH_FORM_INVALID_TYPE = -1,
    DM_AUTH_FORM_PEER_TO_PEER = 0,
    DM_AUTH_FORM_IDENTICAL_ACCOUNT = 1,
    DM_AUTH_FORM_ACROSS_ACCOUNT = 2,
    DM_AUTH_FORM_SHARE = 3
} DmAuthForm;

typedef enum DmNetworkType {
    DM_NETWORK_TYPE_UNKNOWN = 0,
    DM_NETWORK_TYPE_WIFI = 1,
    DM_NETWORK_TYPE_BLE = 2,
    DM_NETWORK_TYPE_BR = 3,
    DM_NETWORK_TYPE_P2P = 4,
    DM_NETWORK_TYPE_USB = 8,
    DM_NETWORK_TYPE_COUNT = 5
} DmNetworkType;

typedef enum DmCommonNotifyEvent {
    DM_COMMON_NOTIFY_MIN = 0,
    REG_DEVICE_STATE = 1,
    UN_REG_DEVICE_STATE = 2,
    REG_DEVICE_SCREEN_STATE = 3,
    UN_REG_DEVICE_SCREEN_STATE = 4,
    REG_REMOTE_DEVICE_TRUST_CHANGE = 5,
    UN_REG_REMOTE_DEVICE_TRUST_CHANGE = 6,
    REG_CREDENTIAL_AUTH_STATUS_NOTIFY = 7,
    UN_REG_CREDENTIAL_AUTH_STATUS_NOTIFY = 8,
    REG_SERVICE_STATE = 11,
    UN_REG_SERVICE_STATE = 12,
    DM_COMMON_NOTIFY_MAX = 13
} DmCommonNotifyEvent;

#define DM_NOTIFY_EVENT_MIN DM_COMMON_NOTIFY_MIN
#define DM_NOTIFY_EVENT_MAX DM_COMMON_NOTIFY_MAX

typedef enum DMLocalServiceInfoAuthType {
    DM_LOCAL_SERVICE_INFO_AUTH_TYPE_TRUST_ONETIME = 0,
    DM_LOCAL_SERVICE_INFO_AUTH_TYPE_CANCEL = 1,
    DM_LOCAL_SERVICE_INFO_AUTH_TYPE_TRUST_ALWAYS = 6,
    DM_LOCAL_SERVICE_INFO_AUTH_TYPE_MAX = 7
} DMLocalServiceInfoAuthType;

typedef enum DMLocalServiceInfoPinExchangeType {
    DM_LOCAL_SERVICE_INFO_PIN_EXCHANGE_PINBOX = 1,
    DM_LOCAL_SERVICE_INFO_PIN_EXCHANGE_QR_FROMDP = 2,
    DM_LOCAL_SERVICE_INFO_PIN_EXCHANGE_FROMDP = 3,
    DM_LOCAL_SERVICE_INFO_PIN_EXCHANGE_IMPORT_AUTH_CODE = 5,
    DM_LOCAL_SERVICE_INFO_PIN_EXCHANGE_ULTRASOUND = 6,
    DM_LOCAL_SERVICE_INFO_PIN_EXCHANGE_MAX = 7
} DMLocalServiceInfoPinExchangeType;

typedef enum DMLocalServiceInfoAuthBoxType {
    DM_LOCAL_SERVICE_INFO_AUTH_BOX_TYPE_STATE3 = 1,
    DM_LOCAL_SERVICE_INFO_AUTH_BOX_TYPE_SKIP_CONFIRM = 2,
    DM_LOCAL_SERVICE_INFO_AUTH_BOX_TYPE_TWO_IN1 = 3,
    DM_LOCAL_SERVICE_INFO_AUTH_BOX_TYPE_MAX = 4
} DMLocalServiceInfoAuthBoxType;

typedef enum DmDiscoverMode {
    DM_DISCOVER_MODE_PASSIVE = 0x55,
    DM_DISCOVER_MODE_ACTIVE = 0xAA
} DmDiscoverMode;

typedef enum DmExchangeMedium {
    DM_AUTO = 0,
    DM_BLE = 1,
    DM_COAP = 2,
    DM_USB = 3,
    DM_MEDIUM_BUTT = 4
} DmExchangeMedium;

typedef enum DmExchangeFreq {
    DM_LOW = 0,
    DM_MID = 1,
    DM_HIGH = 2,
    DM_SUPER_HIGH = 3,
    DM_EXTREME_HIGH = 4,
    DM_FREQ_BUTT = 5
} DmExchangeFreq;

typedef enum DMSrvDiscoveryMode {
    SERVICE_PUBLISH_MODE_PASSIVE = 0x15,
    SERVICE_PUBLISH_MODE_ACTIVE = 0x25
} DMSrvDiscoveryMode;

typedef enum DMSrvMediumType {
    SERVICE_MEDIUM_TYPE_AUTO = 0,
    SERVICE_MEDIUM_TYPE_BLE = 1,
    SERVICE_MEDIUM_TYPE_BLE_TRIGGER = 2,
    SERVICE_MEDIUM_TYPE_MDNS = 3,
    SERVICE_MEDIUM_TYPE_BUTT = 4
} DMSrvMediumType;

typedef enum DMPublishState {
    SERVICE_UNPUBLISHED_STATE = 0,
    SERVICE_PUBLISHED_STATE = 1
} DMPublishState;

typedef enum DmServiceState {
    SERVICE_STATE_UNKNOWN = -1,
    SERVICE_STATE_ONLINE = 0,
    SERVICE_STATE_OFFLINE = 2,
    SERVICE_INFO_CHANGED = 3
} DmServiceState;

typedef struct {
    int32_t publishId;
    DmDiscoverMode mode;
    DmExchangeFreq freq;
    bool ranging;
    DmExchangeMedium medium;
    char capability[DM_MAX_DEVICE_CAPABILITY_LEN];
} DmPublishInfo;

void DmPublishInfoInit(DmPublishInfo* info);
void DmPublishInfoDestroy(DmPublishInfo* info);

typedef struct {
    char deviceId[DM_MAX_DEVICE_ID_LEN];
    char deviceName[DM_MAX_DEVICE_NAME_LEN];
    uint16_t deviceTypeId;
    char networkId[DM_MAX_DEVICE_ID_LEN];
    int32_t range;
    int32_t networkType;
    DmAuthForm authForm;
    DmString extraData;
} DmDeviceInfo;

void DmDeviceInfoInit(DmDeviceInfo* info);
void DmDeviceInfoDestroy(DmDeviceInfo* info);

typedef struct {
    char deviceId[DM_MAX_DEVICE_ID_LEN];
    char deviceName[DM_MAX_DEVICE_NAME_LEN];
    uint16_t deviceTypeId;
    char networkId[DM_MAX_DEVICE_ID_LEN];
    DmString extraData;
} DmDeviceBasicInfo;

void DmDeviceBasicInfoInit(DmDeviceBasicInfo* info);
void DmDeviceBasicInfoDestroy(DmDeviceBasicInfo* info);
DmDeviceInfo DmDeviceBasicInfoToDeviceInfo(const DmDeviceBasicInfo* basic);

typedef struct {
    DmString deviceId;
    DmString networkId;
    DmString brMac;
    DmString bleMac;
    DmString wifiIp;
    uint16_t wifiPort;
    int64_t serviceId;
} PeerTargetId;

typedef PeerTargetId DmPeerTargetId;

void DmPeerTargetIdInit(PeerTargetId* id);
void DmPeerTargetIdDestroy(PeerTargetId* id);

typedef struct {
    DmString accountId;
    DmString pkgName;
    DmString networkId;
    int32_t userId;
    uint64_t tokenId;
    DmString extra;
} DmAccessCaller;

void DmAccessCallerInit(DmAccessCaller* caller);
void DmAccessCallerDestroy(DmAccessCaller* caller);

typedef struct {
    DmString accountId;
    DmString networkId;
    DmString peerId;
    DmString pkgName;
    int32_t userId;
    uint64_t tokenId;
    DmString extra;
} DmAccessCallee;

void DmAccessCalleeInit(DmAccessCallee* callee);
void DmAccessCalleeDestroy(DmAccessCallee* callee);

typedef struct {
    int32_t userId;
    DmString pkgName;
    uint32_t tokenId;
} ProcessInfo;

void DmProcessInfoInit(ProcessInfo* info);
void DmProcessInfoDestroy(ProcessInfo* info);
int DmProcessInfoCmp(ProcessInfo a, ProcessInfo b);

DM_SET_DEFINE(ProcessInfo);
DM_VEC_DEFINE(ProcessInfo);
DM_MAP_DEFINE(int_DmSet_ProcessInfo, int, DmSet_ProcessInfo);

typedef struct {
    int32_t processUserId;
    DmString processPkgName;
    int32_t notifyUserId;
    DmString pkgName;
    DmString subId;
    DmString udid;
} DmNotifyKey;

void DmNotifyKeyInit(DmNotifyKey* key);
void DmNotifyKeyDestroy(DmNotifyKey* key);

typedef struct {
    DmString bundleName;
    int32_t authBoxType;
    int32_t authType;
    int32_t pinExchangeType;
    DmString pinCode;
    DmString description;
    DmString extraInfo;
} DMLocalServiceInfo;

void DmLocalServiceInfoInit(DMLocalServiceInfo* info);
void DmLocalServiceInfoDestroy(DMLocalServiceInfo* info);

DM_VEC_DEFINE(uint64_t);
DM_VEC_DEFINE(DmDeviceInfo);
DM_VEC_DEFINE(DmDeviceBasicInfo);
DM_VEC_DEFINE(DMLocalServiceInfo);
DM_MAP_DEFINE(DmString_DmDeviceInfo, DmString, DmDeviceInfo);
DM_HMAP_DEFINE(DmString_DmDeviceInfo, DmString, DmDeviceInfo);

typedef struct {
    DmString deviceId;
    int32_t userId;
} DevUserInfo;

void DmDevUserInfoInit(DevUserInfo* info);
void DmDevUserInfoDestroy(DevUserInfo* info);

typedef struct {
    DmString localUdid;
    int32_t localUserId;
    DmString peerUdid;
    int32_t peerUserId;
} DMAclQuadInfo;

void DmAclQuadInfoInit(DMAclQuadInfo* info);
void DmAclQuadInfoDestroy(DMAclQuadInfo* info);

typedef struct {
    int64_t serviceId;
    DmString serviceType;
    DmString serviceName;
    DmString serviceDisplayName;
} ServiceInfo;

void DmServiceInfoStructInit(ServiceInfo* info);
void DmServiceInfoStructDestroy(ServiceInfo* info);

typedef struct {
    ServiceInfo serviceInfo;
    DmString pkgName;
} DiscoveryServiceInfo;

void DmDiscoveryServiceInfoInit(DiscoveryServiceInfo* info);
void DmDiscoveryServiceInfoDestroy(DiscoveryServiceInfo* info);

typedef struct {
    int32_t regServiceId;
    DmString deviceId;
    int32_t userId;
    int64_t tokenId;
    int8_t publishState;
    int64_t serviceId;
    DmString serviceType;
    DmString serviceName;
    DmString serviceDisplayName;
} ServiceInfoProfile;

void DmServiceInfoProfileInit(ServiceInfoProfile* info);
void DmServiceInfoProfileDestroy(ServiceInfoProfile* info);

typedef struct {
    ServiceInfo serviceInfo;
    DmString customData;
    uint32_t dataLen;
} ServiceRegInfo;

void DmServiceRegInfoInit(ServiceRegInfo* info);
void DmServiceRegInfoDestroy(ServiceRegInfo* info);

typedef struct {
    ServiceInfo serviceInfo;
    DMSrvDiscoveryMode discoverMode;
    int32_t regServiceId;
    DMSrvMediumType media;
    DmExchangeFreq freq;
} PublishServiceParam;

void DmPublishServiceParamInit(PublishServiceParam* param);
void DmPublishServiceParamDestroy(PublishServiceParam* param);

typedef struct {
    int32_t userId;
    int64_t serviceId;
    int64_t displayId;
    int8_t publishState;
    DmString deviceId;
    DmString networkId;
    DmAuthForm authform;
    uint64_t serviceOwnerTokenId;
    DmString serviceOwnerPkgName;
    uint64_t serviceRegisterTokenId;
    DmString serviceType;
    DmString serviceName;
    DmString serviceDisplayName;
    DmString serviceCode;
    DmString customData;
    uint32_t dataLen;
    int64_t timeStamp;
    DmString description;
} DmServiceInfo;

void DmServiceInfoInit(DmServiceInfo* info);
void DmServiceInfoDestroy(DmServiceInfo* info);

typedef struct {
    DmString pkgName;
    int32_t localUserId;
    DmString networkId;
    int64_t serviceId;
    int32_t callerUserId;
    uint32_t callerTokenId;
} ServiceSyncInfo;

void DmServiceSyncInfoInit(ServiceSyncInfo* info);
void DmServiceSyncInfoDestroy(ServiceSyncInfo* info);

typedef struct {
    int32_t userId;
    int64_t displayId;
    uint64_t serviceOwnerTokenId;
    DmString serviceOwnerPkgName;
    uint64_t serviceRegisterTokenId;
    DmString serviceType;
    DmString serviceName;
    DmString serviceDisplayName;
    DmString customData;
    DmString serviceCode;
    uint32_t dataLen;
    int64_t timeStamp;
    DmString description;
} DmRegisterServiceInfo;

void DmRegisterServiceInfoInit(DmRegisterServiceInfo* info);
void DmRegisterServiceInfoDestroy(DmRegisterServiceInfo* info);

typedef struct {
    DMSrvDiscoveryMode discoverMode;
    DMSrvMediumType media;
    DmExchangeFreq freq;
} DmPublishServiceParam;

void DmPublishServiceParamCInit(DmPublishServiceParam* param);
void DmPublishServiceParamCDestroy(DmPublishServiceParam* param);

typedef struct {
    DmString serviceType;
    DmString serviceName;
    DmString serviceDisplayName;
    DmExchangeFreq freq;
    DMSrvMediumType medium;
    DMSrvDiscoveryMode mode;
} DmDiscoveryServiceParam;

void DmDiscoveryServiceParamInit(DmDiscoveryServiceParam* param);
void DmDiscoveryServiceParamDestroy(DmDiscoveryServiceParam* param);

DM_MMAP_DEFINE(int_int64_t, int, int64_t);

typedef struct {
    int64_t localTokenId;
    DmString localPkgName;
    int32_t bindType;
    DmString peerUdid;
    DmVecInt64_t serviceIds;
    bool isActive;
    int32_t peerUserId;
    DmMmap_int_int64_t bindTypeToServiceIdMap;
} DmUserRemovedServiceInfo;

void DmUserRemovedServiceInfoInit(DmUserRemovedServiceInfo* info);
void DmUserRemovedServiceInfoDestroy(DmUserRemovedServiceInfo* info);

typedef struct {
    int32_t userId;
    uint64_t tokenId;
    DmString pkgName;
    int64_t serviceId;
} DmRegisterServiceState;

void DmRegisterServiceStateInit(DmRegisterServiceState* state);
void DmRegisterServiceStateDestroy(DmRegisterServiceState* state);

typedef struct {
    uint16_t subscribeId;
    DmDiscoverMode mode;
    DmExchangeMedium medium;
    DmExchangeFreq freq;
    bool isSameAccount;
    bool isWakeRemote;
    char capability[DM_MAX_DEVICE_CAPABILITY_LEN];
} DmSubscribeInfo;

void DmSubscribeInfoInit(DmSubscribeInfo* info);
void DmSubscribeInfoDestroy(DmSubscribeInfo* info);

DM_VEC_DEFINE(DmServiceInfo);
DM_VEC_DEFINE(DmRegisterServiceInfo);

#ifdef __cplusplus
}
#endif

#endif
