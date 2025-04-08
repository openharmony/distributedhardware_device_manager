/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "app_manager.h"
#include "dm_radar_helper.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_common_event_manager.h"
#include "parameter.h"
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
    if (mineHiChainConnector_ == nullptr) {
        mineHiChainConnector_ = std::make_shared<MineHiChainConnector>();
    }
    if (discoveryMgr_ == nullptr) {
        discoveryMgr_ = std::make_shared<DmDiscoveryManager>(softbusConnector_, listener, hiChainConnector_);
    }
    if (publishMgr_ == nullptr) {
        publishMgr_ = std::make_shared<DmPublishManager>(softbusConnector_, listener);
    }
    if (hiChainAuthConnector_ == nullptr) {
        hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    }
    if (deviceStateMgr_ == nullptr) {
        deviceStateMgr_ = std::make_shared<DmDeviceStateManager>(softbusConnector_, listener,
                                                                 hiChainConnector_, hiChainAuthConnector_);
        deviceStateMgr_->RegisterSoftbusStateCallback();
    }
    if (authMgr_ == nullptr) {
        authMgr_ = std::make_shared<DmAuthManager>(softbusConnector_, hiChainConnector_, listener,
            hiChainAuthConnector_);
        softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(authMgr_);
        hiChainConnector_->RegisterHiChainCallback(authMgr_);
        hiChainAuthConnector_->RegisterHiChainAuthCallback(authMgr_);
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
    if (pkgName.empty()) {
        LOGE("StartDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
}

int32_t DeviceManagerServiceImpl::StartDeviceDiscovery(const std::string &pkgName, const uint16_t subscribeId,
    const std::string &filterOptions)
{
    if (pkgName.empty()) {
        LOGE("StartDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
}

int32_t DeviceManagerServiceImpl::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (pkgName.empty()) {
        LOGE("StopDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
}

int32_t DeviceManagerServiceImpl::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo)
{
    if (pkgName.empty()) {
        LOGE("PublishDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return publishMgr_->PublishDeviceDiscovery(pkgName, publishInfo);
}

int32_t DeviceManagerServiceImpl::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (pkgName.empty()) {
        LOGE("UnPublishDeviceDiscovery failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return publishMgr_->UnPublishDeviceDiscovery(pkgName, publishId);
}

int32_t DeviceManagerServiceImpl::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerServiceImpl::AuthenticateDevice failed, pkgName is %{public}s, deviceId is %{public}s,"
            "extra is %{public}s", pkgName.c_str(), GetAnonyString(deviceId).c_str(), extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (deviceStateMgr_ != nullptr) {
        deviceStateMgr_->DeleteOffLineTimer(deviceId);
    }
    return authMgr_->AuthenticateDevice(pkgName, authType, deviceId, extra);
}

int32_t DeviceManagerServiceImpl::UnAuthenticateDevice(const std::string &pkgName, const std::string &networkId)
{
    if (pkgName.empty() || networkId.empty()) {
        LOGE("DeviceManagerServiceImpl::UnAuthenticateDevice failed, pkgName is %{public}s, networkId is %{public}s",
            pkgName.c_str(), GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return authMgr_->UnAuthenticateDevice(pkgName, networkId);
}

int32_t DeviceManagerServiceImpl::StopAuthenticateDevice(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerServiceImpl::StopAuthenticateDevice failed");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return authMgr_->StopAuthenticateDevice(pkgName);
}

int32_t DeviceManagerServiceImpl::BindDevice(const std::string &pkgName, int32_t authType, const std::string &udidHash,
    const std::string &bindParam)
{
    if (pkgName.empty() || udidHash.empty()) {
        LOGE("DeviceManagerServiceImpl::BindDevice failed, pkgName is %{public}s, udidHash is %{public}s, bindParam is"
            "%{public}s", pkgName.c_str(), GetAnonyString(udidHash).c_str(), bindParam.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (deviceStateMgr_ != nullptr) {
        deviceStateMgr_->DeleteOffLineTimer(udidHash);
    }
    return authMgr_->AuthenticateDevice(pkgName, authType, udidHash, bindParam);
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udidHash)
{
    if (pkgName.empty() || udidHash.empty()) {
        LOGE("DeviceManagerServiceImpl::UnBindDevice failed, pkgName is %{public}s, udidHash is %{public}s",
            pkgName.c_str(), GetAnonyString(udidHash).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return authMgr_->UnBindDevice(pkgName, udidHash);
}

int32_t DeviceManagerServiceImpl::SetUserOperation(std::string &pkgName, int32_t action,
    const std::string &params)
{
    if (pkgName.empty() || params.empty()) {
        LOGE("DeviceManagerServiceImpl::SetUserOperation error: Invalid parameter, pkgName: %{public}s, extra:"
            "%{public}s", pkgName.c_str(), params.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authMgr_ != nullptr) {
        authMgr_->OnUserOperation(action, params);
    }
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleOffline(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    LOGI("DeviceManagerServiceImpl::HandleOffline");
    std::string trustDeviceId = deviceStateMgr_->GetUdidByNetWorkId(std::string(devInfo.networkId));
    LOGI("deviceStateMgr Udid: %{public}s", GetAnonyString(trustDeviceId).c_str());
    if (trustDeviceId == "") {
        LOGE("HandleOffline not get udid in deviceStateMgr.");
        return;
    }
    std::string udisHash = softbusConnector_->GetDeviceUdidHashByUdid(trustDeviceId);
    if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udisHash.c_str(), udisHash.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(udisHash).c_str());
        return;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    std::string requestDeviceId = static_cast<std::string>(localUdid);
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().GetOfflineParamFromAcl(trustDeviceId, requestDeviceId);
    LOGI("The offline device bind type is %{public}d.", offlineParam.bindType);
    if (offlineParam.leftAclNumber == 0 && offlineParam.bindType == INVALIED_TYPE) {
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    } else if (offlineParam.bindType == IDENTICAL_ACCOUNT_TYPE) {
        LOGI("The offline device is identical account bind type.");
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    } else if (offlineParam.bindType == DEVICE_PEER_TO_PEER_TYPE) {
        LOGI("The offline device is device-level bind type.");
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    } else if (offlineParam.bindType == APP_PEER_TO_PEER_TYPE) {
        LOGI("The offline device is app-level bind type.");
        softbusConnector_->SetPkgNameVec(offlineParam.pkgNameVec);
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    }
    if (offlineParam.leftAclNumber == 0) {
        LOGI("Delete credential in HandleDeviceOffline.");
        hiChainAuthConnector_->DeleteCredential(trustDeviceId, MultipleUserConnector::GetCurrentAccountUserID());
    }
}

void DeviceManagerServiceImpl::HandleOnline(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    LOGI("DeviceManagerServiceImpl::HandleOnline");
    std::string trustDeviceId = "";
    if (softbusConnector_->GetUdidByNetworkId(devInfo.networkId, trustDeviceId) != DM_OK) {
        LOGE("HandleDeviceOffline get udid failed.");
        return;
    }
    std::string udisHash = softbusConnector_->GetDeviceUdidHashByUdid(trustDeviceId);
    if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udisHash.c_str(), udisHash.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(udisHash).c_str());
        return;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    std::string requestDeviceId = static_cast<std::string>(localUdid);
    uint32_t bindType = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    LOGI("The online device bind type is %{public}d.", bindType);
    if (bindType == INVALIED_TYPE && isCredentialType_.load()) {
        PutIdenticalAccountToAcl(requestDeviceId, trustDeviceId);
        devInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    } else if (bindType == IDENTICAL_ACCOUNT_TYPE) {
        devInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    } else if (bindType == DEVICE_PEER_TO_PEER_TYPE) {
        devInfo.authForm = DmAuthForm::PEER_TO_PEER;
    } else if (bindType == DEVICE_ACROSS_ACCOUNT_TYPE) {
        devInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    } else if (bindType == APP_PEER_TO_PEER_TYPE) {
        std::vector<std::string> pkgNameVec =
            DeviceProfileConnector::GetInstance().GetPkgNameFromAcl(requestDeviceId, trustDeviceId);
        if (pkgNameVec.size() == 0) {
            LOGI("The online device not need report pkgname");
            return;
        }
        softbusConnector_->SetPkgNameVec(pkgNameVec);
        devInfo.authForm = DmAuthForm::PEER_TO_PEER;
    } else if (bindType == APP_ACROSS_ACCOUNT_TYPE) {
        std::vector<std::string> pkgNameVec =
            DeviceProfileConnector::GetInstance().GetPkgNameFromAcl(requestDeviceId, trustDeviceId);
        if (pkgNameVec.size() == 0) {
            LOGI("The online device not need report pkgname");
            return;
        }
        softbusConnector_->SetPkgNameVec(pkgNameVec);
        devInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    }
    LOGI("DeviceManagerServiceImpl::HandleOnline success devInfo auform %{public}d.", devInfo.authForm);
    deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
}

void DeviceManagerServiceImpl::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    if (deviceStateMgr_ == nullptr) {
        LOGE("deviceStateMgr_ is nullpter!");
        return;
    }
    if (devState == DEVICE_STATE_ONLINE) {
        HandleOnline(devState, devInfo);
    } else if (devState == DEVICE_STATE_OFFLINE) {
        HandleOffline(devState, devInfo);
    } else {
        std::string udiddHash = GetUdidHashByNetworkId(devInfo.networkId);
        if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udiddHash.c_str(), udiddHash.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(udiddHash).c_str());
            return;
        }
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    }
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
    std::string peerUdid = "";
    softbusConnector_->GetSoftbusSession()->GetPeerDeviceId(sessionId, peerUdid);
    struct RadarInfo info = {
        .funcName = "OnSessionOpened",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .isTrust = static_cast<int32_t>(TrustStatus::NOT_TRUST),
        .peerUdid = peerUdid,
        .channelId = sessionId,
    };
    if (!DmRadarHelper::GetInstance().ReportAuthSessionOpenCb(info)) {
        LOGE("ReportAuthSessionOpenCb failed");
    }
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
        LOGE("DeviceManagerServiceImpl::ImportCredential failed, pkgName is %{public}s, credentialInfo is %{public}s",
            pkgName.c_str(), GetAnonyString(credentialInfo).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    isCredentialType_.store(true);
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
    isCredentialType_.store(false);
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
    isCredentialType_.store(true);
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
    isCredentialType_.store(false);
    return DM_OK;
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

int32_t DeviceManagerServiceImpl::RegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("RegisterUiStateCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authMgr_ == nullptr) {
        LOGE("authMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return authMgr_->RegisterUiStateCallback(pkgName);
}

int32_t DeviceManagerServiceImpl::UnRegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("RegisterUiStateCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authMgr_ == nullptr) {
        LOGE("authMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return authMgr_->UnRegisterUiStateCallback(pkgName);
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
    LOGI("NotifyEvent begin, pkgName : %{public}s, eventId : %{public}d", pkgName.c_str(), eventId);
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
        if (deviceStateMgr_->ProcNotifyEvent(eventId, deviceId) != DM_OK) {
            LOGE("NotifyEvent failed");
            return ERR_DM_INPUT_PARA_INVALID;
        };
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::GetGroupType(std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("GetGroupType begin");
    if (softbusConnector_ == nullptr || hiChainConnector_ == nullptr) {
        LOGE("softbusConnector_ or hiChainConnector_ is nullptr");
        return ERR_DM_POINT_NULL;
    }

    for (auto it = deviceList.begin(); it != deviceList.end(); ++it) {
        std::string udid = "";
        int32_t ret = softbusConnector_->GetUdidByNetworkId(it->networkId, udid);
        if (ret != DM_OK) {
            LOGE("GetUdidByNetworkId failed ret: %{public}d", ret);
            return ret;
        }
        std::string deviceId = softbusConnector_->GetDeviceUdidHashByUdid(udid);
        if (memcpy_s(it->deviceId, DM_MAX_DEVICE_ID_LEN, deviceId.c_str(), deviceId.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(deviceId).c_str());
        }
        it->authForm = hiChainConnector_->GetGroupType(udid);
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::GetUdidHashByNetWorkId(const char *networkId, std::string &deviceId)
{
    if (softbusConnector_ == nullptr || hiChainConnector_ == nullptr) {
        LOGE("softbusConnector_ or hiChainConnector_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    std::string udid = "";
    int32_t ret = softbusConnector_->GetUdidByNetworkId(networkId, udid);
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId failed ret: %{public}d", ret);
        return ret;
    }
    deviceId = softbusConnector_->GetDeviceUdidHashByUdid(udid);
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (pkgName.empty() || authCode.empty()) {
        LOGE("ImportAuthCode failed, pkgName or authCode is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    return authMgr_->ImportAuthCode(pkgName, authCode);
}

int32_t DeviceManagerServiceImpl::ExportAuthCode(std::string &authCode)
{
    int32_t ret = authMgr_->GeneratePincode();
    authCode = std::to_string(ret);
    LOGI("ExportAuthCode success, authCode: %{public}s.", GetAnonyString(authCode).c_str());
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam)
{
    if (pkgName.empty()) {
        LOGE("BindTarget failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return authMgr_->BindTarget(pkgName, targetId, bindParam);
}

void DeviceManagerServiceImpl::PutIdenticalAccountToAcl(std::string requestDeviceId, std::string trustDeviceId)
{
    LOGI("DeviceManagerServiceImpl::PutIdenticalAccountAcl start.");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    Crypto::GetUdidHash(requestDeviceId, reinterpret_cast<uint8_t *>(localDeviceId));
    std::string localUdidHash = static_cast<std::string>(localDeviceId);
    DmAclInfo aclInfo;
    aclInfo.bindType = IDENTICAL_ACCOUNT;
    aclInfo.trustDeviceId = trustDeviceId;
    aclInfo.authenticationType = ALLOW_AUTH_ALWAYS;
    aclInfo.deviceIdHash = localUdidHash;
    DmAccesser accesser;
    accesser.requestUserId = MultipleUserConnector::GetCurrentAccountUserID();
    accesser.requestAccountId = MultipleUserConnector::GetOhosAccountId();
    MultipleUserConnector::SetSwitchOldUserId(accesser.requestUserId);
    MultipleUserConnector::SetSwitchOldAccountId(accesser.requestAccountId);
    accesser.requestDeviceId = requestDeviceId;
    DmAccessee accessee;
    accessee.trustDeviceId = trustDeviceId;
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, accesser, accessee);
}

int32_t DeviceManagerServiceImpl::DpAclAdd(const std::string &udid)
{
    LOGI("DeviceManagerServiceImpl DpAclAdd start.");
    MultipleUserConnector::SetSwitchOldUserId(MultipleUserConnector::GetCurrentAccountUserID());
    MultipleUserConnector::SetSwitchOldAccountId(MultipleUserConnector::GetOhosAccountId());
    if (deviceStateMgr_->CheckIsOnline(udid)) {
        LOGI("DeviceManagerServiceImpl DpAclAdd identical account and online");
        deviceStateMgr_->OnDeviceOnline(udid, DmAuthForm::IDENTICAL_ACCOUNT);
    }
    LOGI("DeviceManagerServiceImpl::DpAclAdd completed");
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::IsSameAccount(const std::string &udid)
{
    if (udid.empty()) {
        LOGE("DeviceManagerServiceImpl::IsSameAccount error: udid: %{public}s", GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }

    return DeviceProfileConnector::GetInstance().IsSameAccount(udid);
}

std::unordered_map<std::string, DmAuthForm> DeviceManagerServiceImpl::GetAppTrustDeviceIdList(
    std::string pkgname)
{
    char localDeviceId[DEVICE_UUID_LENGTH];
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string deviceId = reinterpret_cast<char *>(localDeviceId);
    return DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgname, deviceId);
}

void DeviceManagerServiceImpl::OnUnbindSessionOpened(int32_t socket, PeerSocketInfo info)
{
    SoftbusSession::OnUnbindSessionOpened(socket, info);
}

void DeviceManagerServiceImpl::OnUnbindSessionCloseed(int32_t socket)
{
    SoftbusSession::OnSessionClosed(socket);
}

void DeviceManagerServiceImpl::OnUnbindBytesReceived(int32_t socket, const void *data, uint32_t dataLen)
{
    SoftbusSession::OnBytesReceived(socket, data, dataLen);
}

void DeviceManagerServiceImpl::LoadHardwareFwkService()
{
    DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
}

void DeviceManagerServiceImpl::AccountCommonEventCallback(int32_t userId, std::string commonEventType)
{
    if (commonEventType == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED ||
        commonEventType == EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT) {
        authMgr_->CommonEventCallback(userId, commonEventType);
        LOGI("DeviceManagerServiceImpl::account event: %{public}s, userId: %{public}s",
            commonEventType.c_str(), GetAnonyInt32(userId).c_str());
        return;
    }
    LOGI("DeviceManagerServiceImpl::AccountCommonEventCallback error.");
}

void DeviceManagerServiceImpl::ScreenCommonEventCallback(std::string commonEventType)
{
    if (commonEventType == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED) {
        LOGI("DeviceManagerServiceImpl::ScreenCommonEventCallback on screen locked.");
        authMgr_->OnScreenLocked();
        return;
    }
    LOGI("DeviceManagerServiceImpl::ScreenCommonEventCallback error.");
}

int32_t DeviceManagerServiceImpl::CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    return DeviceProfileConnector::GetInstance().CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
}

int32_t DeviceManagerServiceImpl::CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    return DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
}

void DeviceManagerServiceImpl::HandleDeviceNotTrust(const std::string &udid)
{
    LOGI("DeviceManagerServiceImpl::HandleDeviceNotTrust udid: %{public}s.", GetAnonyString(udid).c_str());
    if (udid.empty()) {
        LOGE("HandleDeviceNotTrust udid is empty.");
        return;
    }
    CHECK_NULL_VOID(authMgr_);
    authMgr_->HandleDeviceNotTrust(udid);
}

void DeviceManagerServiceImpl::HandleIdentAccountLogout(const std::string &udid, int32_t userId,
    const std::string &accountId)
{
    LOGI("Udid %{public}s, userId %{public}d, accountId %{public}s.", GetAnonyString(udid).c_str(),
        userId, GetAnonyString(accountId).c_str());
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, userId, udid);
    CHECK_NULL_VOID(hiChainConnector_);
    authMgr_->DeleteGroup(DM_PKG_NAME, udid);
}

void DeviceManagerServiceImpl::HandleUserRemoved(int32_t preUserId)
{
    LOGI("PreUserId %{public}d.", preUserId);
    DeviceProfileConnector::GetInstance().DeleteAclForUserRemoved(preUserId);
    CHECK_NULL_VOID(hiChainConnector_);
    hiChainConnector_->DeleteAllGroup(preUserId);
}

std::map<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndBindType(int32_t userId,
    const std::string &accountId)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    return DeviceProfileConnector::GetInstance().GetDeviceIdAndBindType(userId, accountId, localUdid);
}

void DeviceManagerServiceImpl::HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo)
{
    LOGI("In");
    CHECK_NULL_VOID(deviceStateMgr_);
    CHECK_NULL_VOID(softbusConnector_);
    std::string trustDeviceId = "";
    if (softbusConnector_->GetUdidByNetworkId(devInfo.networkId, trustDeviceId) != DM_OK) {
        LOGE("get udid failed.");
        return;
    }
    std::string udidHash = softbusConnector_->GetDeviceUdidHashByUdid(trustDeviceId);
    if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udidHash.c_str(), udidHash.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(udidHash).c_str());
        return;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    std::string requestDeviceId = static_cast<std::string>(localUdid);
    uint32_t bindType = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    LOGI("bind type is %{public}d.", bindType);
    if (bindType == INVALIED_TYPE) {
        return;
    } else if (bindType == IDENTICAL_ACCOUNT_TYPE || bindType == DEVICE_PEER_TO_PEER_TYPE ||
        bindType == DEVICE_ACROSS_ACCOUNT_TYPE) {
        softbusConnector_->ClearPkgName();
        LOGI("networkId: %{public}s", GetAnonyString(devInfo.networkId).c_str());
    } else if (bindType == APP_PEER_TO_PEER_TYPE || bindType == APP_ACROSS_ACCOUNT_TYPE) {
        std::vector<std::string> pkgNameVec =
            DeviceProfileConnector::GetInstance().GetPkgNameFromAcl(requestDeviceId, trustDeviceId);
        if (pkgNameVec.size() == 0) {
            LOGI("not need report pkgname");
            return;
        }
        softbusConnector_->SetPkgNameVec(pkgNameVec);
    }
    deviceStateMgr_->HandleDeviceScreenStatusChange(devInfo);
}

void DeviceManagerServiceImpl::HandleCredentialAuthStatus(const std::string &proofInfo, uint16_t deviceTypeId,
                                                          int32_t errcode)
{
    CHECK_NULL_VOID(deviceStateMgr_);
    deviceStateMgr_->HandleCredentialAuthStatus(proofInfo, deviceTypeId, errcode);
}

int32_t DeviceManagerServiceImpl::RegisterAuthenticationType(int32_t authenticationType)
{
    CHECK_NULL_RETURN(authMgr_, ERR_DM_POINT_NULL);
    return authMgr_->RegisterAuthenticationType(authenticationType);
}

extern "C" IDeviceManagerServiceImpl *CreateDMServiceObject(void)
{
    return new DeviceManagerServiceImpl;
}
} // namespace DistributedHardware
} // namespace OHOS
