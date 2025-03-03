/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "softbus_session.h"
#endif
#include "softbus_state_callback.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusConnector {
public:
    /**
     * @tc.name: SoftbusConnector::OnSoftbusJoinLNNResult
     * @tc.desc: OnSoftbus JoinLNN Result of the SoftbusConnector
     * @tc.type: FUNC
     */
    static void OnSoftbusJoinLNNResult(ConnectionAddr *addr, const char *networkId, int32_t result);

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
    static void JoinLnn(const std::string &deviceId, bool isForceJoin = false);

    /**
     * @tc.name: SoftbusConnector::JoinLnnByHml
     * @tc.desc: join lnn by session id
     * @tc.type: FUNC
     */
    static void JoinLnnByHml(int32_t sessionId, int32_t sessionKeyId, int32_t remoteSessionKeyId);

public:
    SoftbusConnector();
    ~SoftbusConnector();
    int32_t RegisterSoftbusStateCallback(const std::shared_ptr<ISoftbusStateCallback> callback);
    int32_t UnRegisterSoftbusStateCallback();
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
    void HandleDeviceOnline(std::string deviceId, int32_t authForm);
    void HandleDeviceOffline(std::string deviceId);
    void SetProcessInfo(ProcessInfo processInfo);
    bool CheckIsOnline(const std::string &targetDeviceId);
    void SetProcessInfoVec(std::vector<ProcessInfo> processInfoVec);
    std::vector<ProcessInfo> GetProcessInfo();
    void ClearProcessInfo();
    DmDeviceInfo GetDeviceInfoByDeviceId(const std::string &deviceId);
    void DeleteOffLineTimer(std::string &udidHash);

private:
    static void ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceInfo &dmDeviceInfo);
    static void ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceBasicInfo &dmDeviceBasicInfo);
    static ConnectionAddr *GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type);
    static void ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo);

private:
    static std::string remoteUdidHash_;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::shared_ptr<SoftbusSession> softbusSession_;
#endif
    static std::map<std::string, std::shared_ptr<DeviceInfo>> discoveryDeviceInfoMap_;
    std::shared_ptr<ISoftbusStateCallback> deviceStateManagerCallback_;
    static std::unordered_map<std::string, std::string> deviceUdidMap_;
    static std::vector<ProcessInfo> processInfoVec_;
    static std::mutex discoveryDeviceInfoMutex_;
    static std::mutex deviceUdidLocks_;
    static std::mutex processInfoVecMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_CONNECTOR_H
