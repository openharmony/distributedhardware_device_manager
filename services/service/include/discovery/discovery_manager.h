/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISCOVERY_MANAGER_H
#define OHOS_DISCOVERY_MANAGER_H

#include <set>
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "deviceprofile_connector.h"
#endif
#include "discovery_filter.h"
#include "idevice_manager_service_listener.h"
#include "dm_timer.h"
#include "mine_softbus_listener.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct DiscoveryContext {
    std::string pkgName;
    std::string extra;
    uint16_t subscribeId;
    std::string filterOp;
    std::vector<DeviceFilters> filters;
} DiscoveryContext;

typedef struct MultiUserDiscovery {
    std::string pkgName;
    int32_t userId;
} MultiUserDiscovery;

typedef enum {
    PROXY_TRANSMISION = 0,
    PROXY_HEARTBEAT = 1,
    PROXY_HICAR = 2,
    PROXY_ACS = 3,
    PROXY_SHARE = 4,
    PROXY_CASTPLUS = 5,
    PROXY_DM = 6,
    PROXY_WEAR = 7,
    PROXY_WINPC = 8,
    PROXY_COLLABORATION_FWK = 9,
    PROXY_DMSDP = 10,
    CUSTOM_UNKNOWN,
} MetaNodeType;

class DiscoveryManager : public ISoftbusDiscoveringCallback, public std::enable_shared_from_this<DiscoveryManager> {
public:
    DiscoveryManager(std::shared_ptr<SoftbusListener> softbusListener,
        std::shared_ptr<IDeviceManagerServiceListener> listener);
    ~DiscoveryManager() override;

    // interfaces from ISoftbusDiscoveringCallback
    void OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info, bool isOnline) override;
    void OnDiscoveringResult(const std::string &pkgName, int32_t subscribeId, int32_t result) override;
    int32_t StartDiscovering(const std::string &pkgName, const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions);
    int32_t StopDiscovering(const std::string &pkgName, uint16_t subscribeId);
    int32_t EnableDiscoveryListener(const std::string &pkgName, const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions);
    int32_t DisableDiscoveryListener(const std::string &pkgName, const std::map<std::string, std::string> &extraParam);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    static IDeviceProfileConnector* GetCommonDependencyObj();
    static bool IsCommonDependencyReady();
    static bool CloseCommonDependencyObj();
#endif
    void ClearDiscoveryCache(const ProcessInfo &processInfo);
    std::set<uint16_t> ClearDiscoveryPkgName(const std::string &pkgName);

private:
    void StartDiscoveryTimer(const std::string &pkgName);
    void HandleDiscoveryTimeout(const std::string &pkgName);
    int32_t StartDiscovering4MetaType(const std::string &pkgName, DmSubscribeInfo &dmSubInfo,
        const std::map<std::string, std::string> &param);
    int32_t StartDiscoveringNoMetaType(const std::string &pkgName, DmSubscribeInfo &dmSubInfo,
        const std::map<std::string, std::string> &param);
    int32_t StartDiscovering4MineLibary(const std::string &pkgName, DmSubscribeInfo &dmSubInfo,
        const std::string &searchJson);
    int32_t HandleDiscoveryQueue(const std::string &pkgName, uint16_t subscribeId,
        const std::map<std::string, std::string> &filterOps);
    int32_t GetDeviceAclParam(const std::string &pkgName, int32_t userId, std::string deviceId, bool &isOnline,
        int32_t &authForm);
    void ConfigDiscParam(const std::map<std::string, std::string> &discoverParam, DmSubscribeInfo *dmSubInfo);
    bool CompareCapability(uint32_t capabilityType, const std::string &capabilityStr);
    void OnDeviceFound(const std::string &pkgName, const uint32_t capabilityType,
        const DmDeviceInfo &info, const DeviceFilterPara &filterPara);
    void UpdateInfoFreq(const std::map<std::string, std::string> &discoverParam, DmSubscribeInfo &dmSubInfo);
    std::string AddMultiUserIdentify(const std::string &pkgName);
    std::string RemoveMultiUserIdentify(const std::string &pkgName);
    void GetPkgNameAndUserId(const std::string &pkgName, std::string &callerPkgName, int32_t &userId);
    void UpdateInfoMedium(const std::map<std::string, std::string> &discoverParam, DmSubscribeInfo &dmSubInfo);
    int32_t GenInnerSubId(const std::string &pkgName, uint16_t subId);
    int32_t GetAndRemoveInnerSubId(const std::string &pkgName, uint16_t subId);
    int32_t StopDiscoveringByInnerSubId(const std::string &pkgName, uint16_t subscribeId);

private:
    std::mutex locks_;
    std::mutex subIdMapLocks_;
    std::shared_ptr<DmTimer> timer_;
    // Caller subId to inner subId. The key is the caller pkgName, the value is
    // a list of externals subId to inner subId.
    std::map<std::string, std::map<uint16_t, uint16_t>> pkgName2SubIdMap_;
    std::shared_ptr<SoftbusListener> softbusListener_;
    std::shared_ptr<MineSoftbusListener> mineSoftbusListener_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::map<std::string, DiscoveryContext> discoveryContextMap_;

    std::set<std::string> pkgNameSet_;
    std::map<std::string, std::string> capabilityMap_;
    std::mutex capabilityMapLocks_;
    std::mutex multiUserDiscLocks_;
    std::map<std::string, MultiUserDiscovery> multiUserDiscMap_;

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    static bool isSoLoaded_;
    static IDeviceProfileConnector *dpConnector_;
    static void *dpConnectorHandle_;
#endif

    std::set<uint16_t> randSubIdSet_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISCOVERY_MANAGER_H
