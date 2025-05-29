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

#ifndef OHOS_DM_AUTH_ATTEST_COMMON_H
#define OHOS_DM_AUTH_ATTEST_COMMON_H

#define DM_CERTS_COUNT 4
#define UDID_BUF_LEN 65
#define DM_CERTIFICATE_SIZE 8192

#include <stdint.h>
#include <string>

#include "dm_anonymous.h"
#include "dm_auth_manager_base.h"
#include "dm_cert.h"
#include "dm_crypto.h"
#include "dm_random.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
class AuthAttestCommon {
public:
    static AuthAttestCommon &GetInstance(void)
    {
        static AuthAttestCommon instance;
        return instance;
    }

    std::string SerializeDmCertChain(const DmCertChain *chain);
    bool DeserializeDmCertChain(const std::string &data, DmCertChain *outChain);
    void FreeDmCertChain(DmCertChain &chain);
private:
    AuthAttestCommon();
    ~AuthAttestCommon();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif  // OHOS_DM_AUTH_ATTEST_COMMON_H