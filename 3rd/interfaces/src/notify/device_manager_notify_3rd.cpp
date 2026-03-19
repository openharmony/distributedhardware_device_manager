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

#include "device_manager_notify_3rd.h"

#include <thread>

#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"
#include "ipc_skeleton.h"
#include "permission_manager_3rd.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify3rd);

void DeviceManagerNotify3rd::RegisterDeathRecipientCallback(const std::string &businessName,
    std::shared_ptr<DmInit3rdCallback> DmInit3rdCallback)
{
    if (businessName.empty() || DmInit3rdCallback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<ffrt::mutex> autoLock(dmInit3rdCallbacklock_);
    CHECK_SIZE_VOID(dmInit3rdCallbackMap_);
    dmInit3rdCallbackMap_[businessName] = DmInit3rdCallback;
}

void DeviceManagerNotify3rd::UnRegisterDeathRecipientCallback(const std::string &businessName)
{
    if (businessName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<ffrt::mutex> autoLock(dmInit3rdCallbacklock_);
    dmInit3rdCallbackMap_.erase(businessName);
}

std::map<std::string, std::shared_ptr<DmInit3rdCallback>> DeviceManagerNotify3rd::GetDmInit3rdCallback()
{
    std::lock_guard<ffrt::mutex> autoLock(dmInit3rdCallbacklock_);
    std::map<std::string, std::shared_ptr<DmInit3rdCallback>> currentDmInitCallback = dmInit3rdCallbackMap_;
    return currentDmInitCallback;
}

void DeviceManagerNotify3rd::OnRemoteDied()
{
    LOGW("OnRemoteDied");
    std::map<std::string, std::shared_ptr<DmInit3rdCallback>> dmInit3rdCallbackMap = GetDmInit3rdCallback();
    for (auto iter : dmInit3rdCallbackMap) {
        LOGI("OnRemoteDied, pkgName:%{public}s", iter.first.c_str());
        if (iter.second != nullptr) {
            iter.second->OnRemoteDied();
        }
    }
}

int32_t DeviceManagerNotify3rd::RegisterAuthCallback(const std::string &businessName,
    std::shared_ptr<DmAuthCallback> dmAuthCallback)
{
    if (businessName.empty() || dmAuthCallback == nullptr) {
        LOGE("Invalid parameter, businessName is empty or dmAuthCallback is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string processName = "";
    if (PermissionManager3rd::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed.");
        return ERR_DM_FAILED;
    }

    ProcessInfo3rd processInfo3rd;
    processInfo3rd.tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    processInfo3rd.uid = OHOS::IPCSkeleton::GetCallingUid();
    processInfo3rd.processName = processName;
    processInfo3rd.businessName = businessName;
    int32_t userId = MultipleUserConnector3rd::GetCurrentAccountUserID();
    processInfo3rd.userId = userId;

    std::lock_guard<ffrt::mutex> autoLock(dmAuthCallbackLock_);
    dmAuthCallbackMap_[processInfo3rd] = dmAuthCallback;
    LOGI("completed");
    return DM_OK;
}

int32_t DeviceManagerNotify3rd::UnRegisterAuthCallback(const std::string &businessName)
{
    if (businessName.empty()) {
        LOGE("Invalid parameter, businessName is empty .");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string processName = "";
    if (PermissionManager3rd::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed.");
        return ERR_DM_FAILED;
    }

    ProcessInfo3rd processInfo3rd;
    processInfo3rd.tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    processInfo3rd.uid = OHOS::IPCSkeleton::GetCallingUid();
    processInfo3rd.processName = processName;
    processInfo3rd.businessName = businessName;
    int32_t userId = MultipleUserConnector3rd::GetCurrentAccountUserID();
    processInfo3rd.userId = userId;

    std::lock_guard<ffrt::mutex> autoLock(dmAuthCallbackLock_);
    if (dmAuthCallbackMap_.find(processInfo3rd) == dmAuthCallbackMap_.end()) {
        LOGE("Invalid parameter.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    dmAuthCallbackMap_.erase(processInfo3rd);
    LOGI("completed");
    return DM_OK;
}

void DeviceManagerNotify3rd::OnAuthResult(const ProcessInfo3rd &processInfo3rd, int32_t result, int32_t status,
    const std::string &authContent)
{
    std::shared_ptr<DmAuthCallback> tempCbk;
    {
        std::lock_guard<ffrt::mutex> autoLock(dmAuthCallbackLock_);
        if (dmAuthCallbackMap_.find(processInfo3rd) == dmAuthCallbackMap_.end()) {
            LOGE("No callback found.");
            return;
        }
        tempCbk = dmAuthCallbackMap_[processInfo3rd];
        dmAuthCallbackMap_.erase(processInfo3rd);
    }
    if (tempCbk == nullptr) {
        LOGE("callback is nullptr.");
        return;
    }
    tempCbk->OnAuthResult(processInfo3rd, result, status, authContent);
}
} // namespace DistributedHardware
} // namespace OHOS