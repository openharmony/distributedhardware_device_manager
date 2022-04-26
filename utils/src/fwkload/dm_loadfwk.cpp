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
int32_t DmLoadFwk::LoadFwk(void)
{
    LOGI("enter DmLoadFwk::LoadFwk");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("failed to get system ability mgr.");
        return DM_SERVICE_NOT_READY;
    }
    const uint32_t nLoadCount = 3;
    int32_t ret = DM_OK;
    sptr<DistributedHardwareLoadCallback> loadCallback = new DistributedHardwareLoadCallback();
    for (size_t i = 0; i < nLoadCount; i++) {
        ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, loadCallback);
        if (ret == DM_OK) {
            break;
        }
    }
    if (ret != DM_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%d, ret code:%d, nLoadCount:%d",
            DISTRIBUTED_HARDWARE_SA_ID, ret, nLoadCount);
        return ERR_DM_LOAD_FWK_SA_FAIL;
    }
    LOGI("leave DmLoadFwk::LoadFwk");
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
