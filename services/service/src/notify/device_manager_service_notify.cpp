/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "device_manager_service_notify.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerServiceNotify);
namespace {
constexpr uint32_t MAX_CALLBACK_LEN = 1000;
const static std::map<DmCommonNotifyEvent, DmCommonNotifyEvent> unRegNotifyEventMap_ = {
    {DmCommonNotifyEvent::UN_REG_DEVICE_STATE, DmCommonNotifyEvent::REG_DEVICE_STATE},
    {DmCommonNotifyEvent::UN_REG_DEVICE_SCREEN_STATE, DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE},
    {DmCommonNotifyEvent::UN_REG_REMOTE_DEVICE_TRUST_CHANGE, DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE},
    {DmCommonNotifyEvent::UN_REG_CREDENTIAL_AUTH_STATUS_NOTIFY, DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY},
    {DmCommonNotifyEvent::UN_REG_AUTH_CODE_INVALID, DmCommonNotifyEvent::REG_AUTH_CODE_INVALID},
    {DmCommonNotifyEvent::UN_REG_SERVICE_STATE, DmCommonNotifyEvent::REG_SERVICE_STATE},
};

const static std::set<DmCommonNotifyEvent> regNotifyEventSet_ = {
    DmCommonNotifyEvent::REG_DEVICE_STATE,
    DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE,
    DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE,
    DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY,
    DmCommonNotifyEvent::REG_AUTH_CODE_INVALID,
    DmCommonNotifyEvent::REG_SERVICE_STATE,
};
}

int32_t DeviceManagerServiceNotify::RegisterCallBack(int32_t dmCommonNotifyEvent, const ProcessInfo &processInfo)
{
    LOGI("start event %{public}d pkgName: %{public}s.", dmCommonNotifyEvent, processInfo.pkgName.c_str());
    if (!PermissionManager::GetInstance().CheckDataSyncPermission() &&
        !PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission.");
        return ERR_DM_NO_PERMISSION;
    }
    if (processInfo.pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmCommonNotifyEvent notifyEvent = static_cast<DmCommonNotifyEvent>(dmCommonNotifyEvent);
    std::lock_guard<std::mutex> autoLock(callbackLock_);
    if (unRegNotifyEventMap_.find(notifyEvent) != unRegNotifyEventMap_.end()) {
        if (callbackMap_.find(unRegNotifyEventMap_.at(notifyEvent)) == callbackMap_.end()) {
            LOGI("notifyEvent is not exist %{public}d.", unRegNotifyEventMap_.at(notifyEvent));
            return DM_OK;
        }
        callbackMap_.at(unRegNotifyEventMap_.at(notifyEvent)).erase(processInfo);
        return DM_OK;
    }
    if (regNotifyEventSet_.find(notifyEvent) == regNotifyEventSet_.end()) {
        LOGE("notifyEvent is not support %{public}d.", notifyEvent);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (callbackMap_.find(notifyEvent) == callbackMap_.end()) {
        std::set<ProcessInfo> processInfoSet;
        processInfoSet.insert(processInfo);
        callbackMap_[notifyEvent] = processInfoSet;
        return DM_OK;
    }
    if (callbackMap_[notifyEvent].size() >= MAX_CALLBACK_LEN) {
        LOGE("too many callbacks dmCommonNotifyEvent: %{public}d, pkgName: %{public}s", dmCommonNotifyEvent,
            processInfo.pkgName.c_str());
        return ERR_DM_FAILED;
    }
    callbackMap_.at(notifyEvent).insert(processInfo);
    return DM_OK;
}

void DeviceManagerServiceNotify::GetCallBack(int32_t dmCommonNotifyEvent, std::set<ProcessInfo> &processInfos)
{
    LOGI("start event %{public}d.", dmCommonNotifyEvent);
    DmCommonNotifyEvent notifyEvent = static_cast<DmCommonNotifyEvent>(dmCommonNotifyEvent);
    std::lock_guard<std::mutex> autoLock(callbackLock_);
    if (regNotifyEventSet_.find(notifyEvent) == regNotifyEventSet_.end()) {
        LOGE("notifyEvent is not support %{public}d.", notifyEvent);
        return;
    }
    if (callbackMap_.find(notifyEvent) == callbackMap_.end()) {
        LOGE("callback is empty %{public}d.", notifyEvent);
        return;
    }
    processInfos = callbackMap_.at(notifyEvent);
}

void DeviceManagerServiceNotify::ClearDiedProcessCallback(const ProcessInfo &processInfo)
{
    LOGI("start pkgName %{public}s.", processInfo.pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(callbackLock_);
    for (auto iter = callbackMap_.begin(); iter != callbackMap_.end();) {
        iter->second.erase(processInfo);
        if (iter->second.empty()) {
            iter = callbackMap_.erase(iter);
        } else {
            ++iter;
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
