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

#include "device_manager_service_impl.h"

#include <chrono>
#include <random>
#include <algorithm>
#include <functional>

#include "app_manager.h"
#include "dm_error_type.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_distributed_hardware_load.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "dm_softbus_cache.h"
#include "multiple_user_connector.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_common_event_manager.h"
#include "parameter.h"
#include "dm_random.h"
#include "common_event_support.h"
using namespace OHOS::EventFwk;
#endif

namespace OHOS {
namespace DistributedHardware {

namespace {

// One year 365 * 24 * 60 * 60
constexpr int32_t MAX_ALWAYS_ALLOW_SECONDS = 31536000;
constexpr int32_t MIN_PIN_CODE = 100000;
constexpr int32_t MAX_PIN_CODE = 999999;
// New protocol field definition. To avoid dependency on the new protocol header file,
// do not directly depend on the new protocol header file.
constexpr int32_t MSG_TYPE_REQ_ACL_NEGOTIATE = 80;
constexpr int32_t MSG_TYPE_RESP_ACL_NEGOTIATE = 90;
constexpr int32_t MSG_TYPE_REQ_AUTH_TERMINATE = 104;
constexpr int32_t AUTH_SRC_FINISH_STATE = 12;
constexpr int32_t MAX_DATA_LEN = 65535;
constexpr int32_t ULTRASONIC_AUTHTYPE = 6;
constexpr const char* DM_TAG_LOGICAL_SESSION_ID = "logicalSessionId";
constexpr const char* DM_TAG_PEER_DISPLAY_ID = "peerDisplayId";
constexpr const char* DM_TAG_ACCESSEE_USER_ID = "accesseeUserId";
constexpr const char* DM_TAG_EXTRA_INFO = "extraInfo";
constexpr const char* CHANGE_PINTYPE = "1";
// currently, we just support one bind session in one device at same time
constexpr size_t MAX_NEW_PROC_SESSION_COUNT_TEMP = 1;
const int32_t USLEEP_TIME_US_500000 = 500000; // 500ms

const std::map<std::string, std::string> BUNDLENAME_MAPPING = {
    { "wear_link_service", "watch_system_service" }
};

static bool IsMessageOldVersion(const JsonObject &jsonObject, std::shared_ptr<Session> session)
{
    std::string dmVersion = "";
    std::string edition = "";
    if (jsonObject[TAG_DMVERSION].IsString()) {
        dmVersion = jsonObject[TAG_DMVERSION].Get<std::string>();
    }
    if (jsonObject[TAG_EDITION].IsString()) {
        edition = jsonObject[TAG_EDITION].Get<std::string>();
    }
    dmVersion = AuthManagerBase::ConvertSrcVersion(dmVersion, edition);

    // Assign the physical session version and release the semaphore.
    session->version_ = dmVersion;

    // If the version number is higher than 5.0.4 (the highest version of the old protocol),
    // there is no need to switch to the old protocol.
    if (CompareVersion(dmVersion, DM_VERSION_5_0_OLD_MAX) == true) {
        return false;
    }

    return true;
}

std::string CreateTerminateMessage(void)
{
    JsonObject jsonObject;
    jsonObject[TAG_MSG_TYPE] = MSG_TYPE_REQ_AUTH_TERMINATE;
    jsonObject[TAG_REPLY] = ERR_DM_VERSION_INCOMPATIBLE;
    jsonObject[TAG_AUTH_FINISH] = false;

    return jsonObject.Dump();
}

}

std::condition_variable DeviceManagerServiceImpl::cleanEventCv_;
std::mutex DeviceManagerServiceImpl::cleanEventMutex_;
std::queue<uint64_t> DeviceManagerServiceImpl::cleanEventQueue_;

Session::Session(int sessionId, std::string deviceId)
{
    sessionId_ = sessionId;
    deviceId_ = deviceId;
}

DeviceManagerServiceImpl::DeviceManagerServiceImpl()
{
    running_ = true;
    thread_ = std::thread(&DeviceManagerServiceImpl::CleanWorker, this);
    LOGI("DeviceManagerServiceImpl constructor");
}

DeviceManagerServiceImpl::~DeviceManagerServiceImpl()
{
    Stop();
    thread_.join();
    LOGI("DeviceManagerServiceImpl destructor");
}

static uint64_t StringToUint64(const std::string& str)
{
    // Calculate the length of the substring, taking the minimum of the string length and 8
    size_t subStrLength = std::min(str.length(), static_cast<size_t>(8U));

    // Extract substring
    std::string substr = str.substr(str.length() - subStrLength);

    // Convert substring to uint64_t
    uint64_t result = 0;
    for (size_t i = 0; i < subStrLength; ++i) {
        result <<= 8; // Shift left 8 bits
        result |= static_cast<uint64_t>(substr[i]);
    }

    return result;
}


static uint64_t GetTokenId(bool isSrcSide, int32_t displayId, std::string &bundleName)
{
    uint64_t tokenId = 0;
    if (isSrcSide) {
        // src end
        tokenId = IPCSkeleton::GetCallingTokenID();
    } else {
        // sink end
        int64_t tmpTokenId;
         // get userId
        int32_t targetUserId = AuthManagerBase::DmGetUserId(displayId);
        if (targetUserId == -1) {
            return tokenId;
        }
        if (AppManager::GetInstance().GetHapTokenIdByName(targetUserId, bundleName, 0, tmpTokenId) == DM_OK) {
            tokenId = static_cast<uint64_t>(tmpTokenId);
        } else if (AppManager::GetInstance().GetNativeTokenIdByName(bundleName, tmpTokenId) == DM_OK) {
            tokenId = static_cast<uint64_t>(tmpTokenId);
        } else {
            // get deviceId, take the 8 character value as tokenId
            char localDeviceId[DEVICE_UUID_LENGTH] = {0};
            GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
            std::string deviceId = std::string(localDeviceId);
            if (deviceId.length() != 0) {
                tokenId = StringToUint64(deviceId);
            }
        }
    }
    return tokenId;
}

uint64_t DeviceManagerServiceImpl::FetchCleanEvent()
{
    std::unique_lock lock(cleanEventMutex_);
    cleanEventCv_.wait(lock, [&] {
        return !running_.load() || !cleanEventQueue_.empty();
    });

    if (!running_.load()) return 0;

    uint64_t logicalSessionId = cleanEventQueue_.front();
    cleanEventQueue_.pop();
    return logicalSessionId;
}

void DeviceManagerServiceImpl::CleanWorker()
{
    while (running_.load()) {
        auto logicalSessionId = FetchCleanEvent();
        LOGI("DeviceManagerServiceImpl::CleanWorker clean auth_mgr, its logicalSessionId: %{public}" PRIu64 "",
            logicalSessionId);
        CleanAuthMgrByLogicalSessionId(logicalSessionId);
    }
    while (!cleanEventQueue_.empty()) {
        uint64_t logicalSessionId = cleanEventQueue_.front();
        cleanEventQueue_.pop();
        CleanAuthMgrByLogicalSessionId(logicalSessionId);
    }
    LOGI("DeviceManagerServiceImpl::CleanWorker end");
}

void DeviceManagerServiceImpl::Stop()
{
    std::lock_guard lock(cleanEventMutex_);
    running_.store(false);
    cleanEventCv_.notify_all();
}

void DeviceManagerServiceImpl::NotifyCleanEvent(uint64_t logicalSessionId)
{
    LOGI("DeviceManagerServiceImpl::NotifyCleanEvent logicalSessionId: %{public}" PRIu64 ".", logicalSessionId);
    std::lock_guard lock(cleanEventMutex_);
    // Store into the queue
    cleanEventQueue_.push(logicalSessionId);
    cleanEventCv_.notify_one();
}

void DeviceManagerServiceImpl::ImportConfig(std::shared_ptr<AuthManagerBase> authMgr, uint64_t tokenId)
{
    // Import configuration
    if (configsMap_.find(tokenId) != configsMap_.end()) {
        authMgr->ImportAuthCode(configsMap_[tokenId]->pkgName, configsMap_[tokenId]->authCode);
        authMgr->RegisterAuthenticationType(configsMap_[tokenId]->authenticationType);
        LOGI("DeviceManagerServiceImpl::ImportConfig import authCode Successful.");
    }
    return;
}

int32_t DeviceManagerServiceImpl::InitAndRegisterAuthMgr(bool isSrcSide, uint64_t tokenId,
    std::shared_ptr<Session> session, uint64_t logicalSessionId)
{
    if (session == nullptr) {
        LOGE("InitAndRegisterAuthMgr, The physical link is not created.");
        return ERR_DM_AUTH_OPEN_SESSION_FAILED;
    }
    // If version is empty, allow creation for the first time, create a new protocol auth_mgr to negotiate version;
    // subsequent creations wait, and directly use version to create the corresponding auth_mgr after release.
    if (session->version_ == "") {
        bool expected = false;
        if (session->flag_.compare_exchange_strong(expected, true)) {
            LOGI("The physical link is being created and the dual-end device version is aligned.");
        } else {
            // Do not allow simultaneous version negotiation, return error directly
            LOGE("Version negotiation is not allowed at the same time.");
            return ERR_DM_AUTH_BUSINESS_BUSY;
        }
    }

    std::lock_guard<std::mutex> lock(authMgrMtx_);
    if (authMgrMap_.find(tokenId) == authMgrMap_.end()) {
        if (session->version_ == "" || CompareVersion(session->version_, DM_VERSION_5_0_OLD_MAX)) {
            if (authMgrMap_.size() > MAX_NEW_PROC_SESSION_COUNT_TEMP) {
                LOGE("Other bind session exist, can not start new one.");
                return ERR_DM_AUTH_BUSINESS_BUSY;
            }
            // Create a new auth_mgr, create authMgrMap_[tokenId]
            if (isSrcSide) {
                // src end
                authMgrMap_[tokenId] = std::make_shared<AuthSrcManager>(softbusConnector_, hiChainConnector_,
                    listener_, hiChainAuthConnector_);
            } else {
                // sink end
                authMgrMap_[tokenId] = std::make_shared<AuthSinkManager>(softbusConnector_, hiChainConnector_,
                    listener_, hiChainAuthConnector_);
            }
            // Register resource destruction notification function
            authMgrMap_[tokenId]->RegisterCleanNotifyCallback(&DeviceManagerServiceImpl::NotifyCleanEvent);
            hiChainAuthConnector_->RegisterHiChainAuthCallbackById(logicalSessionId, authMgrMap_[tokenId]);
            LOGI("DeviceManagerServiceImpl::Initialize authMgrMap_ token: %{public}" PRId64 ".", tokenId);
            ImportConfig(authMgrMap_[tokenId], tokenId);
            return DM_OK;
        } else {
            LOGI("DeviceManagerServiceImpl::InitAndRegisterAuthMgr old authMgr.");
            if (authMgr_ == nullptr) {
                CreateGlobalClassicalAuthMgr();
            }
            authMgr_->PrepareSoftbusSessionCallback();
            authMgrMap_[tokenId] = authMgr_;
            ImportConfig(authMgr_, tokenId);
            // The value of logicalSessionId in the old protocol is always 0.
            logicalSessionId2TokenIdMap_[0] = tokenId;
            return DM_OK;
        }
    }
    // authMgr_ has been created, indicating that a binding event already exists.
    // Other requests are rejected, and an error code is returned.
    LOGE("BindTarget failed, this device is being bound. Please try again later.");
    return ERR_DM_AUTH_BUSINESS_BUSY;
}

void DeviceManagerServiceImpl::CleanSessionMap(int sessionId, std::shared_ptr<Session> session)
{
    session->logicalSessionCnt_.fetch_sub(1);
    if (session->logicalSessionCnt_.load(std::memory_order_relaxed) == 0) {
        usleep(USLEEP_TIME_US_500000);
        softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
        std::lock_guard<std::mutex> lock(mapMutex_);
        if (sessionsMap_.find(sessionId) != sessionsMap_.end()) {
            sessionsMap_[sessionId] = nullptr;
            sessionsMap_.erase(sessionId);
        }
        if (deviceId2SessionIdMap_.find(session->deviceId_) != deviceId2SessionIdMap_.end()) {
            deviceId2SessionIdMap_.erase(session->deviceId_);
        }
    }
    return;
}

void DeviceManagerServiceImpl::CleanSessionMapByLogicalSessionId(uint64_t logicalSessionId)
{
    if (logicalSessionId2SessionIdMap_.find(logicalSessionId) != logicalSessionId2SessionIdMap_.end()) {
        auto sessionId = logicalSessionId2SessionIdMap_[logicalSessionId];
        auto session = GetCurSession(sessionId);
        if (session != nullptr) {
            CleanSessionMap(sessionId, session);
        }
        logicalSessionId2SessionIdMap_.erase(logicalSessionId);
    }
    logicalSessionId2TokenIdMap_.erase(logicalSessionId);
    return;
}

void DeviceManagerServiceImpl::CleanAuthMgrByLogicalSessionId(uint64_t logicalSessionId)
{
    uint64_t tokenId = 0;
    if (logicalSessionId2TokenIdMap_.find(logicalSessionId) != logicalSessionId2TokenIdMap_.end()) {
        tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
    } else {
        LOGE("logicalSessionId(%{public}" PRIu64 ") can not find the tokenId.", logicalSessionId);
        return;
    }

    if (configsMap_.find(tokenId) != configsMap_.end()) {
        configsMap_[tokenId] = nullptr;
        configsMap_.erase(tokenId);
    }

    CleanSessionMapByLogicalSessionId(logicalSessionId);
    if (logicalSessionId == 0 && authMgr_ != nullptr) {
        authMgr_->SetTransferReady(true);
        authMgr_->ClearSoftbusSessionCallback();
    }
    hiChainAuthConnector_->UnRegisterHiChainAuthCallbackById(logicalSessionId);
    if (authMgrMap_.find(tokenId) != authMgrMap_.end()) {
        authMgrMap_[tokenId] = nullptr;
        authMgrMap_.erase(tokenId);
    }
    return;
}

std::shared_ptr<AuthManagerBase> DeviceManagerServiceImpl::GetAuthMgr()
{
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (authMgrMap_.find(tokenId) != authMgrMap_.end()) {
        LOGI("DeviceManagerServiceImpl::GetAuthMgr authMgrMap_ token: %{public}" PRId64 ".", tokenId);
        return authMgrMap_[tokenId];
    }
    LOGE("DeviceManagerServiceImpl::GetAuthMgr authMgrMap_ not found, token: %{public}" PRId64 ".", tokenId);
    return nullptr;
}

// Needed in the callback function
std::shared_ptr<AuthManagerBase> DeviceManagerServiceImpl::GetAuthMgrByTokenId(uint64_t tokenId)
{
    if (authMgrMap_.find(tokenId) != authMgrMap_.end()) {
        LOGI("DeviceManagerServiceImpl::GetAuthMgrByTokenId authMgrMap_ token: %{public}" PRId64 ".", tokenId);
        return authMgrMap_[tokenId];
    }
    LOGE("DeviceManagerServiceImpl::GetAuthMgrByTokenId authMgrMap_ not found, token: %{public}" PRId64 ".", tokenId);
    return nullptr;
}

std::shared_ptr<AuthManagerBase> DeviceManagerServiceImpl::GetCurrentAuthMgr()
{
    uint64_t tokenId = 0;
    if (logicalSessionId2TokenIdMap_.find(0) != logicalSessionId2TokenIdMap_.end()) {
        tokenId = logicalSessionId2TokenIdMap_[0];
    }
    for (auto &pair : authMgrMap_) {
        if (pair.first != tokenId) {
            return pair.second;
        }
    }
    return authMgr_;
}

static uint64_t GenerateRandNum(int sessionId)
{
    // Get the current timestamp
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().
        time_since_epoch()).count();

    // Generate random numbers
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rand_dis(1, 0xFFFFFFFF);
    uint32_t randomNumber = rand_dis(gen);

    // Combination of random numbers
    uint64_t randNum = (static_cast<uint64_t>(timestamp) << 32) |
                      (static_cast<uint64_t>(sessionId) << 16) |
                      static_cast<uint64_t>(randomNumber);

    return randNum;
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
        deviceStateMgr_->RegisterSoftbusStateCallback();
    }
    if (credentialMgr_ == nullptr) {
        credentialMgr_ = std::make_shared<DmCredentialManager>(hiChainConnector_, listener);
    }
    if (dpInitedCallback_ == nullptr) {
        dpInitedCallback_ = sptr<DpInitedCallback>(new DpInitedCallback());
        DeviceProfileConnector::GetInstance().SubscribeDeviceProfileInited(dpInitedCallback_);
    }
    listener_ = listener;
    CreateGlobalClassicalAuthMgr();
    if (authMgr_ != nullptr) {
        authMgr_->ClearSoftbusSessionCallback();
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
    softbusConnector_->UnRegisterConnectorCallback();
    softbusConnector_->UnRegisterSoftbusStateCallback();
    softbusConnector_->GetSoftbusSession()->UnRegisterSessionCallback();
    hiChainConnector_->UnRegisterHiChainCallback();
    hiChainAuthConnector_->UnRegisterHiChainAuthCallback();
    authMgr_ = nullptr;
    for (auto& pair : authMgrMap_) {
        pair.second = nullptr;
    }
    authMgrMap_.clear();
    for (auto& pair : sessionsMap_) {
        pair.second = nullptr;
    }
    sessionsMap_.clear();
    for (auto& pair : configsMap_) {
        pair.second = nullptr;
    }
    configsMap_.clear();
    deviceId2SessionIdMap_.clear();
    deviceIdMutexMap_.clear();
    sessionEnableMutexMap_.clear();
    sessionEnableCvMap_.clear();
    logicalSessionId2TokenIdMap_.clear();
    logicalSessionId2SessionIdMap_.clear();
    deviceStateMgr_ = nullptr;
    softbusConnector_ = nullptr;
    abilityMgr_ = nullptr;
    hiChainConnector_ = nullptr;
    DeviceProfileConnector::GetInstance().UnSubscribeDeviceProfileInited();
    dpInitedCallback_ = nullptr;
}

int32_t DeviceManagerServiceImpl::UnAuthenticateDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel)
{
    if (pkgName.empty() || udid.empty()) {
        LOGE("DeviceManagerServiceImpl::UnAuthenticateDevice failed, pkgName is %{public}s, udid is %{public}s",
            pkgName.c_str(), GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto authMgr = GetAuthMgr();
    if (authMgr == nullptr) {
        LOGE("authMgr is nullptr, invoke the old protocal.");
        if (authMgr_ == nullptr) {
            LOGE("classical authMgr_ is nullptr");
            return ERR_DM_POINT_NULL;
        }
        return authMgr_->UnAuthenticateDevice(pkgName, udid, bindLevel);
    }
    return authMgr->UnAuthenticateDevice(pkgName, udid, bindLevel);
}

int32_t DeviceManagerServiceImpl::StopAuthenticateDevice(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerServiceImpl::StopAuthenticateDevice failed");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto authMgr = GetAuthMgr();
    if (authMgr == nullptr) {
        LOGE("authMgr is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return authMgr->StopAuthenticateDevice(pkgName);
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel)
{
    if (pkgName.empty() || udid.empty()) {
        LOGE("DeviceManagerServiceImpl::UnBindDevice failed, pkgName is %{public}s, udid is %{public}s",
            pkgName.c_str(), GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string extra = "";
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    return DeleteAclV2(pkgName, std::string(localDeviceId), udid, bindLevel, extra);
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel, const std::string &extra)
{
    if (pkgName.empty() || udid.empty()) {
        LOGE("DeviceManagerServiceImpl::UnBindDevice failed, pkgName is %{public}s, udid is %{public}s",
            pkgName.c_str(), GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    return DeleteAclV2(pkgName, std::string(localDeviceId), udid, bindLevel, extra);
}

int32_t DeviceManagerServiceImpl::SetUserOperation(std::string &pkgName, int32_t action,
    const std::string &params)
{
    if (pkgName.empty() || params.empty()) {
        LOGE("DeviceManagerServiceImpl::SetUserOperation error: Invalid parameter, pkgName: %{public}s, extra:"
            "%{public}s", pkgName.c_str(), params.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto authMgr = GetCurrentAuthMgr();
    if (authMgr != nullptr) {
        authMgr->OnUserOperation(action, params);
    }
    return DM_OK;
}

void DeviceManagerServiceImpl::CreateGlobalClassicalAuthMgr()
{
    LOGI("global classical authMgr_ not exit, create one");
    // Create old auth_mar, only create an independent one
    authMgr_ = std::make_shared<DmAuthManager>(softbusConnector_, hiChainConnector_, listener_,
        hiChainAuthConnector_);
    authMgr_->RegisterCleanNotifyCallback(&DeviceManagerServiceImpl::NotifyCleanEvent);
    softbusConnector_->RegisterConnectorCallback(authMgr_);
    softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(authMgr_);
    hiChainConnector_->RegisterHiChainCallback(authMgr_);
    hiChainAuthConnector_->RegisterHiChainAuthCallback(authMgr_);
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
    std::string requestDeviceId = std::string(localUdid);
    std::map<int32_t, int32_t> userIdAndBindLevel =
        DeviceProfileConnector::GetInstance().GetUserIdAndBindLevel(requestDeviceId, trustDeviceId);
    ProcessInfo processInfo;
    processInfo.pkgName = std::string(DM_PKG_NAME);
    processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
    if (userIdAndBindLevel.empty() || userIdAndBindLevel.find(processInfo.userId) == userIdAndBindLevel.end()) {
        userIdAndBindLevel[processInfo.userId] = INVALIED_TYPE;
    }
    for (const auto &item : userIdAndBindLevel) {
        if (static_cast<uint32_t>(item.second) == INVALIED_TYPE) {
            LOGI("The offline device is identical account bind type.");
            devInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
            processInfo.userId = item.first;
            softbusConnector_->SetProcessInfo(processInfo);
        } else if (static_cast<uint32_t>(item.second) == USER) {
            LOGI("The offline device is device bind type.");
            devInfo.authForm = DmAuthForm::PEER_TO_PEER;
            processInfo.userId = item.first;
            softbusConnector_->SetProcessInfo(processInfo);
        } else if (static_cast<uint32_t>(item.second) == SERVICE || static_cast<uint32_t>(item.second) == APP) {
            LOGI("The offline device is PEER_TO_PEER_TYPE bind type, %{public}" PRIu32, item.second);
            std::vector<ProcessInfo> processInfoVec =
                DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(requestDeviceId, trustDeviceId,
                    item.first);
            softbusConnector_->SetProcessInfoVec(processInfoVec);
        }
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
    }
}

void DeviceManagerServiceImpl::HandleOnline(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    LOGI("DeviceManagerServiceImpl::HandleOnline networkId: %{public}s.",
        GetAnonyString(devInfo.networkId).c_str());
    std::string trustDeviceId = "";
    if (softbusConnector_->GetUdidByNetworkId(devInfo.networkId, trustDeviceId) != DM_OK) {
        LOGE("HandleOnline get udid failed.");
        return;
    }
    std::string udisHash = softbusConnector_->GetDeviceUdidHashByUdid(trustDeviceId);
    if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udisHash.c_str(), udisHash.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(udisHash).c_str());
        return;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    std::string requestDeviceId = std::string(localUdid);
    uint32_t bindType = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    LOGI("The online device bind type is %{public}" PRIu32, bindType);
    ProcessInfo processInfo;
    processInfo.pkgName = std::string(DM_PKG_NAME);
    processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
    if (bindType == IDENTICAL_ACCOUNT_TYPE) {
        devInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
        softbusConnector_->SetProcessInfo(processInfo);
    } else if (bindType == DEVICE_PEER_TO_PEER_TYPE) {
        devInfo.authForm = DmAuthForm::PEER_TO_PEER;
        softbusConnector_->SetProcessInfo(processInfo);
    } else if (bindType == DEVICE_ACROSS_ACCOUNT_TYPE) {
        devInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
        softbusConnector_->SetProcessInfo(processInfo);
    } else if (bindType == APP_PEER_TO_PEER_TYPE || bindType == SERVICE_PEER_TO_PEER_TYPE) {
        std::vector<ProcessInfo> processInfoVec =
            DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(requestDeviceId, trustDeviceId,
                MultipleUserConnector::GetFirstForegroundUserId());
        softbusConnector_->SetProcessInfoVec(processInfoVec);
        devInfo.authForm = DmAuthForm::PEER_TO_PEER;
    } else if (bindType == APP_ACROSS_ACCOUNT_TYPE || bindType == SERVICE_ACROSS_ACCOUNT_TYPE) {
        std::vector<ProcessInfo> processInfoVec =
            DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(requestDeviceId, trustDeviceId,
                MultipleUserConnector::GetFirstForegroundUserId());
        softbusConnector_->SetProcessInfoVec(processInfoVec);
        devInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    }
    LOGI("DeviceManagerServiceImpl::HandleOnline success devInfo auform %{public}d.", devInfo.authForm);
    deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo);
}

void DeviceManagerServiceImpl::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    LOGI("DeviceManagerServiceImpl::HandleDeviceStatusChange start, devState = %{public}d, networkId: %{public}s.",
        devState, GetAnonyString(devInfo.networkId).c_str());
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
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        softbusConnector_->SetProcessInfo(processInfo);
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
    {
        std::lock_guard<std::mutex> lock(sessionEnableMutexMap_[sessionId]);
        if (result == 0) {
            sessionEnableCvReadyMap_[sessionId] = true;
            LOGE("OnSessionOpened successful, sessionId: %{public}d", sessionId);
        } else {
            LOGE("OnSessionOpened failed, sessionId: %{public}d, res: %{public}d", sessionId, result);
        }
        sessionEnableCvMap_[sessionId].notify_all();
    }
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

    // Get the remote deviceId, sink end gives sessionsMap[deviceId] = session;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        if (sessionsMap_.find(sessionId) == sessionsMap_.end()) {
            sessionsMap_[sessionId] = std::make_shared<Session>(sessionId, peerUdid);
        }
    }

    return SoftbusSession::OnSessionOpened(sessionId, result);
}

void DeviceManagerServiceImpl::OnSessionClosed(int sessionId)
{
    SoftbusSession::OnSessionClosed(sessionId);
}

static JsonObject GetJsonObjectFromData(const void *data, unsigned int dataLen)
{
    std::string message = std::string(reinterpret_cast<const char *>(data), dataLen);
    return JsonObject(message);
}

// When downgrading the version, determine whether it is src or sink based on the message.
// src: Received 90 message.
// sink: Received 80 message.
static bool IsAuthManagerSourceByMessage(int32_t msgType)
{
    return msgType == MSG_TYPE_RESP_ACL_NEGOTIATE;
}

// Get the current session object
std::shared_ptr<Session> DeviceManagerServiceImpl::GetCurSession(int sessionId)
{
    std::shared_ptr<Session> curSession = nullptr;
    // Get the remote deviceId, sink end gives sessionsMap[deviceId] = session;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        if (sessionsMap_.find(sessionId) != sessionsMap_.end()) {
            curSession = sessionsMap_[sessionId];
        } else {
            LOGE("OnBytesReceived, The local session cannot be found.");
        }
    }
    return curSession;
}

std::shared_ptr<AuthManagerBase> DeviceManagerServiceImpl::GetAuthMgrByMessage(int32_t msgType,
    uint64_t logicalSessionId, const JsonObject &jsonObject, std::shared_ptr<Session> curSession)
{
    uint64_t tokenId = 0;
    if (msgType == MSG_TYPE_REQ_ACL_NEGOTIATE) {
        std::string bundleName;
        int32_t displayId = 0;
        if (jsonObject[TAG_PEER_BUNDLE_NAME_V2].IsString()) {
            bundleName = jsonObject[TAG_PEER_BUNDLE_NAME_V2].Get<std::string>();
        }
        if (jsonObject[DM_TAG_PEER_DISPLAY_ID].IsNumberInteger()) {
            displayId = jsonObject[DM_TAG_PEER_DISPLAY_ID].Get<int32_t>();
        }
        tokenId = GetTokenId(false, displayId, bundleName);
        if (tokenId == 0) {
            LOGE("GetAuthMgrByMessage, Get tokenId failed.");
            return nullptr;
        }
        if (InitAndRegisterAuthMgr(false, tokenId, curSession, logicalSessionId) != DM_OK) {
            return nullptr;
        }
        curSession->logicalSessionSet_.insert(logicalSessionId);
        if (logicalSessionId2TokenIdMap_.find(logicalSessionId) != logicalSessionId2TokenIdMap_.end()) {
            LOGE("GetAuthMgrByMessage, logicalSessionId exists in logicalSessionId2TokenIdMap_.");
            return nullptr;
        }
        logicalSessionId2TokenIdMap_[logicalSessionId] = tokenId;
    } else {
        if (curSession->logicalSessionSet_.find(logicalSessionId) == curSession->logicalSessionSet_.end()) {
            LOGE("GetAuthMgrByMessage, The logical session ID does not exist in the physical session.");
            return nullptr;
        }
        tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
    }

    return GetAuthMgrByTokenId(tokenId);
}

int32_t DeviceManagerServiceImpl::TransferSrcOldAuthMgr(std::shared_ptr<Session> curSession)
{
    // New Old Receive 90, destroy new authMgr, create old authMgr, source side
    // The old protocol has only one session, reverse lookup logicalSessionId and tokenId
    int sessionId = curSession->sessionId_;
    uint64_t logicalSessionId = 0;
    uint64_t tokenId = 0;
    for (auto& pair : logicalSessionId2SessionIdMap_) {
        if (pair.second == sessionId) {
            logicalSessionId = pair.first;
            tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
        }
    }
    if (logicalSessionId == 0 || tokenId == 0) {
        LOGE("DeviceManagerServiceImpl::TransferSrcOldAuthMgr can not find logicalSessionId and tokenId.");
        return ERR_DM_AUTH_FAILED;
    }
    std::string pkgName;
    PeerTargetId peerTargetId;
    std::map<std::string, std::string> bindParam;
    auto authMgr = GetAuthMgrByTokenId(tokenId);
    authMgr->GetBindTargetParams(pkgName, peerTargetId, bindParam);
    DmBindCallerInfo callerInfo;
    authMgr->GetCallerInfo(callerInfo);
    int32_t authType = -1;
    authMgr->ParseAuthType(bindParam, authType);
    authMgrMap_.erase(tokenId);
    if (InitAndRegisterAuthMgr(true, tokenId, curSession, logicalSessionId) != DM_OK) {
        return ERR_DM_AUTH_FAILED;
    }

    int ret = TransferByAuthType(authType, curSession, authMgr, bindParam, logicalSessionId);
    if (ret != DM_OK) {
        LOGE("DeviceManagerServiceImpl::TransferByAuthType TransferByAuthType failed.");
        return ret;
    }
    authMgr = nullptr;
    authMgr_->SetCallerInfo(callerInfo);
    if (authMgr_->BindTarget(pkgName, peerTargetId, bindParam, sessionId, 0) != DM_OK) {
        LOGE("DeviceManagerServiceImpl::TransferSrcOldAuthMgr authManager BindTarget failed");
        return ERR_DM_AUTH_FAILED;
    }

    if (authType == DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE) {
        int32_t sessionSide = GetSessionSide(curSession->sessionId_);
        authMgr_->OnSessionOpened(curSession->sessionId_, sessionSide, 0);
    }

    LOGI("DeviceManagerServiceImpl::TransferSrcOldAuthMgr src transfer to old version success");
    authMgr_->SetTransferReady(false);
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::TransferByAuthType(int32_t authType,
    std::shared_ptr<Session> curSession, std::shared_ptr<AuthManagerBase> authMgr,
    std::map<std::string, std::string> &bindParam, uint64_t logicalSessionId)
{
    int sessionId = curSession->sessionId_;
    if (authType == DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE) {
        authMgr_->EnableInsensibleSwitching();
        curSession->logicalSessionSet_.insert(0);
        curSession->logicalSessionCnt_.fetch_add(1);
        logicalSessionId2SessionIdMap_[0] = sessionId;
        authMgr->OnSessionDisable();
    } else {
        authMgr_->DisableInsensibleSwitching();
        // send stop message
        // Cannot stop using the new protocol. The new protocol is a signal mechanism and cannot be stopped serially.
        // There will be a delay, causing new objects to be created before the stop is complete.
        // Then the timeout mechanism of the new protocol will stop SoftBus again.
        std::string endMessage = CreateTerminateMessage();
        (void)softbusConnector_->GetSoftbusSession()->SendData(sessionId, endMessage);
        // Close new protocol session
        CleanAuthMgrByLogicalSessionId(logicalSessionId);
    }
    if (authType == ULTRASONIC_AUTHTYPE) {
        int32_t ret = ChangeUltrasonicTypeToPin(bindParam);
        if (ret != DM_OK) {
            LOGE("DeviceManagerServiceImpl::TransferSrcOldAuthMgr ChangeUltrasonicTypeToPin failed.");
            return ret;
        }
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::ChangeUltrasonicTypeToPin(std::map<std::string, std::string> &bindParam)
{
    auto iter = bindParam.find(PARAM_KEY_AUTH_TYPE);
    if (iter == bindParam.end()) {
        LOGE("DeviceManagerServiceImpl::ChangeUltrasonicTypeToPin bindParam:%{public}s not exist.",
            PARAM_KEY_AUTH_TYPE);
        return ERR_DM_AUTH_FAILED;
    }
    iter->second = CHANGE_PINTYPE;
    LOGI("DeviceManagerServiceImpl::ChangeUltrasonicTypeToPin bindParam:%{public}s PINTYPE.", PARAM_KEY_AUTH_TYPE);
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::TransferSinkOldAuthMgr(const JsonObject &jsonObject,
    std::shared_ptr<Session> curSession)
{
    // Old New Received 80, New Old authMgr, Sink End
    std::string bundleName;
    if (jsonObject[TAG_BUNDLE_NAME].IsString()) {
        bundleName = jsonObject[TAG_BUNDLE_NAME].Get<std::string>();
    } else if (jsonObject[TAG_PEER_BUNDLE_NAME].IsString()) {
        bundleName = jsonObject[TAG_PEER_BUNDLE_NAME].Get<std::string>();
    } else if (jsonObject[TAG_HOST_PKGLABEL].IsString()) {
        bundleName = jsonObject[TAG_HOST_PKGLABEL].Get<std::string>();
    } else {
        LOGE("DeviceManagerServiceImpl::TransferSinkOldAuthMgr can not find bundleName.");
        return ERR_DM_AUTH_FAILED;
    }
    if (softbusConnector_ == nullptr) {
        LOGE("softbusConnector_ is nullpter!");
        return ERR_DM_AUTH_FAILED;
    }
    int32_t deviceType = softbusConnector_->GetLocalDeviceTypeId();
    if (deviceType == DmDeviceType::DEVICE_TYPE_WATCH &&
        BUNDLENAME_MAPPING.find(bundleName) != BUNDLENAME_MAPPING.end()) {
        bundleName = BUNDLENAME_MAPPING.find(bundleName)->second;
    }
    uint64_t tokenId = GetTokenId(false, -1, bundleName);
    if (InitAndRegisterAuthMgr(false, tokenId, curSession, 0) != DM_OK) {
        // Internal error log printing completed
        return ERR_DM_AUTH_FAILED;
    }

    // Parameter 2 sessionSide is 0, authMgr_ is empty, it must be the sink end.
    // The src end will create the protocol object when BindTarget.
    authMgr_->OnSessionOpened(curSession->sessionId_, 0, 0);
    LOGI("DeviceManagerServiceImpl::TransferSinkOldAuthMgr sink transfer to old version success");
    authMgr_->SetTransferReady(false);
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::TransferOldAuthMgr(int32_t msgType, const JsonObject &jsonObject,
    std::shared_ptr<Session> curSession)
{
    int ret = DM_OK;
    if ((authMgr_ == nullptr || authMgr_->IsTransferReady()) &&
        (msgType == MSG_TYPE_REQ_ACL_NEGOTIATE || msgType == MSG_TYPE_RESP_ACL_NEGOTIATE)) {
        if (IsMessageOldVersion(jsonObject, curSession)) {
            if (IsAuthManagerSourceByMessage(msgType)) {
                ret = TransferSrcOldAuthMgr(curSession);
            } else {
                ret = TransferSinkOldAuthMgr(jsonObject, curSession);
            }
        }
    }

    return ret;
}


void DeviceManagerServiceImpl::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (sessionId < 0 || data == nullptr || dataLen <= 0 || dataLen > MAX_DATA_LEN) {
        LOGE("[OnBytesReceived] Fail to receive data from softbus with sessionId: %{public}d, dataLen: %{public}d.",
            sessionId, dataLen);
        return;
    }

    LOGI("start, sessionId: %{public}d, dataLen: %{public}d.", sessionId, dataLen);

    JsonObject jsonObject = GetJsonObjectFromData(data, dataLen);
    if (jsonObject.IsDiscarded() || !IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("OnBytesReceived, MSG_TYPE parse failed.");
        return;
    }
    int32_t msgType = jsonObject[TAG_MSG_TYPE].Get<int32_t>();
    uint64_t logicalSessionId = 0;
    if (IsUint64(jsonObject, DM_TAG_LOGICAL_SESSION_ID)) {
        logicalSessionId = jsonObject[DM_TAG_LOGICAL_SESSION_ID].Get<std::uint64_t>();
    }

    std::shared_ptr<Session> curSession = GetCurSession(sessionId);
    if (curSession == nullptr) {
        LOGE("InitAndRegisterAuthMgr, The physical link is not created.");
        return;
    }

    std::shared_ptr<AuthManagerBase> authMgr = nullptr;
    if  (logicalSessionId != 0) {
        authMgr = GetAuthMgrByMessage(msgType, logicalSessionId, jsonObject, curSession);
        if (authMgr == nullptr) {
            return;
        }
    } else {
        /**
        Monitor old messages on ports 80/90
        1. New-to-old: When the src side receives a 90 message and detects a version mismatch, it receives
        the 90 message, destroys the current new authMgr, creates a new old protocol authMgr, and re-BindTarget.
        2. Old-to-new: When the sink side receives an 80 message and detects a version mismatch, it receives the 80
        message, directly creates a new old protocol authMgr, and re-OnSessionOpened and OnBytesReceived.
        */
        if (curSession->version_ == "" || CompareVersion(curSession->version_, DM_VERSION_5_0_OLD_MAX)) {
            if (TransferOldAuthMgr(msgType, jsonObject, curSession) != DM_OK) {
                LOGE("DeviceManagerServiceImpl::OnBytesReceived TransferOldAuthMgr failed");
                return;
            }
        } else {
            LOGI("DeviceManagerServiceImpl::OnBytesReceived Reuse Old AuthMgr, sessionId: %{public}d.", sessionId);
        }
        authMgr = authMgr_;
    }

    std::string message = std::string(reinterpret_cast<const char *>(data), dataLen);
    if (msgType == AUTH_DEVICE_REQ_NEGOTIATE || msgType == AUTH_DEVICE_RESP_NEGOTIATE) {
        authMgr->OnAuthDeviceDataReceived(sessionId, message);
    } else {
        authMgr->OnDataReceived(sessionId, message);
    }
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
    auto authMgr = GetCurrentAuthMgr();
    if (authMgr == nullptr) {
        LOGE("authMgr is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return authMgr->RegisterUiStateCallback(pkgName);
}

int32_t DeviceManagerServiceImpl::UnRegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("UnRegisterUiStateCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto authMgr = GetCurrentAuthMgr();
    if (authMgr == nullptr) {
        LOGE("authMgr is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return authMgr->UnRegisterUiStateCallback(pkgName);
}

int32_t DeviceManagerServiceImpl::PraseNotifyEventJson(const std::string &event, JsonObject &jsonObject)
{
    jsonObject.Parse(event);
    if (jsonObject.IsDiscarded()) {
        LOGE("event prase error.");
        return ERR_DM_FAILED;
    }
    if ((!jsonObject.Contains("extra")) || (!jsonObject["extra"].IsObject())) {
        LOGE("extra error");
        return ERR_DM_FAILED;
    }
    if ((!jsonObject["extra"].Contains("deviceId")) || (!jsonObject["extra"]["deviceId"].IsString())) {
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
        JsonObject jsonObject;
        if (PraseNotifyEventJson(event, jsonObject) != DM_OK) {
            LOGE("NotifyEvent json invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        std::string deviceId;
        jsonObject["extra"]["deviceId"].GetTo(deviceId);
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
            return ERR_DM_SECURITY_FUNC_FAILED;
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

std::shared_ptr<Config> DeviceManagerServiceImpl::GetConfigByTokenId()
{
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (configsMap_.find(tokenId) == configsMap_.end()) {
        configsMap_[tokenId] = std::make_shared<Config>();
    }
    return configsMap_[tokenId];
}

int32_t DeviceManagerServiceImpl::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (pkgName.empty() || authCode.empty()) {
        LOGE("ImportAuthCode failed, pkgName or authCode is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("DeviceManagerServiceImpl::ImportAuthCode pkgName is %{public}s, authCode is %{public}s",
        pkgName.c_str(), GetAnonyString(authCode).c_str());
    auto authMgr = GetAuthMgr();
    if (authMgr == nullptr) {
        auto config = GetConfigByTokenId();
        config->pkgName = pkgName;
        config->authCode = authCode;   // If registered multiple times, only the last one is kept
        return DM_OK;
    }

    return authMgr->ImportAuthCode(pkgName, authCode);
}

int32_t DeviceManagerServiceImpl::ExportAuthCode(std::string &authCode)
{
    int32_t ret = GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
    authCode = std::to_string(ret);
    LOGI("ExportAuthCode success, authCode: %{public}s.", GetAnonyString(authCode).c_str());
    return DM_OK;
}

static JsonObject GetExtraJsonObject(const std::map<std::string, std::string> &bindParam)
{
    std::string extra;
    auto iter = bindParam.find(PARAM_KEY_BIND_EXTRA_DATA);
    if (iter != bindParam.end()) {
        extra = iter->second;
    } else {
        extra = ConvertMapToJsonString(bindParam);
    }

    return JsonObject(extra);
}

static int32_t GetHmlInfo(const JsonObject &jsonObject, bool &hmlEnable160M, int32_t &hmlActionId)
{
    if (jsonObject[PARAM_KEY_HML_ENABLE_160M].IsBoolean()) {
        hmlEnable160M = jsonObject[PARAM_KEY_HML_ENABLE_160M].Get<bool>();
        LOGI("hmlEnable160M %{public}d", hmlEnable160M);
    }
    if (!IsString(jsonObject, PARAM_KEY_HML_ACTIONID)) {
        LOGE("PARAM_KEY_HML_ACTIONID is not string");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string actionIdStr = jsonObject[PARAM_KEY_HML_ACTIONID].Get<std::string>();
    if (!IsNumberString(actionIdStr)) {
        LOGE("PARAM_KEY_HML_ACTIONID is not number");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t actionId = std::atoi(actionIdStr.c_str());
    if (actionId <= 0) {
        LOGE("PARAM_KEY_HML_ACTIONID is <= 0");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    hmlActionId = actionId;
    return DM_OK;
}

static bool IsHmlSessionType(const JsonObject &jsonObject)
{
    std::string connSessionType;
    if (jsonObject[PARAM_KEY_CONN_SESSIONTYPE].IsString()) {
        connSessionType = jsonObject[PARAM_KEY_CONN_SESSIONTYPE].Get<std::string>();
        LOGI("connSessionType %{public}s", connSessionType.c_str());
    }
    return connSessionType == CONN_SESSION_TYPE_HML;
}

int DeviceManagerServiceImpl::OpenAuthSession(const std::string& deviceId,
    const std::map<std::string, std::string> &bindParam)
{
    bool hmlEnable160M = false;
    int32_t hmlActionId = 0;
    JsonObject jsonObject = GetExtraJsonObject(bindParam);
    if (jsonObject.IsDiscarded()) {
        LOGE("extra string not a json type.");
        return -1;
    }
    if (IsHmlSessionType(jsonObject)) {
        auto ret = GetHmlInfo(jsonObject, hmlEnable160M, hmlActionId);
        if (ret != DM_OK) {
            LOGE("OpenAuthSession failed, GetHmlInfo failed.");
            return ret;
        }
        LOGI("hmlActionId %{public}d, hmlEnable160M %{public}d", hmlActionId, hmlEnable160M);
        return softbusConnector_->GetSoftbusSession()->OpenAuthSessionWithPara(deviceId,
            hmlActionId, hmlEnable160M);
    } else {
        return softbusConnector_->GetSoftbusSession()->OpenAuthSession(deviceId);
    }
}

std::shared_ptr<Session> DeviceManagerServiceImpl::GetOrCreateSession(const std::string& deviceId,
    const std::map<std::string, std::string> &bindParam)
{
    std::shared_ptr<Session> instance;
    int sessionId = -1;
    // Acquire global lock to ensure thread safety for maps
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        if (deviceId2SessionIdMap_.find(deviceId) != deviceId2SessionIdMap_.end()) {
            sessionId = deviceId2SessionIdMap_[deviceId];
        }
        if (sessionsMap_.find(sessionId) != sessionsMap_.end()) {
            return sessionsMap_[sessionId];
        }
    }

    // Get the lock corresponding to deviceId
    std::mutex& device_mutex = deviceIdMutexMap_[deviceId];
    std::lock_guard<std::mutex> lock(device_mutex);

    // Check again whether the corresponding object already exists (because other threads may have created it during
    //  the lock acquisition in the previous step)
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        if (deviceId2SessionIdMap_.find(deviceId) != deviceId2SessionIdMap_.end()) {
            sessionId = deviceId2SessionIdMap_[deviceId];
        }
        if (sessionsMap_.find(sessionId) != sessionsMap_.end()) {
            return sessionsMap_[sessionId];
        }

        sessionId = OpenAuthSession(deviceId, bindParam);
        if (sessionId < 0) {
            LOGE("OpenAuthSession failed, stop the authentication");
            return nullptr;
        }

        std::unique_lock<std::mutex> cvLock(sessionEnableMutexMap_[sessionId]);
        sessionEnableCvReadyMap_[sessionId] = false;
        if (sessionEnableCvMap_[sessionId].wait_for(cvLock, std::chrono::seconds(WAIT_TIMEOUT),
            [&] { return sessionEnableCvReadyMap_[sessionId]; })) {
            LOGI("session enable, sessionId: %{public}d.", sessionId);
        } else {
            LOGE("wait session enable timeout or enable fail, sessionId: %{public}d.", sessionId);
            return nullptr;
        }
        instance = std::make_shared<Session>(sessionId, deviceId);
        deviceId2SessionIdMap_[deviceId] = sessionId;
        sessionsMap_[sessionId] = instance;
    }
    return instance;
}

int32_t DeviceManagerServiceImpl::GetDeviceInfo(const PeerTargetId &targetId, std::string &addrType,
    std::string &deviceId, std::shared_ptr<DeviceInfo> deviceInfo, int32_t &index)
{
    ConnectionAddr addr;
    if (!targetId.wifiIp.empty() && targetId.wifiIp.length() <= IP_STR_MAX_LEN) {
        LOGI("parse wifiIp: %{public}s.", GetAnonyString(targetId.wifiIp).c_str());
        if (!addrType.empty()) {
            addr.type = static_cast<ConnectionAddrType>(std::atoi(addrType.c_str()));
        } else {
            addr.type = ConnectionAddrType::CONNECTION_ADDR_WLAN;
        }
        if (memcpy_s(addr.info.ip.ip, IP_STR_MAX_LEN, targetId.wifiIp.c_str(), targetId.wifiIp.length()) != 0) {
            LOGE("get ip addr: %{public}s failed", GetAnonyString(targetId.wifiIp).c_str());
            return ERR_DM_SECURITY_FUNC_FAILED;
        }
        addr.info.ip.port = targetId.wifiPort;
        deviceInfo->addr[index] = addr;
        deviceId = targetId.wifiIp;
        index++;
    } else if (!targetId.brMac.empty() && targetId.brMac.length() <= BT_MAC_LEN) {
        LOGI("parse brMac: %{public}s.", GetAnonyString(targetId.brMac).c_str());
        addr.type = ConnectionAddrType::CONNECTION_ADDR_BR;
        if (memcpy_s(addr.info.br.brMac, BT_MAC_LEN, targetId.brMac.c_str(), targetId.brMac.length()) != 0) {
            LOGE("get brMac addr: %{public}s failed", GetAnonyString(targetId.brMac).c_str());
            return ERR_DM_SECURITY_FUNC_FAILED;
        }
        deviceInfo->addr[index] = addr;
        deviceId = targetId.brMac;
        index++;
    } else if (!targetId.bleMac.empty() && targetId.bleMac.length() <= BT_MAC_LEN) {
        LOGI("parse bleMac: %{public}s.", GetAnonyString(targetId.bleMac).c_str());
        addr.type = ConnectionAddrType::CONNECTION_ADDR_BLE;
        if (memcpy_s(addr.info.ble.bleMac, BT_MAC_LEN, targetId.bleMac.c_str(), targetId.bleMac.length()) != 0) {
            LOGE("get bleMac addr: %{public}s failed", GetAnonyString(targetId.bleMac).c_str());
            return ERR_DM_SECURITY_FUNC_FAILED;
        }
        if (!targetId.deviceId.empty()) {
            Crypto::ConvertHexStringToBytes(addr.info.ble.udidHash, UDID_HASH_LEN,
                targetId.deviceId.c_str(), targetId.deviceId.length());
        }
        deviceInfo->addr[index] = addr;
        deviceId = targetId.bleMac;
        index++;
    } else {
        LOGE("DeviceManagerServiceImpl::GetDeviceInfo failed, not addr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}

bool DeviceManagerServiceImpl::IsAuthNewVersion(int32_t bindLevel, std::string localUdid, std::string remoteUdid,
    int32_t tokenId, int32_t userId)
{
    std::string extraInfo = DeviceProfileConnector::GetInstance().IsAuthNewVersion(
        bindLevel, localUdid, remoteUdid, tokenId, userId);
    JsonObject extraInfoJson(extraInfo);
    if (extraInfoJson.IsDiscarded()) {
        LOGE("IsAuthNewVersion extraInfoJson error");
        return false;
    }
    if (!extraInfoJson[TAG_DMVERSION].IsString()) {
        LOGE("IsAuthNewVersion PARAM_KEY_OS_VERSION error");
        return false;
    }
    std::string dmVersion = extraInfoJson[TAG_DMVERSION].Get<std::string>();
    if (CompareVersion(dmVersion, std::string(DM_VERSION_5_1_0)) || dmVersion == std::string(DM_VERSION_5_1_0)) {
        return true;
    }
    return false;
}

int32_t DeviceManagerServiceImpl::ParseConnectAddr(const PeerTargetId &targetId, std::string &deviceId,
    const std::map<std::string, std::string> &bindParam)
{
    std::string addrType;
    if (bindParam.count(PARAM_KEY_CONN_ADDR_TYPE) != 0) {
        addrType = bindParam.at(PARAM_KEY_CONN_ADDR_TYPE);
    }

    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    int32_t index = 0;
    int32_t ret = GetDeviceInfo(targetId, addrType, deviceId, deviceInfo, index);
    if (ret != DM_OK) {
        LOGE("GetDeviceInfo failed, ret: %{public}d", ret);
    }
    deviceInfo->addrNum = static_cast<uint32_t>(index);
    if (softbusConnector_->AddMemberToDiscoverMap(deviceId, deviceInfo) != DM_OK) {
        LOGE("DeviceManagerServiceImpl::ParseConnectAddr failed, AddMemberToDiscoverMap failed.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    deviceInfo = nullptr;
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam)
{
    if (pkgName.empty()) {
        LOGE("BindTarget failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::string deviceId = "";
    PeerTargetId targetIdTmp = const_cast<PeerTargetId&>(targetId);
    if (ParseConnectAddr(targetId, deviceId, bindParam) == DM_OK) {
        targetIdTmp.deviceId = deviceId;
    } else {
        if (targetId.deviceId.empty()) {
            LOGE("DeviceManagerServiceImpl::BindTarget failed, ParseConnectAddr failed.");
            return ERR_DM_INPUT_PARA_INVALID;
        }
    }
    // Created only at the source end. The same target device will not be created repeatedly with the new protocol.
    std::shared_ptr<Session> curSession = GetOrCreateSession(targetIdTmp.deviceId, bindParam);
    if (curSession == nullptr) {
        LOGE("Failed to create the session. Target deviceId: %{public}s.", targetIdTmp.deviceId.c_str());
        return ERR_DM_AUTH_OPEN_SESSION_FAILED;
    }

    // Logical session random number
    int sessionId = curSession->sessionId_;
    uint64_t logicalSessionId = 0;
    if (curSession->version_ == "" || CompareVersion(curSession->version_, DM_VERSION_5_0_OLD_MAX)) {
        logicalSessionId = GenerateRandNum(sessionId);
        if (curSession->logicalSessionSet_.find(logicalSessionId) != curSession->logicalSessionSet_.end()) {
            LOGE("Failed to create the logical session.");
            return ERR_DM_LOGIC_SESSION_CREATE_FAILED;
        }
    }

    // Create on the src end.
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t ret = InitAndRegisterAuthMgr(true, tokenId, curSession, logicalSessionId);
    if (ret != DM_OK) {
        LOGE("InitAndRegisterAuthMgr failed, ret %{public}d.", ret);
        return ret;
    }
    curSession->logicalSessionSet_.insert(logicalSessionId);
    curSession->logicalSessionCnt_.fetch_add(1);
    logicalSessionId2TokenIdMap_[logicalSessionId] = tokenId;
    logicalSessionId2SessionIdMap_[logicalSessionId] = sessionId;

    auto authMgr = GetAuthMgrByTokenId(tokenId);
    if (authMgr == nullptr) {
        return ERR_DM_POINT_NULL;
    }
    authMgr->SetBindTargetParams(targetId);
    if ((ret = authMgr->BindTarget(pkgName, targetIdTmp, bindParam, sessionId, logicalSessionId)) != DM_OK) {
        LOGE("authMgr BindTarget failed, ret %{public}d.", ret);
        CleanAuthMgrByLogicalSessionId(logicalSessionId);
    }
    return ret;
}

int32_t DeviceManagerServiceImpl::DpAclAdd(const std::string &udid)
{
    LOGI("DeviceManagerServiceImpl DpAclAdd start.");
    MultipleUserConnector::SetSwitchOldUserId(MultipleUserConnector::GetCurrentAccountUserID());
    MultipleUserConnector::SetSwitchOldAccountId(MultipleUserConnector::GetOhosAccountId());
    if (deviceStateMgr_->CheckIsOnline(udid)) {
        LOGI("DeviceManagerServiceImpl DpAclAdd identical account and online");
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        softbusConnector_->SetProcessInfo(processInfo);
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

uint64_t DeviceManagerServiceImpl::GetTokenIdByNameAndDeviceId(std::string pkgName,
    std::string requestDeviceId)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerServiceImpl::GetTokenIdByNameAndDeviceId error: pkgName.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (requestDeviceId.empty()) {
        LOGE("DeviceManagerServiceImpl::GetTokenIdByNameAndDeviceId error: requestDeviceId.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    return DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(pkgName, requestDeviceId);
}

std::unordered_map<std::string, DmAuthForm> DeviceManagerServiceImpl::GetAppTrustDeviceIdList(
    std::string pkgname)
{
    char localDeviceId[DEVICE_UUID_LENGTH];
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string deviceId = reinterpret_cast<char *>(localDeviceId);
    return DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgname, deviceId);
}

void DeviceManagerServiceImpl::LoadHardwareFwkService()
{
    DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
}

void DeviceManagerServiceImpl::HandleIdentAccountLogout(const std::string &localUdid, int32_t localUserId,
    const std::string &peerUdid, int32_t peerUserId)
{
    LOGI("localUdid %{public}s, localUserId %{public}d, peerUdid %{public}s, peerUserId %{public}d.",
        GetAnonyString(localUdid).c_str(), localUserId, GetAnonyString(peerUdid).c_str(), peerUserId);
    DmOfflineParam offlineParam;
    bool notifyOffline = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId,
        peerUdid, peerUserId, offlineParam);
    if (notifyOffline) {
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = localUserId;
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfo(processInfo);
        CHECK_NULL_VOID(deviceStateMgr_);
        deviceStateMgr_->OnDeviceOffline(peerUdid);
    }
    CHECK_NULL_VOID(hiChainConnector_);
    hiChainConnector_->DeleteAllGroup(localUserId);
    CHECK_NULL_VOID(hiChainAuthConnector_);
    hiChainAuthConnector_->DeleteCredential(peerUdid, localUserId, peerUserId);
    LOGE("DeleteSkCredAndAcl start");
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
}

void DeviceManagerServiceImpl::HandleUserRemoved(std::vector<std::string> peerUdids, int32_t preUserId)
{
    LOGI("PreUserId %{public}d.", preUserId);
    char localDeviceId[DEVICE_UUID_LENGTH];
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = reinterpret_cast<char *>(localDeviceId);
    std::multimap<std::string, int32_t> peerUserIdMap;     // key: peerUdid  value: peerUserId
    DmOfflineParam offlineParam;
    DeviceProfileConnector::GetInstance().DeleteAclForUserRemoved(localUdid, preUserId, peerUdids, peerUserIdMap,
        offlineParam);
    CHECK_NULL_VOID(hiChainConnector_);
    hiChainConnector_->DeleteAllGroup(preUserId);

    if (peerUserIdMap.empty()) {
        LOGE("peerUserIdMap is empty");
        return;
    }
    CHECK_NULL_VOID(hiChainAuthConnector_);
    for (const auto &item : peerUserIdMap) {
        hiChainAuthConnector_->DeleteCredential(item.first, preUserId, item.second);
    }
    LOGE("DeleteSkCredAndAcl start");
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
}

void DeviceManagerServiceImpl::HandleRemoteUserRemoved(int32_t userId, const std::string &remoteUdid)
{
    LOGI("remoteUdid %{public}s, userId %{public}d", GetAnonyString(remoteUdid).c_str(), userId);
    std::vector<int32_t> localUserIds;
    DmOfflineParam offlineParam;
    DeviceProfileConnector::GetInstance().DeleteAclForRemoteUserRemoved(remoteUdid, userId, localUserIds, offlineParam);
    if (localUserIds.empty()) {
        return;
    }
    CHECK_NULL_VOID(hiChainAuthConnector_);
    std::vector<std::pair<int32_t, std::string>> delInfoVec;
    for (int32_t localUserId : localUserIds) {
        delInfoVec.push_back(std::pair<int32_t, std::string>(localUserId, remoteUdid));
        hiChainAuthConnector_->DeleteCredential(remoteUdid, localUserId, userId);
    }
    CHECK_NULL_VOID(hiChainConnector_);
    hiChainConnector_->DeleteGroupByACL(delInfoVec, localUserIds);
    LOGE("DeleteSkCredAndAcl start");
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
}

void DeviceManagerServiceImpl::HandleUserSwitched(const std::vector<std::string> &deviceVec,
    int32_t currentUserId, int32_t beforeUserId)
{
    LOGI("currentUserId: %{public}s, beforeUserId: %{public}s", GetAnonyInt32(currentUserId).c_str(),
        GetAnonyInt32(beforeUserId).c_str());
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    DeviceProfileConnector::GetInstance().HandleUserSwitched(localUdid, deviceVec, currentUserId, beforeUserId);
}

void DeviceManagerServiceImpl::ScreenCommonEventCallback(std::string commonEventType)
{
    if (commonEventType == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED) {
        LOGI("DeviceManagerServiceImpl::ScreenCommonEventCallback on screen locked.");
        for (auto& pair : authMgrMap_) {
            if (pair.second != nullptr) {
                LOGI("DeviceManagerServiceImpl::ScreenCommonEventCallback tokenId: %{public}" PRId64 ".", pair.first);
                pair.second->OnScreenLocked();
            }
        }
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
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_POINT_NULL);
    bool ret = hiChainConnector_->IsDevicesInP2PGroup(srcUdid, sinkUdid);
    if (!ret) {
        int32_t checkRet = DeviceProfileConnector::GetInstance().CheckAccessControl(caller,
            srcUdid, callee, sinkUdid);
        return checkRet;
    } else {
        return DM_OK;
    }
}

void DeviceManagerServiceImpl::HandleDeviceNotTrust(const std::string &udid)
{
    LOGI("DeviceManagerServiceImpl::HandleDeviceNotTrust udid: %{public}s.", GetAnonyString(udid).c_str());
    if (udid.empty()) {
        LOGE("HandleDeviceNotTrust udid is empty.");
        return;
    }
    DeviceProfileConnector::GetInstance().DeleteAccessControlList(udid);
    CHECK_NULL_VOID(hiChainConnector_);
    hiChainConnector_->DeleteAllGroupByUdid(udid);
}

int32_t DeviceManagerServiceImpl::GetBindLevel(const std::string &pkgName, const std::string &localUdid,
    const std::string &udid, uint64_t &tokenId)
{
    return DeviceProfileConnector::GetInstance().GetBindLevel(pkgName, localUdid, udid, tokenId);
}

std::map<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndBindLevel(int32_t userId)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    std::vector<int32_t> userIds;
    userIds.push_back(userId);
    return DeviceProfileConnector::GetInstance().GetDeviceIdAndBindLevel(userIds, localUdid);
}

std::multimap<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndUserId(int32_t userId,
    const std::string &accountId)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    return DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(userId, accountId, localUdid);
}

void DeviceManagerServiceImpl::HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
    const std::string &remoteUdid)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    std::multimap<std::string, int32_t> devIdAndUserMap =
        DeviceProfileConnector::GetInstance().GetDevIdAndUserIdByActHash(localUdid, remoteUdid,
            remoteUserId, remoteAccountHash);
    CHECK_NULL_VOID(listener_);
    std::string uuid = "";
    SoftbusCache::GetInstance().GetUuidByUdid(remoteUdid, uuid);
    listener_->OnDeviceTrustChange(remoteUdid, uuid, DmAuthForm::IDENTICAL_ACCOUNT);
    for (const auto &item : devIdAndUserMap) {
        DmOfflineParam offlineParam;
        LOGI("remoteUdid %{public}s.", GetAnonyString(remoteUdid).c_str());
        bool notifyOffline = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(item.first, item.second,
            remoteUdid, remoteUserId, offlineParam);
        if (notifyOffline) {
            ProcessInfo processInfo;
            processInfo.pkgName = std::string(DM_PKG_NAME);
            processInfo.userId = item.second;
            CHECK_NULL_VOID(softbusConnector_);
            softbusConnector_->SetProcessInfo(processInfo);
            CHECK_NULL_VOID(deviceStateMgr_);
            deviceStateMgr_->OnDeviceOffline(remoteUdid);
        }
        CHECK_NULL_VOID(hiChainConnector_);
        hiChainConnector_->DeleteAllGroup(item.second);
        CHECK_NULL_VOID(hiChainAuthConnector_);
        hiChainAuthConnector_->DeleteCredential(remoteUdid, item.second, remoteUserId);
        LOGE("DeleteSkCredAndAcl start");
        DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    }
}

DmAuthForm DeviceManagerServiceImpl::ConvertBindTypeToAuthForm(int32_t bindType)
{
    LOGI("BindType %{public}d.", bindType);
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    if (static_cast<uint32_t>(bindType) == DM_IDENTICAL_ACCOUNT) {
        authForm = IDENTICAL_ACCOUNT;
    } else if (static_cast<uint32_t>(bindType) == DM_POINT_TO_POINT) {
        authForm = PEER_TO_PEER;
    } else if (static_cast<uint32_t>(bindType) == DM_ACROSS_ACCOUNT) {
        authForm = ACROSS_ACCOUNT;
    } else {
        LOGE("Invalied bindType.");
    }
    return authForm;
}

int32_t DeviceManagerServiceImpl::DeleteGroup(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DeviceManagerServiceImpl::DeleteGroup");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_FAILED;
    }
    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_POINT_NULL);
    hiChainConnector_->GetRelatedGroups(deviceId, groupList);
    for (const auto &item : groupList) {
        std::string groupId = item.groupId;
        hiChainConnector_->DeleteGroup(groupId);
    }
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    DmOfflineParam offlineParam;
    int32_t bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(
        remoteUserId, remoteUdid, localUdid, offlineParam);
    if (static_cast<uint32_t>(bindType) == DM_INVALIED_TYPE) {
        LOGE("Invalied bindtype.");
        return;
    }
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    DeleteGroup(DM_PKG_NAME, remoteUdid);
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
}

void DeviceManagerServiceImpl::HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId)
{
    LOGI("HandleAppUnBindEvent tokenId = %{public}d.", tokenId);
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId, localUdid);
    if (offlineParam.leftAclNumber != 0) {
        LOGI("HandleAppUnBindEvent app-level type leftAclNumber not zero.");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        softbusConnector_->HandleDeviceOffline(remoteUdid);
        DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
        return;
    }
    if (offlineParam.leftAclNumber == 0) {
        LOGI("HandleAppUnBindEvent app-level type leftAclNumber is zero.");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        if (!offlineParam.hasLnnAcl) {
            CHECK_NULL_VOID(hiChainAuthConnector_);
            hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
                remoteUserId);
        } else {
            DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
        }
        return;
    }
}

void DeviceManagerServiceImpl::HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId, int32_t peerTokenId)
{
    LOGI("HandleAppUnBindEvent peerTokenId = %{public}d.", peerTokenId);
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid,
            tokenId, localUdid, peerTokenId);
    if (offlineParam.leftAclNumber != 0) {
        LOGI("HandleAppUnBindEvent app-level type leftAclNumber not zero.");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        softbusConnector_->HandleDeviceOffline(remoteUdid);
        DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
        return;
    }
    if (offlineParam.leftAclNumber == 0) {
        LOGI("HandleAppUnBindEvent app-level type leftAclNumber is zero.");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        if (!offlineParam.hasLnnAcl) {
            CHECK_NULL_VOID(hiChainAuthConnector_);
            hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
                remoteUserId);
        } else {
            DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
        }
        return;
    }
}

void DeviceManagerServiceImpl::HandleServiceUnBindEvent(int32_t userId, const std::string &remoteUdid,
    int32_t remoteTokenId)
{
    LOGI("HandleServiceUnBindEvent remoteTokenId = %{public}d, userId: %{public}d, remoteUdid: %{public}s.",
        remoteTokenId, userId, GetAnonyString(remoteUdid).c_str());
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    int32_t localUserId = MultipleUserConnector::GetCurrentAccountUserID();
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().HandleServiceUnBindEvent(
        userId, remoteUdid, localUdid, remoteTokenId);

    CHECK_NULL_VOID(softbusConnector_);
    if (offlineParam.hasLnnAcl) {
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
    } else {
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        softbusConnector_->HandleDeviceOffline(remoteUdid);
    }
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
}

void DeviceManagerServiceImpl::HandleSyncUserIdEvent(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid, bool isCheckUserStatus)
{
    LOGI("remote udid: %{public}s, foregroundUserIds: %{public}s, backgroundUserIds: %{public}s",
        GetAnonyString(remoteUdid).c_str(), GetIntegerList<uint32_t>(foregroundUserIds).c_str(),
        GetIntegerList<uint32_t>(backgroundUserIds).c_str());
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    std::vector<int32_t> rmtFrontUserIdsTemp(foregroundUserIds.begin(), foregroundUserIds.end());
    std::vector<int32_t> rmtBackUserIdsTemp(backgroundUserIds.begin(), backgroundUserIds.end());
    std::vector<int32_t> localUserIds;
    int32_t ret = MultipleUserConnector::GetForegroundUserIds(localUserIds);
    if (ret != DM_OK) {
        LOGE("Get foreground userids failed, ret: %{public}d", ret);
        return;
    }
    if (isCheckUserStatus) {
        MultipleUserConnector::ClearLockedUser(localUserIds);
    }
    DmOfflineParam offlineParam;
    DeviceProfileConnector::GetInstance().UpdateACL(localUdid, localUserIds, remoteUdid,
        rmtFrontUserIdsTemp, rmtBackUserIdsTemp, offlineParam);
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(rmtBackUserIdsTemp, remoteUdid,
        localUserIds, localUdid);
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(rmtFrontUserIdsTemp, remoteUdid,
        localUserIds, localUdid);
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
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        softbusConnector_->SetProcessInfo(processInfo);
    } else if (bindType == APP_PEER_TO_PEER_TYPE || bindType == APP_ACROSS_ACCOUNT_TYPE) {
        std::vector<ProcessInfo> processInfoVec =
            DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(requestDeviceId, trustDeviceId,
                MultipleUserConnector::GetFirstForegroundUserId());
        softbusConnector_->SetProcessInfoVec(processInfoVec);
    }
    deviceStateMgr_->HandleDeviceScreenStatusChange(devInfo);
}

int32_t DeviceManagerServiceImpl::SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string remoteAclList)
{
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    return softbusConnector_->SyncLocalAclListProcess(localDevUserInfo, remoteDevUserInfo, remoteAclList);
}

int32_t DeviceManagerServiceImpl::GetAclListHash(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclList)
{
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    return softbusConnector_->GetAclListHash(localDevUserInfo, remoteDevUserInfo, aclList);
}

void DeviceManagerServiceImpl::HandleCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId,
                                                          int32_t errcode)
{
    CHECK_NULL_VOID(credentialMgr_);
    credentialMgr_->HandleCredentialAuthStatus(deviceList, deviceTypeId, errcode);
}

int32_t DeviceManagerServiceImpl::ProcessAppUnintall(const std::string &appId, int32_t accessTokenId)
{
    CHECK_NULL_RETURN(listener_, ERR_DM_POINT_NULL);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    LOGI("delete ACL size is %{public}zu, appId %{public}s", profiles.size(), GetAnonyString(appId).c_str());
    if (profiles.size() == 0) {
        return DM_OK;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<std::pair<int32_t, std::string>> delACLInfoVec;
    std::vector<int32_t> userIdVec;
    std::map<int64_t, DistributedDeviceProfile::AccessControlProfile> delProfileMap;
    DeleteAclByTokenId(accessTokenId, profiles, delProfileMap, delACLInfoVec, userIdVec);
    for (auto item : delProfileMap) {
        DmOfflineParam lnnAclParam;
        bool isLastLnnAcl = false;
        for (auto it : profiles) {
            CheckIsLastLnnAcl(it, item.second, lnnAclParam, isLastLnnAcl, localUdid);
        }
        if (!isLastLnnAcl) {
            DeleteSkCredAndAcl(lnnAclParam.needDelAclInfos);
        }
    }

    if (delACLInfoVec.size() == 0) {
        return DM_OK;
    }
    if (userIdVec.size() == 0) {
        return DM_OK;
    }
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_POINT_NULL);
    hiChainConnector_->DeleteGroupByACL(delACLInfoVec, userIdVec);
    return DM_OK;
}

void DeviceManagerServiceImpl::CheckIsLastLnnAcl(DistributedDeviceProfile::AccessControlProfile profile,
    DistributedDeviceProfile::AccessControlProfile delProfile, DmOfflineParam &lnnAclParam, bool &isLastLnnAcl,
    const std::string &localUdid)
{
    if (DeviceProfileConnector::GetInstance().IsLnnAcl(profile) && CheckLnnAcl(delProfile, profile)) {
        if (profile.GetAccesser().GetAccesserDeviceId() == localUdid) {
            DeviceProfileConnector::GetInstance().CacheAcerAclId(profile, lnnAclParam.needDelAclInfos);
        }
        if (profile.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            DeviceProfileConnector::GetInstance().CacheAceeAclId(profile, lnnAclParam.needDelAclInfos);
        }
    }
    if (!DeviceProfileConnector::GetInstance().IsLnnAcl(profile) && CheckLnnAcl(delProfile, profile)) {
        isLastLnnAcl = true;
    }
}

void DeviceManagerServiceImpl::DeleteAclByTokenId(const int32_t accessTokenId,
    std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles,
    std::map<int64_t, DistributedDeviceProfile::AccessControlProfile> &delProfileMap,
    std::vector<std::pair<int32_t, std::string>> &delACLInfoVec, std::vector<int32_t> &userIdVec)
{
    for (auto &item : profiles) {
        int64_t accesssertokenId = item.GetAccesser().GetAccesserTokenId();
        int64_t accessseetokenId = item.GetAccessee().GetAccesseeTokenId();
        if (accessTokenId != static_cast<int32_t>(accesssertokenId) ||
            accessTokenId != static_cast<int32_t>(accessseetokenId)) {
            continue;
        }
        if (accessTokenId == static_cast<int32_t>(accesssertokenId)) {
            DmOfflineParam offlineParam;
            delProfileMap[item.GetAccessControlId()] = item;
            DeviceProfileConnector::GetInstance().CacheAcerAclId(item, offlineParam.needDelAclInfos);
            DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
            listener_->OnAppUnintall(item.GetAccesser().GetAccesserBundleName());
            if (item.GetBindLevel() == USER) {
                userIdVec.push_back(item.GetAccesser().GetAccesserUserId());
                delACLInfoVec.push_back(std::pair<int32_t, std::string>(item.GetAccesser().GetAccesserUserId(),
                    item.GetAccessee().GetAccesseeDeviceId()));
            }
        }
        if (accessTokenId == static_cast<int32_t>(accesssertokenId)) {
            DmOfflineParam offlineParam;
            DeviceProfileConnector::GetInstance().CacheAceeAclId(item, offlineParam.needDelAclInfos);
            delProfileMap[item.GetAccessControlId()] = item;
            DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
            listener_->OnAppUnintall(item.GetAccessee().GetAccesseeBundleName());
            if (item.GetBindLevel() == USER) {
                userIdVec.push_back(item.GetAccessee().GetAccesseeUserId());
                delACLInfoVec.push_back(std::pair<int32_t, std::string>(item.GetAccessee().GetAccesseeUserId(),
                    item.GetAccesser().GetAccesserDeviceId()));
            }
        }
    }
    for (auto item : delProfileMap) {
        for (auto it = profiles.begin(); it != profiles.end();) {
            if (item.first == it->GetAccessControlId()) {
                it = profiles.erase(it);
            } else {
                it++;
            }
        }
    }
}

bool DeviceManagerServiceImpl::CheckLnnAcl(DistributedDeviceProfile::AccessControlProfile delProfile,
    DistributedDeviceProfile::AccessControlProfile lastprofile)
{
    if ((delProfile.GetAccesser().GetAccesserDeviceId() == lastprofile.GetAccesser().GetAccesserDeviceId() &&
        delProfile.GetAccesser().GetAccesserUserId() == lastprofile.GetAccesser().GetAccesserUserId() &&
        delProfile.GetAccessee().GetAccesseeDeviceId() == lastprofile.GetAccessee().GetAccesseeDeviceId() &&
        delProfile.GetAccessee().GetAccesseeUserId() == lastprofile.GetAccessee().GetAccesseeUserId()) ||
        (delProfile.GetAccesser().GetAccesserDeviceId() == lastprofile.GetAccessee().GetAccesseeDeviceId() &&
        delProfile.GetAccesser().GetAccesserUserId() == lastprofile.GetAccessee().GetAccesseeUserId() &&
        delProfile.GetAccessee().GetAccesseeDeviceId() == lastprofile.GetAccesser().GetAccesserDeviceId() &&
        delProfile.GetAccessee().GetAccesseeUserId() == lastprofile.GetAccesser().GetAccesserUserId())) {
        return true;
    }
    return false;
}

std::multimap<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndUserId(int32_t localUserId)
{
    LOGI("localUserId %{public}d.", localUserId);
    char localdeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localdeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localdeviceId);
    return DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(localUdid, localUserId);
}

int32_t DeviceManagerServiceImpl::SaveOnlineDeviceInfo(const std::vector<DmDeviceInfo> &deviceList)
{
    CHECK_NULL_RETURN(deviceStateMgr_, ERR_DM_POINT_NULL);
    for (auto item : deviceList) {
        deviceStateMgr_->SaveOnlineDeviceInfo(item);
    }
    return DM_OK;
}

void DeviceManagerServiceImpl::HandleDeviceUnBind(int32_t bindType, const std::string &peerUdid,
    const std::string &localUdid, int32_t localUserId, const std::string &localAccountId)
{
    return DeviceProfileConnector::GetInstance().HandleDeviceUnBind(bindType, peerUdid,
        localUdid, localUserId, localAccountId);
}

int32_t DeviceManagerServiceImpl::RegisterAuthenticationType(int32_t authenticationType)
{
    if (authenticationType != USER_OPERATION_TYPE_ALLOW_AUTH &&
        authenticationType != USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        LOGE("Invalid parameter.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto authMgr = GetAuthMgr();
    if (authMgr == nullptr) {
        auto config = GetConfigByTokenId();
        config->authenticationType = authenticationType;   // only the last registration is retained
        return DM_OK;
    }
    return authMgr->RegisterAuthenticationType(authenticationType);
}

void DeviceManagerServiceImpl::DeleteAlwaysAllowTimeOut()
{
    LOGI("Start DeleteAlwaysAllowTimeOut");
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    std::string remoteUdid = "";
    int32_t remoteUserId = -1;
    int64_t currentTime =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t currentUserId = MultipleUserConnector::GetCurrentAccountUserID();
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    for (auto &item : profiles) {
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            continue;
        }
        if ((currentTime - item.GetLastAuthTime()) > MAX_ALWAYS_ALLOW_SECONDS && item.GetLastAuthTime() > 0) {
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.GetAccessControlId());
            if (item.GetAccesser().GetAccesserUserId() == currentUserId &&
                item.GetAccesser().GetAccesserDeviceId() == localUdid) {
                remoteUserId = item.GetAccessee().GetAccesseeUserId();
            }
            if (item.GetAccessee().GetAccesseeUserId() == currentUserId &&
                item.GetAccessee().GetAccesseeDeviceId() == localUdid) {
                remoteUserId = item.GetAccesser().GetAccesserUserId();
            }
            remoteUdid = item.GetTrustDeviceId();
            CheckDeleteCredential(remoteUdid, remoteUserId);
        }
    }
}

void DeviceManagerServiceImpl::CheckDeleteCredential(const std::string &remoteUdid, int32_t remoteUserId)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    bool leftAcl = false;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == remoteUdid) {
            leftAcl = true;
        }
    }
    if (!leftAcl) {
        LOGI("CheckDeleteCredential delete credential");
        hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
            remoteUserId);
    }
}

int32_t DeviceManagerServiceImpl::CheckDeviceInfoPermission(const std::string &localUdid,
    const std::string &peerDeviceId)
{
    int32_t ret = DeviceProfileConnector::GetInstance().CheckDeviceInfoPermission(localUdid, peerDeviceId);
    if (ret != DM_OK) {
        LOGE("CheckDeviceInfoPermission failed, ret: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::DeleteAcl(const std::string &pkgName, const std::string &localUdid,
    const std::string &remoteUdid, int32_t bindLevel, const std::string &extra)
{
    LOGI("DeleteAcl pkgName %{public}s, localUdid %{public}s, remoteUdid %{public}s, bindLevel %{public}d.",
        pkgName.c_str(), GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), bindLevel);
    if (static_cast<uint32_t>(bindLevel) == USER) {
        DeleteGroup(pkgName, remoteUdid);
    }
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localUdid, remoteUdid, bindLevel, extra);
    if (offlineParam.bindType == INVALIED_TYPE) {
        LOGE("Acl not contain the pkgname bind data.");
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
    if (static_cast<uint32_t>(bindLevel) == APP) {
        ProcessInfo processInfo;
        processInfo.pkgName = pkgName;
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        if (offlineParam.leftAclNumber != 0) {
            LOGI("The pkgName unbind app-level type leftAclNumber not zero.");
            softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
            softbusConnector_->HandleDeviceOffline(remoteUdid);
            return DM_OK;
        }
        if (offlineParam.leftAclNumber == 0) {
            LOGI("The pkgName unbind app-level type leftAclNumber is zero.");
            softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
            hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
                offlineParam.peerUserId);
            return DM_OK;
        }
    }
    if (static_cast<uint32_t>(bindLevel) == USER && offlineParam.leftAclNumber != 0) {
        LOGI("Unbind deivce-level, retain identical account bind type.");
        return DM_OK;
    }
    if (static_cast<uint32_t>(bindLevel) == USER && offlineParam.leftAclNumber == 0) {
        LOGI("Unbind deivce-level, retain null.");
        hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
            offlineParam.peerUserId);
        return DM_OK;
    }
    return ERR_DM_FAILED;
}

int32_t DeviceManagerServiceImpl::DeleteSkCredAndAcl(const std::vector<DmAclIdParam> &acls)
{
    LOGI("DeleteSkCredAndAcl start.");
    int32_t ret = DM_OK;
    if (acls.empty()) {
        return ret;
    }
    CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
    for (auto item : acls) {
        ret = DeviceProfileConnector::GetInstance().DeleteSessionKey(item.userId, item.skId);
        if (ret != DM_OK) {
            LOGE("DeleteSessionKey err, userId:%{public}d, skId:%{public}d, ret:%{public}d", item.userId, item.skId,
                ret);
        }
        ret = hiChainAuthConnector_->DeleteCredential(item.userId, item.credId);
        if (ret != DM_OK) {
            LOGE("DeletecredId err, userId:%{public}d, credId:%{public}s, ret:%{public}d", item.userId,
                item.credId.c_str(), ret);
        }
        DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.accessControlId);
    }
    return ret;
}

int32_t DeviceManagerServiceImpl::DeleteAclForProcV2(const std::string &localUdid, uint32_t localTokenId,
    const std::string &remoteUdid, int32_t bindLevel, const std::string &extra, int32_t userId)
{
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().FilterNeedDeleteACL(
        localUdid, localTokenId, remoteUdid, extra);

    // first, clear the unbind sk/cred/acl
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);

    // second, determin if need clear lnn acl
    if (offlineParam.allLeftAppOrSvrAclInfos.empty()) {
        LOGI("No app or service acl exist, clear lnn acl");
        DeleteSkCredAndAcl(offlineParam.allLnnAclInfos);
    }

    // third, determin if need report offline to unbind bundle
    if (offlineParam.allUserAclInfos.empty()) {
        LOGI("after clear target acl, No user acl exist, report offline");
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        softbusConnector_->HandleDeviceOffline(remoteUdid);
    }

    return DM_OK;
}

int32_t DeviceManagerServiceImpl::DeleteAclV2(const std::string &pkgName, const std::string &localUdid,
    const std::string &remoteUdid, int32_t bindLevel, const std::string &extra)
{
    LOGI("DeleteAclV2 pkgName %{public}s, localUdid %{public}s, remoteUdid %{public}s, bindLevel %{public}d.",
        pkgName.c_str(), GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), bindLevel);
    uint32_t tokenId = 0;
    MultipleUserConnector::GetTokenId(tokenId);
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    bool isNewVersion = IsAuthNewVersion(bindLevel, localUdid, remoteUdid, tokenId, userId);
    if (!isNewVersion) {
        return DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    }
    return DeleteAclForProcV2(localUdid, tokenId, remoteUdid, bindLevel, extra, userId);
}

void DeviceManagerServiceImpl::HandleCommonEventBroadCast(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid)
{
    LOGI("remote udid: %{public}s, foregroundUserIds: %{public}s, backgroundUserIds: %{public}s",
        GetAnonyString(remoteUdid).c_str(), GetIntegerList<uint32_t>(foregroundUserIds).c_str(),
        GetIntegerList<uint32_t>(backgroundUserIds).c_str());
    std::vector<int32_t> rmtFrontUserIdsTemp(foregroundUserIds.begin(), foregroundUserIds.end());
    std::vector<int32_t> rmtBackUserIdsTemp(backgroundUserIds.begin(), backgroundUserIds.end());
    std::vector<int32_t> localUserIds;
    int32_t ret = MultipleUserConnector::GetForegroundUserIds(localUserIds);
    if (ret != DM_OK) {
        LOGE("Get foreground userids failed, ret: %{public}d", ret);
        return;
    }
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    MultipleUserConnector::ClearLockedUser(localUserIds);
    DmOfflineParam offlineParam;
    DeviceProfileConnector::GetInstance().UpdateACL(localUdid, localUserIds, remoteUdid,
        rmtFrontUserIdsTemp, rmtBackUserIdsTemp, offlineParam);
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(rmtBackUserIdsTemp, remoteUdid,
        localUserIds, localUdid);
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(rmtFrontUserIdsTemp, remoteUdid,
        localUserIds, localUdid);
}

extern "C" IDeviceManagerServiceImpl *CreateDMServiceObject(void)
{
    return new DeviceManagerServiceImpl;
}
} // namespace DistributedHardware
} // namespace OHOS
