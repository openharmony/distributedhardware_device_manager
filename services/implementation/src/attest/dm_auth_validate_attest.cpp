/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_auth_validate_attest.h"

#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {

int32_t ProcessValidationResult(const char *deviceIdHash, char *udidStr, uint64_t randNum, HksParamSet *outputParam)
{
    if (deviceIdHash == nullptr || udidStr == nullptr || outputParam == nullptr) {
        LOGE("input param is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    uint32_t cnt = 0;
    HksBlob *blob = &outputParam->params[cnt].blob;
    if (memcpy_s(&randNum, sizeof(uint64_t), blob->data, blob->size) != EOK) {
        LOGE("memcpy randNum failed");
        return ERR_DM_GET_PARAM_FAILED;
    }
    blob = &outputParam->params[++cnt].blob;
    if (memcpy_s(udidStr, UDID_BUF_LEN, blob->data, blob->size) != EOK) {
        LOGE("memcpy udidStr failed");
        return ERR_DM_GET_PARAM_FAILED;
    }
    std::string certDeviceIdHash = Crypto::GetUdidHash(std::string(udidStr));
    if (strcmp(deviceIdHash, certDeviceIdHash.c_str()) != 0) {
        LOGE("verifyCertificate fail");
        return ERR_DM_DESERIAL_CERT_FAILED;
    }
    return DM_OK;
}

int32_t AuthValidateAttest::VerifyCertificate(DmCertChain &dmCertChain, const char *deviceIdHash)
{
    if (deviceIdHash == nullptr) {
        LOGE("deviceIdHash is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("VerifyCertificate start!");
    char udidStr[UDID_BUF_LEN] = {0};
    uint64_t randNum = 0;
    HksCertChain hksCertChain;
    int32_t ret = ConvertDmCertChainToHksCertChain(dmCertChain, hksCertChain);
    if (ret != DM_OK) {
        LOGE("ConvertDmCertChainToHksCertChain fail, ret=%{public}d", ret);
        return ret;
    }
    HksParamSet *outputParam = NULL;
    HksParam outputData[] = {
        {.tag = HKS_TAG_ATTESTATION_CHALLENGE, .blob = {sizeof(uint64_t), (uint8_t *) &randNum}},
        {.tag = HKS_TAG_ATTESTATION_ID_UDID, .blob = {UDID_BUF_LEN, (uint8_t *)udidStr}},
    };
    ret = FillHksParamSet(&outputParam, outputData, sizeof(outputData) / sizeof(outputData[0]));
    if (ret != DM_OK) {
        LOGE("FillHksParamSet failed, ret=%{public}d", ret);
        FreeHksCertChain(hksCertChain);
        return ERR_DM_FAILED;
    }
    ret = HksValidateCertChain(&hksCertChain, outputParam);
    if (ret != HKS_SUCCESS) {
        LOGE("HksValidateCertChain fail, ret=%{public}d", ret);
        FreeHksCertChain(hksCertChain);
        return ret;
    }
    ret = ProcessValidationResult(deviceIdHash, udidStr, randNum, outputParam);
    if (ret != DM_OK) {
        LOGE("ProcessValidationResult fail, ret=%{public}d", ret);
        FreeHksCertChain(hksCertChain);
        return ret;
    }
    FreeHksCertChain(hksCertChain);
    return DM_OK;
}

int32_t AuthValidateAttest::FillHksParamSet(HksParamSet **paramSet, HksParam *param, int32_t paramNums)
{
    if (param == nullptr) {
        LOGE("param is null");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksInitParamSet failed, hks ret = %{public}d", ret);
        return ERR_DM_FAILED;
    }
    ret = HksAddParams(*paramSet, param, paramNums);
    if (ret != HKS_SUCCESS) {
        LOGE("HksAddParams failed, hks ret = %{public}d", ret);
        HksFreeParamSet(paramSet);
        return ERR_DM_FAILED;
    }
    ret = HksBuildParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksBuildParamSet failed, hks ret = %{public}d", ret);
        HksFreeParamSet(paramSet);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void AuthValidateAttest::FreeHksCertChain(HksCertChain &chain)
{
    if (chain.certs != nullptr) {
        for (uint32_t i = 0; i < chain.certsCount; ++i) {
            chain.certs[i].size = 0;
            delete[] chain.certs[i].data;
            chain.certs[i].data = nullptr;
        }
        delete[] chain.certs;
        chain.certs = nullptr;
    }
    chain.certsCount = 0;
}

int32_t AllocateHksBlobArray(uint32_t count, HksBlob **outArray)
{
    HksBlob *arr = new HksBlob[count]{0};
    if (arr == nullptr) {
        LOGE("Alloc failed for certs");
        return ERR_DM_MALLOC_FAILED;
    }
    *outArray = arr;
    return DM_OK;
}

int32_t CopySingleCert(const DmBlob &src, HksBlob &dest)
{
    if (src.data == nullptr || src.size == 0) {
        LOGE("Invalid src cert");
        return ERR_DM_FAILED;
    }
    dest.data = new uint8_t[src.size]{0};
    if (dest.data == nullptr) {
        LOGE("Alloc failed for size");
        return ERR_DM_MALLOC_FAILED;
    }
    dest.size = src.size;
    if (memcpy_s(dest.data, src.size, src.data, src.size) != 0) {
        LOGE("memcpy_s failed size");
        delete[] dest.data;
        dest.data = nullptr;
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t AuthValidateAttest::ConvertDmCertChainToHksCertChain(DmCertChain &dmCertChain, HksCertChain &hksCertChain)
{
    if (dmCertChain.certCount == 0 || dmCertChain.cert == nullptr) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    HksBlob *newCerts = nullptr;
    int32_t ret = AllocateHksBlobArray(dmCertChain.certCount, &newCerts);
    if (ret != DM_OK) {
        LOGE("AllocateHksBlobArray fail, ret = %{public}d", ret);
        return ret;
    }
    for (uint32_t i = 0; i < dmCertChain.certCount; ++i) {
        if ((ret = CopySingleCert(dmCertChain.cert[i], newCerts[i])) != DM_OK) {
            FreeHksCertChain(hksCertChain);
            delete[] newCerts;
            return ret;
        }
    }
    hksCertChain.certs = newCerts;
    hksCertChain.certsCount = dmCertChain.certCount;
    return DM_OK;
}
}  // namespace DistributedHardware
}  // namespace OHOS