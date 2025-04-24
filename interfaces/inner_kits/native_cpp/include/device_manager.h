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

#ifndef OHOS_DEVICE_MANAGER_H
#define OHOS_DEVICE_MANAGER_H

#include "device_manager_callback.h"
#include "dm_publish_info.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManager {
public:
    EXPORT static DeviceManager &GetInstance();

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
     * @brief Get device info list of trusted devices.
     * @param pkgName    package name.
     * @param extra      extra info.This parameter can be null.
     * @param isRefresh  refresh the list quickly.
     * @param deviceList device info list.
     * @return Returns a list of trusted devices.
     */
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        bool isRefresh, std::vector<DmDeviceInfo> &deviceList) = 0;
	/**
     * @brief Get device info list of available devices.
     * @param pkgName    package name.
     * @param extra      extra info.This parameter can be null.
     * @param deviceList device info list.
     * @return Returns a list of available devices.
     */
    virtual int32_t GetAvailableDeviceList(const std::string &pkgName,
        std::vector<DmDeviceBasicInfo> &deviceList) = 0;
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
     * @brief Register device status callback.
     * @param pkgName  package name.
     * @param callback device status callback.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterDevStatusCallback(const std::string &pkgName, const std::string &extra,
        std::shared_ptr<DeviceStatusCallback> callback) = 0;
    /**
     * @brief Unregister device status callback.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName) = 0;
    /**
     * @brief Unregister device status callback.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterDevStatusCallback(const std::string &pkgName) = 0;
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
     * @brief Initiate device discovery.
     * @param pkgName       package name.
     * @param subscribeId subscribe id to discovery device.
     * @param extra         extra info.This parameter can be null.
     * @param callback      discovery callback.
     * @return Returns 0 if success.
     */
    virtual int32_t StartDeviceDiscovery(const std::string &pkgName, uint64_t tokenId,
        const std::string &filterOptions, std::shared_ptr<DiscoveryCallback> callback) = 0;
    /**
     * @brief Stop device discovery.
     * @param pkgName       package name.
     * @param subscribeInfo subscribe info to discovery device.
     * @return Returns 0 if success.
     */
    virtual int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId) = 0;
    /**
     * @brief Stop device discovery.
     * @param pkgName package name.
     * @param tokenId app flag to discovery device.
     * @return Returns 0 if success.
     */
    virtual int32_t StopDeviceDiscovery(uint64_t tokenId, const std::string &pkgName) = 0;
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
    [[deprecated]] virtual int32_t VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
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
    [[deprecated]] virtual int32_t GetFaParam(const std::string &pkgName, DmAuthParam &faParam) = 0;
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
    [[deprecated]] virtual int32_t RegisterDevStateCallback(const std::string &pkgName,
        const std::string &extra) = 0;
    /**
     * @brief Unregister device status callback.
     * @param pkgName package name.
     * @param extra   extra info.This parameter can be null.
     * @return Returns 0 if success.
     */
    [[deprecated]] virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName,
        const std::string &extra) = 0;
    /**
     * @brief Request credential information.
     * @param pkgName       package name.
     * @param reqJsonStr    request credential params, the params is json string, it includes version and userId.
     * @param returnJsonStr return json string, it includes deviceId, devicePk, useId and version.
     * @return Returns 0 if success.
     */
    [[deprecated]] virtual int32_t RequestCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr) = 0;
    /**
     * @brief Import credential information.
     * @param pkgName        package name.
     * @param credentialInfo import credential params, the params is json string, it includes processType, authType,
     * userId, deviceId, version, devicePk and credentialData, the credentialData is array, each array element
     * includes credentialType, credentialId, serverPk, pkInfoSignature, pkInfo, authCode, peerDeviceId.
     * @return Returns 0 if success.
     */
    [[deprecated]] virtual int32_t ImportCredential(const std::string &pkgName,
        const std::string &credentialInfo) = 0;
    /**
     * @brief Delete credential information.
     * @param pkgName    package name.
     * @param deleteInfo delete credential params. the params is json string, it includes processType, authType, userId.
     * @return Returns 0 if success.
     */
    [[deprecated]] virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo) = 0;
    /**
     * @brief Register credential callback.
     * @param pkgName  package name.
     * @param callback credential callback.
     * @return Returns 0 if success.
     */
    [[deprecated]] virtual int32_t RegisterCredentialCallback(const std::string &pkgName,
        std::shared_ptr<CredentialCallback> callback) = 0;
    /**
     * @brief UnRegister credential callback.
     * @param pkgName package name.
     * @return Returns 0 if success.
     */
    [[deprecated]] virtual int32_t UnRegisterCredentialCallback(const std::string &pkgName) = 0;
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

    /**
     * @tc.name: DeviceManagerImpl::CheckAPIAccessPermission
     * @tc.desc: check permission for device manager API
     * @tc.type: FUNC
     */
    virtual int32_t CheckAPIAccessPermission() = 0;

    /**
     * @brief Get local device netWorkId.
     * @param pkgName package name.
     * @return Returns local device networkId.
     */
    virtual int32_t GetLocalDeviceNetWorkId(const std::string &pkgName, std::string &networkId) = 0;

    /**
     * @brief Get local deviceId.
     * @param pkgName package name.
     * @return Returns local deviceId.
     */
    virtual int32_t GetLocalDeviceId(const std::string &pkgName, std::string &networkId) = 0;

    /**
     * @brief Get local device name.
     * @param pkgName package name.
     * @return Returns device name.
     */
    virtual int32_t GetLocalDeviceName(const std::string &pkgName, std::string &deviceName) = 0;

    /**
     * @brief Get local device type.
     * @param pkgName package name.
     * @return Returns device type.
     */
    virtual int32_t GetLocalDeviceType(const std::string &pkgName, int32_t &deviceType) = 0;

    /**
     * @brief Get device name.
     * @param pkgName package name.
     * @param networkId device networkId.
     * @return Returns device name.
     */
    virtual int32_t GetDeviceName(const std::string &pkgName, const std::string &networkId,
        std::string &deviceName) = 0;

    /**
     * @brief Get device type.
     * @param pkgName package name.
     * @param networkId device networkId.
     * @return Returns device type.
     */
    virtual int32_t GetDeviceType(const std::string &pkgName, const std::string &networkId, int32_t &deviceType) = 0;

    /**
     * @brief Bind the specified device.
     * @param pkgName    package name.
     * @param bindType   bindType of device to bind.
     * @param deviceInfo device id of device to bind.
     * @param extra      extra info.This parameter can be null.
     * @param callback   callback.
     * @return Returns 0 if success.
     */
    virtual int32_t BindDevice(const std::string &pkgName, int32_t bindType, const std::string &deviceId,
        const std::string &extra, std::shared_ptr<AuthenticateCallback> callback) = 0;
    
    /**
     * @brief UnBind the specified device.
     * @param pkgName    package name.
     * @param deviceId device id to UnBindDevice.
     * @return Returns 0 if success.
     */
    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &deviceId) = 0;

    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &deviceId, const std::string &extra) = 0;

    virtual int32_t CheckNewAPIAccessPermission() = 0;

    /**
     * @brief Get Network Type by NetworkId.
     * @param pkgName   package name.
     * @param netWorkId netWork Id.
     * @param netWorkType netWork Type.
     * @return Returns 0 if success.
     */
    virtual int32_t GetNetworkTypeByNetworkId(const std::string &pkgName,
        const std::string &netWorkId, int32_t &netWorkType) = 0;

    /**
     * @brief Import Auth Code.
     * @param pkgName  BindDevice caller package name.
     * @param authCode  Authentication code.
     * @return Returns 0 if success.
     */
    virtual int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode) = 0;

    /**
     * @brief Export Auth Code.
     * @param authCode  Authentication code.
     * @return Returns 0 if success.
     */
    virtual int32_t ExportAuthCode(std::string &authCode) = 0;

    // The following interfaces are provided since OpenHarmony 4.1 Version.
    /**
     * @brief Start to discover nearby devices or services.
     * @param pkgName       package name.
     * @param discoverParam discover parameters.
     * @param filterOptions filter option parameters.
     * @param callback      discovery callback.
     * @return Returns 0 if success.
     */
    virtual int32_t StartDiscovering(const std::string &pkgName, std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions, std::shared_ptr<DiscoveryCallback> callback) = 0;

    /**
     * @brief Stop discovering nearby devices or services.
     * @param pkgName       package name.
     * @param discoverParam discover parameters.
     * @return Returns 0 if success.
     */
    virtual int32_t StopDiscovering(const std::string &pkgName, std::map<std::string, std::string> &discoverParam) = 0;

    /**
     * @brief Registerthe discovery callback.
     * @param pkgName       package name.
     * @param discoverParam discover parameters.
     * @param filterOptions filter option parameters.
     * @param callback      discovery callback.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterDiscoveryCallback(const std::string &pkgName,
        std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions,
        std::shared_ptr<DiscoveryCallback> callback) = 0;

    /**
     * @brief UnRegisterthe discovery callback.
     * @param pkgName       package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterDiscoveryCallback(const std::string &pkgName) = 0;

    /**
     * @brief Start to advertise the nearby devices or services.
     * @param pkgName        package name.
     * @param advertiseParam advertise parameters.
     * @param callback       advertise callback.
     * @return Returns 0 if success.
     */
    virtual int32_t StartAdvertising(const std::string &pkgName, std::map<std::string, std::string> &advertiseParam,
        std::shared_ptr<PublishCallback> callback) = 0;

    /**
     * @brief Stop to advertise the nearby devices or services.
     * @param pkgName        package name.
     * @param advertiseParam advertise parameters.
     * @return Returns 0 if success.
     */
    virtual int32_t StopAdvertising(const std::string &pkgName, std::map<std::string, std::string> &advertiseParam) = 0;

    /**
     * @brief Bind the specified target.
     * @param pkgName    package name.
     * @param targetId   id of target to bind.
     * @param bindParam  bind parameters.
     * @param callback   bind result callback.
     * @return Returns 0 if success.
     */
    virtual int32_t BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        std::map<std::string, std::string> &bindParam, std::shared_ptr<BindTargetCallback> callback) = 0;
    
    /**
     * @brief Unbind the specified target.
     * @param pkgName     package name.
     * @param targetId    id of target to unbind.
     * @param unbindParam unbind parameters.
     * @param callback    bind result callback.
     * @return Returns 0 if success.
     */
    virtual int32_t UnbindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        std::map<std::string, std::string> &unbindParam, std::shared_ptr<UnbindTargetCallback> callback) = 0;

    /**
     * @brief Get device info list of trusted devices.
     * @param pkgName        package name.
     * @param filterOptions  filter option parameters.
     * @param isRefresh      refresh the list quickly.
     * @param deviceList     device info list.
     * @return Returns a list of trusted devices.
     */
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName,
        const std::map<std::string, std::string> &filterOptions, bool isRefresh,
        std::vector<DmDeviceInfo> &deviceList) = 0;

    /**
     * @brief Register device state callback.
     * @param pkgName     package name.
     * @param extraParam  extra parameters.
     * @param callback    device status callback.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName,
        const std::map<std::string, std::string> &extraParam, std::shared_ptr<DeviceStateCallback> callback) = 0;

    /**
     * @brief Check the specific caller whether has permission to access the target.
     * @param tokenId  the caller token id.
     * @param targetId the target id.
     * @return Returns 0 if success.
     */
    virtual int32_t CheckAccessToTarget(uint64_t tokenId, const std::string &targetId) = 0;

    /**
     * @brief Register Pin Code Holder Callback
     * @param pkgName  package name.
     * @param callback  the callback to be invoked upon CreateAuthCodeHolder or DestroyAuthCodeHolder.
     * @return Returns 0 if success.
     */
    virtual int32_t RegisterPinHolderCallback(const std::string &pkgName,
        std::shared_ptr<PinHolderCallback> callback) = 0;

    /**
     * @brief Create Pin Code Holder
     * @param pkgName  package name.
     * @param targetId  id of target to create pin holder.
     * @param pinType  pin code holder type.
     * @param payload  business custom data.
     * @return Returns 0 if success.
     */
    virtual int32_t CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload) = 0;

    /**
     * @brief Destroy Pin Code Holder
     * @param pkgName  package name.
     * @param targetId  id of target to destroy pin holder.
     * @param pinType  pin code holder type.
     * @param payload  business custom data.
     * @return Returns 0 if success.
     */
    virtual int32_t DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload) = 0;

    /**
    * @brief Request credential information.
    * @param pkgName       package name.
    * @param returnJsonStr return json string, it includes deviceId, devicePk, useId and version.
    * @return Returns 0 if success.
    */
    virtual int32_t RequestCredential(const std::string &pkgName, std::string &returnJsonStr) = 0;

    /**
     * @brief Check credential information.
     * @param pkgName       package name.
     * @param reqJsonStr    request credential params, the params is json string, it includes version and userId.
     * @param returnJsonStr return json string, it includes deviceId, devicePk, useId and version.
     * @return Returns 0 if success.
     */
    virtual int32_t CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                    std::string &returnJsonStr) = 0;

    /**
     * @brief Import credential information.
     * @param pkgName       package name.
     * @param reqJsonStr    request credential params, the params is json string, it includes version and userId.
     * @param returnJsonStr return json string, it includes deviceId, devicePk, useId and version.
     * @return Returns 0 if success.
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                     std::string &returnJsonStr) = 0;

    /**
     * @brief Delete credential information.
     * @param pkgName       package name.
     * @param reqJsonStr    request credential params, the params is json string, it includes version and userId.
     * @param returnJsonStr return json string, it includes deviceId, devicePk, useId and version.
     * @return Returns 0 if success.
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
                                     std::string &returnJsonStr) = 0;

    virtual int32_t DpAclAdd(const int64_t accessControlId, const std::string &udid, const int32_t bindType) = 0;
    virtual int32_t GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId,
                                           int32_t &securityLevel) = 0;
    virtual bool IsSameAccount(const std::string &netWorkId) = 0;
    virtual bool CheckAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee) = 0;
    virtual bool CheckIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee) = 0;
    virtual int32_t GetErrCode(int32_t errCode) = 0;
    virtual int32_t ShiftLNNGear(const std::string &pkgName) = 0;
    virtual int32_t RegDevTrustChangeCallback(const std::string &pkgName,
        std::shared_ptr<DevTrustChangeCallback> callback) = 0;
    virtual int32_t RegisterDeviceScreenStatusCallback(const std::string &pkgName,
        std::shared_ptr<DeviceScreenStatusCallback> callback) = 0;
    virtual int32_t UnRegisterDeviceScreenStatusCallback(const std::string &pkgName) = 0;
    virtual int32_t GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
        int32_t &screenStatus) = 0;

    /**
     * @brief Set Dn Policy
     * @param pkgName  package name.
     * @param policy contain DM_POLICY_STRATEGY_FOR_BLE and DM_POLICY_TIMEOUT key and value.
     *               DM_POLICY_STRATEGY_FOR_BLE: Strategy BLE networking go-online policy, suppress or restore.
     *               DM_POLICY_TIMEOUT: Indicates the duration for suppressing ble networking.
     * @return Returns 0 if success.
     */
    virtual int32_t SetDnPolicy(const std::string &pkgName, std::map<std::string, std::string> &policy) = 0;
    virtual int32_t StopAuthenticateDevice(const std::string &pkgName) = 0;
    virtual int32_t GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid, std::string &networkId) = 0;
    virtual int32_t RegisterCredentialAuthStatusCallback(const std::string &pkgName,
        std::shared_ptr<CredentialAuthStatusCallback> callback) = 0;
    virtual int32_t UnRegisterCredentialAuthStatusCallback(const std::string &pkgName) = 0;
    virtual int32_t RegisterSinkBindCallback(const std::string &pkgName,
        std::shared_ptr<BindTargetCallback> callback) = 0;
    virtual int32_t UnRegisterSinkBindCallback(const std::string &pkgName) = 0;
    virtual int32_t GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual int32_t RegisterAuthenticationType(const std::string &pkgName,
        const std::map<std::string, std::string> &authParam) = 0;

    /**
     * @brief Query the device list of same account.
     * @param filterOptions  query parameters.
     * @param callback       qeury result callback.
     * @return Returns 0 if success.
     */
    virtual int32_t GetDeviceProfileInfoList(const std::string &pkgName,
        const DmDeviceProfileInfoFilterOptions &filterOptions,
        std::shared_ptr<GetDeviceProfileInfoListCallback> callback) = 0;

    virtual int32_t GetDeviceIconInfo(const std::string &pkgName, const DmDeviceIconInfoFilterOptions &filterOptions,
        std::shared_ptr<GetDeviceIconInfoCallback> callback) = 0;
    virtual int32_t PutDeviceProfileInfoList(const std::string &pkgName,
        const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfoList) = 0;
    virtual int32_t GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
        std::string &displayName) = 0;
    virtual int32_t RegisterLocalServiceInfo(const DMLocalServiceInfo &info) = 0;
    virtual int32_t UnRegisterLocalServiceInfo(const std::string &bundleName, int32_t pinExchangeType) = 0;
    virtual int32_t UpdateLocalServiceInfo(const DMLocalServiceInfo &info) = 0;
    virtual int32_t GetLocalServiceInfoByBundleNameAndPinExchangeType(const std::string &bundleName,
        int32_t pinExchangeType, DMLocalServiceInfo &info) = 0;
    virtual int32_t SetLocalDeviceName(const std::string &pkgName, const std::string &deviceName,
        std::shared_ptr<SetLocalDeviceNameCallback> callback) = 0;
    virtual int32_t SetRemoteDeviceName(const std::string &pkgName, const std::string &deviceId,
        const std::string &deviceName, std::shared_ptr<SetRemoteDeviceNameCallback> callback) = 0;
    virtual int32_t RestoreLocalDeviceName(const std::string &pkgName) = 0;
    virtual int32_t GetDeviceNetworkIdList(const std::string &bundleName, const NetworkIdQueryFilter &queryFilter,
        std::vector<std::string> &networkIds) = 0;
    /**
     * @brief UnRegister Pin Code Holder Callback
     * @param pkgName  package name.
     * @return Returns 0 if success.
     */
    virtual int32_t UnRegisterPinHolderCallback(const std::string &pkgName) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DEVICE_MANAGER_H
