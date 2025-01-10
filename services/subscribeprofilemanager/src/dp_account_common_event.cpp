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

#include "dp_account_common_event.h"

#include <pthread.h>
#include <thread>

#include "common_event_support.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_log.h"
#include "ffrt.h"
#include "iservice_registry.h"
#include "multi_user_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using OHOS::EventFwk::MatchingSkills;
using OHOS::EventFwk::CommonEventManager;

constexpr int32_t MAX_TRY_TIMES = 3;

namespace {
    const std::string TAG = "AccountCommonEvent";
}

std::vector<std::string> DpAccountEventSubscriber::GetSubscriberEventNameVec() const
{
    return eventNameVec_;
}

DpAccountCommonEventManager::~DpAccountCommonEventManager()
{
    DpAccountCommonEventManager::UnsubscribeAccountCommonEvent();
}

bool DpAccountCommonEventManager::SubscribeAccountCommonEvent(const std::vector<std::string> &eventNameVec,
    const AccountEventCallback &callback)
{
    if (eventNameVec.empty() || callback == nullptr) {
        HILOGE("eventNameVec is empty or callback is nullptr.");
        return false;
    }
    std::lock_guard<std::mutex> locker(evenSubscriberMutex_);
    if (eventValidFlag_) {
        HILOGE("failed to subscribe account commom eventName size: %{public}zu", eventNameVec.size());
        return false;
    }

    MatchingSkills matchingSkills;
    for (auto &item : eventNameVec) {
        matchingSkills.AddEvent(item);
    }
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<DpAccountEventSubscriber>(subscriberInfo, callback, eventNameVec);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("samgrProxy is nullptr");
        subscriber_ = nullptr;
        return false;
    }
    statusChangeListener_ = new (std::nothrow) SystemAbilityStatusChangeListener(subscriber_);
    if (statusChangeListener_ == nullptr) {
        HILOGE("statusChangeListener_ is nullptr");
        subscriber_ = nullptr;
        return false;
    }
    while (counter_ != MAX_TRY_TIMES) {
        if (samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_) == ERR_OK) {
            HILOGI("SubscribeAccountEvent success.");
            counter_ = 0;
            break;
        }
        if (++counter_ == MAX_TRY_TIMES) {
            HILOGE("SubscribeAccountEvent failed.");
        }
        sleep(1);
    }
    eventNameVec_ = eventNameVec;
    eventValidFlag_ = true;
    HILOGI("success to subscribe account commom event name size: %{public}zu", eventNameVec.size());
    return true;
}

bool DpAccountCommonEventManager::UnsubscribeAccountCommonEvent()
{
    std::lock_guard<std::mutex> locker(evenSubscriberMutex_);
    if (!eventValidFlag_) {
        HILOGE("failed to unsubscribe account commom event name size: %{public}zu because event is invalid.",
            eventNameVec_.size());
        return false;
    }
    if (subscriber_ != nullptr) {
        HILOGI("start to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriber_)) {
            HILOGE("failed to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
            return false;
        }
        HILOGI("success to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
        subscriber_ = nullptr;
    }
    if (statusChangeListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            HILOGE("samgrProxy is nullptr");
            return false;
        }
        int32_t ret = samgrProxy->UnSubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
        if (ret != ERR_OK) {
            HILOGE("failed to unsubscribe system ability COMMON_EVENT_SERVICE_ID ret:%{public}d", ret);
            return false;
        }
        statusChangeListener_ = nullptr;
    }

    HILOGI("success to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
    eventValidFlag_ = false;
    return true;
}

void DpAccountEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    std::string receiveEvent = data.GetWant().GetAction();
    HILOGI("Received account event: %{public}s", receiveEvent.c_str());
    int32_t userId = data.GetCode();
    bool isValidEvent = false;
    if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED ||
        receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        userId = data.GetCode();
        isValidEvent = true;
    }
    if (userId <= 0 || !isValidEvent) {
        HILOGE("Invalid account type event.");
        return;
    }
    ffrt::submit([=]() { callback_(userId, receiveEvent); });
}

void DpAccountCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbility is added with said: %{public}d.", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        return;
    }
    if (changeSubscriber_ == nullptr) {
        HILOGE("failed to subscribe account commom event because changeSubscriber_ is nullptr.");
        return;
    }
    std::vector<std::string> eventNameVec = changeSubscriber_->GetSubscriberEventNameVec();
    HILOGI("start to subscribe account commom eventName: %{public}zu", eventNameVec.size());
    if (!CommonEventManager::SubscribeCommonEvent(changeSubscriber_)) {
        HILOGE("failed to subscribe account commom event: %{public}zu", eventNameVec.size());
    }
}

void DpAccountCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbility is removed with said: %{public}d.", systemAbilityId);
}
} // namespace DistributedHardware
} // namespace OHOS
