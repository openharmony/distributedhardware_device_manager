/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SERVICE_H
#define OHOS_DM_SERVICE_H

#include <string>
#include <memory>
#if defined(__LITEOS_M__)
#include "dm_mutex.h"
#else
#include <mutex>
#endif

#include "advertise_manager.h"
#include "discovery_manager.h"
#include "pin_holder.h"
#include "device_manager_service_listener.h"
#include "idevice_manager_service_impl.h"
#include "hichain_listener.h"
#include "i_dm_check_api_white_list.h"
#include "i_dm_service_impl_ext_resident.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_account_common_event.h"
#include "dm_datashare_common_event.h"
#include "dm_package_common_event.h"
#include "dm_screen_common_event.h"
#include "relationship_sync_mgr.h"
#include "local_service_info.h"
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
#include "dm_publish_common_event.h"
#endif // SUPPORT_BLUETOOTH SUPPORT_WIFI
#endif

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerService {
DM_DECLARE_SINGLE_INSTANCE_BASE(DeviceManagerService);
public:
    DeviceManagerService() {}

    ~DeviceManagerService();

    int32_t Init();

    int32_t InitSoftbusListener();

    void InitHichainListener();

    DM_EXPORT void RegisterCallerAppId(const std::string &pkgName);

    DM_EXPORT void UnRegisterCallerAppId(const std::string &pkgName);
    
    void UninitSoftbusListener();

    int32_t InitDMServiceListener();

    DM_EXPORT void UninitDMServiceListener();

    int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                 std::vector<DmDeviceInfo> &deviceList);

    int32_t ShiftLNNGear(const std::string &pkgName, const std::string &callerId, bool isRefresh, bool isWakeUp);

    int32_t GetLocalDeviceInfo(DmDeviceInfo &info);

    int32_t GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info);

    int32_t GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &udid);

    int32_t GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &uuid);

    int32_t PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo);

    int32_t UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId);

    int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                               const std::string &extra);

    int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &networkId);

    int32_t StopAuthenticateDevice(const std::string &pkgName);

    int32_t BindDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
        const std::string &bindParam);

    int32_t UnBindDevice(const std::string &pkgName, const std::string &udidHash);

    int32_t UnBindDevice(const std::string &pkgName, const std::string &udidHash, const std::string &extra);

    int32_t ValidateUnBindDeviceParams(const std::string &pkgName, const std::string &udidHash);

    int32_t ValidateUnBindDeviceParams(const std::string &pkgName, const std::string &udidHash, const std::string &extra);

    int32_t SetUserOperation(std::string &pkgName, int32_t action, const std::string &params);

    void HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo);

    int OnSessionOpened(int sessionId, int result);

    void OnSessionClosed(int sessionId);

    void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int OnPinHolderSessionOpened(int sessionId, int result);

    void OnPinHolderSessionClosed(int sessionId);

    void OnPinHolderBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int32_t DmHiDumper(const std::vector<std::string>& args, std::string &result);

    int32_t RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr);

    int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo);

    int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo);

    int32_t MineRequestCredential(const std::string &pkgName, std::string &returnJsonStr);

    int32_t CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr);

    int32_t ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr);

    int32_t DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr);

    int32_t RegisterCredentialCallback(const std::string &pkgName);

    int32_t UnRegisterCredentialCallback(const std::string &pkgName);

    int32_t NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event);

    int32_t CheckApiPermission(int32_t permissionLevel);

    void LoadHardwareFwkService();

    int32_t GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId, std::string &uuid);

    int32_t GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid, const std::string &appId,
        std::string &encryptedUuid);

    int32_t GetNetworkTypeByNetworkId(const std::string &pkgName, const std::string &netWorkId, int32_t &networkType);
    int32_t RegisterUiStateCallback(const std::string &pkgName);
    int32_t UnRegisterUiStateCallback(const std::string &pkgName);
    int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode);
    int32_t ExportAuthCode(std::string &authCode);

    int32_t RegisterPinHolderCallback(const std::string &pkgName);
    int32_t CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload);
    int32_t DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload);

    // The following interfaces are provided since OpenHarmony 4.1 Version.
    int32_t StartDiscovering(const std::string &pkgName, const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions);

    int32_t StopDiscovering(const std::string &pkgName, const std::map<std::string, std::string> &discoverParam);

    int32_t EnableDiscoveryListener(const std::string &pkgName, const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions);

    int32_t DisableDiscoveryListener(const std::string &pkgName, const std::map<std::string, std::string> &extraParam);

    int32_t StartAdvertising(const std::string &pkgName, const std::map<std::string, std::string> &advertiseParam);

    int32_t StopAdvertising(const std::string &pkgName, const std::map<std::string, std::string> &advertiseParam);

    int32_t BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam);
    
    int32_t UnbindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &unbindParam);

    int32_t DpAclAdd(const std::string &udid);

    int32_t GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId, int32_t &networkType);
    int32_t IsSameAccount(const std::string &networkId);
    int32_t InitAccountInfo();
    int32_t InitScreenLockEvent();
    bool CheckAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee);
    bool CheckIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee);
    void HandleDeviceNotTrust(const std::string &msg);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    void HandleCredentialDeleted(const char *credId, const char *credInfo);
    void HandleDeviceTrustedChange(const std::string &msg);
    bool ParseRelationShipChangeType(const RelationShipChangeMsg &relationShipMsg);
    void HandleUserIdCheckSumChange(const std::string &msg);
    void HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid);
    void HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid,
        const std::vector<std::string> &acceptEventUdids);
#endif
    int32_t SetDnPolicy(const std::string &pkgName, std::map<std::string, std::string> &policy);
    void ClearDiscoveryCache(const ProcessInfo &processInfo);
    void HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo);
    int32_t GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
        int32_t &screenStatus);
    void SubscribePackageCommonEvent();
    int32_t GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid, std::string &networkId);
    void HandleCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId, int32_t errcode);
    int32_t SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
        const DevUserInfo &remoteDevUserInfo, std::string remoteAclList);
    int32_t GetAclListHash(const DevUserInfo &localDevUserInfo,
        const DevUserInfo &remoteDevUserInfo, std::string &aclList);
    void ProcessSyncUserIds(const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid);

    void ProcessUninstApp(int32_t userId, int32_t tokenId);
    void ProcessUnBindApp(int32_t userId, int32_t tokenId, const std::string &extra, const std::string &udid);

    int32_t SetLocalDisplayNameToSoftbus(const std::string &displayName);
    void RemoveNotifyRecord(const ProcessInfo &processInfo);
    int32_t RegDevStateCallbackToService(const std::string &pkgName);
    int32_t GetTrustedDeviceList(const std::string &pkgName, std::vector<DmDeviceInfo> &deviceList);
    void HandleDeviceUnBind(const char *peerUdid, const GroupInformation &groupInfo);
    int32_t GetAnonyLocalUdid(const std::string &pkgName, std::string &anonyUdid);
    int32_t GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        std::vector<DmDeviceInfo> &deviceList);
    int32_t RegisterAuthenticationType(const std::string &pkgName,
        const std::map<std::string, std::string> &authParam);
    int32_t GetDeviceProfileInfoList(const std::string &pkgName, DmDeviceProfileInfoFilterOptions &filterOptions);
    int32_t GetDeviceIconInfo(const std::string &pkgName, DmDeviceIconInfoFilterOptions &filterOptions);
    int32_t PutDeviceProfileInfoList(const std::string &pkgName,
        std::vector<DmDeviceProfileInfo> &deviceProfileInfoList);
    int32_t GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength, std::string &displayName);
    int32_t SetLocalDeviceName(const std::string &pkgName, const std::string &deviceName);
    int32_t SetRemoteDeviceName(const std::string &pkgName, const std::string &deviceId, const std::string &deviceName);
    std::vector<std::string> GetDeviceNamePrefixs();
    int32_t RestoreLocalDeviceName(const std::string &pkgName);
    int32_t RegisterLocalServiceInfo(const DMLocalServiceInfo &serviceInfo);
    int32_t UnRegisterLocalServiceInfo(const std::string &bundleName, int32_t pinExchangeType);
    int32_t UpdateLocalServiceInfo(const DMLocalServiceInfo &serviceInfo);
    int32_t GetLocalServiceInfoByBundleNameAndPinExchangeType(const std::string &bundleName, int32_t pinExchangeType,
        DMLocalServiceInfo &serviceInfo);
    void ClearPublishIdCache(const std::string &pkgName);
    bool IsPC();
    int32_t GetDeviceNetworkIdList(const std::string &pkgName, const NetworkIdQueryFilter &queryFilter,
        std::vector<std::string> &networkIds);
    void ProcessSyncAccountLogout(const std::string &accountId, const std::string &peerUdid, int32_t userId);
    int32_t UnRegisterPinHolderCallback(const std::string &pkgName);
    void ProcessReceiveRspAppUninstall(const std::string &remoteUdid);
    void ProcessReceiveRspAppUnbind(const std::string &remoteUdid);
    void ProcessCommonUserStatusEvent(const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid);
    int32_t GetLocalDeviceName(std::string &deviceName);
private:
    bool IsDMServiceImplReady();
    bool IsDMImplSoLoaded();
    bool IsDMServiceAdapterSoLoaded();
    bool IsDMServiceAdapterResidentLoad();
    bool IsMsgEmptyAndDMServiceImplReady(const std::string &msg);
    void UnloadDMServiceImplSo();
    void UnloadDMServiceAdapterResident();
    void SendUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId, uint64_t tokenId,
        int32_t bindLevel);
    void SendUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId, uint64_t tokenId,
        int32_t bindLevel, uint64_t peerTokenId);
    void SendDeviceUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId);
    void SendAppUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId, uint64_t tokenId);
    void SendAppUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId,
        uint64_t tokenId, uint64_t peerTokenId);
    void SendAppUnInstallBroadCast(const std::vector<std::string> &peerUdids, int32_t userId,
        uint64_t tokenId);
    void SendServiceUnBindBroadCast(const std::vector<std::string> &peerUdids, int32_t userId, uint64_t tokenId);
    void SendAccountLogoutBroadCast(const std::vector<std::string> &peerUdids, const std::string &accountId,
        const std::string &accountName, int32_t userId);
    /**
     * @brief send local foreground or background userids by broadcast
     *
     * @param peerUdids the broadcast target device udid list
     * @param foregroundUserIds local foreground userids
     * @param backgroundUserIds local background userids
     */
    void SendUserIdsBroadCast(const std::vector<std::string> &peerUdids,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds,
        bool isNeedResponse);
    void SendUserRemovedBroadCast(const std::vector<std::string> &peerUdids, int32_t userId);
    /**
     * @brief parse dsoftbus checksum msg
     *
     * @param msg checksum msg
     * @param networkId remote device networkid
     * @param discoveryType remote device link type, wifi or ble/br
     * @return int32_t 0 for success
     */
    int32_t ParseCheckSumMsg(const std::string &msg, std::string &networkId, uint32_t &discoveryType, bool &isChange);
    void ProcessCheckSumByWifi(std::string networkId, std::vector<int32_t> foregroundUserIds,
        std::vector<int32_t> backgroundUserIds);
    void ProcessCheckSumByBT(std::string networkId, std::vector<int32_t> foregroundUserIds,
        std::vector<int32_t> backgroundUserIds);
    void AddHmlInfoToBindParam(int32_t actionId, std::string &bindParam);

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    void SubscribeAccountCommonEvent();
    void SendShareTypeUnBindBroadCast(const char *credId, const int32_t localUserId,
        const std::vector<std::string> &peerUdids);
    DM_EXPORT void AccountCommonEventCallback(const std::string commonEventType,
        int32_t currentUserId, int32_t beforeUserId);
    void SubscribeScreenLockEvent();
    void ScreenCommonEventCallback(std::string commonEventType);
    DM_EXPORT void ConvertUdidHashToAnoyDeviceId(DmDeviceInfo &deviceInfo);
    DM_EXPORT int32_t ConvertUdidHashToAnoyDeviceId(const std::string &udidHash,
        std::string &anoyDeviceId);
    DM_EXPORT int32_t GetUdidHashByAnoyDeviceId(const std::string &anoyDeviceId,
        std::string &udidHash);
    void HandleAccountLogout(int32_t userId, const std::string &accountId, const std::string &accountName);
    void HandleUserRemoved(int32_t removedUserId);
    /**
     * @brief process the user switch
     *
     * @param currentUserId the user id which switched to foreground.
     * @param beforeUserId the user id which switched to backend.
     */
    void HandleUserSwitched(int32_t curUserId, int32_t preUserId);
    void HandleUserIdsBroadCast(const std::vector<UserIdInfo> &remoteUserIdInfos,
        const std::string &remoteUdid, bool isNeedResponse);
    void HandleShareUnbindBroadCast(const int32_t userId, const std::string &credId);
    void NotifyRemoteLocalUserSwitch(int32_t curUserId, int32_t preUserId, const std::vector<std::string> &peerUdids,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds);
    void NotifyRemoteLocalUserSwitchByWifi(int32_t curUserId, int32_t preUserId,
        const std::map<std::string, std::string> &wifiDevices, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds);
    int32_t SendUserIdsByWifi(const std::string &networkId, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds);
    void HandleUserSwitchTimeout(int32_t curUserId, int32_t preUserId, const std::string &udid);
    bool InitDPLocalServiceInfo(const DMLocalServiceInfo &serviceInfo,
        DistributedDeviceProfile::LocalServiceInfo &dpLocalServiceItem);
    void InitServiceInfo(const DistributedDeviceProfile::LocalServiceInfo &dpLocalServiceItem,
        DMLocalServiceInfo &serviceInfo);
    void InitServiceInfos(const std::vector<DistributedDeviceProfile::LocalServiceInfo> &dpLocalServiceItems,
        std::vector<DMLocalServiceInfo> &serviceInfos);
    void HandleUserSwitched();

    void NotifyRemoteUninstallApp(int32_t userId, int32_t tokenId);
    void NotifyRemoteUninstallAppByWifi(int32_t userId, int32_t tokenId,
        const std::map<std::string, std::string> &wifiDevices);
    int32_t SendUninstAppByWifi(int32_t userId, int32_t tokenId, const std::string &networkId);

    void GetNotifyRemoteUnBindAppWay(int32_t userId, int32_t tokenId,
        std::map<std::string, std::string> &wifiDevices, bool &isBleWay);
    void NotifyRemoteUnBindAppByWifi(int32_t userId, int32_t tokenId, std::string extra,
        const std::map<std::string, std::string> &wifiDevices);
    int32_t SendUnBindAppByWifi(int32_t userId, int32_t tokenId, std::string extra,
        const std::string &networkId, const std::string &udid);

    void NotifyRemoteLocalUserSwitch(const std::string &localUdid, const std::vector<std::string> &peerUdids,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds);
    void NotifyRemoteLocalUserSwitchByWifi(const std::string &localUdid,
        const std::map<std::string, std::string> &wifiDevices, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds);
    void HandleUserSwitchTimeout(const std::string &localUdid, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds, const std::string &udid);
    void UpdateAclAndDeleteGroup(const std::string &localUdid, const std::vector<std::string> &deviceVec,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds);
    void HandleUserSwitchedEvent(int32_t currentUserId, int32_t beforeUserId);
    void HandleUserStopEvent(int32_t stopUserId);
    void DivideNotifyMethod(const std::vector<std::string> &peerUdids, std::vector<std::string> &bleUdids,
        std::map<std::string, std::string> &wifiDevices);
    void NotifyRemoteLocalUserStop(const std::string &localUdid,
        const std::vector<std::string> &peerUdids, int32_t stopUserId);
    void SendUserStopBroadCast(const std::vector<std::string> &peerUdids, int32_t stopUserId);
    void HandleUserStopBroadCast(int32_t stopUserId, const std::string &remoteUdid);
    void NotifyRemoteLocalUserStopByWifi(const std::string &localUdid,
        const std::map<std::string, std::string> &wifiDevices, int32_t stopUserId);

    void HandleAccountCommonEvent(const std::string commonEventType);
    bool IsUserStatusChanged(std::vector<int32_t> foregroundUserVec, std::vector<int32_t> backgroundUserVec);
    void NotifyRemoteAccountCommonEvent(const std::string commonEventType, const std::string &localUdid,
        const std::vector<std::string> &peerUdids, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds);
    void NotifyRemoteAccountCommonEventByWifi(const std::string &localUdid,
        const std::map<std::string, std::string> &wifiDevices, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds);
    int32_t SendAccountCommonEventByWifi(const std::string &networkId,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds);
    void HandleCommonEventTimeout(const std::string &localUdid, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds, const std::string &udid);
    void UpdateAcl(const std::string &localUdid, const std::vector<std::string> &peerUdids,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds);
    void HandleCommonEventBroadCast(const std::vector<UserIdInfo> &remoteUserIdInfos,
        const std::string &remoteUdid, bool isNeedResponse);
    void SendCommonEventBroadCast(const std::vector<std::string> &peerUdids,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds,
        bool isNeedResponse);

#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
    void SubscribePublishCommonEvent();
    void QueryDependsSwitchState();
#endif // SUPPORT_BLUETOOTH  SUPPORT_WIFI
    DM_EXPORT void SubscribeDataShareCommonEvent();
#endif
    void HandleNetworkConnected();
    void NotifyRemoteLocalLogout(const std::vector<std::string> &peerUdids,
        const std::string &accountIdHash, const std::string &accountName, int32_t userId);
    void CheckPermission(bool &isOnlyShowNetworkId);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
    bool IsCallerInWhiteList();
    bool IsDMAdapterCheckApiWhiteListLoaded();
#endif

private:
    bool isImplsoLoaded_ = false;
    bool isAdapterResidentSoLoaded_ = false;
    void *residentSoHandle_ = nullptr;
    std::mutex isImplLoadLock_;
    std::mutex isAdapterResidentLoadLock_;
    std::mutex hichainListenerLock_;
    std::mutex userVecLock_;
    std::shared_ptr<AdvertiseManager> advertiseMgr_;
    std::shared_ptr<DiscoveryManager> discoveryMgr_;
    std::shared_ptr<SoftbusListener> softbusListener_;
    std::shared_ptr<HichainListener> hichainListener_;
    std::shared_ptr<DeviceManagerServiceListener> listener_;
    std::shared_ptr<IDeviceManagerServiceImpl> dmServiceImpl_;
    std::shared_ptr<IDMServiceImplExtResident> dmServiceImplExtResident_;
    std::string localDeviceId_;
    std::shared_ptr<PinHolder> pinHolder_;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::shared_ptr<DmAccountCommonEventManager> accountCommonEventManager_;
    std::shared_ptr<DmPackageCommonEventManager> packageCommonEventManager_;
    std::shared_ptr<DmScreenCommonEventManager> screenCommonEventManager_;
    std::vector<int32_t> foregroundUserVec_;
    std::vector<int32_t> backgroundUserVec_;
    std::mutex unInstallLock_;
    int64_t SendLastBroadCastTime_ = 0;
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
    std::shared_ptr<DmPublishCommonEventManager> publshCommonEventManager_;
#endif // SUPPORT_BLUETOOTH  SUPPORT_WIFI
    DM_EXPORT std::shared_ptr<DmDataShareCommonEventManager> dataShareCommonEventManager_;
#endif
    std::string localNetWorkId_ = "";
    std::shared_ptr<DmTimer> timer_;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
    bool isAdapterCheckApiWhiteListSoLoaded_ = false;
    void *checkApiWhiteListSoHandle_ = nullptr;
    std::mutex isAdapterCheckApiWhiteListLoadedLock_;
    std::shared_ptr<IDMCheckApiWhiteList> dmCheckApiWhiteList_;
#endif
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_H
