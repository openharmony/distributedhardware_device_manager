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

#ifndef OHOS_DM_PERMISSION_STANDARD_PERMISSION_MANAGER_3RD_H
#define OHOS_DM_PERMISSION_STANDARD_PERMISSION_MANAGER_3RD_H

#include <cstdint>
#include <string>
#include "dm_single_instance_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class PermissionManager3rd {
    DM_DECLARE_SINGLE_INSTANCE_3RD(PermissionManager3rd);
public:
    bool CheckSystemSA(const std::string &pkgName);
    int32_t GetCallerProcessName(std::string &processName);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PERMISSION_STANDARD_PERMISSION_MANAGER_3RD_H

