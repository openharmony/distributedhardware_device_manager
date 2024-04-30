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
#include "pin_holder.h"
#include "device_manager_service_listener.h"
#include "idevice_manager_service_impl.h"
#include "i_dm_service_impl_ext.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerService {
DECLARE_SINGLE_INSTANCE_BASE(DeviceManagerService);
public:
    DeviceManagerService() {}

    ~DeviceManagerService();

    int32_t Init();

    int32_t InitSoftbusListener();

    void RegisterDeviceManagerListener(const std::string &pkgName);

    void UnRegisterDeviceManagerListener(const std::string &pkgName);
    
    void UninitSoftbusListener();

    int32_t InitDMServiceListener();

    void UninitDMServiceListener();

    int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                 std::vector<DmDeviceInfo> &deviceList);

    int32_t GetAvailableDeviceList(const std::string &pkgName,
                                 std::vector<DmDeviceBasicInfo> &deviceBasicInfoList);

    int32_t ShiftLNNGear(const std::string &pkgName, const std::string &callerId, bool isRefresh);

    int32_t GetLocalDeviceInfo(DmDeviceInfo &info);

    int32_t GetLocalDeviceNetworkId(std::string &networkId);

    int32_t GetLocalDeviceId(const std::string &pkgName, std::string &deviceId);

    int32_t GetLocalDeviceType(int32_t &deviceType);

    int32_t GetLocalDeviceName(std::string &deviceName);

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
    int32_t IsSameAccount(const std::string &udid);
    int32_t CheckRelatedDevice(const std::string &udid, const std::string &bundleName);

private:
    bool IsDMServiceImplReady();
    bool IsDMServiceAdapterLoad();
    void UnloadDMServiceImplSo();
    void UnloadDMServiceAdapter();

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
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_H