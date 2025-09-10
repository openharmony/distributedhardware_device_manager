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

#include "dm_distributed_hardware_load.h"

#include "dm_log.h"
#include "if_system_ability_manager.h"      // for ISystemAbilityManager
#include "iservice_registry.h"              // for SystemAbilityManagerClient
#include "system_ability_definition.h"      // for DISTRIBUTED_HARDWARE_SA_ID

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DmDistributedHardwareLoad);
constexpr uint32_t MAX_LOAD_VALUE = 3;
constexpr int32_t DM_OK = 0;
DM_EXPORT void DmDistributedHardwareLoad::LoadDistributedHardwareFwk(void)
{
    LOGI("start");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("failed to get system ability mgr.");
        return;
    }
    distributedHardwareLoadCount_.fetch_add(1);
    sptr<DistributedHardwareLoadCallback> distributedHardwareLoadCallback_(new DistributedHardwareLoadCallback());
    int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, distributedHardwareLoadCallback_);
    if (ret != DM_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%{public}d, ret code:%{public}d",
            DISTRIBUTED_HARDWARE_SA_ID, ret);
    }
    return;
}
void DmDistributedHardwareLoad::InitDistributedHardwareLoadCount(void)
{
    distributedHardwareLoadCount_.store(0);
}
uint32_t DmDistributedHardwareLoad::GetDistributedHardwareLoadCount()
{
    return distributedHardwareLoadCount_.load();
}

void DistributedHardwareLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load SA success, systemAbilityId:%{public}d, remoteObject result:%{public}s",
        systemAbilityId, (remoteObject != nullptr) ? "true" : "false");
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return;
    }
    DmDistributedHardwareLoad::GetInstance().InitDistributedHardwareLoadCount();
}
void DistributedHardwareLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGE("Load SA failed, systemAbilityId:%{public}d", systemAbilityId);

    if (DmDistributedHardwareLoad::GetInstance().GetDistributedHardwareLoadCount() < MAX_LOAD_VALUE) {
        DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
    } else {
        DmDistributedHardwareLoad::GetInstance().InitDistributedHardwareLoadCount();
    }
    return;
}
} // namespace DistributedHardware
} // namespace OHOS
