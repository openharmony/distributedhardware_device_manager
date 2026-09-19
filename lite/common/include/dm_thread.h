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


#ifndef DM_THREAD_H
#define DM_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    pthread_mutex_t mtx;
} DmMutex;

typedef struct {
    pthread_cond_t cond;
    DmMutex* mtx;
} DmCond;

typedef struct {
    pthread_t th;
    bool running;
} DmThread;

typedef struct {
    sem_t sem;
} DmSem;

int DmMutexInit(DmMutex* m);
void DmMutexLock(DmMutex* m);
void DmMutexUnlock(DmMutex* m);
void DmMutexDestroy(DmMutex* m);

int DmCondInit(DmCond* c, DmMutex* m);
void DmCondWait(DmCond* c);
void DmCondSignal(DmCond* c);
void DmCondBroadcast(DmCond* c);
void DmCondDestroy(DmCond* c);

int DmThreadCreate(DmThread* t, void* (*func)(void*), void* arg);
void DmThreadJoin(DmThread* t);
void DmThreadDetach(DmThread* t);

int DmSemInit(DmSem* s, int val);
void DmSemWait(DmSem* s);
void DmSemPost(DmSem* s);
void DmSemDestroy(DmSem* s);

uint64_t DmGetTimestampMs(void);

#ifdef __cplusplus
}
#endif

#endif
