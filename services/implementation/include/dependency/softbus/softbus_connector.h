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

#ifndef OHOS_DM_SOFTBUS_CONNECTOR_H
#define OHOS_DM_SOFTBUS_CONNECTOR_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "discovery_service.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "softbus_bus_center.h"
#include "softbus_discovery_callback.h"
#include "softbus_session.h"
#include "softbus_state_callback.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusConnector {
public:
    /**
     * @tc.name: SoftbusConnector::OnPublishSuccess
     * @tc.desc: OnPublishSuccess of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnPublishSuccess(int publishId);

    /**
     * @tc.name: SoftbusConnector::OnPublishFail
     * @tc.desc: OnPublishFail of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnPublishFail(int publishId, PublishFailReason reason);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusDeviceFound
     * @tc.desc: OnSoftbus DeviceFound of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusDeviceFound(const DeviceInfo *device);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusDiscoveryFailed
     * @tc.desc: OnSoftbus Discovery Failed of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusDiscoveryFailed(int subscribeId, DiscoveryFailReason failReason);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusDiscoverySuccess
     * @tc.desc: OnSoftbus Discovery Success of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusDiscoverySuccess(int subscribeId);

    /**
     * @tc.name: SoftbusConnector::OnPublishSuccess
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

public:
    SoftbusConnector();
    ~SoftbusConnector();
    int32_t RegisterSoftbusStateCallback(const std::string &pkgName,
                                         const std::shared_ptr<ISoftbusStateCallback> callback);
    int32_t UnRegisterSoftbusStateCallback(const std::string &pkgName);
    int32_t RegisterSoftbusDiscoveryCallback(const std::string &pkgName,
                                             const std::shared_ptr<ISoftbusDiscoveryCallback> callback);
    int32_t UnRegisterSoftbusDiscoveryCallback(const std::string &pkgName);
    int32_t StartDiscovery(const DmSubscribeInfo &subscribeInfo);
    int32_t StopDiscovery(uint16_t subscribeId);
    std::shared_ptr<SoftbusSession> GetSoftbusSession();
    bool HaveDeviceInMap(std::string deviceId);
    void HandleDeviceOnline(const DmDeviceInfo &info);
    void HandleDeviceOffline(const DmDeviceInfo &info);

private:
    int32_t Init();
    static void CovertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceInfo &dmDeviceInfo);
    static ConnectionAddr *GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type);

private:
    enum PulishStatus {
        STATUS_UNKNOWN = 0,
        ALLOW_BE_DISCOVERY = 1,
        NOT_ALLOW_BE_DISCOVERY = 2,
    };
    static PulishStatus publishStatus;
    static IDiscoveryCallback softbusDiscoveryCallback_;
    static IPublishCallback softbusPublishCallback_;
    std::shared_ptr<SoftbusSession> softbusSession_;
    static std::map<std::string, std::shared_ptr<DeviceInfo>> discoveryDeviceInfoMap_;
    static std::map<std::string, std::shared_ptr<ISoftbusStateCallback>> stateCallbackMap_;
    static std::map<std::string, std::shared_ptr<ISoftbusDiscoveryCallback>> discoveryCallbackMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_CONNECTOR_H
