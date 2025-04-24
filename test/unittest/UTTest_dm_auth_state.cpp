/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dm_log.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_context.h"
#include "dm_auth_state_machine.h"
#include "UTTest_dm_auth_state.h"

using namespace testing;
namespace OHOS {
namespace DistributedHardware {

void DmAuthMessageProcessorTest::SetUpTestCase()
{
}

void DmAuthMessageProcessorTest::TearDownTestCase()
{
}

void DmAuthMessageProcessorTest::SetUp()
{
}

void DmAuthMessageProcessorTest::TearDown()
{
}

HWTEST_F(DmAuthStateTest, GetTaskTimeout_001, testing::ext::TestSize.Level1)
{
    const char *taskName = "taskName";
    int32_t taskTimeOut = 6;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authType = AUTH_TYPE_UNKNOW;
    std::shared_ptr<DmAuthState> state = std::make_shared<DmAuthState>();
    EXPECT_EQ(state->GetTaskTimeout(context, taskName, taskTimeOut), taskTimeOut);
}

HWTEST_F(DmAuthStateTest, GetTaskTimeout_002, testing::ext::TestSize.Level1)
{
    const char *taskName = "deviceManagerTimer:authenticate";
    int32_t taskTimeOut = 6;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authType = AUTH_TYPE_NFC;
    std::shared_ptr<DmAuthState> state = std::make_shared<DmAuthState>();
    EXPECT_EQ(state->GetTaskTimeout(context, taskName, taskTimeOut), 20);
}

HWTEST_F(DmAuthStateTest, GetTaskTimeout_003, testing::ext::TestSize.Level1)
{
    const char *taskName = "test";
    int32_t taskTimeOut = 6;
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->authType = AUTH_TYPE_NFC;
    std::shared_ptr<DmAuthState> state = std::make_shared<DmAuthState>();
    EXPECT_EQ(state->GetTaskTimeout(context, taskName, taskTimeOut), taskTimeOut);
}
} // namespace DistributedHardware
} // namespace OHOS
