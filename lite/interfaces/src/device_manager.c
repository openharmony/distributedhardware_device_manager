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


#include "device_manager.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_container.h"
#include "dm_thread.h"
#include "dm_constants.h"
#include "dm_impl.h"

static DeviceManagerVtable g_vtable = {
    .InitDeviceManager = DmImplInitDeviceManager,
    .UnInitDeviceManager = DmImplUnInitDeviceManager,
    .GetTrustedDeviceList = DmImplGetTrustedDeviceList,
    .GetLocalDeviceInfo = DmImplGetLocalDeviceInfo,
    .GetDeviceInfoByNetworkId = DmImplGetDeviceInfoByNetworkId,
    .RegisterDevStateCallback = DmImplRegisterDevStateCallback,
    .UnRegisterDevStateCallback = DmImplUnRegisterDevStateCallback,
    .StartDeviceDiscovery = DmImplStartDeviceDiscovery,
    .StopDeviceDiscovery = DmImplStopDeviceDiscovery,
    .PublishDeviceDiscovery = DmImplPublishDeviceDiscovery,
    .UnPublishDeviceDiscovery = DmImplUnPublishDeviceDiscovery,
    .SetLocalDeviceName = DmImplSetLocalDeviceName,
};

static DeviceManager g_instance = {
    .vtable = &g_vtable,
};

DM_EXPORT DeviceManager* DmDeviceManagerGetInstance(void)
{
    return &g_instance;
}
