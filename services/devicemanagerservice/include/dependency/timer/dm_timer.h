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

#ifndef TIMER_H
#define TIMER_H

#include <assert.h>
#include <errno.h>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
#include <unistd.h>
namespace OHOS {
namespace DistributedHardware {
typedef void (*TimeoutHandle)(void *data, std::string timerName);
class DmTimer {
public:
    DmTimer(std::string name, time_t expire, void *user, TimeoutHandle mHandle)
        :userData_(user), timerName_(name), expire_(expire), mHandle_(mHandle) {};
public:
    void *userData_;
    bool isTrigger = false;
    std::string timerName_;
    time_t expire_;
    TimeoutHandle mHandle_;;
};

class TimeHeap {
public:
    TimeHeap();
    ~TimeHeap();
    /**
     * @tc.name: TimeHeap::AddTimer
     * @tc.desc: Add timer to time heap
     * @tc.type: FUNC
     */
    int32_t AddTimer(std::string name, int timeout, TimeoutHandle mHandle, void *user);

    /**
     * @tc.name: TimeHeap::DelTimer
     * @tc.desc: Delete timer of the time heap
     * @tc.type: FUNC
     */
    int32_t DelTimer(std::string name);

    /**
     * @tc.name: TimeHeap::DelAll
     * @tc.desc: Delete all timer of the time heap
     * @tc.type: FUNC
     */
    int32_t DelAll();

private:
    /**
     * @tc.name: TimeHeap::Run
     * @tc.desc: timer wait for timeout
     * @tc.type: FUNC
     */
    void Run();

    /**
     * @tc.name: TimeHeap::Run
     * @tc.desc: timerout event triggering
     * @tc.type: FUNC
     */
    int32_t Tick();

    /**
     * @tc.name: TimeHeap::Run
     * @tc.desc: sort the time heap
     * @tc.type: FUNC
     */
    int32_t MoveUp(std::shared_ptr<DmTimer> timer);
private:
    int32_t hsize_ = 0;
    int32_t epollFd_;
    int32_t pipefd[2];
    std::thread mThread_;
    std::vector<std::shared_ptr<DmTimer>> minHeap_;
};
}
}
#endif