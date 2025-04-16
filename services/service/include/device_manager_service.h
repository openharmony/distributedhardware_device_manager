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
#include "dm_device_profile_info.h"
#include "pin_holder.h"
#include "device_manager_service_listener.h"
#include "idevice_manager_service_impl.h"
#include "i_dm_service_impl_ext.h"
#include "dm_single_instance.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_account_common_event.h"
#include "dm_package_common_event.h"
#include "dm_screen_common_event.h"
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

    void RegisterCallerAppId(const std::string &pkgName);

    void UnRegisterCallerAppId(const std::string &pkgName);
    
    void UninitSoftbusListener();

    int32_t InitDMServiceListener();

    void UninitDMServiceListener();

    int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                 std::vector<DmDeviceInfo> &deviceList);

    int32_t ShiftLNNGear(const std::string &pkgName, const std::string &callerId, bool isRefresh, bool isWakeUp);

    int32_t GetLocalDeviceInfo(DmDeviceInfo &info);

    int32_t GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info);

    int32_t GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &udid);

    int32_t GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &uuid);

    int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                 const std::string &extra);

    int32_t StartDeviceDiscovery(const std::string &pkgName, const uint16_t subscribeInfo,
                                const std::string &filterOptions);

    int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId);

    int32_t PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo);

    int32_t UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId);

    int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                               const std::string &extra);

    int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &networkId);

    int32_t StopAuthenticateDevice(const std::string &pkgName);

    int32_t BindDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
        const std::string &bindParam);

    int32_t UnBindDevice(const std::string &pkgName, const std::string &deviceId);

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

    void OnUnbindSessionOpened(int32_t socket, PeerSocketInfo info);
    void OnUnbindSessionCloseed(int32_t socket);
    void OnUnbindBytesReceived(int32_t socket, const void *data, uint32_t dataLen);

    int32_t DpAclAdd(const std::string &udid);

    int32_t GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId, int32_t &networkType);
    int32_t IsSameAccount(const std::string &networkId);
    int32_t InitAccountInfo();
    int32_t InitScreenLockEvent();
    bool CheckAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee);
    bool CheckIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee);
    void HandleDeviceNotTrust(const std::string &msg);

    int32_t SetDnPolicy(const std::string &pkgName, std::map<std::string, std::string> &policy);

    void SubscribePackageCommonEvent();
    int32_t GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid, std::string &networkId);
    void HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo);
    int32_t GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
        int32_t &screenStatus);
    void HandleCredentialAuthStatus(const std::string &proofInfo, uint16_t deviceTypeId, int32_t errcode);
    int32_t GetDeviceProfileInfoList(const std::string &pkgName, DmDeviceProfileInfoFilterOptions &filterOptions);
    int32_t RegisterAuthenticationType(const std::string &pkgName,
        const std::map<std::string, std::string> &authParam);
private:
    bool IsDMServiceImplReady();
    bool IsDMServiceAdapterLoad();
    bool IsDMImplSoLoaded();
    void UnloadDMServiceImplSo();
    void UnloadDMServiceAdapter();
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    void SubscribeAccountCommonEvent();
    void AccountCommonEventCallback(int32_t userId, std::string commonEventType);
    void SubscribeScreenLockEvent();
    void ScreenCommonEventCallback(std::string commonEventType);
    void ConvertUdidHashToAnoyDeviceId(DmDeviceInfo &deviceInfo);
    int32_t ConvertUdidHashToAnoyDeviceId(const std::string &udidHash, std::string &anoyDeviceId);
    int32_t GetUdidHashByAnoyDeviceId(const std::string &anoyDeviceId, std::string &udidHash);
    void HandleAccountLogout(int32_t userId, const std::string &accountId);
    void HandleUserRemoved(int32_t preUserId);
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
    void SubscribePublishCommonEvent();
    void QueryDependsSwitchState();
#endif // SUPPORT_BLUETOOTH  SUPPORT_WIFI
#endif

private:
    bool isImplsoLoaded_ = false;
    bool isAdapterSoLoaded_ = false;
    std::mutex isImplLoadLock_;
    std::mutex isAdapterLoadLock_;
    std::shared_ptr<AdvertiseManager> advertiseMgr_;
    std::shared_ptr<DiscoveryManager> discoveryMgr_;
    std::shared_ptr<SoftbusListener> softbusListener_;
    std::shared_ptr<DeviceManagerServiceListener> listener_;
    std::shared_ptr<IDeviceManagerServiceImpl> dmServiceImpl_;
    std::shared_ptr<IDMServiceImplExt> dmServiceImplExt_;
    std::string localDeviceId_;
    std::shared_ptr<PinHolder> pinHolder_;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::shared_ptr<DmAccountCommonEventManager> accountCommonEventManager_;
    std::shared_ptr<DmPackageCommonEventManager> packageCommonEventManager_;
    std::shared_ptr<DmScreenCommonEventManager> screenCommonEventManager_;
#if defined(SUPPORT_BLUETOOTH) || defined(SUPPORT_WIFI)
    std::shared_ptr<DmPublishCommonEventManager> publshCommonEventManager_;
#endif // SUPPORT_BLUETOOTH  SUPPORT_WIFI
#endif
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_H