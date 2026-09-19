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


#ifndef DM_TIMER_H
#define DM_TIMER_H

#include <stdint.h>
#include "dm_container.h"
#include "dm_thread.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

typedef void (*DmTimerCallback)(const DmString* name, void* ctx);

typedef struct DmTimerEntry {
    DmString name;
    DmTimerCallback callback;
    void* ctx;
    int32_t timeOutMs;
    DmThread thread;
    volatile bool active;
} DmTimerEntry;

typedef DmTimerEntry* DmTimerEntry_ptr;
DM_VEC_DEFINE(DmTimerEntry_ptr);

typedef struct DmTimer {
    DmMutex timerMutex;
    DmVec_DmTimerEntry_ptr timerVec;
    bool initialized;
} DmTimer;

DM_EXPORT void DmTimerCreate(DmTimer* t);
DM_EXPORT void DmTimerDestroy(DmTimer* t);
DM_EXPORT int32_t DmTimerStart(DmTimer* t, const DmString* name, int32_t timeOut,
    DmTimerCallback callback, void* ctx);
DM_EXPORT int32_t DmTimerDelete(DmTimer* t, const DmString* timerName);
DM_EXPORT int32_t DmTimerDeleteAll(DmTimer* t);

#endif
