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


#include "dm_thread.h"
#include <time.h>

#define DM_MS_PER_SEC 1000
#define DM_NS_PER_MS 1000000

int DmMutexInit(DmMutex* m)
{
    return pthread_mutex_init(&m->mtx, NULL);
}

void DmMutexLock(DmMutex* m)
{
    pthread_mutex_lock(&m->mtx);
}

void DmMutexUnlock(DmMutex* m)
{
    pthread_mutex_unlock(&m->mtx);
}

void DmMutexDestroy(DmMutex* m)
{
    pthread_mutex_destroy(&m->mtx);
}

int DmCondInit(DmCond* c, DmMutex* m)
{
    c->mtx = m;
    return pthread_cond_init(&c->cond, NULL);
}

void DmCondWait(DmCond* c)
{
    pthread_cond_wait(&c->cond, &c->mtx->mtx);
}

void DmCondSignal(DmCond* c)
{
    pthread_cond_signal(&c->cond);
}

void DmCondBroadcast(DmCond* c)
{
    pthread_cond_broadcast(&c->cond);
}

void DmCondDestroy(DmCond* c)
{
    pthread_cond_destroy(&c->cond);
}

int DmThreadCreate(DmThread* t, void* (*func)(void*), void* arg)
{
    t->running = true;
    int ret = pthread_create(&t->th, NULL, func, arg);
    if (ret != 0) {
        t->running = false;
    }
    return ret;
}

void DmThreadJoin(DmThread* t)
{
    pthread_join(t->th, NULL);
    t->running = false;
}

void DmThreadDetach(DmThread* t)
{
    pthread_detach(t->th);
    t->running = false;
}

int DmSemInit(DmSem* s, int val)
{
    return sem_init(&s->sem, 0, val);
}

void DmSemWait(DmSem* s)
{
    sem_wait(&s->sem);
}

void DmSemPost(DmSem* s)
{
    sem_post(&s->sem);
}

void DmSemDestroy(DmSem* s)
{
    sem_destroy(&s->sem);
}

uint64_t DmGetTimestampMs(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * DM_MS_PER_SEC + (uint64_t)ts.tv_nsec / DM_NS_PER_MS;
}
