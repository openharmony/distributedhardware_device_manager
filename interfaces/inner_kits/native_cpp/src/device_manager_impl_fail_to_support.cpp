/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "device_manager_impl_fail_to_support.h"
#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
DeviceManagerImplFailToSupport &DeviceManagerImplFailToSupport::GetInstance()
{
    static DeviceManagerImplFailToSupport instance;
    return instance;
}

int32_t DeviceManagerImplFailToSupport::InitDeviceManager(const std::string &pkgName,
    std::shared_ptr<DmInitCallback> dmInitCallback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnInitDeviceManager(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    bool isRefresh, std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetAvailableDeviceList(const std::string &pkgName,
    std::vector<DmDeviceBasicInfo> &deviceList)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceInfo(const std::string &pkgName, const std::string networkId,
    DmDeviceInfo &deviceInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
    std::shared_ptr<DeviceStateCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterDevStatusCallback(const std::string &pkgName, const std::string &extra,
    std::shared_ptr<DeviceStatusCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterDevStateCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterDevStatusCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StartDeviceDiscovery(const std::string &pkgName,
    const DmSubscribeInfo &subscribeInfo, const std::string &extra, std::shared_ptr<DiscoveryCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StartDeviceDiscovery(const std::string &pkgName, uint64_t tokenId,
    const std::string &filterOptions, std::shared_ptr<DiscoveryCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StopDeviceDiscovery(uint64_t tokenId, const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StartDiscovering(const std::string &pkgName,
    std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions,
    std::shared_ptr<DiscoveryCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StopDiscovering(const std::string &pkgName,
    std::map<std::string, std::string> &discoverParam)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::PublishDeviceDiscovery(const std::string &pkgName,
    const DmPublishInfo &publishInfo, std::shared_ptr<PublishCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const DmDeviceInfo &deviceInfo, const std::string &extra, std::shared_ptr<AuthenticateCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnAuthenticateDevice(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StopAuthenticateDevice(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterDeviceManagerFaCallback(const std::string &pkgName,
    std::shared_ptr<DeviceManagerUiCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
    std::shared_ptr<VerifyAuthCallback> callback)
{
    LOGI("device not support");
    (void)pkgName;
    (void)authPara;
    (void)callback;
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetFaParam(const std::string &pkgName, DmAuthParam &dmFaParam)
{
    LOGI("device not support");
    (void)pkgName;
    (void)dmFaParam;
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::SetUserOperation(const std::string &pkgName,
    int32_t action, const std::string &params)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
    std::string &udid)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
    std::string &uuid)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    LOGI("device not support");
    (void)pkgName;
    (void)extra;
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    LOGI("device not support");
    (void)pkgName;
    (void)extra;
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RequestCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterCredentialCallback(const std::string &pkgName,
    std::shared_ptr<CredentialCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterCredentialCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::NotifyEvent(const std::string &pkgName,
    const int32_t eventId, const std::string &event)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RequestCredential(const std::string &pkgName, std::string &returnJsonStr)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetEncryptedUuidByNetworkId(const std::string &pkgName,
    const std::string &networkId, std::string &uuid)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
    const std::string &appId, std::string &encryptedUuid)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::CheckAPIAccessPermission()
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::CheckNewAPIAccessPermission()
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalDeviceNetWorkId(const std::string &pkgName, std::string &networkId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalDeviceId(const std::string &pkgName, std::string &deviceId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalDeviceName(const std::string &pkgName, std::string &deviceName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalDeviceName(std::string &deviceName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalDeviceType(const std::string &pkgName,  int32_t &deviceType)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceName(const std::string &pkgName, const std::string &networkId,
    std::string &deviceName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceType(const std::string &pkgName,
    const std::string &networkId, int32_t &deviceType)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::BindDevice(const std::string &pkgName, int32_t bindType,
    const std::string &deviceId, const std::string &bindParam, std::shared_ptr<AuthenticateCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnBindDevice(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnBindDevice(const std::string &pkgName, const std::string &deviceId,
    const std::string &extra)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetNetworkTypeByNetworkId(const std::string &pkgName,
    const std::string &netWorkId, int32_t &netWorkType)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::ExportAuthCode(std::string &authCode)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::ImportAuthInfo(const DmAuthInfo &dmAuthInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::ExportAuthInfo(DmAuthInfo &dmAuthInfo, uint32_t pinLength)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterAuthCodeInvalidCallback(const std::string &pkgName,
    std::shared_ptr<AuthCodeInvalidCallback> cb)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterAuthCodeInvalidCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterDiscoveryCallback(const std::string &pkgName,
    std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions,
    std::shared_ptr<DiscoveryCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterDiscoveryCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StartAdvertising(const std::string &pkgName,
    std::map<std::string, std::string> &advertiseParam, std::shared_ptr<PublishCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StopAdvertising(const std::string &pkgName,
    std::map<std::string, std::string> &advertiseParam)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    std::map<std::string, std::string> &bindParam, std::shared_ptr<BindTargetCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnbindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    std::map<std::string, std::string> &unbindParam, std::shared_ptr<UnbindTargetCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetTrustedDeviceList(const std::string &pkgName,
    const std::map<std::string, std::string> &filterOptions, bool isRefresh,
    std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterDevStateCallback(const std::string &pkgName,
    const std::map<std::string, std::string> &extraParam, std::shared_ptr<DeviceStateCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::CheckAccessToTarget(uint64_t tokenId, const std::string &targetId)
{
    LOGI("device not support");
    (void)tokenId;
    (void)targetId;
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterPinHolderCallback(const std::string &pkgName,
    std::shared_ptr<PinHolderCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}
int32_t DeviceManagerImplFailToSupport::DpAclAdd(const int64_t accessControlId,
    const std::string &udid, const int32_t bindType)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId,
    int32_t &securityLevel)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

bool DeviceManagerImplFailToSupport::IsSameAccount(const std::string &netWorkId)
{
    LOGI("device not support");
    return false;
}

int32_t DeviceManagerImplFailToSupport::GetErrCode(int32_t errCode)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::ShiftLNNGear(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::SetDnPolicy(const std::string &pkgName,
    std::map<std::string, std::string> &policy)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegDevTrustChangeCallback(const std::string &pkgName,
    std::shared_ptr<DevTrustChangeCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterDeviceScreenStatusCallback(const std::string &pkgName,
    std::shared_ptr<DeviceScreenStatusCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterDeviceScreenStatusCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
    int32_t &screenStatus)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid,
    std::string &networkId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterCredentialAuthStatusCallback(const std::string &pkgName,
    std::shared_ptr<CredentialAuthStatusCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterCredentialAuthStatusCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterSinkBindCallback(const std::string &pkgName,
    std::shared_ptr<BindTargetCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterSinkBindCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterAuthenticationType(const std::string &pkgName,
    const std::map<std::string, std::string> &authParam)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceProfileInfoList(const std::string &pkgName,
    const DmDeviceProfileInfoFilterOptions &filterOptions, std::shared_ptr<GetDeviceProfileInfoListCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceIconInfo(const std::string &pkgName,
    const DmDeviceIconInfoFilterOptions &filterOptions, std::shared_ptr<GetDeviceIconInfoCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::PutDeviceProfileInfoList(const std::string &pkgName,
    const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &deviceProfileInfoList)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
    std::string &displayName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterLocalServiceInfo(const DMLocalServiceInfo &info)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterLocalServiceInfo(const std::string &bundleName,
    int32_t pinExchangeType)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UpdateLocalServiceInfo(const DMLocalServiceInfo &info)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalServiceInfoByBundleNameAndPinExchangeType(
    const std::string &bundleName, int32_t pinExchangeType, DMLocalServiceInfo &info)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::SetLocalDeviceName(const std::string &pkgName, const std::string &deviceName,
    std::shared_ptr<SetLocalDeviceNameCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::SetRemoteDeviceName(const std::string &pkgName, const std::string &deviceId,
    const std::string &deviceName, std::shared_ptr<SetRemoteDeviceNameCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RestoreLocalDeviceName(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetDeviceNetworkIdList(const std::string &bundleName,
    const NetworkIdQueryFilter &queryFilter, std::vector<std::string> &networkIds)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterPinHolderCallback(const std::string &pkgName)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

bool DeviceManagerImplFailToSupport::CheckAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("device not support");
    return false;
}

bool DeviceManagerImplFailToSupport::CheckIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("device not support");
    return false;
}

bool DeviceManagerImplFailToSupport::CheckSrcAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("device not support");
    return false;
}

bool DeviceManagerImplFailToSupport::CheckSinkAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("device not support");
    return false;
}

bool DeviceManagerImplFailToSupport::CheckSrcIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("device not support");
    return false;
}

bool DeviceManagerImplFailToSupport::CheckSinkIsSameAccount(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("device not support");
    return false;
}

int32_t DeviceManagerImplFailToSupport::GetIdentificationByDeviceIds(const std::string &pkgName,
    const std::vector<std::string> deviceIdList, std::map<std::string, std::string> &deviceIdentificationMap)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::BindServiceTarget(const std::string &pkgName, const PeerTargetId &targetId,
    std::map<std::string, std::string> &bindParam, std::shared_ptr<BindTargetCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId,
    std::shared_ptr<ServiceInfoStateCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::LeaveLNN(const std::string &pkgName, const std::string &networkId,
    std::shared_ptr<LeaveLNNCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetAuthTypeByUdidHash(const std::string &udidHash, const std::string &pkgName,
    DMLocalServiceInfoAuthType &authType)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalServiceInfoByBundleNameAndPinExchangeType(const std::string &bundleName,
    int32_t pinExchangeType, DmAuthInfo &dmAuthInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::RegisterServiceInfo(const DmRegisterServiceInfo &regServiceInfo,
    int64_t &serviceId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnRegisterServiceInfo(int64_t serviceId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StartPublishService(const std::string &pkgName, int64_t serviceId,
    const DmPublishServiceParam &publishServiceParam, std::shared_ptr<ServicePublishCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StopPublishService(const std::string &pkgName, int64_t serviceId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StartDiscoveryService(const std::string &pkgName,
    const DmDiscoveryServiceParam &discParam, std::shared_ptr<ServiceDiscoveryCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::StopDiscoveryService(const std::string &pkgName,
    const DmDiscoveryServiceParam &discParam)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::SyncServiceInfoByServiceId(const std::string &pkgName, int32_t localUserId,
    const std::string &networkId, int64_t serviceId, std::shared_ptr<SyncServiceInfoCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::SyncAllServiceInfo(const std::string &pkgName, int32_t localUserId,
    const std::string &networkId, std::shared_ptr<SyncServiceInfoCallback> callback)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::GetLocalServiceInfoByServiceId(int64_t serviceId,
    DmRegisterServiceInfo &serviceInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}
 
int32_t DeviceManagerImplFailToSupport::GetTrustServiceInfo(const std::string &pkgName,
    const std::map<std::string, std::string> &param, std::vector<DmServiceInfo> &serviceInfoList)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}
 
int32_t DeviceManagerImplFailToSupport::GetRegisterServiceInfo(const std::map<std::string, std::string> &param,
    std::vector<DmRegisterServiceInfo> &regServiceInfos)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}
 
int32_t DeviceManagerImplFailToSupport::GetPeerServiceInfoByServiceId(const std::string &networkId, int64_t serviceId,
    DmRegisterServiceInfo& serviceInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UnbindServiceTarget(const std::string &pkgName,
    const std::map<std::string, std::string> &unbindParam, const std::string &netWorkId, int64_t serviceId)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}

int32_t DeviceManagerImplFailToSupport::UpdateServiceInfo(int64_t serviceId,
    const DmRegisterServiceInfo &regServiceInfo)
{
    LOGI("device not support");
    return ERR_DM_DEVICE_NOT_SUPPORT;
}
} // namespace DistributedHardware
} // namespace OHOS
