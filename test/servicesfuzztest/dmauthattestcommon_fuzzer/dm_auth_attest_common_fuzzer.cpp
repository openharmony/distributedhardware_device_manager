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

#include "dm_auth_attest_common_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "dm_auth_attest_common.h"

namespace OHOS {
namespace DistributedHardware {

void DmAuthAttestCommonFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DmCertChain* chain = new DmCertChain;
    chain->cert = new DmBlob[1];
    chain->certCount = 1;
    int32_t maxLen = 65535;
    chain->cert[0].size = size;
    if (size > 0 && size <= maxLen) {
        chain->cert[0].data = new uint8_t[size];
        *chain->cert[0].data = fdp.ConsumeIntegral<uint8_t>();
    }
    std::string str_value = fdp.ConsumeRandomLengthString();
    JsonItemObject jsonItem;
    jsonItem.PushBack(str_value);
    AuthAttestCommon::GetInstance().SerializeDmCertChain(nullptr);
    AuthAttestCommon::GetInstance().SerializeDmCertChain(chain);
    AuthAttestCommon::GetInstance().FreeDmCertChain(*chain);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmAuthAttestCommonFuzzTest(data, size);
    return 0;
}
