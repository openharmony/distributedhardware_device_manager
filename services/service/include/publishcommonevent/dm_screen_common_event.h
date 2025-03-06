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

#ifndef OHOS_SCREEN_COMMON_EVENT_H
#define OHOS_SCREEN_COMMON_EVENT_H

#include "common_event_manager.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::CommonEventSubscribeInfo;
using ScreenEventCallback = std::function<void(std::string)>;

class DmScreenEventSubscriber : public CommonEventSubscriber {
public:
    DmScreenEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo, const ScreenEventCallback &callback,
        const std::vector<std::string> &eventNameVec) : CommonEventSubscriber(subscribeInfo),
        eventNameVec_(eventNameVec), callback_(callback) {}
    ~DmScreenEventSubscriber() override = default;
    std::vector<std::string> GetSubscriberEventNameVec() const;
    void OnReceiveEvent(const CommonEventData &data) override;

private:
    std::vector<std::string> eventNameVec_;
    ScreenEventCallback callback_;
};

class DmScreenCommonEventManager {
public:
    DmScreenCommonEventManager() = default;
    ~DmScreenCommonEventManager();
    bool SubscribeScreenCommonEvent(const std::vector<std::string> &eventNameVec,
        const ScreenEventCallback &callback);
    bool UnsubscribeScreenCommonEvent();

private:
    std::vector<std::string> eventNameVec_;
    bool eventValidFlag_ = false;
    std::mutex evenSubscriberMutex_;
    std::shared_ptr<DmScreenEventSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;
    int32_t counter_ = 0;

private:
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::shared_ptr<DmScreenEventSubscriber> ScreenSubscriber)
            : changeSubscriber_(ScreenSubscriber) {}
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    private:
        std::shared_ptr<DmScreenEventSubscriber> changeSubscriber_;
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_SCREEN_COMMON_EVENT_H
