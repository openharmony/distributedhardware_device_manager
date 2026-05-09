/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UTTest_dm_auth_state_machine_cred.h"
#include "dm_log_3rd.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

void DmAuthStateMachineCredTest::SetUpTestCase()
{
    LOGI("SetUpTestCase start");
}

void DmAuthStateMachineCredTest::TearDownTestCase()
{
    LOGI("TearDownTestCase start");
}

void DmAuthStateMachineCredTest::SetUp()
{
    LOGI("SetUp start");
    context = std::make_shared<DmAuthCredContext>();
}

void DmAuthStateMachineCredTest::TearDown()
{
    LOGI("TearDown start");
    if (stateMachine != nullptr) {
        stateMachine->Stop();
    }
    context = nullptr;
    stateMachine = nullptr;
}

HWTEST_F(DmAuthStateMachineCredTest, Constructor_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    EXPECT_NE(stateMachine, nullptr);
}

HWTEST_F(DmAuthStateMachineCredTest, Constructor_002, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    EXPECT_NE(stateMachine, nullptr);
}

HWTEST_F(DmAuthStateMachineCredTest, GetCurState_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    DmAuthStateType state = stateMachine->GetCurState();
    EXPECT_EQ(state, DmAuthStateType::CRED_AUTH_IDLE_STATE);
}

HWTEST_F(DmAuthStateMachineCredTest, TransitionTo_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::shared_ptr<DmAuthStateCred> state = std::make_shared<AuthCredSrcStartState>();
    int32_t ret = stateMachine->TransitionTo(state);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthStateMachineCredTest, TransitionTo_002, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::shared_ptr<DmAuthStateCred> state = std::make_shared<AuthCredSinkFinishState>();
    int32_t ret = stateMachine->TransitionTo(state);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthStateMachineCredTest, IsWaitEvent_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    bool ret = stateMachine->IsWaitEvent();
    EXPECT_EQ(ret, false);
}

}
}