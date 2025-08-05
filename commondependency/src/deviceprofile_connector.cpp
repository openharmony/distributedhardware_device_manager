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

#include "deviceprofile_connector.h"
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
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;

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
}
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector);
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

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    std::string aceeAccountId = profile.GetAccessee().GetAccesseeAccountId();
    std::string aceePkgName = profile.GetAccessee().GetAccesseeBundleName();
    std::string aceeCredId = profile.GetAccessee().GetAccesseeCredentialIdStr();
    int32_t aceeSkId = profile.GetAccessee().GetAccesseeSessionKeyId();

    LOGI("bindType %{public}d, bindLevel %{public}d, acerDeviceId %{public}s, acerUserId %{public}d,"
        "acerAccountId %{public}s, acerPkgName %{public}s, acerCredId %{public}s,"
        "acerSkId %{public}d, aceeDeviceId %{public}s, aceeUserId %{public}d, aceeAccountId %{public}s,"
        "aceePkgName %{public}s, aceeCredId %{public}s, aceeSkId %{public}d.",
        bindType, bindLevel, GetAnonyString(acerDeviceId).c_str(), acerUserId, GetAnonyString(acerAccountId).c_str(),
        acerPkgName.c_str(), GetAnonyString(acerCredId).c_str(), acerSkId,
        GetAnonyString(aceeDeviceId).c_str(), aceeUserId, GetAnonyString(aceeAccountId).c_str(),
        aceePkgName.c_str(), GetAnonyString(aceeCredId).c_str(), aceeSkId);
}

std::string GetLocalDeviceId()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    return std::string(localDeviceId);
}

DM_EXPORT int32_t DeviceProfileConnector::GetVersionByExtra(std::string &extraInfo, std::string &dmVersion)
{
    JsonObject extraInfoJson(extraInfo);
    if (extraInfoJson.IsDiscarded()) {
        LOGE("GetVersionByExtra extraInfoJson error");
        return ERR_DM_FAILED;
    }
    if (!extraInfoJson[TAG_DMVERSION].IsString()) {
        LOGE("GetVersionByExtra PARAM_KEY_OS_VERSION error");
        return ERR_DM_FAILED;
    }
    dmVersion = extraInfoJson[TAG_DMVERSION].Get<std::string>();
    return DM_OK;
}

DM_EXPORT void DeviceProfileConnector::GetAllVerionAclMap(DistributedDeviceProfile::AccessControlProfile &acl,
    std::map<std::string, std::vector<std::string>> &aclMap, std::string dmVersion)
{
    std::vector<std::string> needGenVersions = {};
    // if not set version, send all support version acl hash
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
        return;
    }
    auto iter = aclMap.find(dmVersion);
    if (iter != aclMap.end()) {
        aclMap[dmVersion].push_back(Crypto::Sha256(aclStr));
    } else {
        std::vector<std::string> aclStrVec;
        aclStrVec.push_back(Crypto::Sha256(aclStr));
        aclMap[dmVersion] = aclStrVec;
    }
}

DM_EXPORT int32_t DeviceProfileConnector::GetAclListHashStr(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclListHash, std::string dmVersion)
{
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
        LOGI("DeviceProfileConnector:: acl list is empty");
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
    itemObject[TAG_ACL_HASH_KEY_VERSION] = value.version;
    JsonObject hashList(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (const auto &val : value.aclHashList) {
        hashList.PushBack(val);
    }
    itemObject[TAG_ACL_HASH_KEY_ACLHASHLIST] = hashList.Dump();
}

DM_EXPORT void DeviceProfileConnector::AclHashVecToJson(
    JsonItemObject &itemObject, const std::vector<AclHashItem> &values)
{
    for (const auto &val : values) {
        JsonObject object;
        AclHashItemToJson(object, val);
        itemObject.PushBack(object.Dump());
    }
}

DM_EXPORT void DeviceProfileConnector::AclHashItemFromJson(const JsonItemObject &itemObject, AclHashItem &value)
{
    value.version = itemObject[TAG_ACL_HASH_KEY_VERSION].Get<std::string>();
    std::string hashListStr = itemObject[TAG_ACL_HASH_KEY_ACLHASHLIST].Get<std::string>();
    JsonObject hashList;
    hashList.Parse(hashListStr);
    for (auto const &item : hashList.Items()) {
        value.aclHashList.push_back(item.Get<std::string>());
    }
}

DM_EXPORT void DeviceProfileConnector::AclHashVecFromJson(const JsonItemObject &itemObject,
    std::vector<AclHashItem> &values)
{
    for (auto const &item : itemObject.Items()) {
        JsonObject object;
        AclHashItem aclItem;
        object.Parse(item.Get<std::string>());
        AclHashItemFromJson(object, aclItem);
        values.push_back(aclItem);
    }
}

DM_EXPORT bool DeviceProfileConnector::ChecksumAcl(DistributedDeviceProfile::AccessControlProfile &acl,
    std::vector<std::string> &acLStrList)
{
    std::string aclStr = AccessToStr(acl);
    auto aclIter = find(acLStrList.begin(), acLStrList.end(), Crypto::Sha256(aclStr));
    return (aclIter != acLStrList.end());
}

DM_EXPORT std::string DeviceProfileConnector::AccessToStr(DistributedDeviceProfile::AccessControlProfile acl)
{
    std::string aclStr = "";
    DistributedDeviceProfile::Accesser accesser = acl.GetAccesser();
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

    DistributedDeviceProfile::Accessee accessee = acl.GetAccessee();
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
        LOGI("DeviceProfileConnector::GetAclList acl is empty");
    }
    return aclList;
}

DM_EXPORT std::string DeviceProfileConnector::IsAuthNewVersion(int32_t bindLevel, std::string localUdid,
    std::string remoteUdid, int32_t tokenId, int32_t userId)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s, bindLevel %{public}d.",
        GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), bindLevel);
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
DM_EXPORT DmOfflineParam DeviceProfileConnector::FilterNeedDeleteACL(const std::string &localDeviceId,
    uint32_t localTokenId, const std::string &remoteDeviceId, const std::string &extra)
{
    LOGI("localDeviceId %{public}s, remoteDeviceId %{public}s",
        GetAnonyString(localDeviceId).c_str(), GetAnonyString(remoteDeviceId).c_str());
    DmOfflineParam offlineParam;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
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
    for (auto delAclId : delAclIdVec) {
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
    int64_t acerTokenId = acl.GetAccesser().GetAccesserTokenId();
    int64_t aceeTokenId = acl.GetAccessee().GetAccesseeTokenId();
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    // Process target match acl which need delete
    if ((acerTokenId == static_cast<int64_t>(localTokenId)) &&
        (acerDeviceId == localUdid) && (aceeDeviceId == remoteUdid) &&
        (peerTokenId == 0 || (peerTokenId != 0 && aceeTokenId == static_cast<int64_t>(peerTokenId)))) {
        ProcessInfo processInfo;
        processInfo.pkgName = acl.GetAccesser().GetAccesserBundleName();
        processInfo.userId = acl.GetAccesser().GetAccesserUserId();
        offlineParam.processVec.push_back(processInfo);
        CacheAcerAclId(acl, offlineParam.needDelAclInfos);
        LOGI("Src del acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
             ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
             GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
             acl.GetBindLevel());
        isMatch = true;
    }

    if ((aceeTokenId == static_cast<int64_t>(localTokenId)) &&
        (aceeDeviceId == localUdid) && (acerDeviceId == remoteUdid) &&
        (peerTokenId == 0 || (peerTokenId != 0 && acerTokenId == static_cast<int64_t>(peerTokenId)))) {
        ProcessInfo processInfo;
        processInfo.pkgName = acl.GetAccessee().GetAccesseeBundleName();
        processInfo.userId = acl.GetAccessee().GetAccesseeUserId();
        offlineParam.processVec.push_back(processInfo);
        CacheAceeAclId(acl, offlineParam.needDelAclInfos);
        LOGI("Sink del acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
             ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
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
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    // process User Or SameAccount acl
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
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    // process left service/app acl
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
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    JsonStrHandle::GetInstance().GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }

        // First, find need delete acl
        if (FindTargetAcl(item, localUdid, localTokenId, remoteUdid, peerTokenId, offlineParam)) {
            continue;
        }

        // Second, find the LNN acl
        if (FindLnnAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Third, find the user or sameaccount acl, dertermine if report offline
        if (FindUserAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Fourth, find the left service/app acl, determine if del lnn acl
        if (FindLeftAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }
    }
}

DM_EXPORT std::vector<AccessControlProfile> DeviceProfileConnector::GetAccessControlProfile()
{
    std::vector<AccessControlProfile> profiles;
    std::map<std::string, std::string> queryParams;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    queryParams[USERID] = std::to_string(userId);
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
    }
    return profiles;
}

std::vector<AccessControlProfile> DeviceProfileConnector::GetAccessControlProfileByUserId(int32_t userId)
{
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
    std::vector<AccessControlProfile> profiles;
    std::vector<AccessControlProfile> aclProfileVec;
    std::map<std::string, std::string> queryParams;
    queryParams[USERID] = std::to_string(userId);
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
        return aclProfileVec;
    }
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
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
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

std::unordered_map<std::string, DmAuthForm> DeviceProfileConnector::GetAuthFormMap(const std::string &pkgName,
    const std::string &deviceId, const std::vector<DistributedDeviceProfile::AccessControlProfile> &profilesFilter,
    const int32_t &userId)
{
    std::unordered_map<std::string, DmAuthForm> deviceIdMap;
    for (auto &item : profilesFilter) {
        std::string trustDeviceId = item.GetTrustDeviceId();
        LOGI("trustDeviceId: %{public}s, status: %{public}d, acerUserId: %{public}d, aceeUserId: %{public}d",
            GetAnonyString(trustDeviceId).c_str(), item.GetStatus(), item.GetAccesser().GetAccesserUserId(),
            item.GetAccessee().GetAccesseeUserId());
        if (trustDeviceId == deviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        DmDiscoveryInfo discoveryInfo = {pkgName, deviceId};
        int32_t bindType = HandleDmAuthForm(item, discoveryInfo);
        LOGI("The udid %{public}s in ACL authForm is %{public}d.", GetAnonyString(trustDeviceId).c_str(), bindType);
        if (bindType == DmAuthForm::INVALID_TYPE) {
            continue;
        }
        if (deviceIdMap.find(trustDeviceId) == deviceIdMap.end()) {
            if (CheckSinkShareType(item, userId, deviceId, trustDeviceId, bindType)) {
                LOGI("GetAuthFormMap CheckSinkShareType true.");
                continue;
            }
            deviceIdMap[trustDeviceId] = static_cast<DmAuthForm>(bindType);
            continue;
        }
        DmAuthForm authForm = deviceIdMap.at(trustDeviceId);
        if (bindType == authForm) {
            continue;
        }
        uint32_t highestBindType = CheckBindType(trustDeviceId, deviceId);
        if (highestBindType == IDENTICAL_ACCOUNT_TYPE) {
            deviceIdMap[trustDeviceId] = DmAuthForm::IDENTICAL_ACCOUNT;
            continue;
        } else if (highestBindType == SHARE_TYPE) {
            if (CheckSinkShareType(item, userId, deviceId, trustDeviceId, DmAuthForm::SHARE)) {
                continue;
            }
            deviceIdMap[trustDeviceId] = DmAuthForm::SHARE;
            continue;
        } else if (highestBindType == DEVICE_PEER_TO_PEER_TYPE || highestBindType == APP_PEER_TO_PEER_TYPE ||
            highestBindType == SERVICE_PEER_TO_PEER_TYPE) {
            deviceIdMap[trustDeviceId] = DmAuthForm::PEER_TO_PEER;
            continue;
        } else if (highestBindType == APP_ACROSS_ACCOUNT_TYPE ||
            highestBindType == DEVICE_ACROSS_ACCOUNT_TYPE || highestBindType == SERVICE_ACROSS_ACCOUNT_TYPE) {
            deviceIdMap[trustDeviceId] = DmAuthForm::ACROSS_ACCOUNT;
            continue;
        } else {
            LOGE("GetAuthFormMap highestBindType match failed.");
            continue;
        }
    }
    return deviceIdMap;
}

bool DeviceProfileConnector::CheckSinkShareType(const DistributedDeviceProfile::AccessControlProfile &profile,
    const int32_t &userId, const std::string &deviceId, const std::string &trustDeviceId, const int32_t &bindType)
{
    if (profile.GetAccessee().GetAccesseeUserId() == userId &&
        profile.GetAccessee().GetAccesseeDeviceId() == deviceId &&
        profile.GetAccesser().GetAccesserDeviceId() == trustDeviceId &&
        bindType == DmAuthForm::SHARE) {
        return true;
    }
    return false;
}

int32_t DeviceProfileConnector::GetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool &isOnline, int32_t &authForm)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(discoveryInfo.userId);
    std::vector<int32_t> bindTypes;
    for (auto &item : profiles) {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetUdidHash(item.GetTrustDeviceId(), reinterpret_cast<uint8_t *>(deviceIdHash)) != DM_OK) {
            LOGE("get deviceIdHash by deviceId: %{public}s failed.", GetAnonyString(deviceIdHash).c_str());
            continue;
        }
        if (IsLnnAcl(item) || static_cast<std::string>(deviceIdHash) != discoveryInfo.remoteDeviceIdHash ||
            (discoveryInfo.localDeviceId == item.GetAccesser().GetAccesserDeviceId() &&
            discoveryInfo.userId != item.GetAccesser().GetAccesserUserId()) ||
            (discoveryInfo.localDeviceId == item.GetAccessee().GetAccesseeDeviceId() &&
            discoveryInfo.userId != item.GetAccessee().GetAccesseeUserId())) {
            continue;
        }
        int32_t bindType = HandleDmAuthForm(item, discoveryInfo);
        if (bindType == DmAuthForm::INVALID_TYPE) {
            continue;
        }
        bindTypes.push_back(bindType);
    }
    if (std::count(bindTypes.begin(), bindTypes.end(), DmAuthForm::IDENTICAL_ACCOUNT) > 0) {
        isOnline = true;
        authForm = DmAuthForm::IDENTICAL_ACCOUNT;
        LOGI("The found device is identical account device bind type.");
        return DM_OK;
    }
    if (std::count(bindTypes.begin(), bindTypes.end(), DmAuthForm::PEER_TO_PEER) > 0) {
        isOnline = true;
        authForm = DmAuthForm::PEER_TO_PEER;
        LOGI("The found device is peer-to-peer device bind-level.");
        return DM_OK;
    }
    if (std::count(bindTypes.begin(), bindTypes.end(), DmAuthForm::ACROSS_ACCOUNT) > 0) {
        isOnline = true;
        authForm = DmAuthForm::ACROSS_ACCOUNT;
        LOGI("The found device is across-account device bind-level.");
        return DM_OK;
    }
    authForm = DmAuthForm::INVALID_TYPE;
    return DM_OK;
}

bool DeviceProfileConnector::CheckAuthFormProxyTokenId(const std::string pkgName, const std::string &extraStr)
{
    std::vector<int64_t> proxyTokenIdVec = JsonStrHandle::GetInstance().GetProxyTokenIdByExtra(extraStr);
    int64_t callingTokenId = static_cast<int64_t>(IPCSkeleton::GetCallingTokenID());
    for (auto &proxyTokenId : proxyTokenIdVec) {
        std::string proxyBundleName;
        if (AppManager::GetInstance().GetBundleNameByTokenId(proxyTokenId, proxyBundleName) != DM_OK) {
            continue;
        }
        if (callingTokenId == proxyTokenId && pkgName == proxyBundleName) {
            return true;
        }
    }
    return false;
}

int32_t DeviceProfileConnector::CheckAuthForm(DmAuthForm form, AccessControlProfile profiles,
    DmDiscoveryInfo discoveryInfo)
{
    if (IsLnnAcl(profiles)) {
        return DmAuthForm::INVALID_TYPE;
    }
    if (profiles.GetBindLevel() == USER || ((profiles.GetBindLevel() == APP || profiles.GetBindLevel() == SERVICE) &&
        discoveryInfo.pkgname == "")) {
        return form;
    }
    if (profiles.GetBindLevel() == APP || profiles.GetBindLevel() == SERVICE) {
        if ((discoveryInfo.pkgname == profiles.GetAccesser().GetAccesserBundleName() ||
            CheckAuthFormProxyTokenId(discoveryInfo.pkgname, profiles.GetAccesser().GetAccesserExtraData())) &&
            discoveryInfo.localDeviceId == profiles.GetAccesser().GetAccesserDeviceId()) {
            return form;
        }
        if ((discoveryInfo.pkgname == profiles.GetAccessee().GetAccesseeBundleName() ||
            CheckAuthFormProxyTokenId(discoveryInfo.pkgname, profiles.GetAccessee().GetAccesseeExtraData())) &&
            discoveryInfo.localDeviceId == profiles.GetAccessee().GetAccesseeDeviceId()) {
            return form;
        }
    }
    return DmAuthForm::INVALID_TYPE;
}

int32_t DeviceProfileConnector::HandleDmAuthForm(AccessControlProfile profiles, DmDiscoveryInfo discoveryInfo)
{
    if (profiles.GetBindType() == DM_IDENTICAL_ACCOUNT) {
        return DmAuthForm::IDENTICAL_ACCOUNT;
    }
    if (profiles.GetBindType() == DM_POINT_TO_POINT) {
        return CheckAuthForm(DmAuthForm::PEER_TO_PEER, profiles, discoveryInfo);
    }
    if (profiles.GetBindType() == DM_ACROSS_ACCOUNT) {
        return CheckAuthForm(DmAuthForm::PEER_TO_PEER, profiles, discoveryInfo);
    }
    if (profiles.GetBindType() == DM_SHARE) {
        return CheckAuthForm(DmAuthForm::SHARE, profiles, discoveryInfo);
    }
    return DmAuthForm::INVALID_TYPE;
}

DM_EXPORT uint32_t DeviceProfileConnector::CheckBindType(std::string peerUdid,
    std::string localUdid)
{
    std::vector<AccessControlProfile> filterProfiles = GetAclProfileByUserId(localUdid,
        MultipleUserConnector::GetFirstForegroundUserId(), peerUdid);
    LOGI("filterProfiles size is %{public}zu", filterProfiles.size());
    uint32_t highestPriority = INVALIED_TYPE;
    for (auto &item : filterProfiles) {
        if (IsLnnAcl(item) || peerUdid != item.GetTrustDeviceId()) {
            continue;
        }
        uint32_t priority = static_cast<uint32_t>(GetAuthForm(item, peerUdid, localUdid));
        if (priority > highestPriority) {
            highestPriority = priority;
        }
    }
    return highestPriority;
}

int32_t DeviceProfileConnector::GetAuthForm(DistributedDeviceProfile::AccessControlProfile profiles,
    const std::string &trustDev, const std::string &reqDev)
{
    LOGI("BindType %{public}d, bindLevel %{public}d",
        profiles.GetBindType(), profiles.GetBindLevel());
    uint32_t priority = INVALIED_TYPE;
    uint32_t bindType = profiles.GetBindType();

    switch (bindType) {
        case DM_IDENTICAL_ACCOUNT:
            priority = IDENTICAL_ACCOUNT_TYPE;
            break;
        case DM_SHARE:
            priority = SHARE_TYPE;
            break;
        case DM_POINT_TO_POINT:
            if (profiles.GetBindLevel() == USER) {
                priority = DEVICE_PEER_TO_PEER_TYPE;
            }
            if (profiles.GetBindLevel() == SERVICE) {
                priority = SERVICE_PEER_TO_PEER_TYPE;
            }
            if (profiles.GetBindLevel() == APP) {
                priority = APP_PEER_TO_PEER_TYPE;
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profiles.GetBindLevel() == USER) {
                priority = DEVICE_ACROSS_ACCOUNT_TYPE;
            }
            if (profiles.GetBindLevel() == SERVICE) {
                priority = SERVICE_ACROSS_ACCOUNT_TYPE;
            }
            if (profiles.GetBindLevel() == APP) {
                priority = APP_ACROSS_ACCOUNT_TYPE;
            }
            break;
        default:
            LOGE("unknown bind type %{public}d.", bindType);
            break;
    }
    return priority;
}

DM_EXPORT std::vector<int32_t> DeviceProfileConnector::GetBindTypeByPkgName(
    std::string pkgName, std::string requestDeviceId, std::string trustUdid)
{
    LOGI("Start requestDeviceId %{public}s, trustUdid %{public}s.", GetAnonyString(requestDeviceId).c_str(),
        GetAnonyString(trustUdid).c_str());
    std::vector<int32_t> bindTypeVec;
    if (requestDeviceId.empty() || trustUdid.empty() || requestDeviceId == trustUdid) {
        LOGE("Input udid param invalied.");
        return bindTypeVec;
    }
    std::vector<AccessControlProfile> profiles =
        GetAccessControlProfileByUserId(MultipleUserConnector::GetFirstForegroundUserId());
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (IsLnnAcl(item) || trustUdid != item.GetTrustDeviceId() || item.GetStatus() != ACTIVE) {
            continue;
        }
        GetParamBindTypeVec(item, requestDeviceId, bindTypeVec, trustUdid);
    }
    return bindTypeVec;
}

DM_EXPORT uint64_t DeviceProfileConnector::GetTokenIdByNameAndDeviceId(std::string extra, std::string requestDeviceId)
{
    uint64_t peerTokenId = 0;
    std::string pkgName = "";
    JsonStrHandle::GetInstance().GetPeerAppInfoParseExtra(extra, peerTokenId, pkgName);
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    for (auto &item : profiles) {
        if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == requestDeviceId) {
            peerTokenId = static_cast<uint64_t>(item.GetAccesser().GetAccesserTokenId());
            break;
        }
        if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == requestDeviceId) {
            peerTokenId = static_cast<uint64_t>(item.GetAccessee().GetAccesseeTokenId());
            break;
        }
    }
    return peerTokenId;
}

void DeviceProfileConnector::GetParamBindTypeVec(AccessControlProfile profile, std::string requestDeviceId,
    std::vector<int32_t> &bindTypeVec, std::string trustUdid)
{
    if (!(profile.GetAccesser().GetAccesserDeviceId() == trustUdid &&
        profile.GetAccessee().GetAccesseeDeviceId() == requestDeviceId) &&
        !(profile.GetAccessee().GetAccesseeDeviceId() == trustUdid &&
        profile.GetAccesser().GetAccesserDeviceId() == requestDeviceId)) {
        LOGE("input udid param invalied.");
        return;
    }
    uint32_t bindType = profile.GetBindType();
    switch (bindType) {
        case DM_IDENTICAL_ACCOUNT:
            bindTypeVec.push_back(IDENTICAL_ACCOUNT_TYPE);
            break;
        case DM_POINT_TO_POINT:
            if (profile.GetBindLevel() == USER) {
                bindTypeVec.push_back(DEVICE_PEER_TO_PEER_TYPE);
            }
            if (profile.GetBindLevel() == APP) {
                bindTypeVec.push_back(APP_PEER_TO_PEER_TYPE);
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profile.GetBindLevel() == USER) {
                bindTypeVec.push_back(DEVICE_ACROSS_ACCOUNT_TYPE);
            }
            if (profile.GetBindLevel() == APP) {
                bindTypeVec.push_back(APP_ACROSS_ACCOUNT_TYPE);
            }
            break;
        default:
            LOGE("unknown bind type %{public}d.", bindType);
            break;
    }
}

std::vector<int32_t> DeviceProfileConnector::CompareBindType(std::vector<AccessControlProfile> profiles,
    std::string pkgName, std::vector<int32_t> &sinkBindType, std::string localDeviceId, std::string targetDeviceId)
{
    std::vector<int32_t> bindTypeIndex;
    for (uint32_t index = 0; index < profiles.size(); index++) {
        if (IsLnnAcl(profiles[index]) || profiles[index].GetTrustDeviceId() != targetDeviceId ||
            profiles[index].GetStatus() != ACTIVE) {
            continue;
        }
        DmDiscoveryInfo paramInfo = {
            .pkgname = pkgName,
            .localDeviceId = localDeviceId,
        };
        ProcessBindType(profiles[index], localDeviceId, sinkBindType, bindTypeIndex, index, targetDeviceId);
    }
    return bindTypeIndex;
}

void DeviceProfileConnector::ProcessBindType(AccessControlProfile profiles, std::string localDeviceId,
    std::vector<int32_t> &sinkBindType, std::vector<int32_t> &bindTypeIndex, uint32_t index, std::string targetDeviceId)
{
    if (profiles.GetBindType() == DM_IDENTICAL_ACCOUNT) {
        sinkBindType.push_back(IDENTICAL_ACCOUNT_TYPE);
        bindTypeIndex.push_back(index);
    }
    if (profiles.GetBindType() == DM_POINT_TO_POINT) {
        if (profiles.GetBindLevel() == USER) {
            sinkBindType.push_back(DEVICE_PEER_TO_PEER_TYPE);
            bindTypeIndex.push_back(index);
        }
        if (profiles.GetBindLevel() == APP) {
            if (profiles.GetAccesser().GetAccesserDeviceId() == targetDeviceId &&
                profiles.GetAccessee().GetAccesseeDeviceId() == localDeviceId) {
                sinkBindType.push_back(APP_PEER_TO_PEER_TYPE);
                bindTypeIndex.push_back(index);
            }
            if (profiles.GetAccessee().GetAccesseeDeviceId() == targetDeviceId &&
                profiles.GetAccesser().GetAccesserDeviceId() == localDeviceId) {
                sinkBindType.push_back(APP_PEER_TO_PEER_TYPE);
                bindTypeIndex.push_back(index);
            }
        }
    }
    if (profiles.GetBindType() == DM_ACROSS_ACCOUNT) {
        if (profiles.GetBindLevel() == USER) {
            sinkBindType.push_back(DEVICE_ACROSS_ACCOUNT_TYPE);
            bindTypeIndex.push_back(index);
        }
        if (profiles.GetBindLevel() == APP) {
            if (profiles.GetAccesser().GetAccesserDeviceId() == targetDeviceId &&
                profiles.GetAccessee().GetAccesseeDeviceId() == localDeviceId) {
                sinkBindType.push_back(APP_ACROSS_ACCOUNT_TYPE);
                bindTypeIndex.push_back(index);
            }
            if (profiles.GetAccessee().GetAccesseeDeviceId() == targetDeviceId &&
                profiles.GetAccesser().GetAccesserDeviceId() == localDeviceId) {
                sinkBindType.push_back(APP_ACROSS_ACCOUNT_TYPE);
                bindTypeIndex.push_back(index);
            }
        }
    }
}

DM_EXPORT std::vector<int32_t> DeviceProfileConnector::SyncAclByBindType(
    std::string pkgName, std::vector<int32_t> bindTypeVec, std::string localDeviceId, std::string targetDeviceId)
{
    std::vector<AccessControlProfile> profiles =
        GetAccessControlProfileByUserId(MultipleUserConnector::GetFirstForegroundUserId());
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindType;
    std::vector<int32_t> bindTypeIndex =
        CompareBindType(profiles, pkgName, sinkBindType, localDeviceId, targetDeviceId);
    LOGI("SyncAclByBindType sinkBindType size is %{public}zu", sinkBindType.size());
    for (uint32_t sinkIndex = 0; sinkIndex < sinkBindType.size(); sinkIndex++) {
        bool deleteAclFlag = true;
        for (uint32_t srcIndex = 0; srcIndex < bindTypeVec.size(); srcIndex++) {
            if (sinkBindType[sinkIndex] == bindTypeVec[srcIndex]) {
                deleteAclFlag = false;
                bindType.push_back(bindTypeVec[sinkIndex]);
            }
        }
        if (deleteAclFlag) {
            int32_t deleteIndex = profiles[bindTypeIndex[sinkIndex]].GetAccessControlId();
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(deleteIndex);
            LOGI("SyncAclByBindType deleteAcl index is %{public}d", deleteIndex);
        }
    }
    return bindType;
}


DM_EXPORT
std::vector<OHOS::DistributedHardware::ProcessInfo> DeviceProfileConnector::GetProcessInfoFromAclByUserId(
    const std::string &localDeviceId, const std::string &targetDeviceId, int32_t userId)
{
    std::vector<AccessControlProfile> filterProfiles = GetAclProfileByUserId(localDeviceId,
        userId, targetDeviceId);
    LOGI("filterProfiles size is %{public}zu", filterProfiles.size());
    std::vector<OHOS::DistributedHardware::ProcessInfo> processInfoVec;
    for (auto &item : filterProfiles) {
        if (IsLnnAcl(item) || item.GetTrustDeviceId() != targetDeviceId) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        OHOS::DistributedHardware::ProcessInfo processInfo;
        std::string extraStr;
        if (accesserUdid == localDeviceId) {
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            processInfoVec.push_back(processInfo);
            extraStr = item.GetAccesser().GetAccesserExtraData();
        } else if (accesseeUdid == localDeviceId) {
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            processInfoVec.push_back(processInfo);
            extraStr = item.GetAccessee().GetAccesseeExtraData();
        } else {
            continue;
        }
        std::vector<int64_t> proxyTokenIdVec = JsonStrHandle::GetInstance().GetProxyTokenIdByExtra(extraStr);
        for (auto &proxyTokenId : proxyTokenIdVec) {
            std::string proxyBundleName;
            if (AppManager::GetInstance().GetBundleNameByTokenId(proxyTokenId, proxyBundleName) != DM_OK) {
                continue;
            }
            processInfo.pkgName = proxyBundleName;
            processInfoVec.push_back(processInfo);
        }
    }
    return processInfoVec;
}

DM_EXPORT AccessControlProfile DeviceProfileConnector::GetAccessControlProfileByAccessControlId(
    int64_t accessControlId)
{
    AccessControlProfile profile;
    profile.SetAccessControlId(0);
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        if (item.GetAccessControlId() == accessControlId) {
            return item;
        }
    }
    return profile;
}

DM_EXPORT std::vector<std::pair<int64_t, int64_t>> DeviceProfileConnector::GetAgentToProxyVecFromAclByUserId(
    const std::string &localDeviceId, const std::string &targetDeviceId, int32_t userId)
{
    std::vector<AccessControlProfile> filterProfiles = GetAclProfileByUserId(localDeviceId,
        userId, targetDeviceId);
    LOGI("filterProfiles size is %{public}zu", filterProfiles.size());
    std::vector<std::pair<int64_t, int64_t>> agentToProxyVec;
    for (auto &item : filterProfiles) {
        if (IsLnnAcl(item) || item.GetTrustDeviceId() != targetDeviceId) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int64_t agentTokenId;
        std::string extraStr;
        if (accesserUdid == localDeviceId) {
            agentTokenId = item.GetAccesser().GetAccesserTokenId();
            extraStr = item.GetAccesser().GetAccesserExtraData();
        } else if (accesseeUdid == localDeviceId) {
            agentTokenId = item.GetAccessee().GetAccesseeTokenId();
            extraStr = item.GetAccessee().GetAccesseeExtraData();
        } else {
            continue;
        }
        std::vector<int64_t> proxyTokenIdVec = JsonStrHandle::GetInstance().GetProxyTokenIdByExtra(extraStr);
        for (auto &proxyTokenId : proxyTokenIdVec) {
            agentToProxyVec.push_back(std::pair<int64_t, int64_t>(agentTokenId, proxyTokenId));
        }
    }
    return agentToProxyVec;
}

int32_t DeviceProfileConnector::PutAccessControlList(DmAclInfo aclInfo, DmAccesser dmAccesser, DmAccessee dmAccessee)
{
    LOGI("Start.");
    Accesser accesser;
    accesser.SetAccesserDeviceId(dmAccesser.requestDeviceId);
    accesser.SetAccesserUserId(dmAccesser.requestUserId);
    accesser.SetAccesserAccountId(dmAccesser.requestAccountId);
    accesser.SetAccesserTokenId(dmAccesser.requestTokenId);
    accesser.SetAccesserBundleName(dmAccesser.requestBundleName);
    accesser.SetAccesserDeviceName(dmAccesser.requestDeviceName);
    Accessee accessee;
    accessee.SetAccesseeDeviceId(dmAccessee.trustDeviceId);
    accessee.SetAccesseeUserId(dmAccessee.trustUserId);
    accessee.SetAccesseeAccountId(dmAccessee.trustAccountId);
    accessee.SetAccesseeTokenId(dmAccessee.trustTokenId);
    accessee.SetAccesseeBundleName(dmAccessee.trustBundleName);
    accessee.SetAccesseeDeviceName(dmAccessee.trustDeviceName);
    AccessControlProfile profile;
    profile.SetBindType(aclInfo.bindType);
    profile.SetBindLevel(aclInfo.bindLevel);
    profile.SetStatus(ACTIVE);
    profile.SetTrustDeviceId(aclInfo.trustDeviceId);
    profile.SetDeviceIdType((int32_t)DeviceIdType::UDID);
    profile.SetDeviceIdHash(aclInfo.deviceIdHash);
    profile.SetAuthenticationType(aclInfo.authenticationType);
    profile.SetAccessee(accessee);
    profile.SetAccesser(accesser);
    int32_t ret = DistributedDeviceProfileClient::GetInstance().PutAccessControlProfile(profile);
    if (ret != DM_OK) {
        LOGE("PutAccessControlProfile failed.");
    }
    return ret;
}

DM_EXPORT bool DeviceProfileConnector::DeleteAclForAccountLogOut(
    const DMAclQuadInfo &info, const std::string &accountId, DmOfflineParam &offlineParam)
{
    LOGI("localUdid %{public}s, localUserId %{public}d, peerUdid %{public}s, peerUserId %{public}d.",
        GetAnonyString(info.localUdid).c_str(), info.localUserId, GetAnonyString(info.peerUdid).c_str(),
        info.peerUserId);
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    bool notifyOffline = false;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != info.peerUdid) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        std::string accesserAccountId = item.GetAccesser().GetAccesserAccountId();
        std::string accesseeAccountId = item.GetAccessee().GetAccesseeAccountId();
        if (accesserUdid == info.localUdid && accesserUserId == info.localUserId &&
            accesseeUdid == info.peerUdid && accesseeUserId == info.peerUserId &&
            accesserAccountId == accountId) {
            offlineParam.bindType = item.GetBindType();
            ProcessInfo processInfo;
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.pkgName = processInfo.pkgName.empty() ? std::string(DM_PKG_NAME) : processInfo.pkgName;
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            offlineParam.processVec.emplace_back(processInfo);
            notifyOffline = (item.GetStatus() == ACTIVE);
            CacheAcerAclId(item, offlineParam.needDelAclInfos);
            continue;
        }
        if (accesserUdid == info.peerUdid && accesserUserId == info.peerUserId &&
            accesseeUdid == info.localUdid && accesseeUserId == info.localUserId &&
            accesseeAccountId == accountId) {
            offlineParam.bindType = item.GetBindType();
            ProcessInfo processInfo;
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.pkgName = processInfo.pkgName.empty() ? std::string(DM_PKG_NAME) : processInfo.pkgName;
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            offlineParam.processVec.emplace_back(processInfo);
            notifyOffline = (item.GetStatus() == ACTIVE);
            CacheAceeAclId(item, offlineParam.needDelAclInfos);
            continue;
        }
    }
    return notifyOffline;
}

DM_EXPORT bool DeviceProfileConnector::DeleteAclByActhash(
    const DMAclQuadInfo &info, const std::string &accountIdHash, DmOfflineParam &offlineParam)
{
    LOGI("localUdid %{public}s, localUserId %{public}d, peerUdid %{public}s, peerUserId %{public}d.",
        GetAnonyString(info.localUdid).c_str(), info.localUserId, GetAnonyString(info.peerUdid).c_str(),
        info.peerUserId);
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    bool notifyOffline = false;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != info.peerUdid) {
            continue;
        }
        CacheOfflineParam(item, info, accountIdHash, offlineParam, notifyOffline);
    }
    return notifyOffline;
}

void DeviceProfileConnector::CacheOfflineParam(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DMAclQuadInfo &info, const std::string &accountIdHash, DmOfflineParam &offlineParam, bool &notifyOffline)
{
    std::string accesserUdid = profile.GetAccesser().GetAccesserDeviceId();
    std::string accesseeUdid = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t accesserUserId = profile.GetAccesser().GetAccesserUserId();
    int32_t accesseeUserId = profile.GetAccessee().GetAccesseeUserId();
    std::string accesserAccountId = profile.GetAccesser().GetAccesserAccountId();
    std::string accesseeAccountId = profile.GetAccessee().GetAccesseeAccountId();
    char accesserAccountIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetAccountIdHash(accesserAccountId, reinterpret_cast<uint8_t *>(accesserAccountIdHash)) != DM_OK) {
        LOGE("GetAccountHash failed.");
        return;
    }
    char accesseeAccountIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetAccountIdHash(accesseeAccountId, reinterpret_cast<uint8_t *>(accesseeAccountIdHash)) != DM_OK) {
        LOGE("GetAccountHash failed.");
        return;
    }
    if (accesserUdid == info.localUdid && accesserUserId == info.localUserId &&
        accesseeUdid == info.peerUdid && accesseeUserId == info.peerUserId &&
        std::string(accesseeAccountIdHash) == accountIdHash) {
        offlineParam.bindType = profile.GetBindType();
        ProcessInfo processInfo;
        processInfo.pkgName = profile.GetAccesser().GetAccesserBundleName();
        processInfo.pkgName = processInfo.pkgName.empty() ? std::string(DM_PKG_NAME) : processInfo.pkgName;
        processInfo.userId = profile.GetAccesser().GetAccesserUserId();
        offlineParam.processVec.emplace_back(processInfo);
        notifyOffline = (profile.GetStatus() == ACTIVE);
        CacheAcerAclId(profile, offlineParam.needDelAclInfos);
        return;
    }
    if (accesserUdid == info.peerUdid && accesserUserId == info.peerUserId &&
        accesseeUdid == info.localUdid && accesseeUserId == info.localUserId &&
        std::string(accesserAccountIdHash) == accountIdHash) {
        offlineParam.bindType = profile.GetBindType();
        ProcessInfo processInfo;
        processInfo.pkgName = profile.GetAccessee().GetAccesseeBundleName();
        processInfo.pkgName = processInfo.pkgName.empty() ? std::string(DM_PKG_NAME) : processInfo.pkgName;
        processInfo.userId = profile.GetAccessee().GetAccesseeUserId();
        offlineParam.processVec.emplace_back(processInfo);
        notifyOffline = (profile.GetStatus() == ACTIVE);
        CacheAceeAclId(profile, offlineParam.needDelAclInfos);
        return;
    }
}

DM_EXPORT void DeviceProfileConnector::DeleteAclForUserRemoved(std::string localUdid, int32_t userId,
    std::vector<std::string> peerUdids, std::multimap<std::string, int32_t> &peerUserIdMap,
    DmOfflineParam &offlineParam)
{
    LOGI("localUdid %{public}s, userId %{public}d.", GetAnonyString(localUdid).c_str(), userId);
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (const auto &item : profiles) {
        if (find(peerUdids.begin(), peerUdids.end(), item.GetTrustDeviceId()) == peerUdids.end()) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesserUserId == userId) {
            if (!IsValueExist(peerUserIdMap, accesseeUdid, accesseeUserId)) {
                peerUserIdMap.insert(std::pair<std::string, int32_t>(accesseeUdid, accesseeUserId));
            }
            CacheAcerAclId(item, offlineParam.needDelAclInfos);
            continue;
        }
        if (accesseeUdid == localUdid && accesseeUserId == userId) {
            if (!IsValueExist(peerUserIdMap, accesserUdid, accesserUserId)) {
                peerUserIdMap.insert(std::pair<std::string, int32_t>(accesserUdid, accesserUserId));
            }
            CacheAceeAclId(item, offlineParam.needDelAclInfos);
            continue;
        }
    }
}


DM_EXPORT void DeviceProfileConnector::DeleteAclForRemoteUserRemoved(
    std::string peerUdid, int32_t peerUserId, std::vector<int32_t> &userIds, DmOfflineParam &offlineParam)
{
    LOGI("peerUdid %{public}s, peerUserId %{public}d.", GetAnonyString(peerUdid).c_str(), peerUserId);
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (const auto &item : profiles) {
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == peerUdid && accesserUserId == peerUserId) {
            if (item.GetBindLevel() == USER) {
                userIds.push_back(accesseeUserId);
            }
            CacheAceeAclId(item, offlineParam.needDelAclInfos);
        }
        if (accesseeUdid == peerUdid && accesseeUserId == peerUserId) {
            if (item.GetBindLevel() == USER) {
                userIds.push_back(accesserUserId);
            }
            CacheAcerAclId(item, offlineParam.needDelAclInfos);
        }
    }
}

DM_EXPORT void DeviceProfileConnector::DeleteAccessControlList(const std::string &udid)
{
    LOGI("Udid: %{public}s.", GetAnonyString(udid).c_str());
    if (udid.empty()) {
        LOGE("DeleteAccessControlList udid is empty.");
        return;
    }
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("Size is %{public}zu", profiles.size());
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() == udid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
}

DmOfflineParam DeviceProfileConnector::DeleteAccessControlList(const std::string &pkgName,
    const std::string &localDeviceId, const std::string &remoteDeviceId,
    int32_t bindLevel, const std::string &extra)
{
    LOGI("pkgName %{public}s, localDeviceId %{public}s, remoteDeviceId %{public}s, bindLevel %{public}d.",
        pkgName.c_str(), GetAnonyString(localDeviceId).c_str(), GetAnonyString(remoteDeviceId).c_str(), bindLevel);
    DmOfflineParam offlineParam;
    offlineParam.bindType = INVALIED_TYPE;
    if (static_cast<uint32_t>(bindLevel) > APP || static_cast<uint32_t>(bindLevel) < USER) {
        LOGE("Invalied bindlevel.");
        return offlineParam;
    }
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(localDeviceId,
        MultipleUserConnector::GetFirstForegroundUserId());
    if (profiles.empty()) {
        LOGE("Acl is empty.");
        return offlineParam;
    }
    switch (bindLevel) {
        case APP:
            if (extra == "") {
                DeleteAppBindLevel(offlineParam, pkgName, profiles, localDeviceId, remoteDeviceId);
            } else {
                DeleteAppBindLevel(offlineParam, pkgName, profiles, localDeviceId, remoteDeviceId, extra);
            }
            break;
        case SERVICE:
            DeleteServiceBindLevel(offlineParam, pkgName, profiles, localDeviceId, remoteDeviceId);
            break;
        case USER:
            DeleteDeviceBindLevel(offlineParam, profiles, localDeviceId, remoteDeviceId);
            break;
        default:
            break;
    }
    return offlineParam;
}

void DeviceProfileConnector::DeleteAppBindLevel(DmOfflineParam &offlineParam, const std::string &pkgName,
    const std::vector<AccessControlProfile> &profiles, const std::string &localUdid, const std::string &remoteUdid)
{
    int32_t bindNums = 0;
    int32_t deleteNums = 0;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid || item.GetBindType() == DM_IDENTICAL_ACCOUNT ||
            item.GetBindLevel() != APP) {
            continue;
        }
        bindNums++;
        if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = APP;
            ProcessInfo processInfo;
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            offlineParam.processVec.push_back(processInfo);
            offlineParam.peerUserId = item.GetAccessee().GetAccesseeUserId();
            LOGI("Src delete acl pkgName %{public}s, bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s",
                pkgName.c_str(), item.GetBindType(), GetAnonyString(localUdid).c_str(),
                GetAnonyString(remoteUdid).c_str());
            continue;
        }
        if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = APP;
            ProcessInfo processInfo;
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            offlineParam.processVec.push_back(processInfo);
            offlineParam.peerUserId = item.GetAccesser().GetAccesserUserId();
            LOGI("Sink delete acl pkgName %{public}s, bindType %{public}u, localUdid %{public}s, remoteUdid %{public}s",
                pkgName.c_str(), item.GetBindType(), GetAnonyString(localUdid).c_str(),
                GetAnonyString(remoteUdid).c_str());
            continue;
        }
    }
    offlineParam.leftAclNumber = bindNums - deleteNums;
}

void DeviceProfileConnector::DeleteAppBindLevel(DmOfflineParam &offlineParam, const std::string &pkgName,
    const std::vector<AccessControlProfile> &profiles, const std::string &localUdid,
    const std::string &remoteUdid, const std::string &extra)
{
    int32_t bindNums = 0;
    int32_t deleteNums = 0;
    uint64_t peerTokenId = 0;
    std::string peerBundleName;
    JsonStrHandle::GetInstance().GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    LOGI("DeviceProfileConnector::DeleteAppBindLevel peerBundleName %{public}s", peerBundleName.c_str());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid || item.GetBindType() == DM_IDENTICAL_ACCOUNT ||
            item.GetBindLevel() != APP) {
            continue;
        }
        bindNums++;
        if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeBundleName() == peerBundleName &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = APP;
            ProcessInfo processInfo;
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            offlineParam.processVec.push_back(processInfo);
            offlineParam.peerUserId = item.GetAccessee().GetAccesseeUserId();
            LOGI("Src delete acl pkgName %{public}s, bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s",
                pkgName.c_str(), item.GetBindType(), GetAnonyString(localUdid).c_str(),
                GetAnonyString(remoteUdid).c_str());
            continue;
        }
        if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccesser().GetAccesserBundleName() == peerBundleName &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = APP;
            ProcessInfo processInfo;
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            offlineParam.processVec.push_back(processInfo);
            offlineParam.peerUserId = item.GetAccesser().GetAccesserUserId();
            LOGI("Sink delete acl pkgName %{public}s, bindType %{public}u, localUdid %{public}s, remoteUdid %{public}s",
                pkgName.c_str(), item.GetBindType(), GetAnonyString(localUdid).c_str(),
                GetAnonyString(remoteUdid).c_str());
            continue;
        }
    }
    offlineParam.leftAclNumber = bindNums - deleteNums;
}

void DeviceProfileConnector::DeleteDeviceBindLevel(DmOfflineParam &offlineParam,
    const std::vector<AccessControlProfile> &profiles, const std::string &localUdid, const std::string &remoteUdid)
{
    int32_t bindNums = 0;
    int32_t deleteNums = 0;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid || item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            continue;
        }
        bindNums++;
        if (item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = USER;
            LOGI("Src delete acl bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s", item.GetBindType(),
                GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str());
            continue;
        }
        if (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = USER;
            LOGI("Sink delete acl bindType %{public}u, localUdid %{public}s, remoteUdid %{public}s", item.GetBindType(),
                GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str());
            continue;
        }
    }
    offlineParam.leftAclNumber = bindNums - deleteNums;
}

void DeviceProfileConnector::DeleteServiceBindLevel(DmOfflineParam &offlineParam, const std::string &pkgName,
    const std::vector<AccessControlProfile> &profiles, const std::string &localUdid, const std::string &remoteUdid)
{
    int32_t bindNums = 0;
    int32_t deleteNums = 0;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid || item.GetBindType() == DM_IDENTICAL_ACCOUNT ||
            item.GetBindLevel() != SERVICE) {
            continue;
        }
        bindNums++;
        if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = SERVICE;
            LOGI("Src delete acl pkgName %{public}s, bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s",
                pkgName.c_str(), item.GetBindType(), GetAnonyString(localUdid).c_str(),
                GetAnonyString(remoteUdid).c_str());
            continue;
        }
        if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = SERVICE;
            LOGI("Sink delete acl pkgName %{public}s, bindType %{public}u, localUdid %{public}s, remoteUdid %{public}s",
                pkgName.c_str(), item.GetBindType(), GetAnonyString(localUdid).c_str(),
                GetAnonyString(remoteUdid).c_str());
            continue;
        }
    }
    offlineParam.leftAclNumber = bindNums - deleteNums;
}

int32_t DeviceProfileConnector::UpdateAccessControlList(int32_t userId, std::string &oldAccountId,
    std::string &newAccountId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if ((item.GetAccesser().GetAccesserUserId() == userId &&
            item.GetAccesser().GetAccesserAccountId() == oldAccountId) ||
            (item.GetAccessee().GetAccesseeUserId() == userId &&
            item.GetAccessee().GetAccesseeAccountId() == oldAccountId)) {
            item.SetStatus(INACTIVE);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
        }
        if ((item.GetAccesser().GetAccesserUserId() == userId &&
            item.GetAccesser().GetAccesserAccountId() == newAccountId) ||
            (item.GetAccessee().GetAccesseeUserId() == userId &&
            item.GetAccessee().GetAccesseeAccountId() == newAccountId)) {
            item.SetStatus(ACTIVE);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
        }
    }
    return DM_OK;
}

DM_EXPORT bool DeviceProfileConnector::CheckSrcDevIdInAclForDevBind(
    const std::string &pkgName, const std::string &deviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == deviceId && item.GetStatus() == ACTIVE &&
            item.GetBindLevel() == USER && (item.GetAccessee().GetAccesseeBundleName() == pkgName ||
            item.GetAccesser().GetAccesserBundleName() == "") && item.GetAccessee().GetAccesseeUserId() == 0 &&
            item.GetAccessee().GetAccesseeAccountId() == "") {
            return true;
        }
    }
    return false;
}

bool DeviceProfileConnector::CheckSinkDevIdInAclForDevBind(const std::string &pkgName, const std::string &deviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == deviceId && item.GetStatus() == ACTIVE &&
            item.GetBindLevel() == USER && (item.GetAccesser().GetAccesserBundleName() == pkgName ||
            item.GetAccesser().GetAccesserBundleName() == "") && item.GetAccesser().GetAccesserUserId() == 0 &&
            item.GetAccesser().GetAccesserAccountId() == "") {
            return true;
        }
    }
    return false;
}

bool DeviceProfileConnector::CheckDevIdInAclForDevBind(const std::string &pkgName, const std::string &deviceId)
{
    return (CheckSinkDevIdInAclForDevBind(pkgName, deviceId) || CheckSrcDevIdInAclForDevBind(pkgName, deviceId));
}


DM_EXPORT uint32_t DeviceProfileConnector::DeleteTimeOutAcl(const std::string &deviceId, DmOfflineParam &offlineParam)
{
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    uint32_t res = 0;
    int32_t currentUserId = MultipleUserConnector::GetCurrentAccountUserID();
    std::string localUdid = GetLocalDeviceId();
    DmAclIdParam dmAclIdParam;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != deviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        if (CacheLnnAcl(item, localUdid, dmAclIdParam)) {
            continue;
        }
        res++;
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (item.GetAuthenticationType() == ALLOW_AUTH_ONCE) {
            res--;
            if (accesserUserId == currentUserId && accesserUdid == localUdid && accesseeUdid == deviceId) {
                CacheAcerAclId(item, offlineParam.needDelAclInfos);
                offlineParam.peerUserId = item.GetAccessee().GetAccesseeUserId();
            }
            if (accesseeUserId == currentUserId && accesseeUdid == localUdid && accesserUdid == deviceId) {
                CacheAceeAclId(item, offlineParam.needDelAclInfos);
                offlineParam.peerUserId = item.GetAccesser().GetAccesserUserId();
            }
        }
    }
    if (res == 0) {
        offlineParam.needDelAclInfos.push_back(dmAclIdParam);
    }
    return res;
}


bool DeviceProfileConnector::CacheLnnAcl(AccessControlProfile profile, const std::string &localUdid,
    DmAclIdParam &dmAclIdParam)
{
    if (IsLnnAcl(profile)) {
        if (profile.GetAccesser().GetAccesserDeviceId() == localUdid) {
            dmAclIdParam.udid = profile.GetAccesser().GetAccesserDeviceId();
            dmAclIdParam.userId = profile.GetAccesser().GetAccesserUserId();
            dmAclIdParam.skId = profile.GetAccesser().GetAccesserSessionKeyId();
            dmAclIdParam.credId = profile.GetAccesser().GetAccesserCredentialIdStr();
            dmAclIdParam.accessControlId = profile.GetAccessControlId();
        }
        if (profile.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            dmAclIdParam.udid = profile.GetAccessee().GetAccesseeDeviceId();
            dmAclIdParam.userId = profile.GetAccessee().GetAccesseeUserId();
            dmAclIdParam.skId = profile.GetAccessee().GetAccesseeSessionKeyId();
            dmAclIdParam.credId = profile.GetAccessee().GetAccesseeCredentialIdStr();
            dmAclIdParam.accessControlId = profile.GetAccessControlId();
        }
        return true;
    }
    return false;
}

DM_EXPORT int32_t DeviceProfileConnector::GetTrustNumber(const std::string &deviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    int32_t trustNumber = 0;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == deviceId && item.GetStatus() == ACTIVE) {
            trustNumber++;
        }
    }
    return trustNumber;
}

DM_EXPORT int32_t DeviceProfileConnector::IsSameAccount(const std::string &udid)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == udid && item.GetStatus() == ACTIVE) {
            if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
                LOGI("The udid %{public}s is identical bind.", GetAnonyString(udid).c_str());
                return DM_OK;
            }
        }
    }
    return ERR_DM_FAILED;
}

DM_EXPORT int32_t DeviceProfileConnector::CheckIsSameAccountByUdidHash(const std::string &udidHash)
{
    if (udidHash.empty()) {
        LOGE("udidHash is empty!");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    for (auto &item : profiles) {
        if (Crypto::GetUdidHash(item.GetTrustDeviceId()) == udidHash) {
            if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
                LOGI("The udidHash %{public}s is identical bind.", GetAnonyString(udidHash).c_str());
                return DM_OK;
            }
        }
    }
    return ERR_DM_VERIFY_SAME_ACCOUNT_FAILED;
}

bool checkAccesserACL(AccessControlProfile& profile, const DmAccessCaller &caller,
    const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    if ((profile.GetAccesser().GetAccesserUserId() == caller.userId ||
        profile.GetAccesser().GetAccesserUserId() == 0 ||
        profile.GetAccesser().GetAccesserUserId() == -1) &&
        profile.GetAccesser().GetAccesserDeviceId() == srcUdid &&
        profile.GetAccessee().GetAccesseeDeviceId() == sinkUdid) {
        if (callee.userId != 0 && callee.userId == profile.GetAccessee().GetAccesseeUserId()) {
            LOGI("accesser success add, callee userId not null!");
            return true;
        } else if (callee.userId == 0 ||
            profile.GetAccessee().GetAccesseeUserId() == -1 ||
            profile.GetAccessee().GetAccesseeUserId() == 0) {
            LOGI("accesser success add!");
            return true;
        }
    }
    return false;
}

bool checkAccesseeACL(AccessControlProfile& profile, const DmAccessCaller &caller,
    const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    if ((profile.GetAccessee().GetAccesseeUserId() == caller.userId ||
        profile.GetAccessee().GetAccesseeUserId() == 0 ||
        profile.GetAccessee().GetAccesseeUserId() == -1) &&
        profile.GetAccessee().GetAccesseeDeviceId() == srcUdid &&
        profile.GetAccesser().GetAccesserDeviceId() == sinkUdid) {
        if (callee.userId != 0 && callee.userId == profile.GetAccesser().GetAccesserUserId()) {
            LOGI("accessee success add, callee userId not null!");
            return true;
        } else if (callee.userId == 0 ||
            profile.GetAccesser().GetAccesserUserId() == -1 ||
            profile.GetAccesser().GetAccesserUserId() == 0) {
            LOGI("accessee success add!");
            return true;
        }
    }
    return false;
}

std::vector<AccessControlProfile> GetACLByDeviceIdAndUserId(std::vector<AccessControlProfile> profiles,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("srcUdid = %{public}s, caller.userId = %{public}s, sinkUdid = %{public}s, callee.userId = %{public}s",
        GetAnonyString(srcUdid).c_str(), GetAnonyInt32(caller.userId).c_str(),
        GetAnonyString(sinkUdid).c_str(), GetAnonyInt32(callee.userId).c_str());
    std::vector<AccessControlProfile> profilesFilter;
    for (auto &item : profiles) {
        if (checkAccesserACL(item, caller, srcUdid, callee, sinkUdid) ||
            checkAccesseeACL(item, caller, srcUdid, callee, sinkUdid)) {
            profilesFilter.push_back(item);
        }
    }
    return profilesFilter;
}

DM_EXPORT bool DeviceProfileConnector::CheckAccessControl(
    const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("srcUdid %{public}s, srcUserId %{public}d, srcPkgName %{public}s"
        "srcAccountId %{public}s, sinkUdid %{public}s, sinkUserId %{public}d, sinkPkgName %{public}s,"
        "sinkAccountId %{public}s.", GetAnonyString(srcUdid).c_str(), caller.userId,
        caller.pkgName.c_str(), GetAnonyString(caller.accountId).c_str(),
        GetAnonyString(sinkUdid).c_str(), callee.userId, callee.pkgName.c_str(),
        GetAnonyString(callee.accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<AccessControlProfile> profilesFilter =
        GetACLByDeviceIdAndUserId(profiles, caller, srcUdid, callee, sinkUdid);
    for (auto &item : profilesFilter) {
        if (item.GetStatus() != ACTIVE || (item.GetTrustDeviceId() != sinkUdid &&
            item.GetTrustDeviceId() != srcUdid)) {
            continue;
        }
        if (SingleUserProcess(item, caller, callee)) {
            return true;
        }
    }
    return false;
}

bool DeviceProfileConnector::SingleUserProcess(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    LOGI("BindType %{public}d, bindLevel %{public}d.",
        profile.GetBindType(), profile.GetBindLevel());
    uint32_t bindType = profile.GetBindType();
    bool ret = false;
    switch (bindType) {
        case DM_IDENTICAL_ACCOUNT:
            ret = true;
            break;
        case DM_POINT_TO_POINT:
            if (profile.GetBindLevel() == USER && !IsLnnAcl(profile)) {
                ret = true;
            } else if ((profile.GetBindLevel() == APP || profile.GetBindLevel() == SERVICE) &&
                (profile.GetAccesser().GetAccesserBundleName() == caller.pkgName ||
                profile.GetAccessee().GetAccesseeBundleName() == caller.pkgName)) {
                ret = CheckAppLevelAccess(profile, caller, callee);
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profile.GetBindLevel() == USER && !IsLnnAcl(profile)) {
                ret = true;
            } else if ((profile.GetBindLevel() == APP || profile.GetBindLevel() == SERVICE) &&
                (profile.GetAccesser().GetAccesserBundleName() == caller.pkgName ||
                profile.GetAccessee().GetAccesseeBundleName() == caller.pkgName)) {
                ret = CheckAppLevelAccess(profile, caller, callee);
            }
            break;
        default:
            LOGE("unknown bind type %{public}d.", bindType);
            break;
    }
    return ret;
}

bool DeviceProfileConnector::CheckAppLevelAccess(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    if (caller.tokenId == 0 || callee.tokenId == 0) {
        return true;
    } else {
        if ((static_cast<uint64_t>(profile.GetAccesser().GetAccesserTokenId()) == caller.tokenId &&
            static_cast<uint64_t>(profile.GetAccessee().GetAccesseeTokenId()) == callee.tokenId) ||
            (static_cast<uint64_t>(profile.GetAccesser().GetAccesserTokenId()) == callee.tokenId &&
            static_cast<uint64_t>(profile.GetAccessee().GetAccesseeTokenId()) == caller.tokenId)) {
            return true;
        } else {
            return false;
        }
    }
}

DM_EXPORT bool DeviceProfileConnector::CheckIsSameAccount(
    const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("srcUdid %{public}s, srcUserId %{public}d, srcPkgName %{public}s,"
        "srcAccountId %{public}s, sinkUdid %{public}s, sinkUserId %{public}d, sinkPkgName %{public}s,"
        "sinkAccountId %{public}s.", GetAnonyString(srcUdid).c_str(), caller.userId,
        caller.pkgName.c_str(), GetAnonyString(caller.accountId).c_str(),
        GetAnonyString(sinkUdid).c_str(), callee.userId, callee.pkgName.c_str(),
        GetAnonyString(callee.accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<AccessControlProfile> profilesFilter
        = GetACLByDeviceIdAndUserId(profiles, caller, srcUdid, callee, sinkUdid);
    for (auto &item : profilesFilter) {
        if (item.GetStatus() != ACTIVE || (item.GetTrustDeviceId() != sinkUdid &&
            item.GetTrustDeviceId() != srcUdid)) {
            continue;
        }
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            LOGI("The udid %{public}s is identical bind.", GetAnonyString(item.GetTrustDeviceId()).c_str());
            return true;
        }
    }
    return false;
}

DM_EXPORT int32_t DeviceProfileConnector::GetBindLevel(const std::string &pkgName,
    const std::string &localUdid, const std::string &udid, uint64_t &tokenId)
{
    LOGI("pkgName %{public}s, udid %{public}s.", pkgName.c_str(), GetAnonyString(udid).c_str());
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(localUdid,
        MultipleUserConnector::GetFirstForegroundUserId());
    int32_t bindLevel = INVALIED_TYPE;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != udid) {
            continue;
        }
        if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == udid) {
            tokenId = static_cast<uint64_t>(item.GetAccesser().GetAccesserTokenId());
            bindLevel = static_cast<int32_t>(item.GetBindLevel());
            LOGI("Src get bindLevel %{public}d.", bindLevel);
            continue;
        }
        if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == udid) {
            tokenId = static_cast<uint64_t>(item.GetAccessee().GetAccesseeTokenId());
            bindLevel = static_cast<int32_t>(item.GetBindLevel());
            LOGI("Sink get bindLevel %{public}d.", bindLevel);
            continue;
        }
    }
    return bindLevel;
}

std::map<std::string, int32_t> DeviceProfileConnector::GetDeviceIdAndBindLevel(std::vector<int32_t> userIds,
    const std::string &localUdid)
{
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::map<std::string, int32_t> deviceIdMap;
    for (const auto &item : profiles) {
        if (IsLnnAcl(item)) {
            continue;
        }
        if (find(userIds.begin(), userIds.end(), item.GetAccesser().GetAccesserUserId()) != userIds.end() &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid) {
            LOGI("Get Device Bind type localUdid %{public}s is src.", GetAnonyString(localUdid).c_str());
            UpdateBindType(item.GetTrustDeviceId(), item.GetBindLevel(), deviceIdMap);
            continue;
        }
        if (find(userIds.begin(), userIds.end(), item.GetAccessee().GetAccesseeUserId()) != userIds.end() &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            LOGI("Get Device Bind type localUdid %{public}s is sink.", GetAnonyString(localUdid).c_str());
            UpdateBindType(item.GetTrustDeviceId(), item.GetBindLevel(), deviceIdMap);
            continue;
        }
    }
    return deviceIdMap;
}

std::vector<std::string> DeviceProfileConnector::GetDeviceIdAndUdidListByTokenId(const std::vector<int32_t> &userIds,
    const std::string &localUdid, int32_t tokenId)
{
    if (userIds.empty() || localUdid.empty()) {
        LOGE("userIds or localUdid is empty.");
        return {};
    }
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::map<std::string, int32_t> deviceIdMap;
    std::vector<std::string> udidList;

    for (const auto &item : profiles) {
        if (IsLnnAcl(item)) {
            continue;
        }

        if (find(userIds.begin(), userIds.end(), item.GetAccesser().GetAccesserUserId()) != userIds.end() &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId()) == tokenId) {
            LOGI("Get Device Bind type localUdid %{public}s is src, tokenId %{public}s.",
                GetAnonyString(localUdid).c_str(), GetAnonyInt32(tokenId).c_str());
            UpdateBindType(item.GetTrustDeviceId(), item.GetBindLevel(), deviceIdMap);
            continue;
        }

        if (find(userIds.begin(), userIds.end(), item.GetAccessee().GetAccesseeUserId()) != userIds.end() &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId()) == tokenId) {
            LOGI("Get Device Bind type localUdid %{public}s is sink, tokenId %{public}s.",
                GetAnonyString(localUdid).c_str(), GetAnonyInt32(tokenId).c_str());
            UpdateBindType(item.GetTrustDeviceId(), item.GetBindLevel(), deviceIdMap);
            continue;
        }
    }

    for (const auto &item : deviceIdMap) {
        udidList.push_back(item.first);
    }
    return udidList;
}

DM_EXPORT std::multimap<std::string, int32_t> DeviceProfileConnector::GetDeviceIdAndUserId(
    int32_t userId, const std::string &accountId, const std::string &localUdid)
{
    LOGI("localUdid %{public}s, userId %{public}d, accountId %{public}s.", GetAnonyString(localUdid).c_str(),
        userId, GetAnonyString(accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    std::multimap<std::string, int32_t> deviceIdMap;
    for (const auto &item : profiles) {
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        std::string accesserAccountId = item.GetAccesser().GetAccesserAccountId();
        std::string accesseeAccountId = item.GetAccessee().GetAccesseeAccountId();
        if (accesserUdid == localUdid && accesserUserId == userId && accesserAccountId == accountId) {
            LOGI("Account logout trust udid %{public}s userid %{public}d is src.",
                GetAnonyString(accesseeUdid).c_str(), accesseeUserId);
            deviceIdMap.insert(std::pair<std::string, int32_t>(accesseeUdid, accesseeUserId));
            continue;
        }
        if (accesseeUdid == localUdid && accesseeUserId == userId && accesseeAccountId == accountId) {
            LOGI("Account logout trust udid %{public}s userid %{public}d is sink.",
                GetAnonyString(accesserUdid).c_str(), accesserUserId);
            deviceIdMap.insert(std::pair<std::string, int32_t>(accesserUdid, accesserUserId));
            continue;
        }
    }
    return deviceIdMap;
}

void DeviceProfileConnector::UpdateBindType(const std::string &udid, int32_t compareParam,
    std::map<std::string, int32_t> &deviceMap)
{
    LOGI("BindType %{public}d.", compareParam);
    if (deviceMap.find(udid) == deviceMap.end()) {
        deviceMap[udid] = compareParam;
    } else {
        deviceMap[udid] = std::min(deviceMap[udid], compareParam);
    }
}

int32_t DeviceProfileConnector::HandleAccountLogoutEvent(int32_t remoteUserId,
    const std::string &remoteAccountHash, const std::string &remoteUdid, const std::string &localUdid)
{
    LOGI("RemoteUserId %{public}d, remoteAccountHash %{public}s, remoteUdid %{public}s, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteAccountHash).c_str(), GetAnonyString(remoteUdid).c_str(),
        GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(remoteUserId);
    int32_t bindType = DM_INVALIED_TYPE;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesseeUdid == remoteUdid && accesseeUserId == remoteUserId) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            bindType = DM_IDENTICAL_ACCOUNT;
            continue;
        }
        if (accesseeUdid == localUdid && accesserUdid == remoteUdid && accesserUserId == remoteUserId) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            bindType = DM_IDENTICAL_ACCOUNT;
            continue;
        }
    }
    return bindType;
}


DM_EXPORT int32_t DeviceProfileConnector::HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    const std::string &localUdid, DmOfflineParam &offlineParam)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, localUdid %{public}s.", remoteUserId,
        GetAnonyString(remoteUdid).c_str(), GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(remoteUdid, remoteUserId, localUdid);
    int32_t bindType = DM_INVALIED_TYPE;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            bindType = DM_IDENTICAL_ACCOUNT;
            continue;
        }
        if (item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
            offlineParam.bindType = USER;
            CacheAcerAclId(item, offlineParam.needDelAclInfos);
            LOGI("Src delete acl bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s", item.GetBindType(),
                GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str());
            bindType = std::min(bindType, static_cast<int32_t>(item.GetBindType()));
            continue;
        }
        if (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid) {
            offlineParam.bindType = USER;
            CacheAceeAclId(item, offlineParam.needDelAclInfos);
            LOGI("Sink delete acl bindType %{public}u, localUdid %{public}s, remoteUdid %{public}s", item.GetBindType(),
                GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str());
            bindType = std::min(bindType, static_cast<int32_t>(item.GetBindType()));
            continue;
        }
    }
    return bindType;
}


DM_EXPORT DmOfflineParam DeviceProfileConnector::HandleAppUnBindEvent(int32_t remoteUserId,
    const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteUdid).c_str(), GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(remoteUdid, remoteUserId, localUdid);
    DmOfflineParam offlineParam;
    if (profiles.empty()) {
        LOGE("Acl is empty.");
        return offlineParam;
    }
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }

        // First, find need delete acl
        if (FindTargetAcl(item, localUdid, remoteUserId, remoteUdid, tokenId, offlineParam)) {
            continue;
        }

        // Second, find the LNN acl
        if (FindLnnAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Third, find the user or sameaccount acl, dertermine if report offline
        if (FindUserAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Fourth, find the left service/app acl, determine if del lnn acl
        if (FindLeftAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }
    }
    return offlineParam;
}

bool DeviceProfileConnector::FindTargetAcl(const DistributedDeviceProfile::AccessControlProfile &acl,
    const std::string &localUdid, const int32_t remoteUserId, const std::string &remoteUdid,
    const int32_t tokenId, const int32_t peerTokenId, DmOfflineParam &offlineParam)
{
    bool isMatch = false;
    std::string acerDeviceId = acl.GetAccesser().GetAccesserDeviceId();
    std::string aceeDeviceId = acl.GetAccessee().GetAccesseeDeviceId();
    int32_t acerTokenId = static_cast<int32_t>(acl.GetAccesser().GetAccesserTokenId());
    int32_t aceeTokenId = static_cast<int32_t>(acl.GetAccessee().GetAccesseeTokenId());
    if (acl.GetAccesser().GetAccesserUserId() == remoteUserId && acerDeviceId == remoteUdid &&
        aceeDeviceId == localUdid && (acerTokenId == peerTokenId) && (aceeTokenId == tokenId)) {
        ProcessInfo processInfo;
        processInfo.pkgName = acl.GetAccessee().GetAccesseeBundleName();
        processInfo.userId = acl.GetAccessee().GetAccesseeUserId();
        offlineParam.processVec.push_back(processInfo);
        CacheAceeAclId(acl, offlineParam.needDelAclInfos);
        LOGI("Src del acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
            ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
            GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
            acl.GetBindLevel());
        isMatch = true;
    }
    if (acl.GetAccessee().GetAccesseeUserId() == remoteUserId && aceeDeviceId == remoteUdid &&
        acerDeviceId == localUdid && (aceeTokenId == peerTokenId) && (acerTokenId == tokenId)) {
        ProcessInfo processInfo;
        processInfo.pkgName = acl.GetAccesser().GetAccesserBundleName();
        processInfo.userId = acl.GetAccesser().GetAccesserUserId();
        offlineParam.processVec.push_back(processInfo);
        CacheAcerAclId(acl, offlineParam.needDelAclInfos);
        LOGI("Sink del acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
            ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
            GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
            acl.GetBindLevel());
        isMatch = true;
    }
    return isMatch;
}

DM_EXPORT DmOfflineParam DeviceProfileConnector::HandleAppUnBindEvent(int32_t remoteUserId,
    const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid, int32_t peerTokenId)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteUdid).c_str(), GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(remoteUdid, remoteUserId, localUdid);
    DmOfflineParam offlineParam;
    if (profiles.empty()) {
        LOGE("Acl is empty.");
        return offlineParam;
    }
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }

        // First, find need delete acl
        if (FindTargetAcl(item, localUdid, remoteUserId, remoteUdid, tokenId, peerTokenId, offlineParam)) {
            continue;
        }

        // Second, find the LNN acl
        if (FindLnnAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Third, find the user or sameaccount acl, dertermine if report offline
        if (FindUserAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Fourth, find the left service/app acl, determine if del lnn acl
        if (FindLeftAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }
    }
    return offlineParam;
}

bool DeviceProfileConnector::FindTargetAcl(const DistributedDeviceProfile::AccessControlProfile &acl,
    const std::string &localUdid, const int32_t remoteUserId, const std::string &remoteUdid,
    const int32_t remoteTokenId, DmOfflineParam &offlineParam)
{
    bool isMatch = false;
    if (acl.GetAccesser().GetAccesserUserId() == remoteUserId &&
        acl.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
        (static_cast<int32_t>(acl.GetAccesser().GetAccesserTokenId()) == remoteTokenId) &&
        acl.GetAccessee().GetAccesseeDeviceId() == localUdid) {
        ProcessInfo processInfo;
        processInfo.pkgName = acl.GetAccessee().GetAccesseeBundleName();
        processInfo.userId = acl.GetAccessee().GetAccesseeUserId();
        offlineParam.processVec.push_back(processInfo);
        CacheAceeAclId(acl, offlineParam.needDelAclInfos);
        LOGI("Src del acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
            ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
            GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
            acl.GetBindLevel());
        isMatch = true;
    }

    if (acl.GetAccessee().GetAccesseeUserId() == remoteUserId &&
        acl.GetAccessee().GetAccesseeDeviceId() == remoteUdid &&
        (static_cast<int32_t>(acl.GetAccessee().GetAccesseeTokenId()) == remoteTokenId) &&
        acl.GetAccesser().GetAccesserDeviceId() == localUdid) {
        ProcessInfo processInfo;
        processInfo.pkgName = acl.GetAccesser().GetAccesserBundleName();
        processInfo.userId = acl.GetAccesser().GetAccesserUserId();
        offlineParam.processVec.push_back(processInfo);
        CacheAcerAclId(acl, offlineParam.needDelAclInfos);
        LOGI("Sink del acl aclId: %{public}" PRId64 ", localUdid %{public}s, remoteUdid %{public}s"
            ", bindType %{public}d, bindLevel: %{public}d", acl.GetAccessControlId(),
            GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str(), acl.GetBindType(),
            acl.GetBindLevel());
        isMatch = true;
    }
    return isMatch;
}

DmOfflineParam DeviceProfileConnector::HandleServiceUnBindEvent(int32_t remoteUserId,
    const std::string &remoteUdid, const std::string &localUdid, int32_t tokenId)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteUdid).c_str(), GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(remoteUdid, remoteUserId, localUdid);
    DmOfflineParam offlineParam;
    if (profiles.empty()) {
        LOGE("Acl is empty.");
        return offlineParam;
    }
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }

        // First, find need delete acl
        if (FindTargetAcl(item, localUdid, remoteUserId, remoteUdid, tokenId, offlineParam)) {
            continue;
        }

        // Second, find the LNN acl
        if (FindLnnAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Third, find the user or sameaccount acl, dertermine if report offline
        if (FindUserAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }

        // Fourth, find the left service/app acl, determine if del lnn acl
        if (FindLeftAcl(item, localUdid, remoteUdid, offlineParam)) {
            continue;
        }
    }
    return offlineParam;
}

DM_EXPORT std::vector<AccessControlProfile> DeviceProfileConnector::GetAllAccessControlProfile()
{
    std::vector<AccessControlProfile> profiles;
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetAllAccessControlProfile(profiles);
    if (ret != DM_OK) {
        LOGE("DP failed, ret = %{public}d", ret);
    }
    return profiles;
}

DM_EXPORT std::vector<AccessControlProfile> DeviceProfileConnector::GetAllAclIncludeLnnAcl()
{
    std::vector<AccessControlProfile> profiles;
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetAllAclIncludeLnnAcl(profiles);
    if (ret != DM_OK) {
        LOGE("DP failed, ret = %{public}d", ret);
    }
    return profiles;
}

DM_EXPORT void DeviceProfileConnector::DeleteAccessControlById(
    int64_t accessControlId)
{
    DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(accessControlId);
}

DM_EXPORT int32_t DeviceProfileConnector::HandleUserSwitched(
    const std::string &localUdid, const std::vector<std::string> &deviceVec, int32_t currentUserId,
    int32_t beforeUserId)
{
    if (deviceVec.empty()) {
        LOGI("no remote device.");
        return DM_OK;
    }
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    std::vector<AccessControlProfile> activeProfiles;
    std::vector<AccessControlProfile> inActiveProfiles;
    for (auto &item : profiles) {
        if (std::find(deviceVec.begin(), deviceVec.end(), item.GetTrustDeviceId()) == deviceVec.end()) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserUserId() == beforeUserId) ||
            (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeUserId() == beforeUserId)) {
            if (item.GetStatus() == ACTIVE) {
                item.SetStatus(INACTIVE);
                inActiveProfiles.push_back(item);
            }
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserUserId() == currentUserId && item.GetStatus() == INACTIVE) || (
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeUserId() == currentUserId && item.GetStatus() == INACTIVE)) {
            item.SetStatus(ACTIVE);
            activeProfiles.push_back(item);
            continue;
        }
    }
    HandleUserSwitched(activeProfiles, inActiveProfiles);
    return DM_OK;
}

DM_EXPORT void DeviceProfileConnector::HandleUserSwitched(
    const std::vector<AccessControlProfile> &activeProfiles,
    const std::vector<AccessControlProfile> &inActiveProfiles)
{
    for (auto &item : inActiveProfiles) {
        DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
    }

    for (auto &item : activeProfiles) {
        DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
    }
}

std::vector<AccessControlProfile> DeviceProfileConnector::GetAclProfileByUserId(const std::string &localUdid,
    int32_t userId, const std::string &remoteUdid)
{
    LOGI("localUdid %{public}s, localUserId %{public}d, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        userId, GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<AccessControlProfile> profilesTemp;
    for (const auto &item : profiles) {
        if (item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserUserId() == userId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
            profilesTemp.push_back(item);
        } else if (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeUserId() == userId &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid) {
            profilesTemp.push_back(item);
        }
    }
    return profilesTemp;
}

DM_EXPORT void DeviceProfileConnector::HandleSyncForegroundUserIdEvent(
    const std::vector<int32_t> &remoteUserIds, const std::string &remoteUdid,
    const std::vector<int32_t> &localUserIds, std::string &localUdid)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        if (item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid &&
            find(localUserIds.begin(), localUserIds.end(),
                item.GetAccesser().GetAccesserUserId()) != localUserIds.end() &&
            find(remoteUserIds.begin(), remoteUserIds.end(),
                item.GetAccessee().GetAccesseeUserId()) != remoteUserIds.end() && item.GetStatus() == INACTIVE) {
            item.SetStatus(ACTIVE);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
        } else if (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
            find(localUserIds.begin(), localUserIds.end(),
                item.GetAccessee().GetAccesseeUserId()) != localUserIds.end() &&
            find(remoteUserIds.begin(), remoteUserIds.end(),
                item.GetAccesser().GetAccesserUserId()) != remoteUserIds.end() && item.GetStatus() == INACTIVE) {
            item.SetStatus(ACTIVE);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
        }
    }
}

std::vector<ProcessInfo> DeviceProfileConnector::GetOfflineProcessInfo(std::string &localUdid,
    const std::vector<int32_t> &localUserIds, const std::string &remoteUdid, const std::vector<int32_t> &remoteUserIds)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<ProcessInfo> processInfos;
    int32_t bindLevel = 100;
    for (const auto &item : profiles) {
        if (IsLnnAcl(item)) {
            continue;
        }
        ProcessInfo processInfo;
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesseeUdid == remoteUdid &&
            find(localUserIds.begin(), localUserIds.end(), accesserUserId) != localUserIds.end() &&
            find(remoteUserIds.begin(), remoteUserIds.end(), accesseeUserId) != remoteUserIds.end() &&
            item.GetStatus() == ACTIVE) {
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            bindLevel = std::min(bindLevel, static_cast<int32_t>(item.GetBindLevel()));
            processInfos.push_back(processInfo);
        } else if (accesseeUdid == localUdid && accesserUdid == remoteUdid &&
            find(localUserIds.begin(), localUserIds.end(), accesseeUserId) != localUserIds.end() &&
            find(remoteUserIds.begin(), remoteUserIds.end(), accesserUserId) != remoteUserIds.end() &&
            item.GetStatus() == ACTIVE) {
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            bindLevel = std::min(bindLevel, static_cast<int32_t>(item.GetBindLevel()));
            processInfos.push_back(processInfo);
        }
    }
    if (bindLevel == INVALIED_TYPE || bindLevel == USER) {
        processInfos.clear();
        for (const auto &item : localUserIds) {
            ProcessInfo processInfo;
            processInfo.pkgName = std::string(DM_PKG_NAME);
            processInfo.userId = item;
            processInfos.push_back(processInfo);
        }
    }
    return processInfos;
}

DM_EXPORT std::map<int32_t, int32_t> DeviceProfileConnector::GetUserIdAndBindLevel(
    const std::string &localUdid, const std::string &peerUdid)
{
    LOGI("localUdid %{public}s, peerUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(peerUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::map<int32_t, int32_t> userIdAndBindLevel;
    for (const auto &item : profiles) {
        if (IsLnnAcl(item)) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserid = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserid = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesseeUdid == peerUdid) {
            if (userIdAndBindLevel.find(accesserUserid) == userIdAndBindLevel.end()) {
                userIdAndBindLevel[accesserUserid] = static_cast<int32_t>(item.GetBindLevel());
            } else {
                userIdAndBindLevel[accesserUserid] =
                    std::min(static_cast<int32_t>(item.GetBindLevel()), userIdAndBindLevel[accesserUserid]);
            }
        } else if (accesseeUdid == localUdid && accesserUdid == peerUdid) {
            if (userIdAndBindLevel.find(accesseeUserid) == userIdAndBindLevel.end()) {
                userIdAndBindLevel[accesseeUserid] = static_cast<int32_t>(item.GetBindLevel());
            } else {
                userIdAndBindLevel[accesseeUserid] =
                    std::min(static_cast<int32_t>(item.GetBindLevel()), userIdAndBindLevel[accesseeUserid]);
            }
        }
    }
    return userIdAndBindLevel;
}

DM_EXPORT void DeviceProfileConnector::UpdateACL(std::string &localUdid,
    const std::vector<int32_t> &localUserIds, const std::string &remoteUdid,
    const std::vector<int32_t> &remoteFrontUserIds, const std::vector<int32_t> &remoteBackUserIds,
    DmOfflineParam &offlineParam)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (auto item : profiles) {
        // deleta signal trust acl.
        DeleteSigTrustACL(item, remoteUdid, remoteFrontUserIds, remoteBackUserIds, offlineParam);
        // update identical account userId.
        UpdatePeerUserId(item, localUdid, localUserIds, remoteUdid, remoteFrontUserIds);
    }
}

void DeviceProfileConnector::DeleteSigTrustACL(AccessControlProfile profile, const std::string &remoteUdid,
    const std::vector<int32_t> &remoteFrontUserIds, const std::vector<int32_t> &remoteBackUserIds,
    DmOfflineParam &offlineParam)
{
    LOGI("start.");
    std::string accesserUdid = profile.GetAccesser().GetAccesserDeviceId();
    std::string accesseeUdid = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t accesserUserid = profile.GetAccesser().GetAccesserUserId();
    int32_t accesseeUserid = profile.GetAccessee().GetAccesseeUserId();
    if (accesserUdid == remoteUdid && accesserUserid != 0 && accesserUserid != -1 &&
        find(remoteFrontUserIds.begin(), remoteFrontUserIds.end(), accesserUserid) == remoteFrontUserIds.end() &&
        find(remoteBackUserIds.begin(), remoteBackUserIds.end(), accesserUserid) == remoteBackUserIds.end()) {
        CacheAcerAclId(profile, offlineParam.needDelAclInfos);
        return;
    }
    if (accesseeUdid == remoteUdid && accesseeUserid != 0 && accesseeUserid != -1 &&
        find(remoteFrontUserIds.begin(), remoteFrontUserIds.end(), accesseeUserid) == remoteFrontUserIds.end() &&
        find(remoteBackUserIds.begin(), remoteBackUserIds.end(), accesseeUserid) == remoteBackUserIds.end()) {
        CacheAceeAclId(profile, offlineParam.needDelAclInfos);
        return;
    }
}

void DeviceProfileConnector::UpdatePeerUserId(AccessControlProfile profile, std::string &localUdid,
    const std::vector<int32_t> &localUserIds, const std::string &remoteUdid,
    const std::vector<int32_t> &remoteFrontUserIds)
{
    LOGI("start.");
    if (remoteFrontUserIds.empty() || localUserIds.empty()) {
        LOGI("remoteFrontUserIds or localUserIds is empty.");
        return;
    }
    std::string accesserUdid = profile.GetAccesser().GetAccesserDeviceId();
    std::string accesseeUdid = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t accesserUserid = profile.GetAccesser().GetAccesserUserId();
    int32_t accesseeUserid = profile.GetAccessee().GetAccesseeUserId();
    uint32_t bindType = profile.GetBindType();
    if (accesserUdid == localUdid && accesseeUdid == remoteUdid && bindType == DM_IDENTICAL_ACCOUNT &&
        find(localUserIds.begin(), localUserIds.end(), accesserUserid) != localUserIds.end() &&
        (accesseeUserid == 0 || accesseeUserid == -1)) {
        Accessee accessee = profile.GetAccessee();
        accessee.SetAccesseeUserId(remoteFrontUserIds[0]);
        profile.SetAccessee(accessee);
        DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(profile);
        return;
    }
}

DM_EXPORT std::multimap<std::string, int32_t> DeviceProfileConnector::GetDevIdAndUserIdByActHash(
    const std::string &localUdid, const std::string &peerUdid,
    int32_t peerUserId, const std::string &peerAccountHash)
{
    LOGI("localUdid %{public}s, peerUdid %{public}s, peerUserId %{public}d, peerAccountHash %{public}s.",
        GetAnonyString(localUdid).c_str(), GetAnonyString(peerUdid).c_str(), peerUserId, peerAccountHash.c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::multimap<std::string, int32_t> deviceIdMap;
    for (const auto &item : profiles) {
        std::string accesserAccountId = item.GetAccesser().GetAccesserAccountId();
        std::string accesseeAccountId = item.GetAccessee().GetAccesseeAccountId();
        char accesserAccountIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetAccountIdHash(accesserAccountId, reinterpret_cast<uint8_t *>(accesserAccountIdHash)) != DM_OK) {
            LOGE("GetAccountHash failed.");
            return deviceIdMap;
        }
        char accesseeAccountIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetAccountIdHash(accesseeAccountId, reinterpret_cast<uint8_t *>(accesseeAccountIdHash)) != DM_OK) {
            LOGE("GetAccountHash failed.");
            return deviceIdMap;
        }
        LOGI("accesserAccountIdHash %{public}s, accesseeAccountIdHash %{public}s", accesserAccountIdHash,
            accesseeAccountIdHash);
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserid = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserid = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesseeUdid == peerUdid && accesseeUserid == peerUserId &&
            std::string(accesseeAccountIdHash) == peerAccountHash) {
            deviceIdMap.insert(std::pair<std::string, int32_t>(accesserUdid, accesserUserid));
            continue;
        }
        if (accesseeUdid == localUdid && accesserUdid == peerUdid && accesserUserid == peerUserId &&
            std::string(accesserAccountIdHash) == peerAccountHash) {
            deviceIdMap.insert(std::pair<std::string, int32_t>(accesseeUdid, accesseeUserid));
            continue;
        }
    }
    return deviceIdMap;
}

DM_EXPORT std::multimap<std::string, int32_t> DeviceProfileConnector::GetDeviceIdAndUserId(
    const std::string &localUdid, int32_t localUserId)
{
    LOGI("localUdid %{public}s, userId %{public}d.", GetAnonyString(localUdid).c_str(), localUserId);
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::multimap<std::string, int32_t> deviceIdMap;
    for (const auto &item : profiles) {
        std::string accesserDeviceId = item.GetAccesser().GetAccesserDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        std::string accesseeDeviceId = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserDeviceId == localUdid && accesserUserId == localUserId) {
            if (!IsValueExist(deviceIdMap, accesseeDeviceId, accesseeUserId)) {
                deviceIdMap.insert(std::pair<std::string, int32_t>(accesseeDeviceId, accesseeUserId));
            }
            continue;
        }
        if (accesseeDeviceId == localUdid && accesseeUserId == localUserId) {
            if (!IsValueExist(deviceIdMap, accesserDeviceId, accesserUserId)) {
                deviceIdMap.insert(std::pair<std::string, int32_t>(accesserDeviceId, accesserUserId));
            }
            continue;
        }
    }
    return deviceIdMap;
}

DM_EXPORT void DeviceProfileConnector::HandleSyncBackgroundUserIdEvent(
    const std::vector<int32_t> &remoteUserIds, const std::string &remoteUdid,
    const std::vector<int32_t> &localUserIds, std::string &localUdid)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        std::string accesserDeviceId = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeDeviceId = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserDeviceId == localUdid && accesseeDeviceId == remoteUdid &&
            (find(remoteUserIds.begin(), remoteUserIds.end(), accesseeUserId) != remoteUserIds.end() ||
            find(localUserIds.begin(), localUserIds.end(), accesserUserId) == localUserIds.end())) {
            if (item.GetStatus() == ACTIVE) {
                item.SetStatus(INACTIVE);
                DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
            }
        } else if ((accesseeDeviceId == localUdid && accesserDeviceId == remoteUdid) &&
            (find(remoteUserIds.begin(), remoteUserIds.end(), accesserUserId) != remoteUserIds.end() ||
            find(localUserIds.begin(), localUserIds.end(), accesseeUserId) == localUserIds.end())) {
            if (item.GetStatus() == ACTIVE) {
                item.SetStatus(INACTIVE);
                DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
            }
        }
    }
}

DM_EXPORT void DeviceProfileConnector::HandleDeviceUnBind(int32_t bindType,
    const std::string &peerUdid, const std::string &localUdid, int32_t localUserId, const std::string &localAccountId)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    if (profiles.empty()) {
        LOGI("profiles is empty");
        return;
    }
    for (auto &item : profiles) {
        if ((item.GetBindType() == static_cast<uint32_t>(bindType)) &&
            item.GetTrustDeviceId() == peerUdid &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserUserId() == localUserId &&
            item.GetAccesser().GetAccesserAccountId() == localAccountId) {
            DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.GetAccessControlId());
        }
    }
}

DM_EXPORT int32_t DeviceProfileConnector::SubscribeDeviceProfileInited(
    sptr<DistributedDeviceProfile::IDpInitedCallback> dpInitedCallback)
{
    LOGI("In");
    if (dpInitedCallback == nullptr) {
        LOGE("dpInitedCallback is nullptr");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t ret = DistributedDeviceProfileClient::GetInstance().SubscribeDeviceProfileInited(
        DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, dpInitedCallback);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

DM_EXPORT int32_t DeviceProfileConnector::UnSubscribeDeviceProfileInited()
{
    LOGI("In");
    int32_t ret = DistributedDeviceProfileClient::GetInstance().UnSubscribeDeviceProfileInited(
        DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

DM_EXPORT int32_t DeviceProfileConnector::PutAllTrustedDevices(
    const std::vector<DistributedDeviceProfile::TrustedDeviceInfo> &deviceInfos)
{
    LOGI("In deviceInfos.size:%{public}zu", deviceInfos.size());
    int32_t ret = DistributedDeviceProfileClient::GetInstance().PutAllTrustedDevices(deviceInfos);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

DM_EXPORT int32_t DeviceProfileConnector::CheckDeviceInfoPermission(
    const std::string &localUdid, const std::string &peerDeviceId)
{
    LOGI("CheckDeviceInfoPermission Start.");
    int32_t localUserId = 0;
    uint32_t tempLocalTokenId = 0;
    MultipleUserConnector::GetTokenIdAndForegroundUserId(tempLocalTokenId, localUserId);
    int64_t localTokenId = static_cast<int64_t>(tempLocalTokenId);
    std::string localAccountId = MultipleUserConnector::GetOhosAccountIdByUserId(localUserId);
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(localUserId);
    for (auto &item : profiles) {
        if (IsLnnAcl(item)) {
            continue;
        }
        if (item.GetTrustDeviceId() == peerDeviceId) {
            if (item.GetBindType() == DM_IDENTICAL_ACCOUNT || item.GetBindLevel() == USER) {
                return DM_OK;
            }
        }
        int32_t profileUserId = item.GetAccesser().GetAccesserUserId();
        if (item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            (profileUserId == localUserId || profileUserId == -1 || profileUserId == 0) &&
            item.GetAccesser().GetAccesserTokenId() == localTokenId &&
            item.GetAccessee().GetAccesseeDeviceId() == peerDeviceId) {
            return DM_OK;
        }
        profileUserId = item.GetAccessee().GetAccesseeUserId();
        if (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            (profileUserId == localUserId || profileUserId == -1 || profileUserId == 0) &&
            item.GetAccessee().GetAccesseeTokenId() == localTokenId &&
            item.GetAccesser().GetAccesserDeviceId() == peerDeviceId) {
            return DM_OK;
        }
    }
    return ERR_DM_NO_PERMISSION;
}

int32_t DeviceProfileConnector::UpdateAclDeviceName(const std::string &udid, const std::string &newDeviceName)
{
    std::vector<AccessControlProfile> allProfile =
        DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    for (AccessControlProfile profile : allProfile) {
        Accessee acee = profile.GetAccessee();
        if (acee.GetAccesseeDeviceId() == udid) {
            acee.SetAccesseeDeviceName(newDeviceName);
            profile.SetAccessee(acee);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(profile);
            return DM_OK;
        }
        Accesser acer = profile.GetAccesser();
        if (acer.GetAccesserDeviceId() == udid) {
            acer.SetAccesserDeviceName(newDeviceName);
            profile.SetAccesser(acer);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(profile);
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

int32_t DeviceProfileConnector::PutLocalServiceInfo(
    const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo)
{
    int32_t ret = DistributedDeviceProfileClient::GetInstance().PutLocalServiceInfo(localServiceInfo);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::DeleteLocalServiceInfo(const std::string &bundleName, int32_t pinExchangeType)
{
    int32_t ret = DistributedDeviceProfileClient::GetInstance().DeleteLocalServiceInfo(bundleName, pinExchangeType);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::UpdateLocalServiceInfo(
    const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo)
{
    int32_t ret = DistributedDeviceProfileClient::GetInstance().UpdateLocalServiceInfo(localServiceInfo);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::GetLocalServiceInfoByBundleNameAndPinExchangeType(const std::string &bundleName,
    int32_t pinExchangeType, DistributedDeviceProfile::LocalServiceInfo &localServiceInfo)
{
    LOGI("getLocalServiceInfo bundleName = %{public}s, pinExchangeType = %{public}d",
        bundleName.c_str(), pinExchangeType);
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetLocalServiceInfoByBundleAndPinType(bundleName,
        pinExchangeType, localServiceInfo);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::PutSessionKey(int32_t userId, const std::vector<unsigned char> &sessionKeyArray,
    int32_t &sessionKeyId)
{
    if (sessionKeyArray.empty() || sessionKeyArray.size() > MAX_SESSION_KEY_LENGTH) {
        LOGE("SessionKey size invalid");
        return ERR_DM_FAILED;
    }
    int32_t ret = DistributedDeviceProfileClient::GetInstance().PutSessionKey(static_cast<uint32_t>(userId),
        sessionKeyArray, sessionKeyId);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::DeleteSessionKey(int32_t userId, int32_t sessionKeyId)
{
    int32_t ret =
        DistributedDeviceProfileClient::GetInstance().DeleteSessionKey(static_cast<uint32_t>(userId), sessionKeyId);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::GetSessionKey(int32_t userId, int32_t sessionKeyId,
    std::vector<unsigned char> &sessionKeyArray)
{
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetSessionKey(static_cast<uint32_t>(userId),
        sessionKeyId, sessionKeyArray);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

bool DeviceProfileConnector::CheckAclStatusNotMatch(const DistributedDeviceProfile::AccessControlProfile &profile,
    const std::string &localUdid, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    if ((profile.GetAccesser().GetAccesserDeviceId() == localUdid &&
        (find(backgroundUserIds.begin(), backgroundUserIds.end(), profile.GetAccesser().GetAccesserUserId()) !=
        backgroundUserIds.end()) && (profile.GetStatus() == ACTIVE)) ||
        (profile.GetAccessee().GetAccesseeDeviceId() == localUdid &&
        (find(backgroundUserIds.begin(), backgroundUserIds.end(), profile.GetAccessee().GetAccesseeUserId()) !=
        backgroundUserIds.end()) && (profile.GetStatus() == ACTIVE))) {
        return true;
    }
    if ((profile.GetAccesser().GetAccesserDeviceId() == localUdid &&
        (find(foregroundUserIds.begin(), foregroundUserIds.end(), profile.GetAccesser().GetAccesserUserId()) !=
        foregroundUserIds.end()) && profile.GetStatus() == INACTIVE) ||
        (profile.GetAccessee().GetAccesseeDeviceId() == localUdid &&
        (find(foregroundUserIds.begin(), foregroundUserIds.end(), profile.GetAccessee().GetAccesseeUserId()) !=
        foregroundUserIds.end()) && profile.GetStatus() == INACTIVE)) {
        return true;
    }
    return false;
}

bool DeviceProfileConnector::CheckAclStatusAndForegroundNotMatch(const std::string &localUdid,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    LOGI("CheckAclStatusAndForegroundNotMatch profiles size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (CheckAclStatusNotMatch(item, localUdid, foregroundUserIds, backgroundUserIds)) {
            return true;
        }
    }
    return false;
}

DM_EXPORT int32_t DeviceProfileConnector::HandleUserSwitched(
    const std::string &localUdid, const std::vector<std::string> &deviceVec,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("OnStart HandleUserSwitched");
    if (deviceVec.empty()) {
        LOGI("no remote device.");
        return DM_OK;
    }
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    std::vector<AccessControlProfile> activeProfiles;
    std::vector<AccessControlProfile> inActiveProfiles;
    for (auto &item : profiles) {
        if (std::find(deviceVec.begin(), deviceVec.end(), item.GetTrustDeviceId()) == deviceVec.end()) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            (find(backgroundUserIds.begin(), backgroundUserIds.end(),
            item.GetAccesser().GetAccesserUserId()) != backgroundUserIds.end())) ||
            (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            (find(backgroundUserIds.begin(), backgroundUserIds.end(),
            item.GetAccessee().GetAccesseeUserId()) != backgroundUserIds.end()))) {
            if (item.GetStatus() == ACTIVE) {
                item.SetStatus(INACTIVE);
                inActiveProfiles.push_back(item);
            }
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            (find(foregroundUserIds.begin(), foregroundUserIds.end(),
            item.GetAccesser().GetAccesserUserId()) != foregroundUserIds.end()) && item.GetStatus() == INACTIVE) || (
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            (find(foregroundUserIds.begin(), foregroundUserIds.end(),
            item.GetAccessee().GetAccesseeUserId()) != foregroundUserIds.end()) && item.GetStatus() == INACTIVE)) {
            item.SetStatus(ACTIVE);
            activeProfiles.push_back(item);
            continue;
        }
    }
    HandleUserSwitched(activeProfiles, inActiveProfiles);
    return DM_OK;
}

int32_t DeviceProfileConnector::HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid)
{
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        if ((item.GetAccesser().GetAccesserDeviceId() == stopEventUdid &&
            item.GetAccesser().GetAccesserUserId() == stopUserId && item.GetStatus() == ACTIVE) ||
            (item.GetAccessee().GetAccesseeDeviceId() == stopEventUdid &&
            item.GetAccessee().GetAccesseeUserId() == stopUserId && item.GetStatus() == ACTIVE)) {
            item.SetStatus(INACTIVE);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
        }
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid,
    const std::vector<std::string> &acceptEventUdids)
{
    if (acceptEventUdids.empty()) {
        LOGI("no remote device.");
        return DM_OK;
    }
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    for (auto &item : profiles) {
        if (std::find(acceptEventUdids.begin(), acceptEventUdids.end(), item.GetTrustDeviceId()) ==
            acceptEventUdids.end()) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == stopEventUdid &&
            item.GetAccesser().GetAccesserUserId() == stopUserId && item.GetStatus() == ACTIVE) ||
            (item.GetAccessee().GetAccesseeDeviceId() == stopEventUdid &&
            item.GetAccessee().GetAccesseeUserId() == stopUserId && item.GetStatus() == ACTIVE)) {
            item.SetStatus(INACTIVE);
            DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
        }
    }
    return DM_OK;
}

DM_EXPORT bool DeviceProfileConnector::IsLnnAcl(const DistributedDeviceProfile::AccessControlProfile &profile)
{
    std::string extraData = profile.GetExtraData();
    if (extraData.empty()) {
        return false;
    }

    JsonObject extJson(extraData);
    if (extJson.IsDiscarded() || !IsString(extJson, ACL_IS_LNN_ACL_KEY)) {
        return false;
    }

    std::string isLnnAclStr = extJson[ACL_IS_LNN_ACL_KEY].Get<std::string>();
    std::string isLnnAclTrue = std::string(ACL_IS_LNN_ACL_VAL_TRUE);
    return isLnnAclStr == isLnnAclTrue;
}

DM_EXPORT void DeviceProfileConnector::CacheAcerAclId(const DistributedDeviceProfile::AccessControlProfile &profile,
    std::vector<DmAclIdParam> &aclInfos)
{
    DmAclIdParam dmAclIdParam;
    dmAclIdParam.udid = profile.GetAccesser().GetAccesserDeviceId();
    dmAclIdParam.userId = profile.GetAccesser().GetAccesserUserId();
    dmAclIdParam.skId = profile.GetAccesser().GetAccesserSessionKeyId();
    dmAclIdParam.credId = profile.GetAccesser().GetAccesserCredentialIdStr();
    dmAclIdParam.accessControlId = profile.GetAccessControlId();
    aclInfos.push_back(dmAclIdParam);
}

DM_EXPORT void DeviceProfileConnector::CacheAceeAclId(const DistributedDeviceProfile::AccessControlProfile &profile,
    std::vector<DmAclIdParam> &aclInfos)
{
    DmAclIdParam dmAclIdParam;
    dmAclIdParam.udid = profile.GetAccessee().GetAccesseeDeviceId();
    dmAclIdParam.userId = profile.GetAccessee().GetAccesseeUserId();
    dmAclIdParam.skId = profile.GetAccessee().GetAccesseeSessionKeyId();
    dmAclIdParam.credId = profile.GetAccessee().GetAccesseeCredentialIdStr();
    dmAclIdParam.accessControlId = profile.GetAccessControlId();
    aclInfos.push_back(dmAclIdParam);
}

DM_EXPORT int32_t DeviceProfileConnector::HandleAccountCommonEvent(
    const std::string &localUdid, const std::vector<std::string> &deviceVec,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("HandleAccountCommonEvent start");
    if (deviceVec.empty()) {
        LOGI("no remote device.");
        return DM_OK;
    }
    std::vector<AccessControlProfile> profiles = GetAllAclIncludeLnnAcl();
    std::vector<AccessControlProfile> activeProfiles;
    std::vector<AccessControlProfile> inActiveProfiles;
    for (auto &item : profiles) {
        if (std::find(deviceVec.begin(), deviceVec.end(), item.GetTrustDeviceId()) == deviceVec.end()) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            (find(foregroundUserIds.begin(), foregroundUserIds.end(),
            item.GetAccesser().GetAccesserUserId()) == foregroundUserIds.end()) && item.GetStatus() == ACTIVE) ||
            (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            (find(foregroundUserIds.begin(), foregroundUserIds.end(),
            item.GetAccessee().GetAccesseeUserId()) == foregroundUserIds.end()) && item.GetStatus() == ACTIVE)) {
            item.SetStatus(INACTIVE);
            inActiveProfiles.push_back(item);
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            (find(foregroundUserIds.begin(), foregroundUserIds.end(),
            item.GetAccesser().GetAccesserUserId()) != foregroundUserIds.end()) && item.GetStatus() == INACTIVE) || (
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            (find(foregroundUserIds.begin(), foregroundUserIds.end(),
            item.GetAccessee().GetAccesseeUserId()) != foregroundUserIds.end()) && item.GetStatus() == INACTIVE)) {
            item.SetStatus(ACTIVE);
            activeProfiles.push_back(item);
            continue;
        }
    }
    for (auto &item : inActiveProfiles) {
        DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
    }
    for (auto &item : activeProfiles) {
        DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
    }
    return DM_OK;
}

DM_EXPORT bool DeviceProfileConnector::CheckSrcAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("srcUdid %{public}s, srcUserId %{public}d, srcPkgName %{public}s,"
        "srcAccountId %{public}s, sinkUdid %{public}s, sinkUserId %{public}d, sinkPkgName %{public}s,"
        "sinkAccountId %{public}s.", GetAnonyString(srcUdid).c_str(), caller.userId,
        caller.pkgName.c_str(), GetAnonyString(caller.accountId).c_str(),
        GetAnonyString(sinkUdid).c_str(), callee.userId, callee.pkgName.c_str(),
        GetAnonyString(callee.accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::string localUdid = GetLocalDeviceId();
    std::string trustUdid = (localUdid == srcUdid ? sinkUdid : srcUdid);
    for (const auto &item : profiles) {
        PrintProfile(item);
        if (item.GetTrustDeviceId() != trustUdid || item.GetStatus() != ACTIVE) {
            continue;
        }
        switch (item.GetBindType()) {
            case DM_IDENTICAL_ACCOUNT:
                if (CheckSrcAcuntAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
                    return true;
                }
                break;
            case DM_SHARE:
                if (CheckSrcShareAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
                    return true;
                }
                break;
            case DM_POINT_TO_POINT:
            case DM_ACROSS_ACCOUNT:
                if (CheckSrcP2PAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
                    return true;
                }
                break;
            default:
                break;
        }
    }
    return false;
}

bool DeviceProfileConnector::CheckSrcAcuntAccessControl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();
    std::string acerAccountId = profile.GetAccesser().GetAccesserAccountId();

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    std::string aceeAccountId = profile.GetAccessee().GetAccesseeAccountId();
    //bind type is identical account, accesser is caller, accessee is callee
    if (srcUdid == acerDeviceId && caller.userId == acerUserId && caller.accountId == acerAccountId &&
        sinkUdid == aceeDeviceId && acerAccountId == aceeAccountId) {
        return true;
    }
    //bind type is identical account, accessee is caller, accesser is callee
    if (srcUdid == aceeDeviceId && caller.userId == aceeUserId && caller.accountId == aceeAccountId &&
        sinkUdid == acerDeviceId && acerAccountId == aceeAccountId) {
        return true;
    }
    return false;
}

bool DeviceProfileConnector::CheckSrcShareAccessControl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();
    std::string acerAccountId = profile.GetAccesser().GetAccesserAccountId();

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();

    if (srcUdid == acerDeviceId && caller.userId == acerUserId && caller.accountId == acerAccountId &&
        sinkUdid == aceeDeviceId) {
        return true;
    }
    return false;
}

bool DeviceProfileConnector::CheckSrcP2PAccessControl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();
    int32_t acerTokenId = static_cast<int32_t>(profile.GetAccesser().GetAccesserTokenId());
    std::string acerPkgName = profile.GetAccesser().GetAccesserBundleName();

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    int32_t aceeTokenId = static_cast<int32_t>(profile.GetAccessee().GetAccesseeTokenId());
    std::string aceePkgName = profile.GetAccessee().GetAccesseeBundleName();

    uint32_t bindLevel = profile.GetBindLevel();
    switch (bindLevel) {
        case USER:
            //bind level is user, accesser is caller, accessee is callee
            if (srcUdid == acerDeviceId && caller.userId == acerUserId && sinkUdid == aceeDeviceId) {
                return true;
            }
            //bind level is user, accessee is caller, accesser is callee
            if (srcUdid == aceeDeviceId && caller.userId == aceeUserId && sinkUdid == acerDeviceId) {
                return true;
            }
            break;
        case SERVICE:
        case APP:
            //bind level is user, accesser is caller, accessee is callee
            if (srcUdid == acerDeviceId && caller.userId == acerUserId &&
                static_cast<int32_t>(caller.tokenId) == acerTokenId && sinkUdid == aceeDeviceId) {
                return true;
            }
            //bind level is user, accessee is caller, accesser is callee
            if (srcUdid == aceeDeviceId && caller.userId == aceeUserId &&
                static_cast<int32_t>(caller.tokenId) == aceeTokenId && sinkUdid == acerDeviceId) {
                return true;
            }
            break;
        default:
            LOGE("invalid bindLevel %{public}d.", bindLevel);
            break;
    }
    return false;
}

DM_EXPORT bool DeviceProfileConnector::CheckSinkAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("srcUdid %{public}s, srcUserId %{public}d, srcPkgName %{public}s,"
        "srcAccountId %{public}s, sinkUdid %{public}s, sinkUserId %{public}d, sinkPkgName %{public}s,"
        "sinkAccountId %{public}s.", GetAnonyString(srcUdid).c_str(), caller.userId,
        caller.pkgName.c_str(), GetAnonyString(caller.accountId).c_str(),
        GetAnonyString(sinkUdid).c_str(), callee.userId, callee.pkgName.c_str(),
        GetAnonyString(callee.accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::string localUdid = GetLocalDeviceId();
    std::string trustUdid = (localUdid == srcUdid ? sinkUdid : srcUdid);
    for (const auto &item : profiles) {
        PrintProfile(item);
        if (item.GetTrustDeviceId() != trustUdid || item.GetStatus() != ACTIVE) {
            continue;
        }
        switch (item.GetBindType()) {
            case DM_IDENTICAL_ACCOUNT:
                if (CheckSinkAcuntAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
                    return true;
                }
                break;
            case DM_SHARE:
                if (CheckSinkShareAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
                    return true;
                }
                break;
            case DM_POINT_TO_POINT:
            case DM_ACROSS_ACCOUNT:
                if (CheckSinkP2PAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
                    return true;
                }
                break;
            default:
                break;
        }
    }
    return false;
}

bool DeviceProfileConnector::CheckSinkAcuntAccessControl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();
    std::string acerAccountId = profile.GetAccesser().GetAccesserAccountId();

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    std::string aceeAccountId = profile.GetAccessee().GetAccesseeAccountId();

    //bind type is identical account, accesser is caller, accessee is callee
    if (srcUdid == acerDeviceId && caller.userId == acerUserId && caller.accountId == acerAccountId &&
        sinkUdid == aceeDeviceId && callee.userId == aceeUserId && callee.accountId == aceeAccountId &&
        caller.accountId == callee.accountId) {
        return true;
    }

    //bind type is identical account, accessee is caller, accesser is callee
    if (srcUdid == aceeDeviceId && caller.userId == aceeUserId && caller.accountId == aceeAccountId &&
        sinkUdid == acerDeviceId && callee.userId == acerUserId && callee.accountId == acerAccountId &&
        caller.accountId == callee.accountId) {
        return true;
    }
    return false;
}

bool DeviceProfileConnector::CheckSinkShareAccessControl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();
    std::string acerAccountId = profile.GetAccesser().GetAccesserAccountId();

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    std::string aceeAccountId = profile.GetAccessee().GetAccesseeAccountId();

    if (srcUdid == acerDeviceId && caller.userId == acerUserId && caller.accountId == acerAccountId &&
        sinkUdid == aceeDeviceId && callee.userId == aceeUserId && callee.accountId == aceeAccountId) {
        return true;
    }
    return false;
}

bool DeviceProfileConnector::CheckSinkP2PAccessControl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid)
{
    uint32_t bindLevel = profile.GetBindLevel();
    switch (bindLevel) {
        case USER:
            if (CheckSinkUserP2PAcl(profile, caller, srcUdid, callee, sinkUdid)) {
                return true;
            }
            break;
        case SERVICE:
        case APP:
            if (CheckSinkAppOrServiceP2PAcl(profile, caller, srcUdid, callee, sinkUdid)) {
                return true;
            }
            break;
        default:
            LOGE("invalid bindLevel %{public}d.", bindLevel);
            break;
    }
    return false;
}

bool DeviceProfileConnector::CheckSinkUserP2PAcl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee,
    const std::string &sinkUdid)
{
    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    //bind level is user, accesser is caller, accessee is callee
    if (srcUdid == acerDeviceId && caller.userId == acerUserId && sinkUdid == aceeDeviceId &&
        callee.userId == aceeUserId) {
        return true;
    }
    //bind level is user, accessee is caller, accesser is callee
    if (srcUdid == aceeDeviceId && caller.userId == aceeUserId && sinkUdid == acerDeviceId &&
        callee.userId == acerUserId) {
        return true;
    }
    return false;
}

bool DeviceProfileConnector::CheckSinkAppOrServiceP2PAcl(const DistributedDeviceProfile::AccessControlProfile &profile,
    const DmAccessCaller &caller, const std::string &srcUdid, const DmAccessCallee &callee,
    const std::string &sinkUdid)
{
    std::string acerDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    int32_t acerUserId = profile.GetAccesser().GetAccesserUserId();
    int32_t acerTokenId = static_cast<int32_t>(profile.GetAccesser().GetAccesserTokenId());

    std::string aceeDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t aceeUserId = profile.GetAccessee().GetAccesseeUserId();
    int32_t aceeTokenId = static_cast<int32_t>(profile.GetAccessee().GetAccesseeTokenId());
    //bind level is app or service, accesser is caller, accessee is callee
    if (srcUdid == acerDeviceId && caller.userId == acerUserId &&
        static_cast<int32_t>(caller.tokenId) == acerTokenId && sinkUdid == aceeDeviceId &&
        callee.userId == aceeUserId && static_cast<int32_t>(callee.tokenId) == aceeTokenId) {
        return true;
    }
    //bind level is app or service, accessee is caller, accesser is callee
    if (srcUdid == aceeDeviceId && caller.userId == aceeUserId &&
        static_cast<int32_t>(caller.tokenId) == aceeTokenId && sinkUdid == acerDeviceId &&
        callee.userId == acerUserId && static_cast<int32_t>(callee.tokenId) == acerTokenId) {
        return true;
    }
    return false;
}

DM_EXPORT bool DeviceProfileConnector::CheckSrcIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("srcUdid %{public}s, srcUserId %{public}d, srcPkgName %{public}s, "
        "srcAccountId %{public}s, sinkUdid %{public}s, sinkUserId %{public}d, sinkPkgName %{public}s,"
        "sinkAccountId %{public}s.", GetAnonyString(srcUdid).c_str(), caller.userId,
        caller.pkgName.c_str(), GetAnonyString(caller.accountId).c_str(),
        GetAnonyString(sinkUdid).c_str(), callee.userId, callee.pkgName.c_str(),
        GetAnonyString(callee.accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::string localUdid = GetLocalDeviceId();
    std::string trustUdid = (localUdid == srcUdid ? sinkUdid : srcUdid);
    for (const auto &item : profiles) {
        PrintProfile(item);
        if (item.GetTrustDeviceId() != trustUdid || item.GetStatus() != ACTIVE ||
            item.GetBindType() != DM_IDENTICAL_ACCOUNT) {
            continue;
        }
        if (CheckSrcAcuntAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
            return true;
        }
    }
    return false;
}

DM_EXPORT bool DeviceProfileConnector::CheckSinkIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("srcUdid %{public}s, srcUserId %{public}d, srcPkgName %{public}s,"
        "srcAccountId %{public}s, sinkUdid %{public}s, sinkUserId %{public}d, sinkPkgName %{public}s,"
        "sinkAccountId %{public}s.", GetAnonyString(srcUdid).c_str(), caller.userId,
        caller.pkgName.c_str(), GetAnonyString(caller.accountId).c_str(),
        GetAnonyString(sinkUdid).c_str(), callee.userId, callee.pkgName.c_str(),
        GetAnonyString(callee.accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::string localUdid = GetLocalDeviceId();
    std::string trustUdid = (localUdid == srcUdid ? sinkUdid : srcUdid);
    for (const auto &item : profiles) {
        PrintProfile(item);
        if (item.GetTrustDeviceId() != trustUdid || item.GetStatus() != ACTIVE ||
            item.GetBindType() != DM_IDENTICAL_ACCOUNT) {
            continue;
        }
        if (CheckSinkAcuntAccessControl(item, caller, srcUdid, callee, sinkUdid)) {
            return true;
        }
    }
    return false;
}

DM_EXPORT void DeviceProfileConnector::DeleteHoDevice(const std::string &peerUdid,
    const std::vector<int32_t> &foreGroundUserIds, const std::vector<int32_t> &backGroundUserIds)
{
    if (peerUdid.empty() || foreGroundUserIds.empty() || backGroundUserIds.empty()) {
        LOGE("invalid input param.");
        return;
    }
    std::vector<int32_t> localUserIds(foreGroundUserIds.begin(), foreGroundUserIds.end());
    std::copy(backGroundUserIds.begin(), backGroundUserIds.end(), std::back_inserter(localUserIds));
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::string localUdid = GetLocalDeviceId();
    for (const auto &item : profiles) {
        if (peerUdid != item.GetTrustDeviceId() || item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            continue;
        }
        std::string acerDeviceId = item.GetAccesser().GetAccesserDeviceId();
        int32_t acerUserId = item.GetAccesser().GetAccesserUserId();
        std::string acerPkgName = item.GetAccesser().GetAccesserBundleName();
        std::string aceeDeviceId = item.GetAccessee().GetAccesseeDeviceId();
        int32_t aceeUserId = item.GetAccessee().GetAccesseeUserId();
        std::string aceePkgName = item.GetAccessee().GetAccesseeBundleName();

        if (localUdid == acerDeviceId && peerUdid == aceeDeviceId && !CheckExtWhiteList(acerPkgName) &&
            std::find(localUserIds.begin(), localUserIds.end(), acerUserId) != localUserIds.end()) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            continue;
        }
        if (peerUdid == acerDeviceId && localUdid == aceeDeviceId && !CheckExtWhiteList(aceePkgName) &&
            std::find(localUserIds.begin(), localUserIds.end(), aceeUserId) != localUserIds.end()) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            continue;
        }
    }
}

bool DeviceProfileConnector::CheckExtWhiteList(const std::string &pkgName)
{
    LOGI("start pkgName %{public}s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("bundleName empty.");
        return false;
    }
    for (uint32_t index = 0 ; index < AUTH_EXT_WHITE_LIST_NUM ; index++) {
        if (pkgName == g_extWhiteList[index]) {
            return true;
        }
    }
    return false;
}

bool DeviceProfileConnector::IsAllowAuthAlways(const std::string &localUdid, int32_t userId,
    const std::string &peerUdid, const std::string &pkgName, int64_t tokenId)
{
    auto acls = GetAclProfileByDeviceIdAndUserId(localUdid, userId, peerUdid);
    for (const auto &item : acls) {
        auto accesser = item.GetAccesser();
        auto accessee = item.GetAccessee();
        if (item.GetAuthenticationType() == ALLOW_AUTH_ALWAYS &&
            ((accesser.GetAccesserBundleName() == pkgName && accesser.GetAccesserTokenId() == tokenId) ||
            (accessee.GetAccesseeBundleName() == pkgName && accessee.GetAccesseeTokenId() == tokenId))) {
            return true;
        }
    }
    return false;
}

IDeviceProfileConnector *CreateDpConnectorInstance()
{
    return &DeviceProfileConnector::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
