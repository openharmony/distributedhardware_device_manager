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


#include "dm_random.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "dm_log.h"

#define DM_MIN_RANDOM 1
#define DM_MAX_RANDOM_UINT16 32767
#define DM_INVALID_FLAG_ID 0
#define DM_MAX_RANDOM 9

static void (*g_dmSeedFunc)(unsigned int) = srandom;

static void DmSeedRand(void)
{
    time_t t = time(NULL);
    if (t == (time_t)-1) {
        t = 1;
    }
    g_dmSeedFunc((unsigned int)t);
}

static long (*g_dmRandFunc)(void) = random;

static long DmGetRand(void)
{
    return g_dmRandFunc();
}

int32_t DmGenRandInt(int32_t randMin, int32_t randMax)
{
    DmSeedRand();
    return (int32_t)(randMin + DmGetRand() % (randMax - randMin));
}

DM_EXPORT int64_t DmGenRandLongLong(int64_t randMin, int64_t randMax)
{
    DmSeedRand();
    return randMin + (int64_t)DmGetRand() % (randMax - randMin);
}

DM_EXPORT uint64_t DmGenRandUnLongLong(uint64_t randMin, uint64_t randMax)
{
    DmSeedRand();
    return randMin + (uint64_t)DmGetRand() % (randMax - randMin);
}

uint16_t DmGenRandUint(uint16_t randMin, uint16_t randMax)
{
    return (uint16_t)DmGenRandInt(DM_MIN_RANDOM, DM_MAX_RANDOM_UINT16);
}

uint16_t DmGenUniqueRandUint(DmSet_int* randUint16Set)
{
    uint16_t randUint = DM_INVALID_FLAG_ID;
    bool isExist = false;
    do {
        randUint = (uint16_t)DmGenRandInt(DM_MIN_RANDOM, DM_MAX_RANDOM_UINT16);
        if (DmSet_int_Contains(randUint16Set, (int)randUint)) {
            LOGE("The randUint: %{public}d is exist.", randUint);
            isExist = true;
        } else {
            isExist = false;
        }
    } while (isExist);
    DmSet_int_Insert(randUint16Set, (int)randUint);
    return randUint;
}
