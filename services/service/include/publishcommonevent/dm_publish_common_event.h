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

#ifndef OHOS_PUBLISH_COMMON_EVENT_H
#define OHOS_PUBLISH_COMMON_EVENT_H

#include "common_event_manager.h"
#include "system_ability_status_change_stub.h"

#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::CommonEventSubscribeInfo;
using PublishEventCallback = std::function<void(int32_t, int32_t, int32_t)>;

class DmPublishEventSubscriber : public CommonEventSubscriber {
public:
    DmPublishEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo, const PublishEventCallback &callback,
        const std::vector<std::string> &eventNameVec) : CommonEventSubscriber(subscribeInfo),
        eventNameVec_(eventNameVec), callback_(callback) {}
    ~DmPublishEventSubscriber() override = default;
    std::vector<std::string> GetSubscriberEventNameVec() const;
    void OnReceiveEvent(const CommonEventData &data) override;
    void SetWifiState(const int32_t wifiState);
    int32_t GetWifiState();
    void SetBluetoothState(const int32_t bluetoothState);
    int32_t GetBluetoothState();
    void SetScreenState(const int32_t screenState);
    int32_t GetScreenState();

private:
    void SetScreenEventState(const std::string &receiveEvent);
    std::vector<std::string> eventNameVec_;
    int32_t wifiState_ { -1 };
    int32_t bluetoothState_ { -1 };
    int32_t screenState_ = DM_SCREEN_UNKNOWN;
    PublishEventCallback callback_;
    std::mutex wifiStateMutex_;
    std::mutex bluetoothStateMutex_;
    std::mutex screenStateMutex_;
};

class DmPublishCommonEventManager {
public:
    DmPublishCommonEventManager() = default;
    ~DmPublishCommonEventManager();
    bool SubscribePublishCommonEvent(const std::vector<std::string> &eventNameVec,
        const PublishEventCallback &callback);
    bool UnsubscribePublishCommonEvent();
    void SetSubscriber(std::shared_ptr<DmPublishEventSubscriber> subscriber);
    std::shared_ptr<DmPublishEventSubscriber> GetSubscriber();

private:
    std::vector<std::string> eventNameVec_;
    bool eventValidFlag_ = false;
    std::mutex evenSubscriberMutex_;
    std::mutex subscriberMutex_;
    std::shared_ptr<DmPublishEventSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;
    int32_t counter_ = 0;

private:
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::shared_ptr<DmPublishEventSubscriber> publishSubscriber)
            : changeSubscriber_(publishSubscriber) {}
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    private:
        std::shared_ptr<DmPublishEventSubscriber> changeSubscriber_;
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_PUBLISH_COMMON_EVENT_H
