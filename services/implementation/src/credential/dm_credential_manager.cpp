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

#include "dm_credential_manager.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_hash.h"
#include "dm_log.h"
#include "dm_random.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
void from_json(const nlohmann::json &jsonObject, CredentialData &credentialData)
{
    if (!jsonObject.contains(FIELD_CREDENTIAL_TYPE) || !jsonObject.contains(FIELD_CREDENTIAL_ID) ||
        !jsonObject.contains(FIELD_SERVER_PK) || !jsonObject.contains(FIELD_PKINFO_SIGNATURE) ||
        !jsonObject.contains(FIELD_PKINFO) || !jsonObject.contains(FIELD_AUTH_CODE) ||
        !jsonObject.contains(FIELD_PEER_DEVICE_ID)) {
        LOGE("CredentialData json key not complete");
        return;
    }
    jsonObject[FIELD_CREDENTIAL_TYPE].get_to(credentialData.credentialType);
    jsonObject[FIELD_CREDENTIAL_ID].get_to(credentialData.credentialId);
    jsonObject[FIELD_SERVER_PK].get_to(credentialData.serverPk);
    jsonObject[FIELD_PKINFO_SIGNATURE].get_to(credentialData.pkInfoSignature);
    jsonObject[FIELD_PKINFO].get_to(credentialData.pkInfo);
    jsonObject[FIELD_AUTH_CODE].get_to(credentialData.authCode);
    jsonObject[FIELD_PEER_DEVICE_ID].get_to(credentialData.peerDeviceId);
}

DmCredentialManager::DmCredentialManager(std::shared_ptr<HiChainConnector> hiChainConnector,
                                         std::shared_ptr<IDeviceManagerServiceListener> listener)
    : hiChainConnector_(hiChainConnector), listener_(listener)
{
    LOGI("DmCredentialManager constructor");
}

DmCredentialManager::~DmCredentialManager()
{
    LOGI("DmCredentialManager destructor");
}

int32_t DmCredentialManager::RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr)
{
    LOGI("start to request credential.");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    nlohmann::json jsonObject = nlohmann::json::parse(reqJsonStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("reqJsonStr string not a json type.");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.contains(FIELD_USER_ID) || !jsonObject.contains(FIELD_CREDENTIAL_VERSION)) {
        LOGE("user id or credential version string key not exist!");
        return ERR_DM_FAILED;
    }
    std::string userId = jsonObject[FIELD_USER_ID];
    std::string credentialVersion = jsonObject[FIELD_CREDENTIAL_VERSION];
    nlohmann::json jsonObj;
    jsonObj[FIELD_CREDENTIAL_VERSION] = credentialVersion;
    jsonObj[FIELD_USER_ID] = userId;
    jsonObj[FIELD_DEVICE_ID] = localDeviceId;
    std::string tmpStr = jsonObj.dump();
    return hiChainConnector_->getRegisterInfo(tmpStr.c_str(), returnJsonStr);
}

int32_t DmCredentialManager::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    std::lock_guard<std::mutex> autoLock(locks_);
    std::vector<std::string>::iterator iter = std::find(credentialVec_.begin(), credentialVec_.end(), pkgName);
    if (iter == credentialVec_.end()) {
        LOGE("credentialInfo not found by pkgName %s", GetAnonyString(pkgName).c_str());
        return ERR_DM_FAILED;
    }
    pkgName_ = pkgName;
    nlohmann::json jsonObject = nlohmann::json::parse(credentialInfo, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("credentialInfo string not a json type.");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.contains(FIELD_PROCESS_TYPE)) {
        LOGE("credential type string key not exist!");
        return ERR_DM_FAILED;
    }
    int32_t processType = jsonObject[FIELD_PROCESS_TYPE];
    if (processType == LOCAL_CREDENTIAL_DEAL_TYPE) {
        return ImportLocalCredential(credentialInfo);
    } else if (processType == REMOTE_CREDENTIAL_DEAL_TYPE) {
        return DM_OK;
    } else {
        LOGE("credential type error!");
    }
    return ERR_DM_FAILED;
}

int32_t DmCredentialManager::ImportLocalCredential(const std::string &credentialInfo)
{
    LOGI("ImportLocalCredential start");
    nlohmann::json jsonObject = nlohmann::json::parse(credentialInfo, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("credentialInfo string not a json type.");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.contains(FIELD_AUTH_TYPE) || !jsonObject.contains(FIELD_USER_ID)
        || !jsonObject.contains(FIELD_CREDENTIAL_DATA)) {
        LOGE("auth type or user id string key not exist!");
        return ERR_DM_FAILED;
    }
    int32_t authType = jsonObject[FIELD_AUTH_TYPE];
    if (authType == SAME_ACCOUNT_TYPE) {
        authType = IDENTICAL_ACCOUNT_GROUP;
    }
    if (authType == CROSS_ACCOUNT_TYPE) {
        authType = ACROSS_ACCOUNT_AUTHORIZE_GROUP;
    }
    std::string userId = jsonObject[FIELD_USER_ID];
    requestId_ = GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
    nlohmann::json creArray = jsonObject[FIELD_CREDENTIAL_DATA.c_str()];
    if (!creArray.is_array()) {
        LOGI("ImportLocalCredential credentialData is not a array object!");
        return ERR_DM_FAILED;
    }
    std::vector<CredentialData> vecCredentialData =
        jsonObject[FIELD_CREDENTIAL_DATA.c_str()].get<std::vector<CredentialData>>();
    if (vecCredentialData.size() != 1) {
        LOGI("ImportLocalCredential credentialData err");
        return ERR_DM_FAILED;
    }
    LOGI("ImportLocalCredential get credentialData success!");
    nlohmann::json jsonOutObj;
    if (GetCredentialData(credentialInfo, vecCredentialData[0], jsonOutObj) != DM_OK) {
        LOGE("failed to get credentialData field from input credential.");
        return ERR_DM_FAILED;
    }
    if (hiChainConnector_->CreateGroup(requestId_, authType, userId, jsonOutObj) != DM_OK) {
        LOGE("failed to create hichain group function.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DmCredentialManager::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    std::lock_guard<std::mutex> autoLock(locks_);
    std::vector<std::string>::iterator iter = std::find(credentialVec_.begin(), credentialVec_.end(), pkgName);
    if (iter == credentialVec_.end()) {
        LOGE("credentialInfo not found by pkgName %s", GetAnonyString(pkgName).c_str());
        return ERR_DM_FAILED;
    }
    pkgName_ = pkgName;
    nlohmann::json jsonObject = nlohmann::json::parse(deleteInfo, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("deleteInfo string not a json type.");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.contains(FIELD_PROCESS_TYPE) || !jsonObject.contains(FIELD_AUTH_TYPE)
        || !jsonObject.contains(FIELD_USER_ID)) {
        LOGE("process type string key not exist!");
        return ERR_DM_FAILED;
    }
    int32_t processType = jsonObject[FIELD_PROCESS_TYPE];
    int32_t authType = jsonObject[FIELD_AUTH_TYPE];
    if (authType == SAME_ACCOUNT_TYPE) {
        authType = IDENTICAL_ACCOUNT_GROUP;
    }
    if (authType == CROSS_ACCOUNT_TYPE) {
        authType = ACROSS_ACCOUNT_AUTHORIZE_GROUP;
    }
    std::string userId = jsonObject[FIELD_USER_ID];
    requestId_ = GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
    if (processType == LOCAL_CREDENTIAL_DEAL_TYPE) {
        return hiChainConnector_->DeleteGroup(requestId_, userId, authType);
    } else if (processType == REMOTE_CREDENTIAL_DEAL_TYPE) {
        return DM_OK;
    } else {
        LOGE("credential type error!");
    }
    return DM_OK;
}

void DmCredentialManager::OnGroupResult(int64_t requestId, int32_t action,
    const std::string &resultInfo)
{
    LOGI("DmCredentialManager::OnImportResult");
    if (requestId_ != requestId) {
        return;
    }
    listener_->OnCredentialResult(pkgName_, action, resultInfo);
}

void DmCredentialManager::RegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DmCredentialManager::RegisterCredentialCallback input param is empty");
        return;
    }
    LOGI("DmCredentialManager::RegisterCredentialCallback pkgName=%s",
        GetAnonyString(pkgName).c_str());
    credentialVec_.push_back(pkgName);
    hiChainConnector_->RegisterHiChainGroupCallback(std::shared_ptr<IDmGroupResCallback>(shared_from_this()));
}

void DmCredentialManager::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DmCredentialManager::UnRegisterCredentialStateCallback input param is empty");
        return;
    }
    LOGI("DmCredentialManager::UnRegisterCredentialStateCallback pkgName=%s",
        GetAnonyString(pkgName).c_str());
    std::vector<std::string>::iterator iter = std::find(credentialVec_.begin(), credentialVec_.end(), pkgName);
    if (iter != credentialVec_.end()) {
        credentialVec_.erase(iter);
    }
    hiChainConnector_->UnRegisterHiChainGroupCallback();
}

int32_t DmCredentialManager::GetCredentialData(const std::string &credentialInfo, const CredentialData &inputCreData,
    nlohmann::json &jsonOutObj)
{
    nlohmann::json jsonCreObj;
    jsonCreObj[FIELD_CREDENTIAL_TYPE] = inputCreData.credentialType;
    int32_t credentialType = inputCreData.credentialType;
    if (credentialType == NONSYMMETRY_CREDENTIAL_TYPE) {
        nlohmann::json jsonObject = nlohmann::json::parse(credentialInfo, nullptr, false);
        if (jsonObject.is_discarded()) {
            LOGE("credentialInfo string not a json type.");
            return ERR_DM_FAILED;
        }
        if (!jsonObject.contains(FIELD_USER_ID) || !jsonObject.contains(FIELD_CREDENTIAL_VERSION)
            || !jsonObject.contains(FIELD_DEVICE_ID) || !jsonObject.contains(FIELD_DEVICE_PK)) {
            LOGE("auth type or user id string key not exist!");
            return ERR_DM_FAILED;
        }
        std::string userId = jsonObject[FIELD_USER_ID];
        std::string deviceId = jsonObject[FIELD_DEVICE_ID];
        std::string verSion = jsonObject[FIELD_CREDENTIAL_VERSION];
        std::string devicePk = jsonObject[FIELD_DEVICE_PK];
        nlohmann::json jsonPkInfo;
        jsonPkInfo[FIELD_USER_ID] = userId;
        jsonPkInfo[FIELD_DEVICE_ID] = deviceId;
        jsonPkInfo[FIELD_CREDENTIAL_VERSION] = verSion;
        jsonPkInfo[FIELD_DEVICE_PK] = devicePk;
        jsonCreObj[FIELD_PKINFO] = jsonPkInfo;
        jsonCreObj[FIELD_SERVER_PK] = inputCreData.serverPk;
        jsonCreObj[FIELD_PKINFO_SIGNATURE] = inputCreData.pkInfoSignature;
    } else if (credentialType == SYMMETRY_CREDENTIAL_TYPE) {
        jsonCreObj[FIELD_AUTH_CODE] = inputCreData.authCode;
    } else {
        LOGE("invalid credentialType field!");
        return ERR_DM_FAILED;
    }
    jsonOutObj = jsonCreObj;
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
