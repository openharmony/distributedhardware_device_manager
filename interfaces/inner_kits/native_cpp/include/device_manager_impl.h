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

#ifndef OHOS_DEVICE_MANAGER_IMPL_H
#define OHOS_DEVICE_MANAGER_IMPL_H

#include "device_manager.h"
#include "device_manager_ipc_interface_code.h"
#if !defined(__LITEOS_M__)
#include "ipc_client_manager.h"
#include "ipc_client_proxy.h"
#endif
#include <set>

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImpl : public DeviceManager {
public:
    static DeviceManagerImpl &GetInstance();

public:
    /**
     * @tc.name: DeviceManagerImpl::InitDeviceManager
     * @tc.desc: Initialize DeviceManager
     * @tc.type: FUNC
     */
    virtual int32_t InitDeviceManager(const std::string &pkgName,
                                      std::shared_ptr<DmInitCallback> dmInitCallback) override;
    /**
     * @tc.name: DeviceManagerImpl::UnInitDeviceManager
     * @tc.desc: UnInitialize DeviceManager
     * @tc.type: FUNC
     */
    virtual int32_t UnInitDeviceManager(const std::string &pkgName) override;
    /**
     * @tc.name: DeviceManagerImpl::GetTrustedDeviceList
     * @tc.desc: Get device list of trusted devices
     * @tc.type: FUNC
     */
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                         std::vector<DmDeviceInfo> &deviceList) override;
    /**
     * @tc.name: DeviceManagerImpl::GetTrustedDeviceList
     * @tc.desc: Get device list of trusted devices
     * @tc.type: FUNC
     */
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        bool isRefresh, std::vector<DmDeviceInfo> &deviceList) override;
	/**
     * @tc.name: DeviceManagerImpl::GetAvailableDeviceList
     * @tc.desc: Get device list of trusted devices
     * @tc.type: FUNC
     */
    virtual int32_t GetAvailableDeviceList(const std::string &pkgName,
    	std::vector<DmDeviceBasicInfo> &deviceList) override;
    /**
     * @tc.name: DeviceManagerImpl::GetLocalDeviceInfo
     * @tc.desc: Get local device information
     * @tc.type: FUNC
     */
    virtual int32_t GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info) override;
    /**
     * @tc.name: DeviceManagerImpl::GetDeviceInfo
     * @tc.desc: Get local device information by networkId
     * @tc.type: FUNC
     */
    virtual int32_t GetDeviceInfo(const std::string &pkgName, const std::string networkId,
                                  DmDeviceInfo &deviceInfo) override;
    /**
     * @tc.name: DeviceManagerImpl::RegisterDevStateCallback
     * @tc.desc: Register device development status callback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
                                             std::shared_ptr<DeviceStateCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::RegisterDevStatusCallback
     * @tc.desc: Register device development status callback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterDevStatusCallback(const std::string &pkgName, const std::string &extra,
                                                std::shared_ptr<DeviceStatusCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterDevStateCallback
     * @tc.desc: UnRegister device development status callback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName) override;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterDevStatusCallback
     * @tc.desc: UnRegister device development status callback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterDevStatusCallback(const std::string &pkgName) override;
    /**
     * @tc.name: DeviceManagerImpl::StartDeviceDiscovery
     * @tc.desc: Initiate device discovery
     * @tc.type: FUNC
     */
    virtual int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                         const std::string &extra,
                                         std::shared_ptr<DiscoveryCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::StartDeviceDiscovery
     * @tc.desc: Initiate device discovery
     * @tc.type: FUNC
     */
    virtual int32_t StartDeviceDiscovery(const std::string &pkgName, uint64_t tokenId,
                                         const std::string &filterOptions,
                                         std::shared_ptr<DiscoveryCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::StopDeviceDiscovery
     * @tc.desc: Stop device discovery
     * @tc.type: FUNC
     */
    virtual int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId) override;
    /**
     * @brief Stop device discovery.
     * @param pkgName package name.
     * @param tokenId app flag to discovery device.
     * @return Returns 0 if success.
     */
    virtual int32_t StopDeviceDiscovery(uint64_t tokenId, const std::string &pkgName) override;
    /**
     * @tc.name: DeviceManagerImpl::PublishDeviceDiscovery
     * @tc.desc: Publish device discovery
     * @tc.type: FUNC
     */
    virtual int32_t PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo,
        std::shared_ptr<PublishCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::UnPublishDeviceDiscovery
     * @tc.desc: UnPublish device discovery
     * @tc.type: FUNC
     */
    virtual int32_t UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId) override;
    /**
     * @tc.name: DeviceManagerImpl::AuthenticateDevice
     * @tc.desc: Complete verifying the device
     * @tc.type: FUNC
     */
    virtual int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const DmDeviceInfo &deviceInfo,
                                       const std::string &extra,
                                       std::shared_ptr<AuthenticateCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::UnAuthenticateDevice
     * @tc.desc: Cancel complete verification of device
     * @tc.type: FUNC
     */
    virtual int32_t UnAuthenticateDevice(const std::string &pkgName, const DmDeviceInfo &deviceInfo) override;
    /**
     * @tc.name: DeviceManagerImpl::VerifyAuthentication
     * @tc.desc: Verify device authentication
     * @tc.type: FUNC
     */
    virtual int32_t VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
                                         std::shared_ptr<VerifyAuthCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::RegisterDeviceManagerFaCallback
     * @tc.desc: Register Fa callback for device manager
     * @tc.type: FUNC
     */
    virtual int32_t RegisterDeviceManagerFaCallback(const std::string &pkgName,
                                                    std::shared_ptr<DeviceManagerUiCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterDeviceManagerFaCallback
     * @tc.desc: Unregister Fa callback for device manager
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterDeviceManagerFaCallback(const std::string &pkgName) override;
    /**
     * @tc.name: DeviceManagerImpl::GetFaParam
     * @tc.desc: Get Fa Param
     * @tc.type: FUNC
     */
    virtual int32_t GetFaParam(const std::string &pkgName, DmAuthParam &dmFaParam) override;
    /**
     * @tc.name: DeviceManagerImpl::SetUserOperation
     * @tc.desc: Set User Actions
     * @tc.type: FUNC
     */
    virtual int32_t SetUserOperation(const std::string &pkgName, int32_t action, const std::string &params) override;
    /**
     * @tc.name: DeviceManagerImpl::GetUdidByNetworkId
     * @tc.desc: Get Udid by NetworkId
     * @tc.type: FUNC
     */
    virtual int32_t GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                       std::string &udid) override;
    /**
     * @tc.name: DeviceManagerImpl::GetUuidByNetworkId
     * @tc.desc: Get Uuid by NetworkId
     * @tc.type: FUNC
     */
    virtual int32_t GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                       std::string &uuid) override;
    /**
     * @tc.name: DeviceManagerImpl::RegisterDevStateCallback
     * @tc.desc: Register development status callback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra) override;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterDevStateCallback
     * @tc.desc: Unregister development status callback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra) override;
    /**
     * @tc.name: DeviceManagerImpl::RequestCredential
     * @tc.desc: RequestCredential
     * @tc.type: FUNC
     */
    virtual int32_t RequestCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr) override;
    /**
     * @tc.name: DeviceManagerImpl::ImportCredential
     * @tc.desc: ImportCredential
     * @tc.type: FUNC
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo) override;
    /**
     * @tc.name: DeviceManagerImpl::RegisterPinHolderCallback
     * @tc.desc: RegisterPinHolderCallback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterPinHolderCallback(const std::string &pkgName,
        std::shared_ptr<PinHolderCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::CreatePinHolder
     * @tc.desc: CreatePinHolder
     * @tc.type: FUNC
     */
    virtual int32_t CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload) override;
    /**
     * @tc.name: DeviceManagerImpl::DestroyPinHolder
     * @tc.desc: DestroyPinHolder
     * @tc.type: FUNC
     */
    virtual int32_t DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload) override;
    /**
     * @tc.name: DeviceManagerImpl::DeleteCredential
     * @tc.desc: DeleteCredential
     * @tc.type: FUNC
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo) override;
    /**
     * @tc.name: DeviceManagerImpl::RegisterCredentialCallback
     * @tc.desc: RegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterCredentialCallback(const std::string &pkgName,
        std::shared_ptr<CredentialCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterCredentialCallback
     * @tc.desc: UnRegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterCredentialCallback(const std::string &pkgName) override;

    /**
     * @tc.name: DeviceManagerImpl::NotifyEvent
     * @tc.desc: NotifyEvent
     * @tc.type: FUNC
     */
    virtual int32_t NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event) override;
    /**
     * @tc.name: DeviceManagerImpl::RequestCredential
     * @tc.desc: RequestCredential
     * @tc.type: FUNC
     */
    virtual int32_t RequestCredential(const std::string &pkgName, std::string &returnJsonStr) override;
    /**
     * @tc.name: DeviceManagerImpl::CheckCredential
     * @tc.desc: CheckCredential
     * @tc.type: FUNC
     */
    virtual int32_t CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                    std::string &returnJsonStr) override;
    /**
     * @tc.name: DeviceManagerImpl::ImportCredential
     * @tc.desc: ImportCredential
     * @tc.type: FUNC
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                     std::string &returnJsonStr) override;
    /**
     * @tc.name: DeviceManagerImpl::DeleteCredential
     * @tc.desc: DeleteCredential
     * @tc.type: FUNC
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                     std::string &returnJsonStr) override;

    virtual int32_t GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
        std::string &uuid) override;

    virtual int32_t GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
        const std::string &appId, std::string &encryptedUuid) override;

    /**
     * @tc.name: DeviceManagerImpl::CheckAPIAccessPermission
     * @tc.desc: check permission for device manager API
     * @tc.type: FUNC
     */
    virtual int32_t CheckAPIAccessPermission() override;
    virtual int32_t CheckNewAPIAccessPermission() override;

    int32_t OnDmServiceDied();
    int32_t RegisterUiStateCallback(const std::string &pkgName);
    int32_t UnRegisterUiStateCallback(const std::string &pkgName);

    virtual int32_t GetLocalDeviceNetWorkId(const std::string &pkgName, std::string &networkId) override;
    virtual int32_t GetLocalDeviceId(const std::string &pkgName, std::string &networkId) override;
    virtual int32_t GetLocalDeviceType(const std::string &pkgName, int32_t &deviceType) override;
    virtual int32_t GetLocalDeviceName(const std::string &pkgName, std::string &deviceName) override;
    virtual int32_t GetLocalDeviceName(std::string &deviceName) override;
    virtual int32_t GetDeviceName(const std::string &pkgName, const std::string &networkId,
        std::string &deviceName) override;
    virtual int32_t GetDeviceType(const std::string &pkgName,
        const std::string &networkId, int32_t &deviceType) override;
    virtual int32_t BindDevice(const std::string &pkgName, int32_t bindType, const std::string &deviceId,
        const std::string &extra, std::shared_ptr<AuthenticateCallback> callback) override;
    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &deviceId) override;
    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &deviceId,
        const std::string &extra) override;
    virtual int32_t GetNetworkTypeByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                       int32_t &netWorkType) override;
    virtual int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode) override;
    virtual int32_t ExportAuthCode(std::string &authCode) override;

    // The following interfaces are provided since OpenHarmony 4.1 Version.
    virtual int32_t StartDiscovering(const std::string &pkgName, std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions, std::shared_ptr<DiscoveryCallback> callback) override;

    virtual int32_t StopDiscovering(const std::string &pkgName,
        std::map<std::string, std::string> &discoverParam) override;

    virtual int32_t RegisterDiscoveryCallback(const std::string &pkgName,
        std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions,
        std::shared_ptr<DiscoveryCallback> callback) override;

    virtual int32_t UnRegisterDiscoveryCallback(const std::string &pkgName) override;

    virtual int32_t StartAdvertising(const std::string &pkgName, std::map<std::string, std::string> &advertiseParam,
        std::shared_ptr<PublishCallback> callback) override;

    virtual int32_t StopAdvertising(const std::string &pkgName,
        std::map<std::string, std::string> &advertiseParam) override;

    virtual int32_t BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        std::map<std::string, std::string> &bindParam, std::shared_ptr<BindTargetCallback> callback) override;
    
    virtual int32_t UnbindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        std::map<std::string, std::string> &unbindParam, std::shared_ptr<UnbindTargetCallback> callback) override;

    virtual int32_t GetTrustedDeviceList(const std::string &pkgName,
        const std::map<std::string, std::string> &filterOptions, bool isRefresh,
        std::vector<DmDeviceInfo> &deviceList) override;

    virtual int32_t RegisterDevStateCallback(const std::string &pkgName,
        const std::map<std::string, std::string> &extraParam,
        std::shared_ptr<DeviceStateCallback> callback) override;

    virtual int32_t CheckAccessToTarget(uint64_t tokenId, const std::string &targetId) override;

    virtual int32_t DpAclAdd(const int64_t accessControlId, const std::string &udid, const int32_t bindType) override;

    virtual int32_t GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId,
                                           int32_t &securityLevel) override;

    virtual bool IsSameAccount(const std::string &netWorkId) override;
    virtual bool CheckAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee) override;
    virtual bool CheckIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee) override;
    virtual int32_t GetErrCode(int32_t errCode) override;
    virtual int32_t ShiftLNNGear(const std::string &pkgName) override;
    virtual int32_t RegDevTrustChangeCallback(const std::string &pkgName,
        std::shared_ptr<DevTrustChangeCallback> callback) override;

    virtual int32_t SetDnPolicy(const std::string &pkgName, std::map<std::string, std::string> &policy) override;
    virtual int32_t RegisterDeviceScreenStatusCallback(const std::string &pkgName,
        std::shared_ptr<DeviceScreenStatusCallback> callback) override;
    virtual int32_t UnRegisterDeviceScreenStatusCallback(const std::string &pkgName) override;
    virtual int32_t GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
        int32_t &screenStatus) override;
    virtual int32_t StopAuthenticateDevice(const std::string &pkgName) override;
    virtual int32_t GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid,
        std::string &networkId) override;
    virtual int32_t RegisterCredentialAuthStatusCallback(const std::string &pkgName,
        std::shared_ptr<CredentialAuthStatusCallback> callback) override;
    virtual int32_t UnRegisterCredentialAuthStatusCallback(const std::string &pkgName) override;
    virtual int32_t RegisterSinkBindCallback(const std::string &pkgName,
        std::shared_ptr<BindTargetCallback> callback) override;
    virtual int32_t UnRegisterSinkBindCallback(const std::string &pkgName) override;
    virtual int32_t GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        std::vector<DmDeviceInfo> &deviceList) override;
    void SyncCallbacksToService(std::map<DmCommonNotifyEvent, std::set<std::string>> &callbackMap);
    virtual int32_t RegisterAuthenticationType(const std::string &pkgName,
        const std::map<std::string, std::string> &authParam) override;

    virtual int32_t GetDeviceProfileInfoList(const std::string &pkgName,
        const DmDeviceProfileInfoFilterOptions &filterOptions,
        std::shared_ptr<GetDeviceProfileInfoListCallback> callback) override;

    virtual int32_t GetDeviceIconInfo(const std::string &pkgName, const DmDeviceIconInfoFilterOptions &filterOptions,
        std::shared_ptr<GetDeviceIconInfoCallback> callback) override;
    virtual int32_t PutDeviceProfileInfoList(const std::string &pkgName,
        const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfoList) override;
    virtual int32_t GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
        std::string &displayName) override;
    virtual int32_t RegisterLocalServiceInfo(const DMLocalServiceInfo &info) override;
    virtual int32_t UnRegisterLocalServiceInfo(const std::string &bundleName, int32_t pinExchangeType) override;
    virtual int32_t UpdateLocalServiceInfo(const DMLocalServiceInfo &info) override;
    virtual int32_t GetLocalServiceInfoByBundleNameAndPinExchangeType(const std::string &bundleName,
        int32_t pinExchangeType, DMLocalServiceInfo &info) override;
    virtual int32_t SetLocalDeviceName(const std::string &pkgName, const std::string &deviceName,
        std::shared_ptr<SetLocalDeviceNameCallback> callback) override;
    virtual int32_t SetRemoteDeviceName(const std::string &pkgName, const std::string &deviceId,
        const std::string &deviceName, std::shared_ptr<SetRemoteDeviceNameCallback> callback) override;
    virtual int32_t RestoreLocalDeviceName(const std::string &pkgName) override;
    virtual int32_t GetDeviceNetworkIdList(const std::string &bundleName, const NetworkIdQueryFilter &queryFilter,
        std::vector<std::string> &networkIds) override;
    virtual int32_t UnRegisterPinHolderCallback(const std::string &pkgName) override;
    virtual bool CheckSrcAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee) override;
    virtual bool CheckSinkAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee) override;
    virtual bool CheckSrcIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee) override;
    virtual bool CheckSinkIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee) override;

private:
    DeviceManagerImpl() = default;
    ~DeviceManagerImpl() = default;
    DeviceManagerImpl(const DeviceManagerImpl &) = delete;
    DeviceManagerImpl &operator=(const DeviceManagerImpl &) = delete;
    DeviceManagerImpl(DeviceManagerImpl &&) = delete;
    DeviceManagerImpl &operator=(DeviceManagerImpl &&) = delete;

    uint16_t AddDiscoveryCallback(const std::string &pkgName, std::map<std::string, std::string> &discoverParam,
        std::shared_ptr<DiscoveryCallback> callback);
    uint16_t RemoveDiscoveryCallback(const std::string &pkgName);
    int32_t AddPublishCallback(const std::string &pkgName);
    int32_t RemovePublishCallback(const std::string &pkgName);
    int32_t CheckApiPermission(int32_t permissionLevel);
    void ConvertDeviceInfoToDeviceBasicInfo(const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo);
    uint16_t GetSubscribeIdFromMap(const std::string &pkgName);
    void SyncCallbackToService(DmCommonNotifyEvent dmCommonNotifyEvent, const std::string &pkgName);
    int32_t GetAnonyLocalUdid(const std::string &pkgName, std::string &anonyUdid);
    bool CheckAclByIpcCode(const DmAccessCaller &caller, const DmAccessCallee &callee,
        const DMIpcCmdInterfaceCode &ipcCode);

private:
#if !defined(__LITEOS_M__)
    std::shared_ptr<IpcClientProxy> ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());
#endif
    std::mutex subMapLock;
    std::map<std::string, uint16_t> pkgName2SubIdMap_;

    std::mutex pubMapLock;
    std::map<std::string, uint16_t> pkgName2PubIdMap_;

    std::string anonyLocalUdid_;

    std::set<uint16_t> randSubIdSet_;
    std::set<uint16_t> randPubIdSet_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IMPL_H
