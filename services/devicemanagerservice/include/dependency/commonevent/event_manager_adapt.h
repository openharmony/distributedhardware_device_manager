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

#ifndef OHOS_EVENT_MANAGER_ADAPT_H
#define OHOS_EVENT_MANAGER_ADAPT_H

#include <map>
#include <mutex>

#include "common_event.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "dm_log.h"
#include "matching_skills.h"

namespace OHOS {
namespace DistributedHardware {
using CommomEventCallback = void (*)(void);

class DmCommonEventManager {
public:
    static DmCommonEventManager &GetInstance();

public:
    bool SubscribeServiceEvent(const std::string &event, CommomEventCallback callback);
    bool UnsubscribeServiceEvent(const std::string &event);

    class EventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        explicit EventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
            : EventFwk::CommonEventSubscriber(subscribeInfo)
        {
        }

        void OnReceiveEvent(const EventFwk::CommonEventData &data);
        void addEventCallback(const std::string &event, CommomEventCallback callback);
        void deleteEventCallback(const std::string &event);

    private:
        std::mutex callbackLock_;
        std::map<std::string, CommomEventCallback> dmEventCallback_;
    };

private:
    DmCommonEventManager() = default;
    virtual ~DmCommonEventManager();
    DmCommonEventManager(const DmCommonEventManager &) = delete;
    DmCommonEventManager &operator=(const DmCommonEventManager &) = delete;
    DmCommonEventManager(DmCommonEventManager &&) = delete;
    DmCommonEventManager &operator=(DmCommonEventManager &&) = delete;

private:
    static void DealCallback(void);
    static void InitCallbackThread(void);

private:
    static std::once_flag onceFlag_;
    static std::list<CommomEventCallback> callbackQueue_;
    static std::mutex callbackQueueMutex_;
    static std::condition_variable notEmpty_;
    std::map<std::string, std::shared_ptr<EventSubscriber>> dmEventSubscriber_;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_EVENT_MANAGER_ADAPT_H