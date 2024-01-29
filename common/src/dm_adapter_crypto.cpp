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

#include "dm_adapter_crypto.h"

#include <securec.h>

#include "mbedtls/cipher.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/gcm.h"
#include "mbedtls/md.h"
#include "mbedtls/platform.h"
#include "md.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
#ifndef MBEDTLS_CTR_DRBG_C
#define MBEDTLS_CTR_DRBG_C
#endif

#ifndef MBEDTLS_MD_C
#define MBEDTLS_MD_C
#endif

#ifndef MBEDTLS_SHA256_C
#define MBEDTLS_SHA256_C
#endif

#ifndef MBEDTLS_ENTROPY_C
#define MBEDTLS_ENTROPY_C
#endif

#ifndef MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_CIPHER_MODE_CTR
#endif

#ifndef MBEDTLS_AES_C
#define MBEDTLS_AES_C
#endif

#ifndef MBEDTLS_CIPHER_C
#define MBEDTLS_CIPHER_C
#endif

#define EVP_AES_128_KEYLEN 16
#define EVP_AES_256_KEYLEN 32

void DmAdapterCrypto::MbedAesGcmEncrypt(AesGcmCipherKey *cipherKey, const unsigned char *plainText,
    int32_t plainTextSize, unsigned char *cipherText, int32_t cipherTextLen)
{
    mbedtls_cipher_context_t ctx;
    const mbedtls_cipher_info_t *info;
    mbedtls_cipher_init(&ctx);
    info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_GCM);
    mbedtls_cipher_setup(&ctx, info);
    mbedtls_cipher_setkey(&ctx, cipherKey->key, cipherKey->keyLen * KEY_BITS_UNIT, MBEDTLS_ENCRYPT);
    size_t olen = 16;
    mbedtls_cipher_auth_encrypt_ext(&ctx, cipherKey->iv, GCM_IV_LEN, NULL, 0, plainText, plainTextSize, cipherText,
        cipherTextLen, &olen, TAG_LEN);
    mbedtls_cipher_free(&ctx);
}

void DmAdapterCrypto::MbedAesGcmDecrypt(AesGcmCipherKey *cipherKey, const unsigned char *cipherText,
    int32_t cipherTextSize, unsigned char *plain, int32_t plainLen)
{
    mbedtls_cipher_context_t ctx;
    const mbedtls_cipher_info_t* info;
    mbedtls_cipher_init(&ctx);
    info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_GCM);
    mbedtls_cipher_setup(&ctx, info);
    mbedtls_cipher_setkey(&ctx, cipherKey->key, cipherKey->keyLen * KEY_BITS_UNIT, MBEDTLS_DECRYPT);
    size_t olen = 16;
    int ret = mbedtls_cipher_auth_decrypt_ext(&ctx, cipherKey->iv, GCM_IV_LEN, NULL, 0, cipherText, cipherTextSize,
        plain, plainLen, &olen, TAG_LEN);
    if (ret != 0) {
        LOGI("MbedAesGcmDEcrypt result :%d", ret);
    }
    mbedtls_cipher_free(&ctx);
}
}; // namespace DistributeHardware
}; // namespace OHOS