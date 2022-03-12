/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dm_timer.h"
#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t MILL_SECONDS_PER_SECOND = 1000;
}
DmTimer::DmTimer(std::string &name)
{
    mStatus_ = DmTimerStatus::DM_STATUS_INIT;
    mTimeOutSec_ = 0;
    mHandle_ = nullptr;
    mHandleData_ = nullptr;
    (void)memset_s(mTimeFd_, sizeof(mTimeFd_), 0, sizeof(mTimeFd_));
    mEpFd_ = 0;
    mTimerName_ = name;
}

DmTimer::~DmTimer()
{
    DMLOG(DM_LOG_INFO, "DmTimer %s Destory in", mTimerName_.c_str());
    Release();
}

DmTimerStatus DmTimer::Start(uint32_t timeOut, TimeoutHandle handle, void *data)
{
    DMLOG(DM_LOG_INFO, "DmTimer %s start timeout(%d)", mTimerName_.c_str(), timeOut);
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

    return mStatus_;
}

void DmTimer::Stop(int32_t code)
{
    DMLOG(DM_LOG_INFO, "DmTimer %s Stop code (%d)", mTimerName_.c_str(), code);
    if (mTimeFd_[1]) {
        char event = 'S';
        if (write(mTimeFd_[1], &event, 1) < 0) {
            DMLOG(DM_LOG_ERROR, "DmTimer %s Stop timer failed, errno %d", mTimerName_.c_str(), errno);
            return;
        }
        DMLOG(DM_LOG_INFO, "DmTimer %s Stop success", mTimerName_.c_str());
    }

    return;
}

void DmTimer::WiteforTimeout()
{
    DMLOG(DM_LOG_INFO, "DmTimer %s start timer at (%d)s", mTimerName_.c_str(), mTimeOutSec_);
    mHandle_(mHandleData_);
    Release();

    DMLOG(DM_LOG_ERROR, "DmTimer %s end timer at (%d)s", mTimerName_.c_str(), mTimeOutSec_);
    return;
}

int32_t DmTimer::CreateTimeFd()
{
    DMLOG(DM_LOG_INFO, "DmTimer %s creatTimeFd", mTimerName_.c_str());
    return 0;
}

void DmTimer::Release()
{
    DMLOG(DM_LOG_INFO, "DmTimer %s Release in", mTimerName_.c_str());
    if (mStatus_ == DmTimerStatus::DM_STATUS_INIT) {
        DMLOG(DM_LOG_INFO, "DmTimer %s already Release", mTimerName_.c_str());
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
}
}
