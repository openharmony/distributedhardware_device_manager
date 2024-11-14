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
#ifndef OHOS_DEVICE_MANAGER_FFI_H
#define OHOS_DEVICE_MANAGER_FFI_H

#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

extern "C" {
typedef struct {
    char *deviceId;
    char *deviceName;
    uint16_t deviceType;
    char *networkId;
} FfiDeviceBasicInfo;

typedef struct {
    FfiDeviceBasicInfo *head;
    int64_t size;
} FfiDeviceBasicInfoArray;

FFI_EXPORT int64_t FfiOHOSDistributedDeviceManagerCreateDeviceManager(const char *name, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerReleaseDeviceManager(int64_t id, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerGetAvailableDeviceList(
    int64_t id, FfiDeviceBasicInfoArray *deviceInfoList, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerFreeDeviceInfoList(FfiDeviceBasicInfoArray deviceInfoList);

FFI_EXPORT const char *FfiOHOSDistributedDeviceManagerGetLocalDeviceNetworkId(int64_t id, int32_t *errCode);

FFI_EXPORT const char *FfiOHOSDistributedDeviceManagerGetLocalDeviceName(int64_t id, int32_t *errCode);

FFI_EXPORT int32_t FfiOHOSDistributedDeviceManagerGetLocalDeviceType(int64_t id, int32_t *errCode);

FFI_EXPORT const char *FfiOHOSDistributedDeviceManagerGetLocalDeviceId(int64_t id, int32_t *errCode);

FFI_EXPORT const char *FfiOHOSDistributedDeviceManagerGetDeviceName(
    int64_t id, const char *networkId, int32_t *errCode);

FFI_EXPORT int32_t FfiOHOSDistributedDeviceManagerGetDeviceType(int64_t id, const char *networkId, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerStartDiscovering(int64_t id, const char *extra, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerStopDiscovering(int64_t id, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerBindTarget(int64_t id, const char *deviceId, const char *bindParam,
    bool isMetaType, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerUnbindTarget(int64_t id, const char *deviceId, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerOn(int64_t id, const char *type, void *callback, int32_t *errCode);

FFI_EXPORT void FfiOHOSDistributedDeviceManagerOff(int64_t id, const char *type, int32_t *errCode);
}

#endif // OHOS_DEVICE_MANAGER_FFI_H
