/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "app_manager.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(AppManager);

const std::string AppManager::GetAppId()
{
    return "";
}

void AppManager::RegisterCallerAppId(const std::string &pkgName)
{
    (void)pkgName;
}

void AppManager::UnRegisterCallerAppId(const std::string &pkgName)
{
    (void)pkgName;
}

int32_t AppManager::GetAppIdByPkgName(const std::string &pkgName, std::string &appId)
{
    (void)pkgName;
    (void)appId;
    return DM_OK;
}

bool AppManager::IsSystemSA()
{
    return true;
}

bool AppManager::IsSystemApp()
{
    return true;
}

int32_t AppManager::GetCallerName(bool isSystemSA, std::string &callerName)
{
    (void)isSystemSA;
    (void)callerName;
    return DM_OK;
}
int32_t AppManager::GetNativeTokenIdByName(std::string &processName, int64_t &tokenId)
{
    (void)processName;
    (void)tokenId;
    return DM_OK;
}

int32_t AppManager::GetHapTokenIdByName(int32_t userId, std::string &bundleName, int32_t instIndex, int64_t &tokenId)
{
    (void)userId;
    (void)bundleName;
    (void)instIndex;
    (void)tokenId;
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
