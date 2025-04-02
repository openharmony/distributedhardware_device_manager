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

#include "permission_manager.h"

#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(PermissionManager);

bool PermissionManager::CheckPermission(void)
{
    return true;
}

bool PermissionManager::CheckNewPermission(void)
{
    return true;
}

bool PermissionManager::CheckMonitorPermission(void)
{
    return true;
}

int32_t PermissionManager::GetCallerProcessName(std::string &processName)
{
    (void)processName;
    return DM_OK;
}

bool PermissionManager::CheckProcessNameValidOnAuthCode(const std::string &processName)
{
    (void)processName;
    return true;
}

bool PermissionManager::CheckProcessNameValidOnPinHolder(const std::string &processName)
{
    (void)processName;
    return true;
}

bool PermissionManager::CheckWhiteListSystemSA(const std::string &pkgName)
{
    (void)pkgName;
    return true;
}

std::unordered_set<std::string> PermissionManager::GetWhiteListSystemSA()
{
    return std::unordered_set<std::string>{};
}

bool PermissionManager::CheckSystemSA(const std::string &pkgName)
{
    (void)pkgName;
    return true;
}

bool PermissionManager::CheckProcessNameValidModifyLocalDeviceName(const std::string &processName)
{
    (void)processName;
    return true;
}

bool PermissionManager::CheckProcessNameValidOnSetDnPolicy(const std::string &processName)
{
    (void)processName;
    return true;
}

bool PermissionManager::CheckProcessNameValidModifyRemoteDeviceName(const std::string &processName)
{
    (void)processName;
    return true;
}

bool PermissionManager::CheckProcessNameValidOnGetDeviceInfo(const std::string &processName)
{
    (void)processName;
    return true;
}

bool PermissionManager::CheckProcessNameValidPutDeviceProfileInfoList(const std::string &processName)
{
    (void)processName;
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
