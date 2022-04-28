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

#include "dm_constants.h"
#include "dm_log.h"
#include "dm_distributed_hardware_load.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DmDistributedHardwareLoad);
void DmDistributedHardwareLoad::LoadDistributedHardwareFwk(void)
{
    LOGI("enter DmDistributedHardwareLoad::LoadDistributedHardwareFwk");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("failed to get system ability mgr.");
    }
    distributedHardwareLoadCount_++;
    sptr<DistributedHardwareLoadCallback> distributedHardwareLoadCallback_ = new DistributedHardwareLoadCallback();
    int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, distributedHardwareLoadCallback_);
    if (ret != DM_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%d, ret code:%d",
            DISTRIBUTED_HARDWARE_SA_ID, ret);
    }
    return;
}
void DmDistributedHardwareLoad::InitDistributedHardwareLoadCount(void)
{
    distributedHardwareLoadCount_ = 0;
}
uint32_t DmDistributedHardwareLoad::GetDistributedHardwareLoadCount()
{
    return distributedHardwareLoadCount_;
}

void DistributedHardwareLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    LOGI("DmDistributedHhardware Load SA success, systemAbilityId:%d, remoteObject result:%s",
        systemAbilityId, (remoteObject != nullptr) ? "true" : "false");
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return;
    }
    DmDistributedHardwareLoad::GetInstance().InitDistributedHardwareLoadCount();
}
void DistributedHardwareLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGE("DmDistributedHhardware Load SA failed, systemAbilityId:%d", systemAbilityId);
    
    if (DmDistributedHardwareLoad::GetInstance().GetDistributedHardwareLoadCount() < MAX_LOAD_VAVLUE) {
        DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
    } else {
        DmDistributedHardwareLoad::GetInstance().InitDistributedHardwareLoadCount();
    }
    return;
}
} // namespace DistributedHardware
} // namespace OHOS
