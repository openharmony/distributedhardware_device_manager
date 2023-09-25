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

#include "auth_ui_state_manager.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* UI_STATE_MSG = "uiStateMsg";
AuthUiStateManager::AuthUiStateManager(std::shared_ptr<IDeviceManagerServiceListener> listener) : listener_(listener)
{
    LOGI("AuthUiStateManager constructor");
}

void AuthUiStateManager::RegisterUiStateCallback(const std::string pkgName)
{
    std::lock_guard<std::mutex> lock(pkgSetMutex_);
    pkgSet_.emplace(pkgName);
}

void AuthUiStateManager::UnRegisterUiStateCallback(const std::string pkgName)
{
    std::lock_guard<std::mutex> lock(pkgSetMutex_);
    if (pkgSet_.find(pkgName) == pkgSet_.end()) {
        LOGE("AuthUiStateManager UnRegisterUiStateCallback pkgName is not exist.");
        return;
    }
    pkgSet_.erase(pkgName);
}

void AuthUiStateManager::UpdateUiState(const DmUiStateMsg msg)
{
    if (listener_ == nullptr) {
        LOGE("AuthUiStateManager::UpdateUiState listener is null.");
        return;
    }
    nlohmann::json jsonObj;
    jsonObj[UI_STATE_MSG] = msg;
    std::string paramJson = jsonObj.dump();
    std::lock_guard<std::mutex> lock(pkgSetMutex_);
    for (auto pkgName : pkgSet_) {
        listener_->OnUiCall(pkgName, paramJson);
    }
    LOGI("AuthUiStateManager::UpdateUiState complete.");
}
} // namespace DistributedHardware
} // namespace OHOS
