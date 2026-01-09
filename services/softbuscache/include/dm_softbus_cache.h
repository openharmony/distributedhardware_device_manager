/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SOFTBUS_CACHE_H
#define OHOS_DM_SOFTBUS_CACHE_H

#include <cstdint>
#include <string>
#include <mutex>
#include "dm_device_info.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif
#include "dm_single_instance.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusCache {
    DM_DECLARE_SINGLE_INSTANCE(SoftbusCache);

public:
    void SaveDeviceInfo(DmDeviceInfo deviceInfo);
    void DeleteDeviceInfo(const DmDeviceInfo &nodeInfo);
    void DeleteDeviceInfo();
    void ChangeDeviceInfo(const DmDeviceInfo deviceInfo);
    void SaveDeviceSecurityLevel(const char *networkId);
    void DeleteDeviceSecurityLevel(const char *networkId);
    int32_t GetDeviceInfoFromCache(std::vector<DmDeviceInfo> &deviceInfoList);
    bool GetDeviceInfoByDeviceId(const std::string &deviceId, std::string &uuid, DmDeviceInfo &devInfo);
    int32_t GetUdidFromCache(const char *networkId, std::string &udid);
    int32_t GetUuidFromCache(const char *networkId, std::string &uuid);
    int32_t GetSecurityDeviceLevel(const char *networkId, int32_t &securityLevel);
    void UpdateDeviceInfoCache();
    void SaveLocalDeviceInfo();
    void DeleteLocalDeviceInfo();
    int32_t GetLocalDeviceInfo(DmDeviceInfo &nodeInfo);
    int32_t GetDevInfoByNetworkId(const std::string &networkId, DmDeviceInfo &nodeInfo);
    void UpDataLocalDevInfo();
    int32_t GetUdidByUdidHash(const std::string &udidHash, std::string &udid);
    int32_t GetUuidByUdid(const std::string &udid, std::string &uuid);
    int32_t GetNetworkIdFromCache(const std::string &udid, std::string &networkId);
    int32_t GetDeviceNameFromCache(const std::string &udid, std::string &deviceName);
    bool CheckIsOnline(const std::string &udidHash);
    bool CheckIsOnlineByPeerUdid(const std::string &peerUdid);
private:
    int32_t GetUdidByNetworkId(const char *networkId, std::string &udid);
    int32_t GetUuidByNetworkId(const char *networkId, std::string &uuid);
    int32_t ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceInfo &devInfo);
    int32_t GetDevInfoFromBus(const std::string &networkId, DmDeviceInfo &devInfo);
    int32_t GetDevLevelFromBus(const char *networkId, int32_t &securityLevel);

private:
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::mutex deviceInfosMutex_;
    ffrt::mutex deviceSecurityLevelMutex_;
#else
    std::mutex deviceInfosMutex_;
    std::mutex deviceSecurityLevelMutex_;
#endif
    std::unordered_map<std::string, std::pair<std::string, DmDeviceInfo>> deviceInfo_;
    std::unordered_map<std::string, int32_t> deviceSecurityLevel_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PERMISSION_STANDARD_PERMISSION_MANAGER_H
