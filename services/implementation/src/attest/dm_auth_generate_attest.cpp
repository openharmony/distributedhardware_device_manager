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

#include "dm_auth_generate_attest.h"

#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {

constexpr uint64_t MIN_DCM_RANDOM = 0;
constexpr uint64_t MAX_DCM_RANDOM = 9999999999;

int32_t AuthGenerateAttest::GenerateCertificate(DmCertChain &dmCertChain)
{
    LOGI("GenerateCertificate Start");
    DcmIdType ids[] = { DCM_ID_TYPE_UDID };
    uint64_t randomNum = GenRandLongLong(MIN_DCM_RANDOM, MAX_DCM_RANDOM);
    LOGI("GenerateCertificate randomNum=%{public}lu", randomNum);
    DcmBlob challengeBlob = {sizeof(randomNum), (uint8_t *) &randomNum};
    DcmCertChain *dcmCertChain = new DcmCertChain();
    if (dcmCertChain == nullptr) {
        LOGE("new dcmCertChain fail!");
        return ERR_DM_MALLOC_FAILED;
    }
    InitCertChain(dcmCertChain);
    int32_t ret = DcmAttestIdsEx(ids, sizeof(ids)/sizeof(ids[0]), &challengeBlob, DCM_CERT_TYPE_ROOT_V2, dcmCertChain);
    if (ret != DCM_SUCCESS) {
        LOGE("DcmAttestIdsEx fail, ret=%{public}d", ret);
        FreeCertChain(dcmCertChain);
        return ret;
    }
    ret = ConvertDcmCertChainToDmCertChain(*dcmCertChain, dmCertChain);
    if (ret != DM_OK) {
        LOGE("covertToSoftbusCertChain fail. ret=%{public}d", ret);
        FreeCertChain(dcmCertChain);
        return ret;
    }
    FreeCertChain(dcmCertChain);
    return DM_OK;
}

int32_t AuthGenerateAttest::InitCertChain(DcmCertChain *certChain)
{
    LOGI("InitCertChain Start");
    certChain->certCount = DM_CERTS_COUNT;
    certChain->cert = new DcmBlob[certChain->certCount];
    if (certChain->cert == nullptr) {
        certChain->certCount = 0;
        LOGE("new dcmCertChain.cert fail!");
        return ERR_DM_MALLOC_FAILED;
    }
    for (uint32_t i = 0; i < certChain->certCount; ++i) {
        certChain->cert[i].data = new uint8_t[DM_CERTIFICATE_SIZE];
        if (certChain->cert[i].data == nullptr) {
            certChain->cert[i].size = 0;
            for (uint32_t j = 0; j < i; ++j) {
                delete[] certChain->cert[j].data;
                certChain->cert[j].data = nullptr;
                certChain->cert[j].size = 0;
            }
            delete[] certChain->cert;
            certChain->cert = nullptr;
            certChain->certCount = 0;
            LOGE("new dcmCertChain.cert.data fail!");
            return ERR_DM_MALLOC_FAILED;
        }
        certChain->cert[i].size = DM_CERTIFICATE_SIZE;
    }
    return DM_OK;
}

void AuthGenerateAttest::FreeCertChain(DcmCertChain *chain)
{
    if (chain == nullptr) {
        return;
    }
    for (uint32_t i = 0; i < chain->certCount; ++i) {
        delete[] chain->cert[i].data;
        chain->cert[i].data = nullptr;
        chain->cert[i].size = 0;
    }
    delete[] chain->cert;
    chain->cert = nullptr;
    chain->certCount = 0;
    delete chain;
}

int32_t ValidateInput(DcmCertChain &dcmCertChain)
{
    if (dcmCertChain.certCount > 0 && dcmCertChain.cert == nullptr) {
        LOGE("Invalid cert chain: certCount>0 but cert array is null!");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}

int32_t CopyCertificates(DcmCertChain &dcmCertChain, DmBlob *newCertArray, uint32_t &allocatedCerts)
{
    for (uint32_t i = 0; i < dcmCertChain.certCount; ++i) {
        const auto &src = dcmCertChain.cert[i];
        auto &dest = newCertArray[i];
        dest.size = src.size;
        dest.data = nullptr;
        if (src.size == 0 || src.data == nullptr) continue;
        dest.data = new uint8_t[src.size];
        if (dest.data == nullptr) {
            allocatedCerts = i;
            return ERR_DM_MALLOC_FAILED;
        }

        if (memcpy_s(dest.data, src.size, src.data, src.size) != DM_OK) {
            delete[] dest.data;
            dest.data = nullptr;
            allocatedCerts = i;
            return ERR_DM_FAILED;
        }
        allocatedCerts = i + 1;
    }
    return DM_OK;
}

int32_t AuthGenerateAttest::ConvertDcmCertChainToDmCertChain(DcmCertChain &dcmCertChain, DmCertChain &dmCertChain)
{
    LOGI("ConvertDcmCertChainToDmCertChain start!");
    int32_t ret = ValidateInput(dcmCertChain);
    if (ret != DM_OK) {
        return ret;
    }
    if (dcmCertChain.certCount == 0) {
        dmCertChain.cert = nullptr;
        dmCertChain.certCount = 0;
        return DM_OK;
    }
    DmBlob *newCertArray = new DmBlob[dcmCertChain.certCount];
    if (newCertArray == nullptr) {
        LOGE("Failed to allocate cert array!");
        return ERR_DM_MALLOC_FAILED;
    }
    uint32_t allocatedCerts = 0;
    ret = CopyCertificates(dcmCertChain, newCertArray, allocatedCerts);
    if (ret != DM_OK) {
        for (uint32_t j = 0; j < allocatedCerts; ++j) {
            delete[] newCertArray[j].data;
        }
        delete[] newCertArray;
        return ret;
    }
    dmCertChain.cert = newCertArray;
    dmCertChain.certCount = dcmCertChain.certCount;
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS