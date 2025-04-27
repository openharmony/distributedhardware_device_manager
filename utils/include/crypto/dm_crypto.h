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

#ifndef OHOS_DM_CRYPTO_H
#define OHOS_DM_CRYPTO_H
#include <string>
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_kv_info.h"
#endif

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT


namespace OHOS {
namespace DistributedHardware {
class Crypto {
public:
    DM_EXPORT static void DmGenerateStrHash(const void *data, size_t dataSize,
        unsigned char *outBuf, uint32_t outBufLen, uint32_t startIndex);
    DM_EXPORT static std::string Sha256(const std::string &text, bool isUpper = false);
    DM_EXPORT static std::string Sha256(const void *data, size_t size,
        bool isUpper = false);
    DM_EXPORT static int32_t ConvertHexStringToBytes(unsigned char *outBuf,
        uint32_t outBufLen, const char *inBuf, uint32_t inLen);
    // follow the dsoftbus udid hash policy, use the first 8 bytes of full udid hash,
    // after convert to string, it it 16 bytes.
    DM_EXPORT static int32_t GetUdidHash(const std::string &udid,
        unsigned char *udidHash);
    DM_EXPORT static std::string GetUdidHash(const std::string &udid);
    // use the first 16 bytes of full tokenId hash
    // after convert to string, it it 32 bytes.
    DM_EXPORT static std::string GetTokenIdHash(const std::string &tokenId);
    DM_EXPORT static std::string GetGroupIdHash(const std::string &groupId);
    static int32_t GetSecRandom(uint8_t *out, size_t outLen);
    static std::string GetSecSalt();
    static std::string GetHashWithSalt(const std::string &text, const std::string &salt);
    // follow the dsoftbus accountid hash policy, use the first 3 bytes of full accountid hash,
    // after convert to string, it it 6 bytes.
    DM_EXPORT static int32_t GetAccountIdHash(const std::string &accountId,
        unsigned char *accountIdHash);
    // use the first 16 bytes of full accountId hash
    // after convert to string, it it 32 bytes.
    DM_EXPORT static std::string GetAccountIdHash16(const std::string &accountId);
    DM_EXPORT static int32_t ConvertBytesToHexString(char *outBuf, uint32_t outBufLen,
        const unsigned char *inBuf, uint32_t inLen);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DM_EXPORT static int32_t ConvertUdidHashToAnoyAndSave(const std::string &appId,
        const std::string &udidHash, DmKVValue &kvValue);
    DM_EXPORT static int32_t ConvertUdidHashToAnoyDeviceId(const std::string &appId,
        const std::string &udidHash, DmKVValue &kvValue);
    static int32_t GetAnoyDeviceInfo(const std::string &appId, const std::string &udidHash, DmKVValue &kvValue);
    static int32_t ConvertUdidHashToAnoyGenerate(const std::string &appId, const std::string &udidHash,
        DmKVValue &kvValue);
#endif
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CRYPTO_H
