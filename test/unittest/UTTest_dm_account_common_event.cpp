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
#include "UTTest_dm_account_common_event.h"

#include "common_event_support.h"
#include "dm_account_common_event.h"
#include "dm_constants.h"
#include "matching_skills.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
void DmAccountCommonEventManagerTest::SetUp()
{
}

void DmAccountCommonEventManagerTest::TearDown()
{
}

void DmAccountCommonEventManagerTest::SetUpTestCase()
{
}

void DmAccountCommonEventManagerTest::TearDownTestCase()
{
}

void AccountCommonEventCallbacks(const std::string commonEventType, int32_t currentUserId, int32_t beforeUserId)
{
}
namespace {
HWTEST_F(DmAccountCommonEventManagerTest, SubscribeAccountCommonEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAccountCommonEventManager> accountCommonEventManager
        = std::make_shared<DmAccountCommonEventManager>();
    std::vector<std::string> commonEventVec;
    bool ret = accountCommonEventManager->SubscribeAccountCommonEvent(commonEventVec, nullptr);
    ASSERT_EQ(ret, false);
    commonEventVec.emplace_back(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    ret = accountCommonEventManager->SubscribeAccountCommonEvent(commonEventVec, nullptr);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAccountCommonEventManagerTest, UnSubscribeAccountCommonEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAccountCommonEventManager> accountCommonEventManager
        = std::make_shared<DmAccountCommonEventManager>();
    bool ret = accountCommonEventManager->UnsubscribeAccountCommonEvent();
    ASSERT_EQ(ret, false);
    accountCommonEventManager->eventValidFlag_ = true;
    ret = accountCommonEventManager->UnsubscribeAccountCommonEvent();
    ASSERT_EQ(ret, true);
}

HWTEST_F(DmAccountCommonEventManagerTest, OnReceiveEvent_001, testing::ext::TestSize.Level1)
{
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    data.SetWant(want);
    data.SetCode(0);

    std::vector<std::string> changeEventVec;
    changeEventVec.push_back("changeEvent");
    std::string strEvent = "test";
    std::vector<std::string> strEventVec;
    strEventVec.push_back(strEvent);
    AccountEventCallback callback = [=](const auto &eventType, const auto &currentUserId, const auto &beforeUserId) {
        AccountCommonEventCallbacks(eventType, currentUserId, beforeUserId);
    };
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(strEvent);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<DmAccountCommonEventManager> accountCommonEventManager
        = std::make_shared<DmAccountCommonEventManager>();
    accountCommonEventManager->subscriber_
        = std::make_shared<DmAccountEventSubscriber>(subscriberInfo, callback, strEventVec);
    accountCommonEventManager->subscriber_->OnReceiveEvent(data);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    data.SetWant(want);
    data.SetCode(1);
    accountCommonEventManager->subscriber_->OnReceiveEvent(data);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT);
    data.SetWant(want);
    accountCommonEventManager->subscriber_->OnReceiveEvent(data);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN);
    data.SetWant(want);
    accountCommonEventManager->subscriber_->OnReceiveEvent(data);
    EXPECT_NE(accountCommonEventManager->subscriber_->GetSubscriberEventNameVec(), changeEventVec);
}
}
} // namespace DistributedHardware
} // namespace OHOS
