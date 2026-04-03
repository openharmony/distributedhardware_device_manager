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

#ifndef OHOS_DM_SA_LOAD_3RD_H
#define OHOS_DM_SA_LOAD_3RD_H

#include <mutex>
#include "ffrt.h"

#include "dm_single_instance_3rd.h"
#include "iremote_object.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace DistributedHardware {
class DMLoad3rdCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

class DmServiceLoad3rd {
    DM_DECLARE_SINGLE_INSTANCE_3RD(DmServiceLoad3rd);
public:
    int32_t LoadDMService(void);
    void SetLoadFinish(void);
private:
    std::atomic<bool> isDMServiceLoading_ = false;
    ffrt::mutex dmServiceLoadLock_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SA_LOAD_H