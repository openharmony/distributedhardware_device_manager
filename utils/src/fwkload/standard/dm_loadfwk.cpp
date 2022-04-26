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

#include "dm_loadfwk.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DmLoadFwk);

int32_t DmLoadFwk::LoadFwk(void)
{
    LOGI("enter DmLoadFwk::LoadFwk");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("failed to get system ability mgr.");
        return DM_SERVICE_NOT_READY;
    }
    if (loadCallback_ == nullptr) {
        const uint32_t nLoadCount = 3;
        loadCallback_ = new DistributedHardwareLoadCallback(nLoadCount);
    }
    int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, loadCallback_);
    if (ret != DM_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%d, ret code:%d",
            DISTRIBUTED_HARDWARE_SA_ID, ret);
        return ERR_DM_LOAD_FWK_SA_FAIL;
    }
    LOGI("leave DmLoadFwk::LoadFwk");
    return DM_OK;
}
void DmLoadFwk::ResetLoadCallback(void)
{
    if (loadCallback_ != nullptr) {
        loadCallback_ = nullptr;
    }
}
} // namespace DistributedHardware
} // namespace OHOS
