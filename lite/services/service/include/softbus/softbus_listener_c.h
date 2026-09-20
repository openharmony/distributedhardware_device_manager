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


#ifndef DM_SOFTBUS_LISTENER_C_H
#define DM_SOFTBUS_LISTENER_C_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_device_info_c.h"
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DmSoftbusEventType {
    DM_EVENT_TYPE_UNKNOWN = 0,
    DM_EVENT_TYPE_ONLINE = 1,
    DM_EVENT_TYPE_OFFLINE = 2,
    DM_EVENT_TYPE_CHANGED = 3,
    DM_EVENT_TYPE_SCREEN = 4,
} DmSoftbusEventType;

typedef struct DmSoftbusEvent {
    DmDeviceInfo dmDeviceInfo;
    DmSoftbusEventType eventType;
} DmSoftbusEvent;

int32_t DmSoftbusListenerGetTrustedDeviceList(DmVec_DmDeviceInfo* deviceInfoList);
int32_t DmSoftbusListenerGetLocalDeviceInfo(DmDeviceInfo* deviceInfo);
int32_t DmSoftbusListenerGetDeviceInfo(const DmString* networkId, DmDeviceInfo* info);

int32_t DmSoftbusListenerGetUuidByNetworkId(const char* networkId, DmString* uuid);
int32_t DmSoftbusListenerGetUdidByNetworkId(const char* networkId, DmString* udid);
int32_t DmSoftbusListenerGetDeviceSecurityLevel(const char* networkId, int32_t* securityLevel);
int32_t DmSoftbusListenerGetNetworkTypeByNetworkId(const char* networkId, int32_t* networkType);

int32_t DmSoftbusListenerRefreshSoftbusLnn(const char* pkgName, const DmSubscribeInfo* dmSubInfo,
    const DmString* customData);
int32_t DmSoftbusListenerStopRefreshSoftbusLnn(uint16_t subscribeId);
int32_t DmSoftbusListenerPublishSoftbusLnn(const DmPublishInfo* dmPubInfo, const DmString* capability,
    const DmString* customData);
int32_t DmSoftbusListenerStopPublishSoftbusLnn(int32_t publishId);

int32_t DmSoftbusListenerRegisterSoftbusLnnOpsCbk(const DmString* pkgName);
int32_t DmSoftbusListenerUnregisterSoftbusLnnOpsCbk(const DmString* pkgName);

int32_t DmSoftbusListenerSetLocalDisplayName(const DmString* displayName);

void DmSoftbusListenerOnSoftbusDeviceFound(const void* device);
void DmSoftbusListenerOnSoftbusDiscoveryResult(int subscribeId, int result);
void DmSoftbusListenerOnSoftbusDeviceOnline(void* info);
void DmSoftbusListenerOnSoftbusDeviceOffline(void* info);
void DmSoftbusListenerOnSoftbusDeviceInfoChanged(int type, void* info);
void DmSoftbusListenerOnSoftbusPublishResult(int publishId, int result);

int32_t DmSoftbusListenerRegisterCallbacksInner(void);

#ifdef __cplusplus
}
#endif

#endif
