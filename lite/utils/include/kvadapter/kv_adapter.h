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


#ifndef DM_KV_ADAPTER_H
#define DM_KV_ADAPTER_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"
#include "dm_thread.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

typedef struct KVAdapter KVAdapter;

#ifdef __cplusplus
extern "C" {
#endif

DM_EXPORT KVAdapter* DmKvAdapterCreate(void);
DM_EXPORT void DmKvAdapterDestroy(KVAdapter* adapter);
DM_EXPORT int32_t DmKvAdapterInit(KVAdapter* adapter);
DM_EXPORT void DmKvAdapterUninit(KVAdapter* adapter);
DM_EXPORT int32_t DmKvAdapterReinit(KVAdapter* adapter);
DM_EXPORT int32_t DmKvAdapterPut(KVAdapter* adapter, const char* key, const char* value);
DM_EXPORT int32_t DmKvAdapterGet(KVAdapter* adapter, const char* key, DmString* value);
DM_EXPORT int32_t DmKvAdapterDeleteKvStore(KVAdapter* adapter);
DM_EXPORT int32_t DmKvAdapterDeleteByAppId(KVAdapter* adapter, const char* appId, const char* prefix);
DM_EXPORT int32_t DmKvAdapterDeleteBatch(KVAdapter* adapter, const DmVec_DmString* keys);
DM_EXPORT int32_t DmKvAdapterDelete(KVAdapter* adapter, const char* key);
DM_EXPORT int32_t DmKvAdapterGetAllOstypeData(KVAdapter* adapter, const char* key, DmVec_DmString* values);
DM_EXPORT int32_t DmKvAdapterGetOstypeCountByPrefix(KVAdapter* adapter, const char* prefix, int32_t* count);

#ifdef __cplusplus
}
#endif

#endif
