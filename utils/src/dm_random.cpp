/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dm_log.h"

#if defined(__LITEOS_M__)
#include <time.h>
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t DM_MIN_RANDOM = 1;
const int32_t DM_MAX_RANDOM_UINT16 = INT16_MAX;
const int32_t DM_INVALID_FLAG_ID = 0;
const int32_t DM_MIN_PINCODE_SIZE = 6;
const int32_t DM_MAX_PINCODE_SIZE = 1024;
const int32_t DM_MAX_RANDOM = 9;
}

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

DM_EXPORT int64_t GenRandLongLong(int64_t randMin, int64_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<long long> disRand(randMin, randMax);
    return disRand(genRand);
}

DM_EXPORT uint64_t GenRandUnLongLong(uint64_t randMin, uint64_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<unsigned long long> disRand(randMin, randMax);
    return disRand(genRand);
}

uint16_t GenRandUint(uint16_t randMin, uint16_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<int> disRand(randMin, randMax);
    return disRand(genRand);
}

uint16_t GenUniqueRandUint(std::set<uint16_t> &randUint16Set)
{
    uint16_t randUint = DM_INVALID_FLAG_ID;
    bool isExist = false;
    do {
        randUint = GenRandInt(DM_MIN_RANDOM, DM_MAX_RANDOM_UINT16);
        if (randUint16Set.find(randUint) != randUint16Set.end()) {
            LOGE("The randUint: %{public}d is exist.", randUint);
            isExist = true;
        } else {
            isExist = false;
        }
    } while (isExist);
    randUint16Set.emplace(randUint);
    return randUint;
}

std::string GeneratePinCode(uint32_t pinLength)
{
    if (pinLength < DM_MIN_PINCODE_SIZE || pinLength > DM_MAX_PINCODE_SIZE) {
        LOGE("pinLength error: Invalid para");
        return "";
    }
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::string pinCode = std::to_string(GenRandInt(DM_MIN_RANDOM, DM_MAX_RANDOM));
    uint32_t left_digit_count = pinLength - 1;
    while (pinCode.length() < left_digit_count) {
        uint64_t rest_num = gen();
        pinCode += std::to_string(rest_num);
    }
    pinCode = pinCode.substr(0, pinLength);
    return pinCode;
}
} // namespace DistributedHardware
} // namespace OHOS
