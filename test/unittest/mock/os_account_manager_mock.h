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
#ifndef OHOS_DM_OS_ACCOUNT_MANAGER_MOCK_H
#define OHOS_DM_OS_ACCOUNT_MANAGER_MOCK_H

#include <gmock/gmock.h>
#include "os_account_manager.h"

namespace OHOS {
namespace DistributedHardware {
class IOsAccountManager {
public:
    IOsAccountManager() = default;
    virtual ~IOsAccountManager() = default;

    virtual ErrCode GetForegroundOsAccountLocalId(int32_t &localId) = 0;
    virtual ErrCode GetForegroundOsAccountLocalId(const uint64_t displayId, int32_t &localId) = 0;
    static std::shared_ptr<IOsAccountManager> GetOrCreateOsAccountManager();
    static void ReleaseAccountManager();
private:
    static std::shared_ptr<IOsAccountManager> accountManager_;
};

class OsAccountManagerMock : public IOsAccountManager {
public:
    MOCK_METHOD(ErrCode, GetForegroundOsAccountLocalId, (int32_t &));
    MOCK_METHOD(ErrCode, GetForegroundOsAccountLocalId, (const uint64_t, int32_t &));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif  // OHOS_DM_OS_ACCOUNT_MANAGER_MOCK_H
