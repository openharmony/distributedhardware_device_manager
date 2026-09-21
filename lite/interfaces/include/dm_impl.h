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


#ifndef DM_IMPL_H
#define DM_IMPL_H

#include "device_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t DmImplInitDeviceManager(const DmString* pkgName, DmInitCallback* dmInitCallback);
int32_t DmImplUnInitDeviceManager(const DmString* pkgName);
int32_t DmImplGetTrustedDeviceList(const DmString* pkgName, const DmString* extra, DmVec_DmDeviceInfo* deviceList);
int32_t DmImplGetLocalDeviceInfo(const DmString* pkgName, DmDeviceInfo* info);
int32_t DmImplGetDeviceInfoByNetworkId(const DmString* pkgName, const DmString* networkId, DmDeviceInfo* info);
int32_t DmImplRegisterDevStateCallback(const DmString* pkgName, const DmString* extra, DeviceStateCallback* callback);
int32_t DmImplUnRegisterDevStateCallback(const DmString* pkgName);
int32_t DmImplStartDeviceDiscovery(const DmString* pkgName, const DmSubscribeInfo* subscribeInfo,
    const DmString* extra, DiscoveryCallback* callback);
int32_t DmImplStopDeviceDiscovery(const DmString* pkgName, uint16_t subscribeId);
int32_t DmImplPublishDeviceDiscovery(const DmString* pkgName, const DmPublishInfo* publishInfo,
    PublishCallback* callback);
int32_t DmImplUnPublishDeviceDiscovery(const DmString* pkgName, int32_t publishId);
int32_t DmImplSetLocalDeviceName(const DmString* pkgName, const DmString* deviceName);

#ifdef __cplusplus
}
#endif

#endif
