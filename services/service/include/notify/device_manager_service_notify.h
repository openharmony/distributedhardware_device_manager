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

#ifndef OHOS_DM_SERVICE_NOTIFY_H
#define OHOS_DM_SERVICE_NOTIFY_H

#include <mutex>
#include <set>

#include "dm_device_info.h"
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceNotify {
    DM_DECLARE_SINGLE_INSTANCE(DeviceManagerServiceNotify);

public:
    int32_t RegisterCallBack(int32_t dmCommonNotifyEvent, const ProcessInfo &processInfo);
    void GetCallBack(int32_t dmCommonNotifyEvent, std::set<ProcessInfo> &processInfos);
    void ClearDiedProcessCallback(const ProcessInfo &processInfo);

private:
    std::mutex callbackLock_;
    std::map<DmCommonNotifyEvent, std::set<ProcessInfo>> callbackMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_NOTIFY_H
