/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dm_softbus_adapter_crypto.h"

#include "dm_constants.h"
#include "dm_log.h"
#include "md.h"
#include "securec.h"
#include <sstream>
#include <string>

namespace OHOS {
namespace DistributedHardware {

namespace {
constexpr int SHA_HASH_LEN = 32;
constexpr int SHORT_DEVICE_ID_HASH_LENGTH = 16;
constexpr int HEXIFY_UNIT_LEN = 2;
constexpr int HEX_DIGIT_MAX_NUM = 16;
constexpr int DEC_MAX_NUM = 10;
constexpr int HEX_MAX_BIT_NUM = 4;

#define HEXIFY_LEN(len) ((len) * HEXIFY_UNIT_LEN + 1)
} // namespace

int32_t DmGenerateStrHash(const unsigned char *str, uint32_t len, unsigned char *hash)
{
    if (str == nullptr || hash == nullptr || len == 0) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info = NULL;
    mbedtls_md_init(&ctx);

    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (mbedtls_md_setup(&ctx, info, 0) != 0) {
        mbedtls_md_free(&ctx);
        return ERR_DM_FAILED;
    }
    if (mbedtls_md_starts(&ctx) != 0) {
        mbedtls_md_free(&ctx);
        return ERR_DM_FAILED;
    }
    if (mbedtls_md_update(&ctx, str, len) != 0) {
        mbedtls_md_free(&ctx);
        return ERR_DM_FAILED;
    }
    if (mbedtls_md_finish(&ctx, hash) != 0) {
        mbedtls_md_free(&ctx);
        return ERR_DM_FAILED;
    }

    mbedtls_md_free(&ctx);
    return DM_OK;
}

int32_t ConvertBytesToHexString(char *outBuf, uint32_t outBufLen, const unsigned char *inBuf,
    uint32_t inLen)
{
    if ((outBuf == nullptr) || (inBuf == nullptr) || (outBufLen < HEXIFY_LEN(inLen))) {
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

int32_t DmSoftbusAdapterCrypto::ConvertHexStringToBytes(unsigned char *outBuf, uint32_t outBufLen, const char *inBuf,
    uint32_t inLen)
{
    (void)outBufLen;
    if ((outBuf == NULL) || (inBuf == NULL) || (inLen % HEXIFY_UNIT_LEN != 0)) {
        LOGE("invalid param");
        return ERR_DM_FAILED;
    }

    uint32_t outLen = inLen / HEXIFY_UNIT_LEN;
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
            LOGE("HexToString Error! %c", c);
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
            LOGE("HexToString Error! %c", c2);
            return ERR_DM_FAILED;
        }
        *outBuf++ = (c << HEX_MAX_BIT_NUM) | c2;
        i++;
    }
    return DM_OK;
}

int32_t DmSoftbusAdapterCrypto::GetUdidHash(const std::string &udid, unsigned char *udidHash)
{
    char hashResult[SHA_HASH_LEN] = {0};
    int32_t ret = DmGenerateStrHash((const uint8_t *)udid.c_str(), strlen(udid.c_str()), (uint8_t *)hashResult);
    if (ret != DM_OK) {
        LOGE("GenerateStrHash failed");
        return ret;
    }
    ret = ConvertBytesToHexString((char *)udidHash, SHORT_DEVICE_ID_HASH_LENGTH + 1, (const uint8_t *)hashResult,
        SHORT_DEVICE_ID_HASH_LENGTH / HEXIFY_UNIT_LEN);
    if (ret != DM_OK) {
        LOGE("ConvertBytesToHexString failed");
        return ret;
    }
    return DM_OK;
}

std::string DmSoftbusAdapterCrypto::GetGroupIdHash(const std::string &groupId)
{
    char hashResult[SHA_HASH_LEN] = {0};
    int32_t ret = DmGenerateStrHash(reinterpret_cast<const uint8_t *>(groupId.c_str()), strlen(groupId.c_str()),
        reinterpret_cast<uint8_t *>(hashResult));
    if (ret != DM_OK) {
        LOGE("GenerateStrHash failed");
        return "";
    }

    std::stringstream ss;
    for (int i = 0; i < SHA_HASH_LEN; i++) {
        ss << std::hex << (int)hashResult[i];
    }
    return ss.str().substr(0, SHORT_DEVICE_ID_HASH_LENGTH);
}
} // namespace DistributedHardware
} // namespace OHOS