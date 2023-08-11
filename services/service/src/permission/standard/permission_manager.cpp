/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "ipc_skeleton.h"
#include "access_token.h"
#include "hap_token_info.h"
#include "native_token_info.h"
#include "accesstoken_kit.h"
#include "dm_log.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(PermissionManager);

namespace {
constexpr const char* DM_SERVICE_ACCESS_PERMISSION = "ohos.permission.ACCESS_SERVICE_DM";
constexpr const char* DM_SERVICE_ACCESS_NEWPERMISSION = "ohos.permission.DISTRIBUTED_DATASYNC";
}

bool PermissionManager::CheckPermission(void)
{
    LOGI("Enter PermissionManager::CheckPermission");
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        return false;
    }
    LOGI("PermissionManager::tokenCaller ID == %d", tokenCaller);

    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP || tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        int32_t ret = AccessTokenKit::VerifyAccessToken(tokenCaller, DM_SERVICE_ACCESS_PERMISSION);
        if (ret == PermissionState::PERMISSION_GRANTED) {
            return true;
        }
    }
    LOGE("DM service access is denied, please apply for corresponding permissions");
    return false;
}

bool PermissionManager::CheckNewPermission(void)
{
    LOGI("Enter PermissionManager::CheckNewPermission");
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        return false;
    }
    LOGI("PermissionManager::tokenCaller ID == %d", tokenCaller);

    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP || tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        int32_t ret = AccessTokenKit::VerifyAccessToken(tokenCaller, DM_SERVICE_ACCESS_NEWPERMISSION);
        if (ret == PermissionState::PERMISSION_GRANTED) {
            return true;
        }
    }
    LOGE("DM service access is denied, please apply for corresponding new permissions");
    return false;
}

} // namespace DistributedHardware
} // namespace OHOS
