/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_H
#define OHOS_DEVICE_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "device_manager_callback.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManager {
public:
    static DeviceManager &GetInstance();

public:
    /**
     * @tc.name: DeviceManagerImpl::InitDeviceManager
     * @tc.desc: Initialize DeviceManager
     * @tc.type: FUNC
     */
    virtual int32_t InitDeviceManager(const std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback) = 0;
    /**
     * @tc.name: DeviceManagerImpl::UnInitDeviceManager
     * @tc.desc: UnInitialize DeviceManager
     * @tc.type: FUNC
     */
    virtual int32_t UnInitDeviceManager(const std::string &pkgName) = 0;
    /**
     * @tc.name: DeviceManagerImpl::GetTrustedDeviceList
     * @tc.desc: Get device list of trusted devices
     * @tc.type: FUNC
     */
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        std::vector<DmDeviceInfo> &deviceList) = 0;
    /**
     * @tc.name: DeviceManagerImpl::GetLocalDeviceInfo
     * @tc.desc: Get local device information
     * @tc.type: FUNC
     */
    virtual int32_t GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &deviceInfo) = 0;
    /**
     * @brief Register device status callback
     * @param pkgName package name
     * @param extra extra info.This parameter can be null.
     * @param callback device state callback
     * @return return 0 if success
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
        std::shared_ptr<DeviceStateCallback> callback) = 0;
    /**
     * @brief Unregister device status callback
     * @param pkgName package name
     * @return return 0 if success
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName) = 0;
    /**
     * @tc.name: DeviceManagerImpl::StartDeviceDiscovery
     * @tc.desc: Initiate device discovery
     * @tc.type: FUNC
     */
    virtual int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
        const std::string &extra, std::shared_ptr<DiscoveryCallback> callback) = 0;
    /**
     * @tc.name: DeviceManagerImpl::StopDeviceDiscovery
     * @tc.desc: Stop device discovery
     * @tc.type: FUNC
     */
    virtual int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId) = 0;
    /**
     * @tc.name: DeviceManagerImpl::PublishDeviceDiscovery
     * @tc.desc: Publish device discovery
     * @tc.type: FUNC
     */
    virtual int32_t PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo,
        std::shared_ptr<PublishCallback> callback) = 0;
    /**
     * @tc.name: DeviceManagerImpl::UnPublishDeviceDiscovery
     * @tc.desc: UnPublish device discovery
     * @tc.type: FUNC
     */
    virtual int32_t UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId) = 0;
    /**
     * @tc.name: DeviceManagerImpl::AuthenticateDevice
     * @tc.desc: Complete verifying the device
     * @tc.type: FUNC
     */
    virtual int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const DmDeviceInfo &deviceInfo,
        const std::string &extra, std::shared_ptr<AuthenticateCallback> callback) = 0;
    /**
     * @tc.name: DeviceManagerImpl::UnAuthenticateDevice
     * @tc.desc: Cancel complete verification of device
     * @tc.type: FUNC
     */
    virtual int32_t UnAuthenticateDevice(const std::string &pkgName, const DmDeviceInfo &deviceInfo) = 0;
    /**
     * @tc.name: DeviceManagerImpl::VerifyAuthentication
     * @tc.desc: Verify device authentication
     * @tc.type: FUNC
     */
    virtual int32_t VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
        std::shared_ptr<VerifyAuthCallback> callback) = 0;
    /**
     * @tc.name: DeviceManagerImpl::RegisterDeviceManagerFaCallback
     * @tc.desc: Register Fa callback for device manager
     * @tc.type: FUNC
     */
    virtual int32_t RegisterDeviceManagerFaCallback(const std::string &pkgName,
        std::shared_ptr<DeviceManagerUiCallback> callback) = 0;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterDeviceManagerFaCallback
     * @tc.desc: Unregister Fa callback for device manager
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterDeviceManagerFaCallback(const std::string &pkgName) = 0;
    /**
     * @tc.name: DeviceManagerImpl::GetFaParam
     * @tc.desc: Get Fa Param
     * @tc.type: FUNC
     */
    virtual int32_t GetFaParam(const std::string &pkgName, DmAuthParam &faParam) = 0;
    /**
     * @tc.name: DeviceManagerImpl::SetUserOperation
     * @tc.desc: Set User Actions
     * @tc.type: FUNC
     */
    virtual int32_t SetUserOperation(const std::string &pkgName, int32_t action, const std::string &params) = 0;
    /**
     * @tc.name: DeviceManagerImpl::GetUdidByNetworkId
     * @tc.desc: Get Udid by NetworkId
     * @tc.type: FUNC
     */
    virtual int32_t GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &udid) = 0;
    /**
     * @tc.name: DeviceManagerImpl::GetUuidByNetworkId
     * @tc.desc: Get Uuid by NetworkId
     * @tc.type: FUNC
     */
    virtual int32_t GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &uuid) = 0;
   /**
     * @brief Unregister device status callback
     * @param pkgName package name
     * @param extra extra info.This parameter can be null.
     * @return return 0 if success
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra) = 0;
    /**
     * @brief Unregister device status callback
     * @param pkgName package name
     * @param extra extra info.This parameter can be null.
     * @return return 0 if success
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra) = 0;
        /**
     * @tc.name: DeviceManagerImpl::RequestCredential
     * @tc.desc: RequestCredential
     * @tc.type: FUNC
     */
    virtual int32_t RequestCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr) = 0;
    /**
     * @tc.name: DeviceManagerImpl::ImportCredential
     * @tc.desc: ImportCredential
     * @tc.type: FUNC
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo) = 0;
    /**
     * @tc.name: DeviceManagerImpl::DeleteCredential
     * @tc.desc: DeleteCredential
     * @tc.type: FUNC
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo) = 0;
    /**
     * @tc.name: DeviceManagerImpl::RegisterCredentialCallback
     * @tc.desc: RegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterCredentialCallback(const std::string &pkgName,
        std::shared_ptr<CredentialCallback> callback) = 0;
    /**
     * @tc.name: DeviceManagerImpl::UnRegisterCredentialCallback
     * @tc.desc: UnRegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterCredentialCallback(const std::string &pkgName) = 0;
    /**
     * @brief Notify Event to DM
     * @param pkgName package name
     * @param event event info
     */
    virtual int32_t NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event) = 0;

    /**
     * @brief Get encrypted uuid.
     * @param networkId device networkId.
     * @return Returns encrypted uuid.
     */
    virtual int32_t GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
        std::string &uuid) = 0;

    /**
     * @brief Get encrypted uuid.
     * @param uuid device uuid.
     * @param tokenId tokenId.
     * @return Returns encrypted uuid.
     */
    virtual int32_t GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid, const std::string &appId,
        std::string &encryptedUuid) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DEVICE_MANAGER_H
