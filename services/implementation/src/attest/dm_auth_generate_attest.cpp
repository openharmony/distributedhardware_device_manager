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
    DcmBlob challengeBlob = {sizeof(randomNum), (uint8_t *)& randomNum};
    DcmCertChain* dcmCertChain = new DcmCertChain();
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
    certChain->certCount = DM_CERTS_COUNT;
    certChain->cert = new DcmBlob[certChain->certCount];
    for (uint32_t i = 0; i < certChain->certCount; ++i) {
        certChain->cert[i].size = DM_CERTIFICATE_SIZE;
        certChain->cert[i].data = new uint8_t[DM_CERTIFICATE_SIZE];
    }
    return DM_OK;
}

void AuthGenerateAttest::FreeCertChain(DcmCertChain* chain)
{
    if (!chain) {
        return;
    }
    for (uint32_t i = 0; i < chain->certCount; ++i) {
        delete[] chain->cert[i].data;
        chain->cert[i].data = nullptr;
    }
    delete[] chain->cert;
    chain->cert = nullptr;
    delete chain;
}

int32_t AuthGenerateAttest::ConvertDcmCertChainToDmCertChain(DcmCertChain &dcmCertChain, DmCertChain &dmCertChain)
{
    LOGI("ConvertDcmCertChainToDmCertChain start!");
    dmCertChain.certCount = dcmCertChain.certCount;
    if (dcmCertChain.certCount > 0) {
        dmCertChain.cert = new DmBlob[dcmCertChain.certCount];
        for (uint32_t i = 0; i < dcmCertChain.certCount; ++i) {
            dmCertChain.cert[i].size = dcmCertChain.cert[i].size;
            if (dcmCertChain.cert[i].size > 0 && dcmCertChain.cert[i].data != nullptr) {
                dmCertChain.cert[i].data = new uint8_t[dcmCertChain.cert[i].size];
                memcpy_s(dmCertChain.cert[i].data, dmCertChain.cert[i].size,
                    dcmCertChain.cert[i].data, dcmCertChain.cert[i].size);
            } else {
                dmCertChain.cert[i].data = nullptr;
            }
        }
    } else {
        dmCertChain.cert = nullptr;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS