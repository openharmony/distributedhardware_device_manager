/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ADVERTISE_MANAGER_H
#define OHOS_ADVERTISE_MANAGER_H

#include "dm_timer.h"
#include "idevice_manager_service_listener.h"
#include "i_softbus_lnn_ops_callback.h"
#include "softbus_listener.h"

namespace OHOS {
namespace DistributedHardware {
class AdvertiseManager : public ISoftbusLnnOpsCallback, public std::enable_shared_from_this<AdvertiseManager> {
public:
    AdvertiseManager(std::shared_ptr<SoftbusListener> softbusListener,
        std::shared_ptr<IDeviceManagerServiceListener> listener);
    ~AdvertiseManager() override;

    // interfaces from ISoftbusLnnOpsCallback
    void OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info, bool isOnline) override;
    void OnDiscoveringResult(const std::string &pkgName, int32_t subscribeId, int32_t result) override;
    void OnAdvertisingResult(const std::string &pkgName, int32_t publishId, int32_t result) override;

    int32_t StartAdvertising(const std::string &pkgName, const std::map<std::string, std::string> &advertiseParam);
    int32_t StopAdvertising(const std::string &pkgName, int32_t publishId);

private:
    void HandleAutoStopAdvertise(const std::string &timerName, const std::string &pkgName, int32_t publishId);

private:
    std::shared_ptr<DmTimer> timer_;
    std::shared_ptr<SoftbusListener> softbusListener_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ADVERTISE_MANAGER_H
