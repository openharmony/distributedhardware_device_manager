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


#ifndef DM_SERVICE_H
#define DM_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"
#include "dm_device_info_c.h"
#include "dm_thread.h"
#include "dm_single_instance.h"
#include "dm_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeviceManagerService {
    int32_t dummy;
} DeviceManagerService;

DM_DECLARE_SINGLE_INSTANCE(DeviceManagerService);

int32_t DmServiceInit(void);

int32_t DmServiceGetTrustedDeviceList(const DmString* pkgName, const DmString* extra, DmVec_DmDeviceInfo* deviceList);
int32_t DmServiceShiftLnnGear(const DmString* pkgName, const DmString* callerId, bool isRefresh, bool isWakeUp);
int32_t DmServiceGetLocalDeviceInfo(DmDeviceInfo* info);
int32_t DmServiceGetDeviceInfo(const DmString* networkId, DmDeviceInfo* info);
int32_t DmServiceGetUdidByNetworkId(const DmString* pkgName, const DmString* netWorkId, DmString* udid);
int32_t DmServiceGetUuidByNetworkId(const DmString* pkgName, const DmString* netWorkId, DmString* uuid);
int32_t DmServicePublishDeviceDiscovery(const DmString* pkgName, const DmPublishInfo* publishInfo);
int32_t DmServiceUnpublishDeviceDiscovery(const DmString* pkgName, int32_t publishId);

int32_t DmServiceRequestCredential(const DmString* reqJsonStr, DmString* returnJsonStr);
int32_t DmServiceImportCredential(const DmString* pkgName, const DmString* credentialInfo);
int32_t DmServiceDeleteCredential(const DmString* pkgName, const DmString* deleteInfo);
int32_t DmServiceRegisterCredentialCallback(const DmString* pkgName);
int32_t DmServiceUnregisterCredentialCallback(const DmString* pkgName);
int32_t DmServiceNotifyEvent(const DmString* pkgName, int32_t eventId, const DmString* event);
int32_t DmServiceCheckApiPermission(int32_t permissionLevel, const DmString* pkgName);
int32_t DmServiceGetNetworkTypeByNetworkId(const DmString* pkgName, const DmString* netWorkId, int32_t* networkType);

int32_t DmServiceStartAdvertising(const DmString* pkgName, const DmMap_DmString_DmString* advertiseParam);
int32_t DmServiceStopAdvertising(const DmString* pkgName, const DmMap_DmString_DmString* advertiseParam);
int32_t DmServiceLeaveLnn(const DmString* pkgName, const DmString* networkId);

int32_t DmServiceGetDeviceSecurityLevel(const DmString* pkgName, const DmString* networkId, int32_t* securityLevel);
bool DmServiceCheckAccessControl(const DmAccessCaller* caller, const DmAccessCallee* callee);
int32_t DmServiceSetLocalDisplayNameToSoftbus(const DmString* displayName);

int32_t DmServiceTestTriggerAclWrite(const char* json, uint32_t len);
int32_t DmServiceTestTriggerCredDelete(const char* credId, const char* credInfo);

#ifdef __cplusplus
}
#endif

#endif
