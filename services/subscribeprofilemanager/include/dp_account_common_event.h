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

#ifndef OHOS_DP_ACCOUNT_COMMON_EVENT_H
#define OHOS_DP_ACCOUNT_COMMON_EVENT_H

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "matching_skills.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::CommonEventSubscribeInfo;
using AccountEventCallback = std::function<void(int32_t, std::string)>;

class DpAccountEventSubscriber : public CommonEventSubscriber {
public:
    DpAccountEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo, const AccountEventCallback &callback,
        const std::vector<std::string> &eventNameVec) : CommonEventSubscriber(subscribeInfo),
        eventNameVec_(eventNameVec), callback_(callback) {}
    ~DpAccountEventSubscriber() override = default;
    std::vector<std::string> GetSubscriberEventNameVec() const;
    void OnReceiveEvent(const CommonEventData &data) override;

private:
    std::vector<std::string> eventNameVec_;
    AccountEventCallback callback_;
};

class DpAccountCommonEventManager {
public:
    DpAccountCommonEventManager() = default;
    ~DpAccountCommonEventManager();
    bool SubscribeAccountCommonEvent(const std::vector<std::string> &eventNameVec,
        const AccountEventCallback &callback);
    bool UnsubscribeAccountCommonEvent();

private:
    std::vector<std::string> eventNameVec_;
    bool eventValidFlag_ = false;
    std::mutex evenSubscriberMutex_;
    std::shared_ptr<DpAccountEventSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;
    int32_t counter_ = 0;

private:
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::shared_ptr<DpAccountEventSubscriber> AccountSubscriber)
            : changeSubscriber_(AccountSubscriber) {}
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    private:
        std::shared_ptr<DpAccountEventSubscriber> changeSubscriber_;
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DP_ACCOUNT_COMMON_EVENT_H
