/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_auth_state.h"
#include "dm_auth_context.h"
#include "dm_auth_manager_base.h"
#include "dm_auth_state_machine.h"

namespace OHOS {
namespace DistributedHardware {

DmAuthStateMachine::DmAuthStateMachine(std::shared_ptr<DmAuthContext> context)
{
    exceptionEvent_= {
        DmEventType::ON_ERROR,  // Authentication error, there is a possibility of retry.
        DmEventType::ON_TIMEOUT,
        DmEventType::ON_FAIL,  // Authentication failed
        DmEventType::ON_SCREEN_LOCKED,
        DmEventType::ON_ULTRASONIC_PIN_TIMEOUT,
    };

    running_ = true;
    direction_ = context->direction;

    if (direction_ == DM_AUTH_SOURCE) {
        this->InsertSrcTransTable();
    } else {
        this->InsertSinkTransTable();
    }

    this->SetCurState(DmAuthStateType::AUTH_IDLE_STATE);
    thread_ = std::thread(&DmAuthStateMachine::Run, this, context);
}

DmAuthStateMachine::~DmAuthStateMachine()
{
    Stop();
}

void DmAuthStateMachine::InsertSrcTransTable()
{
    // Source-end state transition table
    stateTransitionTable_.insert({
        {DmAuthStateType::AUTH_IDLE_STATE, {DmAuthStateType::AUTH_SRC_START_STATE}},
        {DmAuthStateType::AUTH_SRC_START_STATE, {DmAuthStateType::AUTH_SRC_NEGOTIATE_STATE}},
        {DmAuthStateType::AUTH_SRC_NEGOTIATE_STATE, {DmAuthStateType::AUTH_SRC_CONFIRM_STATE}},
        {DmAuthStateType::AUTH_SRC_CONFIRM_STATE, {
            DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE,
            DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE, {
            DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_START_STATE,
            DmAuthStateType::AUTH_SRC_PIN_INPUT_STATE,
            DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_START_STATE,
            DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_START_STATE,
            DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_PIN_INPUT_STATE, {
            DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE, {
            DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE,
            DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE, {
            DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE,
            DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE, {
            DmAuthStateType::AUTH_SRC_CREDENTIAL_EXCHANGE_STATE,
            DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_CREDENTIAL_EXCHANGE_STATE, {
            DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_START_STATE,
            DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_START_STATE,
         {DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE}},

        {DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE,
         {DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_DONE_STATE}},

        {DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_DONE_STATE,
         {DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE, DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE}},

        {DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE, {DmAuthStateType::AUTH_SRC_FINISH_STATE}},

        {DmAuthStateType::AUTH_SRC_FINISH_STATE, {}}
    });

    InsertUltrasonicSrcTransTable();

    return;
}

void DmAuthStateMachine::InsertUltrasonicSrcTransTable()
{
    stateTransitionTable_.insert({
        {DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_START_STATE, {
            DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_DONE_STATE,
            DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_DONE_STATE, {
            DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE,
            DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_START_STATE, {
            DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_DONE_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_DONE_STATE, {
            DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE,
            DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SRC_PIN_NEGOTIATE_START_STATE, {
            DmAuthStateType::AUTH_SRC_CREDENTIAL_AUTH_START_STATE,
            DmAuthStateType::AUTH_SRC_PIN_INPUT_STATE,
            DmAuthStateType::AUTH_SRC_REVERSE_ULTRASONIC_START_STATE,
            DmAuthStateType::AUTH_SRC_FORWARD_ULTRASONIC_START_STATE,
            DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE,
        }}
    });

    return;
}

void DmAuthStateMachine::InsertSinkTransTable()
{
    // Sink-end state transition table
    stateTransitionTable_.insert({
        {DmAuthStateType::AUTH_IDLE_STATE, {DmAuthStateType::AUTH_SINK_NEGOTIATE_STATE}},
        {DmAuthStateType::AUTH_SINK_NEGOTIATE_STATE, {
            DmAuthStateType::AUTH_SINK_CONFIRM_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_CONFIRM_STATE, {
            DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE,
            DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE, {
            DmAuthStateType::AUTH_SINK_PIN_DISPLAY_STATE,
            DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_START_STATE,
            DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_START_STATE,
            DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_PIN_DISPLAY_STATE, {
            DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE, {
            DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE,
            DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE, {
            DmAuthStateType::AUTH_SINK_PIN_AUTH_DONE_STATE,
            DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_PIN_AUTH_DONE_STATE, {
            DmAuthStateType::AUTH_SINK_CREDENTIAL_EXCHANGE_STATE,
            DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE,
            DmAuthStateType::AUTH_SINK_FINISH_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_CREDENTIAL_EXCHANGE_STATE, {
            DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE, {
            DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE,
         {DmAuthStateType::AUTH_SINK_DATA_SYNC_STATE, DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE}},
        {DmAuthStateType::AUTH_SINK_DATA_SYNC_STATE, {DmAuthStateType::AUTH_SINK_FINISH_STATE}},
        {DmAuthStateType::AUTH_SINK_FINISH_STATE, {}}
    });

    InsertUltrasonicSinkTransTable();

    return;
}

void DmAuthStateMachine::InsertUltrasonicSinkTransTable()
{
    stateTransitionTable_.insert({
        {DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_START_STATE, {
            DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_DONE_STATE,
            DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_DONE_STATE, {
            DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE,
            DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_START_STATE, {
            DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_DONE_STATE,
            DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE,
        }},
        {DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_DONE_STATE, {
            DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE,
            DmAuthStateType::AUTH_SINK_PIN_NEGOTIATE_START_STATE,
        }}
    });
    return;
}

// Notification status transition. The execution status corresponds to specific actions and exception handling.
int32_t DmAuthStateMachine::TransitionTo(std::shared_ptr<DmAuthState> state)
{
    int32_t ret = DM_OK;
    DmAuthStateType nextState = state->GetStateType();
    {
        std::lock_guard lock(stateMutex_);
        // The states after the finish status are illegal states.
        if (preState_ == DmAuthStateType::AUTH_SRC_FINISH_STATE ||
            preState_ == DmAuthStateType::AUTH_SINK_FINISH_STATE) {
            LOGE("DmAuthStateMachine::TransitionTo next state is invalid.");
            return ERR_DM_NEXT_STATE_INVALID;
        }
        if (this->CheckStateTransitValid(nextState)) {
            LOGI("DmAuthStateMachine: The state transition from %{public}d to %{public}d.", preState_, nextState);
            statesQueue_.push(state);
            preState_ = nextState;
        } else {
            // The state transition is invalid.
            LOGE("DmAuthStateMachine: The state transition does not meet the rule from %{public}d to %{public}d.",
                preState_, nextState);
            ret = ERR_DM_NEXT_STATE_INVALID;
            reason = ERR_DM_NEXT_STATE_INVALID;
            if (direction_ == DM_AUTH_SOURCE) {
                statesQueue_.push(std::make_shared<AuthSrcFinishState>());
                preState_ = DmAuthStateType::AUTH_SRC_FINISH_STATE;
            } else {
                statesQueue_.push(std::make_shared<AuthSinkFinishState>());
                preState_ = DmAuthStateType::AUTH_SINK_FINISH_STATE;
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
DmEventType DmAuthStateMachine::WaitExpectEvent(DmEventType eventType)
{
    /*
    1. Actual event = Expected event, return actual event
    2. Actual event = Abnormal event (event timeout). The actual event is also returned.
    3. Actual event = Other events, continue to block, but there is a timeout limit.
    */
    std::unique_lock lock(eventMutex_);
    auto startTime = std::chrono::high_resolution_clock::now();
    while (running_.load()) {
        if (eventCv_.wait_for(lock, std::chrono::milliseconds(WAIT_TIMEOUT), [&] {
            return !running_.load() || !eventQueue_.empty() || eventCvReady_;
        })) {
            eventCvReady_ = false;
            LOGI("DmAuthStateMachine: WaitExpectEvent wait successful.");
        } else {
            if (!eventCvReady_) {
                LOGE("DmAuthStateMachine: WaitExpectEvent wait timeout.");
                return DmEventType::ON_TIMEOUT;
            }
            eventCvReady_ = false;
        }
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
void DmAuthStateMachine::NotifyEventFinish(DmEventType eventType)
{
    LOGI("DmAuthStateMachine: NotifyEventFinish Event:%{public}d.", eventType);
    {
        std::unique_lock lock(eventMutex_);
        eventQueue_.push(eventType);
        eventCvReady_ = true;
    }
    eventCv_.notify_one();
    if (eventType == DmEventType::ON_FAIL) {
        if (direction_ == DM_AUTH_SOURCE) {
            this->TransitionTo(std::make_shared<AuthSrcFinishState>());
        } else {
            this->TransitionTo(std::make_shared<AuthSinkFinishState>());
        }
    }
}

// Cyclically wait for state transition and execute action.
void DmAuthStateMachine::Run(std::shared_ptr<DmAuthContext> context)
{
    while (running_.load()) {
        auto state = FetchAndSetCurState();
        if (!state.has_value()) {
            break;
        }
        if (reason != DM_OK) {
            context->reason = reason;
        }
        // Obtain the status and execute the status action.
        DmAuthStateType stateType = state.value()->GetStateType();
        context->state = static_cast<int32_t>(stateType);
        int32_t ret = state.value()->Action(context);
        if (ret != DM_OK) {
            LOGE("DmAuthStateMachine::Run err:%{public}d", ret);
            if (context->reason == DM_OK) {
                // If the context reason is not set, set action ret.
                context->reason = ret;
            }
            context->successFinished = false;
            if (context->direction == DM_AUTH_SOURCE) {
                this->TransitionTo(std::make_shared<AuthSrcFinishState>());
            } else {
                this->TransitionTo(std::make_shared<AuthSinkFinishState>());
            }
        } else {
            LOGI("DmAuthStateMachine::Run ok state:%{public}d", stateType);
        }
    }
    LOGI("DmAuthStateMachine::Run end");
}

std::optional<std::shared_ptr<DmAuthState>> DmAuthStateMachine::FetchAndSetCurState()
{
    std::unique_lock lock(stateMutex_);
    stateCv_.wait(lock, [&] {
        return !running_.load() || !statesQueue_.empty();
    });

    if (!running_.load()) return std::nullopt;

    std::shared_ptr<DmAuthState> state = statesQueue_.front();
    statesQueue_.pop();
    SetCurState(state->GetStateType());
    return state;
}

void DmAuthStateMachine::NotifyStateWait()
{
    std::lock_guard lock(stateMutex_);  // Use locking to prevent signal loss
    running_.store(false);
    stateCv_.notify_all();
}

void DmAuthStateMachine::NotifyEventWait()
{
    std::lock_guard lock(eventMutex_);  // Use locking to prevent signal loss
    running_.store(false);
    eventCv_.notify_all();
}

void DmAuthStateMachine::Stop()
{
    NotifyStateWait();
    NotifyEventWait();
    if (thread_.joinable()) {  // Prevent dobule join
        thread_.join();
    }
}

void DmAuthStateMachine::SetCurState(DmAuthStateType state)
{
    LOGI("DmAuthStateMachine::SetCurState state: %{public}d", state);
    curState_ = state;
}

DmAuthStateType DmAuthStateMachine::GetCurState()
{
    return curState_;
}

// Verify the validity of the next state transition.
bool DmAuthStateMachine::CheckStateTransitValid(DmAuthStateType nextState)
{
    // Check whether the next state is AuthSrcFinishState or AuthSinkFinishState
    // which can directly switch to the state and return.
    if (direction_ == DM_AUTH_SOURCE) {
        if (nextState == DmAuthStateType::AUTH_SRC_FINISH_STATE) {
            return true;
        }
    } else {
        if (nextState == DmAuthStateType::AUTH_SINK_FINISH_STATE) {
            return true;
        }
    }

    // Check whether the state transition table is met.
    auto it = stateTransitionTable_.find(preState_);
    if (it != stateTransitionTable_.end()) {
        const std::set<DmAuthStateType>& allowedStates = it->second;
        return allowedStates.find(nextState) != allowedStates.end();
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
