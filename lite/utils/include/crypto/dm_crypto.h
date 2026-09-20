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


#ifndef DM_CRYPTO_H
#define DM_CRYPTO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

#define DM_MAX_DEVICE_ID_LEN 97

DM_EXPORT void DmGenerateStrHash(const void* data, size_t dataSize,
    unsigned char* outBuf, uint32_t outBufLen, uint32_t startIndex);
DM_EXPORT DmString DmCryptoSha256Str(const DmString* text, bool isUpper);
DM_EXPORT DmString DmCryptoSha256Data(const void* data, size_t size, bool isUpper);
DM_EXPORT int32_t DmConvertHexStringToBytes(unsigned char* outBuf,
    uint32_t outBufLen, const char* inBuf, uint32_t inLen);
DM_EXPORT int32_t DmGetUdidHashBuf(const DmString* udid, unsigned char* udidHash);
DM_EXPORT DmString DmGetUdidHashStr(const DmString* udid);
DM_EXPORT DmString DmGetTokenIdHash(const DmString* tokenId);
DM_EXPORT DmString DmGetGroupIdHash(const DmString* groupId);
int32_t DmGetSecRandom(uint8_t* out, size_t outLen);
DmString DmGetSecSalt(void);
DmString DmGetHashWithSalt(const DmString* text, const DmString* salt);
DM_EXPORT int32_t DmGetAccountIdHashBuf(const DmString* accountId, unsigned char* accountIdHash);
DM_EXPORT DmString DmGetAccountIdHash16(const DmString* accountId);
DM_EXPORT int32_t DmConvertBytesToHexString(char* outBuf, uint32_t outBufLen,
    const unsigned char* inBuf, uint32_t inLen);

typedef struct DmKVValue {
    DmString udidHash;
    DmString appID;
    DmString anoyDeviceId;
    DmString salt;
    int64_t lastModifyTime;
} DmKVValue;

#ifdef __cplusplus
}
#endif

#endif
