/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_SETTINGS_DATA_EVENT_MONITOR_H
#define OHOS_SETTINGS_DATA_EVENT_MONITOR_H

#include "data_ability_observer_stub.h"
#include "local_device_name_mgr.h"

namespace OHOS {
namespace DistributedHardware {
class LocalDeviceNameMgr;
enum class SettingsDataMonitorType : int32_t {
    USER_DEFINED_DEVICE_NAME_MONITOR = 0,
    DISPLAY_DEVICE_NAME_MONITOR = 1
};

class SettingsDataEventMonitor : public AAFwk::DataAbilityObserverStub {
public:
    SettingsDataEventMonitor(std::shared_ptr<LocalDeviceNameMgr> localDeviceNameMgr,
        SettingsDataMonitorType monitorType);
    void OnChange() override;
private:
    std::weak_ptr<LocalDeviceNameMgr> localDeviceNameMgrWPtr_;
    SettingsDataMonitorType monitorType_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_SETTINGS_DATA_EVENT_MONITOR_H