/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <unordered_map>

#include "ffrt.h"

namespace OHOS {
namespace DistributedHardware {
using TimerCallback = std::function<void (std::string name)>;

class DmTimer {
public:
    __attribute__ ((visibility ("default")))DmTimer();
    __attribute__ ((visibility ("default")))~DmTimer();

    /**
     * @tc.name: DmTimer::StartTimer
     * @tc.desc: start timer running
     * @tc.type: FUNC
     */
    __attribute__ ((visibility ("default")))int32_t StartTimer(std::string name, int32_t timeOut,
        TimerCallback callback);

    /**
     * @tc.name: DmTimer::DeleteTimer
     * @tc.desc: delete timer
     * @tc.type: FUNC
     */
    __attribute__ ((visibility ("default")))int32_t DeleteTimer(std::string timerName);

    /**
     * @tc.name: DmTimer::DeleteAll
     * @tc.desc: delete all timer
     * @tc.type: FUNC
     */
    __attribute__ ((visibility ("default")))int32_t DeleteAll();

private:
    mutable std::mutex timerMutex_;
    std::unordered_map<std::string, ffrt::task_handle> timerVec_ = {};
    std::shared_ptr<ffrt::queue> queue_;
};
}
}
#endif