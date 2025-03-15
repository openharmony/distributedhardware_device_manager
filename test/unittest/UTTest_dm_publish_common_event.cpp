/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_publish_common_event.h"

#include "bluetooth_def.h"
#include "common_event_support.h"
#include "dm_constants.h"
#include "dm_publish_common_event.h"
#include "matching_skills.h"
#include "system_ability_definition.h"
#include "wifi_msg.h"

namespace OHOS {
namespace DistributedHardware {
void DmPublishCommonEventManagerTest::SetUp()
{
}

void DmPublishCommonEventManagerTest::TearDown()
{
}

void DmPublishCommonEventManagerTest::SetUpTestCase()
{
}

void DmPublishCommonEventManagerTest::TearDownTestCase()
{
}

namespace {

HWTEST_F(DmPublishCommonEventManagerTest, SubscribePublishCommonEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmPublishCommonEventManager> publshCommonEventManager
        = std::make_shared<DmPublishCommonEventManager>();
    std::vector<std::string> publishCommonEventVec;
    bool ret = publshCommonEventManager->SubscribePublishCommonEvent(publishCommonEventVec, nullptr);
    ASSERT_EQ(ret, false);
    publishCommonEventVec.emplace_back(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    ret = publshCommonEventManager->SubscribePublishCommonEvent(publishCommonEventVec, nullptr);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmPublishCommonEventManagerTest, UnsubscribePublishCommonEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmPublishCommonEventManager> publshCommonEventManager
        = std::make_shared<DmPublishCommonEventManager>();
    bool ret = publshCommonEventManager->UnsubscribePublishCommonEvent();
    ASSERT_EQ(ret, false);
    publshCommonEventManager->eventValidFlag_ = true;
    ret = publshCommonEventManager->UnsubscribePublishCommonEvent();
    ASSERT_EQ(ret, true);
}

void PublishCommonEventCallback(int32_t bluetoothState, int32_t wifiState, int32_t screenState)
{
    (void)bluetoothState;
    (void)wifiState;
    (void)screenState;
}

HWTEST_F(DmPublishCommonEventManagerTest, OnReceiveEvent_001, testing::ext::TestSize.Level1)
{
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    data.SetWant(want);
    data.SetCode(0);
    std::vector<std::string> changeEventVec;
    changeEventVec.push_back(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    std::string strEvent = "test";
    std::vector<std::string> strEventVec;
    strEventVec.push_back(strEvent);
    PublishEventCallback callback = [=](const auto &arg1, const auto &arg2, const auto &arg3) {
        PublishCommonEventCallback(arg1, arg2, arg3);
    };
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(strEvent);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<DmPublishCommonEventManager> publshCommonEventManager
        = std::make_shared<DmPublishCommonEventManager>();
    publshCommonEventManager->subscriber_
        = std::make_shared<DmPublishEventSubscriber>(subscriberInfo, callback, strEventVec);
    publshCommonEventManager->subscriber_->OnReceiveEvent(data);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    data.SetWant(want);
    data.SetCode(static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_ON));
    publshCommonEventManager->subscriber_->OnReceiveEvent(data);

    data.SetCode(static_cast<int32_t>(Bluetooth::BTStateID::STATE_TURN_OFF));
    publshCommonEventManager->subscriber_->OnReceiveEvent(data);
    
    data.SetCode(-1);
    publshCommonEventManager->subscriber_->OnReceiveEvent(data);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    data.SetWant(want);
    data.SetCode(static_cast<int32_t>(OHOS::Wifi::WifiState::ENABLED));
    publshCommonEventManager->subscriber_->OnReceiveEvent(data);

    data.SetCode(static_cast<int32_t>(OHOS::Wifi::WifiState::DISABLED));
    publshCommonEventManager->subscriber_->OnReceiveEvent(data);

    EXPECT_NE(publshCommonEventManager->subscriber_->GetSubscriberEventNameVec(), changeEventVec);
}
}
} // namespace DistributedHardware
} // namespace OHOS
