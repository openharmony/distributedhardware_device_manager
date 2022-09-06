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

#ifndef OHOS_DM_DISCOVERY_MANAGER_H
#define OHOS_DM_DISCOVERY_MANAGER_H

#include <queue>

#include "dm_discovery_filter.h"
#include "idevice_manager_service_listener.h"
#include "dm_timer.h"
#include "softbus_connector.h"
namespace OHOS {
namespace DistributedHardware {
typedef struct DmDiscoveryContext {
    std::string pkgName;
    std::string extra;
    uint16_t subscribeId;
    std::string filterOp;
    std::vector<DmDeviceFilters> filters;
} DmDiscoveryContext;

class DmDiscoveryManager final : public ISoftbusDiscoveryCallback,
                                 public std::enable_shared_from_this<DmDiscoveryManager> {
public:
    DmDiscoveryManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                       std::shared_ptr<IDeviceManagerServiceListener> listener);
    ~DmDiscoveryManager();

    /**
     * @tc.name: DmDiscoveryManager::StartDeviceDiscovery
     * @tc.desc: Start Device Discovery of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                 const std::string &extra);

    /**
     * @tc.name: DmDiscoveryManager::StopDeviceDiscovery
     * @tc.desc: Stop Device Discovery of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId);

    /**
     * @tc.name: DmDiscoveryManager::OnDeviceFound
     * @tc.desc: OnDevice Found of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    void OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDiscoveryManager::OnDiscoverySuccess
     * @tc.desc: OnDiscovery Success of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId);

    /**
     * @tc.name: DmDiscoveryManager::OnDiscoveryFailed
     * @tc.desc: OnDiscovery Failed of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    void OnDiscoveryFailed(const std::string &pkgName, int32_t subscribeId, int32_t failedReason);

    /**
     * @tc.name: DmDiscoveryManager::HandleDiscoveryTimeout
     * @tc.desc: Handle Discovery Timeout of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    void HandleDiscoveryTimeout(std::string name);
private:
    void CfgDiscoveryTimer();
    int32_t CheckDiscoveryQueue(const std::string &pkgName);

private:
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::queue<std::string> discoveryQueue_;
    std::map<std::string, DmDiscoveryContext> discoveryContextMap_;
    std::shared_ptr<DmTimer> timer_;
    std::mutex locks_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DISCOVERY_MANAGER_H
