/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "event_manager_adapt.h"

#include <thread>

#include "dm_constants.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace DistributedHardware {
std::once_flag DmCommonEventManager::onceFlag_;
std::list<CommomEventCallback> DmCommonEventManager::callbackQueue_;
std::mutex DmCommonEventManager::callbackQueueMutex_;
std::condition_variable DmCommonEventManager::notEmpty_;

DmCommonEventManager &DmCommonEventManager::GetInstance()
{
    static DmCommonEventManager instance;
    std::call_once(onceFlag_, [] {
        std::thread th(DealCallback);
        th.detach();
    });
    return instance;
}

void DmCommonEventManager::DealCallback(void)
{
    while (1) {
        std::unique_lock<std::mutex> callbackQueueLock(callbackQueueMutex_);
        notEmpty_.wait(callbackQueueLock, [] { return !callbackQueue_.empty(); });
        CommomEventCallback funcPrt = callbackQueue_.front();
        funcPrt();
        callbackQueue_.pop_front();
    }
}

bool DmCommonEventManager::SubscribeServiceEvent(const std::string &event, CommomEventCallback callback)
{
    LOGI("Subscribe event: %s", event.c_str());
    if (dmEventSubscriber_.find(event) != dmEventSubscriber_.end()) {
        LOGE("Subscribe event：%s has been added", event.c_str());
        return false;
    }

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(event);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<EventSubscriber> subscriber = std::make_shared<EventSubscriber>(subscriberInfo);
    if (subscriber == nullptr) {
        LOGE("subscriber is nullptr %s", event.c_str());
        return false;
    }
    subscriber->addEventCallback(event, callback);

    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);
    if (subscribeResult) {
        LOGE("Subscribe service event success: %s", event.c_str());
        dmEventSubscriber_[event] = subscriber;
        return subscribeResult;
    } else {
        LOGE("Subscribe service event failed: %s", event.c_str());
        return false;
    }
}

bool DmCommonEventManager::UnsubscribeServiceEvent(const std::string &event)
{
    LOGI("UnSubscribe event: %s", event.c_str());
    if (dmEventSubscriber_.find(event) != dmEventSubscriber_.end()) {
        LOGE("UnSubscribe event: %s not been exist", event.c_str());
        return false;
    }

    bool unsubscribeResult = CommonEventManager::UnSubscribeCommonEvent(dmEventSubscriber_[event]);
    if (unsubscribeResult) {
        LOGI("Unsubscribe service event success: %s", event.c_str());
        dmEventSubscriber_[event]->deleteEventCallback(event);
        dmEventSubscriber_.erase(event);
        return unsubscribeResult;
    } else {
        LOGE("Unsubscribe service event failed: %s", event.c_str());
        return false;
    }
}

DmCommonEventManager::~DmCommonEventManager()
{
    for (auto iter = dmEventSubscriber_.begin(); iter != dmEventSubscriber_.end(); iter++) {
        bool unsubscribeResult = CommonEventManager::UnSubscribeCommonEvent(iter->second);
        if (unsubscribeResult) {
            LOGI("Unsubscribe service event success: %s", iter->first.c_str());
        }
    }
}

void DmCommonEventManager::EventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    std::string event = data.GetWant().GetAction();
    LOGI("Received event: %s, value: %d", event.c_str());

    std::unique_lock<std::mutex> callbackLock(callbackLock_);
    auto iter = dmEventCallback_.find(event);
    if (iter != dmEventCallback_.end()) {
        CommomEventCallback funcPrt = iter->second;
        callbackLock_.unlock();

        std::unique_lock<std::mutex> callbackQueueLock(callbackQueueMutex_);
        if (callbackQueue_.size() <= COMMON_CALLBACK_MAX_SIZE) {
            callbackQueue_.push_back(funcPrt);
            notEmpty_.notify_one();
        } else {
            LOGE("event callback Queue is too long");
        }
    }
}

void DmCommonEventManager::EventSubscriber::addEventCallback(const std::string &event, CommomEventCallback callback)
{
    std::unique_lock<std::mutex> callbackLock(callbackLock_);
    if (dmEventCallback_.find(event) == dmEventCallback_.end()) {
        dmEventCallback_[event] = callback;
        LOGI("add event success: %s", event.c_str());
    }
}

void DmCommonEventManager::EventSubscriber::deleteEventCallback(const std::string &event)
{
    std::unique_lock<std::mutex> callbackLock(callbackLock_);
    if (dmEventCallback_.find(event) != dmEventCallback_.end()) {
        dmEventCallback_.erase(event);
        LOGI("delete event failed: %s", event.c_str());
    }
}
} // namespace DistributedHardware
} // namespace OHOS
