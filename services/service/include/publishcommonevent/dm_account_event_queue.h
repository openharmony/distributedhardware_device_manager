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

#ifndef OHOS_DM_ACCOUNT_EVENT_QUEUE_H
#define OHOS_DM_ACCOUNT_EVENT_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <string>
#include <memory>
#include "dm_account_common_event.h"
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {

class DmAccountEventQueue {
    DM_DECLARE_SINGLE_INSTANCE(DmAccountEventQueue);
public:
    void Start();
    void Stop();
    void PushEvent(const DmAccountEventInfo& eventInfo);
    void SetCallback(const AccountEventCallback& callback);

private:
    void ProcessLoop();
    void ProcessEvent(const DmAccountEventInfo& eventInfo);
    void SendAclChangedBroadcast(int32_t userId);

    std::queue<DmAccountEventInfo> eventQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCond_;
    std::thread processThread_;
    bool isRunning_ = false;
    AccountEventCallback callback_;
};

struct DmSubProfileAccountCache {
    int32_t userId;
    int32_t subProfileId;
    std::string accountId;
};

using DmSubProfileEventInfo = DmAccountEventInfo;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_ACCOUNT_EVENT_QUEUE_H