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

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(AppManager);

const std::string AppManager::GetAppId()
{
    std::string appId = "";
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (AccessTokenKit::GetTokenTypeFlag(tokenId) != TOKEN_HAP) {
        LOGI("The caller is not token_hap.");
        return appId;
    }
    sptr<AppExecFwk::IBundleMgr> bundleManager = nullptr;
    if (!GetBundleManagerProxy(bundleManager)) {
        LOGE("get bundleManager failed.");
        return appId;
    }
    if (bundleManager == nullptr) {
        LOGE("bundleManager is nullptr.");
        return appId;
    }
    int userId;
    ErrCode result = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (result != ERR_OK) {
        LOGE("GetAppIdByCallingUid QueryActiveOsAccountIds failed.");
        return appId;
    }
    std::string BundleName;
    bundleManager->GetNameForUid(IPCSkeleton::GetCallingUid(), BundleName);
    AppExecFwk::BundleInfo bundleInfo;
    int ret = bundleManager->GetBundleInfoV9(
        BundleName, static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO),
        bundleInfo, userId);
    if (ret != 0) {
        LOGE(" GetBundleInfoV9 failed %{public}d.", ret);
        return appId;
    }
    appId = bundleInfo.appId;
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

bool AppManager::GetBundleManagerProxy(sptr<AppExecFwk::IBundleMgr> &bundleManager)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        LOGE("GetBundleManagerProxy Failed to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        LOGE("GetBundleManagerProxy Failed to get bundle manager service.");
        return false;
    }
    bundleManager = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleManager == nullptr) {
        LOGE("bundleManager is nullptr");
        return false;
    }
    return true;
}

bool AppManager::IsSystemSA()
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("IsSystemSA GetCallingTokenID error.");
        return false;
    }
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
