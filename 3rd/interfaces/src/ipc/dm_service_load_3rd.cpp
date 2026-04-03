/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dm_service_load_3rd.h"

#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE_3RD(DmServiceLoad3rd);

int32_t DmServiceLoad3rd::LoadDMService(void)
{
    std::lock_guard<ffrt::mutex> autoLock(dmServiceLoadLock_);
    if (isDMServiceLoading_.load()) {
        LOGI("DM service is loading.");
        return DM_OK;
    }
    LOGI("start");
    isDMServiceLoading_.store(true);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        isDMServiceLoading_.store(false);
        LOGE("failed to get system ability mgr.");
        return ERR_DM_POINT_NULL;
    }
    sptr<DMLoad3rdCallback> dmLoad3rdCallback_(new DMLoad3rdCallback());
    int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, dmLoad3rdCallback_);
    if (ret != DM_OK) {
        isDMServiceLoading_.store(false);
        LOGE("Failed to Load DM service, ret code:%{public}d", ret);
        return ret;
    }
    return DM_OK;
}

void DmServiceLoad3rd::SetLoadFinish(void)
{
    std::lock_guard<ffrt::mutex> autoLock(dmServiceLoadLock_);
    isDMServiceLoading_.store(false);
}

void DMLoad3rdCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load DM service success remoteObject result:%{public}s", (remoteObject != nullptr) ? "true" : "false");
    DmServiceLoad3rd::GetInstance().SetLoadFinish();
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return;
    }
}

void DMLoad3rdCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGE("Load DM service failed, systemAbilityId: %{public}d", systemAbilityId);
    DmServiceLoad3rd::GetInstance().SetLoadFinish();
}
} // namespace DistributedHardware
} // namespace OHOS