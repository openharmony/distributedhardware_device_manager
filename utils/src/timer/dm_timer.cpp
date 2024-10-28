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
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t MIN_TIME_OUT = 0;
    const int32_t MAX_TIME_OUT = 300;
    const int32_t MILLISECOND_TO_SECOND = 1000;
}

CommonEventHandler::CommonEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    LOGI("CommonEventHandler constructor");
}

bool CommonEventHandler::PostTask(const Callback &callback, const std::string &name, int64_t delayTime)
{
    return AppExecFwk::EventHandler::PostTask(callback, name, delayTime);
}

void CommonEventHandler::RemoveTask(const std::string &name)
{
    AppExecFwk::EventHandler::RemoveTask(name);
}

DmTimer::DmTimer()
{
    LOGI("DmTimer constructor");
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<CommonEventHandler>(runner);
}

DmTimer::~DmTimer()
{
    LOGI("DmTimer destructor");
    DeleteAll();
}

int32_t DmTimer::StartTimer(std::string name, int32_t timeOut, TimerCallback callback)
{
    if (name.empty() || timeOut <= MIN_TIME_OUT || timeOut > MAX_TIME_OUT || callback == nullptr) {
        LOGI("DmTimer StartTimer input value invalid");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DmTimer StartTimer start name: %{public}s", name.c_str());
    std::lock_guard<std::mutex> locker(timerMutex_);
    timerVec_.insert(name);

    auto taskFunc = [callback, name] () { callback(name); };
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(taskFunc, name, timeOut * MILLISECOND_TO_SECOND);
    }
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
    if (!timerVec_.empty() || timerVec_.find(timerName) == timerVec_.end()) {
        LOGI("DmTimer DeleteTimer is not exist.");
        return ERR_DM_FAILED;
    }
    timerVec_.erase(timerName);
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(timerName);
    }
    return DM_OK;
}

int32_t DmTimer::DeleteAll()
{
    LOGI("DmTimer DeleteAll start");
    std::lock_guard<std::mutex> locker(timerMutex_);
    if (eventHandler_ ==  nullptr) {
        LOGE("DmTimer eventHandle_ is null");
        timerVec_.clear();
        return ERR_DM_FAILED;
    }
    for (auto name : timerVec_) {
        eventHandler_->RemoveTask(name);
    }
    timerVec_.clear();
    return DM_OK;
}
}
}