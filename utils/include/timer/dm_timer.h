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
#include <unordered_set>

#include "event_handler.h"

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
constexpr const char* AUTH_DEVICE_TIMEOUT_TASK = "deviceManagerTimer:authDevice_";
constexpr const char* SESSION_HEARTBEAT_TIMEOUT_TASK = "deviceManagerTimer:sessionHeartbeat";

using TimerCallback = std::function<void (std::string name)>;

class CommonEventHandler : public AppExecFwk::EventHandler {
    public:
        CommonEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
        ~CommonEventHandler() override = default;

        bool PostTask(const Callback &callback, const std::string &name = std::string(), int64_t delayTime = 0);
        void RemoveTask(const std::string &name);
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
    int32_t StartTimer(std::string name, int32_t timeOut, TimerCallback callback);

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

private:
    mutable std::mutex timerMutex_;
    std::unordered_set<std::string> timerVec_ = {};
    std::shared_ptr<CommonEventHandler> eventHandler_;
};
}
}
#endif