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

#ifndef OHOS_DM_SOFTBUS_LISTENER_H
#define OHOS_DM_SOFTBUS_LISTENER_H

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_radar_helper.h"
#include "dm_subscribe_info.h"
#include "i_softbus_discovering_callback.h"
#include "inner_session.h"
#include "session.h"
#include "socket.h"

namespace OHOS {
namespace DistributedHardware {

class SoftbusListener {
public:
    SoftbusListener();
    ~SoftbusListener();

    static void OnSoftbusDeviceFound(const DeviceInfo *device);
    static void OnSoftbusDiscoveryResult(int subscribeId, RefreshResult result);
    static void OnSoftbusDeviceOnline(NodeBasicInfo *info);
    static void OnSoftbusDeviceOffline(NodeBasicInfo *info);
    static void OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info);
    static void OnSoftbusPublishResult(int publishId, PublishResult result);
    static void DeviceOnLine(DmDeviceInfo deviceInfo);
    static void DeviceOffLine(DmDeviceInfo deviceInfo);
    static void DeviceNameChange(DmDeviceInfo deviceInfo);
    static void OnLocalDevInfoChange();
    static void OnDeviceTrustedChange(TrustChangeType type, const char *msg, uint32_t msgLen);
    static void DeviceNotTrust(const std::string &msg);
    static void DeviceTrustedChange(const std::string &msg);
    static void DeviceUserIdCheckSumChange(const std::string &msg);
    static void OnDeviceScreenStatusChanged(NodeStatusType type, NodeStatus *status);
    static void DeviceScreenStatusChange(DmDeviceInfo deviceInfo);
    static void OnCredentialAuthStatus(const char *deviceList, uint32_t deviceListLen, uint16_t deviceTypeId,
                                       int32_t errcode);
    static void CredentialAuthStatusProcess(std::string deviceList, uint16_t deviceTypeId, int32_t errcode);

    static void CacheDiscoveredDevice(const DeviceInfo *device);
    static void ClearDiscoveredDevice();
    static void ConvertDeviceInfoToDmDevice(const DeviceInfo &device, DmDeviceInfo &dmDevice);
    static int32_t GetUdidByNetworkId(const char *networkId, std::string &udid);
    static int32_t GetTargetInfoFromCache(const std::string &deviceId, PeerTargetId &targetId,
        ConnectionAddrType &addrType);
    static int32_t ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceInfo &devInfo);
    static int32_t ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceBasicInfo &devInfo);
    static std::string ConvertBytesToUpperCaseHexString(const uint8_t arr[], const size_t size);
    static int32_t ConvertScreenStatusToDmDevice(const NodeBasicInfo &nodeInfo, const int32_t devScreenStatus,
        DmDeviceInfo &devInfo);

    int32_t InitSoftbusListener();
    int32_t GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList);
    int32_t GetLocalDeviceInfo(DmDeviceInfo &deviceInfo);
    int32_t GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info);
    int32_t ShiftLNNGear(bool isWakeUp, const std::string &callerId);
    static int32_t GetUuidByNetworkId(const char *networkId, std::string &uuid);
    int32_t GetDeviceSecurityLevel(const char *networkId, int32_t &securityLevel);
    int32_t GetNetworkTypeByNetworkId(const char *networkId, int32_t &networkType);
    int32_t RefreshSoftbusLNN(const char *pkgName, const DmSubscribeInfo &dmSubInfo, const std::string &customData);
    int32_t StopRefreshSoftbusLNN(uint16_t subscribeId);
    int32_t PublishSoftbusLNN(const DmPublishInfo &dmPubInfo, const std::string &capability,
        const std::string &customData);
    int32_t StopPublishSoftbusLNN(int32_t publishId);
    int32_t RegisterSoftbusLnnOpsCbk(const std::string &pkgName,
        const std::shared_ptr<ISoftbusDiscoveringCallback> callback);
    int32_t UnRegisterSoftbusLnnOpsCbk(const std::string &pkgName);
    static IDmRadarHelper* GetDmRadarHelperObj();
    static bool IsDmRadarHelperReady();
    static bool CloseDmRadarHelperObj(std::string name);
    static void CacheDeviceInfo(const std::string deviceId, std::shared_ptr<DeviceInfo> infoPtr);
    static int32_t GetIPAddrTypeFromCache(const std::string &deviceId, const std::string &ip,
        ConnectionAddrType &addrType);
    static IRefreshCallback &GetSoftbusRefreshCb();
    static void SetHostPkgName(const std::string hostName);
    static std::string GetHostPkgName();
    void SendAclChangedBroadcast(const std::string &msg);
    int32_t GetDeviceScreenStatus(const char *networkId, int32_t &screenStatus);
    static int32_t GetNetworkIdByUdid(const std::string &udid, std::string &networkId);
    int32_t SetLocalDeviceName(const std::string &localDeviceName, const std::string &localDisplayName);
    int32_t SetForegroundUserIdsToDSoftBus(const std::string &remoteUserId, const std::vector<uint32_t> &userIds);
private:
    static int32_t FillDeviceInfo(const DeviceInfo &device, DmDeviceInfo &dmDevice);
    int32_t InitSoftPublishLNN();

private:
    static std::string hostName_;
    static bool isRadarSoLoad_;
    static IDmRadarHelper *dmRadarHelper_;
    static void *radarHandle_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_LISTENER_H
