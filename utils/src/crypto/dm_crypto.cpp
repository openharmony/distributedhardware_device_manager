/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "dm_crypto.h"

#include <iostream>
#include <random>
#include <sstream>

#include "openssl/sha.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
constexpr int32_t HEX_TO_UINT8 = 2;
constexpr int WIDTH = 4;
constexpr unsigned char MASK = 0x0F;
} // namespace

std::string Crypto::Sha256(const std::string &text, bool isUpper)
{
    return Sha256(text.data(), text.size(), isUpper);
}

std::string Crypto::Sha256(const void *data, size_t size, bool isUpper)
{
    unsigned char hash[SHA256_DIGEST_LENGTH * HEX_TO_UINT8 + 1] = "";
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, size);
    SHA256_Final(&hash[SHA256_DIGEST_LENGTH], &ctx);
    // here we translate sha256 hash to hexadecimal. each 8-bit char will be presented by two characters([0-9a-f])
    const char* hexCode = isUpper ? "0123456789ABCDEF" : "0123456789abcdef";
    for (int32_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char value = hash[SHA256_DIGEST_LENGTH + i];
        // uint8_t is 2 digits in hexadecimal.
        hash[i * HEX_TO_UINT8] = hexCode[(value >> WIDTH) & MASK];
        hash[i * HEX_TO_UINT8 + 1] = hexCode[value & MASK];
    }
    hash[SHA256_DIGEST_LENGTH * HEX_TO_UINT8] = 0;
    std::stringstream ss;
    ss << hash;
    return ss.str();
}
} // namespace DistributedHardware
} // namespace OHOS
