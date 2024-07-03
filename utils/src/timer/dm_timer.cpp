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

CommonEventHandler::CommonEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    LOGI("CommonEventHandler destructor");
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
    LOGI("DmTimer construct");
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
    LOGI("DmTimer DeleteTimer start name: %{public}s", timerName);
    std::lock_guard<std::mutex> locker(timerMutex_);
    if (!timerVec_.empty() && timerVec_.find(timerName) == timerVec_.end()) {
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