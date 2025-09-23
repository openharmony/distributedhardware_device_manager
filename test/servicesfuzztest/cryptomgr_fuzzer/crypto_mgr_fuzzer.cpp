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
    constexpr uint32_t KEY_LEN = 1024;
}

std::shared_ptr<CryptoMgr> cryproMgr_ = std::make_shared<CryptoMgr>();
void CryptoMgrFirstFuzzTest(FuzzedDataProvider &fdp)
{
    std::string inputMsg = fdp.ConsumeRandomLengthString();
    std::string outputMsg = fdp.ConsumeRandomLengthString();
    cryproMgr_->EncryptMessage(inputMsg, outputMsg);
    cryproMgr_->DecryptMessage(inputMsg, outputMsg);

    AesGcmCipherKey *cipherKey = nullptr;
    uint32_t inLen = 32;
    unsigned char *input = fdp.ConsumeBytes<uint8_t>(inLen).data();
    unsigned char *encryptData = nullptr;
    uint32_t *encryptLen = nullptr;
    cryproMgr_->DoEncryptData(cipherKey, input, inLen, encryptData, encryptLen);

    AesGcmCipherKey cipherKeyPro;
    cipherKey = &cipherKeyPro;
    std::string cipherStr = fdp.ConsumeRandomLengthString();
    inLen = static_cast<uint32_t>(cipherStr.length());
    std::string encryStr = fdp.ConsumeRandomLengthString();
    encryptData = reinterpret_cast<unsigned char *>(encryStr.data());
    uint32_t encryNum = static_cast<uint32_t>(encryStr.length());
    encryptLen = &encryNum;
    cryproMgr_->DoEncryptData(cipherKey, input, inLen, encryptData, encryptLen);

    std::string randStrTemp = fdp.ConsumeRandomLengthString();
    unsigned char *randStr = reinterpret_cast<unsigned char *>(randStrTemp.data());
    uint32_t len = static_cast<uint32_t>(randStrTemp.length());
    cryproMgr_->GenerateRandomArray(randStr, len);
    uint32_t plainTextSize = 32;
    unsigned char *plainText = fdp.ConsumeBytes<uint8_t>(inLen).data();
    uint32_t cipherTextLen = 32;
    unsigned char *cipherText = fdp.ConsumeBytes<uint8_t>(inLen).data();
    cryproMgr_->MbedAesGcmEncrypt(cipherKey, plainText, plainTextSize, cipherText, cipherTextLen);
    cryproMgr_->MbedAesGcmDecrypt(cipherKey, cipherText, cipherTextLen, plainText, plainTextSize);
}

void CryptoMgrSecondFuzzTest(FuzzedDataProvider &fdp)
{
    AesGcmCipherKey *cipherKey = nullptr;
    unsigned char *input = nullptr;
    uint32_t inLen = 0;
    unsigned char *decryptData = nullptr;
    uint32_t *decryptLen = nullptr;

    AesGcmCipherKey cipherKeyPro;
    cipherKey = &cipherKeyPro;
    std::string inputTemp = fdp.ConsumeRandomLengthString();
    input = reinterpret_cast<unsigned char *>(inputTemp.data());
    inLen = static_cast<uint32_t>(inputTemp.length());
    std::string decryInfo = fdp.ConsumeRandomLengthString();
    decryptData = reinterpret_cast<unsigned char*>(decryInfo.data());
    uint32_t decryNum = static_cast<uint32_t>(decryInfo.length());
    decryptLen = &decryNum;
    cryproMgr_->DoDecryptData(cipherKey, input, inLen, decryptData, decryptLen);

    std::string sessionInfo = fdp.ConsumeRandomLengthString();
    uint8_t *sessionKey = reinterpret_cast<uint8_t *>(sessionInfo.data());
    uint32_t keyLen = static_cast<uint32_t>(sessionInfo.length());
    cryproMgr_->SaveSessionKey(sessionKey, keyLen);
    cryproMgr_->ClearSessionKey();
    keyLen = KEY_LEN;
    cryproMgr_->SaveSessionKey(sessionKey, keyLen);
}
    
void CryptoMgrFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    CryptoMgrFirstFuzzTest(fdp);
    CryptoMgrSecondFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CryptoMgrFuzzTest(data, size);
    return 0;
}
