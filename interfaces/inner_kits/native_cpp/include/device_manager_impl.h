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

#ifndef OHOS_DEVICE_MANAGER_IMPL_H
#define OHOS_DEVICE_MANAGER_IMPL_H

#include "device_manager.h"
#if !defined(__LITEOS_M__)
#include "ipc_client_manager.h"
#include "ipc_client_proxy.h"
#endif

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
     * @tc.name: DeviceManagerImpl::GetLocalDeviceInfo
     * @tc.desc: Get local device information
     * @tc.type: FUNC
     */
    virtual int32_t GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &deviceInfo) override;
    /**
     * @tc.name: DeviceManagerImpl::RegisterDevStateCallback
     * @tc.desc: Register device development status callback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
                                             std::shared_ptr<DeviceStateCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterDevStateCallback
     * @tc.desc: UnRegister device development status callback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName) override;
    /**
     * @tc.name: DeviceManagerImpl::StartDeviceDiscovery
     * @tc.desc: Initiate device discovery
     * @tc.type: FUNC
     */
    virtual int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                         const std::string &extra,
                                         std::shared_ptr<DiscoveryCallback> callback) override;
    /**
     * @tc.name: DeviceManagerImpl::StopDeviceDiscovery
     * @tc.desc: Stop device discovery
     * @tc.type: FUNC
     */
    virtual int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId) override;
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
    virtual int32_t RegisterDeviceManagerFaCallback(const std::string &packageName,
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
    virtual int32_t GetFaParam(const std::string &pkgName, DmAuthParam &faParam) override;
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

    virtual int32_t GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
        std::string &uuid) override;

    virtual int32_t GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
        const std::string &appId, std::string &encryptedUuid) override;

    int32_t OnDmServiceDied();
private:
    DeviceManagerImpl() = default;
    ~DeviceManagerImpl() = default;
    DeviceManagerImpl(const DeviceManagerImpl &) = delete;
    DeviceManagerImpl &operator=(const DeviceManagerImpl &) = delete;
    DeviceManagerImpl(DeviceManagerImpl &&) = delete;
    DeviceManagerImpl &operator=(DeviceManagerImpl &&) = delete;

private:
#if !defined(__LITEOS_M__)
    std::shared_ptr<IpcClientProxy> ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());
#endif
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IMPL_H
