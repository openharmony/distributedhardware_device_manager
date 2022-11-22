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

#include "dm_constants.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

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

int32_t GetRandomData(uint8_t *randStr, uint32_t len)
{
    mbedtls_entropy_context *entropy = nullptr;
    mbedtls_ctr_drbg_context *ctrDrbg = nullptr;
    int32_t ret = ERR_DM_FAILED;
    do {
        if (randStr == nullptr || len == 0) {
            break;
        }
        entropy = reinterpret_cast<mbedtls_entropy_context *>(malloc(sizeof(mbedtls_entropy_context)));
        if (entropy == nullptr) {
            break;
        }
        ctrDrbg = reinterpret_cast<mbedtls_ctr_drbg_context *>(malloc(sizeof(mbedtls_ctr_drbg_context)));
        if (ctrDrbg == nullptr) {
            break;
        }
        mbedtls_ctr_drbg_init(ctrDrbg);
        mbedtls_entropy_init(entropy);
        ret = mbedtls_ctr_drbg_seed(ctrDrbg, mbedtls_entropy_func, entropy, nullptr, 0);
        if (ret != 0) {
            break;
        }
        ret = mbedtls_ctr_drbg_random(ctrDrbg, randStr, len);
        if (ret != 0) {
            break;
        }
        ret = DM_OK;
    } while (false);
    if (entropy != nullptr) {
        free(entropy);
        entropy = nullptr;
    }
    if (ctrDrbg != nullptr) {
        free(ctrDrbg);
        ctrDrbg = nullptr;
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
