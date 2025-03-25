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

#ifndef OHOS_DM_APP_MANAGER_H
#define OHOS_DM_APP_MANAGER_H

#include <map>
#include <mutex>
#include <string>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "event_handler.h"

#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class AppManager {
    DM_DECLARE_SINGLE_INSTANCE(AppManager);

public:
    __attribute__ ((visibility ("default")))const std::string GetAppId();
    __attribute__ ((visibility ("default")))void RegisterCallerAppId(const std::string &pkgName);
    __attribute__ ((visibility ("default")))void UnRegisterCallerAppId(const std::string &pkgName);
    __attribute__ ((visibility ("default")))int32_t GetAppIdByPkgName(const std::string &pkgName, std::string &appId);
    __attribute__ ((visibility ("default")))bool IsSystemSA();
    __attribute__ ((visibility ("default")))bool IsSystemApp();
    __attribute__ ((visibility ("default")))int32_t GetCallerName(bool isSystemSA, std::string &callerName);
    __attribute__ ((visibility ("default")))int32_t GetNativeTokenIdByName(std::string &processName, int64_t &tokenId);
    __attribute__ ((visibility ("default")))int32_t GetHapTokenIdByName(int32_t userId, std::string &bundleName,
        int32_t instIndex, int64_t &tokenId);
    __attribute__ ((visibility ("default")))int32_t GetCallerProcessName(std::string &processName);
private:
    bool GetBundleManagerProxy(sptr<AppExecFwk::IBundleMgr> &bundleManager);
    std::mutex appIdMapLock_;
    std::map<std::string, std::string> appIdMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_APP_MANAGER_H
