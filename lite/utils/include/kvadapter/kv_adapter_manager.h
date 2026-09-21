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


#ifndef DM_KV_ADAPTER_MANAGER_H
#define DM_KV_ADAPTER_MANAGER_H

#include "dm_crypto.h"

#include <stdint.h>
#include "dm_container.h"
#include "dm_single_instance.h"
#include "dm_thread.h"
#include "kv_adapter.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

typedef struct KVAdapterManager {
    DmMutex kvAdapterMtx;
    KVAdapter* kvAdapter;
    DmMutex idCacheMapMtx;
    DmHmap_DmString_DmString idCacheMap;
} KVAdapterManager;

DM_DECLARE_SINGLE_INSTANCE(KVAdapterManager);

DM_EXPORT int32_t DmKvAdapterManagerInit(void);
DM_EXPORT void DmKvAdapterManagerUninit(void);
DM_EXPORT void DmKvAdapterManagerReinit(void);
int32_t DmKvAdapterManagerPutByAnoyDeviceId(const char* key, const DmKVValue* value);
DM_EXPORT int32_t DmKvAdapterManagerGet(const char* key, DmKVValue* value);
DM_EXPORT int32_t DmKvAdapterManagerDeleteAgedEntry(void);
DM_EXPORT int32_t DmKvAdapterManagerAppUninstall(const char* appId);
DM_EXPORT int32_t DmKvAdapterManagerGetFreezeData(const char* key, DmString* value);
DM_EXPORT int32_t DmKvAdapterManagerPutFreezeData(const char* key, const DmString* value);
DM_EXPORT int32_t DmKvAdapterManagerDeleteFreezeData(const char* key);
DM_EXPORT int32_t DmKvAdapterManagerGetAllOstypeData(DmVec_DmString* values);
DM_EXPORT int32_t DmKvAdapterManagerPutOstypeData(const char* key, const char* value);
DM_EXPORT int32_t DmKvAdapterManagerDeleteOstypeData(const char* key);
DM_EXPORT int32_t DmKvAdapterManagerGetLocalUserIdData(const char* key, DmString* value);
DM_EXPORT int32_t DmKvAdapterManagerPutLocalUserIdData(const char* key, const DmString* value);
DM_EXPORT int32_t DmKvAdapterManagerGetOsTypeCount(int32_t* count);

DM_EXPORT void DmConvertKvValueToJson(const DmKVValue* kvValue, DmString* result);
DM_EXPORT void DmConvertJsonToKvValue(const DmString* result, DmKVValue* kvValue);

#endif
