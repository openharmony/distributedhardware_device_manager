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

#include "UTTest_dm_screen_common_event.h"

#include "bluetooth_def.h"
#include "common_event_support.h"
#include "dm_constants.h"
#include "dm_screen_common_event.h"
#include "matching_skills.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
void DmScreenCommonEventManagerTest::SetUp()
{
}

void DmScreenCommonEventManagerTest::TearDown()
{
}

void DmScreenCommonEventManagerTest::SetUpTestCase()
{
}

void DmScreenCommonEventManagerTest::TearDownTestCase()
{
}

namespace {

HWTEST_F(DmScreenCommonEventManagerTest, SubscribeScreenCommonEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmScreenCommonEventManager> screenCommonEventManager
        = std::make_shared<DmScreenCommonEventManager>();
    std::vector<std::string> commonEventVec;
    bool ret = screenCommonEventManager->SubscribeScreenCommonEvent(commonEventVec, nullptr);
    ASSERT_EQ(ret, false);
    commonEventVec.emplace_back(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    ret = screenCommonEventManager->SubscribeScreenCommonEvent(commonEventVec, nullptr);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmScreenCommonEventManagerTest, UnSubscribeScreenCommonEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmScreenCommonEventManager> screenCommonEventManager
        = std::make_shared<DmScreenCommonEventManager>();
    bool ret = screenCommonEventManager->UnsubscribeScreenCommonEvent();
    ASSERT_EQ(ret, false);
    screenCommonEventManager->eventValidFlag_ = true;
    ret = screenCommonEventManager->UnsubscribeScreenCommonEvent();
    ASSERT_EQ(ret, true);
}
}
} // namespace DistributedHardware
} // namespace OHOS
