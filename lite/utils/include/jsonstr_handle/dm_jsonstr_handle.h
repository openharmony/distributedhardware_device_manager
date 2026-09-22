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


#ifndef DM_JSONSTR_HANDLE_H
#define DM_JSONSTR_HANDLE_H

#include "dm_container.h"
#include "dm_single_instance.h"
#include "dm_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

typedef struct DmJsonStrHandle {
    DmMutex lock;
} DmJsonStrHandle;

DM_DECLARE_SINGLE_INSTANCE(DmJsonStrHandle);

DM_EXPORT DmVecInt64_t DmJsonstrHandleGetProxyTokenIdByExtra(const DmString* extraInfo);

#ifdef __cplusplus
}
#endif

#endif
