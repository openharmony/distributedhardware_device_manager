/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "dm_log.h"
#include "ipc_skeleton.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(AppManager);

const std::string AppManager::GetAppId()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    LOGI("AppManager::tokenCaller ID == %d", tokenId);
    if (AccessTokenKit::GetTokenTypeFlag(tokenId) != TOKEN_HAP) {
        return "";
    }
    HapTokenInfo tokenInfo;
    auto result = AccessTokenKit::GetHapTokenInfo(tokenId, tokenInfo);
    if (result != RET_SUCCESS) {
        LOGE("GetHapTokenInfo, tokenId = %d, result = %d, ", tokenId, result);
        return "";
    }

    return tokenInfo.appID;
}
} // namespace DistributedHardware
} // namespace OHOS
