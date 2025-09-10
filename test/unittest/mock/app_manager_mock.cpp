/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "app_manager_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(AppManager);
const std::string AppManager::GetAppId()
{
    return DmAppManager::dmAppManager->GetAppId();
}

bool AppManager::IsSystemSA()
{
    return DmAppManager::dmAppManager->IsSystemSA();
}

int32_t AppManager::GetNativeTokenIdByName(std::string &processName, int64_t &tokenId)
{
    return DmAppManager::dmAppManager->GetNativeTokenIdByName(processName, tokenId);
}

int32_t AppManager::GetHapTokenIdByName(int32_t userId, std::string &bundleName, int32_t instIndex, int64_t &tokenId)
{
    return DmAppManager::dmAppManager->GetHapTokenIdByName(userId, bundleName, instIndex, tokenId);
}

int32_t AppManager::GetAppIdByPkgName(const std::string &pkgName, std::string &appId, const int32_t userId)
{
    return DmAppManager::dmAppManager->GetAppIdByPkgName(pkgName, appId, userId);
}

int32_t AppManager::GetBundleNameForSelf(std::string &bundleName)
{
    return DmAppManager::dmAppManager->GetBundleNameForSelf(bundleName);
}
} // namespace DistributedHardware
} // namespace OHOS