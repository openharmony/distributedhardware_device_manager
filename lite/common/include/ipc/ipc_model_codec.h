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


#ifndef DM_IPC_MODEL_CODEC_H
#define DM_IPC_MODEL_CODEC_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_device_info_c.h"
#include "dm_device_profile_info_c.h"
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool (*write_string)(void* parcel, const char* str);
    bool (*write_int32)(void* parcel, int32_t val);
    bool (*write_int64)(void* parcel, int64_t val);
    bool (*write_uint16)(void* parcel, uint16_t val);
    bool (*write_uint32)(void* parcel, uint32_t val);
    bool (*write_uint64)(void* parcel, uint64_t val);
    bool (*write_bool)(void* parcel, bool val);
    bool (*write_raw_data)(void* parcel, const uint8_t* data, int32_t len);
} DmParcelWriteOps;

typedef struct {
    bool (*read_string)(void* parcel, DmString* out);
    int32_t (*read_int32)(void* parcel);
    int64_t (*read_int64)(void* parcel);
    uint16_t (*read_uint16)(void* parcel);
    uint32_t (*read_uint32)(void* parcel);
    uint64_t (*read_uint64)(void* parcel);
    bool (*read_bool)(void* parcel);
    const uint8_t* (*read_raw_data)(void* parcel, int32_t len);
} DmParcelReadOps;

DmParcelWriteOps* DmParcelWriteOpsCreate(void);
void DmParcelWriteOpsDestroy(DmParcelWriteOps* ops);
DmParcelReadOps* DmParcelReadOpsCreate(void);
void DmParcelReadOpsDestroy(DmParcelReadOps* ops);

bool DmReadHelperString(DmParcelReadOps* ops, void* parcel, DmString* out);
bool DmReadHelperInt32(DmParcelReadOps* ops, void* parcel, int32_t* out);
bool DmReadHelperInt64(DmParcelReadOps* ops, void* parcel, int64_t* out);
bool DmReadHelperUint16(DmParcelReadOps* ops, void* parcel, uint16_t* out);
bool DmReadHelperUint32(DmParcelReadOps* ops, void* parcel, uint32_t* out);
bool DmReadHelperUint64(DmParcelReadOps* ops, void* parcel, uint64_t* out);
bool DmReadHelperBool(DmParcelReadOps* ops, void* parcel, bool* out);

void DmIpcModelCodecDecodeDmDeviceBasicInfo(DmParcelReadOps* readOps, void* parcel, DmDeviceBasicInfo* devInfo);
bool DmIpcModelCodecEncodePeerTargetId(const PeerTargetId* targetId, DmParcelWriteOps* writeOps, void* parcel);
void DmIpcModelCodecDecodePeerTargetId(DmParcelReadOps* readOps, void* parcel, PeerTargetId* targetId);
bool DmIpcModelCodecEncodeDmAccessCaller(const DmAccessCaller* caller, DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecEncodeDmAccessCallee(const DmAccessCallee* callee, DmParcelWriteOps* writeOps, void* parcel);
int32_t DmIpcModelCodecDecodeDmDeviceProfileInfoFilterOptions(DmParcelReadOps* readOps, void* parcel,
    DmDeviceProfileInfoFilterOptions* filterOptions);
bool DmIpcModelCodecEncodeDmDeviceProfileInfoFilterOptions(const DmDeviceProfileInfoFilterOptions* filterOptions,
    DmParcelWriteOps* writeOps, void* parcel);
void DmIpcModelCodecDecodeDmServiceProfileInfo(DmParcelReadOps* readOps, void* parcel, DmServiceProfileInfo* svrInfo);
void DmIpcModelCodecDecodeDmServiceProfileInfos(DmParcelReadOps* readOps, void* parcel,
    DmVec_DmServiceProfileInfo* svrInfos);
void DmIpcModelCodecDecodeDmDeviceProfileInfo(DmParcelReadOps* readOps, void* parcel, DmDeviceProfileInfo* devInfo);
bool DmIpcModelCodecEncodeDmServiceProfileInfo(const DmServiceProfileInfo* svrInfo,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecEncodeDmServiceProfileInfos(DmVec_DmServiceProfileInfo* svrInfos,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecEncodeDmDeviceProfileInfo(const DmDeviceProfileInfo* devInfo,
    DmParcelWriteOps* writeOps, void* parcel);
DmString DmIpcModelCodecGetDeviceIconInfoUniqueKeyFromFilter(const DmDeviceIconInfoFilterOptions* iconFilter);
DmString DmIpcModelCodecGetDeviceIconInfoUniqueKeyFromInfo(const DmDeviceIconInfo* iconInfo);
void DmIpcModelCodecDecodeDmDeviceIconInfo(DmParcelReadOps* readOps, void* parcel, DmDeviceIconInfo* deviceIconInfo);
bool DmIpcModelCodecEncodeDmDeviceIconInfo(const DmDeviceIconInfo* deviceIconInfo,
    DmParcelWriteOps* writeOps, void* parcel);
void DmIpcModelCodecDecodeDmDeviceIconInfoFilterOptions(DmParcelReadOps* readOps, void* parcel,
    DmDeviceIconInfoFilterOptions* filterOptions);
bool DmIpcModelCodecEncodeDmDeviceIconInfoFilterOptions(const DmDeviceIconInfoFilterOptions* filterOptions,
    DmParcelWriteOps* writeOps, void* parcel);
void DmIpcModelCodecDecodeDmDeviceInfo(DmParcelReadOps* readOps, void* parcel, DmDeviceInfo* devInfo);
bool DmIpcModelCodecEncodeNetworkIdQueryFilter(const NetworkIdQueryFilter* queryFilter,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeNetworkIdQueryFilter(DmParcelReadOps* readOps, void* parcel,
    NetworkIdQueryFilter* queryFilter);
bool DmIpcModelCodecEncodeStringVector(DmVec_DmString* vec, DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeStringVector(DmParcelReadOps* readOps, void* parcel, DmVec_DmString* vec);
bool DmIpcModelCodecEncodeLocalServiceInfo(const DMLocalServiceInfo* serviceInfo,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecEncodeLocalServiceInfos(DmVec_DMLocalServiceInfo* serviceInfos,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeLocalServiceInfo(DmParcelReadOps* readOps, void* parcel, DMLocalServiceInfo* serviceInfo);
bool DmIpcModelCodecDecodeLocalServiceInfos(DmParcelReadOps* readOps, void* parcel,
    DmVec_DMLocalServiceInfo* serviceInfos);
bool DmIpcModelCodecEncodeServiceIds(DmVecInt64_t* serviceIds, DmParcelWriteOps* writeOps, void* parcel);
void DmIpcModelCodecDecodeServiceIds(DmVecInt64_t* serviceIds, DmParcelReadOps* readOps, void* parcel);
bool DmIpcModelCodecEncodeDmRegServiceInfo(const DmRegisterServiceInfo* regServiceInfo,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeDmRegServiceInfo(DmParcelReadOps* readOps, void* parcel,
    DmRegisterServiceInfo* regServiceInfo);
bool DmIpcModelCodecEncodeDmSrvDiscParam(const DmDiscoveryServiceParam* param,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeDmSrvDiscParam(DmParcelReadOps* readOps, void* parcel, DmDiscoveryServiceParam* param);
bool DmIpcModelCodecEncodeDmPublishServiceParam(const DmPublishServiceParam* publishServiceParam,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeDmPublishServiceParam(DmParcelReadOps* readOps, void* parcel,
    DmPublishServiceParam* publishServiceParam);
bool DmIpcModelCodecEncodeDmServiceInfo(const DmServiceInfo* serviceInfo, DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeDmServiceInfo(DmParcelReadOps* readOps, void* parcel, DmServiceInfo* serviceInfo);
bool DmIpcModelCodecEncodeDmRegisterServiceState(const DmRegisterServiceState* serviceInfo,
    DmParcelWriteOps* writeOps, void* parcel);
bool DmIpcModelCodecDecodeDmRegisterServiceState(DmParcelReadOps* readOps, void* parcel,
    DmRegisterServiceState* dmRegisterServiceState);
bool DmIpcModelCodecEncodeServiceSyncInfo(const ServiceSyncInfo* serviceSyncInfo,
    DmParcelWriteOps* writeOps, void* parcel);
void DmIpcModelCodecDecodeServiceSyncInfo(DmParcelReadOps* readOps, void* parcel, ServiceSyncInfo* serviceSyncInfo);

#ifdef __cplusplus
}
#endif

#endif
