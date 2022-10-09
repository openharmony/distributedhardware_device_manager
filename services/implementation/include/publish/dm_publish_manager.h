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

#ifndef OHOS_DM_PUBLISH_MANAGER_H
#define OHOS_DM_PUBLISH_MANAGER_H

#include <queue>

#include "dm_publish_info.h"
#include "device_manager_service_listener.h"
#include "dm_timer.h"
#include "softbus_connector.h"
#include "softbus_publish_callback.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct DmPublishContext {
    std::string pkgName;
    DmPublishInfo publishInfo;
} DmPublishContext;

class DmPublishManager final : public ISoftbusPublishCallback,
                               public std::enable_shared_from_this<DmPublishManager> {
public:
    DmPublishManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                     std::shared_ptr<IDeviceManagerServiceListener> listener);
    ~DmPublishManager();

    /**
     * @tc.name: DmPublishManager::PublishDeviceDiscovery
     * @tc.desc: Publish Device Discovery of the Dm Publish Manager
     * @tc.type: FUNC
     */
    int32_t PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo);

    /**
     * @tc.name: DmPublishManager::UnPublishDeviceDiscovery
     * @tc.desc: UnPublish Device Discovery of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    int32_t UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId);

    /**
     * @tc.name: DmPublishManager::OnPublishResult
     * @tc.desc: OnPublish Result of the Dm Discovery Manager
     * @tc.type: FUNC
     */
    void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult);

    /**
     * @tc.name: DmPublishManager::HandlePublishDiscoveryTimeout
     * @tc.desc: Handle Publish Timeout of the Dm OnPublish Manager
     * @tc.type: FUNC
     */
    void HandlePublishTimeout(std::string name);
private:
    void CfgPublishTimer();
    int32_t CheckPublishQueue(const std::string &pkgName);

private:
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::queue<std::string> publishQueue_;
    std::map<std::string, DmPublishContext> publishContextMap_;
    std::shared_ptr<DmTimer> timer_;
    std::mutex locks_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PUBLISH_MANAGER_H
