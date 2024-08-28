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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(AppManager);

const std::string AppManager::GetAppId()
{
    std::string appId = "";
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (AccessTokenKit::GetTokenTypeFlag(tokenId) != TOKEN_HAP) {
        LOGE("The caller is not token_hap.");
        return appId;
    }
    HapTokenInfo tokenInfo;
    int32_t result = AccessTokenKit::GetHapTokenInfo(tokenId, tokenInfo);
    if (result != RET_SUCCESS) {
        LOGE("GetHapTokenInfo failed ret is %{public}d.", result);
        return appId;
    }
    appId = tokenInfo.appID;
    return appId;
}

void AppManager::RegisterCallerAppId(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::string appId = GetAppId();
    if (appId.empty()) {
        LOGE("PkgName %{public}s get appid failed.", pkgName.c_str());
        return;
    }
    LOGI("PkgName %{public}s, appId %{public}s.", pkgName.c_str(), GetAnonyString(appId).c_str());
    std::lock_guard<std::mutex> lock(appIdMapLock_);
    appIdMap_[pkgName] = appId;
}

void AppManager::UnRegisterCallerAppId(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("PkgName %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> lock(appIdMapLock_);
    if (appIdMap_.find(pkgName) == appIdMap_.end()) {
        LOGE("AppIdMap not find pkgName.");
        return;
    }
    appIdMap_.erase(pkgName);
}

int32_t AppManager::GetAppIdByPkgName(const std::string &pkgName, std::string &appId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("PkgName %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> lock(appIdMapLock_);
    if (appIdMap_.find(pkgName) == appIdMap_.end()) {
        LOGE("AppIdMap not find pkgName.");
        return ERR_DM_FAILED;
    }
    appId = appIdMap_[pkgName];
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
