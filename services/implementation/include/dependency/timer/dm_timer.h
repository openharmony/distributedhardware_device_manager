/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DM_TIMER_H
#define DM_TIMER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <vector>

namespace OHOS {
namespace DistributedHardware {
constexpr const char* AUTHENTICATE_TIMEOUT_TASK = "deviceManagerTimer:authenticate";
constexpr const char* NEGOTIATE_TIMEOUT_TASK = "deviceManagerTimer:negotiate";
constexpr const char* CONFIRM_TIMEOUT_TASK = "deviceManagerTimer:confirm";
constexpr const char* INPUT_TIMEOUT_TASK = "deviceManagerTimer:input";
constexpr const char* ADD_TIMEOUT_TASK = "deviceManagerTimer:add";
constexpr const char* WAIT_NEGOTIATE_TIMEOUT_TASK = "deviceManagerTimer:waitNegotiate";
constexpr const char* WAIT_REQUEST_TIMEOUT_TASK = "deviceManagerTimer:waitRequest";
constexpr const char* STATE_TIMER_PREFIX = "deviceManagerTimer:stateTimer_";

constexpr int32_t DELAY_TICK_MILLSECONDS = 1000;
typedef std::chrono::steady_clock::time_point timerPoint;
typedef std::chrono::steady_clock steadyClock;
typedef std::chrono::duration<int64_t, std::ratio<1, 1000>> timerDuration;
using TimerCallback = std::function<void (std::string name)>;
const int32_t MILLISECOND_TO_SECOND = 1000;
const int32_t MIN_TIME_OUT = 0;
const int32_t MAX_TIME_OUT = 300;

class Timer {
public:
    Timer(std::string name, int32_t time, TimerCallback callback);
    ~Timer() {};
public:
    std::string timerName_;
    timerPoint expire_;
    bool state_;
    int32_t timeOut_;
    TimerCallback callback_;
};

struct TimerCmpare {
public:
    bool operator () (std::shared_ptr<Timer> frontTimer, std::shared_ptr<Timer> timer)
    {
        int32_t frontTimerOut = frontTimer->timeOut_ - (std::chrono::duration_cast<timerDuration>(steadyClock::now()
            - frontTimer->expire_).count() / MILLISECOND_TO_SECOND);
        int32_t timerOut = timer->timeOut_ - (std::chrono::duration_cast<timerDuration>(steadyClock::now()
            - timer->expire_).count() / MILLISECOND_TO_SECOND);
        return frontTimerOut > timerOut;
    }
};

class DmTimer {
public:
    DmTimer();
    ~DmTimer();

    /**
     * @tc.name: DmTimer::StartTimer
     * @tc.desc: start timer running
     * @tc.type: FUNC
     */
    int32_t StartTimer(std::string name, int32_t time, TimerCallback callback);

    /**
     * @tc.name: DmTimer::DeleteTimer
     * @tc.desc: delete timer
     * @tc.type: FUNC
     */
    int32_t DeleteTimer(std::string timerName);

    /**
     * @tc.name: DmTimer::DeleteAll
     * @tc.desc: delete all timer
     * @tc.type: FUNC
     */
    int32_t DeleteAll();

    /**
     * @tc.name: DmTimer::TimerRunning
     * @tc.desc: timer running
     * @tc.type: FUNC
     */
    int32_t TimerRunning();
private:
    mutable std::mutex timerMutex_;
    mutable std::mutex timerStateMutex_;
    std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, TimerCmpare> timerQueue_;
    std::map<std::string, std::shared_ptr<Timer>> timerMap_;
    std::atomic<bool> timerState_ {false};
    std::condition_variable runTimerCondition_;
    std::condition_variable stopTimerCondition_;
};
}
}
#endif