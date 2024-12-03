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

#include "device_manager_service.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {

const int32_t MAX_DATA_LEN = 65536;

static DataChangeListener dataChangeListener_ = {
    .onDeviceUnBound = HichainListener::onHichainDeviceUnBound,
};

HichainListener::HichainListener()
{
    LOGI("HichainListener constructor start.");
    InitDeviceAuthService();
    deviceGroupManager_ = GetGmInstance();
    if (deviceGroupManager_ == nullptr) {
        LOGE("[HICHAIN]failed to init group manager.");
        return;
    }
    LOGI("HichainListener::constructor success.");
}

HichainListener::~HichainListener()
{
    LOGI("HichainListener::destructor.");
}

void HichainListener::RegisterDataChangeCb()
{
    LOGI("HichainListener::RegisterDataChangeCb start");
    if (deviceGroupManager_ == nullptr) {
        LOGE("deviceGroupManager_ is null!");
        return;
    }
    int32_t ret = deviceGroupManager_->regDataChangeListener(DM_PKG_NAME, &dataChangeListener_);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]regDataChangeListener failed with ret: %{public}d.", ret);
        return;
    }
    LOGI("RegisterDataChangeCb success!");
}

void HichainListener::onHichainDeviceUnBound(const char *peerUdid, const char *groupInfo) 
{
    LOGI("HichainListener::onDeviceUnBound start");
    if (peerUdid == nullptr || groupInfo == nullptr) {
        LOGE("peerUdid or groupInfo is null!");
        return;
    }
    if (strlen(peerUdid) > MAX_DATA_LEN || strlen(groupInfo) > MAX_DATA_LEN) {
        LOGE("peerUdid or groupInfo is invalid");
        return;
    }
    LOGI("peerUdid = %{public}s, groupInfo = %{public}s", peerUdid, groupInfo);
    // 起线程异步调用
    DeviceManagerService::GetInstance().HandleDataChange(peerUdid, groupInfo);
}

} // namespace DistributedHardware
} // namespace OHOS