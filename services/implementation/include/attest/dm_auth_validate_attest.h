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

#ifndef OHOS_DM_AUTH_VALIDATE_ATTEST_H
#define OHOS_DM_AUTH_VALIDATE_ATTEST_H

#include "dm_auth_attest_common.h"

#include "hks_api.h"
#include "hks_param.h"
#include "hks_type.h"

namespace OHOS {
namespace DistributedHardware {
class AuthValidateAttest {
public:
    static AuthValidateAttest &GetInstance(void)
    {
        static AuthValidateAttest instance;
        return instance;
    }

    int32_t VerifyCertificate(const DmCertChain &dmCertChain, const char *deviceUdid);
    int32_t ConvertDmCertChainToHksCertChain(const DmCertChain &dmCertChain, HksCertChain &hksCertChain);
    int32_t FillHksParamSet(struct HksParamSet **paramSet, struct HksParam *param, int32_t paramNums);
    void FreeHksCertChain(HksCertChain &chain);
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_VALIDATE_ATTEST_H