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
#include "dm_log.h"
#include <iostream>
#include <sstream>

#include "openssl/sha.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t HEX_TO_UINT8 = 2;
constexpr int WIDTH = 4;
constexpr unsigned char MASK = 0x0F;
constexpr int DEC_MAX_NUM = 10;
constexpr int HEX_MAX_BIT_NUM = 4;
constexpr uint32_t ERR_DM_FAILED = -20000;
constexpr int32_t DM_OK = 0;
constexpr int32_t ERR_DM_INPUT_PARA_INVALID = -20006;
constexpr int HEX_DIGIT_MAX_NUM = 16;
constexpr int SHORT_DEVICE_ID_HASH_LENGTH = 16;

uint32_t HexifyLen(uint32_t len)
{
    return len * HEX_TO_UINT8 + 1;
}

void DmGenerateStrHash(const void *data, size_t dataSize, unsigned char *outBuf, uint32_t outBufLen,
    uint32_t startIndex)
{
    if (data == nullptr || outBuf == nullptr || startIndex > outBufLen) {
        LOGE("Invalied param.");
        return;
    }
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, dataSize);
    SHA256_Final(&outBuf[startIndex], &ctx);
}

int32_t ConvertBytesToHexString(char *outBuf, uint32_t outBufLen, const unsigned char *inBuf,
    uint32_t inLen)
{
    if ((outBuf == nullptr) || (inBuf == nullptr) || (outBufLen < HexifyLen(inLen))) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    while (inLen > 0) {
        unsigned char h = *inBuf / HEX_DIGIT_MAX_NUM;
        unsigned char l = *inBuf % HEX_DIGIT_MAX_NUM;
        if (h < DEC_MAX_NUM) {
            *outBuf++ = '0' + h;
        } else {
            *outBuf++ = 'a' + h - DEC_MAX_NUM;
        }
        if (l < DEC_MAX_NUM) {
            *outBuf++ = '0' + l;
        } else {
            *outBuf++ = 'a' + l - DEC_MAX_NUM;
        }
        ++inBuf;
        inLen--;
    }
    return DM_OK;
}

std::string Crypto::Sha256(const std::string &text, bool isUpper)
{
    return Sha256(text.data(), text.size(), isUpper);
}

std::string Crypto::Sha256(const void *data, size_t size, bool isUpper)
{
    unsigned char hash[SHA256_DIGEST_LENGTH * HEX_TO_UINT8 + 1] = "";
    DmGenerateStrHash(data, size, hash, HexifyLen(SHA256_DIGEST_LENGTH), SHA256_DIGEST_LENGTH);
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

int32_t Crypto::GetUdidHash(const std::string &udid, unsigned char *udidHash)
{
    unsigned char hash[SHA256_DIGEST_LENGTH] = "";
    DmGenerateStrHash(udid.data(), udid.size(), hash, SHA256_DIGEST_LENGTH, 0);
    if (ConvertBytesToHexString(reinterpret_cast<char *>(udidHash), SHORT_DEVICE_ID_HASH_LENGTH + 1,
        reinterpret_cast<const uint8_t *>(hash), SHORT_DEVICE_ID_HASH_LENGTH / HEX_TO_UINT8) != DM_OK) {
        LOGE("ConvertBytesToHexString failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t Crypto::ConvertHexStringToBytes(unsigned char *outBuf, uint32_t outBufLen, const char *inBuf,
    uint32_t inLen)
{
    (void)outBufLen;
    if ((outBuf == NULL) || (inBuf == NULL) || (inLen % HEX_TO_UINT8 != 0)) {
        LOGE("invalid param");
        return ERR_DM_FAILED;
    }

    uint32_t outLen = inLen / HEX_TO_UINT8;
    uint32_t i = 0;
    while (i < outLen) {
        unsigned char c = *inBuf++;
        if ((c >= '0') && (c <= '9')) {
            c -= '0';
        } else if ((c >= 'a') && (c <= 'f')) {
            c -= 'a' - DEC_MAX_NUM;
        } else if ((c >= 'A') && (c <= 'F')) {
            c -= 'A' - DEC_MAX_NUM;
        } else {
            LOGE("HexToString Error! %{public}c", c);
            return ERR_DM_FAILED;
        }
        unsigned char c2 = *inBuf++;
        if ((c2 >= '0') && (c2 <= '9')) {
            c2 -= '0';
        } else if ((c2 >= 'a') && (c2 <= 'f')) {
            c2 -= 'a' - DEC_MAX_NUM;
        } else if ((c2 >= 'A') && (c2 <= 'F')) {
            c2 -= 'A' - DEC_MAX_NUM;
        } else {
            LOGE("HexToString Error! %{public}c", c2);
            return ERR_DM_FAILED;
        }
        *outBuf++ = (c << HEX_MAX_BIT_NUM) | c2;
        i++;
    }
    return DM_OK;
}

std::string Crypto::GetGroupIdHash(const std::string &groupId)
{
    unsigned char hash[SHA256_DIGEST_LENGTH] = "";
    DmGenerateStrHash(groupId.data(), groupId.size(), hash, SHA256_DIGEST_LENGTH, 0);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << (int)hash[i];
    }
    return ss.str().substr(0, SHORT_DEVICE_ID_HASH_LENGTH);
}
} // namespace DistributedHardware
} // namespace OHOS
