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

#include "os_account_manager_mock.h"

using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IOsAccountManager> IOsAccountManager::accountManager_ = nullptr;

std::shared_ptr<IOsAccountManager> IOsAccountManager::GetOrCreateOsAccountManager()
{
    if (!accountManager_) {
        accountManager_ = std::make_shared<OsAccountManagerMock>();
    }
    return accountManager_;
}

void IOsAccountManager::ReleaseAccountManager()
{
    accountManager_.reset();
    accountManager_ = nullptr;
}
} // namespace DistributedHardware

namespace AccountSA {
ErrCode OsAccountManager::GetForegroundOsAccountLocalId(int32_t &localId)
{
    return IOsAccountManager::GetOrCreateOsAccountManager()->GetForegroundOsAccountLocalId(localId);
}

ErrCode OsAccountManager::GetForegroundOsAccountLocalId(const uint64_t displayId, int32_t &localId)
{
    return IOsAccountManager::GetOrCreateOsAccountManager()->GetForegroundOsAccountLocalId(displayId, localId);
}
} // namespace AccountSA
} // namespace OHOS
