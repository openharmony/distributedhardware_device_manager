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


#ifndef DM_DEVICE_INFO_H
#define DM_DEVICE_INFO_H

#include "dm_device_info_c.h"
#include "dm_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

DM_EXPORT extern const char* DEVICE_TYPE_UNKNOWN_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_PHONE_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_PAD_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_TV_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_CAR_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_WATCH_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_WIFICAMERA_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_PC_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_SMART_DISPLAY_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_2IN1_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_GLASSES_STRING;

bool DmPeerTargetIdEqual(const PeerTargetId* a, const PeerTargetId* b);
bool DmPeerTargetIdLess(const PeerTargetId* a, const PeerTargetId* b);

bool DmProcessInfoEqual(const ProcessInfo* a, const ProcessInfo* b);
bool DmProcessInfoLess(const ProcessInfo* a, const ProcessInfo* b);

bool DmNotifyKeyEqual(const DmNotifyKey* a, const DmNotifyKey* b);
bool DmNotifyKeyLess(const DmNotifyKey* a, const DmNotifyKey* b);

bool DmServiceInfoEqual(const DmServiceInfo* a, const DmServiceInfo* b);
bool DmServiceInfoLess(const DmServiceInfo* a, const DmServiceInfo* b);

bool DmServiceSyncInfoEqual(const ServiceSyncInfo* a, const ServiceSyncInfo* b);
bool DmServiceSyncInfoLess(const ServiceSyncInfo* a, const ServiceSyncInfo* b);

bool DmDiscoveryServiceParamEqual(const DmDiscoveryServiceParam* a, const DmDiscoveryServiceParam* b);

bool DmRegisterServiceStateEqual(const DmRegisterServiceState* a, const DmRegisterServiceState* b);

#ifdef __cplusplus
}
#endif

#endif
