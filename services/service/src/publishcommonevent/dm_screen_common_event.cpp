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

#include "dm_screen_common_event.h"

#include <pthread.h>
#include <thread>

#include "common_event_support.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::MatchingSkills;
using OHOS::EventFwk::CommonEventManager;

#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* DEAL_THREAD = "screen_lock";
#endif
constexpr int32_t MAX_TRY_TIMES = 3;

std::vector<std::string> DmScreenEventSubscriber::GetSubscriberEventNameVec() const
{
    return eventNameVec_;
}

DmScreenCommonEventManager::~DmScreenCommonEventManager()
{
    DmScreenCommonEventManager::UnsubscribeScreenCommonEvent();
}

bool DmScreenCommonEventManager::SubscribeScreenCommonEvent(const std::vector<std::string> &eventNameVec,
    const ScreenEventCallback &callback)
{
    if (eventNameVec.empty() || callback == nullptr) {
        LOGE("eventNameVec is empty or callback is nullptr.");
        return false;
    }
    std::lock_guard<std::mutex> locker(evenSubscriberMutex_);
    if (eventValidFlag_) {
        LOGE("failed to subscribe screen commom eventName size: %{public}zu", eventNameVec.size());
        return false;
    }

    MatchingSkills matchingSkills;
    for (auto &item : eventNameVec) {
        matchingSkills.AddEvent(item);
    }
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<DmScreenEventSubscriber>(subscriberInfo, callback, eventNameVec);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("samgrProxy is nullptr");
        subscriber_ = nullptr;
        return false;
    }
    statusChangeListener_ = new (std::nothrow) SystemAbilityStatusChangeListener(subscriber_);
    if (statusChangeListener_ == nullptr) {
        LOGE("statusChangeListener_ is nullptr");
        subscriber_ = nullptr;
        return false;
    }
    while (counter_ != MAX_TRY_TIMES) {
        if (samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_) == ERR_OK) {
            LOGI("SubscribeScreenEvent success.");
            counter_ = 0;
            break;
        }
        if (++counter_ == MAX_TRY_TIMES) {
            LOGI("SubscribeScreenEvent failed.");
        }
        sleep(1);
    }
    eventNameVec_ = eventNameVec;
    eventValidFlag_ = true;
    LOGI("success to subscribe screen commom event name size: %{public}zu", eventNameVec.size());
    return true;
}

bool DmScreenCommonEventManager::UnsubscribeScreenCommonEvent()
{
    std::lock_guard<std::mutex> locker(evenSubscriberMutex_);
    if (!eventValidFlag_) {
        LOGE("failed to unsubscribe screen commom event name size: %{public}zu because event is invalid.",
            eventNameVec_.size());
        return false;
    }
    if (subscriber_ != nullptr) {
        LOGI("start to unsubscribe screen commom event name size: %{public}zu", eventNameVec_.size());
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriber_)) {
            LOGE("failed to unsubscribe screen commom event name size: %{public}zu", eventNameVec_.size());
            return false;
        }
        LOGI("success to unsubscribe screen commom event name size: %{public}zu", eventNameVec_.size());
        subscriber_ = nullptr;
    }
    if (statusChangeListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            LOGE("samgrProxy is nullptr");
            return false;
        }
        int32_t ret = samgrProxy->UnSubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
        if (ret != ERR_OK) {
            LOGE("failed to unsubscribe system ability COMMON_EVENT_SERVICE_ID ret:%{public}d", ret);
            return false;
        }
        statusChangeListener_ = nullptr;
    }

    LOGI("success to unsubscribe screen commom event name size: %{public}zu", eventNameVec_.size());
    eventValidFlag_ = false;
    return true;
}

void DmScreenEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    std::string receiveEvent = data.GetWant().GetAction();
    LOGI("Received screen event: %{public}s", receiveEvent.c_str());
    if (receiveEvent != EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED) {
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { callback_(receiveEvent); });
#else
    std::thread dealThread(callback_, receiveEvent);
    int32_t ret = pthread_setname_np(dealThread.native_handle(), DEAL_THREAD);
    if (ret != DM_OK) {
        LOGE("dealThread setname failed.");
    }
    dealThread.detach();
#endif
}

void DmScreenCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is added with said: %{public}d.", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        return;
    }
    if (changeSubscriber_ == nullptr) {
        LOGE("failed to subscribe screen commom event because changeSubscriber_ is nullptr.");
        return;
    }
    std::vector<std::string> eventNameVec = changeSubscriber_->GetSubscriberEventNameVec();
    LOGI("start to subscribe screen commom eventName: %{public}zu", eventNameVec.size());
    if (!CommonEventManager::SubscribeCommonEvent(changeSubscriber_)) {
        LOGE("failed to subscribe screen commom event: %{public}zu", eventNameVec.size());
    }
}

void DmScreenCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is removed with said: %{public}d.", systemAbilityId);
}
} // namespace DistributedHardware
} // namespace OHOS
