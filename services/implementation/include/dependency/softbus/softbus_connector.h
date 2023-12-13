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
#include <queue>
#include <string>
#include <vector>

#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "softbus_discovery_callback.h"
#include "softbus_publish_callback.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "softbus_session.h"
#endif
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
    static void OnSoftbusPublishResult(int32_t publishId, PublishResult result);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusDeviceFound
     * @tc.desc: OnSoftbus DeviceFound of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusDeviceFound(const DeviceInfo *device);

    /**
     * @tc.name: SoftbusConnector::OnSoftbusDeviceDiscovery
     * @tc.desc: OnSoftbus DeviceDiscovery of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusDeviceDiscovery(const DeviceInfo *device);

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
     * @tc.name: SoftbusConnector::GetConnectAddr
     * @tc.desc: Get Connect Addr of the SoftbusConnector
     * @tc.type: FUNC
     */
    static ConnectionAddr *GetConnectAddr(const std::string &deviceId, std::string &connectAddr);

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
     * @tc.name: SoftbusConnector::GetDeviceUdidByUdidHash
     * @tc.desc: Get Udid By DeviceId of the SoftbusConnector
     * @tc.type: FUNC
     */
    static std::string GetDeviceUdidByUdidHash(const std::string &udidHash);

    /**
     * @tc.name: SoftbusConnector::JoinLnn
     * @tc.desc: Add the current device to the LNN where a specified device resides
     * @tc.type: FUNC
     */
    static void JoinLnn(const std::string &deviceId);
public:
    SoftbusConnector();
    ~SoftbusConnector();
    int32_t RegisterSoftbusDiscoveryCallback(const std::string &pkgName,
                                             const std::shared_ptr<ISoftbusDiscoveryCallback> callback);
    int32_t UnRegisterSoftbusDiscoveryCallback(const std::string &pkgName);
    int32_t RegisterSoftbusPublishCallback(const std::string &pkgName,
                                           const std::shared_ptr<ISoftbusPublishCallback> callback);
    int32_t UnRegisterSoftbusPublishCallback(const std::string &pkgName);
    int32_t RegisterSoftbusStateCallback(const std::shared_ptr<ISoftbusStateCallback> callback);
    int32_t UnRegisterSoftbusStateCallback();
    int32_t PublishDiscovery(const DmPublishInfo &dmPublishInfo);
    int32_t UnPublishDiscovery(int32_t publishId);
    int32_t StartDiscovery(const DmSubscribeInfo &dmSubscribeInfo);
    int32_t StartDiscovery(const uint16_t subscribeId);
    int32_t StopDiscovery(uint16_t subscribeId);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::shared_ptr<SoftbusSession> GetSoftbusSession();
#endif
    bool HaveDeviceInMap(std::string deviceId);
    std::string GetDeviceUdidHashByUdid(const std::string &udid);
    void EraseUdidFromMap(const std::string &udid);
    std::string GetLocalDeviceName();
    std::string GetLocalDeviceNetworkId();
    int32_t GetLocalDeviceTypeId();
    int32_t AddMemberToDiscoverMap(const std::string &deviceId, std::shared_ptr<DeviceInfo> deviceInfo);
    std::string GetNetworkIdByDeviceId(const std::string &deviceId);
    void HandleDeviceOnline(std::string deviceId);
    void HandleDeviceOffline(std::string deviceId);
    void SetPkgName(std::string pkgName);
    bool CheckIsOnline(const std::string &targetDeviceId);
    void SetPkgNameVec(std::vector<std::string> pkgNameVec);
    std::vector<std::string> GetPkgName();
    void ClearPkgName();
    DmDeviceInfo GetDeviceInfoByDeviceId(const std::string &deviceId);

private:
    static void ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceInfo &dmDeviceInfo);
    static void ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceBasicInfo &dmDeviceBasicInfo);
    static ConnectionAddr *GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type);
    static void ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo);

private:
    enum PulishStatus {
        STATUS_UNKNOWN = 0,
        ALLOW_BE_DISCOVERY = 1,
        NOT_ALLOW_BE_DISCOVERY = 2,
    };
    static PulishStatus publishStatus;
    static IRefreshCallback softbusDiscoveryCallback_;
    static IRefreshCallback softbusDiscoveryByIdCallback_;
    static IPublishCb softbusPublishCallback_;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::shared_ptr<SoftbusSession> softbusSession_;
#endif
    static std::map<std::string, std::shared_ptr<DeviceInfo>> discoveryDeviceInfoMap_;
    static std::map<std::string, std::shared_ptr<ISoftbusDiscoveryCallback>> discoveryCallbackMap_;
    static std::map<std::string, std::shared_ptr<ISoftbusPublishCallback>> publishCallbackMap_;
    std::shared_ptr<ISoftbusStateCallback> deviceStateManagerCallback_;
    static std::queue<std::string> discoveryDeviceIdQueue_;
    static std::unordered_map<std::string, std::string> deviceUdidMap_;
    static std::vector<std::string> pkgNameVec_;
    static std::mutex discoveryCallbackMutex_;
    static std::mutex discoveryDeviceInfoMutex_;
    static std::mutex deviceUdidLocks_;
    static std::mutex pkgNameVecMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_CONNECTOR_H
