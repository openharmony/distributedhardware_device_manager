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


#include "dm_crypto.h"
#include "dm_anonymous.h"
#include "dm_log.h"

#include "kv_adapter_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "securec.h"

#include <mbedtls/sha256.h>
#define DM_SHA256_DIGEST_LENGTH 32

#define DM_HEX_TO_UINT8 2
#define DM_WIDTH 4
#define DM_MASK 0x0F
#define DM_DEC_MAX_NUM 10
#define DM_HEX_MAX_BIT_NUM 4
#define DM_SHORT_DEVICE_ID_HASH_LENGTH 16
#define DM_SALT_LENGTH 8
#define DM_SHORT_ACCOUNTID_ID_HASH_LENGTH 6
#define DM_DB_KEY_DELIMITER "###"

static const char* DM_SALT_DEFAULT = "salt_defsalt_def";

static uint32_t DmHexifyLen(uint32_t len)
{
    return len * DM_HEX_TO_UINT8 + 1;
}

DM_EXPORT void DmGenerateStrHash(const void* data, size_t dataSize, unsigned char* outBuf,
    uint32_t outBufLen, uint32_t startIndex)
{
    if (data == NULL || outBuf == NULL || startIndex > outBufLen) {
        LOGE("Invalied param.");
        return;
    }
    unsigned char hash[DM_SHA256_DIGEST_LENGTH];
    (void)mbedtls_sha256((const unsigned char*)data, dataSize, hash, 0);
    if (memcpy_s(&outBuf[startIndex], outBufLen - startIndex, hash, DM_SHA256_DIGEST_LENGTH) != 0) {
        return;
    }
}

DM_EXPORT int32_t DmConvertBytesToHexString(char* outBuf, uint32_t outBufLen,
    const unsigned char* inBuf, uint32_t inLen)
{
    if ((outBuf == NULL) || (inBuf == NULL) || (outBufLen < DmHexifyLen(inLen))) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    while (inLen > 0) {
        unsigned char h = *inBuf / 16;
        unsigned char l = *inBuf % 16;
        if (h < DM_DEC_MAX_NUM) {
            *outBuf++ = '0' + h;
        } else {
            *outBuf++ = 'a' + h - DM_DEC_MAX_NUM;
        }
        if (l < DM_DEC_MAX_NUM) {
            *outBuf++ = '0' + l;
        } else {
            *outBuf++ = 'a' + l - DM_DEC_MAX_NUM;
        }
        ++inBuf;
        inLen--;
    }
    return DM_OK;
}

DmString DmCryptoSha256Str(const DmString* text, bool isUpper)
{
    return DmCryptoSha256Data(DmStringCstr(text), DmStringSize(text), isUpper);
}

DmString DmCryptoSha256Data(const void* data, size_t size, bool isUpper)
{
    unsigned char hashBuf[DM_SHA256_DIGEST_LENGTH * DM_HEX_TO_UINT8 + 1];
    if (memset_s(hashBuf, sizeof(hashBuf), 0, sizeof(hashBuf)) != 0) {
        return DmStringCreateEmpty();
    }
    DmGenerateStrHash(data, size, hashBuf, DmHexifyLen(DM_SHA256_DIGEST_LENGTH), DM_SHA256_DIGEST_LENGTH);
    const char* hexCode = isUpper ? "0123456789ABCDEF" : "0123456789abcdef";
    for (int32_t i = 0; i < DM_SHA256_DIGEST_LENGTH; ++i) {
        unsigned char value = hashBuf[DM_SHA256_DIGEST_LENGTH + i];
        hashBuf[i * DM_HEX_TO_UINT8] = hexCode[(value >> DM_WIDTH) & DM_MASK];
        hashBuf[i * DM_HEX_TO_UINT8 + 1] = hexCode[value & DM_MASK];
    }
    hashBuf[DM_SHA256_DIGEST_LENGTH * DM_HEX_TO_UINT8] = 0;
    return DmStringCreate((const char*)hashBuf);
}

int32_t DmGetUdidHashBuf(const DmString* udid, unsigned char* udidHash)
{
    unsigned char hash[DM_SHA256_DIGEST_LENGTH];
    if (memset_s(hash, sizeof(hash), 0, sizeof(hash)) != 0) {
        return ERR_DM_FAILED;
    }
    DmGenerateStrHash(DmStringCstr(udid), DmStringSize(udid), hash, DM_SHA256_DIGEST_LENGTH, 0);
    if (DmConvertBytesToHexString((char*)udidHash, DM_SHORT_DEVICE_ID_HASH_LENGTH + 1,
        (const uint8_t*)hash, DM_SHORT_DEVICE_ID_HASH_LENGTH / DM_HEX_TO_UINT8) != DM_OK) {
        LOGE("ConvertBytesToHexString failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT DmString DmGetUdidHashStr(const DmString* udid)
{
    unsigned char hash[DM_SHA256_DIGEST_LENGTH];
    if (memset_s(hash, sizeof(hash), 0, sizeof(hash)) != 0) {
        return DmStringCreateEmpty();
    }
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    DmGenerateStrHash(DmStringCstr(udid), DmStringSize(udid), hash, DM_SHA256_DIGEST_LENGTH, 0);
    if (DmConvertBytesToHexString(udidHash, DM_SHORT_DEVICE_ID_HASH_LENGTH + 1,
        (const uint8_t*)hash, DM_SHORT_DEVICE_ID_HASH_LENGTH / DM_HEX_TO_UINT8) != DM_OK) {
        LOGE("ConvertBytesToHexString failed.");
        return DmStringCreateEmpty();
    }
    return DmStringCreate(udidHash);
}

DM_EXPORT DmString DmGetTokenIdHash(const DmString* tokenId)
{
    unsigned char hash[DM_SHA256_DIGEST_LENGTH];
    if (memset_s(hash, sizeof(hash), 0, sizeof(hash)) != 0) {
        return DmStringCreateEmpty();
    }
    char idHash[DM_MAX_DEVICE_ID_LEN] = {0};
    DmGenerateStrHash(DmStringCstr(tokenId), DmStringSize(tokenId), hash, DM_SHA256_DIGEST_LENGTH, 0);
    if (DmConvertBytesToHexString(idHash, DM_SHA256_DIGEST_LENGTH + 1,
        (const uint8_t*)hash, DM_SHA256_DIGEST_LENGTH / DM_HEX_TO_UINT8) != DM_OK) {
        LOGE("ConvertBytesToHexString failed.");
        return DmStringCreateEmpty();
    }
    return DmStringCreate(idHash);
}

DM_EXPORT int32_t DmConvertHexStringToBytes(unsigned char* outBuf,
    uint32_t outBufLen, const char* inBuf, uint32_t inLen)
{
    if ((outBuf == NULL) || (inBuf == NULL) || (inLen % DM_HEX_TO_UINT8 != 0)) {
        LOGE("invalid param");
        return ERR_DM_FAILED;
    }
    uint32_t outLen = inLen / DM_HEX_TO_UINT8;
    if (outBufLen < outLen) {
        LOGE("out of memory.");
        return ERR_DM_FAILED;
    }
    uint32_t i = 0;
    while (i < outLen) {
        unsigned char c = *inBuf++;
        if ((c >= '0') && (c <= '9')) {
            c -= '0';
        } else if ((c >= 'a') && (c <= 'f')) {
            c -= 'a' - DM_DEC_MAX_NUM;
        } else if ((c >= 'A') && (c <= 'F')) {
            c -= 'A' - DM_DEC_MAX_NUM;
        } else {
            LOGE("HexToString Error! %{public}c", c);
            return ERR_DM_FAILED;
        }
        unsigned char c2 = *inBuf++;
        if ((c2 >= '0') && (c2 <= '9')) {
            c2 -= '0';
        } else if ((c2 >= 'a') && (c2 <= 'f')) {
            c2 -= 'a' - DM_DEC_MAX_NUM;
        } else if ((c2 >= 'A') && (c2 <= 'F')) {
            c2 -= 'A' - DM_DEC_MAX_NUM;
        } else {
            LOGE("HexToString Error! %{public}c", c2);
            return ERR_DM_FAILED;
        }
        *outBuf++ = (c << DM_HEX_MAX_BIT_NUM) | c2;
        i++;
    }
    return DM_OK;
}

DM_EXPORT DmString DmGetGroupIdHash(const DmString* groupId)
{
    unsigned char hash[DM_SHA256_DIGEST_LENGTH];
    if (memset_s(hash, sizeof(hash), 0, sizeof(hash)) != 0) {
        return DmStringCreateEmpty();
    }
    DmGenerateStrHash(DmStringCstr(groupId), DmStringSize(groupId), hash, DM_SHA256_DIGEST_LENGTH, 0);
    char hexBuf[DM_SHA256_DIGEST_LENGTH * DM_HEX_TO_UINT8 + 1] = {0};
    (void)DmConvertBytesToHexString(hexBuf, DM_SHA256_DIGEST_LENGTH * DM_HEX_TO_UINT8 + 1, hash,
        DM_SHA256_DIGEST_LENGTH);
    DmString fullHash = DmStringCreate(hexBuf);
    DmString result = DmStringSubstr(&fullHash, 0, DM_SHORT_DEVICE_ID_HASH_LENGTH);
    DmStringDestroy(&fullHash);
    return result;
}

int32_t DmGetSecRandom(uint8_t* out, size_t outLen)
{
    if (out == NULL || outLen == 0) {
        return -1;
    }
    FILE* f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        return -1;
    }
    size_t rd = fread(out, 1, outLen, f);
    (void)fclose(f);
    if (rd != outLen) {
        return -1;
    }
    return DM_OK;
}

DmString DmGetSecSalt(void)
{
    uint8_t out[DM_SALT_LENGTH] = {0};
    if (DmGetSecRandom(out, DM_SALT_LENGTH) != DM_OK) {
        return DmStringCreate(DM_SALT_DEFAULT);
    }
    char outHex[DM_SALT_LENGTH * DM_HEX_TO_UINT8 + 1] = {0};
    if (DmConvertBytesToHexString(outHex, DM_SALT_LENGTH * DM_HEX_TO_UINT8 + 1, out, DM_SALT_LENGTH) != DM_OK) {
        return DmStringCreate(DM_SALT_DEFAULT);
    }
    return DmStringCreate(outHex);
}

DmString DmGetHashWithSalt(const DmString* text, const DmString* salt)
{
    DmString rawText = DmStringCopy(text);
    DmStringAppend(&rawText, DmStringCstr(salt));
    DmString result = DmCryptoSha256Str(&rawText, false);
    DmStringDestroy(&rawText);
    return result;
}

DM_EXPORT int32_t DmGetAccountIdHashBuf(const DmString* accountId, unsigned char* accountIdHash)
{
    unsigned char hash[DM_SHA256_DIGEST_LENGTH];
    if (memset_s(hash, sizeof(hash), 0, sizeof(hash)) != 0) {
        return ERR_DM_FAILED;
    }
    DmGenerateStrHash(DmStringCstr(accountId), DmStringSize(accountId), hash, DM_SHA256_DIGEST_LENGTH, 0);
    if (DmConvertBytesToHexString((char*)accountIdHash, DM_SHORT_ACCOUNTID_ID_HASH_LENGTH + 1,
        (const uint8_t*)hash, DM_SHORT_ACCOUNTID_ID_HASH_LENGTH / DM_HEX_TO_UINT8) != DM_OK) {
        LOGE("ConvertBytesToHexString failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT DmString DmGetAccountIdHash16(const DmString* accountId)
{
    unsigned char hash[DM_SHA256_DIGEST_LENGTH];
    if (memset_s(hash, sizeof(hash), 0, sizeof(hash)) != 0) {
        return DmStringCreateEmpty();
    }
    char accountIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    DmGenerateStrHash(DmStringCstr(accountId), DmStringSize(accountId), hash, DM_SHA256_DIGEST_LENGTH, 0);
    if (DmConvertBytesToHexString(accountIdHash, DM_SHA256_DIGEST_LENGTH + 1,
        (const uint8_t*)hash, DM_SHA256_DIGEST_LENGTH / DM_HEX_TO_UINT8) != DM_OK) {
        LOGE("ConvertBytesToHexString failed.");
        return DmStringCreateEmpty();
    }
    return DmStringCreate(accountIdHash);
}
