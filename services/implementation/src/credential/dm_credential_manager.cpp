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
#include "dm_log.h"
#include "dm_random.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t LOCAL_CREDENTIAL_DEAL_TYPE = 1;
const int32_t REMOTE_CREDENTIAL_DEAL_TYPE = 2;
const int32_t NONSYMMETRY_CREDENTIAL_TYPE = 2;
const int32_t SYMMETRY_CREDENTIAL_TYPE = 1;
const int32_t UNKNOWN_CREDENTIAL_TYPE = 0;
const int32_t SAME_ACCOUNT_TYPE = 1;
const int32_t CROSS_ACCOUNT_TYPE = 2;

constexpr const char* FIELD_CREDENTIAL_VERSION = "version";
constexpr const char* FIELD_DEVICE_PK = "devicePk";
constexpr const char* FIELD_SERVER_PK = "serverPk";
constexpr const char* FIELD_PKINFO_SIGNATURE = "pkInfoSignature";
constexpr const char* FIELD_PKINFO = "pkInfo";
constexpr const char* FIELD_PROCESS_TYPE = "processType";
constexpr const char* FIELD_AUTH_TYPE = "authType";
constexpr const char* FIELD_CREDENTIAL_DATA = "credentialData";
constexpr const char* FIELD_CREDENTIAL_ID = "credentialId";
constexpr const char* FIELD_PEER_CREDENTIAL_INFO = "peerCredentialInfo";

struct CredentialDataInfo {
    int32_t credentialType;
    std::string credentailId;
    std::string serverPk;
    std::string pkInfoSignature;
    std::string pkInfo;
    std::string authCode;
    std::string peerDeviceId;
    std::string userId;
    CredentialDataInfo() : credentialType(UNKNOWN_CREDENTIAL_TYPE)
    {
    }
};

struct PeerCredentialInfo {
    std::string peerDeviceId;
    std::string peerCredentialId;
};
void from_json(const nlohmann::json &jsonObject, CredentialData &credentialData)
{
    if (!IsInt32(jsonObject, FIELD_CREDENTIAL_TYPE) || !IsString(jsonObject, FIELD_CREDENTIAL_ID) ||
        !IsString(jsonObject, FIELD_SERVER_PK) || !IsString(jsonObject, FIELD_PKINFO_SIGNATURE) ||
        !IsString(jsonObject, FIELD_PKINFO) || !IsString(jsonObject, FIELD_AUTH_CODE) ||
        !IsString(jsonObject, FIELD_PEER_DEVICE_ID)) {
        LOGE("CredentialData json key not complete");
        return;
    }
    credentialData.credentialType = jsonObject[FIELD_CREDENTIAL_TYPE].get<int32_t>();
    credentialData.credentialId = jsonObject[FIELD_CREDENTIAL_ID].get<std::string>();
    credentialData.serverPk = jsonObject[FIELD_SERVER_PK].get<std::string>();
    credentialData.pkInfoSignature = jsonObject[FIELD_PKINFO_SIGNATURE].get<std::string>();
    credentialData.pkInfo = jsonObject[FIELD_PKINFO].get<std::string>();
    credentialData.authCode = jsonObject[FIELD_AUTH_CODE].get<std::string>();
    credentialData.peerDeviceId = jsonObject[FIELD_PEER_DEVICE_ID].get<std::string>();
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
    if (!IsString(jsonObject, FIELD_USER_ID) || !IsString(jsonObject, FIELD_CREDENTIAL_VERSION)) {
        LOGE("user id or credential version string key not exist!");
        return ERR_DM_FAILED;
    }
    std::string userId = jsonObject[FIELD_USER_ID].get<std::string>();
    std::string credentialVersion = jsonObject[FIELD_CREDENTIAL_VERSION].get<std::string>();
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
    if (!IsInt32(jsonObject, FIELD_PROCESS_TYPE)) {
        LOGE("credential type string key not exist!");
        return ERR_DM_FAILED;
    }
    int32_t processType = jsonObject[FIELD_PROCESS_TYPE].get<int32_t>();
    if (processType == LOCAL_CREDENTIAL_DEAL_TYPE) {
        return ImportLocalCredential(credentialInfo);
    } else if (processType == REMOTE_CREDENTIAL_DEAL_TYPE) {
        return ImportRemoteCredential(credentialInfo);
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
    if (!IsInt32(jsonObject, FIELD_AUTH_TYPE) || !IsString(jsonObject, FIELD_USER_ID) ||
        !IsArray(jsonObject, FIELD_CREDENTIAL_DATA)) {
        LOGE("auth type or user id or credential data string key not exist!");
        return ERR_DM_FAILED;
    }
    int32_t authType = jsonObject[FIELD_AUTH_TYPE].get<int32_t>();
    if (authType == SAME_ACCOUNT_TYPE) {
        authType = IDENTICAL_ACCOUNT_GROUP;
    }
    if (authType == CROSS_ACCOUNT_TYPE) {
        authType = ACROSS_ACCOUNT_AUTHORIZE_GROUP;
    }
    std::string userId = jsonObject[FIELD_USER_ID].get<std::string>();
    requestId_ = GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);

    std::vector<CredentialData> vecCredentialData =
        jsonObject[FIELD_CREDENTIAL_DATA].get<std::vector<CredentialData>>();
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
    if (!IsInt32(jsonObject, FIELD_PROCESS_TYPE) || !IsInt32(jsonObject, FIELD_AUTH_TYPE) ||
        !IsString(jsonObject, FIELD_USER_ID)) {
        LOGE("DmCredentialManager::DeleteCredential err json string!");
        return ERR_DM_FAILED;
    }
    int32_t processType = jsonObject[FIELD_PROCESS_TYPE].get<int32_t>();
    int32_t authType = jsonObject[FIELD_AUTH_TYPE].get<int32_t>();
    if (authType == SAME_ACCOUNT_TYPE) {
        authType = IDENTICAL_ACCOUNT_GROUP;
    }
    if (authType == CROSS_ACCOUNT_TYPE) {
        authType = ACROSS_ACCOUNT_AUTHORIZE_GROUP;
    }
    std::string userId = jsonObject[FIELD_USER_ID].get<std::string>();
    requestId_ = GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
    if (processType == LOCAL_CREDENTIAL_DEAL_TYPE) {
        return hiChainConnector_->DeleteGroup(requestId_, userId, authType);
    } else if (processType == REMOTE_CREDENTIAL_DEAL_TYPE) {
        return DeleteRemoteCredential(deleteInfo);
    } else {
        LOGE("credential type error!");
    }
    return ERR_DM_FAILED;
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

int32_t DmCredentialManager::RegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DmCredentialManager::RegisterCredentialCallback input param is empty");
        return ERR_DM_FAILED;
    }
    LOGI("DmCredentialManager::RegisterCredentialCallback pkgName = %s",
        GetAnonyString(pkgName).c_str());
    credentialVec_.push_back(pkgName);
    return hiChainConnector_->RegisterHiChainGroupCallback(std::shared_ptr<IDmGroupResCallback>(shared_from_this()));
}

int32_t DmCredentialManager::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DmCredentialManager::UnRegisterCredentialStateCallback input param is empty");
        return ERR_DM_FAILED;
    }
    LOGI("DmCredentialManager::UnRegisterCredentialStateCallback pkgName = %s",
        GetAnonyString(pkgName).c_str());
    std::vector<std::string>::iterator iter = std::find(credentialVec_.begin(), credentialVec_.end(), pkgName);
    if (iter != credentialVec_.end()) {
        credentialVec_.erase(iter);
    }
    return hiChainConnector_->UnRegisterHiChainGroupCallback();
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
        if (!IsString(jsonObject, FIELD_USER_ID) || !IsString(jsonObject, FIELD_CREDENTIAL_VERSION) ||
            !IsString(jsonObject, FIELD_DEVICE_ID) || !IsString(jsonObject, FIELD_DEVICE_PK)) {
            LOGE("DmCredentialManager::GetCredentialData err json string!");
            return ERR_DM_FAILED;
        }
        std::string userId = jsonObject[FIELD_USER_ID].get<std::string>();
        std::string deviceId = jsonObject[FIELD_DEVICE_ID].get<std::string>();
        std::string verSion = jsonObject[FIELD_CREDENTIAL_VERSION].get<std::string>();
        std::string devicePk = jsonObject[FIELD_DEVICE_PK].get<std::string>();
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

void from_json(const nlohmann::json &jsonObject, CredentialDataInfo &credentialDataInfo)
{
    if (!IsInt32(jsonObject, FIELD_CREDENTIAL_TYPE)) {
        LOGE("credentialType json key not exist");
        return;
    }
    credentialDataInfo.credentialType = jsonObject[FIELD_CREDENTIAL_TYPE].get<int32_t>();
    if (IsString(jsonObject, FIELD_CREDENTIAL_ID)) {
        credentialDataInfo.credentailId = jsonObject[FIELD_CREDENTIAL_ID].get<std::string>();
    }
    if (credentialDataInfo.credentialType == NONSYMMETRY_CREDENTIAL_TYPE) {
        if (IsString(jsonObject, FIELD_SERVER_PK)) {
            credentialDataInfo.serverPk = jsonObject[FIELD_SERVER_PK].get<std::string>();
        }
        if (IsString(jsonObject, FIELD_PKINFO_SIGNATURE)) {
            credentialDataInfo.pkInfoSignature = jsonObject[FIELD_PKINFO_SIGNATURE].get<std::string>();
        }
        if (IsString(jsonObject, FIELD_PKINFO)) {
            nlohmann::json jsonPkInfo = jsonObject[FIELD_PKINFO];
            credentialDataInfo.pkInfo = jsonPkInfo.dump();
        }
    } else if (credentialDataInfo.credentialType == SYMMETRY_CREDENTIAL_TYPE) {
        if (IsString(jsonObject, FIELD_AUTH_CODE)) {
            credentialDataInfo.authCode = jsonObject[FIELD_AUTH_CODE].get<std::string>();
        }
    } else {
        LOGE("credentialType john key is unknown");
        return;
    }
    if (IsString(jsonObject, FIELD_PEER_DEVICE_ID)) {
        credentialDataInfo.peerDeviceId = jsonObject[FIELD_PEER_DEVICE_ID].get<std::string>();
    }
}

void to_json(nlohmann::json &jsonObject, const CredentialDataInfo &credentialDataInfo)
{
    jsonObject[FIELD_DEVICE_ID] = credentialDataInfo.peerDeviceId;
    jsonObject[FIELD_UDID] =credentialDataInfo.peerDeviceId;
    jsonObject[FIELD_USER_ID] = credentialDataInfo.userId;
    jsonObject[FIELD_CREDENTIAL_TYPE] = credentialDataInfo.credentialType;
    jsonObject[FIELD_CREDENTIAL_ID] = atoi(credentialDataInfo.credentailId.c_str());
    if (credentialDataInfo.credentialType == NONSYMMETRY_CREDENTIAL_TYPE) {
        jsonObject[FIELD_SERVER_PK] = credentialDataInfo.serverPk;
        jsonObject[FIELD_PKINFO_SIGNATURE] = credentialDataInfo.pkInfoSignature;
        jsonObject[FIELD_PKINFO] = credentialDataInfo.pkInfo;
    } else if (credentialDataInfo.credentialType == SYMMETRY_CREDENTIAL_TYPE) {
        jsonObject[FIELD_AUTH_CODE] = credentialDataInfo.authCode;
    }
}

int32_t DmCredentialManager::GetAddDeviceList(const nlohmann::json &jsonObject, nlohmann::json &jsonDeviceList)
{
    if (!jsonObject.contains(FIELD_CREDENTIAL_DATA) || !jsonObject[FIELD_CREDENTIAL_DATA].is_object()||
        !IsInt32(jsonObject, FIELD_AUTH_TYPE)) {
        LOGE("credentaildata or authType string key not exist!");
        return ERR_DM_FAILED;
    }
    nlohmann::json credentialJson = jsonObject[FIELD_CREDENTIAL_DATA];
    auto credentialDataList = credentialJson.get<std::vector<CredentialDataInfo>>();
    int32_t authType = jsonObject[FIELD_AUTH_TYPE].get<int32_t>();

    for (auto &credentialData : credentialDataList) {
        if (authType == SAME_ACCOUNT_TYPE) {
            if (IsString(jsonObject, FIELD_USER_ID)) {
                credentialData.userId = jsonObject[FIELD_USER_ID].get<std::string>();
            }
        } else if (authType == CROSS_ACCOUNT_TYPE) {
            if (IsString(jsonObject, FIELD_PEER_USER_ID)) {
                credentialData.userId = jsonObject[FIELD_PEER_USER_ID].get<std::string>();
            }
        }
    }
    for (size_t i = 0; i < credentialDataList.size(); i++) {
        jsonDeviceList[FIELD_DEVICE_LIST][i] = credentialDataList.at(i);
    }
    return DM_OK;
}

int32_t DmCredentialManager::ImportRemoteCredential(const std::string &credentialInfo)
{
    nlohmann::json jsonObject = nlohmann::json::parse(credentialInfo, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("credentialInfo string not a json type.");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, FIELD_AUTH_TYPE)) {
        LOGE("auth type string key not exist!");
        return ERR_DM_FAILED;
    }
    int32_t authType = jsonObject[FIELD_AUTH_TYPE].get<int32_t>();
    std::string userId;
    int32_t groupType = 0;
    if (authType == SAME_ACCOUNT_TYPE) {
        groupType = IDENTICAL_ACCOUNT_GROUP;
        if (!IsString(jsonObject, FIELD_USER_ID)) {
            LOGE("userId string key not exist!");
            return ERR_DM_FAILED;
        } else {
            userId = jsonObject[FIELD_USER_ID].get<std::string>();
        }
    } else if (authType == CROSS_ACCOUNT_TYPE) {
        groupType = ACROSS_ACCOUNT_AUTHORIZE_GROUP;
        if (!IsString(jsonObject, FIELD_PEER_USER_ID)) {
            LOGE("peerUserId string key not exist!");
            return ERR_DM_FAILED;
        } else {
            userId = jsonObject[FIELD_PEER_USER_ID].get<std::string>();
        }
    }
    nlohmann::json jsonDeviceList;
    if (GetAddDeviceList(jsonObject, jsonDeviceList) != DM_OK) {
        LOGE("failed to get add DeviceList.");
        return ERR_DM_FAILED;
    }
    if (hiChainConnector_->addMultiMembers(groupType, userId, jsonDeviceList) != DM_OK) {
        LOGE("failed to add members to group.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void from_json(const nlohmann::json &jsonObject, PeerCredentialInfo &peerCredentialInfo)
{
    if (IsString(jsonObject, FIELD_PEER_USER_ID)) {
        peerCredentialInfo.peerDeviceId = jsonObject[FIELD_PEER_USER_ID].get<std::string>();
    }
}

void to_json(nlohmann::json &jsonObject, const PeerCredentialInfo &peerCredentialInfo)
{
    jsonObject[FIELD_DEVICE_ID] = peerCredentialInfo.peerDeviceId;
}

int32_t GetDeleteDeviceList(const nlohmann::json &jsonObject, nlohmann::json &deviceList)
{
    if (!IsArray(jsonObject, FIELD_PEER_CREDENTIAL_INFO)) {
        LOGE("devicelist string key not exist!");
        return ERR_DM_FAILED;
    }
    auto peerCredentialInfo = jsonObject[FIELD_PEER_CREDENTIAL_INFO].get<std::vector<PeerCredentialInfo>>();
    for (size_t i = 0; i < peerCredentialInfo.size(); i++) {
        deviceList[FIELD_DEVICE_LIST][i] = peerCredentialInfo[i];
    }
    return DM_OK;
}

int32_t DmCredentialManager::DeleteRemoteCredential(const std::string &deleteInfo)
{
    nlohmann::json jsonObject = nlohmann::json::parse(deleteInfo, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("credentialInfo string not a json type.");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, FIELD_AUTH_TYPE)) {
        LOGE("authType, peerCredential or peerUserId string key not exist!");
        return ERR_DM_FAILED;
    }
    int32_t authType = jsonObject[FIELD_AUTH_TYPE].get<int32_t>();
    std::string userId;
    int32_t groupType = 0;
    if (authType == SAME_ACCOUNT_TYPE) {
        if (!IsString(jsonObject, FIELD_USER_ID)) {
            LOGE("userId string key not exist.");
            return ERR_DM_FAILED;
        } else {
            userId = jsonObject[FIELD_USER_ID].get<std::string>();
        }
        groupType = IDENTICAL_ACCOUNT_GROUP;
    } else if (authType == CROSS_ACCOUNT_TYPE) {
        if (!IsString(jsonObject, FIELD_PEER_USER_ID)) {
            LOGE("peerUserId string key not exist.");
            return ERR_DM_FAILED;
        } else {
            userId = jsonObject[FIELD_PEER_USER_ID].get<std::string>();
        }
        groupType = ACROSS_ACCOUNT_AUTHORIZE_GROUP;
    }
    nlohmann::json jsonDeviceList;
    if (GetDeleteDeviceList(jsonObject, jsonDeviceList) != DM_OK) {
        LOGE("failed to get delete DeviceList.");
        return ERR_DM_FAILED;
    }
    if (hiChainConnector_->deleteMultiMembers(groupType, userId, jsonDeviceList) != DM_OK) {
        LOGE("failed to delete members from group.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS