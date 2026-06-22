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

DM_EXPORT int32_t DeviceProfileConnector::GetAclListHashStr(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclListHash, std::string dmVersion)
{
    if (DmSecureUtils::ValidateDeviceId(localDevUserInfo.deviceId) != DM_DEVICE_ID_VALID) {
        LOGE("localDevUserInfo deviceId is invalid");
        return ERR_DM_FAILED;
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteDevUserInfo.deviceId) != DM_DEVICE_ID_VALID) {
        LOGE("remoteDevUserInfo deviceId is invalid");
        return ERR_DM_FAILED;
    }
    
    if (!DmSecureUtils::IsValidUserId(localDevUserInfo.userId)) {
        LOGE("localDevUserInfo userId is invalid");
        return ERR_DM_FAILED;
    }
    
    if (!DmSecureUtils::IsValidUserId(remoteDevUserInfo.userId)) {
        LOGE("remoteDevUserInfo userId is invalid");
        return ERR_DM_FAILED;
    }
    
    std::map<std::string, std::vector<std::string>> aclMap;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    
    for (auto &item : profiles) {
        if (item.GetAccesser().GetAccesserDeviceId() == localDevUserInfo.deviceId &&
            item.GetAccesser().GetAccesserUserId() == localDevUserInfo.userId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteDevUserInfo.deviceId &&
            item.GetAccessee().GetAccesseeUserId() == remoteDevUserInfo.userId) {
            GetAllVerionAclMap(item, aclMap, dmVersion);
            continue;
        }
        if (item.GetAccesser().GetAccesserDeviceId() == remoteDevUserInfo.deviceId &&
            item.GetAccesser().GetAccesserUserId() == remoteDevUserInfo.userId &&
            item.GetAccessee().GetAccesseeDeviceId() == localDevUserInfo.deviceId &&
            item.GetAccessee().GetAccesseeUserId() == localDevUserInfo.userId) {
            GetAllVerionAclMap(item, aclMap, dmVersion);
            continue;
        }
    }
    
    if (aclMap.empty()) {
        LOGI("acl list is empty");
    }
    
    std::vector<AclHashItem> aclStrVec;
    for (auto &item : aclMap) {
        aclStrVec.push_back({item.first, item.second});
    }
    
    JsonObject allAclObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    AclHashVecToJson(allAclObj, aclStrVec);
    aclListHash = allAclObj.Dump();
    
    return DM_OK;
}

DM_EXPORT void DeviceProfileConnector::AclHashItemToJson(JsonItemObject &itemObject, const AclHashItem &value)
{
    if (value.version.empty()) {
        LOGE("AclHashItem version is empty");
        return;
    }
    
    itemObject[TAG_ACL_HASH_KEY_VERSION] = value.version;
    JsonObject hashList(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (const auto &val : value.aclHashList) {
        if (!val.empty()) {
            hashList.PushBack(val);
        }
    }
    itemObject[TAG_ACL_HASH_KEY_ACLHASHLIST] = hashList.Dump();
}

DM_EXPORT void DeviceProfileConnector::AclHashVecToJson(JsonItemObject &itemObject,
    const std::vector<AclHashItem> &values)
{
    for (const auto &val : values) {
        JsonObject object;
        AclHashItemToJson(object, val);
        itemObject.PushBack(object.Dump());
    }
}

DM_EXPORT void DeviceProfileConnector::AclHashItemFromJson(const JsonItemObject &itemObject, AclHashItem &value)
{
    if (itemObject.IsDiscarded() || !IsString(itemObject, TAG_ACL_HASH_KEY_VERSION) ||
        !IsString(itemObject, TAG_ACL_HASH_KEY_ACLHASHLIST)) {
        LOGE("Invalid JSON structure for ACL hash item");
        return;
    }
    value.version = itemObject[TAG_ACL_HASH_KEY_VERSION].Get<std::string>();
    std::string hashListStr = itemObject[TAG_ACL_HASH_KEY_ACLHASHLIST].Get<std::string>();
    if (hashListStr.empty()) {
        LOGE("hashListStr is empty");
        return;
    }
    JsonObject hashList;
    hashList.Parse(hashListStr);
    for (auto const &item : hashList.Items()) {
        if (item.IsDiscarded() || !item.IsString()) {
            LOGE("ACL hash list contains invalid element");
            continue;
        }
        std::string hashItem = item.Get<std::string>();
        if (!hashItem.empty()) {
            value.aclHashList.push_back(hashItem);
        }
    }
}

DM_EXPORT void DeviceProfileConnector::AclHashVecFromJson(const JsonItemObject &itemObject,
    std::vector<AclHashItem> &values)
{
    for (auto const &item : itemObject.Items()) {
        JsonObject object;
        AclHashItem aclItem;
        if (item.IsDiscarded() || !item.IsString()) {
            LOGE("ItemObject contains invalid element");
            continue;
        }
        std::string itemStr = item.Get<std::string>();
        if (itemStr.empty()) {
            continue;
        }
        object.Parse(itemStr);
        AclHashItemFromJson(object, aclItem);
        if (!aclItem.version.empty()) {
            values.push_back(aclItem);
        }
    }
}

DM_EXPORT bool DeviceProfileConnector::ChecksumAcl(DistributedDeviceProfile::AccessControlProfile &acl,
    std::vector<std::string> &acLStrList)
{
    if (acLStrList.empty()) {
        LOGE("acLStrList is empty");
        return false;
    }
    
    std::string aclStr = AccessToStr(acl);
    if (aclStr.empty()) {
        LOGE("aclStr is empty");
        return false;
    }
    
    std::string aclHash = Crypto::Sha256(aclStr);
    DmSecureUtils::SecureClearString(aclStr);
    
    auto aclIter = find(acLStrList.begin(), acLStrList.end(), aclHash);
    return (aclIter != acLStrList.end());
}

DM_EXPORT std::string DeviceProfileConnector::AccessToStr(DistributedDeviceProfile::AccessControlProfile acl)
{
    std::string aclStr = "";
    
    int64_t aclId = acl.GetAccessControlId();
    if (aclId <= 0) {
        LOGE("Invalid accessControlId: %{public}" PRId64, aclId);
        return aclStr;
    }
    
    uint32_t bindType = acl.GetBindType();
    uint32_t bindLevel = acl.GetBindLevel();
    
    if (!DmSecureUtils::IsValidBindType(bindType) && bindType != DM_LNN) {
        LOGE("Invalid bindType: %{public}u", bindType);
        return aclStr;
    }
    
    if (!DmSecureUtils::IsValidBindLevel(bindLevel)) {
        LOGE("Invalid bindLevel: %{public}u", bindLevel);
        return aclStr;
    }
    
    DistributedDeviceProfile::Accesser accesser = acl.GetAccesser();
    DistributedDeviceProfile::Accessee accessee = acl.GetAccessee();
    
    if (DmSecureUtils::ValidateDeviceId(accesser.GetAccesserDeviceId()) != DM_DEVICE_ID_VALID) {
        LOGE("Accesser deviceId is invalid");
        return aclStr;
    }
    
    if (DmSecureUtils::ValidateDeviceId(accessee.GetAccesseeDeviceId()) != DM_DEVICE_ID_VALID) {
        LOGE("Accessee deviceId is invalid");
        return aclStr;
    }
    
    aclStr += accesser.GetAccesserDeviceId();
    aclStr += std::to_string(accesser.GetAccesserUserId());
    aclStr += accesser.GetAccesserAccountId();
    aclStr += std::to_string(accesser.GetAccesserTokenId());
    aclStr += accesser.GetAccesserBundleName();
    aclStr += accesser.GetAccesserHapSignature();
    aclStr += std::to_string(accesser.GetAccesserBindLevel());
    aclStr += accesser.GetAccesserCredentialIdStr();
    aclStr += std::to_string(accesser.GetAccesserStatus());
    aclStr += std::to_string(accesser.GetAccesserSKTimeStamp());

    aclStr += accessee.GetAccesseeDeviceId();
    aclStr += std::to_string(accessee.GetAccesseeUserId());
    aclStr += accessee.GetAccesseeAccountId();
    aclStr += std::to_string(accessee.GetAccesseeTokenId());
    aclStr += accessee.GetAccesseeBundleName();
    aclStr += accessee.GetAccesseeHapSignature();
    aclStr += std::to_string(accessee.GetAccesseeBindLevel());
    aclStr += accessee.GetAccesseeCredentialIdStr();
    aclStr += std::to_string(accessee.GetAccesseeStatus());
    aclStr += std::to_string(accessee.GetAccesseeSKTimeStamp());
    
    return aclStr;
}

DM_EXPORT std::vector<DistributedDeviceProfile::AccessControlProfile> DeviceProfileConnector::GetAclList(
    const std::string localUdid, int32_t localUserId, const std::string remoteUdid, int32_t remoteUserId)
{
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return {};
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return {};
    }
    
    if (!DmSecureUtils::IsValidUserId(localUserId)) {
        LOGE("localUserId is invalid");
        return {};
    }
    
    if (!DmSecureUtils::IsValidUserId(remoteUserId)) {
        LOGE("remoteUserId is invalid");
        return {};
    }
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    std::vector<DistributedDeviceProfile::AccessControlProfile> aclList;
    
    for (auto &item : profiles) {
        if (item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserUserId() == localUserId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid &&
            item.GetAccessee().GetAccesseeUserId() == remoteUserId) {
            aclList.push_back(item);
            continue;
        }
        if (item.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
            item.GetAccesser().GetAccesserUserId() == remoteUserId &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeUserId() == localUserId) {
            aclList.push_back(item);
            continue;
        }
    }
    
    if (aclList.empty()) {
        LOGI("acl is empty");
    }
    
    return aclList;
}

DM_EXPORT std::string DeviceProfileConnector::IsAuthNewVersion(int32_t bindLevel, std::string localUdid,
    std::string remoteUdid, int32_t tokenId, int32_t userId)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s, bindLevel %{public}d.",
        GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), bindLevel);
    
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return "";
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return "";
    }
    
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return "";
    }
    
    if (static_cast<uint32_t>(bindLevel) > APP || static_cast<uint32_t>(bindLevel) < USER) {
        LOGE("Invalied bindlevel bindLevel=%{public}u.", bindLevel);
        return "";
    }
    
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(localUdid, userId);
    if (profiles.empty()) {
        LOGE("Acl is empty.");
        return "";
    }
    
    switch (bindLevel) {
        case APP:
        case SERVICE:
            return GetAppServiceAuthVersionInfo(localUdid, remoteUdid, tokenId, userId, profiles);
        case USER:
            return GetDeviceAuthVersionInfo(localUdid, remoteUdid, profiles);
        default:
            break;
    }
    return "";
}

std::string DeviceProfileConnector::GetAppServiceAuthVersionInfo(std::string localUdid, std::string remoteUdid,
    int32_t tokenId, int32_t userId, std::vector<DistributedDeviceProfile::AccessControlProfile> profiles)
{
    if (profiles.empty()) {
        LOGE("profiles is empty");
        return "";
    }
    
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return "";
    }
    
    if (tokenId < 0) {
        LOGE("tokenId is invalid");
        return "";
    }
    
    for (auto &item : profiles) {
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT ||
            (item.GetBindLevel() != APP && item.GetBindLevel() != SERVICE)) {
            continue;
        }
        
        int32_t acerUserId = item.GetAccesser().GetAccesserUserId();
        int32_t aceeUserId = item.GetAccessee().GetAccesseeUserId();
        std::string acerDeviceId = item.GetAccesser().GetAccesserDeviceId();
        std::string aceeDeviceId = item.GetAccessee().GetAccesseeDeviceId();
        int32_t acerTokenId = static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId());
        int32_t aceeTokenId = static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId());
        
        if (acerUserId == userId && acerTokenId == tokenId &&
            acerDeviceId == localUdid && aceeDeviceId == remoteUdid) {
            return item.GetAccesser().GetAccesserExtraData();
        }
        
        if (aceeUserId == userId && aceeTokenId == tokenId &&
            aceeDeviceId == localUdid && acerDeviceId == remoteUdid) {
            return item.GetAccessee().GetAccesseeExtraData();
        }
    }
    
    return "";
}

std::string DeviceProfileConnector::GetDeviceAuthVersionInfo(std::string localUdid, std::string remoteUdid,
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles)
{
    if (profiles.empty()) {
        LOGE("profiles is empty");
        return "";
    }
    for (auto &item : profiles) {
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT || item.GetBindLevel() != USER || IsLnnAcl(item)) {
            continue;
        }
        std::string acerDeviceId = item.GetAccesser().GetAccesserDeviceId();
        std::string aceeDeviceId = item.GetAccessee().GetAccesseeDeviceId();
        if (acerDeviceId == localUdid && aceeDeviceId == remoteUdid) {
            return item.GetAccesser().GetAccesserExtraData();
        }
        if (aceeDeviceId == localUdid && acerDeviceId == remoteUdid) {
            return item.GetAccessee().GetAccesseeExtraData();
        }
    }
    return "";
}

std::string DeviceProfileConnector::GetAclVersionInfo(const std::string localUdid, const std::string remoteUdid,
    const DistributedDeviceProfile::AccessControlProfile &acl)
{
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return "";
    }
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return "";
    }
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    if (acerDeviceId == localUdid && aceeDeviceId == remoteUdid) {
        return acl.GetAccesser().GetAccesserExtraData();
    }
    if (aceeDeviceId == localUdid && acerDeviceId == remoteUdid) {
        return acl.GetAccessee().GetAccesseeExtraData();
    }
    return "";
}

DM_EXPORT DmOfflineParam DeviceProfileConnector::FilterNeedDeleteACL(const std::string &localDeviceId,
    uint32_t localTokenId, const std::string &remoteDeviceId, const std::string &extra)
{
    LOGI("localDeviceId %{public}s, remoteDeviceId %{public}s",
        GetAnonyString(localDeviceId).c_str(), GetAnonyString(remoteDeviceId).c_str());
    
    DmOfflineParam offlineParam = {};
    
    if (DmSecureUtils::ValidateDeviceId(localDeviceId) != DM_DEVICE_ID_VALID) {
        LOGE("localDeviceId is invalid");
        return offlineParam;
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteDeviceId) != DM_DEVICE_ID_VALID) {
        LOGE("remoteDeviceId is invalid");
        return offlineParam;
    }
    
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return offlineParam;
    }
    
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(localDeviceId, userId,
        remoteDeviceId);
    if (profiles.empty()) {
        LOGE("Acl is empty.");
        return offlineParam;
    }

    FilterNeedDeleteACLInfos(profiles, localDeviceId, localTokenId, remoteDeviceId, extra, offlineParam);
    return offlineParam;
}

void DeviceProfileConnector::CheckLastLnnAcl(const std::string &localDeviceId, int32_t localUserId,
    const std::string &remoteDeviceId, DmOfflineParam &offlineParam,
    std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles)
{
    LOGI("profiles size: %{public}zu", profiles.size());
    
    if (DmSecureUtils::ValidateDeviceId(localDeviceId) != DM_DEVICE_ID_VALID) {
        LOGE("localDeviceId is invalid");
        return;
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteDeviceId) != DM_DEVICE_ID_VALID) {
        LOGE("remoteDeviceId is invalid");
        return;
    }
    
    if (!DmSecureUtils::IsValidUserId(localUserId)) {
        LOGE("localUserId is invalid");
        return;
    }
    
    if (profiles.size() == 1 && IsLnnAcl(profiles[0])) {
        if (profiles[0].GetAccesser().GetAccesserDeviceId() == localDeviceId) {
            CacheAcerAclId(profiles[0], offlineParam.needDelAclInfos);
        }
        if (profiles[0].GetAccessee().GetAccesseeDeviceId() == localDeviceId) {
            CacheAceeAclId(profiles[0], offlineParam.needDelAclInfos);
        }
        offlineParam.hasLnnAcl = true;
    }
}

void DeviceProfileConnector::DeleteCacheAcl(std::vector<int64_t> delAclIdVec,
    std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles)
{
    if (delAclIdVec.empty() || profiles.empty()) {
        LOGI("delAclIdVec or profiles is empty");
        return;
    }
    
    for (auto delAclId : delAclIdVec) {
        if (delAclId <= 0) {
            LOGE("Invalid delAclId: %{public}" PRId64, delAclId);
            continue;
        }
        
        for (auto item = profiles.begin(); item != profiles.end();) {
            if (delAclId == item->GetAccessControlId()) {
                item = profiles.erase(item);
            } else {
                item++;
            }
        }
    }
}

bool DeviceProfileConnector::FindTargetAcl(const DistributedDeviceProfile::AccessControlProfile &acl,
    const std::string &localUdid, const uint32_t localTokenId,
    const std::string &remoteUdid, const uint32_t peerTokenId,
    DmOfflineParam &offlineParam)
{
    bool isMatch = false;
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return false;
    }
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return false;
    }
    int64_t acerTokenId = acl.GetAccesser().GetAccesserTokenId();
    int64_t aceeTokenId = acl.GetAccessee().GetAccesseeTokenId();
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    if ((acerTokenId == static_cast<int64_t>(localTokenId)) &&
        (acerDeviceId == localUdid) && (aceeDeviceId == remoteUdid) &&
        (peerTokenId == 0 || (peerTokenId != 0 && aceeTokenId == static_cast<int64_t>(peerTokenId)))) {
        ProcessInfo processInfo;
        processInfo.pkgName = acl.GetAccesser().GetAccesserBundleName();
        processInfo.userId = acl.GetAccesser().GetAccesserUserId();
        processInfo.tokenId = acl.GetAccesser().GetAccesserTokenId();
        if (!DmSecureUtils::IsValidUserId(processInfo.userId)) {
            LOGE("Invalid userId in ACL");
            return false;
        }
        if ((acl.GetBindLevel() == USER || acl.GetBindType() == DM_IDENTICAL_ACCOUNT)) {
            offlineParam.hasUserAcl = true;
            processInfo.tokenId = 0;
        }
        offlineParam.processVec.push_back(processInfo);
        CacheAcerAclId(acl, offlineParam.needDelAclInfos);
        LOGI("Src del acl aclId: %{public}" PRId64 ", localUdid: %{public}s, remoteUdid: %{public}s"
             ", bindType: %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
             GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
             acl.GetBindLevel());
        isMatch = true;
    }
    return isMatch;
}

bool DeviceProfileConnector::FindLnnAcl(const DistributedDeviceProfile::AccessControlProfile &acl,
    const std::string &localUdid, const std::string &remoteUdid, DmOfflineParam &offlineParam)
{
    bool isMatch = false;
    
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return false;
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return false;
    }
    
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    
    if (IsLnnAcl(acl) && acl.GetTrustDeviceId() == remoteUdid) {
        if (acerDeviceId == localUdid && aceeDeviceId == remoteUdid) {
            LOGI("Src lnn acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
                 ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
                 GetAnonyString(localUdid).c_str(),
                 GetAnonyString(remoteUdid).c_str(), acl.GetBindType(), acl.GetBindLevel());
            CacheAcerAclId(acl, offlineParam.allLnnAclInfos);
        }

        if (aceeDeviceId == localUdid && acerDeviceId == remoteUdid) {
            LOGI("Sink lnn acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
                 ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
                 GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
                 acl.GetBindLevel());
            CacheAceeAclId(acl, offlineParam.allLnnAclInfos);
        }
        isMatch = true;
    }
    
    return isMatch;
}

bool DeviceProfileConnector::FindUserAcl(const DistributedDeviceProfile::AccessControlProfile &acl,
    const std::string &localUdid, const std::string &remoteUdid, DmOfflineParam &offlineParam)
{
    bool isMatch = false;
    
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return false;
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return false;
    }
    
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    
    if (acl.GetBindLevel() == USER || acl.GetBindType() == DM_IDENTICAL_ACCOUNT) {
        if (acerDeviceId == localUdid && aceeDeviceId == remoteUdid) {
            LOGI("Src User acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
                 ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
                 GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
                 acl.GetBindLevel());
            CacheAcerAclId(acl, offlineParam.allUserAclInfos);
        }

        if (aceeDeviceId == localUdid && acerDeviceId == remoteUdid) {
            LOGI("Sink User acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
                 ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
                 GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
                 acl.GetBindLevel());
            CacheAceeAclId(acl, offlineParam.allUserAclInfos);
        }
        isMatch = true;
    }
    
    return isMatch;
}

bool DeviceProfileConnector::FindLeftAcl(const DistributedDeviceProfile::AccessControlProfile &acl,
    const std::string &localUdid, const std::string &remoteUdid, DmOfflineParam &offlineParam)
{
    bool isMatch = false;
    
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return false;
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return false;
    }
    
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    
    if (acl.GetBindLevel() == SERVICE || acl.GetBindLevel() == APP) {
        if (acerDeviceId == localUdid && aceeDeviceId == remoteUdid) {
            LOGI("Src Left acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
                 ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
                 GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
                 acl.GetBindLevel());
            CacheAcerAclId(acl, offlineParam.allLeftAppOrSvrAclInfos);
        }

        if (aceeDeviceId == localUdid && acerDeviceId == remoteUdid) {
            LOGI("Sink Left acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
                 ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
                 GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
                 acl.GetBindLevel());
            CacheAceeAclId(acl, offlineParam.allLeftAppOrSvrAclInfos);
        }
        isMatch = true;
    }
    
    return isMatch;
}

void DeviceProfileConnector::FilterNeedDeleteACLInfos(
    std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles,
    const std::string &localUdid, const uint32_t localTokenId,
    const std::string &remoteUdid, const std::string &extra, DmOfflineParam &offlineParam)
{
    if (profiles.empty()) {
        LOGE("profiles is empty");
        return;
    }
    
    if (DmSecureUtils::ValidateDeviceId(localUdid) != DM_DEVICE_ID_VALID) {
        LOGE("localUdid is invalid");
        return;
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteUdid) != DM_DEVICE_ID_VALID) {
        LOGE("remoteUdid is invalid");
        return;
    }
    
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    JsonStrHandle::GetInstance().GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }

        if (FindTargetAcl(item, localUdid, localTokenId, remoteUdid, peerTokenId, offlineParam)) {
            continue;
        }

        if (FindLnnAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        if (FindUserAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        if (FindLeftAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }
    }
}

DM_EXPORT std::vector<AccessControlProfile> DeviceProfileConnector::GetAccessControlProfile()
{
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return {};
    }
    return GetAccessControlProfileByUserId(userId);
}

std::vector<AccessControlProfile> DeviceProfileConnector::GetAccessControlProfileByUserId(int32_t userId)
{
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return {};
    }
    
    std::vector<AccessControlProfile> profiles;
    std::map<std::string, std::string> queryParams;
    queryParams[USERID] = std::to_string(userId);
    
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfileByUserId failed.");
    }
    
    return profiles;
}

std::vector<AccessControlProfile> DeviceProfileConnector::GetAclProfileByDeviceIdAndUserId(const std::string &deviceId,
    int32_t userId)
{
    if (DmSecureUtils::ValidateDeviceId(deviceId) != DM_DEVICE_ID_VALID) {
        LOGE("deviceId is invalid");
        return {};
    }
    
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return {};
    }
    
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(userId);
    std::vector<AccessControlProfile> aclProfileVec;
    
    for (auto &item : profiles) {
        if ((item.GetAccesser().GetAccesserDeviceId() == deviceId &&
             item.GetAccesser().GetAccesserUserId() == userId) ||
            (item.GetAccessee().GetAccesseeDeviceId() == deviceId &&
             item.GetAccessee().GetAccesseeUserId() == userId)) {
            aclProfileVec.push_back(item);
        }
    }
    
    return aclProfileVec;
}

std::vector<AccessControlProfile> DeviceProfileConnector::GetAclProfileByDeviceIdAndUserId(const std::string &deviceId,
    int32_t userId, const std::string &remoteDeviceId)
{
    if (DmSecureUtils::ValidateDeviceId(deviceId) != DM_DEVICE_ID_VALID) {
        LOGE("deviceId is invalid");
        return {};
    }
    
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return {};
    }
    
    if (DmSecureUtils::ValidateDeviceId(remoteDeviceId) != DM_DEVICE_ID_VALID) {
        LOGE("remoteDeviceId is invalid");
        return {};
    }
    
    std::vector<AccessControlProfile> aclProfileVec;
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    
    for (auto &item : profiles) {
        if ((item.GetAccesser().GetAccesserDeviceId() == deviceId &&
             item.GetAccesser().GetAccesserUserId() == userId &&
             item.GetAccessee().GetAccesseeDeviceId() == remoteDeviceId) ||
            (item.GetAccessee().GetAccesseeDeviceId() == deviceId &&
             item.GetAccessee().GetAccesseeUserId() == userId &&
             item.GetAccesser().GetAccesserDeviceId() == remoteDeviceId)) {
            aclProfileVec.push_back(item);
        }
    }
    
    return aclProfileVec;
}

DM_EXPORT std::unordered_map<std::string, DmAuthForm> DeviceProfileConnector::GetAppTrustDeviceList(
    const std::string &pkgName, const std::string &deviceId)
{
    if (pkgName.empty()) {
        LOGE("pkgName is empty");
        return {};
    }
    
    if (DmSecureUtils::ValidateDeviceId(deviceId) != DM_DEVICE_ID_VALID) {
        LOGE("deviceId is invalid");
        return {};
    }
    
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (!DmSecureUtils::IsValidUserId(userId)) {
        LOGE("userId is invalid");
        return {};
    }
    
    LOGI("localDeviceId: %{public}s, userId: %{public}d", GetAnonyString(deviceId).c_str(), userId);
    
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    std::vector<AccessControlProfile> profilesFilter = {};
    
    for (auto &item : profiles) {
        if (!IsLnnAcl(item) && ((item.GetAccesser().GetAccesserUserId() == userId &&
             item.GetAccesser().GetAccesserDeviceId() == deviceId) ||
            (item.GetAccessee().GetAccesseeUserId() == userId &&
             item.GetAccessee().GetAccesseeDeviceId() == deviceId))) {
            profilesFilter.push_back(item);
        }
    }
    
    return GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
}
} // namespace DistributedHardware
} // namespace OHOS