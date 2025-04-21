/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "device_manager_service_impl_lite.h"

#include <functional>

#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "app_manager.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint32_t DEVICE_BIUND_LEVEL = 1;
DeviceManagerServiceImpl::DeviceManagerServiceImpl()
{
    LOGI("DeviceManagerServiceImpl constructor");
}

DeviceManagerServiceImpl::~DeviceManagerServiceImpl()
{
    LOGI("DeviceManagerServiceImpl destructor");
}

int32_t DeviceManagerServiceImpl::Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener)
{
    LOGI("DeviceManagerServiceImpl Initialize");
    if (softbusConnector_ == nullptr) {
        softbusConnector_ = std::make_shared<SoftbusConnector>();
    }
    if (hiChainConnector_ == nullptr) {
        hiChainConnector_ = std::make_shared<HiChainConnector>();
    }
    if (mineHiChainConnector_ == nullptr) {
        mineHiChainConnector_ = std::make_shared<MineHiChainConnector>();
    }
    if (hiChainAuthConnector_ == nullptr) {
        hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    }
    if (deviceStateMgr_ == nullptr) {
        deviceStateMgr_ = std::make_shared<DmDeviceStateManager>(softbusConnector_, listener,
                                                                 hiChainConnector_, hiChainAuthConnector_);
    }
    if (credentialMgr_ == nullptr) {
        credentialMgr_ = std::make_shared<DmCredentialManager>(hiChainConnector_, listener);
    }

    LOGI("Init success, singleton initialized");
    return DM_OK;
}

void DeviceManagerServiceImpl::Release()
{
    LOGI("DeviceManagerServiceImpl Release");
    deviceStateMgr_ = nullptr;
    softbusConnector_ = nullptr;
    hiChainConnector_ = nullptr;
    mineHiChainConnector_ = nullptr;
    return;
}

int32_t DeviceManagerServiceImpl::UnAuthenticateDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel)
{
    (void)pkgName;
    (void)udid;
    (void)bindLevel;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel)
{
    (void)pkgName;
    (void)udid;
    (void)bindLevel;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel, const std::string &extra)
{
    (void)pkgName;
    (void)udid;
    (void)bindLevel;
    (void)extra;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::SetUserOperation(std::string &pkgName, int32_t action,
    const std::string &params)
{
    (void)pkgName;
    (void)action;
    (void)params;
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    if (deviceStateMgr_ == nullptr) {
        LOGE("deviceStateMgr_ is nullpter!");
        return;
    }
    std::string deviceId = GetUdidHashByNetworkId(devInfo.networkId);
    if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, deviceId.c_str(), deviceId.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(deviceId).c_str());
        return;
    }
    deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    return;
}

std::string DeviceManagerServiceImpl::GetUdidHashByNetworkId(const std::string &networkId)
{
    if (softbusConnector_ == nullptr) {
        LOGE("softbusConnector_ is nullpter!");
        return "";
    }
    std::string udid = "";
    int32_t ret = softbusConnector_->GetUdidByNetworkId(networkId.c_str(), udid);
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId failed ret: %{public}d", ret);
        return "";
    }
    return softbusConnector_->GetDeviceUdidHashByUdid(udid);
}

int DeviceManagerServiceImpl::OnSessionOpened(int sessionId, int result)
{
    (void)sessionId;
    (void)result;
    return DM_OK;
}

void DeviceManagerServiceImpl::OnSessionClosed(int sessionId)
{
    (void)sessionId;
    return;
}

void DeviceManagerServiceImpl::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    (void)sessionId;
    (void)data;
    (void)dataLen;
    return;
}

int DeviceManagerServiceImpl::OnPinHolderSessionOpened(int sessionId, int result)
{
    (void)sessionId;
    (void)result;
    return DM_OK;
}

void DeviceManagerServiceImpl::OnPinHolderSessionClosed(int sessionId)
{
    (void)sessionId;
    return;
}

void DeviceManagerServiceImpl::OnPinHolderBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    (void)sessionId;
    (void)data;
    (void)dataLen;
    return;
}

int32_t DeviceManagerServiceImpl::RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr)
{
    if (reqJsonStr.empty()) {
        LOGE("reqJsonStr is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->RequestCredential(reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerServiceImpl::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    if (pkgName.empty() || credentialInfo.empty()) {
        LOGE("DeviceManagerServiceImpl::ImportCredential failed, pkgName is %{public}s, credentialInfo is %{public}s",
            pkgName.c_str(), GetAnonyString(credentialInfo).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->ImportCredential(pkgName, credentialInfo);
}

int32_t DeviceManagerServiceImpl::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    if (pkgName.empty() || deleteInfo.empty()) {
        LOGE("DeviceManagerServiceImpl::DeleteCredential failed, pkgName is %{public}s, deleteInfo is %{public}s",
            pkgName.c_str(), GetAnonyString(deleteInfo).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->DeleteCredential(pkgName, deleteInfo);
}

int32_t DeviceManagerServiceImpl::MineRequestCredential(const std::string &pkgName, std::string &returnJsonStr)
{
    (void)pkgName;
    if (mineHiChainConnector_->RequestCredential(returnJsonStr) != DM_OK) {
        LOGE("failed to get device credential from hichain");
        return ERR_DM_HICHAIN_CREDENTIAL_REQUEST_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    (void)pkgName;
    if (reqJsonStr.empty()) {
        LOGE("reqJsonStr is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (mineHiChainConnector_->CheckCredential(reqJsonStr, returnJsonStr) != DM_OK) {
        LOGE("failed to check devices credential status");
        return ERR_DM_HICHAIN_CREDENTIAL_CHECK_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    (void)pkgName;
    if (reqJsonStr.empty()) {
        LOGE("reqJsonStr is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (mineHiChainConnector_->ImportCredential(reqJsonStr, returnJsonStr) != DM_OK) {
        LOGE("failed to import devices credential");
        return ERR_DM_HICHAIN_CREDENTIAL_IMPORT_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    (void)pkgName;
    if (reqJsonStr.empty()) {
        LOGE("reqJsonStr is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (mineHiChainConnector_->DeleteCredential(reqJsonStr, returnJsonStr) != DM_OK) {
        LOGE("failed to delete devices credential");
        return ERR_DM_HICHAIN_CREDENTIAL_DELETE_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::RegisterCredentialCallback(const std::string &pkgName)
{
    (void)pkgName;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::UnRegisterCredentialCallback(const std::string &pkgName)
{
    (void)pkgName;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::RegisterUiStateCallback(const std::string &pkgName)
{
    (void)pkgName;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::UnRegisterUiStateCallback(const std::string &pkgName)
{
    (void)pkgName;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::NotifyEvent(const std::string &pkgName, const int32_t eventId,
    const std::string &event)
{
    (void)pkgName;
    (void)eventId;
    (void)event;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::GetGroupType(std::vector<DmDeviceInfo> &deviceList)
{
    (void)deviceList;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::GetUdidHashByNetWorkId(const char *networkId, std::string &deviceId)
{
    (void)networkId;
    (void)deviceId;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    (void)pkgName;
    (void)authCode;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::ExportAuthCode(std::string &authCode)
{
    (void)authCode;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::RegisterPinHolderCallback(const std::string &pkgName)
{
    (void)pkgName;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    (void)pkgName;
    (void)targetId;
    (void)pinType;
    (void)payload;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    (void)pkgName;
    (void)targetId;
    (void)pinType;
    (void)payload;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam)
{
    (void)pkgName;
    (void)targetId;
    (void)bindParam;
    return DM_OK;
}

std::unordered_map<std::string, DmAuthForm> DeviceManagerServiceImpl::GetAppTrustDeviceIdList(
    std::string pkgname)
{
    (void)pkgname;
    std::unordered_map<std::string, DmAuthForm> tmp;
    return tmp;
}

void DeviceManagerServiceImpl::LoadHardwareFwkService()
{
    return;
}

int32_t DeviceManagerServiceImpl::DpAclAdd(const std::string &udid)
{
    (void)udid;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::IsSameAccount(const std::string &udid)
{
    (void)udid;
    return DM_OK;
}

uint64_t DeviceManagerServiceImpl::GetTokenIdByNameAndDeviceId(std::string pkgName,
    std::string requestDeviceId)
{
    (void)pkgName;
    (void)requestDeviceId;
    return 0;
}

void DeviceManagerServiceImpl::ScreenCommonEventCallback(std::string commonEventType)
{
    (void)commonEventType;
    return;
}

int32_t DeviceManagerServiceImpl::CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    (void)caller;
    (void)srcUdid;
    (void)callee;
    (void)sinkUdid;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    (void)caller;
    (void)srcUdid;
    (void)callee;
    (void)sinkUdid;
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleDeviceNotTrust(const std::string &udid)
{
    (void)udid;
    return;
}

int32_t DeviceManagerServiceImpl::GetBindLevel(const std::string &pkgName, const std::string &localUdid,
    const std::string &udid, uint64_t &tokenId)
{
    (void)pkgName;
    (void)udid;
    (void)tokenId;
    (void)localUdid;
    return DEVICE_BIUND_LEVEL;
}

std::multimap<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndUserId(int32_t userId,
    const std::string &accountId)
{
    (void)userId;
    (void)accountId;
    return std::multimap<std::string, int32_t> {};
}

void DeviceManagerServiceImpl::HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
    const std::string &remoteUdid)
{
    (void)remoteUserId;
    (void)remoteAccountHash;
    (void)remoteUdid;
    return;
}

void DeviceManagerServiceImpl::HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid)
{
    (void)remoteUserId;
    (void)remoteUdid;
    return;
}

void DeviceManagerServiceImpl::HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId)
{
    (void)remoteUserId;
    (void)remoteUdid;
    (void)tokenId;
    return;
}

void DeviceManagerServiceImpl::HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId, int32_t peerTokenId)
{
    (void)remoteUserId;
    (void)remoteUdid;
    (void)tokenId;
    (void)peerTokenId;
    return;
}

void DeviceManagerServiceImpl::HandleIdentAccountLogout(const std::string &localUdid, int32_t localUserId,
    const std::string &peerUdid, int32_t peerUserId)
{
    (void)localUdid;
    (void)localUserId;
    (void)peerUdid;
    (void)peerUserId;
    return;
}

void DeviceManagerServiceImpl::HandleUserRemoved(std::vector<std::string> peerUdids, int32_t preUserId)
{
    (void)peerUdids;
    (void)preUserId;
    return;
}

void DeviceManagerServiceImpl::HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo)
{
    (void)devInfo;
    return;
}

void DeviceManagerServiceImpl::HandleUserSwitched(const std::vector<std::string> &deviceVec,
    int32_t currentUserId, int32_t beforeUserId)
{
    (void)deviceVec;
    (void)currentUserId;
    (void)beforeUserId;
    return;
}

int32_t DeviceManagerServiceImpl::StopAuthenticateDevice(const std::string &pkgName)
{
    (void)pkgName;
    return 0;
}

int32_t DeviceManagerServiceImpl::SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string remoteAclList)
{
    (void)localDevUserInfo;
    (void)remoteDevUserInfo;
    (void)remoteAclList;
    return 0;
}

int32_t DeviceManagerServiceImpl::GetAclListHash(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclList)
{
    (void)localDevUserInfo;
    (void)remoteDevUserInfo;
    (void)aclList;
    return 0;
}

void DeviceManagerServiceImpl::HandleCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId,
    int32_t errcode)
{
    (void)deviceList;
    (void)deviceTypeId;
    (void)errcode;
    return;
}

int32_t DeviceManagerServiceImpl::ProcessAppUnintall(const std::string &appId, int32_t accessTokenId)
{
    (void)appId;
    (void)accessTokenId;
    return 0;
}

void DeviceManagerServiceImpl::HandleSyncUserIdEvent(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid, bool isCheckUserStatus)
{
    (void)foregroundUserIds;
    (void)backgroundUserIds;
    (void)remoteUdid;
    (void)isCheckUserStatus;
    return;
}

void DeviceManagerServiceImpl::HandleRemoteUserRemoved(int32_t preUserId, const std::string &remoteUdid)
{
    (void)preUserId;
    (void)remoteUdid;
    return;
}

std::map<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndBindLevel(int32_t userId)
{
    (void)userId;
    return std::map<std::string, int32_t> {};
}

std::multimap<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndUserId(int32_t localUserId)
{
    (void)localUserId;
    return std::multimap<std::string, int32_t> {};
}

int32_t DeviceManagerServiceImpl::SaveOnlineDeviceInfo(const std::vector<DmDeviceInfo> &deviceList)
{
    (void)deviceList;
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleDeviceUnBind(int32_t bindType, const std::string &peerUdid,
    const std::string &localUdid, int32_t localUserId, const std::string &localAccountId)
{
    (void)bindType;
    (void)peerUdid;
    (void)localUdid;
    (void)localUserId;
    (void)localAccountId;
    return;
}

int32_t DeviceManagerServiceImpl::RegisterAuthenticationType(int32_t authenticationType)
{
    (void)authenticationType;
    return DM_OK;
}

void DeviceManagerServiceImpl::DeleteAlwaysAllowTimeOut()
{
    return;
}

void DeviceManagerServiceImpl::CheckDeleteCredential(const std::string &remoteUdid, int32_t remoteUserId)
{
    (void)remoteUdid;
    (void)remoteUserId;
    return;
}

int32_t DeviceManagerServiceImpl::CheckDeviceInfoPermission(const std::string &localUdid,
    const std::string &peerDeviceId)
{
    (void)localUdid;
    (void)peerDeviceId;
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleServiceUnBindEvent(int32_t userId, const std::string &remoteUdid,
    int32_t remoteTokenId)
{
    (void)userId;
    (void)remoteUdid;
    (void)remoteTokenId;
    return;
}

extern "C" IDeviceManagerServiceImpl *CreateDMServiceObject(void)
{
    return new DeviceManagerServiceImpl;
}
} // namespace DistributedHardware
} // namespace OHOS