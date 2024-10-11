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

#include "settings_data_event_monitor.h"

#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
SettingsDataEventMonitor::SettingsDataEventMonitor(std::shared_ptr<LocalDeviceNameMgr> localDeviceNameMgr,
    SettingsDataMonitorType monitorType)
    : localDeviceNameMgrWPtr_(localDeviceNameMgr), monitorType_(monitorType)
{
    LOGI("Ctor SettingsDataEventMonitor, monitorType: %{public}d", (int32_t)monitorType);
}

void SettingsDataEventMonitor::OnChange()
{
    if (localDeviceNameMgrWPtr_.expired()) {
        LOGE("localDeviceNameMgrWPtr_ expired");
        return;
    }
    LOGI("Settings OnChange type: %{public}d", (int32_t)monitorType_);
    std::shared_ptr<LocalDeviceNameMgr> localDevNameSPtr = localDeviceNameMgrWPtr_.lock();
    switch (monitorType_) {
        case SettingsDataMonitorType::USER_DEFINED_DEVICE_NAME_MONITOR:
            localDevNameSPtr->QueryLocalDeviceName();
            break;
        case SettingsDataMonitorType::DISPLAY_DEVICE_NAME_MONITOR:
            localDevNameSPtr->QueryLocalDisplayName();
            break;
        default:
            LOGE("unknwon monitor type");
            break;
    }
}
} // DistributedHardware
} // OHOS