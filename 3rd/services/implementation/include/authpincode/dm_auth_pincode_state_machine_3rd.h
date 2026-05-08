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

#ifndef OHOS_DM_AUTH_PINCODE_STATE_MACHINE_3RD_H
#define OHOS_DM_AUTH_PINCODE_STATE_MACHINE_3RD_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <thread>
#include "ffrt.h"

#include "dm_auth_pincode_state_3rd.h"
#include "dm_constants_3rd.h"

namespace OHOS {
namespace DistributedHardware {

// Define the state transition table type
using StateTransitionTable = std::map<DmAuthPincodeStateType, std::set<DmAuthPincodeStateType>>;

enum DmEventType {
    ON_TRANSMIT = 0,
    ON_SESSION_KEY_RETURNED,
    ON_REQUEST,
    ON_FINISH,
    ON_ERROR,
    ON_TIMEOUT,
    ON_FAIL,
    ON_SCREEN_LOCKED,
    ON_SESSION_OPENED,
};

class DmAuthPincodeStateMachine3rd {
public:
    DmAuthPincodeStateMachine3rd(std::shared_ptr<DmAuthPincodeContext> context);
    ~DmAuthPincodeStateMachine3rd();

    // Notify state transition, execute the corresponding action for the state, and handle exceptions
    // only allowed to be called within OnDataReceived
    int32_t TransitionTo(std::shared_ptr<DmAuthPincodeState3rd> state);

    // Wait for the expected event within the action, block until the expected event is completed or
    // an exception occurs, returning the actual event that occurred (only allowed to be called within actions)
    DmEventType WaitExpectEvent(DmEventType eventType);

    // Notify the completion of an event, passing the event enumeration
    // (only allowed to be called when the event is triggered). If it's an exception event,
    // record it in the context's reason or reply.
    void NotifyEventFinish(DmEventType eventType);

    DmAuthPincodeStateType GetCurState();

    // Stop the thread
    void Stop();

    bool IsWaitEvent();

private:
    // Loop to wait for state transitions and execute actions
    void Run(std::shared_ptr<DmAuthPincodeContext> context);
    void InsertSrcTransTable();
    void InsertSinkTransTable();

    // Fetch the current state and execute it
    std::optional<std::shared_ptr<DmAuthPincodeState3rd>> FetchAndSetCurState();

    void SetCurState(DmAuthPincodeStateType state);

    bool CheckStateTransitValid(DmAuthPincodeStateType nextState);

    void NotifyEventWait();
    void NotifyStateWait();

    DmAuthPincodeStateType curState_;
    DmAuthPincodeStateType preState_{DmAuthPincodeStateType::ACL_AUTH_IDLE_STATE};  // Previous push state

    // State transition table for normal state transitions (all state transitions to the Finish state are valid)
    StateTransitionTable stateTransitionTable_;

    std::queue<DmEventType> eventQueue_;

    // Set of exception events
    std::set<DmEventType> exceptionEvent_;

    // Atomic flag to control the state machine's running state
    std::atomic<bool> running_;

    // Queue for storing states
    std::queue<std::shared_ptr<DmAuthPincodeState3rd>> statesQueue_;

    // Synchronization primitives
    ffrt::mutex stateMutex_;
    ffrt::condition_variable stateCv_;
    ffrt::mutex eventMutex_;
    ffrt::condition_variable eventCv_;

    // Direction of authentication
    DmAuthPincodeDirection direction_;
    std::atomic<int32_t> reason{DM_OK};

    // Thread for state machine execution
    std::thread thread_;

    std::atomic<bool> isWait_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_PINCODE_STATE_MACHINE_3RD_H
