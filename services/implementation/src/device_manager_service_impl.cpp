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
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "distributed_device_profile_client.h"
#include "dm_error_type.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "dm_softbus_cache.h"
#include "multiple_user_connector.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "dm_common_event_manager.h"
#include "parameter.h"
#include "dm_random.h"
#include "common_event_support.h"
#include "ffrt.h"
using namespace OHOS::EventFwk;

namespace OHOS {
namespace DistributedHardware {

namespace {

// One year 365 * 24 * 60 * 60
constexpr int32_t MAX_ALWAYS_ALLOW_SECONDS = 31536000;
constexpr int32_t BROADCAST_CREDID_LENGTH = 6;
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
constexpr const char* FILED_AUTHORIZED_APP_LIST = "authorizedAppList";
constexpr const char* CHANGE_PINTYPE = "1";
constexpr const char* BIND_CALLER_USERID = "bindCallerUserId";
const char* IS_NEED_JOIN_LNN = "IsNeedJoinLnn";
constexpr const char* NEED_JOIN_LNN = "0";
constexpr const char* NO_NEED_JOIN_LNN = "1";
constexpr const char* TAG_SERVICE_ID = "serviceId";
// currently, we just support one bind session in one device at same time
constexpr size_t MAX_NEW_PROC_SESSION_COUNT_TEMP = 1;
const int32_t USLEEP_TIME_US_550000 = 550000; // 550ms
const int32_t USLEEP_TIME_US_20000 = 20000; // 20ms
const int32_t OPEN_AUTH_SESSION_TIMEOUT = 15000; // 15000ms
const int32_t MAX_TRY_STOP_CNT = 5;
const int32_t DEFAULT_SESSION_ID = -1;
const int32_t METATOKEN_PINCODE_LENGTH = 14;
const int32_t METATOKEN_LENGTH = 8;
constexpr const char* BIND_TARGET_PIN_TIMEOUT_TASK = "devicemanagerTimer:authpininfo";
constexpr int32_t BIND_TARGET_PIN_TIMEOUT = 600;
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
    if (dmVersion == "") {
        LOGE("dmVersion is empty.");
        return false;
    }

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

Session::Session(int sessionId, std::string deviceId)
{
    sessionId_ = sessionId;
    deviceId_ = deviceId;
}

DeviceManagerServiceImpl::DeviceManagerServiceImpl()
{
    LOGI("DeviceManagerServiceImpl constructor");
}

DeviceManagerServiceImpl::~DeviceManagerServiceImpl()
{
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

void DeviceManagerServiceImpl::NotifyCleanEvent(uint64_t logicalSessionId, int32_t connDelayCloseTime)
{
    LOGI("logicalSessionId: %{public}" PRIu64 ".", logicalSessionId);
    ffrt::submit([=]() { CleanAuthMgrByLogicalSessionId(logicalSessionId, connDelayCloseTime); });
}

void DeviceManagerServiceImpl::ImportConfig(std::shared_ptr<AuthManagerBase> authMgr, uint64_t tokenId,
    const std::string &pkgName)
{
    // Import configuration
    std::lock_guard<ffrt::mutex> configsLock(configsMapMutex_);
    if (configsMap_.find(tokenId) != configsMap_.end()) {
        authMgr->ImportAuthCode(configsMap_[tokenId]->pkgName, configsMap_[tokenId]->authCode);
        authMgr->RegisterAuthenticationType(configsMap_[tokenId]->authenticationType);
        LOGI("import authCode Successful tokenId %{public}s, pkgName %{public}s, authCode %{public}s.",
            GetAnonyInt32(tokenId).c_str(), pkgName.c_str(), GetAnonyString(configsMap_[tokenId]->authCode).c_str());
        configsMap_[tokenId] = nullptr;
        configsMap_.erase(tokenId);
        return;
    }
    for (auto it = configsMap_.begin(); it != configsMap_.end();) {
        if (it->second != nullptr && pkgName == it->second->pkgName) {
            authMgr->ImportAuthCode(it->second->pkgName, it->second->authCode);
            authMgr->RegisterAuthenticationType(it->second->authenticationType);
            LOGI("import authCode by pkgName Successful.");
            it->second = nullptr;
            it = configsMap_.erase(it);
            return;
        } else {
            ++it;
        }
    }
}

void DeviceManagerServiceImpl::ImportAuthCodeToConfig(std::shared_ptr<AuthManagerBase> authMgr, uint64_t tokenId)
{
    if (authMgr == nullptr) {
        LOGE("authMgr is null.");
        return;
    }
    std::string pkgName;
    std::string authCode;
    authMgr->GetAuthCodeAndPkgName(pkgName, authCode);
    std::lock_guard<ffrt::mutex> configsLock(configsMapMutex_);
    if (configsMap_.find(tokenId) == configsMap_.end()) {
        configsMap_[tokenId] = std::make_shared<Config>();
    }
    configsMap_[tokenId]->pkgName = pkgName;
    configsMap_[tokenId]->authCode = authCode;
}

int32_t DeviceManagerServiceImpl::InitNewProtocolAuthMgr(bool isSrcSide, uint64_t tokenId, uint64_t logicalSessionId,
    const std::string &pkgName, int sessionId)
{
    LOGI("isSrcSide:%{public}d, tokenId: %{public}s, logicalSesId: %{public}" PRIu64 ", pkgname:%{public}s",
        isSrcSide, GetAnonyInt32(tokenId).c_str(), logicalSessionId, pkgName.c_str());
    std::shared_ptr<AuthManagerBase> authMgr = nullptr;
    // Create a new auth_mgr, create authMgr
    if (isSrcSide) {
        // src end
        authMgr = std::make_shared<AuthSrcManager>(softbusConnector_, hiChainConnector_,
            listener_, hiChainAuthConnector_);
    } else {
        // sink end
        authMgr = std::make_shared<AuthSinkManager>(softbusConnector_, hiChainConnector_,
            listener_, hiChainAuthConnector_);
    }
    StopTimerAndDelDpCallback stopTimerAndDelDpCallback = [=](const std::string &pkgName, int32_t pinExchangeType,
        uint64_t tokenId) {
        this->StopAuthInfoTimerAndDeleteDP(pkgName, pinExchangeType, tokenId);
    };
    CleanNotifyCallback cleanNotifyCallback = [=](const auto &logicalSessionId, const auto &connDelayCloseTime) {
        this->NotifyCleanEvent(logicalSessionId, connDelayCloseTime);
    };
    // Register resource destruction notification function
    authMgr->RegisterCleanNotifyCallback(cleanNotifyCallback);
    authMgr->RegisterStopTimerAndDelDpCallback(stopTimerAndDelDpCallback);
    CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
    hiChainAuthConnector_->RegisterHiChainAuthCallbackById(logicalSessionId, authMgr);
    LOGI("Initialize authMgr token: %{public}" PRId64 ".", tokenId);
    ImportConfig(authMgr, tokenId, pkgName);
    return AddAuthMgr(tokenId, sessionId, authMgr);
}

int32_t DeviceManagerServiceImpl::InitOldProtocolAuthMgr(uint64_t tokenId, const std::string &pkgName, int sessionId)
{
    LOGI("tokenId: %{public}s, pkgname:%{public}s", GetAnonyInt32(tokenId).c_str(), pkgName.c_str());
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMtx_);
        if (authMgr_ == nullptr) {
            CreateGlobalClassicalAuthMgr();
        }
        authMgr_->PrepareSoftbusSessionCallback();
        ImportConfig(authMgr_, tokenId, pkgName);
        int32_t ret = AddAuthMgr(tokenId, sessionId, authMgr_);
        if (ret != DM_OK) {
            authMgr_->ClearSoftbusSessionCallback();
            return ret;
        }
    }
    {
        // The value of logicalSessionId in the old protocol is always 0.
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        logicalSessionId2TokenIdMap_[0] = tokenId;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::InitAndRegisterAuthMgr(bool isSrcSide, uint64_t tokenId,
    std::shared_ptr<Session> session, uint64_t logicalSessionId, const std::string &pkgName)
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
    if (session->version_ == "" || CompareVersion(session->version_, DM_VERSION_5_0_OLD_MAX)) {
        return InitNewProtocolAuthMgr(isSrcSide, tokenId, logicalSessionId, pkgName, session->sessionId_);
    }
    LOGI("InitAndRegisterAuthMgr old authMgr.");
    return InitOldProtocolAuthMgr(tokenId, pkgName, session->sessionId_);
}

void DeviceManagerServiceImpl::CleanSessionMap(std::shared_ptr<Session> session, int32_t connDelayCloseTime)
{
    if (session == nullptr) {
        return;
    }
    session->logicalSessionCnt_.fetch_sub(1);
    if (session->logicalSessionCnt_.load(std::memory_order_relaxed) <= 0) {
        CleanSessionMap(session->sessionId_, connDelayCloseTime);
    }
}

void DeviceManagerServiceImpl::CleanSessionMap(int sessionId, int32_t connDelayCloseTime)
{
    LOGI("In sessionId:%{public}d.", sessionId);
    CHECK_NULL_VOID(softbusConnector_);
    CHECK_NULL_VOID(softbusConnector_->GetSoftbusSession());
    std::string peerUdid = "";
    softbusConnector_->GetSoftbusSession()->GetPeerDeviceId(sessionId, peerUdid);
    auto taskFunc = [=]() {
        CHECK_NULL_VOID(softbusConnector_);
        CHECK_NULL_VOID(softbusConnector_->GetSoftbusSession());
        softbusConnector_->GetSoftbusSession()->CloseAuthSession(sessionId);
    };
    {
        std::lock_guard<ffrt::mutex> lock(isNeedJoinLnnMtx_);
        if (!isNeedJoinLnn_) {
            connDelayCloseTime = 0; // no need joinlnn specified by business, close directly
        }
    }
    ffrt::submit(taskFunc, ffrt::task_attr().delay(connDelayCloseTime));
    {
        std::lock_guard<ffrt::mutex> lock(mapMutex_);
        std::shared_ptr<Session> session = nullptr;
        if (sessionsMap_.find(sessionId) != sessionsMap_.end()) {
            session = sessionsMap_[sessionId];
            sessionsMap_.erase(sessionId);
        }
        if (session != nullptr && deviceId2SessionIdMap_.find(session->deviceId_) != deviceId2SessionIdMap_.end()) {
            deviceId2SessionIdMap_.erase(session->deviceId_);
        }
        if (sessionEnableMutexMap_.find(sessionId) != sessionEnableMutexMap_.end()) {
            sessionStopMap_.erase(sessionId);
            sessionEnableMap_.erase(sessionId);
            sessionEnableCvReadyMap_.erase(sessionId);
            sessionEnableMutexMap_.erase(sessionId);
        }
    }
}

void DeviceManagerServiceImpl::CleanSessionMapByLogicalSessionId(uint64_t logicalSessionId, int32_t connDelayCloseTime)
{
    {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        logicalSessionId2TokenIdMap_.erase(logicalSessionId);
    }
    int32_t sessionId = 0;
    {
        std::lock_guard<ffrt::mutex> sessionIdLock(logicalSessionId2SessionIdMapMtx_);
        if (logicalSessionId2SessionIdMap_.find(logicalSessionId) == logicalSessionId2SessionIdMap_.end()) {
            return;
        }
        sessionId = logicalSessionId2SessionIdMap_[logicalSessionId];
        logicalSessionId2SessionIdMap_.erase(logicalSessionId);
    }
    auto session = GetCurSession(sessionId);
    if (session != nullptr) {
        CleanSessionMap(session, connDelayCloseTime);
    }
    return;
}

void DeviceManagerServiceImpl::CleanAuthMgrByLogicalSessionId(uint64_t logicalSessionId, int32_t connDelayCloseTime)
{
    if (logicalSessionId == 0 && authMgr_ != nullptr) {
        authMgr_->SetTransferReady(true);
        authMgr_->ClearSoftbusSessionCallback();
    }
    uint64_t tokenId = 0;
    {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        if (logicalSessionId2TokenIdMap_.find(logicalSessionId) != logicalSessionId2TokenIdMap_.end()) {
            tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
        } else {
            LOGE("logicalSessionId(%{public}" PRIu64 ") can not find the tokenId.", logicalSessionId);
            return;
        }
    }

    CleanSessionMapByLogicalSessionId(logicalSessionId, connDelayCloseTime);
    hiChainAuthConnector_->UnRegisterHiChainAuthCallbackById(logicalSessionId);
    EraseAuthMgr(tokenId);
    return;
}

std::shared_ptr<AuthManagerBase> DeviceManagerServiceImpl::GetAuthMgr()
{
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    return GetAuthMgrByTokenId(tokenId);
}

int32_t DeviceManagerServiceImpl::AddAuthMgr(uint64_t tokenId, int sessionId, std::shared_ptr<AuthManagerBase> authMgr)
{
    if (authMgr == nullptr) {
        LOGE("authMgr is nullptr.");
        return ERR_DM_POINT_NULL;
    }
    {
        std::lock_guard<ffrt::mutex> mapLock(mapMutex_);
        if (sessionEnableMap_.find(sessionId) != sessionEnableMap_.end() && !sessionEnableMap_[sessionId]) {
            LOGE("session is not open, no need add authMgr.");
            return ERR_DM_AUTH_OPEN_SESSION_FAILED;
        }
    }
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        if (authMgrMap_.size() >= MAX_NEW_PROC_SESSION_COUNT_TEMP) {
            LOGE("Other bind session exist, can not start new one. authMgrMap_.size:%{public}zu", authMgrMap_.size());
            return ERR_DM_AUTH_BUSINESS_BUSY;
        }
        authMgrMap_[tokenId] = authMgr;
    }
    {
        std::lock_guard<ffrt::mutex> mapLock(tokenIdSessionIdMapMtx_);
        if (tokenIdSessionIdMap_.find(tokenId) == tokenIdSessionIdMap_.end()) {
            tokenIdSessionIdMap_[tokenId] = sessionId;
        }
    }
    return DM_OK;
}

void DeviceManagerServiceImpl::EraseAuthMgr(uint64_t tokenId)
{
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        if (authMgrMap_.find(tokenId) != authMgrMap_.end()) {
            LOGI("authMgrMap_ erase token: %{public}" PRIu64 ".", tokenId);
            authMgrMap_.erase(tokenId);
        }
    }
    {
        std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
        LOGI("tokenIdSessionIdMap_ erase token: %{public}" PRIu64 ".", tokenId);
        tokenIdSessionIdMap_.erase(tokenId);
    }
}

// Needed in the callback function
std::shared_ptr<AuthManagerBase> DeviceManagerServiceImpl::GetAuthMgrByTokenId(uint64_t tokenId)
{
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        if (authMgrMap_.find(tokenId) != authMgrMap_.end()) {
            LOGI("authMgrMap_ token: %{public}" PRIu64 ".", tokenId);
            return authMgrMap_[tokenId];
        }
    }
    LOGE("authMgrMap_ not found, token: %{public}" PRIu64 ".", tokenId);
    return nullptr;
}

std::shared_ptr<AuthManagerBase> DeviceManagerServiceImpl::GetCurrentAuthMgr()
{
    uint64_t tokenId = 0;
    {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        if (logicalSessionId2TokenIdMap_.find(0) != logicalSessionId2TokenIdMap_.end()) {
            tokenId = logicalSessionId2TokenIdMap_[0];
        }
    }
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        for (auto &pair : authMgrMap_) {
            if (pair.first != tokenId) {
                return pair.second;
            }
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
    uint32_t randomNumber = static_cast<uint32_t>(rand_dis(gen));

    // Combination of random numbers
    uint64_t randNum = (static_cast<uint64_t>(timestamp) << 32) |
                      (static_cast<uint64_t>(sessionId) << 16) |
                      static_cast<uint64_t>(randomNumber);

    return randNum;
}

int32_t DeviceManagerServiceImpl::Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener)
{
    LOGI("Initialize");
    std::lock_guard<ffrt::mutex> lock(dmServiceImplInitMutex_);
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

void DeviceManagerServiceImpl::ReleaseMaps()
{
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        for (auto& pair : authMgrMap_) {
            pair.second = nullptr;
        }
        authMgrMap_.clear();
    }
    for (auto& pair : sessionsMap_) {
        pair.second = nullptr;
    }
    sessionsMap_.clear();
    {
        std::lock_guard<ffrt::mutex> configsLock(configsMapMutex_);
        for (auto& pair : configsMap_) {
            pair.second = nullptr;
        }
        configsMap_.clear();
    }
    deviceId2SessionIdMap_.clear();
    sessionEnableMutexMap_.clear();
    sessionEnableCvMap_.clear();
    sessionStopMap_.clear();
    sessionEnableMap_.clear();
    sessionEnableCvReadyMap_.clear();
    {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        logicalSessionId2TokenIdMap_.clear();
    }
    {
        std::lock_guard<ffrt::mutex> sessionIdLock(logicalSessionId2SessionIdMapMtx_);
        logicalSessionId2SessionIdMap_.clear();
    }
}

void DeviceManagerServiceImpl::Release()
{
    LOGI("Release");
    std::lock_guard<ffrt::mutex> lock(dmServiceImplInitMutex_);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    commonEventManager_ = nullptr;
#endif
    if (softbusConnector_ != nullptr) {
        softbusConnector_->UnRegisterConnectorCallback();
        softbusConnector_->UnRegisterSoftbusStateCallback();
        softbusConnector_->UnRegisterLeaveLNNCallback();
        if (softbusConnector_->GetSoftbusSession() != nullptr) {
            softbusConnector_->GetSoftbusSession()->UnRegisterSessionCallback();
        }
    }
    if (hiChainConnector_ != nullptr) {
        hiChainConnector_->UnRegisterHiChainCallback();
    }
    if (hiChainAuthConnector_ != nullptr) {
        hiChainAuthConnector_->UnRegisterHiChainAuthCallback();
    }
    authMgr_ = nullptr;
    ReleaseMaps();
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
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t sessionId = DEFAULT_SESSION_ID;
    int32_t tryCnt = 0;
    while (tryCnt < MAX_TRY_STOP_CNT) {
        {
            std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
            if (tokenIdSessionIdMap_.find(tokenId) == tokenIdSessionIdMap_.end()) {
                LOGI("sessionId not create, pkgName:%{public}s, tokenId:%{public}" PRIu64, pkgName.c_str(), tokenId);
                return DM_OK;
            }
            sessionId = tokenIdSessionIdMap_[tokenId];
        }
        if (sessionId > 0) {
            break;
        }
        ffrt_usleep(USLEEP_TIME_US_20000);
        tryCnt++;
    }
    if (sessionId == DEFAULT_SESSION_ID) {
        LOGE("sessionId is creating, pkgName:%{public}s, tokenId:%{public}" PRIu64, pkgName.c_str(), tokenId);
        return ERR_DM_AUTH_BUSINESS_BUSY;
    }
    LOGI("pkgName:%{public}s, sessionId:%{public}d, tokenId:%{public}" PRIu64, pkgName.c_str(), sessionId, tokenId);
    if (sessionEnableCvMap_.find(sessionId) != sessionEnableCvMap_.end()) {
        std::unique_lock<ffrt::mutex> cvLock(sessionEnableMutexMap_[sessionId]);
        if (sessionEnableMap_.find(sessionId) != sessionEnableMap_.end() && !sessionEnableMap_[sessionId]) {
            sessionStopMap_[sessionId] = true;
        }
        sessionEnableCvReadyMap_[sessionId] = true;
        sessionEnableCvMap_[sessionId].notify_all();
        LOGI("notify_all by stop, sessionId:%{public}d, tokenId:%{public}" PRIu64, sessionId, tokenId);
    }
    auto authMgr = GetAuthMgrByTokenId(tokenId);
    int32_t ret = DM_OK;
    if (authMgr != nullptr) {
        ret = authMgr->StopAuthenticateDevice(pkgName);
        if (ret == DM_OK) {
            ffrt_usleep(USLEEP_TIME_US_550000);
        }
    } else {
        CleanSessionMap(sessionId, 0);
    }
    EraseAuthMgr(tokenId);
    return ret;
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
    CHECK_NULL_VOID(softbusConnector_);
    CHECK_NULL_VOID(hiChainConnector_);
    CHECK_NULL_VOID(hiChainAuthConnector_);
    // Create old auth_mar, only create an independent one
    authMgr_ = std::make_shared<DmAuthManager>(softbusConnector_, hiChainConnector_, listener_,
        hiChainAuthConnector_);
    StopTimerAndDelDpCallback stopTimerAndDelDpCallback = [=](const std::string &pkgName, int32_t pinExchangeType,
        uint64_t tokenId) {
        this->StopAuthInfoTimerAndDeleteDP(pkgName, pinExchangeType, tokenId);
    };
    CleanNotifyCallback cleanNotifyCallback = [=](const auto &logicalSessionId, const auto &connDelayCloseTime) {
        this->NotifyCleanEvent(logicalSessionId, connDelayCloseTime);
    };
    authMgr_->RegisterCleanNotifyCallback(cleanNotifyCallback);
    authMgr_->RegisterStopTimerAndDelDpCallback(stopTimerAndDelDpCallback);
    softbusConnector_->RegisterConnectorCallback(authMgr_);
    softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(authMgr_);
    hiChainConnector_->RegisterHiChainCallback(authMgr_);
    hiChainAuthConnector_->RegisterHiChainAuthCallback(authMgr_);
    softbusConnector_->RegisterLeaveLNNCallback(authMgr_);
}

void DeviceManagerServiceImpl::HandleOffline(DmDeviceState devState, DmDeviceInfo &devInfo, const bool isOnline)
{
    std::string trustDeviceId = deviceStateMgr_->GetUdidByNetWorkId(std::string(devInfo.networkId));
    LOGI("deviceStateMgr Udid: %{public}s", GetAnonyString(trustDeviceId).c_str());
    if (trustDeviceId == "") {
        LOGE("HandleOffline not get udid in deviceStateMgr.");
        return;
    }
    std::string udisHash = softbusConnector_->GetDeviceUdidHashByUdid(trustDeviceId);
    if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udisHash.c_str(), udisHash.length()) != 0) {
        return;
    }
    char localUdid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdid, DEVICE_UUID_LENGTH);
    std::string requestDeviceId = std::string(localUdid);
    ProcessInfo processInfo;
    processInfo.pkgName = std::string(DM_PKG_NAME);
    processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
    uint32_t bindType = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    std::map<int32_t, int32_t> userIdAndBindLevel =
        DeviceProfileConnector::GetInstance().GetUserIdAndBindLevel(requestDeviceId, trustDeviceId);
    if (userIdAndBindLevel.empty() || userIdAndBindLevel.find(processInfo.userId) == userIdAndBindLevel.end()) {
        userIdAndBindLevel[processInfo.userId] = INVALIED_TYPE;
    }
    std::vector<ProcessInfo> processInfoVec;
    for (const auto &item : userIdAndBindLevel) {
        if (static_cast<uint32_t>(item.second) == INVALIED_TYPE) {
            LOGI("The offline device is identical account bind type.");
            devInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
            processInfo.userId = item.first;
            processInfoVec.push_back(processInfo);
        } else if (static_cast<uint32_t>(item.second) == USER && bindType == SHARE_TYPE) {
            LOGI("The offline device is device bind level and share bind type.");
            devInfo.authForm = DmAuthForm::SHARE;
            processInfo.userId = item.first;
            processInfoVec.push_back(processInfo);
        } else if (static_cast<uint32_t>(item.second) == USER && bindType != SHARE_TYPE) {
            LOGI("The offline device is device bind type.");
            devInfo.authForm = DmAuthForm::PEER_TO_PEER;
            processInfo.userId = item.first;
            processInfoVec.push_back(processInfo);
        } else if (static_cast<uint32_t>(item.second) == SERVICE || static_cast<uint32_t>(item.second) == APP) {
            LOGI("The offline device is PEER_TO_PEER_TYPE bind type, %{public}" PRIu32, item.second);
            CHECK_NULL_VOID(listener_);
            std::set<ProcessInfo> processInfoSet = listener_->GetAlreadyOnlineProcess();
            processInfoVec.assign(processInfoSet.begin(), processInfoSet.end());
        }
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo, processInfoVec, trustDeviceId, isOnline);
    }
}

void DeviceManagerServiceImpl::HandleOnline(DmDeviceState devState, DmDeviceInfo &devInfo, const bool isOnline)
{
    LOGI("networkId: %{public}s.", GetAnonyString(devInfo.networkId).c_str());
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
    SetOnlineProcessInfo(bindType, processInfo, devInfo, requestDeviceId, trustDeviceId, devState, isOnline);
}

void DeviceManagerServiceImpl::SetOnlineProcessInfo(const uint32_t &bindType, ProcessInfo &processInfo,
    DmDeviceInfo &devInfo, const std::string &requestDeviceId, const std::string &trustDeviceId,
    DmDeviceState devState, const bool isOnline)
{
    std::vector<ProcessInfo> processInfoVec;
    if (bindType == IDENTICAL_ACCOUNT_TYPE) {
        devInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
        processInfoVec.push_back(processInfo);
    } else if (bindType == DEVICE_PEER_TO_PEER_TYPE) {
        devInfo.authForm = DmAuthForm::PEER_TO_PEER;
        processInfoVec.push_back(processInfo);
    } else if (bindType == DEVICE_ACROSS_ACCOUNT_TYPE) {
        devInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
        processInfoVec.push_back(processInfo);
    } else if (bindType == APP_PEER_TO_PEER_TYPE || bindType == SERVICE_PEER_TO_PEER_TYPE) {
        processInfoVec = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(requestDeviceId,
            trustDeviceId, MultipleUserConnector::GetFirstForegroundUserId());
        std::set<ProcessInfo> processInfoSet(processInfoVec.begin(), processInfoVec.end());
        processInfoVec.assign(processInfoSet.begin(), processInfoSet.end());
        devInfo.authForm = DmAuthForm::PEER_TO_PEER;
    } else if (bindType == APP_ACROSS_ACCOUNT_TYPE || bindType == SERVICE_ACROSS_ACCOUNT_TYPE) {
        processInfoVec = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(requestDeviceId,
            trustDeviceId, MultipleUserConnector::GetFirstForegroundUserId());
        std::set<ProcessInfo> processInfoSet(processInfoVec.begin(), processInfoVec.end());
        processInfoVec.assign(processInfoSet.begin(), processInfoSet.end());
        devInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    } else if (bindType == SHARE_TYPE) {
        if (CheckSharePeerSrc(trustDeviceId, requestDeviceId)) {
            LOGI("ProcessDeviceStateChange authForm is share, peer is src.");
            return;
        }
        devInfo.authForm = DmAuthForm::SHARE;
        processInfoVec.push_back(processInfo);
    }
    LOGI("HandleOnline success devInfo authForm is %{public}d.", devInfo.authForm);
    deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo, processInfoVec, trustDeviceId, isOnline);
    return;
}

bool DeviceManagerServiceImpl::CheckSharePeerSrc(const std::string &peerUdid, const std::string &localUdid)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    for (auto &aclItem : profiles) {
        if (aclItem.GetBindType() == DM_SHARE && aclItem.GetTrustDeviceId() == peerUdid) {
            if (aclItem.GetAccesser().GetAccesserDeviceId() == peerUdid &&
                aclItem.GetAccessee().GetAccesseeDeviceId() == localUdid) {
                return true;
            }
            if (aclItem.GetAccesser().GetAccesserDeviceId() == localUdid &&
                aclItem.GetAccessee().GetAccesseeDeviceId() == peerUdid) {
                return false;
            }
        }
    }
    return false;
}

void DeviceManagerServiceImpl::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo,
    const bool isOnline)
{
    LOGI("start, devState = %{public}d, networkId: %{public}s.",
        devState, GetAnonyString(devInfo.networkId).c_str());
    if (deviceStateMgr_ == nullptr) {
        LOGE("deviceStateMgr_ is nullpter!");
        return;
    }
    if (devState == DEVICE_STATE_ONLINE) {
        HandleOnline(devState, devInfo, isOnline);
    } else if (devState == DEVICE_STATE_OFFLINE) {
        HandleOffline(devState, devInfo, isOnline);
    } else {
        std::string peerUdid = "";
        std::string udidHash = GetUdidHashByNetworkId(devInfo.networkId, peerUdid);
        if (memcpy_s(devInfo.deviceId, DM_MAX_DEVICE_ID_LEN, udidHash.c_str(), udidHash.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(udidHash).c_str());
            return;
        }
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        std::vector<ProcessInfo> processInfoVec;
        processInfoVec.push_back(processInfo);
        deviceStateMgr_->HandleDeviceStatusChange(devState, devInfo, processInfoVec, peerUdid, isOnline);
    }
}

std::string DeviceManagerServiceImpl::GetUdidHashByNetworkId(const std::string &networkId, std::string &peerUdid)
{
    if (softbusConnector_ == nullptr) {
        LOGE("softbusConnector_ is nullpter!");
        return "";
    }
    int32_t ret = softbusConnector_->GetUdidByNetworkId(networkId.c_str(), peerUdid);
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId failed ret: %{public}d", ret);
        return "";
    }
    return softbusConnector_->GetDeviceUdidHashByUdid(peerUdid);
}

int DeviceManagerServiceImpl::OnSessionOpened(int sessionId, int result)
{
    bool isNeedCloseSession = false;
    if (sessionEnableCvMap_.find(sessionId) != sessionEnableCvMap_.end()) {
        std::lock_guard<ffrt::mutex> lock(sessionEnableMutexMap_[sessionId]);
        if (result == 0) {
            LOGI("OnSessionOpened successful, sessionId: %{public}d", sessionId);
            if (sessionStopMap_.find(sessionId) != sessionStopMap_.end() && sessionStopMap_[sessionId]) {
                isNeedCloseSession = true;
            } else {
                sessionEnableMap_[sessionId] = true;
            }
        } else {
            LOGE("OnSessionOpened failed, sessionId: %{public}d, res: %{public}d", sessionId, result);
        }
        sessionEnableCvReadyMap_[sessionId] = true;
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
    if (isNeedCloseSession) {
        CleanSessionMap(sessionId, 0);
        return DM_OK;
    }
    // Get the remote deviceId, sink end gives sessionsMap[deviceId] = session;
    {
        std::lock_guard<ffrt::mutex> lock(mapMutex_);
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
        std::lock_guard<ffrt::mutex> lock(mapMutex_);
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
        if (InitAndRegisterAuthMgr(false, tokenId, curSession, logicalSessionId, "") != DM_OK) {
            return nullptr;
        }
        curSession->logicalSessionSet_.insert(logicalSessionId);
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
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
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
    }

    return GetAuthMgrByTokenId(tokenId);
}

int32_t DeviceManagerServiceImpl::GetLogicalIdAndTokenIdBySessionId(uint64_t &logicalSessionId,
    uint64_t &tokenId, int32_t sessionId)
{
    std::lock_guard<ffrt::mutex> sessionIdLock(logicalSessionId2SessionIdMapMtx_);
    for (auto& pair : logicalSessionId2SessionIdMap_) {
        if (pair.second == sessionId) {
            logicalSessionId = pair.first;
            std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
            tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
        }
    }
    if (logicalSessionId == 0 || tokenId == 0) {
        LOGE("can not find logicalSessionId and tokenId.");
        return ERR_DM_AUTH_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::TransferSrcOldAuthMgr(std::shared_ptr<Session> curSession)
{
    // New Old Receive 90, destroy new authMgr, create old authMgr, source side
    // The old protocol has only one session, reverse lookup logicalSessionId and tokenId
    CHECK_NULL_RETURN(curSession, ERR_DM_POINT_NULL);
    int sessionId = curSession->sessionId_;
    uint64_t logicalSessionId = 0;
    uint64_t tokenId = 0;
    int32_t ret = GetLogicalIdAndTokenIdBySessionId(logicalSessionId, tokenId, sessionId);
    if (ret != DM_OK) {
        LOGE("failed, logicalSessionId: %{public}" PRIu64 ", tokenId: %{public}s", logicalSessionId,
            GetAnonyInt32(tokenId).c_str());
        return ret;
    }
    std::string pkgName;
    PeerTargetId peerTargetId;
    std::map<std::string, std::string> bindParam;
    auto authMgr = GetAuthMgrByTokenId(tokenId);
    if (authMgr == nullptr) {
        LOGE("DeviceManagerServiceImpl::TransferSrcOldAuthMgr authManager is nullptr");
        return ERR_DM_POINT_NULL;
    }
    authMgr->GetBindTargetParams(pkgName, peerTargetId, bindParam);
    int32_t authType = -1;
    authMgr->ParseAuthType(bindParam, authType);
    ImportAuthCodeToConfig(authMgr, tokenId);
    EraseAuthMgr(tokenId);
    if (InitAndRegisterAuthMgr(true, tokenId, curSession, logicalSessionId, "") != DM_OK) {
        return ERR_DM_AUTH_FAILED;
    }

    ret = TransferByAuthType(authType, curSession, authMgr, bindParam, logicalSessionId);
    if (ret != DM_OK) {
        LOGE("DeviceManagerServiceImpl::TransferByAuthType TransferByAuthType failed.");
        return ret;
    }
    authMgr->DeleteTimer();
    authMgr = nullptr;
    if (authMgr_->BindTarget(pkgName, peerTargetId, bindParam, sessionId, 0) != DM_OK) {
        LOGE("DeviceManagerServiceImpl::TransferSrcOldAuthMgr authManager BindTarget failed");
        return ERR_DM_AUTH_FAILED;
    }

    if (authType == DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE) {
        int32_t sessionSide = GetSessionSide(curSession->sessionId_);
        authMgr_->OnSessionOpened(curSession->sessionId_, sessionSide, 0);
    }

    LOGI("src transfer to old version success");
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
        {
            std::lock_guard<ffrt::mutex> sessionIdLock(logicalSessionId2SessionIdMapMtx_);
            logicalSessionId2SessionIdMap_[0] = sessionId;
        }
        authMgr->OnSessionDisable();
    } else {
        authMgr_->DisableInsensibleSwitching();
        // send stop message
        // Cannot stop using the new protocol. The new protocol is a signal mechanism and cannot be stopped serially.
        // There will be a delay, causing new objects to be created before the stop is complete.
        // Then the timeout mechanism of the new protocol will stop SoftBus again.
        std::string endMessage = CreateTerminateMessage();
        CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
        (void)softbusConnector_->GetSoftbusSession()->SendData(sessionId, endMessage);
        // Close new protocol session
        CleanAuthMgrByLogicalSessionId(logicalSessionId, 0);
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
    LOGI("bindParam:%{public}s PINTYPE.", PARAM_KEY_AUTH_TYPE);
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
    if (InitAndRegisterAuthMgr(false, tokenId, curSession, 0, bundleName) != DM_OK) {
        // Internal error log printing completed
        return ERR_DM_AUTH_FAILED;
    }

    // Parameter 2 sessionSide is 0, authMgr_ is empty, it must be the sink end.
    // The src end will create the protocol object when BindTarget.
    authMgr_->OnSessionOpened(curSession->sessionId_, 0, 0);
    LOGI("sink transfer to old version success");
    authMgr_->SetTransferReady(false);
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::TransferOldAuthMgr(int32_t msgType, const JsonObject &jsonObject,
    std::shared_ptr<Session> curSession)
{
    int ret = ERR_DM_FAILED;
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
    if (authMgr_ != nullptr && !authMgr_->IsTransferReady()) {
        return DM_OK;
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
        if (msgType == MSG_TYPE_REQ_ACL_NEGOTIATE || msgType == MSG_TYPE_RESP_ACL_NEGOTIATE) {
            curSession->version_ = DM_CURRENT_VERSION;
        }
    } else {
        /**
        Monitor old messages on ports 80/90
        1. New-to-old: When the src side receives a 90 message and detects a version mismatch, it receives
        the 90 message, destroys the current new authMgr, creates a new old protocol authMgr, and re-BindTarget.
        2. Old-to-new: When the sink side receives an 80 message and detects a version mismatch, it receives the 80
        message, directly creates a new old protocol authMgr, and re-OnSessionOpened and OnBytesReceived.
        */
        if (curSession->version_ == "") {
            if (TransferOldAuthMgr(msgType, jsonObject, curSession) != DM_OK) {
                LOGE("DeviceManagerServiceImpl::OnBytesReceived TransferOldAuthMgr failed");
                return;
            }
        } else {
            LOGI("Reuse Old AuthMgr, sessionId: %{public}d.", sessionId);
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
        deviceStateMgr_->SaveNotifyEventInfos(eventId, deviceId);
        if (deviceStateMgr_->ProcNotifyEvent(eventId, deviceId) != DM_OK) {
            LOGE("NotifyEvent failed");
            return ERR_DM_INPUT_PARA_INVALID;
        };
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::GetGroupType(std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("begin");
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
    std::lock_guard<ffrt::mutex> configsLock(configsMapMutex_);
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

    LOGI("pkgName is %{public}s, authCode is %{public}s",
        pkgName.c_str(), GetAnonyString(authCode).c_str());
    auto authMgr = GetAuthMgr();
    if (authMgr != nullptr) {
        authMgr->ImportAuthCode(pkgName, authCode);
    }
    auto config = GetConfigByTokenId();
    config->pkgName = pkgName;
    config->authCode = authCode;   // If registered multiple times, only the last one is kept
    return DM_OK;
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
    if (bindParam.find(PARAM_KEY_IS_SERVICE_BIND) != bindParam.end() &&
        bindParam.at(PARAM_KEY_IS_SERVICE_BIND) == DM_VAL_TRUE) {
        CHECK_NULL_RETURN(listener_, ERR_DM_FAILED);
        if (IsNumberString(deviceId)) {
            return listener_->OpenAuthSessionWithPara(std::stoll(deviceId));
        } else {
            LOGE("OpenAuthSession failed");
            return ERR_DM_FAILED;
        }
    }
    bool hmlEnable160M = false;
    int32_t hmlActionId = 0;
    int invalidSessionId = -1;
    JsonObject jsonObject = GetExtraJsonObject(bindParam);
    if (jsonObject.IsDiscarded()) {
        LOGE("extra string not a json type.");
        return invalidSessionId;
    }
    if (softbusConnector_ == nullptr) {
        return invalidSessionId;
    }
    if (IsHmlSessionType(jsonObject)) {
        auto ret = GetHmlInfo(jsonObject, hmlEnable160M, hmlActionId);
        if (ret != DM_OK) {
            LOGE("OpenAuthSession failed, GetHmlInfo failed.");
            return ret;
        }
        LOGI("hmlActionId %{public}d, hmlEnable160M %{public}d", hmlActionId, hmlEnable160M);
        CHECK_NULL_RETURN(listener_, ERR_DM_FAILED);
        return listener_->OpenAuthSessionWithPara(deviceId, hmlActionId, hmlEnable160M);
    } else {
        return softbusConnector_->GetSoftbusSession()->OpenAuthSession(deviceId);
    }
}

std::shared_ptr<Session> DeviceManagerServiceImpl::GetOrCreateSession(const std::string& deviceId,
    const std::map<std::string, std::string> &bindParam, uint64_t tokenId)
{
    std::shared_ptr<Session> instance;
    int sessionId = -1;

    // Check again whether the corresponding object already exists (because other threads may have created it during
    //  the lock acquisition in the previous step)

    std::lock_guard<ffrt::mutex> lock(mapMutex_);
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

    {
        std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
        tokenIdSessionIdMap_[tokenId] = sessionId;
    }

    std::unique_lock<ffrt::mutex> cvLock(sessionEnableMutexMap_[sessionId]);
    sessionEnableCvReadyMap_[sessionId] = false;
    sessionStopMap_[sessionId] = false;
    sessionEnableMap_[sessionId] = false;
    if (!sessionEnableCvMap_[sessionId].wait_for(cvLock, std::chrono::milliseconds(OPEN_AUTH_SESSION_TIMEOUT),
        [&] { return sessionEnableCvReadyMap_[sessionId]; })) {
        LOGE("wait session enable timeout or enable fail, sessionId: %{public}d.", sessionId);
        return nullptr;
    }
    if (sessionStopMap_.find(sessionId) != sessionStopMap_.end() && sessionStopMap_[sessionId]) {
        LOGW("stop by caller, sessionId: %{public}d.", sessionId);
        return nullptr;
    }
    if (sessionEnableMap_.find(sessionId) != sessionEnableMap_.end() && !sessionEnableMap_[sessionId]) {
        LOGW("Open session failed, sessionId: %{public}d.", sessionId);
        return nullptr;
    }
    instance = std::make_shared<Session>(sessionId, deviceId);
    deviceId2SessionIdMap_[deviceId] = sessionId;
    sessionsMap_[sessionId] = instance;

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
    std::string isNeedJoinLnnStr;
    if (bindParam.find(IS_NEED_JOIN_LNN) != bindParam.end()) {
        isNeedJoinLnnStr = bindParam.at(IS_NEED_JOIN_LNN);
    }
    LOGI("isNeedJoinLnnStr: %{public}s.", isNeedJoinLnnStr.c_str());
    {
        std::lock_guard<ffrt::mutex> lock(isNeedJoinLnnMtx_);
        if (isNeedJoinLnnStr == NEED_JOIN_LNN || isNeedJoinLnnStr == NO_NEED_JOIN_LNN) {
            isNeedJoinLnn_ = std::atoi(isNeedJoinLnnStr.c_str());
        }
    }
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    int32_t index = 0;
    int32_t ret = GetDeviceInfo(targetId, addrType, deviceId, deviceInfo, index);
    if (ret != DM_OK) {
        LOGE("GetDeviceInfo failed, ret: %{public}d", ret);
    }
    deviceInfo->addrNum = static_cast<uint32_t>(index);
    if (bindParam.find(PARAM_KEY_IS_SERVICE_BIND) != bindParam.end() &&
        bindParam.at(PARAM_KEY_IS_SERVICE_BIND) == DM_VAL_TRUE) {
        deviceId = std::to_string(targetId.serviceId);
    }
    if (softbusConnector_->AddMemberToDiscoverMap(deviceId, deviceInfo) != DM_OK) {
        LOGE("DeviceManagerServiceImpl::ParseConnectAddr failed, AddMemberToDiscoverMap failed.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    deviceInfo = nullptr;
    return DM_OK;
}

void DeviceManagerServiceImpl::SaveTokenIdAndSessionId(uint64_t &tokenId,
    int32_t &sessionId, uint64_t &logicalSessionId)
{
    {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        logicalSessionId2TokenIdMap_[logicalSessionId] = tokenId;
    }
    {
        std::lock_guard<ffrt::mutex> sessionIdLock(logicalSessionId2SessionIdMapMtx_);
        logicalSessionId2SessionIdMap_[logicalSessionId] = sessionId;
    }
}

bool DeviceManagerServiceImpl::ParseConnectAddrAndSetProcessInfo(PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam, ProcessInfo &processInfo,
    const std::string &pkgName, uint64_t tokenId)
{
    processInfo.pkgName = pkgName;
    if (bindParam.count(BIND_CALLER_USERID) != 0) {
        processInfo.userId = std::atoi(bindParam.at(BIND_CALLER_USERID).c_str());
    }
    std::string deviceId = "";
    if (ParseConnectAddr(targetId, deviceId, bindParam) == DM_OK) {
        targetId.deviceId = deviceId;
    } else {
        if (targetId.deviceId.empty()) {
            LOGE("DeviceManagerServiceImpl::BindTarget failed, ParseConnectAddr failed.");
            OnAuthResultAndOnBindResult(processInfo, targetId, "", ERR_DM_INPUT_PARA_INVALID, tokenId);
            return false;
        }
    }
    return true;
}

void DeviceManagerServiceImpl::OnAuthResultAndOnBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId,
    const std::string &deviceId, int32_t reason, uint64_t tokenId)
{
    {
        std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
        auto iter = tokenIdSessionIdMap_.find(tokenId);
        if (iter != tokenIdSessionIdMap_.end()) {
            tokenIdSessionIdMap_.erase(iter);
        }
    }
    CHECK_NULL_VOID(listener_);
    listener_->OnAuthResult(processInfo, deviceId, "", DmAuthStatus::STATUS_DM_AUTH_DEFAULT, reason);
    listener_->OnBindResult(processInfo, targetId, reason, DmAuthStatus::STATUS_DM_AUTH_DEFAULT, "");
}

void DeviceManagerServiceImpl::BindTargetImpl(uint64_t tokenId, const std::string &pkgName,
    const PeerTargetId &targetId, const std::map<std::string, std::string> &bindParam)
{
    PeerTargetId targetIdTmp = const_cast<PeerTargetId&>(targetId);
    ProcessInfo processInfo;
    if (!ParseConnectAddrAndSetProcessInfo(targetIdTmp, bindParam, processInfo, pkgName, tokenId)) {
        return;
    }
    // Created only at the source end. The same target device will not be created repeatedly with the new protocol.
    std::shared_ptr<Session> curSession = GetOrCreateSession(targetIdTmp.deviceId, bindParam, tokenId);
    if (curSession == nullptr) {
        LOGE("create session failed. Target deviceId: %{public}s.", GetAnonyString(targetIdTmp.deviceId).c_str());
        OnAuthResultAndOnBindResult(processInfo, targetId, targetIdTmp.deviceId, ERR_DM_TIME_OUT, tokenId);
        return;
    }

    // Logical session random number
    int sessionId = curSession->sessionId_;
    uint64_t logicalSessionId = 0;
    if (curSession->version_ == "" || CompareVersion(curSession->version_, DM_VERSION_5_0_OLD_MAX)) {
        logicalSessionId = GenerateRandNum(sessionId);
        if (curSession->logicalSessionSet_.find(logicalSessionId) != curSession->logicalSessionSet_.end()) {
            LOGE("Failed to create the logical session.");
            CleanSessionMap(sessionId, 0);
            OnAuthResultAndOnBindResult(processInfo, targetId, targetIdTmp.deviceId,
                ERR_DM_INPUT_PARA_INVALID, tokenId);
            return;
        }
    }

    // Create on the src end.
    int32_t ret = InitAndRegisterAuthMgr(true, tokenId, curSession, logicalSessionId, "");
    if (ret != DM_OK) {
        LOGE("InitAndRegisterAuthMgr failed, ret %{public}d.", ret);
        CleanSessionMap(sessionId, 0);
        OnAuthResultAndOnBindResult(processInfo, targetId, targetIdTmp.deviceId, ret, tokenId);
        return;
    }
    curSession->logicalSessionSet_.insert(logicalSessionId);
    curSession->logicalSessionCnt_.fetch_add(1);
    SaveTokenIdAndSessionId(tokenId, sessionId, logicalSessionId);
    auto authMgr = GetAuthMgrByTokenId(tokenId);
    if (authMgr == nullptr) {
        CleanAuthMgrByLogicalSessionId(logicalSessionId, 0);
        OnAuthResultAndOnBindResult(processInfo, targetId, targetIdTmp.deviceId, ERR_DM_POINT_NULL, tokenId);
        return;
    }
    authMgr->SetBindTargetParams(targetId);
    if ((ret = authMgr->BindTarget(pkgName, targetIdTmp, bindParam, sessionId, logicalSessionId)) != DM_OK) {
        LOGE("authMgr BindTarget failed, ret %{public}d.", ret);
        CleanAuthMgrByLogicalSessionId(logicalSessionId, 0);
        OnAuthResultAndOnBindResult(processInfo, targetId, targetIdTmp.deviceId, ret, tokenId);
    }
    LOGI("end, tokenId %{public}s.", GetAnonyInt32(tokenId).c_str());
    return;
}

int32_t DeviceManagerServiceImpl::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam)
{
    if (pkgName.empty()) {
        LOGE("BindTarget failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmBindCallerInfo bindCallerInfo;
    GetBindCallerInfo(bindCallerInfo, pkgName);
    std::map<std::string, std::string> bindParamTmp;
    SetBindCallerInfoToBindParam(bindParam, bindParamTmp, bindCallerInfo);
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    LOGI("DeviceManagerServiceImpl In, pkgName:%{public}s, tokenId:%{public}" PRIu64, pkgName.c_str(), tokenId);
    {
        std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
        if (tokenIdSessionIdMap_.find(tokenId) != tokenIdSessionIdMap_.end()) {
            LOGE("BindTarget failed, this device is being bound. please try again later,"
                "pkgName:%{public}s, tokenId:%{public}" PRIu64, pkgName.c_str(), tokenId);
            return ERR_DM_AUTH_BUSINESS_BUSY;
        }
        if (tokenIdSessionIdMap_.size() >= MAX_NEW_PROC_SESSION_COUNT_TEMP) {
            LOGE("Other bind exist, can not start new one. size:%{public}zu", tokenIdSessionIdMap_.size());
            return ERR_DM_AUTH_BUSINESS_BUSY;
        }
        tokenIdSessionIdMap_[tokenId] = DEFAULT_SESSION_ID;
    }
    std::thread newThread(&DeviceManagerServiceImpl::BindTargetImpl, this, tokenId, pkgName, targetId, bindParamTmp);
    newThread.detach();
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::DpAclAdd(const std::string &udid)
{
    LOGI("start udid %{public}s.", GetAnonyString(udid).c_str());
    MultipleUserConnector::SetSwitchOldUserId(MultipleUserConnector::GetCurrentAccountUserID());
    MultipleUserConnector::SetSwitchOldAccountId(MultipleUserConnector::GetOhosAccountId());
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(deviceStateMgr_, ERR_DM_POINT_NULL);
    if (SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(udid)) {
        LOGI("DeviceManagerServiceImpl DpAclAdd identical account and online");
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        softbusConnector_->SetProcessInfo(processInfo);
        deviceStateMgr_->OnDeviceOnline(udid, DmAuthForm::IDENTICAL_ACCOUNT);
    }
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

uint64_t DeviceManagerServiceImpl::GetTokenIdByNameAndDeviceId(std::string extra, std::string requestDeviceId)
{
    if (extra.empty()) {
        LOGE("DeviceManagerServiceImpl::GetTokenIdByNameAndDeviceId error: extra.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (requestDeviceId.empty()) {
        LOGE("DeviceManagerServiceImpl::GetTokenIdByNameAndDeviceId error: requestDeviceId.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    return DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(extra, requestDeviceId);
}

std::unordered_map<std::string, DmAuthForm> DeviceManagerServiceImpl::GetAppTrustDeviceIdList(
    std::string pkgname)
{
    char localDeviceId[DEVICE_UUID_LENGTH];
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string deviceId = reinterpret_cast<char *>(localDeviceId);
    return DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgname, deviceId);
}

void DeviceManagerServiceImpl::HandleIdentAccountLogout(const DMAclQuadInfo &info, const std::string &accountId)
{
    LOGI("localUdid %{public}s, localUserId %{public}d, peerUdid %{public}s, peerUserId %{public}d.",
        GetAnonyString(info.localUdid).c_str(), info.localUserId, GetAnonyString(info.peerUdid).c_str(),
        info.peerUserId);
    DmOfflineParam offlineParam;
    bool notifyOffline = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(info, accountId,
        offlineParam);
    DeleteGroupByBundleName(info.localUdid, info.localUserId, offlineParam.needDelAclInfos);
    CHECK_NULL_VOID(hiChainAuthConnector_);
    {
        std::lock_guard lock(logoutMutex_);
        hiChainAuthConnector_->DeleteCredential(info.peerUdid, info.localUserId, info.peerUserId);
        DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    }

    std::set<std::string> pkgNameSet;
    GetBundleName(info, pkgNameSet, notifyOffline);
    if (notifyOffline) {
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        CHECK_NULL_VOID(listener_);
        listener_->SetExistPkgName(pkgNameSet);
        CHECK_NULL_VOID(deviceStateMgr_);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(info.peerUdid);
        deviceStateMgr_->OnDeviceOffline(info.peerUdid, isOnline);
    }
}

void DeviceManagerServiceImpl::GetBundleName(const DMAclQuadInfo &info, std::set<std::string> &pkgNameSet,
    bool &notifyOffline)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        std::string accesserPkgName = item.GetAccesser().GetAccesserBundleName();
        std::string accesseePkgName = item.GetAccessee().GetAccesseeBundleName();
        if (accesserUdid == info.localUdid && accesserUserId == info.localUserId &&
            accesseeUdid == info.peerUdid && accesseeUserId == info.peerUserId &&
            (!accesserPkgName.empty())) {
            if (item.GetBindLevel() == USER) {
                notifyOffline = false;
                return;
            }
            pkgNameSet.insert(accesserPkgName);
            continue;
        }
        if (accesserUdid == info.peerUdid && accesserUserId == info.peerUserId &&
            accesseeUdid == info.localUdid && accesseeUserId == info.localUserId &&
            (!accesseePkgName.empty())) {
            if (item.GetBindLevel() == USER) {
                notifyOffline = false;
                return;
            }
            pkgNameSet.insert(accesseePkgName);
            continue;
        }
    }
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
        LOGI("on screen locked.");
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        for (auto& pair : authMgrMap_) {
            if (pair.second != nullptr) {
                LOGI("tokenId: %{public}s.", GetAnonyInt32(pair.first).c_str());
                pair.second->OnScreenLocked();
            }
        }
        return;
    }
    LOGI("error.");
}

bool DeviceManagerServiceImpl::CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    return DeviceProfileConnector::GetInstance().CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
}

bool DeviceManagerServiceImpl::CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_POINT_NULL);
    if (!hiChainConnector_->IsDevicesInP2PGroup(srcUdid, sinkUdid)) {
        return DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    } else {
        return true;
    }
}

void DeviceManagerServiceImpl::DeleteAndNotifyOffline(const std::string &udid)
{
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().FilterNeedDeleteACL(udid);
    if (offlineParam.isNewVersion) {
        DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    } else {
        for (auto item : offlineParam.needDelAclInfos) {
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.accessControlId);
        }
        CHECK_NULL_VOID(hiChainAuthConnector_);
        hiChainAuthConnector_->DeleteCredential(udid, MultipleUserConnector::GetCurrentAccountUserID(),
            offlineParam.peerUserId);
    }
    NotifyDeviceOrAppOffline(offlineParam, udid);
}

void DeviceManagerServiceImpl::HandleDeviceNotTrust(const std::string &udid)
{
    LOGI("udid: %{public}s.", GetAnonyString(udid).c_str());
    if (udid.empty()) {
        LOGE("HandleDeviceNotTrust udid is empty.");
        return;
    }
    DeleteAndNotifyOffline(udid);
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

std::vector<std::string> DeviceManagerServiceImpl::GetDeviceIdByUserIdAndTokenId(int32_t userId,
    int32_t tokenId)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    std::vector<int32_t> userIds;
    userIds.push_back(userId);
    return DeviceProfileConnector::GetInstance().GetDeviceIdAndUdidListByTokenId(userIds, localUdid, tokenId);
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
        DMAclQuadInfo info = {item.first, item.second, remoteUdid, remoteUserId};
        bool notifyOffline = DeviceProfileConnector::GetInstance().DeleteAclByActhash(info,
            remoteAccountHash, offlineParam);

        CHECK_NULL_VOID(hiChainConnector_);
        hiChainConnector_->DeleteAllGroup(item.second, remoteUdid);
        CHECK_NULL_VOID(hiChainAuthConnector_);
        {
            std::lock_guard lock(logoutMutex_);
            hiChainAuthConnector_->DeleteCredential(remoteUdid, item.second, remoteUserId);
            DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
        }

        std::set<std::string> pkgNameSet;
        GetBundleName(info, pkgNameSet, notifyOffline);
        if (notifyOffline) {
            CHECK_NULL_VOID(softbusConnector_);
            softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
            CHECK_NULL_VOID(listener_);
            listener_->SetExistPkgName(pkgNameSet);
            CHECK_NULL_VOID(deviceStateMgr_);
            bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
            deviceStateMgr_->OnDeviceOffline(remoteUdid, isOnline);
        }
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
    LOGI("start");
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

void DeviceManagerServiceImpl::HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    DmOfflineParam offlineParam;
    int32_t bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(
        remoteUserId, remoteUdid, localUdid, offlineParam, tokenId);
    if (static_cast<uint32_t>(bindType) == DM_INVALIED_TYPE) {
        LOGE("Invalied bindtype.");
        return;
    }
    DeleteGroup(DM_PKG_NAME, remoteUdid);
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
}

void DeviceManagerServiceImpl::HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId)
{
    LOGI("tokenId = %{public}s.", GetAnonyInt32(tokenId).c_str());
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId, localUdid);

    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);

    //first, not app/service/user/DM_IDENTICAL_ACCOUNT acl, determin if need clear lnn acl
    if (offlineParam.allLeftAppOrSvrAclInfos.empty() && offlineParam.allUserAclInfos.empty() &&
        !offlineParam.allLnnAclInfos.empty()) {
        LOGI("No acl exist, clear lnn acl");
        DeleteSkCredAndAcl(offlineParam.allLnnAclInfos);
    }

    //second, not user/DM_IDENTICAL_ACCOUNT acl, but app/service acl exist
    //determin if need report offline to unbind bundle
    if (!offlineParam.allLeftAppOrSvrAclInfos.empty() && offlineParam.allUserAclInfos.empty()) {
        LOGI("after clear target acl, No user acl exist, report offline");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
        softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
    }
    //third, not lnn acl, determin if delete Credential
    if (offlineParam.allLnnAclInfos.empty()) {
        CHECK_NULL_VOID(hiChainAuthConnector_);
        hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
            remoteUserId);
    }
}

void DeviceManagerServiceImpl::HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId, int32_t peerTokenId)
{
    LOGI("DeviceManagerServiceImpl::HandleAppUnBindEvent remoteUserId: %{public}s, remoteUdid: %{public}s,"
        "tokenId = %{public}s, peerTokenId = %{public}s.", GetAnonyInt32(remoteUserId).c_str(),
        GetAnonyString(remoteUdid).c_str(), GetAnonyInt32(tokenId).c_str(), GetAnonyInt32(peerTokenId).c_str());
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid,
            tokenId, localUdid, peerTokenId);

    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    //first, not app/service/user/DM_IDENTICAL_ACCOUNT acl, determin if need clear lnn acl
    if (offlineParam.allLeftAppOrSvrAclInfos.empty() && offlineParam.allUserAclInfos.empty() &&
        !offlineParam.allLnnAclInfos.empty()) {
        LOGI("No acl exist, clear lnn acl");
        DeleteSkCredAndAcl(offlineParam.allLnnAclInfos);
    }

    //second, not user/DM_IDENTICAL_ACCOUNT acl, but app/service acl exist
    //determin if need report offline to unbind bundle
    if (!offlineParam.allLeftAppOrSvrAclInfos.empty() && offlineParam.allUserAclInfos.empty()) {
        LOGI("after clear target acl, No user acl exist, report offline");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
        softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
    }
    //third, not lnn acl, determin if delete Credential
    if (offlineParam.allLnnAclInfos.empty()) {
        CHECK_NULL_VOID(hiChainAuthConnector_);
        hiChainAuthConnector_->DeleteCredential(remoteUdid, MultipleUserConnector::GetCurrentAccountUserID(),
            remoteUserId);
    }
}

void DeviceManagerServiceImpl::HandleServiceUnBindEvent(int32_t userId, const std::string &remoteUdid,
    int32_t remoteTokenId)
{
    LOGI("remoteTokenId = %{public}s, userId: %{public}d, remoteUdid: %{public}s.",
        GetAnonyInt32(remoteTokenId).c_str(), userId, GetAnonyString(remoteUdid).c_str());
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localUdidTemp);
    DmOfflineParam offlineParam =
        DeviceProfileConnector::GetInstance().HandleServiceUnBindEvent(userId, remoteUdid, localUdid, remoteTokenId);

    //first, app/service acl exist, determin if need local acl
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    //second, not app/service/user/DM_IDENTICAL_ACCOUNT acl exist, determin if need clear lnn acl
    if (offlineParam.allLeftAppOrSvrAclInfos.empty() && offlineParam.allUserAclInfos.empty()) {
        LOGI("No acl exist, clear lnn acl");
        DeleteSkCredAndAcl(offlineParam.allLnnAclInfos);
    }

    //Third, not user/DM_IDENTICAL_ACCOUNT acl exist but app/service acl exist,
    //determin if need report offline to unbind bundle
    if (!offlineParam.allLeftAppOrSvrAclInfos.empty() && offlineParam.allUserAclInfos.empty()) {
        LOGI("after clear target acl, No user acl exist, report offline");
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
        softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
    }
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
    std::vector<ProcessInfo> processInfoVec;
    if (bindType == INVALIED_TYPE) {
        return;
    } else if (bindType == IDENTICAL_ACCOUNT_TYPE || bindType == DEVICE_PEER_TO_PEER_TYPE ||
        bindType == DEVICE_ACROSS_ACCOUNT_TYPE) {
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        processInfoVec.push_back(processInfo);
    } else if (bindType == APP_PEER_TO_PEER_TYPE || bindType == APP_ACROSS_ACCOUNT_TYPE) {
        processInfoVec = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(requestDeviceId,
            trustDeviceId, MultipleUserConnector::GetFirstForegroundUserId());
    }
    deviceStateMgr_->HandleDeviceScreenStatusChange(devInfo, processInfoVec);
}

int32_t DeviceManagerServiceImpl::SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string remoteAclList, bool isDelImmediately)
{
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    return softbusConnector_->SyncLocalAclListProcess(localDevUserInfo, remoteDevUserInfo,
        remoteAclList, isDelImmediately);
}

int32_t DeviceManagerServiceImpl::GetAclListHash(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclList)
{
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    return softbusConnector_->GetAclListHash(localDevUserInfo, remoteDevUserInfo, aclList);
}

void DeviceManagerServiceImpl::DeleteAlwaysAllowTimeOut()
{
    LOGI("Start");
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

int32_t DeviceManagerServiceImpl::ProcessAppUninstall(int32_t userId, int32_t accessTokenId)
{
    LOGE("DeviceManagerServiceImpl::ProcessAppUninstall userId = %{public}s, accessTokenId = %{public}s.",
        GetAnonyInt32(userId).c_str(), GetAnonyInt32(accessTokenId).c_str());
    CHECK_NULL_RETURN(listener_, ERR_DM_POINT_NULL);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    LOGI("delete ACL size is %{public}zu", profiles.size());
    if (profiles.size() == 0) {
        return DM_OK;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<std::pair<int32_t, std::string>> delACLInfoVec;
    std::vector<int32_t> userIdVec;
    GetDelACLInfoVec(accessTokenId, profiles, delACLInfoVec, userIdVec, userId);

    if (delACLInfoVec.size() == 0 || userIdVec.size() == 0) {
        return DM_OK;
    }

    CHECK_NULL_RETURN(hiChainConnector_, ERR_DM_POINT_NULL);
    hiChainConnector_->DeleteGroupByACL(delACLInfoVec, userIdVec);
    return DM_OK;
}

void DeviceManagerServiceImpl::ProcessUnBindApp(int32_t userId, int32_t accessTokenId,
    const std::string &extra, const std::string &udid)
{
    LOGI("DeviceManagerServiceImpl::ProcessUnBindApp userId = %{public}s, accessTokenId = %{public}s,"
        "extra = %{public}s.", GetAnonyInt32(userId).c_str(), GetAnonyInt32(accessTokenId).c_str(),
        GetAnonyString(extra).c_str());

    JsonObject extraInfoJson(extra);
    if (extraInfoJson.IsDiscarded()) {
        LOGE("ParseExtra extraInfoJson error");
        HandleAppUnBindEvent(userId, udid, accessTokenId);
        return;
    }
    if (extraInfoJson.Contains(TAG_PEER_TOKENID) && extraInfoJson[TAG_PEER_TOKENID].IsNumberInteger()) {
        uint64_t peerTokenId = extraInfoJson[TAG_PEER_TOKENID].Get<uint64_t>();
        HandleAppUnBindEvent(userId, udid, accessTokenId, static_cast<int32_t>(peerTokenId));
    }
    return;
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
        if (accessTokenId != static_cast<int32_t>(accesssertokenId) &&
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
        if (accessTokenId == static_cast<int32_t>(accessseetokenId)) {
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

void DeviceManagerServiceImpl::GetDelACLInfoVec(const int32_t &accessTokenId,
    std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles,
    std::vector<std::pair<int32_t, std::string>> &delACLInfoVec, std::vector<int32_t> &userIdVec,
    const uint32_t &userId)
{
    std::map<int64_t, DistributedDeviceProfile::AccessControlProfile> delProfileMap;
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    for (auto &item : profiles) {
        int64_t accesssertokenId = item.GetAccesser().GetAccesserTokenId();
        int64_t accessseetokenId = item.GetAccessee().GetAccesseeTokenId();
        if (accessTokenId != static_cast<int32_t>(accesssertokenId) &&
            accessTokenId != static_cast<int32_t>(accessseetokenId)) {
            continue;
        }
        if (accessTokenId == static_cast<int32_t>(accesssertokenId) &&
            userId == static_cast<uint32_t>(item.GetAccesser().GetAccesserUserId()) &&
            localUdid == item.GetAccessee().GetAccesseeDeviceId()) {
            DmOfflineParam offlineParam;
            delProfileMap[item.GetAccessControlId()] = item;
            DeviceProfileConnector::GetInstance().CacheAceeAclId(item, offlineParam.needDelAclInfos);
            DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
            if (item.GetBindLevel() == USER) {
                userIdVec.push_back(item.GetAccessee().GetAccesseeUserId());
                delACLInfoVec.push_back(std::pair<int32_t, std::string>(item.GetAccessee().GetAccesseeUserId(),
                    item.GetAccesser().GetAccesserDeviceId()));
            }
        }
        if (accessTokenId == static_cast<int32_t>(accessseetokenId) &&
            userId == static_cast<uint32_t>(item.GetAccessee().GetAccesseeUserId()) &&
            localUdid == item.GetAccesser().GetAccesserDeviceId()) {
            DmOfflineParam offlineParam;
            DeviceProfileConnector::GetInstance().CacheAcerAclId(item, offlineParam.needDelAclInfos);
            delProfileMap[item.GetAccessControlId()] = item;
            DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
            if (item.GetBindLevel() == USER) {
                userIdVec.push_back(item.GetAccesser().GetAccesserUserId());
                delACLInfoVec.push_back(std::pair<int32_t, std::string>(item.GetAccesser().GetAccesserUserId(),
                    item.GetAccessee().GetAccesseeDeviceId()));
            }
        }
    }
    DeleteAclByDelProfileMap(delProfileMap, profiles, localUdid);
}

void DeviceManagerServiceImpl::DeleteAclByDelProfileMap(
    const std::map<int64_t, DistributedDeviceProfile::AccessControlProfile> &delProfileMap,
    std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles, const std::string &localUdid)
{
    for (auto item : delProfileMap) {
        for (auto it = profiles.begin(); it != profiles.end();) {
            if (item.first == it->GetAccessControlId()) {
                it = profiles.erase(it);
            } else {
                it++;
            }
        }
    }
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

void DeviceManagerServiceImpl::HandleCredentialDeleted(const char *credId, const char *credInfo,
    const std::string &localUdid, std::string &remoteUdid, bool &isSendBroadCast)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    {
        std::lock_guard lock(logoutMutex_);
        profiles = DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    }

    JsonObject jsonObject;
    jsonObject.Parse(std::string(credInfo));
    if (jsonObject.IsDiscarded()) {
        LOGE("credInfo prase error.");
        return;
    }
    std::string deviceIdTag = "deviceId";
    std::string userIdTag = "osAccountId";
    int32_t localUserId = MultipleUserConnector::GetCurrentAccountUserID();
    int32_t userId = 0;
    if (IsString(jsonObject, deviceIdTag)) {
        remoteUdid = jsonObject[deviceIdTag].Get<std::string>();
    }
    if (IsInt32(jsonObject, userIdTag)) {
        userId = jsonObject[userIdTag].Get<int32_t>();
    }

    for (const auto &item : profiles) {
        if (item.GetBindType() != DM_SHARE) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserCredentialIdStr() == credId &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserUserId() == localUserId &&
            item.GetAccessee().GetAccesseeUserId() == userId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid) ||
            (item.GetAccessee().GetAccesseeCredentialIdStr() == credId &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeUserId() == localUserId &&
            item.GetAccesser().GetAccesserUserId() == userId &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid)) {
            isSendBroadCast = true;
            DeleteSessionKey(userId, item);
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.GetAccessControlId());
        }
    }
}

void DeviceManagerServiceImpl::HandleShareUnbindBroadCast(const std::string &credId, const int32_t &userId,
    const std::string &localUdid)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    {
        std::lock_guard lock(logoutMutex_);
        profiles = DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    }
    int32_t localUserId = MultipleUserConnector::GetCurrentAccountUserID();
    for (const auto &item : profiles) {
        if (item.GetBindType() != DM_SHARE) {
            continue;
        }
        std::string accesserCredId = "";
        std::string accesseeCredId = "";
        for (int32_t i = 0; i < BROADCAST_CREDID_LENGTH; i++) {
            accesserCredId += item.GetAccesser().GetAccesserCredentialIdStr()[i];
            accesseeCredId += item.GetAccessee().GetAccesseeCredentialIdStr()[i];
        }
        if (accesserCredId == credId && item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeUserId() == localUserId &&
            item.GetAccesser().GetAccesserUserId() == userId) {
            DeleteSessionKey(userId, item);
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.GetAccessControlId());
        }
        if (accesseeCredId == credId && item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserUserId() == localUserId &&
            item.GetAccessee().GetAccesseeUserId() == userId) {
            DeleteSessionKey(userId, item);
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.GetAccessControlId());
        }
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
    LOGI("pkgName %{public}s, localUdid %{public}s, remoteUdid %{public}s, bindLevel %{public}d.",
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
            bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
            softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
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

void DeviceManagerServiceImpl::DeleteCredential(DmAclIdParam &acl)
{
    CHECK_NULL_VOID(hiChainAuthConnector_);
    JsonObject credJson;
    int32_t ret = hiChainAuthConnector_->QueryCredInfoByCredId(acl.userId, acl.credId, credJson);
    if (ret != DM_OK || !credJson.Contains(acl.credId)) {
        LOGE("DeleteCredential err, ret:%{public}d", ret);
        return;
    }
    if (!credJson[acl.credId].Contains(FILED_AUTHORIZED_APP_LIST)) {
        ret = hiChainAuthConnector_->DeleteCredential(acl.userId, acl.credId);
        if (ret != DM_OK) {
            LOGE("DeletecredId err, ret:%{public}d", ret);
        }
        return;
    }
    DistributedDeviceProfile::AccessControlProfile profile =
        DeviceProfileConnector::GetInstance().GetAccessControlProfileByAccessControlId(acl.accessControlId);
    if (profile.GetAccessControlId() != acl.accessControlId) {
        LOGE("DeleteCredential, no found profile");
        return;
    }
    std::vector<std::string> appList;
    credJson[acl.credId][FILED_AUTHORIZED_APP_LIST].Get(appList);
    auto erIt = std::find(appList.begin(), appList.end(), std::to_string(profile.GetAccesser().GetAccesserTokenId()));
    if (erIt != appList.end()) {
        appList.erase(erIt);
    }
    auto eeIt = std::find(appList.begin(), appList.end(), std::to_string(profile.GetAccessee().GetAccesseeTokenId()));
    if (eeIt != appList.end()) {
        appList.erase(eeIt);
    }
    if (appList.size() == 0) {
        ret = hiChainAuthConnector_->DeleteCredential(acl.userId, acl.credId);
        if (ret != DM_OK) {
            LOGE("DeletecredId err, ret:%{public}d", ret);
        }
        return;
    }
    hiChainAuthConnector_->UpdateCredential(acl.credId, acl.userId, appList);
}

int32_t DeviceManagerServiceImpl::DeleteSkCredAndAcl(const std::vector<DmAclIdParam> &acls)
{
    LOGI("start.");
    int32_t ret = DM_OK;
    if (acls.empty()) {
        return ret;
    }
    CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
    for (auto item : acls) {
        LOGI("DeleteSkCredAndAcl, userId:%{public}d, skId:%{public}d, credId:%{public}s",
            item.userId, item.skId, GetAnonyString(item.credId).c_str());
        ret = DeviceProfileConnector::GetInstance().DeleteSessionKey(item.userId, item.skId);
        if (ret != DM_OK) {
            LOGE("DeleteSessionKey err, ret:%{public}d", ret);
        }
        DeleteCredential(item);
        DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.accessControlId);
    }
    return ret;
}

int32_t DeviceManagerServiceImpl::DeleteAclForProcV2(const std::string &localUdid, uint32_t localTokenId,
    const std::string &remoteUdid, int32_t bindLevel, const std::string &extra, int32_t userId)
{
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().FilterNeedDeleteACL(
        localUdid, localTokenId, remoteUdid, extra);

    //first, clear the unbind sk/cred/acl
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);

    //second, not app/service/user/DM_IDENTICAL_ACCOUNT acl exist, determin if need clear lnn acl
    if (offlineParam.allLeftAppOrSvrAclInfos.empty() && offlineParam.allUserAclInfos.empty()) {
        LOGI("No acl exist, clear lnn acl");
        DeleteSkCredAndAcl(offlineParam.allLnnAclInfos);
    }

    NotifyDeviceOrAppOffline(offlineParam, remoteUdid);
    return DM_OK;
}

bool DeviceManagerServiceImpl::IsProxyUnbind(const std::string &extra)
{
    if (extra.empty()) {
        return false;
    }
    JsonObject jsonObject;
    jsonObject.Parse(extra);
    if (jsonObject.IsDiscarded()) {
        return false;
    }
    if (IsString(jsonObject, PARAM_KEY_IS_PROXY_UNBIND) &&
        jsonObject[PARAM_KEY_IS_PROXY_UNBIND].Get<std::string>() == DM_VAL_TRUE) {
        return true;
    }
    return false;
}

int32_t DeviceManagerServiceImpl::DeleteAclV2(const std::string &pkgName, const std::string &localUdid,
    const std::string &remoteUdid, int32_t bindLevel, const std::string &extra)
{
    LOGI("pkgName %{public}s, localUdid %{public}s, remoteUdid %{public}s, bindLevel %{public}d.",
        pkgName.c_str(), GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), bindLevel);
    int64_t tokenId = 0;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (IsProxyUnbind(extra) != true) {
        uint32_t callingTokenId = 0;
        MultipleUserConnector::GetTokenId(callingTokenId);
        tokenId = static_cast<int64_t>(callingTokenId);
    } else {
        std::string bundleName = pkgName;
        AppManager::GetInstance().GetTokenIdByBundleName(userId, bundleName, tokenId);
    }
    bool isNewVersion = IsAuthNewVersion(bindLevel, localUdid, remoteUdid, tokenId, userId);
    if (!isNewVersion) {
        return DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    }
    return DeleteAclForProcV2(localUdid, tokenId, remoteUdid, bindLevel, extra, userId);
}

void DeviceManagerServiceImpl::GetBindCallerInfo(DmBindCallerInfo &bindCallerInfo, const std::string &pkgName)
{
    int32_t userId = -1;
    MultipleUserConnector::GetCallerUserId(userId);
    uint32_t callingTokenId = 0;
    MultipleUserConnector::GetCallingTokenId(callingTokenId);
    bool isSystemSA = false;
    std::string bundleName = "";
    AppManager::GetInstance().GetCallerName(isSystemSA, bundleName);
    std::string processName = "";
    AppManager::GetInstance().GetCallerProcessName(processName);
    int32_t bindLevel = static_cast<int32_t>(DmRole::DM_ROLE_FA);
    if (AppManager::GetInstance().IsSystemApp()) {
        bindLevel = static_cast<int32_t>(DmRole::DM_ROLE_FA);
    }
    if (AppManager::GetInstance().IsSystemSA()) {
        isSystemSA = true;
        bindLevel = static_cast<int32_t>(DmRole::DM_ROLE_SA);
    }
    if (AuthManagerBase::CheckProcessNameInWhiteList(bundleName)) {
        bindLevel = static_cast<int32_t>(DmRole::DM_ROLE_USER);
    }
    std::string hostPkgLabel = GetBundleLable(pkgName);
    bindCallerInfo.userId = userId;
    bindCallerInfo.tokenId = static_cast<int32_t>(callingTokenId);
    bindCallerInfo.bindLevel = bindLevel;
    bindCallerInfo.bundleName = bundleName;
    bindCallerInfo.hostPkgLabel  = hostPkgLabel;
    bindCallerInfo.processName = processName;
    bindCallerInfo.isSystemSA = isSystemSA;
    LOGI("userId %{public}d, tokenId %{public}s, bindLevel %{public}d, bundleName %{public}s, hostPkgLabel  %{public}s,"
        "processName %{public}s, isSystemSA %{public}d", userId, GetAnonyInt32(callingTokenId).c_str(), bindLevel,
        GetAnonyString(bundleName).c_str(), GetAnonyString(hostPkgLabel).c_str(), GetAnonyString(processName).c_str(),
        isSystemSA);
}

std::string DeviceManagerServiceImpl::GetBundleLable(const std::string &bundleName)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get ability manager failed");
        return bundleName;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (object == nullptr) {
        LOGE("object is NULL.");
        return bundleName;
    }

    sptr<OHOS::AppExecFwk::IBundleMgr> bms = iface_cast<OHOS::AppExecFwk::IBundleMgr>(object);
    if (bms == nullptr) {
        LOGE("bundle manager service is NULL.");
        return bundleName;
    }

    auto bundleResourceProxy = bms->GetBundleResourceProxy();
    if (bundleResourceProxy == nullptr) {
        LOGE("GetBundleResourceProxy fail");
        return bundleName;
    }
    AppExecFwk::BundleResourceInfo resourceInfo;
    auto result = bundleResourceProxy->GetBundleResourceInfo(bundleName,
        static_cast<uint32_t>(OHOS::AppExecFwk::ResourceFlag::GET_RESOURCE_INFO_ALL), resourceInfo);
    if (result != ERR_OK) {
        LOGE("GetBundleResourceInfo failed");
        return bundleName;
    }
    LOGI("bundle resource label is %{public}s ", (resourceInfo.label).c_str());
    return resourceInfo.label;
}

void DeviceManagerServiceImpl::SetBindCallerInfoToBindParam(const std::map<std::string, std::string> &bindParam,
    std::map<std::string, std::string> &bindParamTmp, const DmBindCallerInfo &bindCallerInfo)
{
    bindParamTmp = const_cast<std::map<std::string, std::string> &>(bindParam);
    bindParamTmp["bindCallerUserId"] = std::to_string(bindCallerInfo.userId);
    bindParamTmp["bindCallerTokenId"] = std::to_string(bindCallerInfo.tokenId);
    bindParamTmp["bindCallerBindLevel"] = std::to_string(bindCallerInfo.bindLevel);
    bindParamTmp["bindCallerBundleName"] = bindCallerInfo.bundleName;
    bindParamTmp["bindCallerHostPkgLabel"] = bindCallerInfo.hostPkgLabel;
    bindParamTmp["bindCallerProcessName"] = bindCallerInfo.processName;
    bindParamTmp["bindCallerIsSystemSA"] = std::to_string(bindCallerInfo.isSystemSA);
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

bool DeviceManagerServiceImpl::CheckSrcAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    return DeviceProfileConnector::GetInstance().CheckSrcAccessControl(caller, srcUdid, callee, sinkUdid);
}

bool DeviceManagerServiceImpl::CheckSinkAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    return DeviceProfileConnector::GetInstance().CheckSinkAccessControl(caller, srcUdid, callee, sinkUdid);
}

bool DeviceManagerServiceImpl::CheckSrcIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    return DeviceProfileConnector::GetInstance().CheckSrcIsSameAccount(caller, srcUdid, callee, sinkUdid);
}

bool DeviceManagerServiceImpl::CheckSinkIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    return DeviceProfileConnector::GetInstance().CheckSinkIsSameAccount(caller, srcUdid, callee, sinkUdid);
}

void DeviceManagerServiceImpl::DeleteHoDevice(const std::string &peerUdid,
    const std::vector<int32_t> &foreGroundUserIds, const std::vector<int32_t> &backGroundUserIds)
{
    LOGI("peerudid %{public}s, foreGroundUserIds %{public}s, backGroundUserIds %{public}s.",
        GetAnonyString(peerUdid).c_str(), GetIntegerList(foreGroundUserIds).c_str(),
        GetIntegerList(backGroundUserIds).c_str());
    //delete acl
    DeviceProfileConnector::GetInstance().DeleteHoDevice(peerUdid, foreGroundUserIds, backGroundUserIds);
    //delete group
    hiChainConnector_->DeleteHoDevice(peerUdid, foreGroundUserIds, backGroundUserIds);
    return;
}

void DeviceManagerServiceImpl::DeleteSessionKey(int32_t userId,
    const DistributedDeviceProfile::AccessControlProfile &profile)
{
    int32_t skId = profile.GetAccesser().GetAccesserSessionKeyId();
    DeviceProfileConnector::GetInstance().DeleteSessionKey(userId, skId);
    skId = profile.GetAccessee().GetAccesseeSessionKeyId();
    DeviceProfileConnector::GetInstance().DeleteSessionKey(userId, skId);
}

int32_t DeviceManagerServiceImpl::BindServiceTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam)
{
    if (pkgName.empty()) {
        LOGE("BindServiceTarget failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> bindParamTmp = bindParam;
    bindParamTmp[PARAM_KEY_IS_SERVICE_BIND] = "true";
    return BindTarget(pkgName, targetId, bindParamTmp);
}

int32_t DeviceManagerServiceImpl::UnbindServiceTarget(const std::string &pkgName, int64_t serviceId)
{
    if (pkgName.empty()) {
        LOGE("failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int64_t tokenIdCaller = IPCSkeleton::GetCallingTokenID();
    std::string peerDeviceId = "";
    int32_t bindLevel = -1;
    LOGI("start pkgName:%{public}s, serviceId:%{public}" PRId64 ", tokenId:%{public}" PRId64,
        pkgName.c_str(), serviceId, tokenIdCaller);
    int32_t ret = DeleteAclExtraDataServiceId(serviceId, tokenIdCaller, peerDeviceId, bindLevel);
    if (ret != DM_OK) {
        LOGE("failed, DeleteAclExtraDataServiceId failed.");
        return ret;
    }

    ServiceInfoProfile serviceInfoProfile;
    int32_t userId = -1;
    MultipleUserConnector::GetCallerUserId(userId);
    ret = DeviceProfileConnector::GetInstance().GetServiceInfoProfileByServiceId(serviceId, serviceInfoProfile);
    if (ret != DM_OK) {
        LOGE("failed, GetServiceInfoProfileByServiceId failed.");
        return ret;
    }
    ret = DeviceProfileConnector::GetInstance().DeleteServiceInfoProfile(serviceInfoProfile.regServiceId, userId);
    if (ret != DM_OK) {
        LOGE("failed, DeleteServiceInfoProfile failed.");
        return ret;
    }
    ret = UnBindDevice(pkgName, peerDeviceId, bindLevel);
    if (ret != DM_OK) {
        LOGE("failed, UnBindDevice failed.");
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::DeleteAclExtraDataServiceId(int64_t serviceId, int64_t tokenIdCaller,
    std::string &udid, int32_t &bindLevel)
{
    bool isDeletedExtra = false;
    char localDeviceIdTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceIdTemp, DEVICE_UUID_LENGTH);
    std::string localDeviceId = std::string(localDeviceIdTemp);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        std::string extraData = item.GetExtraData();
        if (extraData.empty()) {
            LOGW("extraData empty.");
            continue;
        }
        JsonObject json;
        json.Parse(extraData);
        if (json.IsDiscarded()) {
            LOGW("extraData invalid.");
            continue;
        }
        if (IsInt64(json, TAG_SERVICE_ID)) {
            int64_t aclServiceId = json[TAG_SERVICE_ID].Get<int64_t>();
            if (aclServiceId != serviceId) {
                LOGW("aclServiceId(%{public}" PRId64 ") != serviceId(%{public}" PRId64 ").", aclServiceId, serviceId);
                continue;
            }
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        int64_t tokenIdAcl = item.GetAccesser().GetAccesserTokenId();
        if (accesserUdid == localDeviceId && tokenIdCaller == tokenIdAcl) {
            isDeletedExtra = true;
            json.Erase(TAG_SERVICE_ID);
            item.SetExtraData(json.Dump());
            udid = item.GetAccessee().GetAccesseeDeviceId();
            bindLevel = item.GetBindLevel();
            DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
        }
    }
    if (!isDeletedExtra) {
        LOGE("DeleteAclExtraDataServiceId failed, local is sink, not allow unbind.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}


void DeviceManagerServiceImpl::InitTaskOfDelTimeOutAcl(const std::string &peerUdid, int32_t peerUserId,
    int32_t localUserId)
{
    CHECK_NULL_VOID(deviceStateMgr_);
    deviceStateMgr_->StartDelTimerByDP(peerUdid, peerUserId, localUserId);
}

void DeviceManagerServiceImpl::NotifyDeviceOrAppOffline(DmOfflineParam &offlineParam, const std::string &remoteUdid)
{
    if (!offlineParam.allUserAclInfos.empty()) {
        LOGI("left user acl, not notify");
        return;
    }
    if (offlineParam.hasUserAcl && offlineParam.allUserAclInfos.empty() &&
        !offlineParam.allLeftAppOrSvrAclInfos.empty()) {
        LOGI("left app or service acl.");
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        offlineParam.processVec.push_back(processInfo);
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        std::set<std::string> pkgNameSet;
        for (auto &item : offlineParam.allLeftAppOrSvrAclInfos) {
            pkgNameSet.insert(item.pkgName);
        }
        CHECK_NULL_VOID(listener_);
        listener_->SetExistPkgName(pkgNameSet);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
        softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
        return;
    }
    if (!offlineParam.hasUserAcl && offlineParam.allUserAclInfos.empty() &&
        !offlineParam.allLeftAppOrSvrAclInfos.empty()) {
        LOGI("left service or app acl");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
        softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
        return;
    }
    NotifyDeviceOffline(offlineParam, remoteUdid);
}

int32_t DeviceManagerServiceImpl::LeaveLNN(const std::string &pkgName, const std::string &networkId)
{
    CHECK_NULL_RETURN(softbusConnector_, ERR_DM_POINT_NULL);
    return softbusConnector_->LeaveLNN(pkgName, networkId);
}

void DeviceManagerServiceImpl::NotifyDeviceOffline(DmOfflineParam &offlineParam, const std::string &remoteUdid)
{
    if (offlineParam.hasUserAcl && offlineParam.allUserAclInfos.empty() &&
        offlineParam.allLeftAppOrSvrAclInfos.empty()) {
        LOGI("not left acl");
        ProcessInfo processInfo;
        processInfo.pkgName = std::string(DM_PKG_NAME);
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        std::vector<ProcessInfo> processVec = { processInfo };
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(processVec);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
        softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
    }
    if (!offlineParam.hasUserAcl && offlineParam.allUserAclInfos.empty() &&
        offlineParam.allLeftAppOrSvrAclInfos.empty()) {
        LOGI("not left acl");
        CHECK_NULL_VOID(softbusConnector_);
        softbusConnector_->SetProcessInfoVec(offlineParam.processVec);
        bool isOnline = SoftbusCache::GetInstance().CheckIsOnlineByPeerUdid(remoteUdid);
        softbusConnector_->HandleDeviceOffline(remoteUdid, isOnline);
    }
}

void DeviceManagerServiceImpl::GetNotifyEventInfos(std::vector<DmDeviceInfo> &deviceList)
{
    CHECK_NULL_VOID(deviceStateMgr_);
    deviceStateMgr_->GetNotifyEventInfos(deviceList);
}

void DeviceManagerServiceImpl::DeleteGroupByBundleName(const std::string &localUdid, int32_t userId,
    const std::vector<DmAclIdParam> &acls)
{
    if (acls.empty()) {
        LOGI("acls is empty.");
        return;
    }
    CHECK_NULL_VOID(hiChainConnector_);
    std::vector<GroupInfo> groupList;
    hiChainConnector_->GetRelatedGroups(userId, localUdid, groupList);
    for (auto &iter : groupList) {
        for (auto &item : acls) {
            if (!item.pkgName.empty() && iter.groupName.find(item.pkgName) != std::string::npos) {
                int32_t ret = hiChainConnector_->DeleteGroup(iter.groupId);
                LOGI("delete bundleName %{public}s, groupId %{public}s ,result %{public}d.",
                    item.pkgName.c_str(), GetAnonyString(iter.groupId).c_str(), ret);
            }
        }
    }
    std::vector<GroupInfo> groupListExt;
    hiChainConnector_->GetRelatedGroupsExt(userId, localUdid, groupListExt);
    for (auto &iter : groupListExt) {
        for (auto &item : acls) {
            if (!item.pkgName.empty() && iter.groupName.find(item.pkgName) != std::string::npos) {
                int32_t ret = hiChainConnector_->DeleteGroupExt(iter.groupId);
                LOGI("delete groupsExt bundleName %{public}s, groupId %{public}s ,result %{public}d.",
                    item.pkgName.c_str(), GetAnonyString(iter.groupId).c_str(), ret);
            }
        }
    }
}

int32_t DeviceManagerServiceImpl::UpdateLocalServiceInfoToDp(const DmAuthInfo &dmAuthInfo,
    const DistributedDeviceProfile::LocalServiceInfo &dpServiceInfo)
{
    int32_t result = DeviceProfileConnector::GetInstance().UpdateLocalServiceInfo(dpServiceInfo);
    if (result != DM_OK) {
        LOGE("UpdateLocalServiceInfo failed %{public}d .", result);
        return result;
    }
    ImportAuthCode(dmAuthInfo.pinConsumerPkgName, dmAuthInfo.pinCode);
    return DM_OK;
}

int32_t DeviceManagerServiceImpl::ImportAuthInfo(const DmAuthInfo &dmAuthInfo)
{
    int32_t errorCount = 0;
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    bool pinMatchFlag = GetPinMatchFlag(tokenId, dmAuthInfo);
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfoOld;
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfoNew;
    InitDpServiceInfo(dmAuthInfo, dpLocalServiceInfoNew, pinMatchFlag, tokenId, errorCount);
    int32_t ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        dmAuthInfo.pinConsumerPkgName, static_cast<int32_t>(dmAuthInfo.pinExchangeType), dpLocalServiceInfoOld);
    if (ret != DM_OK) {
        LOGE("GetLocalServiceInfoByBundleNameAndPinExchangeType failed");
        int32_t result = DeviceProfileConnector::GetInstance().PutLocalServiceInfo(dpLocalServiceInfoNew);
        if (result != DM_OK) {
            LOGE("PutLocalServiceInfo failed %{public}d .", result);
            return result;
        }
        ImportAuthCode(dmAuthInfo.pinConsumerPkgName, dmAuthInfo.pinCode);
        return DM_OK;
    }
    if (dmAuthInfo.pinExchangeType != DMLocalServiceInfoPinExchangeType::FROMDP &&
        dmAuthInfo.pinExchangeType != DMLocalServiceInfoPinExchangeType::IMPORT_AUTH_CODE) {
        return UpdateLocalServiceInfoToDp(dmAuthInfo, dpLocalServiceInfoNew);
    }
    bool pinChanged = (dpLocalServiceInfoOld.GetPinCode() != std::string(dmAuthInfo.pinCode));
    if (pinChanged) {
        return UpdateLocalServiceInfoToDp(dmAuthInfo, dpLocalServiceInfoNew);
    }
    std::string oldExtra = dpLocalServiceInfoOld.GetExtraInfo();
    if (oldExtra.empty()) {
        return UpdateLocalServiceInfoToDp(dmAuthInfo, dpLocalServiceInfoNew);
    }
    JsonObject mergedExtra(oldExtra);
    if (mergedExtra.IsDiscarded()) {
        return UpdateLocalServiceInfoToDp(dmAuthInfo, dpLocalServiceInfoNew);
    }
    if (IsInt32(mergedExtra, PIN_ERROR_COUNT)) {
        errorCount = mergedExtra[PIN_ERROR_COUNT].Get<int32_t>();
    }
    InitDpServiceInfo(dmAuthInfo, dpLocalServiceInfoNew, pinMatchFlag, tokenId, errorCount);
    return UpdateLocalServiceInfoToDp(dmAuthInfo, dpLocalServiceInfoNew);
}

bool DeviceManagerServiceImpl::GetPinMatchFlag(uint64_t tokenId, const DmAuthInfo &dmAuthInfo)
{
    std::string key = std::to_string(tokenId) + "_" + dmAuthInfo.pinConsumerPkgName
        + "_" + std::to_string(static_cast<int32_t>(dmAuthInfo.pinExchangeType));
    {
        std::lock_guard<std::mutex> lock(tokenIdPinCodeMapLock_);
        auto it = tokenIdPinCodeMap_.find(key);
        if (it == tokenIdPinCodeMap_.end()) {
            return false;
        }
        if (!std::string(dmAuthInfo.metaToken).empty()) {
            auto pinLen = std::string(dmAuthInfo.pinCode).length();
            if (pinLen != METATOKEN_PINCODE_LENGTH) {
                return false;
            }
            return std::string(it->second.pinCode) == std::string(dmAuthInfo.pinCode).substr(METATOKEN_LENGTH);
        }
        return strcmp(it->second.pinCode, dmAuthInfo.pinCode) == 0;
    }
}

void DeviceManagerServiceImpl::InitDpServiceInfo(const DmAuthInfo &dmAuthInfo,
    DistributedDeviceProfile::LocalServiceInfo &dpServiceInfo, bool pinMatchFlag, uint64_t tokenId, int32_t errorCount)
{
    dpServiceInfo.SetBundleName(dmAuthInfo.pinConsumerPkgName);
    dpServiceInfo.SetAuthBoxType(static_cast<int32_t>(dmAuthInfo.authBoxType));
    dpServiceInfo.SetAuthType(static_cast<int32_t>(dmAuthInfo.authType));
    dpServiceInfo.SetPinExchangeType(static_cast<int32_t>(dmAuthInfo.pinExchangeType));
    dpServiceInfo.SetPinCode(dmAuthInfo.pinCode);
    dpServiceInfo.SetDescription(dmAuthInfo.description);
    JsonObject extraInfoObj;
    if (!dmAuthInfo.extraInfo.empty()) {
        extraInfoObj.Parse(dmAuthInfo.extraInfo);
        if (extraInfoObj.IsDiscarded()) {
            LOGI("extraInfo is empty or invalid, create new extraInfo");
            return;
        }
    }
    extraInfoObj[TAG_PIN_USER_ID] = dmAuthInfo.userId;
    extraInfoObj[PIN_CONSUMER_TOKENID] = dmAuthInfo.pinConsumerTokenId;
    extraInfoObj[BIZ_SRC_PKGNAME] = dmAuthInfo.bizSrcPkgName;
    extraInfoObj[BIZ_SINK_PKGNAME] = dmAuthInfo.bizSinkPkgName;
    extraInfoObj[META_TOKEN] = dmAuthInfo.metaToken;
    extraInfoObj[PIN_MATCH_FLAG] = pinMatchFlag;
    extraInfoObj[PIN_ERROR_COUNT] = errorCount;
    extraInfoObj[TAG_TOKENID] = tokenId;
    dpServiceInfo.SetExtraInfo(extraInfoObj.Dump());
}

int32_t DeviceManagerServiceImpl::ExportAuthInfo(DmAuthInfo &dmAuthInfo, uint32_t pinLength)
{
    uint64_t tokenId = IPCSkeleton::GetCallingTokenID();
    std::string pinCode = GeneratePinCode(pinLength);
    int32_t length = static_cast<int32_t>(pinCode.length());
    for (int32_t i = 0; i < length; i++) {
        if (!isdigit(pinCode[i])) {
            LOGE("ImportAuthCode error: Invalid para, authCode format error.");
            return ERR_DM_INPUT_PARA_INVALID;
        }
    }
    if (strncpy_s(dmAuthInfo.pinCode, DM_MAX_PIN_CODE_LEN, pinCode.c_str(), pinCode.length()) != 0) {
        return ERR_DM_FAILED;
    }
    {
        std::lock_guard<std::mutex> lock(tokenIdPinCodeMapLock_);
        std::string key = std::to_string(tokenId) + "_" + dmAuthInfo.pinConsumerPkgName + "_" +
            std::to_string(static_cast<int32_t>(dmAuthInfo.pinExchangeType));
        tokenIdPinCodeMap_[key] = dmAuthInfo;
    }
    StartAuthInfoTimer(dmAuthInfo, tokenId);
    return DM_OK;
}

void DeviceManagerServiceImpl::StartAuthInfoTimer(const DmAuthInfo &dmAuthInfo, uint64_t tokenId)
{
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    std::string taskName = std::string(BIND_TARGET_PIN_TIMEOUT_TASK) + dmAuthInfo.pinConsumerPkgName +
        std::to_string(static_cast<int32_t>(dmAuthInfo.pinExchangeType));
    timer_->DeleteTimer(taskName);
    timer_->StartTimer(taskName, BIND_TARGET_PIN_TIMEOUT, [this, dmAuthInfo](std::string name) {
        DeviceManagerServiceImpl::StopAuthInfoTimerAndDeleteDP(dmAuthInfo.pinConsumerPkgName,
            static_cast<int32_t>(dmAuthInfo.pinExchangeType), dmAuthInfo.pinConsumerTokenId);
            listener_->OnAuthCodeInvalid(dmAuthInfo.pinConsumerPkgName);
    });
}
void DeviceManagerServiceImpl::StopAuthInfoTimerAndDeleteDP(const std::string &pkgName, int32_t pinExchangeType,
    uint64_t tokenId)
{
    if (timer_ != nullptr) {
        std::string taskName = std::string(BIND_TARGET_PIN_TIMEOUT_TASK) + pkgName + std::to_string(pinExchangeType);
        timer_->DeleteTimer(taskName);
    }
    DeviceProfileConnector::GetInstance().DeleteLocalServiceInfo(pkgName, pinExchangeType);
    {
        std::lock_guard<std::mutex> lock(tokenIdPinCodeMapLock_);
        std::string key = std::to_string(tokenId) + "_" + pkgName + "_" + std::to_string(pinExchangeType);
        tokenIdPinCodeMap_.erase(key);
    }
}

//this code line mock start
void DeviceManagerServiceImpl::HandleIdentAccountLogout(const DMAclQuadInfo &info, const std::string &accountId,
    std::vector<DmUserRemovedServiceInfo> &serviceInfos)
{
    return;
}

void DeviceManagerServiceImpl::HandleRemoteUserRemoved(int32_t userId, const std::string &remoteUdid,
    std::vector<DmUserRemovedServiceInfo> &serviceInfos)
{
    return;
}

void DeviceManagerServiceImpl::HandleCommonEventBroadCast(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid,
    std::vector<DmUserRemovedServiceInfo> &serviceInfos)
{
    return;
}

void DeviceManagerServiceImpl::HandleUserRemoved(std::vector<std::string> peerUdids, int32_t preUserId,
    std::vector<DmUserRemovedServiceInfo> &serviceInfos)
{
    return;
}

void DeviceManagerServiceImpl::HandleSyncUserIdEvent(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid, bool isCheckUserStatus,
    std::vector<DmUserRemovedServiceInfo> &serviceInfos)
{
    return;
}

void DeviceManagerServiceImpl::HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
    const std::string &remoteUdid, std::vector<DmUserRemovedServiceInfo> &serviceInfos)
{
    return;
}
//this code line mock end

extern "C" IDeviceManagerServiceImpl *CreateDMServiceObject(void)
{
    return new DeviceManagerServiceImpl;
}
} // namespace DistributedHardware
} // namespace OHOS
