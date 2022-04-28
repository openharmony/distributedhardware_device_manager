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

namespace OHOS {
namespace DistributedHardware {
int32_t TimeHeap::Tick()
{
    LOGI("Tick start");
    if (hsize_ == 0) {
        LOGE("Timer count is 0");
        return DM_AUTH_NO_TIMER;
    }

    std::shared_ptr<DmTimer> top = minHeap_.front();
    top->isTrigger = true;
    do {
        top->mHandle_(top->userData_, top->timerName_);
        DelTimer(top->timerName_);

        if (hsize_ > 0) {
            top = minHeap_.front();
        } else {
            break;
        }
    } while (top->expire_ <= time(NULL));

    return DM_OK;
}

int32_t TimeHeap::MoveUp(std::shared_ptr<DmTimer> timer)
{
    LOGI("MoveUp timer");
    if (timer == nullptr) {
        LOGE("MoveUp timer is null");
        return DM_INVALID_VALUE;
    }

    if (hsize_ == 0) {
        LOGE("Add timer failed");
        return DM_INVALID_VALUE;
    }

    for (int32_t i = 1; i++) {
        LOGE("MoveUp 1 = %d, h = %d", i, hsize_);
        if (i == hsize_) {
            minHeap_.insert(minHeap_.begin() + (i - 1), timer);
            break;
        }
        if (timer->expire_ < minHeap_[i - 1]->expire_) {
            minHeap_.insert(minHeap_.begin() + (i -1), timer);
            break;
        }
    }
    return DM_OK;
}

void TimeHeap::Run()
{
    epoll_event event;
    event.data.fd = pipefd[0];
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, pipefd[0], &event);
    epoll_event events[MAX_EVENT_NUMBER];
    bool stop = false;
    int timeout = NO_TIMER;

    while (!stop) {
        int number = epoll_wait(epollFd_, events, MAX_EVENT_NUMBER, timeout);

        LOGI("RunTimer is doing");
        if (number < 0) {
            LOGE("DmTimer %s epoll_wait error: %d", minHeap_.front()->timerName_.c_str(), errno);
            DelTimer(minHeap_.front()->timerName_);
        }
        if (!number) {
            Tick();
        } else {
            int buffer = 0;
            recv(pipefd[0], (char*)&buffer, sizeof(buffer), 0);
        }

        if (hsize_ == 0) {
            break;
        } else {
            timeout = (minHeap_.front()->expire_ - time(NULL)) * SEC_TO_MM;
        }
    }
}

TimeHeap::TimeHeap(): epollFd_(epoll_create(MAX_EVENTS))
{
    minHeap_.resize(INIT_SIZE);

    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    if (ret != 0) {
        LOGE("open pipe failed");
    }
    assert(ret == 0);
}

TimeHeap::~TimeHeap()
{
    DelAll();
    close(epollFd_);
}

int32_t TimeHeap::AddTimer(std::string name, int timeout, TimeoutHandle mHandle, void *user)
{
    if (name.empty() || name.find(TIMER_PREFIX) != TIMER_DEFAULT) {
        LOGE("DmTimer name is not DM timer");
        return DM_INVALID_VALUE;
    }

    LOGI("AddTimer %s", name.c_str());
    if (timeout <= 0 || mHandle == nullptr || user == nullptr) {
        LOGE("DmTimer %s invalid value", name.c_str());
        return DM_INVALID_VALUE;
    }

    if (hsize_ == 0) {
        mThread_ = std::thread(&TimeHeap::Run, this);
        mThread_.detach();
    }
    if (hsize_ == (int32_t)(minHeap_.size() - 1)) {
        minHeap_.resize(EXPAND_TWICE * minHeap_.size());
    }

    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>(name, timeout + time(NULL), user, mHandle);
    ++hsize_;
    MoveUp(timer);
    if (timer == minHeap_.front()) {
        char msg = 1;
        send(pipefd[1], (char*)&msg, sizeof(msg), 0);
    }
    LOGE("AddTimer %s complete", name.c_str());
    return DM_OK;
}

int32_t TimeHeap::DelTimer(std::string name)
{
    if (name.empty() || name.find(TIMER_PREFIX) != TIMER_DEFAULT) {
        LOGE("DmTimer name is not DM timer");
        return DM_INVALID_VALUE;
    }

    LOGI("DelTimer %s", name.c_str());
    int32_t location = 0;
    bool have = false;
    for (int32_t i = 0; i < hsize_; i++) {
        if (minHeap_[i]->timerName_ == name) {
            location = i;
            have = true;
            break;
        }
    }

    if (!have) {
        LOGE("heap is not have this %s", name.c_str());
        return DM_INVALID_VALUE;
    }

    if (minHeap_[location] == minHeap_.front() && minHeap_[location]->isTrigger == false) {
        char msg = 1;
        send(pipefd[1], &msg, sizeof(msg), 0);
    }
    minHeap_.erase(minHeap_.begin() + location);
    hsize_--;
    LOGI("DelTimer %s complete , timer count %d", name.c_str(), hsize_);
    return DM_OK;
}

int32_t TimeHeap::DelAll()
{
    LOGI("DelAll start");
    for (int32_t i = hsize_ ; i > 0; i--) {
        DelTimer(minHeap_[i - 1]->timerName_);
    }
    LOGI("DelAll complete");
    return DM_OK;
}
}
}