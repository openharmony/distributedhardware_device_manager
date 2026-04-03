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

#include "device_manager_service_impl_3rd.h"

#include <chrono>
#include <random>
#include <cstring>
#include <thread>
#include <securec.h>

#include "ipc_skeleton.h"

#include "app_manager_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_message_processor_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"
#include "kv_adapter_manager_3rd.h"
#include "multiple_user_connector_3rd.h"
#include "permission_manager_3rd.h"
#include "softbus_session_3rd.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
const int32_t DM_MIN_PINCODE_SIZE = 6;
const int32_t DM_MAX_PINCODE_SIZE = 1024;
const int32_t MAX_DATA_LEN = 4096000;
constexpr int32_t MIN_PIN_CODE = 100000;
constexpr int32_t MAX_PIN_CODE = 999999;
constexpr size_t MAX_NEW_PROC_SESSION_COUNT_TEMP = 1;
const int32_t OPEN_AUTH_SESSION_TIMEOUT = 15000; // 15000ms
}

DeviceManagerServiceImpl3rd::DeviceManagerServiceImpl3rd()
{
    LOGI("DeviceManagerServiceImpl3rd constructor");
    KVAdapterManager3rd::GetInstance().Init();
}

DeviceManagerServiceImpl3rd::~DeviceManagerServiceImpl3rd()
{
    LOGI("DeviceManagerServiceImpl3rd destructor");
    KVAdapterManager3rd::GetInstance().UnInit();
}

int DeviceManagerServiceImpl3rd::OnAuth3rdAclSessionOpened(int sessionId, int result)
{
    LOGI("OnSessionOpened success, sessionId: %{public}d, res: %{public}d.", sessionId, result);
    if (sessionEnableCvMap_.find(sessionId) != sessionEnableCvMap_.end()) {
        std::lock_guard<ffrt::mutex> lock(sessionEnableMutexMap_[sessionId]);
        if (result == 0) {
            sessionEnableCvReadyMap_[sessionId] = true;
        }
        sessionEnableCvMap_[sessionId].notify_all();
    }
    return DM_OK;
}

void DeviceManagerServiceImpl3rd::OnAuth3rdAclSessionClosed(int sessionId)
{
    LOGI("OnSessionClosed, success, sessionId: %{public}d.", sessionId);
    return;
}

static JsonObject GetJsonObjectFromData(const void *data, unsigned int dataLen)
{
    std::string message = std::string(reinterpret_cast<const char *>(data), dataLen);
    return JsonObject(message);
}

void DeviceManagerServiceImpl3rd::OnAuth3rdAclBytesReceived(int sessionId, const void *data, uint32_t dataLen)
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
    std::string message = std::string(reinterpret_cast<const char *>(data), dataLen);
    std::shared_ptr<AuthManagerBase3rd> authMgr = nullptr;
    uint64_t logicalSessionId = 0;
    if (IsUint64(jsonObject, DM_TAG_LOGICAL_SESSION_ID)) {
        logicalSessionId = jsonObject[DM_TAG_LOGICAL_SESSION_ID].Get<std::uint64_t>();
    }
    authMgr = GetAuthMgrByMessage(msgType, logicalSessionId, jsonObject);
    if (authMgr == nullptr) {
        return;
    }
    authMgr->OnDataReceived(sessionId, message);
}

int DeviceManagerServiceImpl3rd::OnAuth3rdSessionOpened(int sessionId, int result)
{
    LOGI("OnSessionOpened, success, sessionId: %{public}d.", sessionId);
    return DM_OK;
}

void DeviceManagerServiceImpl3rd::OnAuth3rdSessionClosed(int sessionId)
{
    LOGI("OnSessionClosed, success, sessionId: %{public}d.", sessionId);
    return;
}

void DeviceManagerServiceImpl3rd::OnAuth3rdBytesReceived(int sessionId, const void *data, uint32_t dataLen)
{
    if (sessionId < 0 || data == nullptr || dataLen <= 0 || dataLen > MAX_DATA_LEN) {
        LOGE("[SOFTBUS]fail to receive data from softbus with sessionId: %{public}d, dataLen: %{public}d.", sessionId,
            dataLen);
        return;
    }
    LOGI("start, sessionId: %{public}d, dataLen: %{public}d.", sessionId, dataLen);
    std::string message = std::string(reinterpret_cast<const char *>(data), dataLen);
    LOGI("OnBytesReceived, success, sessionId: %{public}d.", sessionId);
    return;
}

int32_t DeviceManagerServiceImpl3rd::Initialize(const std::shared_ptr<IDeviceManagerServiceListener3rd> &listener)
{
    listener_ = listener;
    if (softbusConnector_ == nullptr) {
        softbusConnector_ = std::make_shared<SoftbusConnector3rd>();
    }
    if (hiChainAuthConnector_ == nullptr) {
        hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector3rd>();
    }
    return DM_OK;
}

void DeviceManagerServiceImpl3rd::Release()
{
    listener_ = nullptr;
    softbusConnector_ = nullptr;
    hiChainAuthConnector_ = nullptr;
    sessionEnableCvMap_.clear();
    sessionEnableMutexMap_.clear();
    return;
}

int32_t DeviceManagerServiceImpl3rd::ImportPinCode3rd(const std::string &businessName, const std::string &pinCode)
{
    if (businessName.empty() || pinCode.empty()) {
        LOGE("Invalid parameter, businessName or pinCode is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, businessName: %{public}s", businessName.c_str());
    if (pinCode.length() < DM_MIN_PINCODE_SIZE || pinCode.length() > DM_MAX_PINCODE_SIZE) {
        LOGE("Invalid pinCode length: %{public}zu", pinCode.length());
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::string processName = "";
    if (PermissionManager3rd::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed.");
        return ERR_DM_FAILED;
    }

    ProcessInfo3rd processInfo3rd;
    processInfo3rd.tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    processInfo3rd.uid = OHOS::IPCSkeleton::GetCallingUid();
    processInfo3rd.processName = processName;
    processInfo3rd.businessName = businessName;
    int32_t userId = MultipleUserConnector3rd::GetCurrentAccountUserID();
    processInfo3rd.userId = userId;

    PinCodeInfo pinCodeInfo;
    if (strcpy_s(pinCodeInfo.pinCode, pinCode.size() + 1, pinCode.c_str()) != DM_OK) {
        LOGE("strcpy_s pin code failed!");
        return ERR_DM_FAILED;
    }
    pinCodeInfo.pincodeValidFlag = true;
    {
        std::lock_guard<ffrt::mutex> lock(pinCodeLock_);
        pinCodeMap_[processInfo3rd] = pinCodeInfo;
    }
    LOGI("completed");
    return DM_OK;
}

bool DeviceManagerServiceImpl3rd::IsInvalidPeerTargetId(const PeerTargetId3rd &targetId)
{
    LOGI("deviceId: %{public}s", GetAnonyString(targetId.deviceId).c_str());
    return targetId.deviceId.empty() && targetId.brMac.empty() && targetId.bleMac.empty() && targetId.wifiIp.empty();
}

void DeviceManagerServiceImpl3rd::GetBindCallerInfo(DmAuthCallerInfo3rd &authCallerInfo3rd, ProcessInfo3rd &processInfo)
{
    uint32_t uid = OHOS::IPCSkeleton::GetCallingUid();
    int32_t userId = MultipleUserConnector3rd::GetCurrentAccountUserID();
    uint32_t callingTokenId = 0;
    MultipleUserConnector3rd::GetCallingTokenId(callingTokenId);
    std::string processName = "";
    AppManager3rd::GetInstance().GetCallerProcessName(processName);

    if (AppManager3rd::GetInstance().IsSystemSA()) {
        authCallerInfo3rd.isSystemSA = true;
        authCallerInfo3rd.bindLevel = static_cast<int32_t>(DmRole::DM_ROLE_SA);
    } else {
        authCallerInfo3rd.bindLevel = static_cast<int32_t>(DmRole::DM_ROLE_FA);
        authCallerInfo3rd.isSystemSA = false;
    }
    authCallerInfo3rd.uid = uid;
    authCallerInfo3rd.userId = userId;
    authCallerInfo3rd.tokenId = callingTokenId;
    authCallerInfo3rd.processName = processName;

    processInfo.processName = processName;
    processInfo.tokenId = callingTokenId;
    processInfo.uid = uid;
    processInfo.userId = userId;
    LOGI("uid:%{public}d, userId %{public}d, tokenId %{public}s, processName %{public}s,", uid, userId,
        GetAnonyInt32(callingTokenId).c_str(),  GetAnonyString(processName).c_str());
}

void DeviceManagerServiceImpl3rd::SetBindCallerInfoToAuthParam(const std::map<std::string, std::string> &authParam,
    std::map<std::string, std::string> &authParamTmp, const DmAuthCallerInfo3rd &authCallerInfo3rd,
    ProcessInfo3rd &processInfo)
{
    if (authParam.count(TAG_BUSINESS_NAME) != 0) {
        processInfo.businessName = authParam.at(TAG_BUSINESS_NAME);
    }
    authParamTmp = const_cast<std::map<std::string, std::string> &>(authParam);
    authParamTmp["bindCallerUserId"] = std::to_string(authCallerInfo3rd.userId);
    authParamTmp[TAG_BIND_CALLER_TOKENID] = std::to_string(authCallerInfo3rd.tokenId);
    authParamTmp["bindCallerUid"] = std::to_string(authCallerInfo3rd.uid);
    authParamTmp[TAG_BIND_CALLER_PROCESSNAME] = authCallerInfo3rd.processName;
    authParamTmp[TAG_BIND_CALLER_IS_SYSTEM_SA] = std::to_string(authCallerInfo3rd.isSystemSA);
    authParamTmp[TAG_BIND_CALLER_BIND_LEVEL] = std::to_string(authCallerInfo3rd.bindLevel);
}

int32_t DeviceManagerServiceImpl3rd::AuthPincode(const PeerTargetId3rd &targetId,
    std::map<std::string, std::string> &authParam)
{
    if (IsInvalidPeerTargetId(targetId)) {
        LOGE("Invalid parameter, params are empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, deviceId: %{public}s", GetAnonyString(targetId.deviceId).c_str());
    ProcessInfo3rd processInfo3rd;
    DmAuthCallerInfo3rd authCallerInfo3rd;
    GetBindCallerInfo(authCallerInfo3rd, processInfo3rd);
    std::map<std::string, std::string> authParamTmp;
    SetBindCallerInfoToAuthParam(authParam, authParamTmp, authCallerInfo3rd, processInfo3rd);
    LOGI("In, processName:%{public}s, tokenId:%{public}u", authCallerInfo3rd.processName.c_str(),
        authCallerInfo3rd.tokenId);
    {
        std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
        if (tokenIdSessionIdMap_.find(processInfo3rd.tokenId) != tokenIdSessionIdMap_.end()) {
            LOGE("AuthPincode failed, this device is being auth. please try again later,"
                "processName:%{public}s, tokenId:%{public}u, sessionId: %{public}d",
                authCallerInfo3rd.processName.c_str(), authCallerInfo3rd.tokenId,
                tokenIdSessionIdMap_[processInfo3rd.tokenId]);
            return ERR_DM_AUTH_BUSINESS_BUSY;
        }
    }
    PinCodeInfo pinCodeInfo;
    {
        std::lock_guard<ffrt::mutex> lock(pinCodeLock_);
        if (pinCodeMap_.find(processInfo3rd) == pinCodeMap_.end()) {
            LOGE("pinCodeInfo is not exist");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        pinCodeInfo = pinCodeMap_[processInfo3rd];
    }
    std::thread newThread(&DeviceManagerServiceImpl3rd::AuthPincodeImpl, this, targetId, pinCodeInfo,
        authParamTmp, processInfo3rd);
    newThread.detach();

    LOGI("AuthPincode completed");
    return DM_OK;
}

void DeviceManagerServiceImpl3rd::AuthPincodeImpl(const PeerTargetId3rd &targetId, const PinCodeInfo pinCodeInfo,
    const std::map<std::string, std::string> &authParamTmp, const ProcessInfo3rd processInfo3rd)
{
    LOGE("processName:%{public}s, tokenId:%{public}u, businessName: %{public}s",
        processInfo3rd.processName.c_str(), processInfo3rd.tokenId, processInfo3rd.businessName.c_str());
    
    targetId3rd_ = targetId;
    authParam_ = authParamTmp;
    return;
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

void DeviceManagerServiceImpl3rd::AuthDeviceAclImpl(const PeerTargetId3rd &targetId, const PinCodeInfo pinCodeInfo,
    const std::map<std::string, std::string> &authParamTmp, const ProcessInfo3rd processInfo3rd)
{
    LOGE("processName:%{public}s, tokenId:%{public}u, businessName: %{public}s",
        processInfo3rd.processName.c_str(), processInfo3rd.tokenId, processInfo3rd.businessName.c_str());
    
    targetId3rd_ = targetId;
    authParam_ = authParamTmp;

    std::shared_ptr<AuthManagerBase3rd> authMgr = nullptr;
    std::lock_guard<ffrt::mutex> autoLock(authMgrMapLock_);
    auto it = authMgrMap_.find(processInfo3rd.tokenId);
    if (it != authMgrMap_.end()) {
        LOGI("AuthMgr already exists for token %{public}s",
            GetAnonyString(std::to_string(processInfo3rd.tokenId)).c_str());
        authMgr = it->second;
    } else {
        authMgr = std::make_shared<AuthSrcManager>(softbusConnector_, listener_, hiChainAuthConnector_);
        authMgrMap_[processInfo3rd.tokenId] = authMgr;
        LOGI("Created new AuthMgr for token %{public}s",
            GetAnonyString(std::to_string(processInfo3rd.tokenId)).c_str());
    }
    int32_t sessionId = softbusConnector_->GetSoftbusSession()->OpenSessionServer(targetId);
    if (sessionId < 0) {
        LOGE("OpenAuthSession failed, stop the authentication");
        return;
    }
    uint64_t logicalSessionId = GenerateRandNum(sessionId);
    CleanNotifyCallback cleanNotifyCallback = [=](const auto &logicalSessionId, const auto &connDelayCloseTime) {
        this->NotifyCleanEvent(logicalSessionId, connDelayCloseTime);
    };
    authMgr->RegisterCleanNotifyCallback(cleanNotifyCallback);
    CHECK_NULL_VOID(hiChainAuthConnector_);
    hiChainAuthConnector_->RegisterHiChainAuthCallbackById(logicalSessionId, authMgr);
    ImportAuthCodeAndUidFromCache(authMgr, processInfo3rd);
    {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        logicalSessionId2TokenIdMap_[logicalSessionId] = processInfo3rd.tokenId;
    }
    sessionEnableCvReadyMap_[sessionId] = false;
    std::unique_lock<ffrt::mutex> cvLock(sessionEnableMutexMap_[sessionId]);
    if (!sessionEnableCvMap_[sessionId].wait_for(cvLock, std::chrono::milliseconds(OPEN_AUTH_SESSION_TIMEOUT),
        [&] { return sessionEnableCvReadyMap_[sessionId]; })) {
        LOGE("wait session enable timeout or enable fail, sessionId: %{public}d.", sessionId);
        return;
    }
    authMgr->AuthDevice3rd(targetId, authParamTmp, sessionId, logicalSessionId);
}

int32_t DeviceManagerServiceImpl3rd::AuthDevice3rd(const PeerTargetId3rd &targetId,
    const std::map<std::string, std::string> &authParam)
{
    if (IsInvalidPeerTargetId(targetId)) {
        LOGE("Invalid parameter, params are empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, deviceId: %{public}s", targetId.deviceId.c_str());
    ProcessInfo3rd processInfo3rd;
    DmAuthCallerInfo3rd authCallerInfo3rd;
    GetBindCallerInfo(authCallerInfo3rd, processInfo3rd);
    std::map<std::string, std::string> authParamTmp;
    SetBindCallerInfoToAuthParam(authParam, authParamTmp, authCallerInfo3rd, processInfo3rd);
    LOGI("In, processName:%{public}s, tokenId:%{public}u", authCallerInfo3rd.processName.c_str(),
        authCallerInfo3rd.tokenId);
    {
        std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
        if (tokenIdSessionIdMap_.find(processInfo3rd.tokenId) != tokenIdSessionIdMap_.end()) {
            LOGE("AuthDevice3rd failed, this device is being auth. please try again later,"
                "processName:%{public}s, tokenId:%{public}u, sessionId: %{public}d",
                authCallerInfo3rd.processName.c_str(), authCallerInfo3rd.tokenId,
                tokenIdSessionIdMap_[processInfo3rd.tokenId]);
            return ERR_DM_AUTH_BUSINESS_BUSY;
        }
    }
    PinCodeInfo pinCodeInfo;
    {
        std::lock_guard<ffrt::mutex> lock(pinCodeLock_);
        if (pinCodeMap_.find(processInfo3rd) == pinCodeMap_.end()) {
            LOGE("pinCodeInfo is not exist");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        pinCodeInfo = pinCodeMap_[processInfo3rd];
    }
    std::thread newThread(&DeviceManagerServiceImpl3rd::AuthDeviceAclImpl, this, targetId, pinCodeInfo,
        authParamTmp, processInfo3rd);
    newThread.detach();
    LOGI("AuthDevice3rd completed");
    return DM_OK;
}

std::shared_ptr<AuthManagerBase3rd> DeviceManagerServiceImpl3rd::GetAuthMgrByTokenId(uint32_t tokenId)
{
    std::lock_guard<ffrt::mutex> autoLock(authMgrMapLock_);
    auto it = authMgrMap_.find(tokenId);
    if (it != authMgrMap_.end()) {
        return it->second;
    }
    return nullptr;
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

static uint32_t GetTokenId(bool isSrcSide, std::string &processName)
{
    int32_t tokenId = 0;
    if (isSrcSide) {
        // src end
        tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    } else {
        // sink end
        int32_t tmpTokenId;
        // get userId
        int32_t targetUserId =  MultipleUserConnector3rd::GetCurrentAccountUserID();
        if (targetUserId == -1) {
            return tokenId;
        }
        if (AppManager3rd::GetInstance().GetHapTokenIdByName(targetUserId, processName, 0, tmpTokenId) == DM_OK) {
            tokenId = static_cast<uint32_t>(tmpTokenId);
        } else if (AppManager3rd::GetInstance().GetNativeTokenIdByName(processName, tmpTokenId) == DM_OK) {
            tokenId = static_cast<uint32_t>(tmpTokenId);
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

std::shared_ptr<AuthManagerBase3rd> DeviceManagerServiceImpl3rd::GetAuthMgrByMessage(int32_t msgType,
    uint64_t logicalSessionId, const JsonObject &jsonObject)
{
    uint32_t tokenId = 0;
    if (msgType == DmMessageType::ACL_REQ_NEGOTIATE) {
        std::string processName;
        std::string businessName;
        if (IsString(jsonObject, TAG_PEER_BUSINESS_NAME)) {
            businessName = jsonObject[TAG_PEER_BUSINESS_NAME].Get<std::string>();
        }
        if (IsString(jsonObject, TAG_PEER_PROCESS_NAME)) {
            processName = jsonObject[TAG_PEER_PROCESS_NAME].Get<std::string>();
        }
        tokenId = GetTokenId(false, processName);
        if (tokenId == 0) {
            LOGE("Get tokenId failed.");
            return nullptr;
        }
        ProcessInfo3rd processInfo3rd;
        processInfo3rd.processName = processName;
        processInfo3rd.businessName = businessName;
        processInfo3rd.tokenId = tokenId;
        processInfo3rd.userId = MultipleUserConnector3rd::GetCurrentAccountUserID();
        if (InitAuthMgr(false, tokenId, logicalSessionId, processInfo3rd) != DM_OK) {
            return nullptr;
        }
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        logicalSessionId2TokenIdMap_[logicalSessionId] = tokenId;
    } else {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        if (logicalSessionId2TokenIdMap_.find(logicalSessionId) == logicalSessionId2TokenIdMap_.end()) {
            LOGE("Get tokenId failed from map.");
            return nullptr;
        }
        tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
    }
    return GetAuthMgrByTokenId(tokenId);
}

int32_t DeviceManagerServiceImpl3rd::InitAuthMgr(bool isSrcSide, uint32_t tokenId, uint64_t logicalSessionId,
    ProcessInfo3rd processInfo3rd)
{
    LOGI("isSrcSide:%{public}d, tokenId: %{public}s",
        isSrcSide, GetAnonyString(std::to_string(tokenId)).c_str());
    std::shared_ptr<AuthManagerBase3rd> authMgr = nullptr;
    // Create a new auth_mgr, create authMgr
    if (isSrcSide) {
        // src end
        authMgr = std::make_shared<AuthSrcManager>(softbusConnector_, listener_,
            hiChainAuthConnector_);
    } else {
        // sink end
        authMgr = std::make_shared<AuthSinkManager>(softbusConnector_, listener_,
            hiChainAuthConnector_);
    }
    CleanNotifyCallback cleanNotifyCallback = [=](const auto &logicalSessionId, const auto &connDelayCloseTime) {
        this->NotifyCleanEvent(logicalSessionId, connDelayCloseTime);
    };
    // Register resource destruction notification function
    authMgr->RegisterCleanNotifyCallback(cleanNotifyCallback);
    CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
    hiChainAuthConnector_->RegisterHiChainAuthCallbackById(logicalSessionId, authMgr);
    LOGI("Initialize authMgr token: %{public}d.", tokenId);
    ImportAuthCodeAndUidFromCache(authMgr, processInfo3rd);
    return AddAuthMgr(tokenId, authMgr);
}

int32_t DeviceManagerServiceImpl3rd::AddAuthMgr(uint32_t tokenId, std::shared_ptr<AuthManagerBase3rd> authMgr)
{
    if (authMgr == nullptr) {
        LOGE("authMgr is nullptr.");
        return ERR_DM_POINT_NULL;
    }
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        if (authMgrMap_.size() >= MAX_NEW_PROC_SESSION_COUNT_TEMP) {
            LOGE("Other bind session exist, can not start new one. authMgrMap_.size:%{public}zu", authMgrMap_.size());
            return ERR_DM_AUTH_BUSINESS_BUSY;
        }
        authMgrMap_[tokenId] = authMgr;
    }
    return DM_OK;
}

void DeviceManagerServiceImpl3rd::ImportAuthCodeAndUidFromCache(std::shared_ptr<AuthManagerBase3rd> authMgr,
    const ProcessInfo3rd processInfo3rd)
{
    CHECK_NULL_VOID(authMgr);
    PinCodeInfo pinCodeInfo;
    {
        std::lock_guard<ffrt::mutex> lock(pinCodeLock_);
        for (const auto &it : pinCodeMap_) {
            if (it.first.tokenId == processInfo3rd.tokenId &&
               it.first.processName == processInfo3rd.processName &&
               it.first.businessName == processInfo3rd.businessName &&
               it.first.userId == processInfo3rd.userId) {
                authMgr->ImportAuthCodeAndUid(processInfo3rd.businessName, it.second.pinCode, it.first.uid);
                return;
            }
        }
    }
}

void DeviceManagerServiceImpl3rd::NotifyCleanEvent(uint64_t logicalSessionId, int32_t connDelayCloseTime)
{
    LOGI("logicalSessionId: %{public}" PRIu64 ".", logicalSessionId);
    ffrt::submit([=]() { CleanAuthMgrByLogicalSessionId(logicalSessionId, connDelayCloseTime); });
}

std::shared_ptr<AuthManagerBase3rd> DeviceManagerServiceImpl3rd::GetAuthMgr()
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return GetAuthMgrByTokenId(tokenId);
}

void DeviceManagerServiceImpl3rd::CleanAuthMgrByLogicalSessionId(uint64_t logicalSessionId, int32_t connDelayCloseTime)
{
    uint32_t tokenId = 0;
    {
        std::lock_guard<ffrt::mutex> tokenIdLock(logicalSessionId2TokenIdMapMtx_);
        if (logicalSessionId2TokenIdMap_.find(logicalSessionId) != logicalSessionId2TokenIdMap_.end()) {
            tokenId = logicalSessionId2TokenIdMap_[logicalSessionId];
        } else {
            LOGE("logicalSessionId(%{public}" PRIu64 ") can not find the tokenId.", logicalSessionId);
            return;
        }
    }
    hiChainAuthConnector_->UnRegisterHiChainAuthCallbackById(logicalSessionId);
    EraseAuthMgr(tokenId);
    return;
}

void DeviceManagerServiceImpl3rd::EraseAuthMgr(uint32_t tokenId)
{
    {
        std::lock_guard<ffrt::mutex> lock(authMgrMapMtx_);
        if (authMgrMap_.find(tokenId) != authMgrMap_.end()) {
            LOGI("authMgrMap_ erase token: %{public}d.", tokenId);
            authMgrMap_.erase(tokenId);
        }
    }
    {
        std::lock_guard<ffrt::mutex> lock(tokenIdSessionIdMapMtx_);
        LOGI("tokenIdSessionIdMap_ erase token: %{public}d", tokenId);
        tokenIdSessionIdMap_.erase(tokenId);
    }
}

extern "C" IDeviceManagerServiceImpl3rd *CreateDMServiceImpl3rdObject(void)
{
    return new DeviceManagerServiceImpl3rd;
}
} // namespace DistributedHardware
} // namespace OHOS