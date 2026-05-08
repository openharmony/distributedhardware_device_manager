/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dm_auth_pincode_state_machine_3rd.h"

#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_state_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {

DmAuthPincodeStateMachine3rd::DmAuthPincodeStateMachine3rd(std::shared_ptr<DmAuthPincodeContext> context)
{
    exceptionEvent_= {
        DmEventType::ON_ERROR,  // Authentication error, there is a possibility of retry.
        DmEventType::ON_TIMEOUT,
        DmEventType::ON_FAIL,  // Authentication failed
        DmEventType::ON_SCREEN_LOCKED,
    };

    running_ = true;
    direction_ = context->direction;

    if (direction_ == DM_AUTH_PINCODE_SOURCE) {
        this->InsertSrcTransTable();
    } else {
        this->InsertSinkTransTable();
    }

    this->SetCurState(DmAuthPincodeStateType::ACL_AUTH_IDLE_STATE);
    thread_ = std::thread(&DmAuthPincodeStateMachine3rd::Run, this, context);
}

DmAuthPincodeStateMachine3rd::~DmAuthPincodeStateMachine3rd()
{
    Stop();
}

void DmAuthPincodeStateMachine3rd::InsertSrcTransTable()
{
    stateTransitionTable_.insert({
        {DmAuthPincodeStateType::ACL_AUTH_IDLE_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_START_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_START_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_NEGOTIATE_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_NEGOTIATE_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_START_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_START_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_DONE_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_PIN_AUTH_DONE_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_FINISH_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_FINISH_STATE, {}}
    });
}

void DmAuthPincodeStateMachine3rd::InsertSinkTransTable()
{
    // Sink-end state transition table
    stateTransitionTable_.insert({
        {DmAuthPincodeStateType::ACL_AUTH_IDLE_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_NEGOTIATE_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_NEGOTIATE_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_START_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_START_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE,
            {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE}},

        {DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE, {}}
    });
}

// Notification status transition. The execution status corresponds to specific actions and exception handling.
int32_t DmAuthPincodeStateMachine3rd::TransitionTo(std::shared_ptr<DmAuthPincodeState3rd> state)
{
    int32_t ret = DM_OK;
    DmAuthPincodeStateType nextState = state->GetStateType();
    {
        std::lock_guard lock(stateMutex_);
        // The states after the finish status are illegal states.
        if (preState_ == DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_FINISH_STATE ||
            preState_ == DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE) {
            LOGE("TransitionTo next state is invalid.");
            return ERR_DM_NEXT_STATE_INVALID;
        }
        if (this->CheckStateTransitValid(nextState)) {
            LOGI("The state transition from %{public}d to %{public}d.", preState_, nextState);
            statesQueue_.push(state);
            preState_ = nextState;
        } else {
            // The state transition is invalid.
            LOGE("The state transition does not meet the rule from %{public}d to %{public}d.", preState_, nextState);
            ret = ERR_DM_NEXT_STATE_INVALID;
            reason.store(ERR_DM_NEXT_STATE_INVALID);
            if (direction_ == DM_AUTH_PINCODE_SOURCE) {
                statesQueue_.push(std::make_shared<AuthPincodeSrcFinishState>());
                preState_ = DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_FINISH_STATE;
            } else {
                statesQueue_.push(std::make_shared<AuthPincodeSinkFinishState>());
                preState_ = DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE;
            }
        }
    }
    stateCv_.notify_one();
    return ret;
}

/*
Expected event in an action, which is used for blocking.
When the expected event is complete or other exceptions occur, the actual event is returned.
Other normal events continue to be blocked (only in the action).
*/
DmEventType DmAuthPincodeStateMachine3rd::WaitExpectEvent(DmEventType eventType)
{
    /*
    1. Actual event = Expected event, return actual event
    2. Actual event = Abnormal event (event timeout). The actual event is also returned.
    3. Actual event = Other events, continue to block, but there is a timeout limit.
    */
    std::unique_lock lock(eventMutex_);
    auto startTime = std::chrono::high_resolution_clock::now();
    while (running_.load()) {
        isWait_.store(true);
        eventCv_.wait(lock, [&] {
            return !running_.load() || !eventQueue_.empty();
        });
        isWait_.store(false);
        if (!running_.load()) {
            return DmEventType::ON_FAIL;
        }

        DmEventType actualEventType = eventQueue_.front();
        eventQueue_.pop();
        // Determine whether the event is an expected event or abnormal event in list.
        if (actualEventType == eventType || (exceptionEvent_.find(actualEventType) != exceptionEvent_.end())) {
            return actualEventType;
        }
        // Event Wait Timeout
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startTime);
        if (elapsedTime.count() >= EVENT_TIMEOUT) {
            break;
        }
    }
    return DmEventType::ON_TIMEOUT;
}

/*
The event is invoked after the event is complete.
The event enumeration can be invoked only when the event is triggered.
If the event is an abnormal event, the reason or reply of the context must be recorded.
*/
void DmAuthPincodeStateMachine3rd::NotifyEventFinish(DmEventType eventType)
{
    LOGI("DmAuthPincodeStateMachine3rd: NotifyEventFinish Event:%{public}d.", eventType);
    {
        std::unique_lock lock(eventMutex_);
        eventQueue_.push(eventType);
    }
    eventCv_.notify_one();
    if (eventType == DmEventType::ON_FAIL) {
        if (direction_ == DM_AUTH_PINCODE_SOURCE) {
            this->TransitionTo(std::make_shared<AuthPincodeSrcFinishState>());
        } else {
            this->TransitionTo(std::make_shared<AuthPincodeSinkFinishState>());
        }
    }
}

// Cyclically wait for state transition and execute action.
void DmAuthPincodeStateMachine3rd::Run(std::shared_ptr<DmAuthPincodeContext> context)
{
    while (running_.load()) {
        auto state = FetchAndSetCurState();
        if (!state.has_value()) {
            break;
        }
        if (reason.load() != DM_OK) {
            context->reason = reason.load();
        }
        // Obtain the status and execute the status action.
        DmAuthPincodeStateType stateType = state.value()->GetStateType();
        context->state = static_cast<int32_t>(GetCurState());
    LOGI("DmAuthPincodeStateMachine3rd::Run stateType: %{public}d, context->state: %{public}d",
        (int32_t)stateType, context->state);
        int32_t ret = state.value()->Action(context);
        if (ret != DM_OK) {
            LOGE("DmAuthPincodeStateMachine3rd::Run err:%{public}d", ret);
            if (context->reason == DM_OK) {
                // If the context reason is not set, set action ret.
                context->reason = ret;
            }
            if (context->direction == DM_AUTH_PINCODE_SOURCE) {
                this->TransitionTo(std::make_shared<AuthPincodeSrcFinishState>());
            } else {
                this->TransitionTo(std::make_shared<AuthPincodeSinkFinishState>());
            }
        } else {
            LOGI("DmAuthPincodeStateMachine3rd::Run ok state:%{public}d", stateType);
        }
    }
    LOGI("DmAuthPincodeStateMachine3rd::Run end");
}

std::optional<std::shared_ptr<DmAuthPincodeState3rd>> DmAuthPincodeStateMachine3rd::FetchAndSetCurState()
{
    std::unique_lock lock(stateMutex_);
    stateCv_.wait(lock, [&] {
        return !running_.load() || !statesQueue_.empty();
    });

    if (!running_.load()) return std::nullopt;

    std::shared_ptr<DmAuthPincodeState3rd> state = statesQueue_.front();
    statesQueue_.pop();
    SetCurState(state->GetStateType());
    return state;
}

void DmAuthPincodeStateMachine3rd::NotifyStateWait()
{
    std::lock_guard lock(stateMutex_);  // Use locking to prevent signal loss
    running_.store(false);
    stateCv_.notify_all();
}

void DmAuthPincodeStateMachine3rd::NotifyEventWait()
{
    std::lock_guard lock(eventMutex_);  // Use locking to prevent signal loss
    running_.store(false);
    eventCv_.notify_all();
}

void DmAuthPincodeStateMachine3rd::Stop()
{
    NotifyStateWait();
    NotifyEventWait();
    if (thread_.joinable()) {  // Prevent dobule join
        thread_.join();
    }
}

bool DmAuthPincodeStateMachine3rd::IsWaitEvent()
{
    return isWait_.load();
}

void DmAuthPincodeStateMachine3rd::SetCurState(DmAuthPincodeStateType state)
{
    LOGI("DmAuthPincodeStateMachine3rd::SetCurState state: %{public}d", state);
    curState_ = state;
}

DmAuthPincodeStateType DmAuthPincodeStateMachine3rd::GetCurState()
{
    return curState_;
}

// Verify the validity of the next state transition.
bool DmAuthPincodeStateMachine3rd::CheckStateTransitValid(DmAuthPincodeStateType nextState)
{
    // Check whether the next state is AuthPincodeSrcFinishState or AuthPincodeSinkFinishState
    // which can directly switch to the state and return.
    if (direction_ == DM_AUTH_PINCODE_SOURCE) {
        if (nextState == DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_FINISH_STATE) {
            return true;
        }
    } else {
        if (nextState == DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE) {
            return true;
        }
    }

    // Check whether the state transition table is met.
    auto it = stateTransitionTable_.find(preState_);
    if (it != stateTransitionTable_.end()) {
        const std::set<DmAuthPincodeStateType>& allowedStates = it->second;
        return allowedStates.find(nextState) != allowedStates.end();
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS