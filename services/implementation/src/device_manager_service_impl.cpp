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
#include "dm_radar_helper.h"
#include "multiple_user_connector.h"
#include "app_manager.h"
#include "parameter.h"
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

    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId > 0) {
        LOGI("get current account user id success");
        MultipleUserConnector::SetSwitchOldUserId(userId);
        MultipleUserConnector::SetSwitchOldAccountId(MultipleUserConnector::GetOhosAccountId());
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    SubscribeCommonEvent();
#endif
    LOGI("Init success, singleton initialized");
    return DM_OK;
}

void DeviceManagerServiceImpl::SubscribeCommonEvent()
{
    LOGI("DeviceManagerServiceImpl::SubscribeCommonEvent");
    if (commonEventManager_ == nullptr) {
        commonEventManager_ = std::make_shared<DmCommonEventManager>();
    }
    CommomEventCallback callback = std::bind(&DmAuthManager::CommonEventCallback, *authMgr_.get(),
        std::placeholders::_1);
    std::vector<std::string> commonEvevtVec;
    commonEvevtVec.emplace_back(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    commonEvevtVec.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    if (commonEventManager_->SubscribeServiceEvent(commonEvevtVec, callback)) {
        LOGI("subscribe service user switch common event success");
    }
    return;
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
    struct RadarInfo info = {
        .funcName = "AuthenticateDevice",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverUserRes(info)) {
        LOGE("ReportDiscoverUserRes failed");
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

int32_t DeviceManagerServiceImpl::BindDevice(const std::string &pkgName, int32_t authType, const std::string &udidHash,
    const std::string &bindParam)
{
    if (pkgName.empty() || udidHash.empty()) {
        LOGE("DeviceManagerServiceImpl::BindDevice failed, pkgName is %{public}s, udidHash is %{public}s, bindParam is"
            "%{public}s", pkgName.c_str(), GetAnonyString(udidHash).c_str(), bindParam.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
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
    std::string trustDeviceId = "";
    if (softbusConnector_->GetUdidByNetworkId(devInfo.networkId, trustDeviceId) != DM_OK) {
        LOGE("HandleDeviceOffline get udid failed.");
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
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    std::string requestDeviceId = static_cast<std::string>(localUdid);
    uint32_t bindType = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    LOGI("The online device bind type is %{public}d.", bindType);
    if (bindType == INVALIED_TYPE) {
        LOGI("The online device is identical account.");
        PutIdenticalAccountToAcl(requestDeviceId, trustDeviceId);
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    } else if (bindType == IDENTICAL_ACCOUNT_TYPE) {
        LOGI("The online device is identical account self-networking.");
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    } else if (bindType == DEVICE_PEER_TO_PEER_TYPE) {
        LOGI("The online device is device-level bind type.");
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    } else if (bindType == APP_PEER_TO_PEER_TYPE) {
        LOGI("The online device is app-level bind type.");
        std::vector<std::string> pkgNameVec =
            DeviceProfileConnector::GetInstance().GetPkgNameFromAcl(requestDeviceId, trustDeviceId);
        if (pkgNameVec.size() == 0) {
            LOGI("The online device not need report pkgname");
            return;
        }
        softbusConnector_->SetPkgNameVec(pkgNameVec);
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    }
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
    }
    if (devState == DEVICE_STATE_ONLINE) {
        HandleOnline(devState, devInfo);
    } else if (devState == DEVICE_STATE_OFFLINE) {
        HandleOffline(devState, devInfo);
    } else {
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
            return ERR_DM_FAILED;
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
        return ERR_DM_FAILED;
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
    if (softbusConnector_->CheckIsOnline(udid)) {
        LOGI("DeviceManagerServiceImpl DpAclAdd identical account and online");
        deviceStateMgr_->OnDeviceOnline(udid);
    }
    LOGI("DeviceManagerServiceImpl::DpAclAdd completed");
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::IsSameAccount(const std::string &udid)
{
    if (udid.empty()) {
        LOGE("DeviceManagerServiceImpl::IsSameAccount error: udid is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    return DeviceProfileConnector::GetInstance().IsSameAccount(udid);
}

int32_t DeviceManagerServiceImpl::CheckRelatedDevice(const std::string &udid, const std::string &bundleName)
{
    if (udid.empty() || bundleName.empty()) {
        LOGE("DeviceManagerServiceImpl::CheckRelatedDevice error: udid: %{public}s bundleName: %{public}s",
            GetAnonyString(udid).c_str(), bundleName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DeviceProfileConnector::GetInstance().CheckRelatedDevice(udid, bundleName);
}

std::map<std::string, DmAuthForm> DeviceManagerServiceImpl::GetAppTrustDeviceIdList(std::string pkgname)
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

extern "C" IDeviceManagerServiceImpl *CreateDMServiceObject(void)
{
    return new DeviceManagerServiceImpl;
}
} // namespace DistributedHardware
} // namespace OHOS
