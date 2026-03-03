/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dm_ability_manager.h"

#include "ability_manager_client.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
AbilityStatus DmAbilityManager::StartAbility(AAFwk::Want &want)
{
    CHECK_NULL_RETURN(AAFwk::AbilityManagerClient::GetInstance(), AbilityStatus::ABILITY_STATUS_FAILED);
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (ret != 0) {
        LOGE("Connect Ability failed, error value = %{public}d", static_cast<int32_t>(ret));
        return AbilityStatus::ABILITY_STATUS_FAILED;
    }
    ErrCode result = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
    if (result != 0) {
        LOGE("Start Ability failed, error value = %{public}d", static_cast<int32_t>(result));
        return AbilityStatus::ABILITY_STATUS_FAILED;
    }
    return AbilityStatus::ABILITY_STATUS_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
