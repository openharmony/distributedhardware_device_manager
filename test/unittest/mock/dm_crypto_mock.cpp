/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dm_crypto_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {


int32_t Crypto::GetAccountIdHash(const std::string &accountId, unsigned char *accountIdHash)
{
    return DmCrypto::dmCrypto->GetAccountIdHash(accountId, accountIdHash);
}

int32_t Crypto::ConvertUdidHashToAnoyAndSave(const std::string &appId, const std::string &udidHash,
    DmKVValue &kvValue)
{
    return DmCrypto::dmCrypto->ConvertUdidHashToAnoyAndSave(appId, udidHash, kvValue);
}

int32_t Crypto::GetUdidHash(const std::string &udid, unsigned char *udidHash)
{
    return DmCrypto::dmCrypto->GetUdidHash(udid, udidHash);
}

std::string Crypto::GetGroupIdHash(const std::string &groupId)
{
    return DmCrypto::dmCrypto->GetGroupIdHash(groupId);
}

int32_t Crypto::ConvertUdidHashToAnoyDeviceId(const std::string &appId, const std::string &udidHash,
    DmKVValue &kvValue)
{
    return DmCrypto::dmCrypto->ConvertUdidHashToAnoyDeviceId(appId, udidHash, kvValue);
}

void Crypto::DmGenerateStrHash(const void *data, size_t dataSize, unsigned char *outBuf, uint32_t outBufLen,
    uint32_t startIndex)
{
    DmCrypto::dmCrypto->DmGenerateStrHash(data, dataSize, outBuf, outBufLen, startIndex);
}

std::string Crypto::Sha256(const std::string &text, bool isUpper)
{
    return DmCrypto::dmCrypto->Sha256(text, isUpper);
}

std::string Crypto::Sha256(const void *data, size_t size, bool isUpper)
{
    return DmCrypto::dmCrypto->Sha256(data, size, isUpper);
}
} // namespace DistributedHardware
} // namespace OHOS