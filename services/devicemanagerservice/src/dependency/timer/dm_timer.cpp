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

#include "dm_constants.h"
#include "dm_log.h"
#include "dm_timer.h"

#include <algorithm>
#include <thread>

namespace OHOS {
namespace DistributedHardware {
bool Compare(const Timer &frontTimer, const Timer &timer)
{
    return frontTimer.timeOut < timer.timeOut;
}

DmTimer::DmTimer()
{
    LOGI("DmTimer constructor");
}

DmTimer::~DmTimer()
{
    LOGI("DmTimer destructor");
    DeleteAll();
    if (timerState_) {
        std::unique_lock<std::mutex> locker(timerStateMutex_);
        stopTimerCondition_.wait(locker, [this] { return static_cast<bool>(!timerState_); });
    }
}

int32_t DmTimer::StartTimer(std::string name, int32_t timeOut, TimerCallback callback)
{
    LOGI("DmTimer StartTimer %s", name.c_str());
    if (name.empty() || timeOut <= MIN_TIME_OUT || timeOut > MAX_TIME_OUT || callback == nullptr) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    Timer timer = {
        .timerName = name,
        .expire = steadyClock::now(),
        .state = true,
        .timeOut = timeOut,
        .callback = callback
    };
    {
        std::lock_guard<std::mutex> locker(timerMutex_);
        for (auto iter : timerHeap_) {
            iter.timeOut = std::chrono::duration_cast<timerDuration>(steadyClock::now()
                - timerHeap_.front().expire).count() / MILLISECOND_TO_SECOND;
            iter.expire = steadyClock::now();
        }
        timerHeap_.push_back(timer);
        sort(timerHeap_.begin(), timerHeap_.end(), Compare);
    }

    if (timerState_) {
        LOGI("DmTimer is running");
        return DM_OK;
    }

    TimerRunning();
    {
        std::unique_lock<std::mutex> locker(timerStateMutex_);
        runTimerCondition_.wait(locker, [this] { return static_cast<bool>(timerState_); });
    }
    return DM_OK;
}

int32_t DmTimer::DeleteTimer(std::string name)
{
    LOGI("DmTimer DeleteTimer size %d", timerHeap_.size());
    if (name.empty()) {
        LOGE("DmTimer DeleteTimer timer name is null");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    auto iter = std::find(timerHeap_.begin(), timerHeap_.end(), Timer {name, });
    if (iter == timerHeap_.end()) {
        LOGE("DmTimer DeleteTimer is not this %s timer,", name.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::lock_guard<std::mutex> locker(timerMutex_);
    iter->state = false;
    return DM_OK;
}

int32_t DmTimer::DeleteAll()
{
    LOGI("DmTimer DeleteAll");
    if (timerHeap_.size() > 0) {
        std::lock_guard<std::mutex> locker(timerMutex_);
        for (auto iter : timerHeap_) {
            iter.state = false;
        }
    }
    return DM_OK;
}

int32_t DmTimer::TimerRunning()
{
    std::thread([this] () {
        {
            timerState_ = true;
            std::unique_lock<std::mutex> locker(timerStateMutex_);
            runTimerCondition_.notify_one();
        }
        while (timerHeap_.size() != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_TICK_MILLSECONDS));
            while (std::chrono::duration_cast<timerDuration>(steadyClock::now()
                   - timerHeap_.front().expire).count() / MILLISECOND_TO_SECOND >= timerHeap_.front().timeOut
                   || !timerHeap_.front().state) {
                std::string name = timerHeap_.front().timerName;
                if (timerHeap_.front().state) {
                    timerHeap_.front().callback(name);
                }

                auto iter = std::find(timerHeap_.begin(), timerHeap_.end(), Timer {name, });
                if (iter != timerHeap_.end()) {
                    std::lock_guard<std::mutex> locker(timerMutex_);
                    timerHeap_.erase(timerHeap_.begin(), timerHeap_.begin()+1);
                    sort(timerHeap_.begin(), timerHeap_.end(), Compare);
                }

                if (timerHeap_.size() == 0) {
                    break;
                }
            }
        }
        {
            timerState_ = false;
            std::unique_lock<std::mutex> locker(timerStateMutex_);
            stopTimerCondition_.notify_one();
        }
    }).detach();
    return DM_OK;
}
}
}