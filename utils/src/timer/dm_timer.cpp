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

#include "dm_log.h"
#include "dm_timer.h"
#include "dm_error_type.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t MIN_TIME_OUT = 0;
const int32_t MAX_TIME_OUT = 300;
const int64_t MICROSECOND_TO_SECOND = 1000000L;
constexpr const char* TIMER_TASK = "TimerTask";
}

DmTimer::DmTimer()
{
    LOGI("DmTimer constructor");
    if (queue_ != nullptr) {
        LOGI("Timer is already init.");
        return;
    }
    queue_ = std::make_shared<ffrt::queue>(TIMER_TASK);
}

DmTimer::~DmTimer()
{
    LOGI("DmTimer destructor");
    DeleteAll();
}

int32_t DmTimer::StartTimer(std::string name, int32_t timeOut, TimerCallback callback)
{
    if (name.empty() || timeOut < MIN_TIME_OUT || timeOut > MAX_TIME_OUT || callback == nullptr) {
        LOGE("DmTimer StartTimer input value invalid");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(queue_, ERR_DM_POINT_NULL);
    LOGI("DmTimer StartTimer start name: %{public}s", name.c_str());
    std::lock_guard<std::mutex> locker(timerMutex_);

    auto taskFunc = [callback, name] () { callback(name); };
    ffrt::task_handle handle = queue_->submit_h(taskFunc, ffrt::task_attr().delay(timeOut * MICROSECOND_TO_SECOND));
    if (handle == nullptr) {
        LOGE("handle is nullptr.");
        return ERR_DM_FAILED;
    }
    timerVec_[name] = std::move(handle);
    return DM_OK;
}

int32_t DmTimer::DeleteTimer(std::string timerName)
{
    if (timerName.empty()) {
        LOGE("DmTimer DeleteTimer timer is null");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DmTimer DeleteTimer start name: %{public}s", timerName.c_str());
    std::lock_guard<std::mutex> locker(timerMutex_);
    auto item = timerVec_.find(timerName);
    if (item == timerVec_.end()) {
        LOGI("Invalid task.");
        return ERR_DM_FAILED;
    }
    if (item->second != nullptr && queue_ != nullptr) {
        int32_t ret = queue_->cancel(item->second);
        if (ret != 0) {
            LOGE("Cancel failed, errCode: %{public}d.", ret);
        }
    }
    timerVec_.erase(timerName);
    return DM_OK;
}

DM_EXPORT int32_t DmTimer::DeleteAll()
{
    LOGI("DmTimer DeleteAll start");
    std::lock_guard<std::mutex> locker(timerMutex_);
    for (const auto &name : timerVec_) {
        if (name.second != nullptr && queue_ != nullptr) {
            int32_t ret = queue_->cancel(name.second);
            if (ret != 0) {
                LOGE("Cancel failed, errCode: %{public}d.", ret);
            }
        }
    }
    timerVec_.clear();
    return DM_OK;
}
}
}