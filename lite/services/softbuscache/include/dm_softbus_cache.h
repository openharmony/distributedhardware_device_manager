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


#ifndef DM_SOFTBUS_CACHE_H
#define DM_SOFTBUS_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_device_info_c.h"
#include "dm_single_instance.h"
#include "dm_thread.h"
#include "dm_container.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"

#ifdef __cplusplus
extern "C" {
#endif

DM_PAIR_DEFINE(DmString_DmDeviceInfo, DmString, DmDeviceInfo);

DM_HMAP_DEFINE(DmString_DmDeviceInfoPair, DmString, DmPair_DmString_DmDeviceInfo);
DM_HMAP_DEFINE(DmString_int, DmString, int);

typedef struct SoftbusCache {
    DmMutex deviceInfosMutex;
    DmMutex deviceSecurityLevelMutex;
    DmMutex localDevInfoMutex;
    DmHmap_DmString_DmDeviceInfoPair deviceInfo;
    DmHmap_DmString_int deviceSecurityLevel;
    DmDeviceInfo localDeviceInfo;
    bool online;
    bool getLocalDevInfo;
} SoftbusCache;

DM_DECLARE_SINGLE_INSTANCE(SoftbusCache);

void DmSoftbusCacheInit(SoftbusCache* cache);

void DmSoftbusCacheSaveDeviceInfo(SoftbusCache* cache, const DmDeviceInfo* deviceInfo);
void DmSoftbusCacheDeleteDeviceInfoByNode(SoftbusCache* cache, const DmDeviceInfo* nodeInfo);
void DmSoftbusCacheSaveDeviceSecurityLevel(SoftbusCache* cache, const char* networkId);
void DmSoftbusCacheDeleteDeviceSecurityLevel(SoftbusCache* cache, const char* networkId);
bool DmSoftbusCacheGetDeviceInfoByDeviceId(SoftbusCache* cache, const DmString* deviceId,
    DmString* uuid, DmDeviceInfo* devInfo);
void DmSoftbusCacheUpdateDeviceInfoCache(SoftbusCache* cache);
void DmSoftbusCacheSaveLocalDeviceInfo(SoftbusCache* cache);
int32_t DmSoftbusCacheGetDevInfoByNetworkId(SoftbusCache* cache, const DmString* networkId,
    DmDeviceInfo* nodeInfo);
int32_t DmSoftbusCacheGetUdidByUdidHash(SoftbusCache* cache, const DmString* udidHash, DmString* udid);
int32_t DmSoftbusCacheGetNetworkIdFromCache(SoftbusCache* cache, const DmString* udid, DmString* networkId);
bool DmSoftbusCacheCheckIsOnlineByPeerUdid(SoftbusCache* cache, const DmString* peerUdid);

#ifdef __cplusplus
}
#endif

#endif
