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

#ifndef OHOS_I_DM_SERVICE_IMPL_H
#define OHOS_I_DM_SERVICE_IMPL_H

#include <string>
#include <vector>

#include "idevice_manager_service_listener.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"

namespace OHOS {
namespace DistributedHardware {
class IDeviceManagerServiceImpl {
public:
    virtual ~IDeviceManagerServiceImpl() {}

    /**
     * @tc.name: IDeviceManagerServiceImpl::Initialize
     * @tc.desc: Initialize the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener);

    /**
     * @tc.name: IDeviceManagerServiceImpl::Release
     * @tc.desc: Release the device manager service impl
     * @tc.type: FUNC
     */
    virtual void Release();

    /**
     * @tc.name: IDeviceManagerServiceImpl::StartDeviceDiscovery
     * @tc.desc: Start Device Discovery of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                         const std::string &extra);

    /**
     * @tc.name: IDeviceManagerServiceImpl::StopDeviceDiscovery
     * @tc.desc: Stop Device Discovery of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId);

   /**
     * @tc.name: IDeviceManagerServiceImpl::PublishDeviceDiscovery
     * @tc.desc: Publish Device Discovery of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo);

    /**
     * @tc.name: IDeviceManagerServiceImpl::UnPublishDeviceDiscovery
     * @tc.desc: UnPublish Device Discovery of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId);

    /**
     * @tc.name: IDeviceManagerServiceImpl::AuthenticateDevice
     * @tc.desc: Authenticate Device of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                                       const std::string &extra);

    /**
     * @tc.name: IDeviceManagerServiceImpl::UnAuthenticateDevice
     * @tc.desc: UnAuthenticate Device of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId);

    /**
     * @tc.name: IDeviceManagerServiceImpl::VerifyAuthentication
     * @tc.desc: Verify Authentication of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t VerifyAuthentication(const std::string &authParam);

    /**
     * @tc.name: IDeviceManagerServiceImpl::GetFaParam
     * @tc.desc: Get FaParam of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t GetFaParam(std::string &pkgName, DmAuthParam &authParam);

    /**
     * @tc.name: IDeviceManagerServiceImpl::SetUserOperation
     * @tc.desc: Se tUser Operation of device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t SetUserOperation(std::string &pkgName, int32_t action, const std::string &params);

    /**
     * @tc.name: IDeviceManagerServiceImpl::RegisterDevStateCallback
     * @tc.desc: Register Device State Callback to device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    /**
     * @tc.name: IDeviceManagerServiceImpl::UnRegisterDevStateCallback
     * @tc.desc: UnRegister Device State Callback to device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    /**
     * @tc.name: IDeviceManagerServiceImpl::HandleDeviceOnline
     * @tc.desc: Handle Device Online to the device manager service impl
     * @tc.type: FUNC
     */
    virtual void HandleDeviceOnline(const DmDeviceInfo &info);

    /**
     * @tc.name: IDeviceManagerServiceImpl::HandleDeviceOffline
     * @tc.desc: Handle Device Offline to the device manager service impl
     * @tc.type: FUNC
     */
    virtual void HandleDeviceOffline(const DmDeviceInfo &info);

    /**
     * @tc.name: IDeviceManagerServiceImpl::OnSessionOpened
     * @tc.desc: Send Session Opened event to the device manager service impl
     * @tc.type: FUNC
     */
    virtual int OnSessionOpened(int sessionId, int result);

    /**
     * @tc.name: IDeviceManagerServiceImpl::OnSessionClosed
     * @tc.desc: Send Session Closed event to the device manager service impl
     * @tc.type: FUNC
     */
    virtual void OnSessionClosed(int sessionId);

    /**
     * @tc.name: IDeviceManagerServiceImpl::OnBytesReceived
     * @tc.desc: Send Bytes Received event to the device manager service impl
     * @tc.type: FUNC
     */
    virtual void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    /**
     * @tc.name: DeviceManagerService::RequestCredential
     * @tc.desc: RequestCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr);
    /**
     * @tc.name: DeviceManagerService::ImportCredential
     * @tc.desc: ImportCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo);
    /**
     * @tc.name: DeviceManagerService::DeleteCredential
     * @tc.desc: DeleteCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo);
    /**
     * @tc.name: DeviceManagerService::RegisterCredentialCallback
     * @tc.desc: RegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterCredentialCallback(const std::string &pkgName);
    /**
     * @tc.name: DeviceManagerService::UnRegisterCredentialCallback
     * @tc.desc: UnRegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterCredentialCallback(const std::string &pkgName);

    /**
     * @tc.name: DeviceManagerService::NotifyEvent
     * @tc.desc: NotifyEvent
     * @tc.type: FUNC
     */
    virtual int32_t NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event);

    /**
     * @tc.name: DeviceManagerService::LoadHardwareFwkService
     * @tc.desc: LoadHardwareFwkService
     * @tc.type: FUNC
     */
    virtual void LoadHardwareFwkService();

    /**
     * @tc.name: DeviceManagerService::GetEncryptedUuidByNetworkId
     * @tc.desc: GenerateEncryptedUuid
     * @tc.type: FUNC
     */
    virtual int32_t GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
    std::string &uuid);

    /**
     * @tc.name: DeviceManagerService::GenerateEncryptedUuid
     * @tc.desc: GenerateEncryptedUuid
     * @tc.type: FUNC
     */
    virtual int32_t GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
    const std::string &appId, std::string &encryptedUuid);
};

using CreateDMServiceFuncPtr = IDeviceManagerServiceImpl *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_IMPL_H
