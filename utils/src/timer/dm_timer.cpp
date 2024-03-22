/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "dm_timer.h"

#include <pthread.h>
#include <thread>

namespace OHOS {
namespace DistributedHardware {

constexpr const char* TIMER_RUNNING = "TimerRunning";
constexpr int32_t ERR_DM_INPUT_PARA_INVALID = -20006;
constexpr int32_t DM_OK = 0;

Timer::Timer(std::string name, int32_t time, TimerCallback callback)
    : timerName_(name), expire_(steadyClock::now()), state_(true), timeOut_(time), callback_(callback) {};

DmTimer::DmTimer()
{
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
    LOGI("DmTimer StartTimer %{public}s", name.c_str());
    if (name.empty() || timeOut <= MIN_TIME_OUT || timeOut > MAX_TIME_OUT || callback == nullptr) {
        LOGI("DmTimer StartTimer input value invalid");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::shared_ptr<Timer> timer = std::make_shared<Timer>(name, timeOut, callback);
    {
        std::lock_guard<std::mutex> locker(timerMutex_);
        timerQueue_.push(timer);
        timerVec_.emplace_back(timer);
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

int32_t DmTimer::DeleteTimer(std::string timerName)
{
    if (timerName.empty()) {
        LOGE("DmTimer DeleteTimer timer is null");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("DmTimer DeleteTimer name %{public}s", timerName.c_str());
    std::lock_guard<std::mutex> locker(timerMutex_);
    for (auto iter : timerVec_) {
        if (iter->timerName_ == timerName) {
            iter->state_ = false;
        }
    }
    return DM_OK;
}

int32_t DmTimer::DeleteAll()
{
    LOGI("DmTimer DeleteAll start");
    std::lock_guard<std::mutex> locker(timerMutex_);
    for (auto iter : timerVec_) {
        LOGI("DmTimer DeleteAll timer.name = %{public}s ", iter->timerName_.c_str());
        iter->state_ = false;
    }
    return DM_OK;
}

int32_t DmTimer::TimerRunning()
{
    int32_t ret = pthread_setname_np(pthread_self(), TIMER_RUNNING);
    if (ret != DM_OK) {
        LOGE("TimerRunning setname failed.");
    }
    std::thread([this] () {
        {
            timerState_ = true;
            std::unique_lock<std::mutex> locker(timerStateMutex_);
            runTimerCondition_.notify_one();
        }
        while (!timerQueue_.empty() && timerState_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_TICK_MILLSECONDS));
            timerMutex_.lock();
            while (!timerQueue_.empty() && (std::chrono::duration_cast<timerDuration>(steadyClock::now() -
                timerQueue_.top()->expire_).count() / MILLISECOND_TO_SECOND >= timerQueue_.top()->timeOut_ ||
                !timerQueue_.top()->state_)) {
                std::string name = timerQueue_.top()->timerName_;
                LOGI("DmTimer TimerRunning timer.name = %{public}s", name.c_str());
                TimerCallback callBack = nullptr;
                if (timerQueue_.top()->state_) {
                    callBack = timerQueue_.top()->callback_;
                }
                timerQueue_.pop();
                DeleteVector(name);
                timerMutex_.unlock();
                if (callBack != nullptr) {
                    callBack(name);
                }
                timerMutex_.lock();
                if (timerQueue_.empty()) {
                    break;
                }
            }
            timerMutex_.unlock();
        }
        {
            timerState_ = false;
            std::unique_lock<std::mutex> locker(timerStateMutex_);
            stopTimerCondition_.notify_one();
        }
    }).detach();
    return DM_OK;
}

void DmTimer::DeleteVector(std::string name)
{
    for (auto iter = timerVec_.begin(); iter != timerVec_.end(); ++iter) {
        if ((*iter)->timerName_ == name) {
            timerVec_.erase(iter);
            break;
        }
    }
}
}
}