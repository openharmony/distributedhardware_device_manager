/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_PRIVACY_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_PRIVACY_H

#include "idistributed_hardware_sink.h"
#include "event_handler.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PRIVACY_SUBTYPE = "subtype";
const std::string PRIVACY_NETWORKID = "networkId";
constexpr uint32_t COMP_START_PAGE = 1;
constexpr uint32_t COMP_STOP_PAGE = 2;
constexpr uint32_t COMP_PRIVACY_DELAY_TIME = 1000; // million seconds
class ComponentPrivacy : public PrivacyResourcesListener {
public:
    ComponentPrivacy();
    virtual ~ComponentPrivacy();
    int32_t OnPrivaceResourceMessage(const ResourceEventType &type, const std::string &subtype,
        const std::string &networkId, bool &isSensitive, bool &isSameAccout) override;
    int32_t OnResourceInfoCallback(const std::string &subtype, const std::string &networkId,
        bool &isSensitive, bool &isSameAccout);
    int32_t StartPrivacePage(const std::string &subtype, const std::string &networkId);
    int32_t StopPrivacePage(const std::string &subtype);
    void SetPageFlagTrue();
    void SetPageFlagFalse();
    bool GetPageFlag();

    class ComponentEventHandler : public AppExecFwk::EventHandler {
    public:
        explicit ComponentEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner,
            ComponentPrivacy *comPrivacy);
        ~ComponentEventHandler() override;

        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    private:
        void ProcessStartPage(const AppExecFwk::InnerEvent::Pointer &event);
        void ProcessStopPage(const AppExecFwk::InnerEvent::Pointer &event);

        ComponentPrivacy *comPrivacyObj_;
    };

private:
    std::string DeviceTypeToString(uint16_t deviceTypeId);
    void HandleClosePage(const std::string &subtype);
    void HandlePullUpPage(const std::string &subtype, const std::string &networkId);

    std::shared_ptr<ComponentPrivacy::ComponentEventHandler> eventHandler_;
    std::atomic<bool> isPrivacePageOpen_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
