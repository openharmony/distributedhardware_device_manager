/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "dm_account_event_queue.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "device_manager_service.h"
#include "deviceprofile_connector.h"
#include "multiple_user_connector.h"
#include "parameter.h"
#include "relationship_sync_mgr.h"

namespace OHOS {
namespace DistributedHardware {

constexpr const char* ACCOUNT_EVENT_QUEUE_THREAD = "account_event_queue";

IMPLEMENT_SINGLE_INSTANCE(DmAccountEventQueue);

void DmAccountEventQueue::SetCallback(const AccountEventCallback& callback)
{
    callback_ = callback;
}

void DmAccountEventQueue::Start()
{
    if (isRunning_) {
        LOGI("Account event queue is already running.");
        return;
    }
    isRunning_ = true;
    processThread_ = std::thread(&DmAccountEventQueue::ProcessLoop, this);
    if (pthread_setname_np(processThread_.native_handle(), ACCOUNT_EVENT_QUEUE_THREAD) != DM_OK) {
        LOGE("Failed to set thread name.");
    }
    LOGI("Account event queue started.");
}

void DmAccountEventQueue::Stop()
{
    if (!isRunning_) {
        LOGI("Account event queue is not running.");
        return;
    }
    isRunning_ = false;
    queueCond_.notify_all();
    if (processThread_.joinable()) {
        processThread_.join();
    }
    LOGI("Account event queue stopped.");
}

void DmAccountEventQueue::PushEvent(const DmAccountEventInfo& eventInfo)
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    eventQueue_.push(eventInfo);
    queueCond_.notify_one();
    LOGI("Pushed event: %{public}s, userId: %{public}d", eventInfo.eventName.c_str(), eventInfo.userId);
}

void DmAccountEventQueue::ProcessLoop()
{
    while (isRunning_) {
        DmAccountEventInfo eventInfo;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCond_.wait(lock, [this]() { return !eventQueue_.empty() || !isRunning_; });
            if (!isRunning_ && eventQueue_.empty()) {
                break;
            }
            if (eventQueue_.empty()) {
                continue;
            }
            eventInfo = eventQueue_.front();
            eventQueue_.pop();
        }
        ProcessEvent(eventInfo);
    }
}

void DmAccountEventQueue::ProcessEvent(const DmAccountEventInfo& eventInfo)
{
    LOGI("Processing event: %{public}s, userId: %{public}d, subProfileId: %{public}d",
        eventInfo.eventName.c_str(), eventInfo.userId, eventInfo.subProfileId);

    if (callback_) {
        callback_(eventInfo);
        return;
    }
}
} // namespace DistributedHardware
} // namespace OHOS