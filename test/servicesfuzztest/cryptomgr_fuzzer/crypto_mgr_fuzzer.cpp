/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <fuzzer/FuzzedDataProvider.h>

#include "crypto_mgr.h"
#include "crypto_mgr_fuzzer.h"
#include "dm_constants.h"
#include "json_object.h"


namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr uint32_t CIPHER_TEXT_LEN = 128;
    constexpr uint32_t PLAIN_TEXT_SIZE = 5;
    constexpr uint32_t PLAIN_LEN = 50;
    constexpr uint32_t CIPHER_TEXT_SIZE = 30;
    constexpr uint32_t KEY_LEN = 1024;
}

std::shared_ptr<CryptoMgr> cryproMgr_ = std::make_shared<CryptoMgr>();
void CryptoMgrFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string inputMsg(reinterpret_cast<const char*>(data), size);
    std::string outputMsg(reinterpret_cast<const char*>(data), size);
    cryproMgr_->EncryptMessage(inputMsg, outputMsg);
    cryproMgr_->DecryptMessage(inputMsg, outputMsg);

    AesGcmCipherKey *cipherKey = nullptr;
    unsigned char *input = nullptr;
    uint32_t inLen = 0;
    unsigned char *encryptData = nullptr;
    uint32_t *encryptLen = nullptr;
    cryproMgr_->DoEncryptData(cipherKey, input, inLen, encryptData, encryptLen);

    AesGcmCipherKey cipherKeyPro;
    cipherKey = &cipherKeyPro;
    std::string cipherStr(reinterpret_cast<const char*>(data), size);
    input = reinterpret_cast<unsigned char *>(cipherStr.data());
    inLen = static_cast<uint32_t>(cipherStr.length());
    std::string encryStr(reinterpret_cast<const char*>(data), size);
    encryptData = reinterpret_cast<unsigned char *>(encryStr.data());
    uint32_t encryNum = static_cast<uint32_t>(encryStr.length());
    encryptLen = &encryNum;
    cryproMgr_->DoEncryptData(cipherKey, input, inLen, encryptData, encryptLen);

    unsigned char *randStr = nullptr;
    uint32_t len = 0;
    cryproMgr_->GenerateRandomArray(randStr, len);
    std::string randStrTemp(reinterpret_cast<const char*>(data), size);
    randStr = reinterpret_cast<unsigned char *>(randStrTemp.data());
    len = static_cast<uint32_t>(randStrTemp.length());
    cryproMgr_->GenerateRandomArray(randStr, len);
}

void CryptoMgrFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    AesGcmCipherKey *cipherKey = nullptr;
    unsigned char *plainText = nullptr;
    uint32_t plainTextSize = 0;
    unsigned char *cipherText = nullptr;
    uint32_t cipherTextLen = 0;
    uint32_t cipherTextSize = 0;
    unsigned char *plain = nullptr;
    uint32_t plainLen = 0;
    cryproMgr_->MbedAesGcmEncrypt(cipherKey, plainText, plainTextSize, cipherText, cipherTextLen);
    cryproMgr_->MbedAesGcmDecrypt(cipherKey, cipherText, cipherTextSize, plain, plainLen);
    AesGcmCipherKey cipherKeyPro;
    cipherKey = &cipherKeyPro;
    char plainTextTemp[PLAIN_TEXT_SIZE] = {1};
    plainText = reinterpret_cast<unsigned char *>(plainTextTemp);
    plainTextSize = PLAIN_TEXT_SIZE;
    char cipherTextTemp[CIPHER_TEXT_LEN] = {11};
    cipherText = reinterpret_cast<unsigned char*>(cipherTextTemp);
    cipherTextLen = CIPHER_TEXT_LEN;
    cryproMgr_->MbedAesGcmEncrypt(cipherKey, plainText, plainTextSize, cipherText, cipherTextLen);

    char cipherTextInfo[CIPHER_TEXT_SIZE] = {10};
    cipherText = reinterpret_cast<unsigned char*>(cipherTextInfo);
    cipherTextSize = CIPHER_TEXT_SIZE;
    char  plainTemp[PLAIN_LEN] = {12};
    plain = reinterpret_cast<unsigned char*>(plainTemp);
    plainLen = PLAIN_LEN;
    cryproMgr_->MbedAesGcmDecrypt(cipherKey, cipherText, cipherTextSize, plain, plainLen);
}


void CryptoMgrSecondFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    AesGcmCipherKey *cipherKey = nullptr;
    unsigned char *input = nullptr;
    uint32_t inLen = 0;
    unsigned char *decryptData = nullptr;
    uint32_t *decryptLen = nullptr;
    cryproMgr_->DoDecryptData(cipherKey, input, inLen, decryptData, decryptLen);

    AesGcmCipherKey cipherKeyPro;
    cipherKey = &cipherKeyPro;
    std::string inputTemp(reinterpret_cast<const char*>(data), size);
    input = reinterpret_cast<unsigned char *>(inputTemp.data());
    inLen = static_cast<uint32_t>(inputTemp.length());
    std::string decryInfo(reinterpret_cast<const char*>(data), size);
    decryptData = reinterpret_cast<unsigned char*>(decryInfo.data());
    uint32_t decryNum = static_cast<uint32_t>(decryInfo.length());
    decryptLen = &decryNum;
    cryproMgr_->DoDecryptData(cipherKey, input, inLen, decryptData, decryptLen);

    std::string sessionInfo(reinterpret_cast<const char*>(data), size);
    uint8_t *sessionKey = reinterpret_cast<uint8_t *>(sessionInfo.data());
    uint32_t keyLen = static_cast<uint32_t>(sessionInfo.length());
    cryproMgr_->SaveSessionKey(sessionKey, keyLen);
    cryproMgr_->ClearSessionKey();
    keyLen = KEY_LEN;
    cryproMgr_->SaveSessionKey(sessionKey, keyLen);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CryptoMgrFuzzTest(data, size);
    OHOS::DistributedHardware::CryptoMgrFirstFuzzTest(data, size);
    OHOS::DistributedHardware::CryptoMgrSecondFuzzTest(data, size);
    return 0;
}
