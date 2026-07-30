/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "dm_account_common_event.h"

#include <pthread.h>
#include <thread>

#include "common_event_support.h"
#include "dm_account_event_queue.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif
#include "iservice_registry.h"
#include "multiple_user_connector.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::EventFwk;

constexpr const char* DEAL_THREAD = "account_common_event";
constexpr int32_t MAX_TRY_TIMES = 3;

constexpr const char* TAG_USER_ID_PARAM = "userId";
constexpr const char* TAG_ACCOUNT_ID_PARAM = "accountId";

std::vector<std::string> DmAccountEventSubscriber::GetSubscriberEventNameVec() const
{
    return eventNameVec_;
}

DmAccountCommonEventManager::~DmAccountCommonEventManager()
{
    DmAccountCommonEventManager::UnsubscribeAccountCommonEvent();
    DmAccountEventQueue::GetInstance().Stop();
}

bool DmAccountCommonEventManager::SubscribeAccountCommonEvent(const std::vector<std::string> &eventNameVec,
    const AccountEventCallback &callback)
{
    if (eventNameVec.empty() || callback == nullptr) {
        LOGE("eventNameVec is empty or callback is nullptr.");
        return false;
    }
    std::lock_guard<ffrt::mutex> locker(evenSubscriberMutex_);
    if (eventValidFlag_) {
        LOGE("failed to subscribe account commom eventName size: %{public}zu", eventNameVec.size());
        return false;
    }

    MatchingSkills matchingSkills;
    for (auto &item : eventNameVec) {
        matchingSkills.AddEvent(item);
    }
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<DmAccountEventSubscriber>(subscriberInfo, callback, eventNameVec);
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
    int32_t counter = 0;
    while (counter <= MAX_TRY_TIMES) {
        if (samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_) == ERR_OK) {
            LOGI("SubscribeAccountEvent success.");
            break;
        }
        if (++counter == MAX_TRY_TIMES) {
            LOGE("SubscribeAccountEvent failed.");
        }
        sleep(1);
    }
    eventNameVec_ = eventNameVec;
    eventValidFlag_ = true;
    DmAccountEventQueue::GetInstance().SetCallback(callback);
    DmAccountEventQueue::GetInstance().Start();
    LOGI("success to subscribe account commom event name size: %{public}zu", eventNameVec.size());
    return true;
}

bool DmAccountCommonEventManager::UnsubscribeAccountCommonEvent()
{
    std::lock_guard<ffrt::mutex> locker(evenSubscriberMutex_);
    if (!eventValidFlag_) {
        LOGE("failed to unsubscribe account commom event name size: %{public}zu because event is invalid.",
            eventNameVec_.size());
        return false;
    }
    if (subscriber_ != nullptr) {
        LOGI("start to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriber_)) {
            LOGE("failed to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
            return false;
        }
        LOGI("success to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
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
    DmAccountEventQueue::GetInstance().Stop();

    LOGI("success to unsubscribe account commom event name size: %{public}zu", eventNameVec_.size());
    eventValidFlag_ = false;
    return true;
}

bool DmAccountEventSubscriber::ParseUserCommonEvent(const CommonEventData &data,
    const std::string &receiveEvent, DmAccountEventInfo &eventInfo)
{
    if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        eventInfo.userId = data.GetCode();
        eventInfo.beforeUserId = std::atoi(data.GetWant().GetStringParam("oldId").c_str());
        LOGI("Received USER_SWITCHED event, userId: %{public}d, beforeUserId: %{public}d",
            eventInfo.userId, eventInfo.beforeUserId);
        return true;
    }
    if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED ||
        receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_STOPPED ||
        receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        eventInfo.beforeUserId = data.GetCode();
        LOGI("Received %{public}s event, beforeUserId: %{public}d", receiveEvent.c_str(), eventInfo.beforeUserId);
        return true;
    }
    if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT ||
        receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN) {
        eventInfo.userId = data.GetWant().GetIntParam("userId", 0);
        eventInfo.beforeUserId = eventInfo.userId;
        eventInfo.subProfileId = data.GetWant().GetIntParam(TAG_SUB_PROFILE_ID, 0);
        LOGI("Received %{public}s event, userId: %{public}d", receiveEvent.c_str(), eventInfo.userId);
        return true;
    }
    if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED ||
        receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_FOREGROUND ||
        receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_USER_BACKGROUND) {
        eventInfo.userId = data.GetCode();
        eventInfo.beforeUserId = eventInfo.userId;
        LOGI("Received %{public}s event, userId: %{public}d", receiveEvent.c_str(), eventInfo.userId);
        return true;
    }
    return false;
}

bool DmAccountEventSubscriber::ParseSubProfileEvent(const CommonEventData &data,
    const std::string &receiveEvent, DmAccountEventInfo &eventInfo)
{
    if (receiveEvent == CommonEventSupport::COMMON_EVENT_OS_ACCOUNT_SUB_PROFILE_DELETED) {
        eventInfo.userId = data.GetWant().GetIntParam(TAG_USER_ID_PARAM, 0);
        eventInfo.subProfileId = data.GetWant().GetIntParam(TAG_SUB_PROFILE_ID, 0);
        LOGI("Received SUB_PROFILE_DELETED event, userId: %{public}d, subProfileId: %{public}d",
            eventInfo.userId, eventInfo.subProfileId);
        return true;
    }
    if (receiveEvent == CommonEventSupport::COMMON_EVENT_OS_ACCOUNT_SUB_PROFILE_SWITCHED) {
        eventInfo.userId = data.GetWant().GetIntParam(TAG_USER_ID_PARAM, 0);
        eventInfo.subProfileId = data.GetWant().GetIntParam(TAG_SUB_PROFILE_ID, 0);
        eventInfo.previousSubProfileId = data.GetWant().GetIntParam(TAG_PREVIOUS_SUB_PROFILE_ID, 0);
        LOGI("Received SUB_PROFILE_SWITCHED event, userId: %{public}d, subProfileId: %{public}d, previous: %{public}d",
            eventInfo.userId, eventInfo.subProfileId, eventInfo.previousSubProfileId);
        return true;
    }
    if (receiveEvent == CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_BOUND) {
        eventInfo.userId = data.GetWant().GetIntParam(TAG_USER_ID_PARAM, 0);
        eventInfo.subProfileId = data.GetWant().GetIntParam(TAG_SUB_PROFILE_ID, 0);
        eventInfo.accountId = data.GetWant().GetStringParam(TAG_ACCOUNT_ID_PARAM);
        LOGI("Received ACCOUNT_BOUND event, userId: %{public}d, subProfileId: %{public}d, accountId: %{public}s",
            eventInfo.userId, eventInfo.subProfileId, GetAnonyString(eventInfo.accountId).c_str());
        return true;
    }
    if (receiveEvent == CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_UNBOUND) {
        eventInfo.userId = data.GetWant().GetIntParam(TAG_USER_ID_PARAM, 0);
        eventInfo.subProfileId = data.GetWant().GetIntParam(TAG_SUB_PROFILE_ID, 0);
        LOGI("Received DISTRIBUTED_ACCOUNT_UNBOUND event, userId: %{public}d, subProfileId: %{public}d",
            eventInfo.userId, eventInfo.subProfileId);
        return true;
    }
    return false;
}

void DmAccountEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    std::string receiveEvent = data.GetWant().GetAction();
    DmAccountEventInfo eventInfo;
    eventInfo.eventName = receiveEvent;

    bool accountValidEvent = ParseUserCommonEvent(data, receiveEvent, eventInfo);
    if (!accountValidEvent) {
        accountValidEvent = ParseSubProfileEvent(data, receiveEvent, eventInfo);
    }

    if (!accountValidEvent) {
        LOGE("Invalid account event type: %{public}s", receiveEvent.c_str());
        return;
    }

    DmAccountEventQueue::GetInstance().PushEvent(eventInfo);
}

void DmAccountCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is added with said: %{public}d.", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        return;
    }
    if (changeSubscriber_ == nullptr) {
        LOGE("failed to subscribe account commom event because changeSubscriber_ is nullptr.");
        return;
    }
    std::vector<std::string> eventNameVec = changeSubscriber_->GetSubscriberEventNameVec();
    LOGI("start to subscribe account commom eventName: %{public}zu", eventNameVec.size());
    if (!CommonEventManager::SubscribeCommonEvent(changeSubscriber_)) {
        LOGE("failed to subscribe account commom event: %{public}zu", eventNameVec.size());
    }
    MultipleUserConnector::CacheAllUsersAccountInfo();
}

void DmAccountCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is removed with said: %{public}d.", systemAbilityId);
}
} // namespace DistributedHardware
} // namespace OHOS