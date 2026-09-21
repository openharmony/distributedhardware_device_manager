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


#include "ipc_client_stub.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_container.h"
#include "dm_device_info_c.h"
#include "device_manager_ipc_interface_code.h"
#include "serializer.h"
#include "ipc_skeleton.h"
#include "device_manager_notify.h"
#include <stdlib.h>
#include "securec.h"

DM_EXPORT IpcClientStub* DmIpcClientStubCreate(void)
{
    IpcClientStub* stub = (IpcClientStub*)calloc(1, sizeof(IpcClientStub));
    return stub;
}

DM_EXPORT void DmIpcClientStubDestroy(IpcClientStub* self)
{
    if (self != NULL) {
        free(self);
    }
}

DM_EXPORT void DmIpcClientStubInit(IpcClientStub* self)
{
    (void)self;
}

DM_EXPORT void DmIpcClientStubDestroyInner(IpcClientStub* self)
{
    (void)self;
}

static int32_t DmIpcClientStubHandleDeviceState(IpcIo* data)
{
    int32_t state = 0;
    int32_t isOnline = 0;
    ReadInt32(data, &state);
    ReadInt32(data, &isOnline);
    size_t readLen = 0;
    const char* networkId = (const char*)ReadString(data, &readLen);
    const char* deviceName = (const char*)ReadString(data, &readLen);
    int32_t deviceTypeId = 0;
    ReadInt32(data, &deviceTypeId);
    const char* deviceId = (const char*)ReadString(data, &readLen);
    DmDeviceInfo deviceInfo;
    (void)memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (networkId != NULL) {
        (void)strncpy_s(deviceInfo.networkId, sizeof(deviceInfo.networkId), networkId,
            sizeof(deviceInfo.networkId) - 1);
    }
    if (deviceName != NULL) {
        (void)strncpy_s(deviceInfo.deviceName, sizeof(deviceInfo.deviceName), deviceName,
            sizeof(deviceInfo.deviceName) - 1);
    }
    if (deviceId != NULL) {
        (void)strncpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), deviceId,
            sizeof(deviceInfo.deviceId) - 1);
    }
    deviceInfo.deviceTypeId = (uint16_t)deviceTypeId;
    LOGI("recv device_state state=%d isOnline=%d networkId=%s name=%s", state, isOnline, networkId, deviceName);
    DeviceManagerNotify* notify = DmDeviceManagerNotifyGetInstance();
    DmNotifyCallDevStateChange(notify, (DmDeviceState)state, &deviceInfo, isOnline ? true : false);
    return DM_OK;
}

static int32_t DmHandleDeviceFoundCase(IpcIo* ipcData)
{
    int32_t subscribeId = 0;
    ReadInt32(ipcData, &subscribeId);
    size_t readLen = 0;
    const char* deviceId = (const char*)ReadString(ipcData, &readLen);
    const char* networkId = (const char*)ReadString(ipcData, &readLen);
    const char* deviceName = (const char*)ReadString(ipcData, &readLen);
    int32_t deviceTypeId = 0;
    ReadInt32(ipcData, &deviceTypeId);
    const char* extraData = (const char*)ReadString(ipcData, &readLen);
    DmDeviceInfo deviceInfo;
    (void)memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (deviceId != NULL) {
        (void)strncpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), deviceId,
            sizeof(deviceInfo.deviceId) - 1);
    }
    if (networkId != NULL) {
        (void)strncpy_s(deviceInfo.networkId, sizeof(deviceInfo.networkId), networkId,
            sizeof(deviceInfo.networkId) - 1);
    }
    if (deviceName != NULL) {
        (void)strncpy_s(deviceInfo.deviceName, sizeof(deviceInfo.deviceName), deviceName,
            sizeof(deviceInfo.deviceName) - 1);
    }
    deviceInfo.deviceTypeId = (uint16_t)deviceTypeId;
    if (extraData) {
        DmStringAssignCstr(&deviceInfo.extraData, extraData);
    }
    LOGI("recv device_found subId=%d networkId=%s extraData=%s", subscribeId, networkId,
        extraData ? extraData : "");
    DeviceManagerNotify* notify = DmDeviceManagerNotifyGetInstance();
    DmNotifyOnDeviceFound(notify, NULL, (uint16_t)subscribeId, &deviceInfo);
    DmDeviceInfoDestroy(&deviceInfo);
    return DM_OK;
}

static int32_t DmHandlePublishFinishCase(IpcIo* ipcData)
{
    int32_t publishId = 0;
    int32_t result = 0;
    ReadInt32(ipcData, &publishId);
    ReadInt32(ipcData, &result);
    LOGI("recv publish_finish publishId=%d result=%d", publishId, result);
    DeviceManagerNotify* notify = DmDeviceManagerNotifyGetInstance();
    DmNotifyOnPublishResult(notify, NULL, publishId, result);
    return DM_OK;
}

DM_EXPORT int32_t DmIpcClientStubOnRemoteRequest(IpcClientStub* self, uint32_t code,
    void* data, void* reply, void* option)
{
    (void)self;
    (void)reply;
    (void)option;
    IpcIo* ipcData = (IpcIo*)data;
    if (ipcData == NULL) {
        return DM_OK;
    }
    LOGI("on_remote_request code=%u", code);
    switch (code) {
        case DM_SERVER_DEVICE_STATE_NOTIFY:
            return DmIpcClientStubHandleDeviceState(ipcData);
        case DM_SERVER_DEVICE_FOUND:
            return DmHandleDeviceFoundCase(ipcData);
        case DM_SERVER_PUBLISH_FINISH:
            return DmHandlePublishFinishCase(ipcData);
        case DM_SERVER_DISCOVER_FINISH: {
            size_t readLen = 0;
            const char* pkgName = (const char*)ReadString(ipcData, &readLen);
            int32_t subscribeId = 0;
            ReadInt32(ipcData, &subscribeId);
            int32_t result = 0;
            ReadInt32(ipcData, &result);
            LOGI("recv discover_finish subscribeId=%d result=%d", subscribeId, result);
            DeviceManagerNotify* notify = DmDeviceManagerNotifyGetInstance();
            DmNotifyOnDiscoveryResult(notify, pkgName, (uint16_t)subscribeId, result);
            break;
        }
        default:
            LOGW("unknown code=%u", code);
            break;
    }
    return DM_OK;
}


DM_EXPORT int32_t DmIpcClientStubSendCmd(IpcClientStub* self, int32_t cmdCode, void* req, void* rsp)
{
    (void)self;
    (void)cmdCode;
    (void)req;
    (void)rsp;
    return DM_OK;
}
