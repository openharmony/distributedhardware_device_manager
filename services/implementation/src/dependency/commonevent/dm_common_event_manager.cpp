/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dm_common_event_manager.h"

#include <thread>

#include "dm_constants.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::MatchingSkills;
using OHOS::EventFwk::CommonEventManager;

std::string DmEventSubscriber::GetSubscriberEventName() const
{
    return eventName_;
}

DmCommonEventManager::~DmCommonEventManager()
{
    DmCommonEventManager::UnsubscribeServiceEvent();
}

bool DmCommonEventManager::SubscribeServiceEvent(const std::string &eventName, const CommomEventCallback &callback)
{
    if (eventName.empty() || callback == nullptr) {
        LOGE("enentNsmr is empty or callback is nullptr.");
        return false;
    }
    std::lock_guard<std::mutex> locker(evenSubscriberMutex_);
    if (eventValidFlag_) {
        LOGE("failed to subscribe commom eventName: %s.", eventName.c_str());
        return false;
    }

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<DmEventSubscriber>(subscriberInfo, callback, eventName);
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
    int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        LOGE("failed to subscribe system ability COMMON_EVENT_SERVICE_ID ret:%d", ret);
        subscriber_ = nullptr;
        statusChangeListener_ = nullptr;
        return false;
    }
    eventName_ = eventName;
    eventValidFlag_ = true;
    LOGI("success to subscribe commom eventName: %s", eventName.c_str());
    return true;
}

bool DmCommonEventManager::UnsubscribeServiceEvent()
{
    std::lock_guard<std::mutex> locker(evenSubscriberMutex_);
    if (!eventValidFlag_) {
        LOGE("failed to unsubscribe commom eventName: %s because event is invalid.", eventName_.c_str());
        return false;
    }
    if (subscriber_ != nullptr) {
        LOGI("start to unsubscribe commom eventName: %s", eventName_.c_str());
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriber_)) {
            LOGE("failed to unsubscribe commom eventName: %s.", eventName_.c_str());
            return false;
        }
        LOGI("success to unsubscribe commom eventName: %s.", eventName_.c_str());
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
            LOGE("failed to unsubscribe system ability COMMON_EVENT_SERVICE_ID ret:%d", ret);
            return false;
        }
        statusChangeListener_ = nullptr;
    }

    LOGI("success to unsubscribe commom eventName: %s", eventName_.c_str());
    eventValidFlag_ = false;
    return true;
}

void DmEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    std::string receiveEvent = data.GetWant().GetAction();
    LOGI("Received event: %s", receiveEvent.c_str());
    if (receiveEvent != eventName_) {
        LOGE("Received event and local event is not match");
        return;
    }
    int32_t userId = data.GetCode();
    if (userId <= 0) {
        LOGE("userId is less zero");
        return;
    }
    std::thread dealThread(callback_, userId);
    dealThread.detach();
}

void DmCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is added with said: %d.", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        return;
    }
    if (changeSubscriber_ == nullptr) {
        LOGE("failed to subscribe commom event because changeSubscriber_ is nullptr.");
        return;
    }
    std::string eventName = changeSubscriber_->GetSubscriberEventName();
    LOGI("start to subscribe commom eventName: %s", eventName.c_str());
    if (!CommonEventManager::SubscribeCommonEvent(changeSubscriber_)) {
        LOGE("failed to subscribe commom event: %s", eventName.c_str());
        return;
    }
}

void DmCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is removed with said: %d.", systemAbilityId);
}
} // namespace DistributedHardware
} // namespace OHOS
