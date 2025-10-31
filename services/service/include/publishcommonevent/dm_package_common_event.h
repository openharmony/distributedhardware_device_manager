/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_PACKAGE_COMMON_EVENT_H
#define OHOS_PACKAGE_COMMON_EVENT_H

#include "common_event_manager.h"
#include "ffrt.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::CommonEventData;
using OHOS::EventFwk::CommonEventSubscriber;
using OHOS::EventFwk::CommonEventSubscribeInfo;
using PackageEventCallback = std::function<void(std::string, std::string, int32_t)>;

class DmPackageEventSubscriber : public CommonEventSubscriber {
public:
    DmPackageEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo, const PackageEventCallback &callback,
        const std::vector<std::string> &eventNameVec) : CommonEventSubscriber(subscribeInfo),
        eventNameVec_(eventNameVec), callback_(callback) {}
    ~DmPackageEventSubscriber() override = default;
    std::vector<std::string> GetSubscriberEventNameVec() const;
    void OnReceiveEvent(const CommonEventData &data) override;

private:
    std::vector<std::string> eventNameVec_;
    PackageEventCallback callback_;
};

class DmPackageCommonEventManager {
public:
    DmPackageCommonEventManager() = default;
    ~DmPackageCommonEventManager();
    bool SubscribePackageCommonEvent(const std::vector<std::string> &eventNameVec,
        const PackageEventCallback &callback);
    bool UnsubscribePackageCommonEvent();

private:
    std::vector<std::string> eventNameVec_;
    bool eventValidFlag_ = false;
    ffrt::mutex evenSubscriberMutex_;
    std::shared_ptr<DmPackageEventSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;

private:
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::shared_ptr<DmPackageEventSubscriber> PackageSubscriber)
            : changeSubscriber_(PackageSubscriber) {}
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

    private:
        std::shared_ptr<DmPackageEventSubscriber> changeSubscriber_;
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_PACKAGE_COMMON_EVENT_H
