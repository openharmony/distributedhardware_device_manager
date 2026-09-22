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


#include "device_manager_notify.h"
#include "dm_log.h"
#include "device_manager_callback.h"
#include <string.h>

static DeviceManagerNotify g_notifyInstance;

static DeviceStateCallback g_devStateCallback;
static bool g_devStateCallbackRegistered = false;

static DiscoveryCallback g_discCallback;
static bool g_discCallbackRegistered = false;

static PublishCallback g_publishCallback;
static bool g_publishCallbackRegistered = false;

DeviceManagerNotify* DmDeviceManagerNotifyGetInstance(void)
{
    return &g_notifyInstance;
}

void DmNotifyRegisterPublishCallback(DeviceManagerNotify* self, const char* pkgName,
    int32_t publishId, void* callback)
{
    (void)self;
    (void)pkgName;
    (void)publishId;
    PublishCallback* cb = (PublishCallback*)callback;
    if (cb != NULL && cb->vtable != NULL) {
        g_publishCallback = *cb;
        g_publishCallbackRegistered = true;
        LOGI("PublishCallback registered");
    } else {
        LOGE("invalid PublishCallback");
    }
}

void DmNotifyRegisterDiscoveryCallback(DeviceManagerNotify* self, const char* pkgName,
    uint16_t subscribeId, void* callback)
{
    (void)self;
    (void)pkgName;
    (void)subscribeId;
    DiscoveryCallback* cb = (DiscoveryCallback*)callback;
    if (cb != NULL && cb->vtable != NULL) {
        g_discCallback = *cb;
        g_discCallbackRegistered = true;
        LOGI("DiscoveryCallback registered");
    } else {
        LOGE("invalid DiscoveryCallback");
    }
}

void DmNotifyRegisterDeviceStateCallback(DeviceManagerNotify* self, const char* pkgName,
    void* callback)
{
    (void)self;
    (void)pkgName;
    DeviceStateCallback* cb = (DeviceStateCallback*)callback;
    if (cb != NULL && cb->vtable != NULL) {
        g_devStateCallback = *cb;
        g_devStateCallbackRegistered = true;
        LOGI("DeviceStateCallback registered");
    } else {
        LOGE("invalid DeviceStateCallback");
    }
}

void DmNotifyCallDevStateChange(DeviceManagerNotify* self, DmDeviceState state,
    const DmDeviceInfo* info, bool isOnline)
{
    (void)self;
    (void)isOnline;
    if (!g_devStateCallbackRegistered) {
        LOGD("no DeviceStateCallback registered");
        return;
    }
    if (g_devStateCallback.vtable == NULL) {
        LOGE("DeviceStateCallback vtable is NULL");
        return;
    }
    if (state == DEVICE_STATE_ONLINE && g_devStateCallback.vtable->OnDeviceOnline) {
        LOGI("calling OnDeviceOnline networkId=%s", info ? info->networkId : "(null)");
        g_devStateCallback.vtable->OnDeviceOnline(g_devStateCallback.ctx, info);
    } else if (state == DEVICE_STATE_OFFLINE && g_devStateCallback.vtable->OnDeviceOffline) {
        LOGI("calling OnDeviceOffline networkId=%s", info ? info->networkId : "(null)");
        g_devStateCallback.vtable->OnDeviceOffline(g_devStateCallback.ctx, info);
    } else if (state == DEVICE_INFO_CHANGED && g_devStateCallback.vtable->OnDeviceChanged) {
        LOGI("calling OnDeviceChanged networkId=%s", info ? info->networkId : "(null)");
        g_devStateCallback.vtable->OnDeviceChanged(g_devStateCallback.ctx, info);
    } else if (state == DEVICE_INFO_READY && g_devStateCallback.vtable->OnDeviceReady) {
        LOGI("calling OnDeviceReady networkId=%s", info ? info->networkId : "(null)");
        g_devStateCallback.vtable->OnDeviceReady(g_devStateCallback.ctx, info);
    } else {
        LOGW("no handler for state=%d", state);
    }
}

void DmNotifyOnDeviceFound(DeviceManagerNotify* self, const char* pkgName,
    uint16_t subscribeId, const DmDeviceInfo* info)
{
    (void)self;
    (void)pkgName;
    if (!g_discCallbackRegistered) {
        LOGD("no DiscoveryCallback registered");
        return;
    }
    if (g_discCallback.vtable == NULL || g_discCallback.vtable->OnDeviceFound == NULL) {
        LOGE("DiscoveryCallback vtable or OnDeviceFound is NULL");
        return;
    }
    LOGI("calling OnDeviceFound subscribeId=%u", subscribeId);
    g_discCallback.vtable->OnDeviceFound(g_discCallback.ctx, subscribeId, info);
}

void DmNotifyOnPublishResult(DeviceManagerNotify* self, const char* pkgName,
    int32_t publishId, int32_t result)
{
    (void)self;
    (void)pkgName;
    if (!g_publishCallbackRegistered) {
        LOGD("no PublishCallback registered");
        return;
    }
    if (g_publishCallback.vtable == NULL || g_publishCallback.vtable->OnPublishResult == NULL) {
        LOGE("PublishCallback vtable or OnPublishResult is NULL");
        return;
    }
    LOGI("calling OnPublishResult publishId=%d result=%d", publishId, result);
    g_publishCallback.vtable->OnPublishResult(g_publishCallback.ctx, publishId, result);
}

void DmNotifyOnDiscoveryResult(DeviceManagerNotify* self, const char* pkgName,
    uint16_t subscribeId, int32_t result)
{
    (void)self;
    (void)pkgName;
    if (!g_discCallbackRegistered) {
        LOGD("no DiscoveryCallback registered");
        return;
    }
    if (g_discCallback.vtable == NULL) {
        LOGE("DiscoveryCallback vtable is NULL");
        return;
    }
    if (result == DM_OK) {
        if (g_discCallback.vtable->OnDiscoverySuccess != NULL) {
            LOGI("calling OnDiscoverySuccess subscribeId=%u", subscribeId);
            g_discCallback.vtable->OnDiscoverySuccess(g_discCallback.ctx, subscribeId);
        }
    } else {
        if (g_discCallback.vtable->OnDiscoveryFailed != NULL) {
            LOGE("calling OnDiscoveryFailed subscribeId=%u reason=%d", subscribeId, result);
            g_discCallback.vtable->OnDiscoveryFailed(g_discCallback.ctx, subscribeId, result);
        }
    }
}
