/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_APP_MANAGER_3RD_H
#define OHOS_DM_APP_MANAGER_3RD_H

#include <map>
#include <mutex>
#include <string>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_single_instance_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class AppManager3rd {
    DM_DECLARE_SINGLE_INSTANCE_3RD(AppManager3rd);

public:
    int32_t GetAppIdByPkgName(const std::string &pkgName, std::string &appId, const int32_t userId);
    bool IsSystemSA();
    bool IsSystemApp();
    int32_t GetCallerName(bool isSystemSA, std::string &callerName);
    int32_t GetNativeTokenIdByName(const std::string &processName, uint32_t &tokenId);
    int32_t GetHapTokenIdByName(int32_t userId, const std::string &bundleName,
        int32_t instIndex, uint32_t &tokenId);
    int32_t GetCallerProcessName(std::string &processName);
    int32_t GetBundleNameForSelf(std::string &bundleName);
    int32_t GetBundleNameByTokenId(int64_t tokenId, std::string &bundleName);
private:
    bool GetBundleManagerProxy(sptr<AppExecFwk::IBundleMgr> &bundleManager);
    std::mutex appIdMapLock_;
    std::map<std::string, std::string> appIdMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_APP_MANAGER_3RD_H
