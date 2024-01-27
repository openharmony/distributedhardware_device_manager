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

#ifndef DM_ADAPTER_CRYPTO_H
#define DM_ADAPTER_CRYPTO_H

#include <stdint.h>
#include <string>

namespace OHOS {
namespace DistributedHardware {
#ifndef AES_GCM_H
#define AES_GCM_H

#define HUKS_AES_GCM_KEY_LEN 256
#define GCM_IV_LEN 12
#define AAD_LEN 16

#define TAG_LEN 16
#define OVERHEAD_LEN (GCM_IV_LEN + TAG_LEN)

#define GCM_KEY_BITS_LEN_128 128
#define GCM_KEY_BITS_LEN_256 256
#define KEY_BITS_UNIT 8

#define BLE_BROADCAST_IV_LEN 16

typedef struct {
    unsigned char *aad;
    int32_t aadLen;
    const unsigned char *input;
    int32_t inputLen;
    unsigned char **output;
    int32_t *outputLen;
} GcmInputParams;

typedef struct {
    int32_t keyLen;
    unsigned char key[16];
    unsigned char iv[GCM_IV_LEN];
} AesGcmCipherKey;

class DmAdapterCrypto {
public:
    static void MbedAesGcmEncrypt(AesGcmCipherKey *cipherKey, const unsigned char *plainText, int32_t plainTextSize,
        unsigned char *cipherText, int32_t cipherTextLen);

    static void MbedAesGcmDecrypt(AesGcmCipherKey *cipherKey, const unsigned char *cipherText,
        int32_t cipherTextSize, unsigned char *plain, int32_t plainLen);
#endif // !AES_GCM_H
};
}; // namespace DistributeHardware
}; // namespace OHOS

#endif // !DM_ADAPTER_CRYPTO_H
