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

#include "auth_ui.h"

#include "dm_ability_manager.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
AuthUi::AuthUi()
{
    LOGI("AuthUi constructor");
}

int32_t AuthUi::ShowConfirmDialog(std::shared_ptr<DmAbilityManager> dmAbilityManager)
{
    if (dmAbilityManager == nullptr) {
        LOGE("AuthUi::dmAbilityManager is null");
        return DM_FAILED;
    }
    dmAbilityMgr_ = dmAbilityManager;
    return StartFaService();
}

int32_t AuthUi::StartFaService()
{
    AbilityStatus status = dmAbilityMgr_->StartAbility(AbilityRole::ABILITY_ROLE_PASSIVE);
    if (status != AbilityStatus::ABILITY_STATUS_SUCCESS) {
        LOGE("AuthUi::StartFaService timeout");
        return DM_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
