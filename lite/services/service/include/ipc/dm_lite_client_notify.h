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


#ifndef DM_LITE_CLIENT_NOTIFY_H
#define DM_LITE_CLIENT_NOTIFY_H

#include <stdint.h>
#include <sys/types.h>
#include "serializer.h"
#include "dm_container.h"
#include "dm_thread.h"
#include "dm_device_info_c.h"
#include "device_manager_ipc_interface_code.h"

#define DM_MAX_REGISTERED_CLIENTS 8

typedef struct {
    SvcIdentity clients[DM_MAX_REGISTERED_CLIENTS];
    DmString pkgNames[DM_MAX_REGISTERED_CLIENTS];
    uint32_t deathCbIds[DM_MAX_REGISTERED_CLIENTS];
    void* deathCtxs[DM_MAX_REGISTERED_CLIENTS];
    pid_t pids[DM_MAX_REGISTERED_CLIENTS];
    DmMutex lock;
    int count;
} DmLiteClientRegistry;

void DmLiteClientNotifyInit(void);
int32_t DmLiteClientNotifyRegister(const DmString* pkgName, const SvcIdentity* svc);
int32_t DmLiteClientNotifyUnregister(const DmString* pkgName);
void DmLiteClientNotifyDeviceState(DmDeviceState state, const DmDeviceInfo* info, bool isOnline);
void DmLiteClientNotifyDeviceFound(uint16_t subscribeId, const DmDeviceInfo* info);
void DmLiteClientNotifyPublishResult(int32_t publishId, int32_t result);
void DmLiteClientNotifyDiscoverResult(const char* pkgName, uint16_t subscribeId, int32_t result);
void DmLiteClientNotifyOnlineDevicesToNewClient(int32_t handle);

#endif
