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


#ifndef DM_CALLBACK_H
#define DM_CALLBACK_H

#include "dm_device_info_c.h"
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*OnRemoteDied)(void* ctx);
} DmInitCallbackVtable;

typedef struct {
    DmInitCallbackVtable* vtable;
    void* ctx;
} DmInitCallback;

typedef struct {
    void (*OnDeviceOnline)(void* ctx, const DmDeviceInfo* deviceInfo);
    void (*OnDeviceOffline)(void* ctx, const DmDeviceInfo* deviceInfo);
    void (*OnDeviceChanged)(void* ctx, const DmDeviceInfo* deviceInfo);
    void (*OnDeviceReady)(void* ctx, const DmDeviceInfo* deviceInfo);
} DeviceStateCallbackVtable;

typedef struct {
    DeviceStateCallbackVtable* vtable;
    void* ctx;
} DeviceStateCallback;

typedef struct {
    void (*OnDeviceOnline)(void* ctx, const DmDeviceBasicInfo* deviceBasicInfo);
    void (*OnDeviceOffline)(void* ctx, const DmDeviceBasicInfo* deviceBasicInfo);
    void (*OnDeviceChanged)(void* ctx, const DmDeviceBasicInfo* deviceBasicInfo);
    void (*OnDeviceReady)(void* ctx, const DmDeviceBasicInfo* deviceBasicInfo);
} DeviceStatusCallbackVtable;

typedef struct {
    DeviceStatusCallbackVtable* vtable;
    void* ctx;
} DeviceStatusCallback;

typedef struct {
    void (*OnDiscoverySuccess)(void* ctx, uint16_t subscribeId);
    void (*OnDiscoveryFailed)(void* ctx, uint16_t subscribeId, int32_t failedReason);
    void (*OnDeviceFound)(void* ctx, uint16_t subscribeId, const DmDeviceInfo* deviceInfo);
    void (*OnDeviceFoundBasicInfo)(void* ctx, uint16_t subscribeId, const DmDeviceBasicInfo* deviceBasicInfo);
} DiscoveryCallbackVtable;

typedef struct {
    DiscoveryCallbackVtable* vtable;
    void* ctx;
} DiscoveryCallback;

typedef struct {
    void (*OnPublishResult)(void* ctx, int32_t publishId, int32_t publishResult);
} PublishCallbackVtable;

typedef struct {
    PublishCallbackVtable* vtable;
    void* ctx;
} PublishCallback;

typedef struct {
    void (*OnCall)(void* ctx, const DmString* paramJson);
} DeviceManagerUiCallbackVtable;

typedef struct {
    DeviceManagerUiCallbackVtable* vtable;
    void* ctx;
} DeviceManagerUiCallback;

typedef struct {
    void (*OnResult)(void* ctx, const DmVecVoid* deviceProfileInfos, int32_t code);
} GetDeviceProfileInfoListCallbackVtable;

typedef struct {
    GetDeviceProfileInfoListCallbackVtable* vtable;
    void* ctx;
} GetDeviceProfileInfoListCallback;

typedef struct {
    void (*OnResult)(void* ctx, const void* deviceIconInfo, int32_t code);
} GetDeviceIconInfoCallbackVtable;

typedef struct {
    GetDeviceIconInfoCallbackVtable* vtable;
    void* ctx;
} GetDeviceIconInfoCallback;

typedef struct {
    void (*OnResult)(void* ctx, int32_t code);
} SetLocalDeviceNameCallbackVtable;

typedef struct {
    SetLocalDeviceNameCallbackVtable* vtable;
    void* ctx;
} SetLocalDeviceNameCallback;

typedef struct {
    void (*OnResult)(void* ctx, int32_t code);
} SetRemoteDeviceNameCallbackVtable;

typedef struct {
    SetRemoteDeviceNameCallbackVtable* vtable;
    void* ctx;
} SetRemoteDeviceNameCallback;

typedef struct {
    void (*OnCredentialResult)(void* ctx, int32_t* action, const DmString* credentialResult);
} CredentialCallbackVtable;

typedef struct {
    CredentialCallbackVtable* vtable;
    void* ctx;
} CredentialCallback;

typedef struct {
    void (*OnDeviceTrustChange)(void* ctx, const DmString* udid, const DmString* uuid, DmAuthForm authForm);
} DevTrustChangeCallbackVtable;

typedef struct {
    DevTrustChangeCallbackVtable* vtable;
    void* ctx;
} DevTrustChangeCallback;

typedef struct {
    void (*OnDeviceScreenStatus)(void* ctx, const DmDeviceInfo* deviceInfo);
} DeviceScreenStatusCallbackVtable;

typedef struct {
    DeviceScreenStatusCallbackVtable* vtable;
    void* ctx;
} DeviceScreenStatusCallback;

typedef struct {
    void (*OnCredentialAuthStatus)(void* ctx, const DmString* deviceList, uint16_t deviceTypeId, int32_t errcode);
} CredentialAuthStatusCallbackVtable;

typedef struct {
    CredentialAuthStatusCallbackVtable* vtable;
    void* ctx;
} CredentialAuthStatusCallback;

typedef struct {
    void (*OnServiceFound)(void* ctx, const DmServiceInfo* service);
    void (*OnServiceDiscoveryResult)(void* ctx, int32_t resReason);
} ServiceDiscoveryCallbackVtable;

typedef struct {
    ServiceDiscoveryCallbackVtable* vtable;
    void* ctx;
} ServiceDiscoveryCallback;

typedef struct {
    void (*OnServiceOnline)(void* ctx, const DmServiceInfo* serviceInfo);
    void (*OnServiceOffline)(void* ctx, const DmServiceInfo* serviceInfo);
    void (*OnServiceInfoChange)(void* ctx, const DmServiceInfo* serviceInfo);
} ServiceInfoStateCallbackVtable;

typedef struct {
    ServiceInfoStateCallbackVtable* vtable;
    void* ctx;
} ServiceInfoStateCallback;

typedef struct {
    void (*OnServicePublishResult)(void* ctx, int64_t serviceId, int32_t reason);
} ServicePublishCallbackVtable;

typedef struct {
    ServicePublishCallbackVtable* vtable;
    void* ctx;
} ServicePublishCallback;

typedef struct {
    void (*OnLeaveLNNCallback)(void* ctx, const DmString* networkId, int32_t retCode);
} LeaveLNNCallbackVtable;

typedef struct {
    LeaveLNNCallbackVtable* vtable;
    void* ctx;
} LeaveLNNCallback;

typedef struct {
    void (*OnSyncServiceInfoResult)(void* ctx, int32_t result, const DmString* content);
} SyncServiceInfoCallbackVtable;

typedef struct {
    SyncServiceInfoCallbackVtable* vtable;
    void* ctx;
} SyncServiceInfoCallback;

#ifdef __cplusplus
}
#endif

#endif
