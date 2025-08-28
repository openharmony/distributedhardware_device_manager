/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_LITE_APP_MANAGER_H
#define OHOS_DM_LITE_APP_MANAGER_H

#include <string>
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class AppManager {
    DM_DECLARE_SINGLE_INSTANCE(AppManager);

public:
    const std::string GetAppId();
    void RegisterCallerAppId(const std::string &pkgName, const int32_t userId);
    void UnRegisterCallerAppId(const std::string &pkgName, const int32_t userId);
    int32_t GetAppIdByPkgName(const std::string &pkgName, std::string &appId, const int32_t userId);
    bool IsSystemSA();
    bool IsSystemApp();
    int32_t GetCallerName(bool isSystemSA, std::string &callerName);
    int32_t GetNativeTokenIdByName(std::string &processName, int64_t &tokenId);
    int32_t GetHapTokenIdByName(int32_t userId, std::string &bundleName, int32_t instIndex, int64_t &tokenId);
    int32_t GetBundleNameForSelf(std::string &bundleName);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_LITE_APP_MANAGER_H
