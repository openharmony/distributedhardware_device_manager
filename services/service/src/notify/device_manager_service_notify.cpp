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
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerServiceNotify);
namespace {
constexpr uint32_t MAX_CALLBACK_LEN = 1000;
const static std::map<DmCommonNotifyEvent, DmCommonNotifyEvent> unRegNotifyEventMap_ = {
    {DmCommonNotifyEvent::UN_REG_DEVICE_STATE, DmCommonNotifyEvent::REG_DEVICE_STATE},
    {DmCommonNotifyEvent::UN_REG_DEVICE_SCREEN_STATE, DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE},
    {DmCommonNotifyEvent::UN_REG_REMOTE_DEVICE_TRUST_CHANGE, DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE},
    {DmCommonNotifyEvent::UN_REG_CREDENTIAL_AUTH_STATUS_NOTIFY, DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY}
};

const static std::set<DmCommonNotifyEvent> regNotifyEventSet_ = {
    DmCommonNotifyEvent::REG_DEVICE_STATE,
    DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE,
    DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE,
    DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY
};
}

int32_t DeviceManagerServiceNotify::RegisterCallBack(int32_t dmCommonNotifyEvent, const std::string &pkgName)
{
    LOGI("start event %{public}d pkgName: %{public}s.", dmCommonNotifyEvent, pkgName.c_str());
    if (pkgName.empty()) {
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
        callbackMap_.at(unRegNotifyEventMap_.at(notifyEvent)).erase(pkgName);
        return DM_OK;
    }
    if (regNotifyEventSet_.find(notifyEvent) == regNotifyEventSet_.end()) {
        LOGE("notifyEvent is not support %{public}d.", notifyEvent);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (callbackMap_.find(notifyEvent) == callbackMap_.end()) {
        std::set<std::string> pkgNameSet;
        pkgNameSet.insert(pkgName);
        callbackMap_[notifyEvent] = pkgNameSet;
        return DM_OK;
    }
    if (callbackMap_[notifyEvent].size() >= MAX_CALLBACK_LEN) {
        LOGE("too many callbacks dmCommonNotifyEvent: %{public}d, pkgName: %{public}s", dmCommonNotifyEvent,
            pkgName.c_str());
        return ERR_DM_FAILED;
    }
    callbackMap_.at(notifyEvent).insert(pkgName);
    return DM_OK;
}

void DeviceManagerServiceNotify::GetCallBack(int32_t dmCommonNotifyEvent, std::set<std::string> &pkgNames)
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
    pkgNames = callbackMap_.at(notifyEvent);
}

void DeviceManagerServiceNotify::ClearDiedProcessCallback(const std::string &pkgName)
{
    LOGI("start pkgName %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(callbackLock_);
    for (auto iter = callbackMap_.begin(); iter != callbackMap_.end();) {
        iter->second.erase(pkgName);
        if (iter->second.empty()) {
            iter = callbackMap_.erase(iter);
        } else {
            ++iter;
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
