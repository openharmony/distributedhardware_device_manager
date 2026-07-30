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

#ifndef OHOS_ACCOUNT_COMMON_EVENT_H
#define OHOS_ACCOUNT_COMMON_EVENT_H

#include "common_event_manager.h"
#include "ffrt.h"
#include "system_ability_status_change_stub.h"
#include <string>
#include <vector>

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::CommonEventSubscribeInfo;

struct DmAccountEventInfo {
    std::string eventName;
    int32_t userId = -1;
    int32_t beforeUserId = -1;
    int32_t subProfileId = 0;
    int32_t previousSubProfileId = 0;
    std::string accountId;
};

using AccountEventCallback = std::function<void(const DmAccountEventInfo&)>;

class DmAccountEventSubscriber : public CommonEventSubscriber {
public:
    DmAccountEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo,
        const AccountEventCallback &callback, const std::vector<std::string> &eventNameVec)
        : CommonEventSubscriber(subscribeInfo), eventNameVec_(eventNameVec), callback_(callback) {}
    ~DmAccountEventSubscriber() override = default;
    std::vector<std::string> GetSubscriberEventNameVec() const;
    void OnReceiveEvent(const CommonEventData &data) override;

private:
    bool ParseUserCommonEvent(const CommonEventData &data, const std::string &receiveEvent,
        DmAccountEventInfo &eventInfo);
    bool ParseSubProfileEvent(const CommonEventData &data, const std::string &receiveEvent,
        DmAccountEventInfo &eventInfo);
    std::vector<std::string> eventNameVec_;
    AccountEventCallback callback_;
};

class DmAccountCommonEventManager {
public:
    DmAccountCommonEventManager() = default;
    ~DmAccountCommonEventManager();
    bool SubscribeAccountCommonEvent(const std::vector<std::string> &eventNameVec,
        const AccountEventCallback &callback);
    bool UnsubscribeAccountCommonEvent();

private:
    std::vector<std::string> eventNameVec_;
    bool eventValidFlag_ = false;
    ffrt::mutex evenSubscriberMutex_;
    std::shared_ptr<DmAccountEventSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;

private:
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::shared_ptr<DmAccountEventSubscriber> AccountSubscriber)
            : changeSubscriber_(AccountSubscriber) {}
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    private:
        std::shared_ptr<DmAccountEventSubscriber> changeSubscriber_;
    };
};

using DmSubProfileEventInfo = DmAccountEventInfo;
using SubProfileEventCallback = AccountEventCallback;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ACCOUNT_COMMON_EVENT_H