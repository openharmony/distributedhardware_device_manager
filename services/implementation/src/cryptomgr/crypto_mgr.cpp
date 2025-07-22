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

#include "crypto_mgr.h"

#include <atomic>
#include <cstdlib>
#include <string>

#include "mbedtls/base64.h"
#include "mbedtls/cipher.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/gcm.h"
#include "mbedtls/md.h"
#include "mbedtls/platform.h"

#include "securec.h"

#include "dm_error_type.h"
#include "dm_crypto.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;
constexpr uint32_t TAG_LEN = 16;
constexpr uint32_t OVERHEAD_LEN = GCM_IV_LEN + TAG_LEN;
constexpr uint32_t MAX_ENCRY_MSG_LEN = 10 * 1024 * 1024; // 10MB
constexpr uint32_t KEY_BITS_UNIT = 8;
constexpr uint32_t HEX_TO_UINT8 = 2;
CryptoMgr::CryptoMgr()
{
    LOGI("CryptoMgr ctor");
}

CryptoMgr::~CryptoMgr()
{
    LOGI("CryptoMgr dtor");
    ClearSessionKey();
}

int32_t CryptoMgr::EncryptMessage(const std::string &inputMsg, std::string &outputMsg)
{
    if (inputMsg.length() > MAX_ENCRY_MSG_LEN) {
        LOGE("Encrypt msg too long, size: %{public}zu", inputMsg.size());
        return ERR_DM_CRYPTO_PARA_INVALID;
    }

    std::lock_guard<std::mutex> lock(sessionKeyMtx_);
    AesGcmCipherKey cipherKey = {.keyLen = sessionKey_.keyLen};
    if (memcpy_s(cipherKey.key, SESSION_KEY_LENGTH, sessionKey_.key, sessionKey_.keyLen) != EOK) {
        LOGE("set key fail");
        return ERR_DM_CRYPTO_OPT_FAILED;
    }

    uint32_t encDataLen = inputMsg.length() + OVERHEAD_LEN;
    unsigned char *encData = (unsigned char *)calloc(encDataLen, sizeof(unsigned char));
    if (encData == nullptr) {
        LOGE("calloc fail");
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    int32_t ret = DoEncryptData(&cipherKey, reinterpret_cast<const unsigned char*>(inputMsg.c_str()),
        static_cast<uint32_t>(inputMsg.length()), encData, &encDataLen);
    if (ret != DM_OK) {
        LOGE("EncryptData fail=%{public}d", ret);
        free(encData);
        encData = nullptr;
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    if (memset_s(&cipherKey, sizeof(AesGcmCipherKey), 0, sizeof(AesGcmCipherKey)) != DM_OK) {
        LOGE("memset_s failed.");
        free(encData);
        encData = nullptr;
        return ERR_DM_SECURITY_FUNC_FAILED;
    }
    const uint32_t hexStrLen = encDataLen * HEX_TO_UINT8 + 1;
    char hexStrTemp[hexStrLen];
    if (memset_s(hexStrTemp, hexStrLen, 0, hexStrLen) != DM_OK) {
        LOGE("memset_s failed.");
        free(encData);
        encData = nullptr;
        return ERR_DM_SECURITY_FUNC_FAILED;
    }
    Crypto::ConvertBytesToHexString(hexStrTemp, hexStrLen, encData, encDataLen);
    outputMsg.clear();
    outputMsg.assign(hexStrTemp, encDataLen * HEX_TO_UINT8);
    free(encData);
    encData = nullptr;
    return DM_OK;
}

int32_t CryptoMgr::DoEncryptData(AesGcmCipherKey *cipherKey, const unsigned char *input, uint32_t inLen,
    unsigned char *encryptData, uint32_t *encryptLen)
{
    if (cipherKey == nullptr || input == nullptr || inLen == 0 || encryptData == nullptr || encryptLen == nullptr) {
        return ERR_DM_CRYPTO_PARA_INVALID;
    }

    if (GenerateRandomArray(cipherKey->iv, sizeof(cipherKey->iv)) != DM_OK) {
        LOGE("generate random iv error.");
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    uint32_t outLen = inLen + OVERHEAD_LEN;
    int32_t result = MbedAesGcmEncrypt(cipherKey, input, inLen, encryptData, outLen);
    if (result <= 0) {
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    *encryptLen = result;
    return DM_OK;
}

int32_t CryptoMgr::MbedAesGcmEncrypt(const AesGcmCipherKey *cipherKey, const unsigned char *plainText,
    uint32_t plainTextSize, unsigned char *cipherText, uint32_t cipherTextLen)
{
    if ((cipherKey == nullptr) || (plainText == nullptr) || (plainTextSize == 0) || cipherText == nullptr ||
        (cipherTextLen < plainTextSize + OVERHEAD_LEN)) {
        LOGE("Encrypt invalid para");
        return ERR_DM_CRYPTO_PARA_INVALID;
    }

    int32_t ret = -1;
    unsigned char tagBuf[TAG_LEN] = { 0 };
    mbedtls_gcm_context aesContext;
    mbedtls_gcm_init(&aesContext);

    ret = mbedtls_gcm_setkey(&aesContext, MBEDTLS_CIPHER_ID_AES, cipherKey->key, cipherKey->keyLen * KEY_BITS_UNIT);
    if (ret != 0) {
        mbedtls_gcm_free(&aesContext);
        return ERR_DM_CRYPTO_OPT_FAILED;
    }

    ret = mbedtls_gcm_crypt_and_tag(&aesContext, MBEDTLS_GCM_ENCRYPT, plainTextSize, cipherKey->iv, GCM_IV_LEN, NULL, 0,
        plainText, cipherText + GCM_IV_LEN, TAG_LEN, tagBuf);
    if (ret != 0) {
        mbedtls_gcm_free(&aesContext);
        return ERR_DM_CRYPTO_OPT_FAILED;
    }

    if (memcpy_s(cipherText, cipherTextLen, cipherKey->iv, GCM_IV_LEN) != EOK) {
        mbedtls_gcm_free(&aesContext);
        return ERR_DM_CRYPTO_OPT_FAILED;
    }

    if (memcpy_s(cipherText + GCM_IV_LEN + plainTextSize, cipherTextLen - GCM_IV_LEN - plainTextSize, tagBuf,
                 TAG_LEN) != 0) {
        mbedtls_gcm_free(&aesContext);
        return ERR_DM_CRYPTO_OPT_FAILED;
    }

    mbedtls_gcm_free(&aesContext);
    return (plainTextSize + OVERHEAD_LEN);
}

int32_t CryptoMgr::GenerateRandomArray(unsigned char *randStr, uint32_t len)
{
    if (randStr == NULL || len == 0) {
        return ERR_DM_CRYPTO_PARA_INVALID;
    }

    static mbedtls_entropy_context entropy;
    static mbedtls_ctr_drbg_context ctrDrbg;
    static std::atomic<bool> initFlag = false;
    int32_t ret;

    if (!initFlag) {
        std::lock_guard<std::mutex> lock(randomLock_);
        mbedtls_ctr_drbg_init(&ctrDrbg);
        mbedtls_entropy_init(&entropy);
        ret = mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy, NULL, 0);
        if (ret != 0) {
            LOGE("gen random seed error, ret=%{public}d", ret);
            return ERR_DM_CRYPTO_OPT_FAILED;
        }
        initFlag = true;
    }

    std::lock_guard<std::mutex> lock(randomLock_);
    ret = mbedtls_ctr_drbg_random(&ctrDrbg, randStr, len);
    if (ret != 0) {
        LOGE("gen random error, ret=%{public}d", ret);
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    return DM_OK;
}

int32_t CryptoMgr::DecryptMessage(const std::string &inputMsg, std::string &outputMsg)
{
    const uint32_t inputMsgBytesLen = inputMsg.length() / HEX_TO_UINT8;
    unsigned char inputMsgBytsTemp[inputMsgBytesLen];
    if (memset_s(inputMsgBytsTemp, inputMsgBytesLen, 0, inputMsgBytesLen) != DM_OK) {
        LOGE("memset_s failed.");
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    Crypto::ConvertHexStringToBytes(inputMsgBytsTemp, inputMsgBytesLen,
        reinterpret_cast<const char*>(inputMsg.c_str()), inputMsg.length());
    std::lock_guard<std::mutex> lock(sessionKeyMtx_);
    AesGcmCipherKey cipherKey = {.keyLen = sessionKey_.keyLen};
    if (memcpy_s(cipherKey.key, SESSION_KEY_LENGTH, sessionKey_.key, sessionKey_.keyLen) != EOK) {
        LOGE("set key fail");
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    if (inputMsgBytesLen < OVERHEAD_LEN) {
        LOGE("invalid para");
        return ERR_DM_CRYPTO_PARA_INVALID;
    }
    uint32_t outLen = inputMsgBytesLen - OVERHEAD_LEN + 1; /* for '\0' */
    unsigned char *outData = (unsigned char *)calloc(outLen, sizeof(unsigned char));
    if (outData == nullptr) {
        LOGE("calloc fail");
        return ERR_DM_CRYPTO_OPT_FAILED;
    }

    int32_t ret = DoDecryptData(&cipherKey, inputMsgBytsTemp,
        static_cast<uint32_t>(inputMsg.length() / HEX_TO_UINT8), outData, &outLen);
    if (ret != DM_OK) {
        LOGE("SoftBusDecryptDataWithSeq fail=%{public}d", ret);
        free(outData);
        outData = nullptr;
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    if (memset_s(&cipherKey, sizeof(AesGcmCipherKey), 0, sizeof(AesGcmCipherKey)) != DM_OK) {
        LOGE("memset_s failed.");
        free(outData);
        outData = nullptr;
        return ERR_DM_SECURITY_FUNC_FAILED;
    }
    outputMsg.clear();
    outputMsg.assign(reinterpret_cast<const char*>(outData), outLen);
    free(outData);
    outData = nullptr;
    return DM_OK;
}

int32_t CryptoMgr::MbedAesGcmDecrypt(const AesGcmCipherKey *cipherKey, const unsigned char *cipherText,
    uint32_t cipherTextSize, unsigned char *plain, uint32_t &plainLen)
{
    if ((cipherKey == NULL) || (cipherText == NULL) || (cipherTextSize <= OVERHEAD_LEN) || plain == NULL ||
        (plainLen < cipherTextSize - OVERHEAD_LEN)) {
        LOGE("Decrypt invalid para");
        return ERR_DM_CRYPTO_PARA_INVALID;
    }

    mbedtls_gcm_context aesContext;
    mbedtls_gcm_init(&aesContext);
    int32_t ret =
        mbedtls_gcm_setkey(&aesContext, MBEDTLS_CIPHER_ID_AES, cipherKey->key, cipherKey->keyLen * KEY_BITS_UNIT);
    if (ret != 0) {
        LOGE("Decrypt mbedtls_gcm_setkey fail.");
        mbedtls_gcm_free(&aesContext);
        return ERR_DM_CRYPTO_OPT_FAILED;
    }

    int32_t actualPlainLen = (int32_t)(cipherTextSize - OVERHEAD_LEN);
    ret = mbedtls_gcm_auth_decrypt(&aesContext, cipherTextSize - OVERHEAD_LEN, cipherKey->iv, GCM_IV_LEN, NULL, 0,
        cipherText + actualPlainLen + GCM_IV_LEN, TAG_LEN, cipherText + GCM_IV_LEN, plain);
    if (ret != 0) {
        LOGE("[TRANS] Decrypt mbedtls_gcm_auth_decrypt fail. ret=%{public}d", ret);
        mbedtls_gcm_free(&aesContext);
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    plainLen = (uint32_t)actualPlainLen;
    mbedtls_gcm_free(&aesContext);
    return DM_OK;
}

int32_t CryptoMgr::DoDecryptData(AesGcmCipherKey *cipherKey, const unsigned char *input, uint32_t inLen,
    unsigned char *decryptData, uint32_t *decryptLen)
{
    if (cipherKey == NULL || input == NULL || inLen < GCM_IV_LEN || decryptData == NULL || decryptLen == NULL ||
        inLen < OVERHEAD_LEN) {
        return ERR_DM_CRYPTO_PARA_INVALID;
    }

    if (memcpy_s(cipherKey->iv, sizeof(cipherKey->iv), input, GCM_IV_LEN) != EOK) {
        LOGE("copy iv failed.");
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    uint32_t outLen = inLen - OVERHEAD_LEN;
    int32_t result = MbedAesGcmDecrypt(cipherKey, input, inLen, decryptData, outLen);
    if (result != DM_OK) {
        return ERR_DM_CRYPTO_OPT_FAILED;
    }
    *decryptLen = outLen;
    return DM_OK;
}

int32_t CryptoMgr::SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen)
{
    if (keyLen > MAX_SESSION_KEY_LENGTH) {
        LOGE("SessionKey too long, len: %{public}d", keyLen);
        return ERR_DM_SAVE_SESSION_KEY_FAILED;
    }

    ClearSessionKey();
    {
        std::lock_guard<std::mutex> lock(sessionKeyMtx_);
        sessionKey_.key = (uint8_t*)calloc(keyLen, sizeof(uint8_t));
        sessionKey_.keyLen = keyLen;
    }
    return DM_OK;
}

std::vector<unsigned char> CryptoMgr::GetSessionKey()
{
    std::lock_guard<std::mutex> lock(sessionKeyMtx_);
    return std::vector<unsigned char>(sessionKey_.key, sessionKey_.key + sessionKey_.keyLen);
}

void CryptoMgr::ClearSessionKey()
{
    std::lock_guard<std::mutex> lock(sessionKeyMtx_);
    if (sessionKey_.key != nullptr) {
        (void)memset_s(sessionKey_.key, sessionKey_.keyLen, 0, sessionKey_.keyLen);
        free(sessionKey_.key);
        sessionKey_.key = nullptr;
        sessionKey_.keyLen = 0;
    }
}

int32_t CryptoMgr::ProcessSessionKey(const uint8_t *sessionKey, const uint32_t keyLen)
{
    if (sessionKey == nullptr || keyLen > MAX_SESSION_KEY_LENGTH) {
        LOGE("Invalid param, SessionKey len: %{public}d", keyLen);
        return ERR_DM_PROCESS_SESSION_KEY_FAILED;
    }
    ClearSessionKey();
    {
        std::lock_guard<std::mutex> lock(sessionKeyMtx_);
        sessionKey_.key = (uint8_t*)calloc(keyLen, sizeof(uint8_t));
        if (memcpy_s(sessionKey_.key, keyLen, sessionKey, keyLen) != DM_OK) {
            LOGE("memcpy_s failed.");
            return ERR_DM_PROCESS_SESSION_KEY_FAILED;
        }
        sessionKey_.keyLen = keyLen;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS