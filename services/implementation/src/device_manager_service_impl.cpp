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

#include "device_manager_service_impl.h"

#include <functional>

#include "dm_constants.h"
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
        if (softbusConnector_ == nullptr) {
            LOGE("Init failed, softbusConnector_ apply for failure");
            return ERR_DM_INIT_FAILED;
        }
    }
    if (hiChainConnector_ == nullptr) {
        hiChainConnector_ = std::make_shared<HiChainConnector>();
        if (hiChainConnector_ == nullptr) {
            LOGE("Init failed, hiChainConnector_ apply for failure");
            return ERR_DM_INIT_FAILED;
        }
    }
    if (deviceStateMgr_ == nullptr) {
        deviceStateMgr_ = std::make_shared<DmDeviceStateManager>(softbusConnector_, listener, hiChainConnector_);
        if (deviceStateMgr_ == nullptr) {
            LOGE("Init failed, deviceStateMgr_ apply for failure");
            return ERR_DM_INIT_FAILED;
        }
        deviceStateMgr_->RegisterSoftbusStateCallback();
    }
    if (discoveryMgr_ == nullptr) {
        discoveryMgr_ = std::make_shared<DmDiscoveryManager>(softbusConnector_, listener);
        if (discoveryMgr_ == nullptr) {
            LOGE("Init failed, discoveryMgr_ apply for failure");
            return ERR_DM_INIT_FAILED;
        }
    }
    if (authMgr_ == nullptr) {
        authMgr_ = std::make_shared<DmAuthManager>(softbusConnector_, listener, hiChainConnector_);
        if (authMgr_ == nullptr) {
            LOGE("Init failed, authMgr_ apply for failure");
            return ERR_DM_INIT_FAILED;
        }
        softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(authMgr_);
        hiChainConnector_->RegisterHiChainCallback(authMgr_);
    }

    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId > 0) {
        LOGI("get current account user id success");
        MultipleUserConnector::SetSwitchOldUserId(userId);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DmCommonEventManager &dmCommonEventManager = DmCommonEventManager::GetInstance();
    CommomEventCallback callback = std::bind(&DmAuthManager::UserSwitchEventCallback, *authMgr_.get(),
        std::placeholders::_1);
    if (dmCommonEventManager.SubscribeServiceEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED, callback)) {
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
    DmCommonEventManager &dmCommonEventManager = DmCommonEventManager::GetInstance();
    if (dmCommonEventManager.UnsubscribeServiceEvent(CommonEventSupport::COMMON_EVENT_USER_STOPPED)) {
        LOGI("subscribe service event success");
    }
#endif
    softbusConnector_->GetSoftbusSession()->UnRegisterSessionCallback();
    hiChainConnector_->UnRegisterHiChainCallback();

    authMgr_ = nullptr;
    deviceStateMgr_ = nullptr;
    discoveryMgr_ = nullptr;
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
        return ERR_DM_INPUT_PARAMETER_EMPTY;
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
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    return discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
}

int32_t DeviceManagerServiceImpl::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("AuthenticateDevice failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    if (deviceId.empty()) {
        LOGE("AuthenticateDevice failed, deviceId is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    return authMgr_->AuthenticateDevice(pkgName, authType, deviceId, extra);
}

int32_t DeviceManagerServiceImpl::UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("UnAuthenticateDevice failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    if (deviceId.empty()) {
        LOGE("UnAuthenticateDevice failed, deviceId is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
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
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    if (authMgr_ != nullptr) {
        authMgr_->GetAuthenticationParam(authParam);
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::SetUserOperation(std::string &pkgName, int32_t action)
{
    if (pkgName.empty()) {
        LOGE("SetUserOperation failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    if (authMgr_ != nullptr) {
        authMgr_->OnUserOperation(action);
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("RegisterDevStateCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
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
        return ERR_DM_INPUT_PARAMETER_EMPTY;
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

extern "C" IDeviceManagerServiceImpl *CreateDMServiceObject(void)
{
    return new DeviceManagerServiceImpl;
}
} // namespace DistributedHardware
} // namespace OHOS