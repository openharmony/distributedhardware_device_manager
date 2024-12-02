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

#include "hichain_listener.h"

#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {

static DataChangeListener dataChangeListener_ = {
    .onDeviceUnBound = HichainListener::onHichainDeviceUnBound,
};

HichainListener::HichainListener()
{
    LOGI("HichainListener constructor.");
}

HichainListener::~HichainListener()
{
    
}

HichainListener::RegisterDataChangeCb()
{
    LOGI("HichainListener::RegisterDataChangeCb start");
    int32_t ret = regDataChangeListener(DM_PKG_NAME, &dataChangeListener_);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]RegNodeDeviceStateCb failed with ret: %{public}d, retryTimes: %{public}u.", ret, retryTimes);
        return;
    }
    LOGI("RegisterDataChangeCb success!");
}

HichainListener::onHichainDeviceUnBound(const char *peerUdid, const char *groupInfo) 
{
    // hichain回调
    LOGI("HichainListener::onDeviceUnBound start");
    LOGI("peerUdid = %{public}s, groupInfo = %{public}s", peerUdid, groupInfo);
    // 起线程异步调用
    DeviceManagerService::GetInstance().HandleDataChange(peerUdid, groupInfo);
}

} // namespace DistributedHardware
} // namespace OHOS