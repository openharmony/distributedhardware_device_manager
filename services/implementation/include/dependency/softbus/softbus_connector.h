/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SOFTBUS_CONNECTOR_H
#define OHOS_DM_SOFTBUS_CONNECTOR_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "softbus_discovery_callback.h"
#include "softbus_publish_callback.h"
#include "softbus_session.h"
#include "softbus_state_callback.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusConnector {
public:
    /**
     * @tc.name: SoftbusConnector::OnSoftbusPublishResult
     * @tc.desc: OnSoftbusPublishResult of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusPublishResult(int publishId, PublishResult result);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusDeviceFound
     * @tc.desc: OnSoftbus DeviceFound of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusDeviceFound(const DeviceInfo *device);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusDiscoveryResult
     * @tc.desc: OnSoftbus Discovery Result of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusDiscoveryResult(int subscribeId, RefreshResult result);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusJoinLNNResult
     * @tc.desc: OnSoftbus JoinLNN Result of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusJoinLNNResult(ConnectionAddr *addr, const char *networkId, int32_t result);

    /**
     * @tc.name: SoftbusConnector::OnParameterChgCallback
     * @tc.desc: OnParameter Chg Callback of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnParameterChgCallback(const char *key, const char *value, void *context);

    /**
     * @tc.name: SoftbusConnector::GetConnectionIpAddress
     * @tc.desc: Get Connection Ip Address of the SoftbusConnector
     * @tc.type: FUNC
     */
    static int32_t GetConnectionIpAddress(const std::string &deviceId, std::string &ipAddress);

    /**
     * @tc.name: SoftbusConnector::GetConnectAddr
     * @tc.desc: Get Connect Addr of the SoftbusConnector
     * @tc.type: FUNC
     */
    static ConnectionAddr *GetConnectAddr(const std::string &deviceId, std::string &connectAddr);

    /**
     * @tc.name: SoftbusConnector::IsDeviceOnLine
     * @tc.desc: Judge Device OnLine of the SoftbusConnector
     * @tc.type: FUNC
     */
    static bool IsDeviceOnLine(const std::string &deviceId);

    /**
     * @tc.name: SoftbusConnector::GetUdidByNetworkId
     * @tc.desc: Get Udid By NetworkId of the SoftbusConnector
     * @tc.type: FUNC
     */
    static int32_t GetUdidByNetworkId(const char *networkId, std::string &udid);

    /**
     * @tc.name: SoftbusConnector::GetUuidByNetworkId
     * @tc.desc: Get Uuid By NetworkId of the SoftbusConnector
     * @tc.type: FUNC
     */
    static int32_t GetUuidByNetworkId(const char *networkId, std::string &uuid);

    /**
     * @tc.name: SoftbusConnector::JoinLnn
     * @tc.desc: Add the current device to the LNN where a specified device resides
     * @tc.type: FUNC
     */
    static void JoinLnn(const std::string &deviceId);
public:
    SoftbusConnector();
    ~SoftbusConnector();
    int32_t RegisterSoftbusStateCallback(const std::string &pkgName,
                                         const std::shared_ptr<ISoftbusStateCallback> callback);
    int32_t UnRegisterSoftbusStateCallback(const std::string &pkgName);
    int32_t RegisterSoftbusDiscoveryCallback(const std::string &pkgName,
                                             const std::shared_ptr<ISoftbusDiscoveryCallback> callback);
    int32_t UnRegisterSoftbusDiscoveryCallback(const std::string &pkgName);
    int32_t RegisterSoftbusPublishCallback(const std::string &pkgName,
                                           const std::shared_ptr<ISoftbusPublishCallback> callback);
    int32_t UnRegisterSoftbusPublishCallback(const std::string &pkgName);
    int32_t PublishDiscovery(const DmPublishInfo &dmPublishInfo);
    int32_t UnPublishDiscovery(int32_t publishId);
    int32_t StartDiscovery(const DmSubscribeInfo &subscribeInfo);
    int32_t StopDiscovery(uint16_t subscribeId);
    std::shared_ptr<SoftbusSession> GetSoftbusSession();
    bool HaveDeviceInMap(std::string deviceId);
    void HandleDeviceOnline(const DmDeviceInfo &info);
    void HandleDeviceOffline(const DmDeviceInfo &info);

private:
    int32_t Init();
    static void ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceInfo &dmDeviceInfo);
    static ConnectionAddr *GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type);

private:
    enum PulishStatus {
        STATUS_UNKNOWN = 0,
        ALLOW_BE_DISCOVERY = 1,
        NOT_ALLOW_BE_DISCOVERY = 2,
    };
    static PulishStatus publishStatus;
    static IRefreshCallback softbusDiscoveryCallback_;
    static IPublishCb softbusPublishCallback_;
    std::shared_ptr<SoftbusSession> softbusSession_;
    static std::map<std::string, std::shared_ptr<DeviceInfo>> discoveryDeviceInfoMap_;
    static std::map<std::string, std::shared_ptr<ISoftbusStateCallback>> stateCallbackMap_;
    static std::map<std::string, std::shared_ptr<ISoftbusDiscoveryCallback>> discoveryCallbackMap_;
    static std::map<std::string, std::shared_ptr<ISoftbusPublishCallback>> publishCallbackMap_;
    static std::mutex discoveryDeviceInfoMutex_;
    static std::mutex stateCallbackMutex_;
    static std::mutex discoveryCallbackMutex_;
    static std::mutex publishCallbackMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_CONNECTOR_H
