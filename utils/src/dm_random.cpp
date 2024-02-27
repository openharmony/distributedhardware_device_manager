/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <random>

#if defined(__LITEOS_M__)
#include <time.h>
#endif

namespace OHOS {
namespace DistributedHardware {
int32_t GenRandInt(int32_t randMin, int32_t randMax)
{
#if defined(__LITEOS_M__)
    srandom(time(NULL));
    return (randMin + random() % (randMax - randMin));
#else
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<int> disRand(randMin, randMax);
    return disRand(genRand);
#endif
}

int64_t GenRandLongLong(int64_t randMin, int64_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<long long> disRand(randMin, randMax);
    return disRand(genRand);
}
} // namespace DistributedHardware
} // namespace OHOS
