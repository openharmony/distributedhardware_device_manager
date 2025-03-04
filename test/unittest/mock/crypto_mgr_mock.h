/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_CRYTO_MGR_MOCK_H
#define OHOS_CRYTO_MGR_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "crypto_mgr.h"

namespace OHOS {
namespace DistributedHardware {
class DmCryptoMgr {
public:
    virtual ~DmCryptoMgr() = default;
public:
    virtual int32_t EncryptMessage(const std::string &inputMsg, std::string &outputMsg) = 0;
    virtual int32_t DecryptMessage(const std::string &inputMsg, std::string &outputMsg) = 0;
    virtual int32_t SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen) = 0;
public:
    static inline std::shared_ptr<DmCryptoMgr> dmCryptoMgr = nullptr;
};

class CryptoMgrMock : public DmCryptoMgr {
public:
    MOCK_METHOD(int32_t, EncryptMessage, (const std::string &, std::string &));
    MOCK_METHOD(int32_t, DecryptMessage, (const std::string &, std::string &));
    MOCK_METHOD(int32_t, SaveSessionKey, (const uint8_t *, const uint32_t));
};
}
}
#endif
