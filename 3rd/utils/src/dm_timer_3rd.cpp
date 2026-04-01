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

#include "dm_timer_3rd.h"

#include "dm_log_3rd.h"
#include "dm_error_type_3rd.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t MIN_TIME_OUT = 0;
const int32_t MAX_TIME_OUT = 600;
const int64_t MICROSECOND_TO_SECOND = 1000000L;
constexpr const char* TIMER_TASK = "TimerTask";
}

DmTimer3rd::DmTimer3rd()
{
    LOGI("DmTimer3rd constructor");
    if (queue_ != nullptr) {
        LOGI("Timer is already init.");
        return;
    }
    queue_ = std::make_shared<ffrt::queue>(TIMER_TASK);
}

DmTimer3rd::~DmTimer3rd()
{
    LOGI("DmTimer3rd destructor");
    DeleteAll();
}

int32_t DmTimer3rd::StartTimer(const std::string name, int32_t timeOut, TimerCallback callback)
{
    if (name.empty() || timeOut < MIN_TIME_OUT || timeOut > MAX_TIME_OUT || callback == nullptr) {
        LOGE("DmTimer3rd StartTimer input value invalid");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(queue_, ERR_DM_POINT_NULL);
    LOGI("DmTimer3rd StartTimer start name: %{public}s", name.c_str());
    std::lock_guard<ffrt::mutex> locker(timerMutex_);

    auto taskFunc = [callback, name] () { callback(name); };
    ffrt::task_handle handle = queue_->submit_h(taskFunc, ffrt::task_attr().delay(timeOut * MICROSECOND_TO_SECOND));
    if (handle == nullptr) {
        LOGE("handle is nullptr.");
        return ERR_DM_FAILED;
    }
    timerVec_[name] = std::move(handle);
    return DM_OK;
}

int32_t DmTimer3rd::DeleteTimer(const std::string timerName)
{
    if (timerName.empty()) {
        LOGE("DmTimer3rd DeleteTimer timer is null");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DmTimer3rd DeleteTimer start name: %{public}s", timerName.c_str());
    std::lock_guard<ffrt::mutex> locker(timerMutex_);
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

int32_t DmTimer3rd::DeleteAll()
{
    LOGI("DmTimer3rd DeleteAll start");
    std::lock_guard<ffrt::mutex> locker(timerMutex_);
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