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

#ifndef OHOS_DM_AUTH_STATE_MACHINE_V2_H
#define OHOS_DM_AUTH_STATE_MACHINE_V2_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <thread>
#include <chrono>

#include "dm_auth_state.h"

namespace OHOS {
namespace DistributedHardware {

// Define the state transition table type
using StateTransitionTable = std::map<DmAuthStateType, std::set<DmAuthStateType>>;

enum DmEventType {
    ON_TRANSMIT = 0,
    ON_SESSION_KEY_RETURNED,
    ON_REQUEST,
    ON_FINISH,
    ON_ERROR,
    ON_ULTRASONIC_PIN_CHANGED,
    ON_ULTRASONIC_PIN_TIMEOUT,

    ON_TIMEOUT,
    ON_USER_OPERATION,
    ON_FAIL,
    ON_SCREEN_LOCKED,
    ON_SESSION_OPENED,
};

class DmAuthStateMachine {
public:
    DmAuthStateMachine(std::shared_ptr<DmAuthContext> context);
    ~DmAuthStateMachine();

    // Notify state transition, execute the corresponding action for the state, and handle exceptions
    // only allowed to be called within OnDataReceived
    int32_t TransitionTo(std::shared_ptr<DmAuthState> state);

    // Wait for the expected event within the action, block until the expected event is completed or
    // an exception occurs, returning the actual event that occurred (only allowed to be called within actions)
    DmEventType WaitExpectEvent(DmEventType eventType);

    // Notify the completion of an event, passing the event enumeration
    // (only allowed to be called when the event is triggered). If it's an exception event,
    // record it in the context's reason or reply.
    void NotifyEventFinish(DmEventType eventType);

    DmAuthStateType GetCurState();

    // Stop the thread
    void Stop();

private:
    // Loop to wait for state transitions and execute actions
    void Run(std::shared_ptr<DmAuthContext> context);
    void InsertSrcTransTable();
    void InsertSinkTransTable();
    void InsertUltrasonicSrcTransTable();
    void InsertUltrasonicSinkTransTable();
    void InsertCredentialAuthSrcTransTable();
    void InsertCredentialAuthSinkTransTable();

    // Fetch the current state and execute it
    std::optional<std::shared_ptr<DmAuthState>> FetchAndSetCurState();

    void SetCurState(DmAuthStateType state);

    bool CheckStateTransitValid(DmAuthStateType nextState);

    void NotifyEventWait();
    void NotifyStateWait();

    DmAuthStateType curState_;
    DmAuthStateType preState_{DmAuthStateType::AUTH_IDLE_STATE};  // Previous push state

    // State transition table for normal state transitions (all state transitions to the Finish state are valid)
    StateTransitionTable stateTransitionTable_;

    std::queue<DmEventType> eventQueue_;

    // Set of exception events
    std::set<DmEventType> exceptionEvent_;

    // Atomic flag to control the state machine's running state
    std::atomic<bool> running_;

    // Queue for storing states
    std::queue<std::shared_ptr<DmAuthState>> statesQueue_;

    // Synchronization primitives
    std::mutex stateMutex_;
    std::condition_variable stateCv_;
    std::mutex eventMutex_;
    std::condition_variable eventCv_;

    // Direction of authentication
    DmAuthDirection direction_;
    int32_t reason{DM_OK};

    // Thread for state machine execution
    std::thread thread_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_STATE_MACHINE_V2_H
