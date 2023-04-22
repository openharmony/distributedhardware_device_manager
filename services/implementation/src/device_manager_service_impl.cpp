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

#include "device_manager_service_impl.h"

#include <functional>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_distributed_hardware_load.h"
#include "dm_log.h"
#include "multiple_user_connector.h"
#include "permission_manager.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_common_event_manager.h"
#include "common_event_support.h"
using namespace OHOS::EventFwk;
#endif

namespace OHOS {
namespace DistributedHardware {
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
    if (deviceStateMgr_ == nullptr) {
        deviceStateMgr_ = std::make_shared<DmDeviceStateManager>(softbusConnector_, listener, hiChainConnector_);
        deviceStateMgr_->RegisterSoftbusStateCallback();
    }
    if (discoveryMgr_ == nullptr) {
        discoveryMgr_ = std::make_shared<DmDiscoveryManager>(softbusConnector_, listener);
    }
    if (publishMgr_ == nullptr) {
        publishMgr_ = std::make_shared<DmPublishManager>(softbusConnector_, listener);
    }
    if (authMgr_ == nullptr) {
        authMgr_ = std::make_shared<DmAuthManager>(softbusConnector_, listener, hiChainConnector_);
        softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(authMgr_);
        hiChainConnector_->RegisterHiChainCallback(authMgr_);
    }
    if (credentialMgr_ == nullptr) {
        credentialMgr_ = std::make_shared<DmCredentialManager>(hiChainConnector_, listener);
    }

    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId > 0) {
        LOGI("get current account user id success");
        MultipleUserConnector::SetSwitchOldUserId(userId);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (commonEventManager_ == nullptr) {
        commonEventManager_ = std::make_shared<DmCommonEventManager>();
    }
    CommomEventCallback callback = std::bind(&DmAuthManager::UserSwitchEventCallback, *authMgr_.get(),
        std::placeholders::_1);
    if (commonEventManager_->SubscribeServiceEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED, callback)) {
        LOGI("subscribe service user switch common event success");
    }
#endif
    LOGI("Init success, singleton initialized");
    return DM_OK;
}

void DeviceManagerServiceImpl::Release()
{
    LOGI("DeviceManagerServiceImpl Release");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    commonEventManager_ = nullptr;
#endif
    softbusConnector_->GetSoftbusSession()->UnRegisterSessionCallback();
    hiChainConnector_->UnRegisterHiChainCallback();
    authMgr_ = nullptr;
    deviceStateMgr_ = nullptr;
    discoveryMgr_ = nullptr;
    publishMgr_ = nullptr;
    softbusConnector_ = nullptr;
    abilityMgr_ = nullptr;
    hiChainConnector_ = nullptr;
}

int32_t DeviceManagerServiceImpl::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
    const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("StartDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
}

int32_t DeviceManagerServiceImpl::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("StopDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
}

int32_t DeviceManagerServiceImpl::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("PublishDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return publishMgr_->PublishDeviceDiscovery(pkgName, publishInfo);
}

int32_t DeviceManagerServiceImpl::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("UnPublishDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return publishMgr_->UnPublishDeviceDiscovery(pkgName, publishId);
}

int32_t DeviceManagerServiceImpl::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerServiceImpl::AuthenticateDevice failed, pkgName is %s, deviceId is %s, extra is %s",
            pkgName.c_str(), GetAnonyString(deviceId).c_str(), extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return authMgr_->AuthenticateDevice(pkgName, authType, deviceId, extra);
}

int32_t DeviceManagerServiceImpl::UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerServiceImpl::AuthenticateDevice failed, pkgName is %s, deviceId is %s",
            pkgName.c_str(), GetAnonyString(deviceId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return authMgr_->UnAuthenticateDevice(pkgName, deviceId);
}

int32_t DeviceManagerServiceImpl::VerifyAuthentication(const std::string &authParam)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    return authMgr_->VerifyAuthentication(authParam);
}

int32_t DeviceManagerServiceImpl::GetFaParam(std::string &pkgName, DmAuthParam &authParam)
{
    if (pkgName.empty()) {
        LOGE("GetFaParam failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authMgr_ != nullptr) {
        authMgr_->GetAuthenticationParam(authParam);
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::SetUserOperation(std::string &pkgName, int32_t action,
    const std::string &params)
{
    if (pkgName.empty() || params.empty()) {
        LOGE("DeviceManagerServiceImpl::SetUserOperation error: Invalid parameter, pkgName: %s, extra: %s",
            pkgName.c_str(), params.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authMgr_ != nullptr) {
        authMgr_->OnUserOperation(action, params);
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerServiceImpl::RegisterDevStateCallback error: Invalid parameter, pkgName: %s, extra: %s",
            pkgName.c_str(), extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (deviceStateMgr_ != nullptr) {
        deviceStateMgr_->RegisterDevStateCallback(pkgName, extra);
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("UnRegisterDevStateCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (deviceStateMgr_!= nullptr) {
        deviceStateMgr_->UnRegisterDevStateCallback(pkgName, extra);
    }
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleDeviceOnline(const DmDeviceInfo &info)
{
    if (softbusConnector_ != nullptr) {
        softbusConnector_->HandleDeviceOnline(info);
    }
}

void DeviceManagerServiceImpl::HandleDeviceOffline(const DmDeviceInfo &info)
{
    if (softbusConnector_ != nullptr) {
        softbusConnector_->HandleDeviceOffline(info);
    }
}

int DeviceManagerServiceImpl::OnSessionOpened(int sessionId, int result)
{
    return SoftbusSession::OnSessionOpened(sessionId, result);
}

void DeviceManagerServiceImpl::OnSessionClosed(int sessionId)
{
    SoftbusSession::OnSessionClosed(sessionId);
}

void DeviceManagerServiceImpl::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    SoftbusSession::OnBytesReceived(sessionId, data, dataLen);
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
        LOGE("DeviceManagerServiceImpl::ImportCredential failed, pkgName is %s, credentialInfo is %s",
            pkgName.c_str(), credentialInfo.c_str());
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
        LOGE("DeviceManagerServiceImpl::DeleteCredential failed, pkgName is %s, deleteInfo is %s",
            pkgName.c_str(), deleteInfo.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->DeleteCredential(pkgName, deleteInfo);
}

int32_t DeviceManagerServiceImpl::RegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("RegisterCredentialCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_ == nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->RegisterCredentialCallback(pkgName);
}

int32_t DeviceManagerServiceImpl::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("UnRegisterCredentialCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->UnRegisterCredentialCallback(pkgName);
}

int32_t DeviceManagerServiceImpl::PraseNotifyEventJson(const std::string &event, nlohmann::json &jsonObject)
{
    jsonObject = nlohmann::json::parse(event, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("event prase error.");
        return ERR_DM_FAILED;
    }
    if ((!jsonObject.contains("extra")) || (!jsonObject["extra"].is_object())) {
        LOGE("extra error");
        return ERR_DM_FAILED;
    }
    if ((!jsonObject["extra"].contains("deviceId")) || (!jsonObject["extra"]["deviceId"].is_string())) {
        LOGE("NotifyEvent deviceId invalid");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::NotifyEvent(const std::string &pkgName, const int32_t eventId,
    const std::string &event)
{
    LOGI("NotifyEvent begin, pkgName : %s, eventId : %d", pkgName.c_str(), eventId);
    if ((eventId <= DM_NOTIFY_EVENT_START) || (eventId >= DM_NOTIFY_EVENT_BUTT)) {
        LOGE("NotifyEvent eventId invalid");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (eventId == DM_NOTIFY_EVENT_ONDEVICEREADY) {
        nlohmann::json jsonObject;
        if (PraseNotifyEventJson(event, jsonObject) != DM_OK) {
            LOGE("NotifyEvent json invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        std::string deviceId;
        jsonObject["extra"]["deviceId"].get_to(deviceId);
        if (deviceStateMgr_== nullptr) {
            LOGE("deviceStateMgr_ is nullptr");
            return ERR_DM_POINT_NULL;
        }
        if (deviceStateMgr_->ProcNotifyEvent(pkgName, eventId, deviceId) != DM_OK) {
            LOGE("NotifyEvent failed");
            return ERR_DM_INPUT_PARA_INVALID;
        };
    }
    return DM_OK;
}

void DeviceManagerServiceImpl::LoadHardwareFwkService()
{
    DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
}

int32_t DeviceManagerServiceImpl::GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
    std::string &uuid)
{
    if (softbusConnector_ == nullptr) {
        LOGE("softbusConnector_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::GetEncryptedUuid for pkgName = %s", pkgName.c_str());
    int32_t ret = softbusConnector_->GetUuidByNetworkId(networkId.c_str(), uuid);
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId failed, ret : %d", ret);
        return ret;
    }

    std::string appId = Crypto::Sha256(PermissionManager::GetInstance().GetAppId());
    LOGI("appId = %s, uuid = %s.", GetAnonyString(appId).c_str(), GetAnonyString(uuid).c_str());
    uuid = Crypto::Sha256(appId + "_" + uuid);
    LOGI("encryptedUuid = %s.", GetAnonyString(uuid).c_str());
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
    const std::string &appId, std::string &encryptedUuid)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    encryptedUuid = Crypto::Sha256(appId + "_" + uuid);
    LOGI("encryptedUuid = %s.", GetAnonyString(encryptedUuid).c_str());
    return DM_OK;
}

extern "C" IDeviceManagerServiceImpl *CreateDMServiceObject(void)
{
    return new DeviceManagerServiceImpl;
}
} // namespace DistributedHardware
} // namespace OHOS
