/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_I_DM_SERVICE_IMPL_EXT_RESIDENT_H
#define OHOS_I_DM_SERVICE_IMPL_EXT_RESIDENT_H

#include "idevice_manager_service_listener.h"
#include "isa_specification_verify.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "service_info.h"
#endif
namespace OHOS {
namespace DistributedHardware {
class IDMServiceImplExtResident {
public:
    virtual ~IDMServiceImplExtResident() = default;
    virtual int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener) = 0;
    virtual int32_t Release() = 0;
    virtual bool IsDMServiceAdapterLoad() = 0;
    virtual bool IsDMServiceAdapterSoLoaded() = 0;
   /**
     * @tc.name: IDMServiceImplExt::BindTargetExt
     * @tc.desc: BindTargetExt
     * @tc.type: FUNC
     */
    virtual int32_t BindTargetExt(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::UnbindTargetExt
     * @tc.desc: UnbindTargetExt
     * @tc.type: FUNC
     */
    virtual int32_t UnbindTargetExt(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &unbindParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::HandleDeviceStatusChange
     * @tc.desc: HandleDeviceStatusChange
     * @tc.type: FUNC
     */
    virtual int32_t HandleDeviceStatusChange(DmDeviceState devState, const DmDeviceInfo &devInfo) = 0;

    /**
     * @tc.name: IDMServiceImplExt::ReplyUiAction
     * @tc.desc: ReplyUiAction
     * @tc.type: FUNC
     */
    virtual int32_t ReplyUiAction(const std::string &pkgName, int32_t action, const std::string &result) = 0;

    /**
     * @tc.name: IDMServiceImplExt::AccountIdLogout
     * @tc.desc: AccountIdLogout
     * @tc.type: FUNC
     */
    virtual int32_t AccountIdLogout(int32_t userId, const std::string &oldAccountId,
        const std::vector<std::string> &peerUdids) = 0;
    /**
     * @tc.name: IDMServiceImplExt::HandleDeviceNotTrust
     * @tc.desc: HandleDeviceNotTrust
     * @tc.type: FUNC
     */
    virtual void HandleDeviceNotTrust(const std::string &udid) = 0;

    /**
     * @tc.name: IDMServiceImplExt::SetDnPolicy
     * @tc.desc: SetDnPolicy
     * @tc.type: FUNC
     */
    virtual int32_t SetDnPolicy(int32_t policy, int32_t timeOut) = 0;

    /**
     * @tc.name: IDMServiceImplExt::AccountUserSwitched
     * @tc.desc: AccountUserSwitched
     * @tc.type: FUNC
     */
    virtual int32_t AccountUserSwitched(int32_t userId, const std::string &accountId) = 0;

    /**
     * @tc.name: IDMServiceImplExt::GetDeviceProfileInfoList
     * @tc.desc: GetDeviceProfileInfoList
     * @tc.type: FUNC
     */
    virtual int32_t GetDeviceProfileInfoList(const std::string &pkgName,
        const DmDeviceProfileInfoFilterOptions &filterOptions) = 0;
    /**
     * @tc.name: IDMServiceImplExt::GetDeviceIconInfo
     * @tc.desc: GetDeviceIconInfo
     * @tc.type: FUNC
     */
    virtual int32_t GetDeviceIconInfo(const std::string &pkgName,
        const DmDeviceIconInfoFilterOptions &filterOptions) = 0;
    virtual int32_t PutDeviceProfileInfoList(const std::string &pkgName,
        const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfoList) = 0;
    virtual std::vector<std::string> GetDeviceNamePrefixs() = 0;
    virtual void HandleNetworkConnected(int32_t networkStatus) = 0;
    virtual int32_t SetLocalDeviceName(const std::string &pkgName, const std::string &deviceName) = 0;
    virtual int32_t SetRemoteDeviceName(const std::string &pkgName, const std::string &deviceId,
        const std::string &deviceName) = 0;
    virtual int32_t GetDeviceProfileInfosFromLocalCache(const NetworkIdQueryFilter &queryFilter,
        std::vector<DmDeviceProfileInfo> &dmDeviceProfileInfos) = 0;
    virtual int32_t RestoreLocalDeviceName() = 0;
    virtual void ClearCacheWhenLogout(int32_t userId, const std::string &oldAccountId) = 0;
    virtual void HandleScreenLockEvent(bool isLock) = 0;
    virtual int32_t OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m) = 0;
    virtual void HandleUserSwitchEvent(int32_t currentUserId, int32_t beforeUserId) = 0;
    virtual int32_t OpenAuthSessionWithPara(int64_t serviceId) = 0;
    virtual bool CheckBuildLink(const DmSaCaller &caller, const DmSaCallee &callee,
        bool isRpc, bool isSrc) = 0;
    virtual int32_t ImportAuthInfo(const DmAuthInfo &dmAuthInfo) = 0;
    virtual int32_t ExportAuthInfo(DmAuthInfo &dmAuthInfo) = 0;
    virtual int32_t InitSoftbusServer() = 0;

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    virtual int32_t RegisterServiceInfo(const DmRegisterServiceInfo &regServiceInfo, int64_t &serviceId) = 0;
    virtual int32_t UnRegisterServiceInfo(int64_t serviceId) = 0;
    virtual int32_t StartPublishService(const std::string &pkgName, int64_t serviceId,
        const DmPublishServiceParam &publishServiceParam) = 0;
    virtual int32_t StopPublishService(const ProcessInfo &processInfo, int64_t serviceId) = 0;
    virtual int32_t StartDiscoveryService(const ProcessInfo &processInfo, const DmDiscoveryServiceParam &discParam) = 0;
    virtual int32_t StopDiscoveryService(const ProcessInfo &processInfo, const DmDiscoveryServiceParam &discParam) = 0;
    virtual int32_t RegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId) = 0;
    virtual int32_t ClearServiceStateCallback(const std::string &pkgName, int32_t userId) = 0;
    virtual int32_t UnRegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId) = 0;
    virtual int32_t HandleServiceStatusChange(DmDeviceState devState, const std::string &peerUdid) = 0;
    virtual int32_t BindServiceOnline(const ServiceStateBindParameter &bindParam) = 0;
    virtual int32_t BindServiceOffline(uint64_t tokenId, const std::string &pkgName, int32_t bindType,
        const std::string &peerUdid, const DistributedDeviceProfile::ServiceInfo &serviceInfo) = 0;
    virtual int32_t SetServiceNodeKeyInfo(const std::string &pkgName) = 0;
    virtual int32_t GetServiceNodeKeyInfo(const std::string &pkgName, const std::string &networkId) = 0;
    virtual int32_t SyncServiceInfoOnline(const std::string &localUdid, const std::string &networkId) = 0;
    virtual int32_t SyncServiceInfoByServiceId(const std::string &pkgName, int32_t localUserId,
        const std::string &networkId, int64_t serviceId) = 0;
    virtual int32_t SyncAllServiceInfo(const std::string &pkgName,
        int32_t localUserId, const std::string &networkId) = 0;
    virtual int32_t GetLocalServiceInfoByServiceId(int64_t serviceId, DmRegisterServiceInfo &serviceInfo) = 0;
    virtual int32_t GetTrustServiceInfo(const std::string &pkgName, const std::map<std::string, std::string> &param,
        std::vector<DmServiceInfo> &serviceInfo) = 0;
    virtual int32_t GetRegisterServiceInfo(const std::map<std::string, std::string> &param,
        std::vector<DmRegisterServiceInfo> &regServiceInfos) = 0;
    virtual int32_t GetPeerServiceInfoByServiceId(const std::string &networkId, int64_t serviceId,
        DmRegisterServiceInfo &serviceInfo) = 0;
    virtual int32_t UnbindServiceTarget(const std::string &pkgName,
        const std::map<std::string, std::string> &unbindParam, const std::string &netWorkId,
        int64_t serviceId) = 0;
    virtual void HandleServiceUnRegEvent(const std::string &peerUdid, int64_t serviceId) = 0;
    virtual int32_t HandleRemoteDied(const ProcessInfo &processInfo) = 0;
    virtual int32_t HandleDMRestart() = 0;
    virtual int32_t HandleSoftbusRestart() = 0;
    virtual int32_t UpdateServiceInfo(int64_t serviceId, const DmRegisterServiceInfo &regServiceInfo) = 0;
    virtual int32_t DeleteServiceInfoForAccountEvent(int32_t peerUserId, const std::string &peerUdid,
        const std::string &localUdid) = 0;
    virtual int32_t SubscribeAllServiceInfo() = 0;
    virtual void HandleProcessUnBindServiceProxy(UnbindServiceProxyParam &param) = 0;
#endif
};

using CreateDMServiceExtResidentFuncPtr = IDMServiceImplExtResident *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_IMPL_EXT_RESIDENT_H
