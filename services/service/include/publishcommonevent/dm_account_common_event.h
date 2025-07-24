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

#ifndef OHOS_ACCOUNT_COMMON_EVENT_H
#define OHOS_ACCOUNT_COMMON_EVENT_H

#include "common_event_manager.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::CommonEventSubscribeInfo;
/**
 * @brief account event callback define, fun(event_type, current userid, before userid)
 * first param, accont event
 * second param, the current userid
 * third param, the userid before.
 *          first param         |       second param        |       third param
 * ------------------------------------------
 * COMMON_EVENT_USER_SWITCHED   |   switch target user id   |   the user id before switch
 * COMMON_EVENT_USER_REMOVED    |           -1              |   the user id removed
 * COMMON_EVENT_HWID_LOGOUT     |  logout in witch user id  |   logout in witch user id
 * COMMON_EVENT_HWID_LOGIN      |  login in witch user id   |   login in witch user id
 */
using AccountEventCallback = std::function<void(std::string, int32_t, int32_t)>;

class DmAccountEventSubscriber : public CommonEventSubscriber {
public:
    DmAccountEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo, const AccountEventCallback &callback,
        const std::vector<std::string> &eventNameVec) : CommonEventSubscriber(subscribeInfo),
        eventNameVec_(eventNameVec), callback_(callback) {}
    ~DmAccountEventSubscriber() override = default;
    std::vector<std::string> GetSubscriberEventNameVec() const;
    void OnReceiveEvent(const CommonEventData &data) override;

private:
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
    std::mutex evenSubscriberMutex_;
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
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ACCOUNT_COMMON_EVENT_H
