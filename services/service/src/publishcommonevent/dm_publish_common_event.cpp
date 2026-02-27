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

#include "dm_publish_common_event.h"

#include <pthread.h>
#include <thread>
#ifdef SUPPORT_BLUETOOTH
#include "bluetooth_def.h"
#endif // SUPPORT_BLUETOOTH
#include "common_event_support.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#ifdef SUPPORT_WIFI
#include "wifi_msg.h"
#endif // SUPPORT_WIFI
#include "dm_log.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif

namespace OHOS {
namespace DistributedHardware {
using OHOS::EventFwk::MatchingSkills;
using OHOS::EventFwk::CommonEventManager;

constexpr const char* DEAL_THREAD = "publish_common_event";
constexpr int32_t MAX_TRY_TIMES = 3;

std::vector<std::string> DmPublishEventSubscriber::GetSubscriberEventNameVec() const
{
    return eventNameVec_;
}

DmPublishCommonEventManager::~DmPublishCommonEventManager()
{
    DmPublishCommonEventManager::UnsubscribePublishCommonEvent();
}

bool DmPublishCommonEventManager::SubscribePublishCommonEvent(const std::vector<std::string> &eventNameVec,
    const PublishEventCallback &callback)
{
    if (eventNameVec.empty() || callback == nullptr) {
        LOGE("eventNameVec is empty or callback is nullptr.");
        return false;
    }
    std::lock_guard<ffrt::mutex> locker(evenSubscriberMutex_);
    if (eventValidFlag_) {
        LOGE("failed to subscribe ble/wifi/screen commom eventName size: %{public}zu", eventNameVec.size());
        return false;
    }

    MatchingSkills matchingSkills;
    for (auto &item : eventNameVec) {
        matchingSkills.AddEvent(item);
    }
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<DmPublishEventSubscriber>(subscriberInfo, callback, eventNameVec);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("samgrProxy is nullptr");
        subscriber_ = nullptr;
        return false;
    }
    statusChangeListener_ = new (std::nothrow) SystemAbilityStatusChangeListener(subscriber_);
    if (statusChangeListener_ == nullptr) {
        LOGE("statusChangeListener_ is nullptr");
        subscriber_ = nullptr;
        return false;
    }
    int32_t counter = 0;
    while (counter <= MAX_TRY_TIMES) {
        if (samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_) == ERR_OK) {
            LOGI("SubscribeServiceEvent success.");
            break;
        }
        if (++counter == MAX_TRY_TIMES) {
            LOGE("SubscribeServiceEvent failed.");
        }
        sleep(1);
    }
    eventNameVec_ = eventNameVec;
    eventValidFlag_ = true;
    SetSubscriber(subscriber_);
    LOGI("success to subscribe ble/wifi/screen commom event name size: %{public}zu", eventNameVec.size());
    return true;
}

bool DmPublishCommonEventManager::UnsubscribePublishCommonEvent()
{
    std::lock_guard<ffrt::mutex> locker(evenSubscriberMutex_);
    if (!eventValidFlag_) {
        LOGE("failed to unsubscribe ble/wifi/screen commom event name size: %{public}zu because event is invalid.",
            eventNameVec_.size());
        return false;
    }
    if (subscriber_ != nullptr) {
        LOGI("start to unsubscribe commom event name size: %{public}zu", eventNameVec_.size());
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriber_)) {
            LOGE("failed to unsubscribe ble/wifi/screen commom event name size: %{public}zu", eventNameVec_.size());
            return false;
        }
        LOGI("success to unsubscribe ble/wifi/screen commom event name size: %{public}zu", eventNameVec_.size());
        subscriber_ = nullptr;
    }
    if (statusChangeListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            LOGE("samgrProxy is nullptr");
            return false;
        }
        int32_t ret = samgrProxy->UnSubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
        if (ret != ERR_OK) {
            LOGE("failed to unsubscribe system ability COMMON_EVENT_SERVICE_ID ret:%{public}d", ret);
            return false;
        }
        statusChangeListener_ = nullptr;
    }

    LOGI("success to unsubscribe ble/wifi/screen commom event name size: %{public}zu", eventNameVec_.size());
    eventValidFlag_ = false;
    return true;
}

void DmPublishCommonEventManager::SetSubscriber(std::shared_ptr<DmPublishEventSubscriber> subscriber)
{
    std::lock_guard<ffrt::mutex> lock(subscriberMutex_);
    subscriber_ = subscriber;
}

std::shared_ptr<DmPublishEventSubscriber> DmPublishCommonEventManager::GetSubscriber()
{
    std::lock_guard<ffrt::mutex> lock(subscriberMutex_);
    return subscriber_;
}

void DmPublishEventSubscriber::SetWifiState(const int32_t wifiState)
{
    std::lock_guard<ffrt::mutex> lock(wifiStateMutex_);
    wifiState_ = wifiState;
}

int32_t DmPublishEventSubscriber::GetWifiState()
{
    std::lock_guard<ffrt::mutex> lock(wifiStateMutex_);
    return wifiState_;
}

void DmPublishEventSubscriber::SetBluetoothState(const int32_t bluetoothState)
{
    std::lock_guard<ffrt::mutex> lock(bluetoothStateMutex_);
    bluetoothState_ = bluetoothState;
}

int32_t DmPublishEventSubscriber::GetBluetoothState()
{
    std::lock_guard<ffrt::mutex> lock(bluetoothStateMutex_);
    return bluetoothState_;
}

void DmPublishEventSubscriber::SetScreenState(const int32_t screenState)
{
    std::lock_guard<ffrt::mutex> lock(screenStateMutex_);
    screenState_ = screenState;
}

int32_t DmPublishEventSubscriber::GetScreenState()
{
    std::lock_guard<ffrt::mutex> lock(screenStateMutex_);
    return screenState_;
}

void DmPublishEventSubscriber::SetScreenEventState(const std::string &receiveEvent)
{
    std::lock_guard<ffrt::mutex> lock(screenStateMutex_);
#ifdef SUPPORT_POWER_MANAGER
    if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        screenState_ = DM_SCREEN_ON;
    } else if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        screenState_ = DM_SCREEN_OFF;
    }
#else
    screenState_ = DM_SCREEN_ON;
#endif // SUPPORT_POWER_MANAGER
}

void DmPublishEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    std::string receiveEvent = data.GetWant().GetAction();
    int32_t eventState = data.GetCode();
    LOGI("On Received receiveEvent: %{public}s, eventState: %{public}d", receiveEvent.c_str(), eventState);
    SetScreenEventState(receiveEvent);

#ifdef SUPPORT_BLUETOOTH
    {
        std::lock_guard<ffrt::mutex> lock(bluetoothStateMutex_);
        if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE &&
            eventState == static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_ON)) {
            bluetoothState_ = static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_ON);
        } else if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE &&
            eventState == static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_OFF)) {
            bluetoothState_ = static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_OFF);
        } else if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE) {
            LOGI("bluetooth eventState: %{public}d", eventState);
            return;
        }
    }
#endif // SUPPORT_BLUETOOTH

#ifdef SUPPORT_WIFI
    {
        std::lock_guard<ffrt::mutex> lock(wifiStateMutex_);
        if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE &&
            eventState == static_cast<int32_t>(OHOS::Wifi::WifiState::ENABLED)) {
            wifiState_ = static_cast<int32_t>(OHOS::Wifi::WifiState::ENABLED);
        } else if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE &&
            eventState == static_cast<int32_t>(OHOS::Wifi::WifiState::DISABLED)) {
            wifiState_ = static_cast<int32_t>(OHOS::Wifi::WifiState::DISABLED);
        } else if (receiveEvent == EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE) {
            LOGI("wifi eventState: %{public}d", eventState);
            return;
        }
    }
#endif // SUPPORT_WIFI

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { callback_(bluetoothState_, wifiState_, screenState_); },
        ffrt::task_attr().name(DEAL_THREAD));
#else
    std::thread dealThread([=]() { callback_(bluetoothState_, wifiState_, screenState_); });
    int32_t ret = pthread_setname_np(dealThread.native_handle(), DEAL_THREAD);
    if (ret != DM_OK) {
        LOGE("dealThread setname failed.");
    }
    dealThread.detach();
#endif
}

void DmPublishCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is added with said: %{public}d.", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        return;
    }
    if (changeSubscriber_ == nullptr) {
        LOGE("failed to subscribe ble/wifi/screen commom event because changeSubscriber_ is nullptr.");
        return;
    }
    std::vector<std::string> eventNameVec = changeSubscriber_->GetSubscriberEventNameVec();
    LOGI("start to subscribe ble/wifi/screen commom eventName: %{public}zu", eventNameVec.size());
    if (!CommonEventManager::SubscribeCommonEvent(changeSubscriber_)) {
        LOGE("failed to subscribe ble/wifi/screen commom event: %{public}zu", eventNameVec.size());
        return;
    }
}

void DmPublishCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbility is removed with said: %{public}d.", systemAbilityId);
}
} // namespace DistributedHardware
} // namespace OHOS
