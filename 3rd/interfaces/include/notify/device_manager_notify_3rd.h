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

#ifndef OHOS_DM_NOTIFY_3RD_H
#define OHOS_DM_NOTIFY_3RD_H

#include <map>
#include "ffrt.h"

#include "device_manager_callback_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_single_instance_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerNotify3rd {
    DM_DECLARE_SINGLE_INSTANCE(DeviceManagerNotify3rd);

public:
    void RegisterDeathRecipientCallback(const std::string &businessName,
                                        std::shared_ptr<DmInit3rdCallback> DmInit3rdCallback);
    void UnRegisterDeathRecipientCallback(const std::string &businessName);
    std::map<std::string, std::shared_ptr<DmInit3rdCallback>> GetDmInit3rdCallback();
    void OnRemoteDied();
    int32_t RegisterAuthCallback(const std::string &businessName, std::shared_ptr<DmAuthCallback> dmAuthCallback);
    int32_t UnRegisterAuthCallback(const std::string &businessName);
    void OnAuthResult(const ProcessInfo3rd &processInfo3rd, int32_t result, int32_t status,
        const std::string &authContent);
private:
    ffrt::mutex dmInit3rdCallbacklock_;
    std::map<std::string, std::shared_ptr<DmInit3rdCallback>> dmInit3rdCallbackMap_;
    ffrt::mutex dmAuthCallbackLock_;
    std::map<ProcessInfo3rd, std::shared_ptr<DmAuthCallback>> dmAuthCallbackMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_NOTIFY_3RD_H