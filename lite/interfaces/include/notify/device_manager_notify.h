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


#ifndef DEVICE_MANAGER_NOTIFY_H
#define DEVICE_MANAGER_NOTIFY_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_device_info_c.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeviceManagerNotify {
    void* placeholder;
} DeviceManagerNotify;

DeviceManagerNotify* DmDeviceManagerNotifyGetInstance(void);

void DmNotifyRegisterPublishCallback(DeviceManagerNotify* self, const char* pkgName,
    int32_t publishId, void* callback);
void DmNotifyRegisterDiscoveryCallback(DeviceManagerNotify* self, const char* pkgName,
    uint16_t subscribeId, void* callback);
void DmNotifyRegisterDeviceStateCallback(DeviceManagerNotify* self, const char* pkgName,
    void* callback);

void DmNotifyCallDevStateChange(DeviceManagerNotify* self, DmDeviceState state,
    const DmDeviceInfo* info, bool isOnline);
void DmNotifyOnDeviceFound(DeviceManagerNotify* self, const char* pkgName,
    uint16_t subscribeId, const DmDeviceInfo* info);
void DmNotifyOnPublishResult(DeviceManagerNotify* self, const char* pkgName,
    int32_t publishId, int32_t result);
void DmNotifyOnDiscoveryResult(DeviceManagerNotify* self, const char* pkgName,
    uint16_t subscribeId, int32_t result);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_MANAGER_NOTIFY_H
