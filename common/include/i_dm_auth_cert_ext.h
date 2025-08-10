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

#ifndef OHOS_I_DM_AUTH_CERT_EXT_H
#define OHOS_I_DM_AUTH_CERT_EXT_H

#include "dm_cert.h"

namespace OHOS {
namespace DistributedHardware {
class IDMAuthCertExt {
public:
    virtual ~IDMAuthCertExt() = default;
    virtual int32_t GenerateCertificate(DmCertChain &dmCertChain) = 0;
    virtual int32_t VerifyCertificate(const DmCertChain &dmCertChain, const char *deviceIdHash) = 0;
    virtual int32_t GenerateCertificateV2(DmCertChain &dmCertChain, const uint64_t random) = 0;
    virtual int32_t VerifyCertificateV2(const DmCertChain &dmCertChain, const char *deviceIdHash,
        const uint64_t random) = 0;
};

using CreateDMAuthCertFuncPtr = IDMAuthCertExt *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_AUTH_CERT_EXT_H