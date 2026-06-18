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

#include <cstdio>
#include <cstring>
#include <cctype>

#include "deviceprofile_connector_Refactor.h"
#include "crypto_mgr.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "multiple_user_connector.h"
#include "distributed_device_profile_client.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"
#include "dm_jsonstr_handle.h"
#include "app_manager.h"

using namespace OHOS::DistributedDeviceProfile;

const uint32_t INVALIED_TYPE = 0;
const uint32_t APP_PEER_TO_PEER_TYPE = 1;
const uint32_t APP_ACROSS_ACCOUNT_TYPE = 2;
const uint32_t SERVICE_PEER_TO_PEER_TYPE = 3;
const uint32_t SERVICE_ACROSS_ACCOUNT_TYPE = 4;
const uint32_t SHARE_TYPE = 5;
const uint32_t DEVICE_PEER_TO_PEER_TYPE = 6;
const uint32_t DEVICE_ACROSS_ACCOUNT_TYPE = 7;
const uint32_t IDENTICAL_ACCOUNT_TYPE = 8;
const uint32_t DM_INVALIED_TYPE = 2048;
const uint32_t SERVICE = 2;
const uint32_t APP = 3;
const uint32_t USER = 1;
const int64_t DM_DEFAULT_SERVICE_ID = 0;
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;
constexpr int32_t INVALID_USER_ID = -1;

const char* TAG_PEER_BUNDLE_NAME = "peerBundleName";
const char* TAG_PEER_TOKENID = "peerTokenId";
const char* TAG_ACL = "accessControlTable";
const char* TAG_DMVERSION = "dmVersion";
const char* TAG_ACL_HASH_KEY_VERSION = "aclVersion";
const char* TAG_ACL_HASH_KEY_ACLHASHLIST = "aclHashList";

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t DM_SUPPORT_ACL_AGING_VERSION_NUM = 1;
const std::string DM_VERSION_STR_5_1_0 = DM_VERSION_5_1_0;
const std::vector<std::string> DM_SUPPORT_ACL_AGING_VERSIONS = {DM_VERSION_STR_5_1_0};
constexpr uint32_t AUTH_EXT_WHITE_LIST_NUM = 1;
constexpr const static char* g_extWhiteList[AUTH_EXT_WHITE_LIST_NUM] = {
    "CastEngineService",
};
enum DmDevBindType : int32_t {
    DEVICE_PEER_TO_PEER_BIND_TYPE = 3,
    DEVICE_ACROSS_ACCOUNT_BIND_TYPE = 4,
    IDENTICAL_ACCOUNT_BIND_TYPE = 5
};
}

void DmSecureUtils::SecureClearString(std::string &str)
{
    if (str.empty()) {
        return;
    }
    volatile char *ptr = const_cast<volatile char *>(str.data());
    for (size_t i = 0; i < str.size(); i++) {
        ptr[i] = 0;
    }
    str.clear();
}

void DmSecureUtils::SecureClearBuffer(void *buffer, size_t size)
{
    if (buffer == nullptr || size == 0) {
        return;
    }
    volatile char *ptr = static_cast<volatile char *>(buffer);
    for (size_t i = 0; i < size; i++) {
        ptr[i] = 0;
    }
}

DmDeviceIdValidationResult DmSecureUtils::ValidateDeviceId(const std::string &deviceId)
{
    if (deviceId.empty()) {
        LOGE("DeviceId is empty");
        return DM_DEVICE_ID_EMPTY;
    }
    
    if (deviceId.length() < DM_MIN_UDID_LEN || deviceId.length() > DM_MAX_UDID_LEN) {
        LOGE("DeviceId length is invalid, len=%{public}zu", deviceId.length());
        return DM_DEVICE_ID_INVALID_LENGTH;
    }
    
    for (size_t i = 0; i < deviceId.length(); i++) {
        char c = deviceId[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != ':') {
            LOGE("DeviceId format is invalid, contains illegal character");
            return DM_DEVICE_ID_INVALID_FORMAT;
        }
    }
    
    return DM_DEVICE_ID_VALID;
}

bool DmSecureUtils::IsValidAccountId(const std::string &accountId)
{
    if (accountId.empty()) {
        return false;
    }
    
    if (accountId.length() > DM_MAX_ACCOUNT_ID_LEN) {
        LOGE("AccountId length exceeds max limit, len=%{public}zu", accountId.length());
        return false;
    }
    
    return true;
}

bool DmSecureUtils::IsValidUserId(int32_t userId)
{
    if (userId < 0) {
        LOGE("UserId is invalid, userId=%{public}d", userId);
        return false;
    }
    return true;
}

bool DmSecureUtils::IsValidTokenId(int64_t tokenId)
{
    if (tokenId < 0) {
        LOGE("TokenId is invalid, tokenId=%{public}" PRId64, tokenId);
        return false;
    }
    return true;
}

bool DmSecureUtils::IsValidBindType(uint32_t bindType)
{
    if (bindType == INVALIED_TYPE) {
        return false;
    }
    if (bindType != DM_IDENTICAL_ACCOUNT && bindType != DM_SHARE &&
        bindType != DM_POINT_TO_POINT && bindType != DM_ACROSS_ACCOUNT) {
        LOGE("BindType is invalid, bindType=%{public}u", bindType);
        return false;
    }
    return true;
}

bool DmSecureUtils::IsValidBindLevel(uint32_t bindLevel)
{
    if (bindLevel == INVALIED_TYPE) {
        return false;
    }
    if (bindLevel != USER && bindLevel != SERVICE && bindLevel != APP) {
        LOGE("BindLevel is invalid, bindLevel=%{public}u", bindLevel);
        return false;
    }
    return true;
}

IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector);

void PrintProfile(const AccessControlProfile &profile)
{
    uint32_t bindType = profile.GetBindType();
    uint32_t bindLevel = profile.GetBindLevel();

    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();
    std::string acerAccountId = profile.GetAccesser().GetAccesserAccountId();
    std::string acerPkgName = profile.GetAccesser().GetAccesserBundleName();
    std::string acerCredId = profile.GetAccesser().GetAccesserCredentialIdStr();
    int32_t acerSkId = profile.GetAccesser().GetAccesserSessionKeyId();
    int64_t acerTokenId = profile.GetAccesser().GetAccesserTokenId();

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    std::string aceeAccountId = profile.GetAccessee().GetAccesseeAccountId();
    std::string aceePkgName = profile.GetAccessee().GetAccesseeBundleName();
    std::string aceeCredId = profile.GetAccessee().GetAccesseeCredentialIdStr();
    int32_t aceeSkId = profile.GetAccessee().GetAccesseeSessionKeyId();
    int64_t aceeTokenId = profile.GetAccessee().GetAccesseeTokenId();

    LOGI("bindType %{public}d, bindLevel %{public}d, status %{public}d, acerDeviceId %{public}s, acerUserId %{public}d,"
        "acerAccountId %{public}s, acerPkgName %{public}s, acerCredId %{public}s,"
        "acerSkId %{public}d, aceeDeviceId %{public}s, aceeUserId %{public}d, aceeAccountId %{public}s,"
        "aceePkgName %{public}s, aceeCredId %{public}s, aceeSkId %{public}d,"
        "acerTokenId %{public}s, aceeTokenId %{public}s.",
        bindType, bindLevel, profile.GetStatus(), GetAnonyString(acerDeviceId).c_str(), acerUserId,
        GetAnonyString(acerAccountId).c_str(), acerPkgName.c_str(), GetAnonyString(acerCredId).c_str(), acerSkId,
        GetAnonyString(aceeDeviceId).c_str(), aceeUserId, GetAnonyString(aceeAccountId).c_str(),
        aceePkgName.c_str(), GetAnonyString(aceeCredId).c_str(), aceeSkId,
        GetAnonyInt64(acerTokenId).c_str(), GetAnonyInt64(aceeTokenId).c_str());
}

std::string GetLocalDeviceId()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    if (ret != DM_OK) {
        LOGE("GetDevUdid failed, ret=%{public}d", ret);
        DmSecureUtils::SecureClearBuffer(localDeviceId, DEVICE_UUID_LENGTH);
        return "";
    }
    
    std::string deviceId(localDeviceId);
    
    DmDeviceIdValidationResult validationResult = DmSecureUtils::ValidateDeviceId(deviceId);
    if (validationResult != DM_DEVICE_ID_VALID) {
        LOGE("Local deviceId validation failed, result=%{public}d", validationResult);
        DmSecureUtils::SecureClearBuffer(localDeviceId, DEVICE_UUID_LENGTH);
        DmSecureUtils::SecureClearString(deviceId);
        return "";
    }
    
    DmSecureUtils::SecureClearBuffer(localDeviceId, DEVICE_UUID_LENGTH);
    return deviceId;
}

DM_EXPORT int32_t DeviceProfileConnector::GetVersionByExtra(std::string &extraInfo, std::string &dmVersion)
{
    if (extraInfo.empty()) {
        LOGE("extraInfo is empty");
        return ERR_DM_FAILED;
    }
    JsonObject extraInfoJson(extraInfo);
    if (extraInfoJson.IsDiscarded()) {
        LOGE("extraInfoJson error");
        return ERR_DM_FAILED;
    }
    if (!extraInfoJson[TAG_DMVERSION].IsString()) {
        LOGE("PARAM_KEY_OS_VERSION error");
        return ERR_DM_FAILED;
    }
    dmVersion = extraInfoJson[TAG_DMVERSION].Get<std::string>();
    if (dmVersion.empty()) {
        LOGE("dmVersion is empty after extraction");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT void DeviceProfileConnector::GetAllVerionAclMap(DistributedDeviceProfile::AccessControlProfile &acl,
    std::map<std::string, std::vector<std::string>> &aclMap, std::string dmVersion)
{
    if (acl.GetAccessControlId() <= 0) {
        LOGE("Invalid acl id");
        return;
    }
    
    std::vector<std::string> needGenVersions = {};
    if (dmVersion.empty()) {
        for (int32_t idx = 0; idx < DM_SUPPORT_ACL_AGING_VERSION_NUM; idx++) {
            needGenVersions.push_back(DM_SUPPORT_ACL_AGING_VERSIONS[idx]);
        }
    } else if (std::find(DM_SUPPORT_ACL_AGING_VERSIONS.begin(), DM_SUPPORT_ACL_AGING_VERSIONS.end(), dmVersion) !=
        DM_SUPPORT_ACL_AGING_VERSIONS.end()) {
        needGenVersions.push_back(dmVersion);
    } else {
        LOGE("dmVersion invalid, %{public}s", dmVersion.c_str());
        return;
    }

    for (auto const &version : needGenVersions) {
        GenerateAclHash(acl, aclMap, version);
    }
}

void DeviceProfileConnector::GenerateAclHash(DistributedDeviceProfile::AccessControlProfile &acl,
    std::map<std::string, std::vector<std::string>> &aclMap, const std::string &dmVersion)
{
    if (dmVersion.empty()) {
        LOGE("dmVersion is empty");
        return;
    }
    
    int32_t versionNum = 0;
    if (!GetVersionNumber(dmVersion, versionNum)) {
        LOGE("GetAllVerionAclMap GetVersionNumber error");
        return;
    }
    
    std::string aclStr;
    switch (versionNum) {
        case DM_VERSION_INT_5_1_0:
            aclStr = AccessToStr(acl);
            break;
        default:
            LOGE("versionNum is invaild, ver: %{public}d", versionNum);
            break;
    }
    
    if (aclStr.empty()) {
        LOGE("aclStr is empty");
        return;
    }
    std::string aclHash = Crypto::Sha256(aclStr);
    DmSecureUtils::SecureClearString(aclStr);
    
    auto iter = aclMap.find(dmVersion);
    if (iter != aclMap.end()) {
        aclMap[dmVersion].push_back(aclHash);
    } else {
        std::vector<std::string> aclStrVec;
        aclStrVec.push_back(aclHash);
        aclMap[dmVersion] = aclStrVec;
    }
}
} // namespace DistributedHardware
} // namespace OHOS