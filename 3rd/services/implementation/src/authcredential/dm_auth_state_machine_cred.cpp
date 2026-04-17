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

#include "dm_log_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_auth_state_cred.h"
#include "dm_auth_context_cred.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_state_machine_cred.h"

namespace OHOS {
namespace DistributedHardware {

DmAuthStateMachineCred::DmAuthStateMachineCred(std::shared_ptr<DmAuthCredContext> context)
{
    exceptionEvent_= {
        DmEventType::ON_ERROR,  // Authentication error, there is a possibility of retry.
        DmEventType::ON_TIMEOUT,
        DmEventType::ON_FAIL,  // Authentication failed
        DmEventType::ON_SCREEN_LOCKED,
    };

    running_ = true;
    direction_ = context->direction;

    if (direction_ == DM_AUTH_CRED_SOURCE) {
        this->InsertSrcTransTable();
    } else {
        this->InsertSinkTransTable();
    }

    this->SetCurState(DmAuthStateType::CRED_AUTH_IDLE_STATE);
    thread_ = std::thread(&DmAuthStateMachineCred::Run, this, context);
}

DmAuthStateMachineCred::~DmAuthStateMachineCred()
{
    Stop();
}

void DmAuthStateMachineCred::InsertSrcTransTable()
{
    stateTransitionTable_.insert({
        {DmAuthStateType::CRED_AUTH_IDLE_STATE, {DmAuthStateType::CRED_AUTH_SRC_START_STATE}},
        {DmAuthStateType::CRED_AUTH_SRC_START_STATE, {DmAuthStateType::CRED_AUTH_SRC_NEGOTIATE_STATE}},
        {DmAuthStateType::CRED_AUTH_SRC_NEGOTIATE_STATE, {DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_START_STATE}},
        {DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_START_STATE,
            {DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE}},
        {DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE,
            {DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE,
            DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_DONE_STATE}},
        {DmAuthStateType::CRED_SRC_CREDENTIAL_AUTH_DONE_STATE, {DmAuthStateType::CRED_AUTH_SRC_FINISH_STATE}},
        {DmAuthStateType::CRED_AUTH_SRC_FINISH_STATE, {}}
    });
}

void DmAuthStateMachineCred::InsertSinkTransTable()
{
    // Sink-end state transition table
    stateTransitionTable_.insert({
        {DmAuthStateType::CRED_AUTH_IDLE_STATE, {DmAuthStateType::CRED_AUTH_SINK_NEGOTIATE_STATE}},
        {DmAuthStateType::CRED_AUTH_SINK_NEGOTIATE_STATE, {
            DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_START_STATE,
        }},
        {DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_START_STATE, {
            DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE}},
        {DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE, {
            DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE,
            DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_DONE_STATE}},
        {DmAuthStateType::CRED_SINK_CREDENTIAL_AUTH_DONE_STATE, {DmAuthStateType::CRED_AUTH_SINK_FINISH_STATE}},
        {DmAuthStateType::CRED_AUTH_SINK_FINISH_STATE, {}}
    });
}

// Notification status transition. The execution status corresponds to specific actions and exception handling.
int32_t DmAuthStateMachineCred::TransitionTo(std::shared_ptr<DmAuthStateCred> state)
{
    int32_t ret = DM_OK;
    DmAuthStateType nextState = state->GetStateType();
    {
        std::lock_guard lock(stateMutex_);
        // The states after the finish status are illegal states.
        if (preState_ == DmAuthStateType::CRED_AUTH_SRC_FINISH_STATE ||
            preState_ == DmAuthStateType::CRED_AUTH_SINK_FINISH_STATE) {
            LOGE("DmAuthStateMachineCred::TransitionTo next state is invalid.");
            return ERR_DM_NEXT_STATE_INVALID;
        }
        if (this->CheckStateTransitValid(nextState)) {
            LOGI("DmAuthStateMachineCred: The state transition from %{public}d to %{public}d.", preState_, nextState);
            statesQueue_.push(state);
            preState_ = nextState;
        } else {
            // The state transition is invalid.
            LOGE("DmAuthStateMachineCred: The state transition does not meet the rule from %{public}d to %{public}d.",
                preState_, nextState);
            ret = ERR_DM_NEXT_STATE_INVALID;
            reason.store(ERR_DM_NEXT_STATE_INVALID);
            if (direction_ == DM_AUTH_CRED_SOURCE) {
                statesQueue_.push(std::make_shared<AuthCredSrcFinishState>());
                preState_ = DmAuthStateType::CRED_AUTH_SRC_FINISH_STATE;
            } else {
                statesQueue_.push(std::make_shared<AuthCredSinkFinishState>());
                preState_ = DmAuthStateType::CRED_AUTH_SINK_FINISH_STATE;
            }
        }
    }
    stateCv_.notify_one();
    return ret;
}

DmEventType DmAuthStateMachineCred::WaitExpectEvent(DmEventType eventType)
{
    std::set<DmEventType> expectedEvents = {eventType};
    return WaitExpectEvents(expectedEvents);
}

/*
Expected events in an action, which is used for blocking.
When any expected event is complete or other exceptions occur, the actual event is returned.
Other normal events continue to be blocked (only in the action).
*/
DmEventType DmAuthStateMachineCred::WaitExpectEvents(const std::set<DmEventType>& expectedEvents)
{
    /*
    1. Actual event is in expectedEvents set, return actual event
    2. Actual event = Abnormal event (event timeout or exception events in list), the actual event is also returned.
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
        
        // Check if the event is in expected events set or is an exception event
        if (expectedEvents.find(actualEventType) != expectedEvents.end() ||
            (exceptionEvent_.find(actualEventType) != exceptionEvent_.end())) {
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
void DmAuthStateMachineCred::NotifyEventFinish(DmEventType eventType)
{
    LOGI("DmAuthStateMachineCred: NotifyEventFinish Event:%{public}d.", eventType);
    {
        std::unique_lock lock(eventMutex_);
        eventQueue_.push(eventType);
    }
    eventCv_.notify_one();
    if (eventType == DmEventType::ON_FAIL) {
        if (direction_ == DM_AUTH_CRED_SOURCE) {
            this->TransitionTo(std::make_shared<AuthCredSrcFinishState>());
        } else {
            this->TransitionTo(std::make_shared<AuthCredSinkFinishState>());
        }
    }
}

// Cyclically wait for state transition and execute action.
void DmAuthStateMachineCred::Run(std::shared_ptr<DmAuthCredContext> context)
{
    while (running_.load()) {
        context->state = static_cast<int32_t>(GetCurState());
        auto state = FetchAndSetCurState();
        if (!state.has_value()) {
            break;
        }
        if (reason.load() != DM_OK) {
            context->reason = reason.load();
        }
        // Obtain the status and execute the status action.
        DmAuthStateType stateType = state.value()->GetStateType();
        int32_t ret = state.value()->Action(context);
        if (ret != DM_OK) {
            LOGE("DmAuthStateMachineCred::Run err:%{public}d", ret);
            if (context->reason == DM_OK) {
                // If the context reason is not set, set action ret.
                context->reason = ret;
            }
            if (context->direction == DM_AUTH_CRED_SOURCE) {
                this->TransitionTo(std::make_shared<AuthCredSrcFinishState>());
            } else {
                this->TransitionTo(std::make_shared<AuthCredSinkFinishState>());
            }
        } else {
            LOGI("DmAuthStateMachineCred::Run ok state:%{public}d", stateType);
        }
    }
    LOGI("DmAuthStateMachineCred::Run end");
}

std::optional<std::shared_ptr<DmAuthStateCred>> DmAuthStateMachineCred::FetchAndSetCurState()
{
    std::unique_lock lock(stateMutex_);
    stateCv_.wait(lock, [&] {
        return !running_.load() || !statesQueue_.empty();
    });

    if (!running_.load()) return std::nullopt;

    std::shared_ptr<DmAuthStateCred> state = statesQueue_.front();
    statesQueue_.pop();
    SetCurState(state->GetStateType());
    return state;
}

void DmAuthStateMachineCred::NotifyStateWait()
{
    std::lock_guard lock(stateMutex_);  // Use locking to prevent signal loss
    running_.store(false);
    stateCv_.notify_all();
}

void DmAuthStateMachineCred::NotifyEventWait()
{
    std::lock_guard lock(eventMutex_);  // Use locking to prevent signal loss
    running_.store(false);
    eventCv_.notify_all();
}

void DmAuthStateMachineCred::Stop()
{
    NotifyStateWait();
    NotifyEventWait();
    if (thread_.joinable()) {  // Prevent dobule join
        thread_.join();
    }
}

bool DmAuthStateMachineCred::IsWaitEvent()
{
    return isWait_.load();
}

void DmAuthStateMachineCred::SetCurState(DmAuthStateType state)
{
    LOGI("DmAuthStateMachineCred::SetCurState state: %{public}d", state);
    curState_ = state;
}

DmAuthStateType DmAuthStateMachineCred::GetCurState()
{
    return curState_;
}

// Verify the validity of the next state transition.
bool DmAuthStateMachineCred::CheckStateTransitValid(DmAuthStateType nextState)
{
    // Check whether the next state is AuthCredSrcFinishState or AuthCredSinkFinishState
    // which can directly switch to the state and return.
    if (direction_ == DM_AUTH_CRED_SOURCE) {
        if (nextState == DmAuthStateType::CRED_AUTH_SRC_FINISH_STATE) {
            return true;
        }
    } else {
        if (nextState == DmAuthStateType::CRED_AUTH_SINK_FINISH_STATE) {
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