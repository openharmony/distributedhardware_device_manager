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

#include "deviceprofile_connector.h"
#include "crypto_mgr.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "multiple_user_connector.h"
#include "distributed_device_profile_client.h"
#include "system_ability_definition.h"

using namespace OHOS::DistributedDeviceProfile;

const uint32_t INVALIED_TYPE = 0;
const uint32_t APP_PEER_TO_PEER_TYPE = 1;
const uint32_t APP_ACROSS_ACCOUNT_TYPE = 2;
const uint32_t DEVICE_PEER_TO_PEER_TYPE = 3;
const uint32_t DEVICE_ACROSS_ACCOUNT_TYPE = 4;
const uint32_t IDENTICAL_ACCOUNT_TYPE = 5;

const uint32_t DM_IDENTICAL_ACCOUNT = 1;
const uint32_t DM_POINT_TO_POINT = 256;
const uint32_t DM_ACROSS_ACCOUNT = 1282;
const uint32_t DM_INVALIED_BINDTYPE = 2048;
const uint32_t DEVICE = 1;
const uint32_t SERVICE = 2;
const uint32_t APP = 3;
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector);
__attribute__ ((visibility ("default")))
std::vector<AccessControlProfile> DeviceProfileConnector::GetAccessControlProfile()
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

__attribute__ ((visibility ("default")))
std::unordered_map<std::string, DmAuthForm> DeviceProfileConnector::GetAppTrustDeviceList(const std::string &pkgName,
    const std::string &deviceId)
{
    int32_t userId = MultipleUserConnector::GetFirstForegroundUserId();
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(deviceId, userId);
    std::vector<AccessControlProfile> profilesFilter = {};
    for (auto &item : profiles) {
        if ((item.GetAccesser().GetAccesserUserId() == userId &&
             item.GetAccesser().GetAccesserDeviceId() == deviceId) ||
            (item.GetAccessee().GetAccesseeUserId() == userId &&
             item.GetAccessee().GetAccesseeDeviceId() == deviceId)) {
            profilesFilter.push_back(item);
        }
    }
    std::unordered_map<std::string, DmAuthForm> deviceIdMap;
    for (auto &item : profilesFilter) {
        std::string trustDeviceId = item.GetTrustDeviceId();
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
            deviceIdMap[trustDeviceId] = static_cast<DmAuthForm>(bindType);
            continue;
        }
        DmAuthForm authForm = deviceIdMap.at(trustDeviceId);
        if (bindType == authForm) {
            continue;
        }
        if (bindType == DmAuthForm::IDENTICAL_ACCOUNT) {
            deviceIdMap[trustDeviceId] = DmAuthForm::IDENTICAL_ACCOUNT;
            continue;
        }
        if (bindType == DmAuthForm::PEER_TO_PEER && authForm == DmAuthForm::ACROSS_ACCOUNT) {
            deviceIdMap[trustDeviceId] = DmAuthForm::PEER_TO_PEER;
            continue;
        }
    }
    return deviceIdMap;
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
        if (static_cast<std::string>(deviceIdHash) != discoveryInfo.remoteDeviceIdHash ||
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

int32_t DeviceProfileConnector::CheckAuthForm(DmAuthForm form, AccessControlProfile profiles,
    DmDiscoveryInfo discoveryInfo)
{
    if (profiles.GetBindLevel() == DEVICE || (profiles.GetBindLevel() == APP && discoveryInfo.pkgname == "")) {
        return form;
    }
    if (profiles.GetBindLevel() == APP) {
        if (discoveryInfo.pkgname == profiles.GetAccesser().GetAccesserBundleName() &&
            discoveryInfo.localDeviceId == profiles.GetAccesser().GetAccesserDeviceId()) {
            return form;
        }
        if (discoveryInfo.pkgname == profiles.GetAccessee().GetAccesseeBundleName() &&
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
        return CheckAuthForm(DmAuthForm::ACROSS_ACCOUNT, profiles, discoveryInfo);
    }
    return DmAuthForm::INVALID_TYPE;
}

__attribute__ ((visibility ("default")))uint32_t DeviceProfileConnector::CheckBindType(std::string peerUdid,
    std::string localUdid)
{
    std::vector<AccessControlProfile> filterProfiles = GetAclProfileByUserId(localUdid,
        MultipleUserConnector::GetFirstForegroundUserId(), peerUdid);
    LOGI("filterProfiles size is %{public}zu", filterProfiles.size());
    uint32_t highestPriority = INVALIED_TYPE;
    for (auto &item : filterProfiles) {
        if (peerUdid != item.GetTrustDeviceId()) {
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
        case DM_POINT_TO_POINT:
            if (profiles.GetBindLevel() == DEVICE) {
                priority = DEVICE_PEER_TO_PEER_TYPE;
            } else if (profiles.GetBindLevel() == APP) {
                priority = APP_PEER_TO_PEER_TYPE;
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profiles.GetBindLevel() == DEVICE) {
                priority = DEVICE_ACROSS_ACCOUNT_TYPE;
            } else if (profiles.GetBindLevel() == APP) {
                priority = APP_ACROSS_ACCOUNT_TYPE;
            }
            break;
        default:
            LOGE("unknown bind type %{public}d.", bindType);
            break;
    }
    return priority;
}

__attribute__ ((visibility ("default")))std::vector<int32_t> DeviceProfileConnector::GetBindTypeByPkgName(
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
        if (trustUdid != item.GetTrustDeviceId() || item.GetStatus() != ACTIVE) {
            continue;
        }
        GetParamBindTypeVec(item, requestDeviceId, bindTypeVec, trustUdid);
    }
    return bindTypeVec;
}

__attribute__ ((visibility ("default")))uint64_t DeviceProfileConnector::GetTokenIdByNameAndDeviceId(
    std::string pkgName, std::string requestDeviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    uint64_t peerTokenId = 0;
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
            if (profile.GetBindLevel() == DEVICE) {
                bindTypeVec.push_back(DEVICE_PEER_TO_PEER_TYPE);
            }
            if (profile.GetBindLevel() == APP) {
                bindTypeVec.push_back(APP_PEER_TO_PEER_TYPE);
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profile.GetBindLevel() == DEVICE) {
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
        if (profiles[index].GetTrustDeviceId() != targetDeviceId || profiles[index].GetStatus() != ACTIVE) {
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
        if (profiles.GetBindLevel() == DEVICE) {
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
        if (profiles.GetBindLevel() == DEVICE) {
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

__attribute__ ((visibility ("default")))std::vector<int32_t> DeviceProfileConnector::SyncAclByBindType(
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

__attribute__ ((visibility ("default")))
std::vector<OHOS::DistributedHardware::ProcessInfo> DeviceProfileConnector::GetProcessInfoFromAclByUserId(
    const std::string &localDeviceId, const std::string &targetDeviceId, int32_t userId)
{
    std::vector<AccessControlProfile> filterProfiles = GetAclProfileByUserId(localDeviceId,
        userId, targetDeviceId);
    LOGI("filterProfiles size is %{public}zu", filterProfiles.size());
    std::vector<OHOS::DistributedHardware::ProcessInfo> processInfoVec;
    for (auto &item : filterProfiles) {
        if (item.GetTrustDeviceId() != targetDeviceId) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        OHOS::DistributedHardware::ProcessInfo processInfo;
        if (accesserUdid == localDeviceId) {
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            processInfoVec.push_back(processInfo);
            continue;
        }
        if (accesseeUdid == localDeviceId) {
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            processInfoVec.push_back(processInfo);
            continue;
        }
    }
    return processInfoVec;
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

__attribute__ ((visibility ("default")))bool DeviceProfileConnector::DeleteAclForAccountLogOut(
    const std::string &localUdid, int32_t localUserId,
    const std::string &peerUdid, int32_t peerUserId)
{
    LOGI("localUdid %{public}s, localUserId %{public}d, peerUdid %{public}s, peerUserId %{public}d.",
        GetAnonyString(localUdid).c_str(), localUserId, GetAnonyString(peerUdid).c_str(), peerUserId);
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<AccessControlProfile> deleteProfiles;
    bool notifyOffline = false;
    bool isDelete = false;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != peerUdid) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesserUserId == localUserId &&
            accesseeUdid == peerUdid && accesseeUserId == peerUserId) {
            if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
                isDelete = true;
            }
            deleteProfiles.push_back(item);
            notifyOffline = (item.GetStatus() == ACTIVE);
            continue;
        }
        if (accesserUdid == peerUdid && accesserUserId == peerUserId &&
            accesseeUdid == localUdid && accesseeUserId == localUserId) {
            if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
                isDelete = true;
            }
            deleteProfiles.push_back(item);
            notifyOffline = (item.GetStatus() == ACTIVE);
            continue;
        }
    }
    if (!isDelete) {
        return false;
    }
    for (const auto &item : deleteProfiles) {
        DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
    }
    return notifyOffline;
}

__attribute__ ((visibility ("default")))void DeviceProfileConnector::DeleteAclForUserRemoved(std::string localUdid,
    int32_t userId)
{
    LOGI("localUdid %{public}s, userId %{public}d.", GetAnonyString(localUdid).c_str(), userId);
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(userId);
    for (const auto &item : profiles) {
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if ((accesserUdid == localUdid && accesserUserId == userId) ||
            (accesseeUdid == localUdid && accesseeUserId == userId)) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
}

__attribute__ ((visibility ("default")))void DeviceProfileConnector::DeleteAclForRemoteUserRemoved(
    std::string peerUdid, int32_t peerUserId, std::vector<int32_t> &userIds)
{
    LOGI("peerUdid %{public}s, peerUserId %{public}d.", GetAnonyString(peerUdid).c_str(), peerUserId);
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(peerUserId);
    for (const auto &item : profiles) {
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == peerUdid && accesserUserId == peerUserId) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            if (item.GetBindLevel() == DEVICE) {
                userIds.push_back(accesseeUserId);
            }
        }
        if (accesseeUdid == peerUdid && accesseeUserId == peerUserId) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            if (item.GetBindLevel() == DEVICE) {
                userIds.push_back(accesserUserId);
            }
        }
    }
}

__attribute__ ((visibility ("default")))void DeviceProfileConnector::DeleteAccessControlList(const std::string &udid)
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
    if (static_cast<uint32_t>(bindLevel) > APP || static_cast<uint32_t>(bindLevel) < DEVICE) {
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
        case DEVICE:
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
            LOGI("Sink delete acl pkgName %{public}s, bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s",
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
    LOGI("DeviceProfileConnector::DeleteAppBindLevel extra %{public}s", extra.c_str());
    int32_t bindNums = 0;
    int32_t deleteNums = 0;
    std::string peerBundleName = extra;
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
            LOGI("Sink delete acl pkgName %{public}s, bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s",
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
            offlineParam.bindType = DEVICE;
            LOGI("Src delete acl bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s", item.GetBindType(),
                GetAnonyString(localUdid).c_str(), GetAnonyString(remoteUdid).c_str());
            continue;
        }
        if (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            deleteNums++;
            offlineParam.bindType = DEVICE;
            LOGI("Sink delete acl bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s", item.GetBindType(),
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
            LOGI("Sink delete acl pkgName %{public}s, bindType %{public}d, localUdid %{public}s, remoteUdid %{public}s",
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

__attribute__ ((visibility ("default")))bool DeviceProfileConnector::CheckSrcDevIdInAclForDevBind(
    const std::string &pkgName, const std::string &deviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == deviceId && item.GetStatus() == ACTIVE &&
            item.GetBindLevel() == DEVICE && (item.GetAccessee().GetAccesseeBundleName() == pkgName ||
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
            item.GetBindLevel() == DEVICE && (item.GetAccesser().GetAccesserBundleName() == pkgName ||
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

__attribute__ ((visibility ("default")))uint32_t DeviceProfileConnector::DeleteTimeOutAcl(const std::string &deviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    uint32_t res = 0;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != deviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        res++;
        if (item.GetAuthenticationType() == ALLOW_AUTH_ONCE) {
            res--;
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
    return res;
}

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::GetTrustNumber(const std::string &deviceId)
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::IsSameAccount(const std::string &udid)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == udid && item.GetStatus() == ACTIVE) {
            if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {  // 同账号
                LOGI("The udid %{public}s is identical bind.", GetAnonyString(udid).c_str());
                return DM_OK;
            }
        }
    }
    return ERR_DM_FAILED;
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::CheckAccessControl(
    const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("PkgName = %{public}s, srcUdid = %{public}s, sinkUdid = %{public}s",
        caller.pkgName.c_str(), GetAnonyString(srcUdid).c_str(), GetAnonyString(sinkUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<AccessControlProfile> profilesFilter =
        GetACLByDeviceIdAndUserId(profiles, caller, srcUdid, callee, sinkUdid);
    for (auto &item : profilesFilter) {
        if (item.GetStatus() != ACTIVE || (item.GetTrustDeviceId() != sinkUdid &&
            item.GetTrustDeviceId() != srcUdid)) {
            continue;
        }
        if (SingleUserProcess(item, caller, callee)) {
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

bool DeviceProfileConnector::CheckIdenticalAccount(int32_t userId, const std::string &accountId)
{
    std::vector<AccessControlProfile> profiles;
    std::map<std::string, std::string> queryParams;
    queryParams[USERID] = std::to_string(userId);
    queryParams[ACCOUNTID] = accountId;
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
    }
    for (auto &item : profiles) {
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT && item.GetStatus() == ACTIVE) {
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
            if (profile.GetBindLevel() == DEVICE || profile.GetBindLevel() == SERVICE) {
                ret = true;
            } else if (profile.GetBindLevel() == APP &&
                (profile.GetAccesser().GetAccesserBundleName() == caller.pkgName ||
                profile.GetAccessee().GetAccesseeBundleName() == caller.pkgName)) {
                ret = CheckAppLevelAccess(profile, caller, callee);
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profile.GetBindLevel() == DEVICE || profile.GetBindLevel() == SERVICE) {
                ret = true;
            } else if (profile.GetBindLevel() == APP &&
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::CheckIsSameAccount(
    const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("DeviceProfileConnector::CheckIsSameAccount pkgName %{public}s, srcUdid %{public}s, sinkUdid %{public}s",
        caller.pkgName.c_str(), GetAnonyString(srcUdid).c_str(), GetAnonyString(sinkUdid).c_str());
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
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::GetBindLevel(const std::string &pkgName,
    const std::string &localUdid, const std::string &udid, uint64_t &tokenId)
{
    LOGI("pkgName %{public}s, tokenId %{public}" PRId64", udid %{public}s.", pkgName.c_str(),
        tokenId, GetAnonyString(udid).c_str());
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
            LOGI("Src get bindLevel %{public}d, tokenid %{public}" PRId64".", bindLevel, tokenId);
            continue;
        }
        if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == udid) {
            tokenId = static_cast<uint64_t>(item.GetAccessee().GetAccesseeTokenId());
            bindLevel = static_cast<int32_t>(item.GetBindLevel());
            LOGI("Sink get bindLevel %{public}d, tokenid %{public}" PRId64".", bindLevel, tokenId);
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

__attribute__ ((visibility ("default")))
std::multimap<std::string, int32_t> DeviceProfileConnector::GetDeviceIdAndUserId(
    int32_t userId, const std::string &accountId, const std::string &localUdid)
{
    LOGI("localUdid %{public}s, userId %{public}d, accountId %{public}s.", GetAnonyString(localUdid).c_str(),
        userId, GetAnonyString(accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::multimap<std::string, int32_t> deviceIdMap;
    for (const auto &item : profiles) {
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserId = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserId = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesserUserId == userId && item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            LOGI("Account logout trust udid %{public}s userid %{public}d is src.",
                GetAnonyString(accesseeUdid).c_str(), accesseeUserId);
            deviceIdMap.insert(std::pair<std::string, int32_t>(accesseeUdid, accesseeUserId));
            continue;
        }
        if (accesseeUdid == localUdid && accesseeUserId == userId && item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
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
    int32_t bindType = DM_INVALIED_BINDTYPE;
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::HandleDevUnBindEvent(int32_t remoteUserId,
    const std::string &remoteUdid, const std::string &localUdid)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, localUdid %{public}s.", remoteUserId,
        GetAnonyString(remoteUdid).c_str(), GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAclProfileByDeviceIdAndUserId(remoteUdid, remoteUserId);
    int32_t bindType = DM_INVALIED_BINDTYPE;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            bindType = DM_IDENTICAL_ACCOUNT;
            continue;
        }
        DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        bindType = std::min(bindType, static_cast<int32_t>(item.GetBindType()));
    }
    return bindType;
}

__attribute__ ((visibility ("default")))
OHOS::DistributedHardware::ProcessInfo DeviceProfileConnector::HandleAppUnBindEvent(int32_t remoteUserId,
    const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, tokenId %{public}d, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteUdid).c_str(), tokenId, GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    ProcessInfo processInfo;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid || item.GetBindType() == DM_IDENTICAL_ACCOUNT ||
            item.GetBindLevel() != APP) {
            continue;
        }
        if (item.GetAccesser().GetAccesserUserId() == remoteUserId &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
            static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId()) == tokenId &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            LOGI("Src device unbind.");
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            continue;
        }
        if (item.GetAccessee().GetAccesseeUserId() == remoteUserId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid &&
            static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId()) == tokenId &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid) {
            LOGI("Sink device unbind.");
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            continue;
        }
    }
    return processInfo;
}

__attribute__ ((visibility ("default")))
OHOS::DistributedHardware::ProcessInfo DeviceProfileConnector::HandleAppUnBindEvent(int32_t remoteUserId,
    const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid, int32_t peerTokenId)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, tokenId %{public}d, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteUdid).c_str(), tokenId, GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    ProcessInfo processInfo;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid || item.GetBindType() == DM_IDENTICAL_ACCOUNT ||
            item.GetBindLevel() != APP) {
            continue;
        }
        if (item.GetAccesser().GetAccesserUserId() == remoteUserId &&
            item.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
            static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId()) == tokenId &&
            static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId()) == peerTokenId &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            LOGI("Src device unbind.");
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            processInfo.pkgName = item.GetAccessee().GetAccesseeBundleName();
            processInfo.userId = item.GetAccessee().GetAccesseeUserId();
            continue;
        }
        if (item.GetAccessee().GetAccesseeUserId() == remoteUserId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid &&
            static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId()) == tokenId &&
            static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId()) == peerTokenId &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid) {
            LOGI("Sink device unbind.");
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            processInfo.pkgName = item.GetAccesser().GetAccesserBundleName();
            processInfo.userId = item.GetAccesser().GetAccesserUserId();
            continue;
        }
    }
    return processInfo;
}

__attribute__ ((visibility ("default")))
std::vector<AccessControlProfile> DeviceProfileConnector::GetAllAccessControlProfile()
{
    std::vector<AccessControlProfile> profiles;
    if (DistributedDeviceProfileClient::GetInstance().GetAllAccessControlProfile(profiles) != DM_OK) {
        LOGE("DP failed.");
    }
    return profiles;
}

__attribute__ ((visibility ("default")))void DeviceProfileConnector::DeleteAccessControlById(
    int64_t accessControlId)
{
    DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(accessControlId);
}

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::HandleUserSwitched(
    const std::string &localUdid, const std::vector<std::string> &deviceVec, int32_t currentUserId,
    int32_t beforeUserId)
{
    if (deviceVec.empty()) {
        LOGI("no remote device.");
        return DM_OK;
    }
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<AccessControlProfile> activeProfiles;
    std::vector<AccessControlProfile> inActiveProfiles;
    std::vector<AccessControlProfile> delActiveProfiles;
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
            if (item.GetBindType() != DM_IDENTICAL_ACCOUNT) {
                delActiveProfiles.push_back(item);
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
    HandleUserSwitched(activeProfiles, inActiveProfiles, delActiveProfiles);
    return DM_OK;
}

__attribute__ ((visibility ("default")))void DeviceProfileConnector::HandleUserSwitched(
    const std::vector<AccessControlProfile> &activeProfiles,
    const std::vector<AccessControlProfile> &inActiveProfiles,
    const std::vector<AccessControlProfile> &delActiveProfiles)
{
    for (auto &item : inActiveProfiles) {
        DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
    }
    for (auto &item : delActiveProfiles) {
        DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
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

__attribute__ ((visibility ("default")))void DeviceProfileConnector::HandleSyncForegroundUserIdEvent(
    const std::vector<int32_t> &remoteUserIds, const std::string &remoteUdid,
    const std::vector<int32_t> &localUserIds, std::string &localUdid)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
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
    if (bindLevel == INVALIED_TYPE || bindLevel == DEVICE) {
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

__attribute__ ((visibility ("default")))std::map<int32_t, int32_t> DeviceProfileConnector::GetUserIdAndBindLevel(
    const std::string &localUdid, const std::string &peerUdid)
{
    LOGI("localUdid %{public}s, peerUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(peerUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::map<int32_t, int32_t> userIdAndBindLevel;
    for (const auto &item : profiles) {
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

__attribute__ ((visibility ("default")))void DeviceProfileConnector::UpdateACL(std::string &localUdid,
    const std::vector<int32_t> &localUserIds, const std::string &remoteUdid,
    const std::vector<int32_t> &remoteFrontUserIds, const std::vector<int32_t> &remoteBackUserIds)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    for (auto item : profiles) {
        // deleta signal trust acl.
        DeleteSigTrustACL(item, remoteUdid, remoteFrontUserIds, remoteBackUserIds);
        // update identical account userId.
        UpdatePeerUserId(item, localUdid, localUserIds, remoteUdid, remoteFrontUserIds);
    }
}

void DeviceProfileConnector::DeleteSigTrustACL(AccessControlProfile profile, const std::string &remoteUdid,
    const std::vector<int32_t> &remoteFrontUserIds, const std::vector<int32_t> &remoteBackUserIds)
{
    LOGI("start.");
    std::string accesserUdid = profile.GetAccesser().GetAccesserDeviceId();
    std::string accesseeUdid = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t accesserUserid = profile.GetAccesser().GetAccesserUserId();
    int32_t accesseeUserid = profile.GetAccessee().GetAccesseeUserId();
    if (accesserUdid == remoteUdid && accesserUserid != 0 && accesserUserid != -1 &&
        find(remoteFrontUserIds.begin(), remoteFrontUserIds.end(), accesserUserid) == remoteFrontUserIds.end() &&
        find(remoteBackUserIds.begin(), remoteBackUserIds.end(), accesserUserid) == remoteBackUserIds.end()) {
        DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(profile.GetAccessControlId());
        return;
    }
    if (accesseeUdid == remoteUdid && accesseeUserid != 0 && accesseeUserid != -1 &&
        find(remoteFrontUserIds.begin(), remoteFrontUserIds.end(), accesseeUserid) == remoteFrontUserIds.end() &&
        find(remoteBackUserIds.begin(), remoteBackUserIds.end(), accesseeUserid) == remoteBackUserIds.end()) {
        DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(profile.GetAccessControlId());
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

__attribute__ ((visibility ("default")))
std::multimap<std::string, int32_t> DeviceProfileConnector::GetDevIdAndUserIdByActHash(const std::string &localUdid,
    const std::string &peerUdid, int32_t peerUserId, const std::string &peerAccountHash)
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
        if (std::string(accesserAccountIdHash) != peerAccountHash ||
            std::string(accesseeAccountIdHash) != peerAccountHash) {
            continue;
        }
        std::string accesserUdid = item.GetAccesser().GetAccesserDeviceId();
        std::string accesseeUdid = item.GetAccessee().GetAccesseeDeviceId();
        int32_t accesserUserid = item.GetAccesser().GetAccesserUserId();
        int32_t accesseeUserid = item.GetAccessee().GetAccesseeUserId();
        if (accesserUdid == localUdid && accesseeUdid == peerUdid && accesseeUserid == peerUserId) {
            deviceIdMap.insert(std::pair<std::string, int32_t>(accesserUdid, accesserUserid));
            continue;
        }
        if (accesseeUdid == localUdid && accesserUdid == peerUdid && accesserUserid == peerUserId) {
            deviceIdMap.insert(std::pair<std::string, int32_t>(accesseeUdid, accesseeUserid));
            continue;
        }
    }
    return deviceIdMap;
}

__attribute__ ((visibility ("default")))std::multimap<std::string, int32_t> DeviceProfileConnector::GetDeviceIdAndUserId(
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

__attribute__ ((visibility ("default")))void DeviceProfileConnector::HandleSyncBackgroundUserIdEvent(
    const std::vector<int32_t> &remoteUserIds, const std::string &remoteUdid,
    const std::vector<int32_t> &localUserIds, std::string &localUdid)
{
    LOGI("localUdid %{public}s, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(),
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
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
            if (item.GetBindType() != DM_IDENTICAL_ACCOUNT) {
                DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            }
        } else if ((accesseeDeviceId == localUdid && accesserDeviceId == remoteUdid) &&
            (find(remoteUserIds.begin(), remoteUserIds.end(), accesserUserId) != remoteUserIds.end() ||
            find(localUserIds.begin(), localUserIds.end(), accesseeUserId) == localUserIds.end())) {
            if (item.GetStatus() == ACTIVE) {
                item.SetStatus(INACTIVE);
                DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(item);
            }
            if (item.GetBindType() != DM_IDENTICAL_ACCOUNT) {
                DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            }
        }
    }
}

__attribute__ ((visibility ("default")))void DeviceProfileConnector::HandleDeviceUnBind(int32_t bindType,
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::SubscribeDeviceProfileInited(
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::UnSubscribeDeviceProfileInited()
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::PutAllTrustedDevices(
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

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::CheckDeviceInfoPermission(
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
        if (item.GetTrustDeviceId() == peerDeviceId) {
            if (item.GetBindType() == DM_IDENTICAL_ACCOUNT || item.GetBindLevel() == DEVICE) {
                return DM_OK;
            }
        }
        int32_t profileUserId = item.GetAccesser().GetAccesserUserId();
        if (item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            (profileUserId == localUserId || profileUserId == -1 || profileUserId == 0) &&
            item.GetAccesser().GetAccesserAccountId() == localAccountId &&
            item.GetAccesser().GetAccesserTokenId() == localTokenId &&
            item.GetAccessee().GetAccesseeDeviceId() == peerDeviceId) {
            return DM_OK;
        }
        profileUserId = item.GetAccessee().GetAccesseeUserId();
        if (item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            (profileUserId == localUserId || profileUserId == -1 || profileUserId == 0) &&
            item.GetAccessee().GetAccesseeAccountId() == localAccountId &&
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
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetLocalServiceInfoByBundleAndPinType(bundleName,
        pinExchangeType, localServiceInfo);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::PutSessionKey(const std::vector<unsigned char> &sessionKeyArray, int32_t &sessionKeyId)
{
    if (sessionKeyArray.empty() || sessionKeyArray.size() > MAX_SESSION_KEY_LENGTH) {
        LOGE("SessionKey size invalid");
        return ERR_DM_FAILED;
    }
    uint32_t userId = static_cast<uint32_t>(MultipleUserConnector::GetCurrentAccountUserID());
    int32_t ret = DistributedDeviceProfileClient::GetInstance().PutSessionKey(userId, sessionKeyArray, sessionKeyId);
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
        backgroundUserIds.end()) &&
        (profile.GetStatus() == ACTIVE || profile.GetBindType() != DM_IDENTICAL_ACCOUNT)) ||
        (profile.GetAccessee().GetAccesseeDeviceId() == localUdid &&
        (find(backgroundUserIds.begin(), backgroundUserIds.end(), profile.GetAccessee().GetAccesseeUserId()) !=
        backgroundUserIds.end()) &&
        (profile.GetStatus() == ACTIVE || profile.GetBindType() != DM_IDENTICAL_ACCOUNT))) {
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
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    LOGI("CheckAclStatusAndForegroundNotMatch profiles size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (CheckAclStatusNotMatch(item, localUdid, foregroundUserIds, backgroundUserIds)) {
            return true;
        }
    }
    return false;
}

__attribute__ ((visibility ("default")))int32_t DeviceProfileConnector::HandleUserSwitched(
    const std::string &localUdid, const std::vector<std::string> &deviceVec,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("OnStart HandleUserSwitched");
    if (deviceVec.empty()) {
        LOGI("no remote device.");
        return DM_OK;
    }
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
    std::vector<AccessControlProfile> activeProfiles;
    std::vector<AccessControlProfile> inActiveProfiles;
    std::vector<AccessControlProfile> delActiveProfiles;
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
            if (item.GetBindType() != DM_IDENTICAL_ACCOUNT) {
                delActiveProfiles.push_back(item);
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
    HandleUserSwitched(activeProfiles, inActiveProfiles, delActiveProfiles);
    return DM_OK;
}

int32_t DeviceProfileConnector::HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid)
{
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
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
    std::vector<AccessControlProfile> profiles = GetAllAccessControlProfile();
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

IDeviceProfileConnector *CreateDpConnectorInstance()
{
    return &DeviceProfileConnector::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
