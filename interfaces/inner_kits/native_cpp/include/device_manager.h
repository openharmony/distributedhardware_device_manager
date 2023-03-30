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
     * @brief Initialize DeviceManager.
     * @param pkgName        package name.
     * @param dmInitCallback the callback to be invoked upon InitDeviceManager.
     * @return Returns 0 if success.
     */
    virtual int32_t InitDeviceManager(const std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback) = 0;
    /**
     * @brief UnInitialize DeviceManager.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnInitDeviceManager(const std::string &pkgName) = 0;
    /**
     * @brief Get device info list of trusted devices.
     * @param pkgName    package name.
     * @param extra      extra info.This parameter can be null.
     * @param deviceList device info list.
     * @return Returns a list of trusted devices.
     */
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        std::vector<DmDeviceInfo> &deviceList) = 0;
    /**
     * @brief Get local device information.
     * @param pkgName    package name.
     * @param deviceInfo device info.
     * @return Returns local device info.
     */
    virtual int32_t GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &deviceInfo) = 0;
    /**
     * @tc.name: DeviceManagerImpl::GetDeviceInfo
     * @tc.desc: Get local device information by networkId
     * @tc.type: FUNC
     */
    virtual int32_t GetDeviceInfo(const std::string &pkgName, const std::string networkId,
        DmDeviceInfo &deviceInfo) = 0;
    /**
     * @brief Register device status callback.
     * @param pkgName  package name.
     * @param extra    extra info.This parameter can be null.
     * @param callback device state callback.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
        std::shared_ptr<DeviceStateCallback> callback) = 0;
    /**
     * @brief Unregister device status callback.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName) = 0;
    /**
     * @brief Initiate device discovery.
     * @param pkgName       package name.
     * @param subscribeInfo subscribe info to discovery device.
     * @param extra         extra info.This parameter can be null.
     * @param callback      discovery callback.
     * @return Returns 0 if success.
     */
    virtual int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
        const std::string &extra, std::shared_ptr<DiscoveryCallback> callback) = 0;
    /**
     * @brief Stop device discovery.
     * @param pkgName       package name.
     * @param subscribeInfo subscribe info to discovery device.
     * @return Returns 0 if success.
     */
    virtual int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId) = 0;
    /**
     * @brief Publish device discovery.
     * @param pkgName     package name.
     * @param publishInfo publish info to Publish discovery device.
     * @param callback    the callback to be invoked upon PublishDeviceDiscovery.
     * @return Returns 0 if success.
     */
    virtual int32_t PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo,
        std::shared_ptr<PublishCallback> callback) = 0;
    /**
     * @brief UnPublish device discovery.
     * @param pkgName   package name.
     * @param publishId service publish ID, identify a publish operation, should be a unique id in package range.
     * @return Returns 0 if success.
     */
    virtual int32_t UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId) = 0;
    /**
     * @brief Authenticate the specified device.
     * @param pkgName    package name.
     * @param authType   authType of device to authenticate.
     * @param deviceInfo deviceInfo of device to authenticate.
     * @param extra      extra info.This parameter can be null.
     * @param callback   the callback to be invoked upon AuthenticateDevice.
     * @return Returns 0 if success.
     */
    virtual int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const DmDeviceInfo &deviceInfo,
        const std::string &extra, std::shared_ptr<AuthenticateCallback> callback) = 0;
    /**
     * @brief Cancel complete verification of device.
     * @param pkgName    package name.
     * @param deviceInfo deviceInfo of device to authenticate.
     * @return Returns 0 if success.
     */
    virtual int32_t UnAuthenticateDevice(const std::string &pkgName, const DmDeviceInfo &deviceInfo) = 0;
    /**
     * @brief Verify device authentication.
     * @param pkgName  package name.
     * @param authPara authPara of device to authenticate.
     * @param callback the callback to be invoked upon VerifyAuthentication.
     * @return Returns 0 if success.
     */
    virtual int32_t VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
        std::shared_ptr<VerifyAuthCallback> callback) = 0;
    /**
     * @brief Register Fa callback for device manager.
     * @param pkgName  package name.
     * @param callback device manager Fa callback.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterDeviceManagerFaCallback(const std::string &pkgName,
        std::shared_ptr<DeviceManagerUiCallback> callback) = 0;
    /**
     * @brief Unregister Fa callback for device manager.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterDeviceManagerFaCallback(const std::string &pkgName) = 0;
    /**
     * @brief Get Fa Param.
     * @param pkgName package name.
     * @param faParam fa param.
     * @return Returns 0 if success.
     */
    virtual int32_t GetFaParam(const std::string &pkgName, DmAuthParam &faParam) = 0;
    /**
     * @brief Set User Actions.
     * @param pkgName package name.
     * @param action  user operation action.
     * @param params  indicates the input param of the user.
     * @return Returns 0 if success.
     */
    virtual int32_t SetUserOperation(const std::string &pkgName, int32_t action, const std::string &params) = 0;
    /**
     * @brief Get Udid by NetworkId.
     * @param pkgName   package name.
     * @param netWorkId netWork Id.
     * @param udid      unique device id.
     * @return Returns 0 if success.
     */
    virtual int32_t GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &udid) = 0;
    /**
     * @brief Get Uuid by NetworkId.
     * @param pkgName   package name.
     * @param netWorkId netWork Id.
     * @param uuid      universally unique id.
     * @return Returns 0 if success.
     */
    virtual int32_t GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &uuid) = 0;
    /**
     * @brief Unregister device status callback.
     * @param pkgName package name.
     * @param extra   extra info.This parameter can be null.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra) = 0;
    /**
     * @brief Unregister device status callback.
     * @param pkgName package name.
     * @param extra   extra info.This parameter can be null.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra) = 0;
    /**
     * @brief Request credential information.
     * @param pkgName       package name.
     * @param reqJsonStr    request credential params, the params is json string, it includes version and userId.
     * @param returnJsonStr return json string, it includes deviceId, devicePk, useId and version.
     * @return Returns 0 if success.
     */
    virtual int32_t RequestCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr) = 0;
    /**
     * @brief Import credential information.
     * @param pkgName        package name.
     * @param credentialInfo import credential params, the params is json string, it includes processType, authType,
     * userId, deviceId, version, devicePk and credentialData, the credentialData is array, each array element
     * includes credentialType, credentialId, serverPk, pkInfoSignature, pkInfo, authCode, peerDeviceId.
     * @return Returns 0 if success.
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo) = 0;
    /**
     * @brief Delete credential information.
     * @param pkgName    package name.
     * @param deleteInfo delete credential params. the params is json string, it includes processType, authType, userId.
     * @return Returns 0 if success.
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo) = 0;
    /**
     * @brief Register credential callback.
     * @param pkgName  package name.
     * @param callback credential callback.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterCredentialCallback(const std::string &pkgName,
        std::shared_ptr<CredentialCallback> callback) = 0;
    /**
     * @brief UnRegister credential callback.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterCredentialCallback(const std::string &pkgName) = 0;
    /**
     * @brief Notify event to device manager.
     * @param pkgName package name.
     * @param event   event info.
     * @param event   event string.
     * @return Returns 0 if success.
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
