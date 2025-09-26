/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "dm_log.h"
#include "ffrt.h"
#include "matching_skills.h"
#include "dm_single_instance.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::CommonEventSubscribeInfo;
using CommomEventCallback = std::function<void(int32_t)>;

class DmEventSubscriber : public CommonEventSubscriber {
public:
    DmEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo, const CommomEventCallback &callback,
        const std::vector<std::string> &eventNameVec) : CommonEventSubscriber(subscribeInfo),
        eventNameVec_(eventNameVec), callback_(callback) {}
    ~DmEventSubscriber() override = default;
    std::vector<std::string> GetSubscriberEventNameVec() const;
    void OnReceiveEvent(const CommonEventData &data) override;

private:
    std::vector<std::string> eventNameVec_;
    CommomEventCallback callback_;
};

class DmCommonEventManager {
public:
    DmCommonEventManager() = default;
    ~DmCommonEventManager();
    bool SubscribeServiceEvent(const std::vector<std::string> &eventNameVec, const CommomEventCallback &callback);
    bool UnsubscribeServiceEvent();

private:
    std::vector<std::string> eventNameVec_;
    bool eventValidFlag_ = false;
    ffrt::mutex evenSubscriberMutex_;
    std::shared_ptr<DmEventSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;

private:
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::shared_ptr<DmEventSubscriber> subscriber)
            : changeSubscriber_(subscriber) {}
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    private:
        std::shared_ptr<DmEventSubscriber> changeSubscriber_;
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_EVENT_MANAGER_ADAPT_H
