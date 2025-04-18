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

#include "show_confirm.h"

#include "dm_auth_manager.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
ShowConfirm::ShowConfirm()
{
    LOGI("ShowConfirm ace constructor");
}

ShowConfirm::~ShowConfirm()
{
}

void ShowConfirm::ShowConfirmDialog(const std::string &params, std::shared_ptr<DmAuthManager> authManager,
                                    std::shared_ptr<DmAbilityManager> dmAbilityMgr)
{
    (void)authManager;
    LOGI("ShowConfirm hap start.");
    if (dmAbilityMgr == nullptr) {
        LOGE("ShowConfirm::dmAbilityManager is null");
        return;
    }
    AbilityStatus status = dmAbilityMgr->StartAbility(params);
    if (status != AbilityStatus::ABILITY_STATUS_SUCCESS) {
        LOGE("ShowConfirm::start ui service fail");
        return;
    }
    LOGI("ShowConfirm hap end");
}
} // namespace DistributedHardware
} // namespace OHOS