/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dm_thread_manager.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(ThreadManager);
constexpr const char* DEVICE_MANAGER_THREAD_QUEUE = "device_manager_thread_queue";
ffrt::task_handle ThreadManager::Submit(const char* threadName, const std::function<void()>& threadFunc)
{
    if (ffrtQueue_ == nullptr) {
        ffrtQueue_ = std::make_shared<ffrt::queue>(DEVICE_MANAGER_THREAD_QUEUE,
            ffrt::queue_attr().qos(ffrt::qos_default));
    }
    return ffrtQueue_->submit_h(threadFunc, ffrt::task_attr().name(threadName));
}

void ThreadManager::Wait(const ffrt::task_handle& handle)
{
    if (ffrtQueue_ == nullptr) {
        LOGE("ThreadManager Wait, ffrtQueue is null.");
        return;
    }
    ffrtQueue_->wait(handle);
}
} // namespace DistributedHardware
} // namespace OHOS
