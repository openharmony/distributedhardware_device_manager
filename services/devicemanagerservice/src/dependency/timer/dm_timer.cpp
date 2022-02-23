/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dm_timer.h"

#include <thread>

#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t MILL_SECONDS_PER_SECOND = 1000;
}
DmTimer::DmTimer(const std::string &name)
{
    if (name.empty()) {
        LOGI("DmTimer name is null");
        return;
    }
    mStatus_ = DmTimerStatus::DM_STATUS_INIT;
    mTimeOutSec_ = 0;
    mHandle_ = nullptr;
    mHandleData_ = nullptr;
    (void)memset_s(mTimeFd_, sizeof(mTimeFd_), 0, sizeof(mTimeFd_));
    (void)memset_s(&mEv_, sizeof(mEv_), 0, sizeof(mEv_));
    (void)memset_s(mEvents_, sizeof(mEvents_), 0, sizeof(mEvents_));
    mEpFd_ = 0;
    mTimerName_ = name;
}

DmTimer::~DmTimer()
{
    if (mTimerName_.empty()) {
        LOGI("DmTimer is not init");
        return;
    }
    LOGI("DmTimer %s Destroy in", mTimerName_.c_str());
    Release();
}

DmTimerStatus DmTimer::Start(uint32_t timeOut, TimeoutHandle handle, void *data)
{
    if (mTimerName_.empty() || handle == nullptr || data == nullptr) {
        LOGI("DmTimer is not init or param empty");
        return DmTimerStatus::DM_STATUS_FINISH;
    }
    LOGI("DmTimer %s start timeout(%d)", mTimerName_.c_str(), timeOut);
    if (mStatus_ != DmTimerStatus::DM_STATUS_INIT) {
        return DmTimerStatus::DM_STATUS_BUSY;
    }

    mTimeOutSec_ = timeOut;
    mHandle_ = handle;
    mHandleData_ = data;

    if (CreateTimeFd()) {
        return DmTimerStatus::DM_STATUS_CREATE_ERROR;
    }

    mStatus_ = DmTimerStatus::DM_STATUS_RUNNING;
    mThread_ = std::thread(&DmTimer::WaitForTimeout, this);
    mThread_.detach();

    return mStatus_;
}

void DmTimer::Stop(int32_t code)
{
    if (mTimerName_.empty()) {
        LOGI("DmTimer is not init");
        return;
    }
    LOGI("DmTimer %s Stop code (%d)", mTimerName_.c_str(), code);
    if (mTimeFd_[1]) {
        char event = 'S';
        if (write(mTimeFd_[1], &event, 1) < 0) {
            LOGE("DmTimer %s Stop timer failed, errno %d", mTimerName_.c_str(), errno);
            return;
        }
        LOGI("DmTimer %s Stop success", mTimerName_.c_str());
    }

    return;
}

void DmTimer::WaitForTimeout()
{
    if (mTimerName_.empty()) {
        LOGI("DmTimer is not init");
        return;
    }
    LOGI("DmTimer %s start timer at (%d)s", mTimerName_.c_str(), mTimeOutSec_);

    int32_t nfds = epoll_wait(mEpFd_, mEvents_, MAX_EVENTS, mTimeOutSec_ * MILL_SECONDS_PER_SECOND);
    if (nfds < 0) {
        LOGE("DmTimer %s epoll_wait returned n=%d, error: %d", mTimerName_.c_str(), nfds, errno);
        if (errno == EINTR) {
            LOGI("DmTimer is stop");
            return;
        }
    }

    char event = 0;
    if (nfds > 0) {
        if (mEvents_[0].events & EPOLLIN) {
            int num = read(mTimeFd_[0], &event, 1);
            if (num > 0) {
                LOGI("DmTimer %s exit with event %d", mTimerName_.c_str(), event);
            } else {
                LOGE("DmTimer %s exit with errno %d", mTimerName_.c_str(), errno);
            }
        }
        Release();
        return;
    }

    mHandle_(mHandleData_, *this);
    Release();

    LOGE("DmTimer %s end timer at (%d)s", mTimerName_.c_str(), mTimeOutSec_);
    return;
}

int32_t DmTimer::CreateTimeFd()
{
    if (mTimerName_.empty()) {
        LOGI("DmTimer is not init");
        return DM_STATUS_FINISH;
    }
    LOGI("DmTimer %s creatTimeFd", mTimerName_.c_str());
    int ret = 0;

    ret = pipe(mTimeFd_);
    if (ret < 0) {
        LOGE("DmTimer %s CreateTimeFd fail:(%d) errno(%d)", mTimerName_.c_str(), ret, errno);
        return ret;
    }

    mEv_.data.fd = mTimeFd_[0];
    mEv_.events = EPOLLIN | EPOLLET;
    mEpFd_ = epoll_create(MAX_EVENTS);
    ret = epoll_ctl(mEpFd_, EPOLL_CTL_ADD, mTimeFd_[0], &mEv_);
    if (ret != 0) {
        Release();
    }

    return ret;
}

void DmTimer::Release()
{
    if (mTimerName_.empty()) {
        LOGI("DmTimer is not init");
        return;
    }
    LOGI("DmTimer %s Release in", mTimerName_.c_str());
    if (mStatus_ == DmTimerStatus::DM_STATUS_INIT) {
        LOGI("DmTimer %s already Release", mTimerName_.c_str());
        return;
    }
    mStatus_ = DmTimerStatus::DM_STATUS_INIT;
    close(mTimeFd_[0]);
    close(mTimeFd_[1]);
    if (mEpFd_ >= 0) {
        close(mEpFd_);
    }
    mTimeFd_[0] = 0;
    mTimeFd_[1] = 0;
    mEpFd_ = 0;
}

std::string DmTimer::GetTimerName()
{
    return mTimerName_;
}
} // namespace DistributedHardware
} // namespace OHOS
