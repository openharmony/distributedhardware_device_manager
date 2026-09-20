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


#ifndef DM_DEVICE_MANAGER_SDK_H
#define DM_DEVICE_MANAGER_SDK_H

#include <stdint.h>
#include "dm_container.h"
#include "dm_device_info_c.h"
#include "dm_thread.h"
#include "device_manager_callback.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeviceManager DeviceManager;

typedef struct {
    int32_t (*InitDeviceManager)(const DmString* pkgName, DmInitCallback* dmInitCallback);
    int32_t (*UnInitDeviceManager)(const DmString* pkgName);
    int32_t (*GetTrustedDeviceList)(const DmString* pkgName, const DmString* extra, DmVec_DmDeviceInfo* deviceList);
    int32_t (*GetLocalDeviceInfo)(const DmString* pkgName, DmDeviceInfo* info);
    int32_t (*GetDeviceInfoByNetworkId)(const DmString* pkgName, const DmString* networkId, DmDeviceInfo* info);
    int32_t (*RegisterDevStateCallback)(const DmString* pkgName, const DmString* extra, DeviceStateCallback* callback);
    int32_t (*UnRegisterDevStateCallback)(const DmString* pkgName);
    int32_t (*StartDeviceDiscovery)(const DmString* pkgName, const DmSubscribeInfo* subscribeInfo,
        const DmString* extra, DiscoveryCallback* callback);
    int32_t (*StopDeviceDiscovery)(const DmString* pkgName, uint16_t subscribeId);
    int32_t (*PublishDeviceDiscovery)(const DmString* pkgName, const DmPublishInfo* publishInfo,
        PublishCallback* callback);
    int32_t (*UnPublishDeviceDiscovery)(const DmString* pkgName, int32_t publishId);
    int32_t (*SetLocalDeviceName)(const DmString* pkgName, const DmString* deviceName);
} DeviceManagerVtable;

struct DeviceManager {
    const DeviceManagerVtable* vtable;
};

DM_EXPORT DeviceManager* DmDeviceManagerGetInstance(void);

#ifdef __cplusplus
}
#endif

#endif
