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

/* SRC_FINISH is reachable directly via the finish-state shortcut for source direction. */
HWTEST_F(DmAuthStateMachineCredTest, TransitionTo_SourceFinishShortcut_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::shared_ptr<DmAuthStateCred> state = std::make_shared<AuthCredSrcFinishState>();
    int32_t ret = stateMachine->TransitionTo(state);
    EXPECT_EQ(ret, DM_OK);
}

/* Finish is terminal: any further transition is rejected with ERR_DM_NEXT_STATE_INVALID. */
HWTEST_F(DmAuthStateMachineCredTest, TransitionTo_IllegalAfterFinish_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    stateMachine->TransitionTo(std::make_shared<AuthCredSrcFinishState>());
    int32_t ret = stateMachine->TransitionTo(std::make_shared<AuthCredSrcStartState>());
    EXPECT_EQ(ret, ERR_DM_NEXT_STATE_INVALID);
}

/* From IDLE only SRC_START is allowed; SRC_NEGOTIATE violates the table -> invalid. */

/* For sink direction the shortcut accepts only SINK_FINISH; SRC_FINISH is rejected. */
HWTEST_F(DmAuthStateMachineCredTest, TransitionTo_InvalidTransitionSink_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    std::shared_ptr<DmAuthStateCred> state = std::make_shared<AuthCredSrcFinishState>();
    int32_t ret = stateMachine->TransitionTo(state);
    EXPECT_EQ(ret, ERR_DM_NEXT_STATE_INVALID);
}

/* Stop() unblocks the run loop and joins; state stays queryable. TearDown Stop() is idempotent. */
HWTEST_F(DmAuthStateMachineCredTest, Stop_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    stateMachine->Stop();
    DmAuthStateType state = stateMachine->GetCurState();
    EXPECT_EQ(state, DmAuthStateType::CRED_AUTH_IDLE_STATE);
}

/**
 * @tc.name: GetCurState_001
 * @tc.desc: Cover DmAuthStateMachineCred::GetCurState on a freshly constructed sink-direction
 *           machine, confirming the initial state is CRED_AUTH_IDLE_STATE.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateMachineCredTest, GetCurState_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    DmAuthStateType state = stateMachine->GetCurState();
    EXPECT_EQ(state, DmAuthStateType::CRED_AUTH_IDLE_STATE);
}

/**
 * @tc.name: NotifyEventFinish_001
 * @tc.desc: Cover DmAuthStateMachineCred::NotifyEventFinish with a normal (non-ON_FAIL) event that
 *           only enqueues the event without triggering a finish-state transition.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateMachineCredTest, NotifyEventFinish_001, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SOURCE;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    stateMachine->NotifyEventFinish(DmEventType::ON_NEXT);
    DmAuthStateType state = stateMachine->GetCurState();
    EXPECT_EQ(state, DmAuthStateType::CRED_AUTH_IDLE_STATE);
}

/**
 * @tc.name: Stop_002
 * @tc.desc: Cover DmAuthStateMachineCred::Stop idempotency by calling Stop twice on a sink
 *           machine, confirming the second call returns normally.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateMachineCredTest, Stop_002, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    stateMachine->Stop();
    stateMachine->Stop();
    DmAuthStateType state = stateMachine->GetCurState();
    EXPECT_EQ(state, DmAuthStateType::CRED_AUTH_IDLE_STATE);
}

/**
 * @tc.name: IsWaitEvent_002
 * @tc.desc: Cover DmAuthStateMachineCred::IsWaitEvent on a fresh sink-direction machine,
 *           confirming it returns false before any event wait begins.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(DmAuthStateMachineCredTest, IsWaitEvent_002, testing::ext::TestSize.Level1)
{
    context->direction = DM_AUTH_CRED_SINK;
    stateMachine = std::make_shared<DmAuthStateMachineCred>(context);
    bool ret = stateMachine->IsWaitEvent();
    EXPECT_EQ(ret, false);
}
}
}