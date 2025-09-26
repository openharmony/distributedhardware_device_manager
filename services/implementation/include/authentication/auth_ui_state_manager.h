/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_AUTH_UI_STATE_MANAGER_H
#define OHOS_DM_AUTH_UI_STATE_MANAGER_H

#include <mutex>
#include <set>
#include <string>

#include "dm_ability_manager.h"
#include "ffrt.h"
#include "idevice_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
enum DmUiStateMsg : int32_t {
    MSG_PIN_CODE_ERROR = 0,
    MSG_PIN_CODE_SUCCESS,
    MSG_CANCEL_PIN_CODE_SHOW,
    MSG_CANCEL_PIN_CODE_INPUT,
    MSG_DOING_AUTH,
    MSG_CANCEL_CONFIRM_SHOW,
};

class AuthUiStateManager {
public:
    AuthUiStateManager(std::shared_ptr<IDeviceManagerServiceListener> listener);
    ~AuthUiStateManager() = default;
    void RegisterUiStateCallback(const std::string pkgName);
    void UnRegisterUiStateCallback(const std::string pkgName);
    void UpdateUiState(const DmUiStateMsg msg);
private:
    std::set<ProcessInfo> pkgSet_;
    ffrt::mutex pkgSetMutex_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
};

} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_UI_STATE_MANAGER_H
