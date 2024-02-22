/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DEVICE_MANAGER_SOFTBUS_ADAPTE_H
#define OHOS_DEVICE_MANAGER_SOFTBUS_ADAPTE_H

#include "device_manager_impl_lite_m.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitSoftbusModle(void);
int UnInitSoftbusModle(void);
int StartSoftbusPublish(const char *pkgName, OnAdvertisingResult cb);
int StopSoftbusPublish(const char *pkgName);
int StartSoftbusDiscovery(const char *pkgName, const int subscribeId, const char *filterOption,
    OnTargetFound callback);
int StopSoftbusDiscovery(const char *pkgName, const int subscribeId);
int RegisterSoftbusDevStateCallback(const char *pkgName, DevStatusCallback callback);
int UnRegisterSoftbusDevStateCallback(const char *pkgName);
int GetSoftbusTrustedDeviceList(const char *pkgName, DmDeviceBasicInfo *deviceList, const int deviceListLen,
    int *trustListLen);
int SoftbusBindTarget(const char *pkgName, const char *deviceId, const int bindType, OnBindResult callback);
int SoftbusUnBindTarget(const char *pkgName, const char *networkId);

#ifdef __cplusplus
}
#endif

#endif // OHOS_DEVICE_MANAGER_SOFTBUS_ADAPTE_H
