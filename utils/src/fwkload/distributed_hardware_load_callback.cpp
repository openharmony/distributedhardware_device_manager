	/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "distributed_hardware_load_callback.h"
#include "dm_log.h"
namespace OHOS {
namespace DistributedHardware {
void DistributedHardwareLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    LOGI("load fwk SA success, systemAbilityId:%d, remoteObject result:%s",
        systemAbilityId, (remoteObject != nullptr) ? "true":"false");
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return;
    }
}

void DistributedHardwareLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGE("load fwk SA failed, systemAbilityId:%d", systemAbilityId);
}
}
}