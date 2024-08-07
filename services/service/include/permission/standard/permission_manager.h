/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_PERMISSION_STANDARD_PERMISSION_MANAGER_H
#define OHOS_DM_PERMISSION_STANDARD_PERMISSION_MANAGER_H

#include <cstdint>
#include <string>
#include <unordered_set>
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class PermissionManager {
    DM_DECLARE_SINGLE_INSTANCE(PermissionManager);

public:
    bool CheckPermission(void);
    bool CheckNewPermission(void);
    bool CheckMonitorPermission(void);
    int32_t GetCallerProcessName(std::string &processName);
    bool CheckProcessNameValidOnAuthCode(const std::string &processName);
    bool CheckProcessNameValidOnPinHolder(const std::string &processName);
    bool CheckSystemSA(const std::string &pkgName);
    std::unordered_set<std::string> GetSystemSA();
};
#define AUTH_CODE_WHITE_LIST_NUM (3)
constexpr const static char g_authCodeWhiteList[AUTH_CODE_WHITE_LIST_NUM][PKG_NAME_SIZE_MAX] = {
    "com.huawei.msdp.hmringgenerator",
    "com.huawei.msdp.hmringdiscriminator",
    "CollaborationFwk",
};

#define PIN_HOLDER_WHITE_LIST_NUM (1)
constexpr const static char g_pinHolderWhiteList[PIN_HOLDER_WHITE_LIST_NUM][PKG_NAME_SIZE_MAX] = {
    "CollaborationFwk",
};

#define SYSTEM_SA_WHITE_LIST_NUM (4)
constexpr const static char systemSaWhiteList[SYSTEM_SA_WHITE_LIST_NUM][PKG_NAME_SIZE_MAX] = {
    "Samgr_Networking",
    "ohos.distributeddata.service",
    "ohos.dslm",
    "ohos.deviceprofile",
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PERMISSION_STANDARD_PERMISSION_MANAGER_H
