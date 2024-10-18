/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "multiple_user_connector.h"
#include "distributed_device_profile_client.h"

using namespace OHOS::DistributedDeviceProfile;

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector);
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

std::unordered_map<std::string, DmAuthForm> DeviceProfileConnector::GetAppTrustDeviceList(const std::string &pkgName,
    const std::string &deviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    std::unordered_map<std::string, DmAuthForm> deviceIdMap;
    for (auto &item : profiles) {
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
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    if (profiles.size() == 0) {
        return DM_OK;
    }
    std::vector<int32_t> bindTypes;
    for (auto &item : profiles) {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetUdidHash(item.GetTrustDeviceId(), reinterpret_cast<uint8_t *>(deviceIdHash)) != DM_OK) {
            LOGE("get deviceIdHash by deviceId: %{public}s failed.", GetAnonyString(deviceIdHash).c_str());
            return ERR_DM_FAILED;
        }
        if (static_cast<std::string>(deviceIdHash) != discoveryInfo.remoteDeviceIdHash || item.GetStatus() != ACTIVE) {
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

uint32_t DeviceProfileConnector::CheckBindType(std::string trustDeviceId, std::string requestDeviceId)
{
    LOGI("Start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    uint32_t highestPriority = INVALIED_TYPE;
    for (auto &item : profiles) {
        if (trustDeviceId != item.GetTrustDeviceId() || item.GetStatus() != ACTIVE) {
            continue;
        }
        uint32_t priority = static_cast<uint32_t>(GetAuthForm(item, trustDeviceId, requestDeviceId));
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
            } else if (profiles.GetBindLevel() == APP && profiles.GetAccesser().GetAccesserDeviceId() == reqDev &&
                profiles.GetAccessee().GetAccesseeDeviceId() == trustDev) {
                priority = APP_PEER_TO_PEER_TYPE;
            } else if (profiles.GetBindLevel() == APP && profiles.GetAccessee().GetAccesseeDeviceId() == reqDev &&
                profiles.GetAccesser().GetAccesserDeviceId() == trustDev) {
                priority = APP_PEER_TO_PEER_TYPE;
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profiles.GetBindLevel() == DEVICE) {
                priority = DEVICE_ACROSS_ACCOUNT_TYPE;
            } else if (profiles.GetBindLevel() == APP && profiles.GetAccesser().GetAccesserDeviceId() == reqDev &&
                profiles.GetAccessee().GetAccesseeDeviceId() == trustDev) {
                priority = APP_ACROSS_ACCOUNT_TYPE;
            } else if (profiles.GetBindLevel() == APP && profiles.GetAccessee().GetAccesseeDeviceId() == reqDev &&
                profiles.GetAccesser().GetAccesserDeviceId() == trustDev) {
                priority = APP_ACROSS_ACCOUNT_TYPE;
            }
            break;
        default:
            LOGE("unknown bind type %{public}d.", bindType);
            break;
    }
    return priority;
}

std::vector<int32_t> DeviceProfileConnector::GetBindTypeByPkgName(std::string pkgName, std::string requestDeviceId,
    std::string trustUdid)
{
    LOGI("Start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    std::vector<int32_t> bindTypeVec;
    for (auto &item : profiles) {
        if (trustUdid != item.GetTrustDeviceId() || item.GetStatus() != ACTIVE) {
            continue;
        }
        GetParamBindTypeVec(item, pkgName, requestDeviceId, bindTypeVec);
    }
    return bindTypeVec;
}

void DeviceProfileConnector::GetParamBindTypeVec(AccessControlProfile profiles, std::string pkgName,
    std::string requestDeviceId, std::vector<int32_t> &bindTypeVec)
{
    if (profiles.GetBindType() == DM_IDENTICAL_ACCOUNT) {
        bindTypeVec.push_back(IDENTICAL_ACCOUNT_TYPE);
    }
    if (profiles.GetBindType() == DM_POINT_TO_POINT) {
        if (profiles.GetBindLevel() == DEVICE) {
            bindTypeVec.push_back(DEVICE_PEER_TO_PEER_TYPE);
        }
        if (profiles.GetBindLevel() == APP) {
            if (profiles.GetAccesser().GetAccesserBundleName() == pkgName &&
                profiles.GetAccesser().GetAccesserDeviceId() == requestDeviceId) {
                bindTypeVec.push_back(APP_PEER_TO_PEER_TYPE);
            }
            if ((profiles.GetAccessee().GetAccesseeBundleName() == pkgName &&
                profiles.GetAccessee().GetAccesseeDeviceId() == requestDeviceId)) {
                bindTypeVec.push_back(APP_PEER_TO_PEER_TYPE);
            }
        }
    }
    if (profiles.GetBindType() == DM_ACROSS_ACCOUNT) {
        if (profiles.GetBindLevel() == DEVICE) {
            bindTypeVec.push_back(DEVICE_ACROSS_ACCOUNT_TYPE);
        }
        if (profiles.GetBindLevel() == APP) {
            if (profiles.GetAccesser().GetAccesserBundleName() == pkgName &&
                profiles.GetAccesser().GetAccesserDeviceId() == requestDeviceId) {
                bindTypeVec.push_back(APP_ACROSS_ACCOUNT_TYPE);
            }
            if ((profiles.GetAccessee().GetAccesseeBundleName() == pkgName &&
                profiles.GetAccessee().GetAccesseeDeviceId() == requestDeviceId)) {
                bindTypeVec.push_back(APP_ACROSS_ACCOUNT_TYPE);
            }
        }
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
        ProcessBindType(profiles[index], paramInfo, sinkBindType, bindTypeIndex, index);
    }
    return bindTypeIndex;
}

void DeviceProfileConnector::ProcessBindType(AccessControlProfile profiles, DmDiscoveryInfo paramInfo,
    std::vector<int32_t> &sinkBindType, std::vector<int32_t> &bindTypeIndex, uint32_t index)
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
            if (profiles.GetAccesser().GetAccesserBundleName() == paramInfo.pkgname &&
                profiles.GetAccesser().GetAccesserDeviceId() == paramInfo.localDeviceId) {
                sinkBindType.push_back(APP_PEER_TO_PEER_TYPE);
                bindTypeIndex.push_back(index);
            }
            if (profiles.GetAccessee().GetAccesseeBundleName() == paramInfo.pkgname &&
                profiles.GetAccessee().GetAccesseeDeviceId() == paramInfo.localDeviceId) {
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
            if (profiles.GetAccesser().GetAccesserBundleName() == paramInfo.pkgname &&
                profiles.GetAccesser().GetAccesserDeviceId() == paramInfo.localDeviceId) {
                sinkBindType.push_back(APP_ACROSS_ACCOUNT_TYPE);
                bindTypeIndex.push_back(index);
            }
            if (profiles.GetAccessee().GetAccesseeBundleName() == paramInfo.pkgname &&
                profiles.GetAccessee().GetAccesseeDeviceId() == paramInfo.localDeviceId) {
                sinkBindType.push_back(APP_ACROSS_ACCOUNT_TYPE);
                bindTypeIndex.push_back(index);
            }
        }
    }
}

std::vector<int32_t> DeviceProfileConnector::SyncAclByBindType(std::string pkgName, std::vector<int32_t> bindTypeVec,
    std::string localDeviceId, std::string targetDeviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindType;
    std::vector<int32_t> bindTypeIndex =
        CompareBindType(profiles, pkgName, sinkBindType, localDeviceId, targetDeviceId);
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
        }
    }
    return bindType;
}

std::vector<std::string> DeviceProfileConnector::GetPkgNameFromAcl(std::string &localDeviceId,
    std::string &targetDeviceId)
{
    LOGI("Start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    std::vector<std::string> pkgNameVec;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != targetDeviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == targetDeviceId) ||
            (item.GetAccesser().GetAccesserDeviceId() == targetDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == localDeviceId)) {
            pkgNameVec.push_back(item.GetAccesser().GetAccesserBundleName());
        }
    }
    return pkgNameVec;
}

DmOfflineParam DeviceProfileConnector::GetOfflineParamFromAcl(std::string trustDeviceId, std::string requestDeviceId)
{
    LOGI("TrustDeviceId = %{public}s and requestDeviceId = %{public}s",
         GetAnonyString(trustDeviceId).c_str(), GetAnonyString(requestDeviceId).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    DmOfflineParam offlineParam;
    offlineParam.leftAclNumber = 0;
    offlineParam.bindType = INVALIED_TYPE;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != trustDeviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        offlineParam.leftAclNumber++;
        uint32_t priority = INVALIED_TYPE;
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            priority = IDENTICAL_ACCOUNT_TYPE;
        } else if (item.GetBindLevel() == DEVICE && item.GetAuthenticationType() == ALLOW_AUTH_ALWAYS) {
            priority = DEVICE_PEER_TO_PEER_TYPE;
        } else if (item.GetBindLevel() == DEVICE && item.GetAuthenticationType() == ALLOW_AUTH_ONCE) {
            priority = DEVICE_PEER_TO_PEER_TYPE;
            offlineParam.pkgNameVec.push_back(item.GetAccesser().GetAccesserBundleName());
        } else if ((item.GetAccesser().GetAccesserDeviceId() == requestDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == trustDeviceId) ||
            (item.GetAccesser().GetAccesserDeviceId() == trustDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == requestDeviceId)) {
            priority = APP_PEER_TO_PEER_TYPE;
            offlineParam.pkgNameVec.push_back(item.GetAccesser().GetAccesserBundleName());
        }
        if (priority > offlineParam.bindType) {
            offlineParam.bindType = priority;
        }
    }
    return offlineParam;
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
    Accessee accessee;
    accessee.SetAccesseeDeviceId(dmAccessee.trustDeviceId);
    accessee.SetAccesseeUserId(dmAccessee.trustUserId);
    accessee.SetAccesseeAccountId(dmAccessee.trustAccountId);
    accessee.SetAccesseeTokenId(dmAccessee.trustTokenId);
    accessee.SetAccesseeBundleName(dmAccessee.trustBundleName);
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

void DeviceProfileConnector::DeleteAclForAccountLogOut(const std::string &localUdid, int32_t userId,
    const std::string &remoteUdid)
{
    LOGI("localUdid %{public}s, userId %{public}d, remoteUdid %{public}s.", GetAnonyString(localUdid).c_str(), userId,
        GetAnonyString(remoteUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(userId);
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() == remoteUdid) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
}

void DeviceProfileConnector::DeleteAclForUserRemoved(int32_t userId)
{
    LOGI("DeleteAclForUserRemoved userId %{public}d.", userId);
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(userId);
    for (const auto &item : profiles) {
        if (item.GetAccesser().GetAccesserUserId() == userId || item.GetAccessee().GetAccesseeUserId() == userId) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
}

void DeviceProfileConnector::DeleteAccessControlList(const std::string &udid)
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
    const std::string &localDeviceId, const std::string &remoteDeviceId, int32_t bindLevel)
{
    LOGI("pkgName %{public}s, localDeviceId %{public}s, remoteDeviceId %{public}s, bindLevel %{public}d.",
        pkgName.c_str(), GetAnonyString(localDeviceId).c_str(), GetAnonyString(remoteDeviceId).c_str(), bindLevel);
    DmOfflineParam offlineParam;
    offlineParam.bindType = INVALIED_TYPE;
    if (static_cast<uint32_t>(bindLevel) > APP || static_cast<uint32_t>(bindLevel) < DEVICE) {
        LOGE("Invalied bindlevel.");
        return offlineParam;
    }
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    if (profiles.empty()) {
        LOGE("Acl is empty.");
        return offlineParam;
    }
    switch (bindLevel) {
        case APP:
            DeleteAppBindLevel(offlineParam, pkgName, profiles, localDeviceId, remoteDeviceId);
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
    LOGI("Start.");
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

bool DeviceProfileConnector::CheckIdenticalAccount(int32_t userId, const std::string &accountId)
{
    LOGI("Start");
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

bool DeviceProfileConnector::CheckSrcDevIdInAclForDevBind(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("Start");
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
    LOGI("Start");
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

uint32_t DeviceProfileConnector::DeleteTimeOutAcl(const std::string &deviceId)
{
    LOGI("Start");
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

int32_t DeviceProfileConnector::GetTrustNumber(const std::string &deviceId)
{
    LOGI("Start");
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

int32_t DeviceProfileConnector::IsSameAccount(const std::string &udid)
{
    LOGI("Start.");
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

int32_t DeviceProfileConnector::CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("PkgName %{public}s, srcUdid %{public}s, sinkUdid %{public}s",
        caller.pkgName.c_str(), GetAnonyString(srcUdid).c_str(), GetAnonyString(sinkUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    for (auto &item : profiles) {
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
                profile.GetAccesser().GetAccesserBundleName() == caller.pkgName) {
                ret = true;
            }
            break;
        case DM_ACROSS_ACCOUNT:
            if (profile.GetBindLevel() == DEVICE || profile.GetBindLevel() == SERVICE) {
                ret = true;
            } else if (profile.GetBindLevel() == APP &&
                profile.GetAccesser().GetAccesserBundleName() == caller.pkgName) {
                ret = true;
            }
            break;
        default:
            LOGE("unknown bind type %{public}d.", bindType);
            break;
    }
    return ret;
}

int32_t DeviceProfileConnector::CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
    const DmAccessCallee &callee, const std::string &sinkUdid)
{
    LOGI("DeviceProfileConnector::CheckIsSameAccount pkgName %{public}s, srcUdid %{public}s, sinkUdid %{public}s",
        caller.pkgName.c_str(), GetAnonyString(srcUdid).c_str(), GetAnonyString(sinkUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    for (auto &item : profiles) {
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

int32_t DeviceProfileConnector::GetBindLevel(const std::string &pkgName, const std::string &localUdid,
    const std::string &udid, uint64_t &tokenId)
{
    LOGI("pkgName %{public}s, tokenId %{public}" PRId64", udid %{public}s.", pkgName.c_str(),
        tokenId, GetAnonyString(udid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    int32_t bindLevel = INVALIED_TYPE;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != udid) {
            continue;
        }
        if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == udid) {
            tokenId = static_cast<uint64_t>(item.GetAccesser().GetAccesserTokenId());
            bindLevel = item.GetBindLevel();
            LOGI("Src get bindLevel %{public}d, tokenid %{public}" PRId64".", bindLevel, tokenId);
            continue;
        }
        if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid &&
            item.GetAccesser().GetAccesserDeviceId() == udid) {
            tokenId = item.GetAccessee().GetAccesseeTokenId();
            bindLevel = static_cast<int32_t>(item.GetBindLevel());
            LOGI("Sink get bindLevel %{public}d, tokenid %{public}" PRId64".", bindLevel, tokenId);
            continue;
        }
    }
    return bindLevel;
}

std::map<std::string, int32_t> DeviceProfileConnector::GetDeviceIdAndBindType(int32_t userId,
    const std::string &accountId, const std::string &localUdid)
{
    LOGI("userId %{public}d, accountId %{public}s.", userId, GetAnonyString(accountId).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfileByUserId(userId);
    std::map<std::string, int32_t> deviceIdMap;
    for (const auto &item : profiles) {
        if (item.GetAccesser().GetAccesserUserId() == userId &&
            item.GetAccesser().GetAccesserAccountId() == accountId &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid) {
            LOGI("Account logout localUdid %{public}s is src.", GetAnonyString(localUdid).c_str());
            UpdateBindType(item.GetTrustDeviceId(), item.GetBindType(), deviceIdMap);
            continue;
        }
        if (item.GetAccessee().GetAccesseeUserId() == userId &&
            item.GetAccessee().GetAccesseeAccountId() == accountId &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            LOGI("Account logout localUdid %{public}s is sink.", GetAnonyString(localUdid).c_str());
            UpdateBindType(item.GetTrustDeviceId(), item.GetBindType(), deviceIdMap);
            continue;
        }
    }
    return deviceIdMap;
}

void DeviceProfileConnector::UpdateBindType(const std::string &udid, int32_t bindType,
    std::map<std::string, int32_t> &deviceMap)
{
    LOGI("BindType %{public}d.", bindType);
    if (deviceMap.find(udid) == deviceMap.end()) {
        deviceMap[udid] = bindType;
    } else {
        deviceMap[udid] = std::min(deviceMap[udid], bindType);
    }
}

int32_t DeviceProfileConnector::HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
    const std::string &remoteUdid, const std::string &localUdid)
{
    LOGI("RemoteUserId %{public}d, remoteAccountHash %{public}s, remoteUdid %{public}s, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteAccountHash).c_str(), GetAnonyString(remoteUdid).c_str(),
        GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    int32_t bindType = DM_INVALIED_BINDTYPE;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }
        DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        bindType = DM_IDENTICAL_ACCOUNT;
    }
    return bindType;
}

int32_t DeviceProfileConnector::HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    const std::string &localUdid)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, localUdid %{public}s.", remoteUserId,
        GetAnonyString(remoteUdid).c_str(), GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    int32_t bindType = DM_INVALIED_BINDTYPE;
    for (const auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteUdid) {
            continue;
        }
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            bindType = DM_IDENTICAL_ACCOUNT;
            continue;
        }
        if (item.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
            item.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            LOGI("Src device unbind.");
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            bindType = std::min(bindType, static_cast<int32_t>(item.GetBindType()));
            continue;
        }
        if (item.GetAccessee().GetAccesseeDeviceId() == remoteUdid &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid) {
            LOGI("Sink device unbind.");
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            bindType = std::min(bindType, static_cast<int32_t>(item.GetBindType()));
            continue;
        }
    }
    return bindType;
}

std::string DeviceProfileConnector::HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    int32_t tokenId, const std::string &localUdid)
{
    LOGI("RemoteUserId %{public}d, remoteUdid %{public}s, tokenId %{public}d, localUdid %{public}s.",
        remoteUserId, GetAnonyString(remoteUdid).c_str(), tokenId, GetAnonyString(localUdid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    std::string pkgName = "";
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
            pkgName = item.GetAccesser().GetAccesserBundleName();
            continue;
        }
        if (item.GetAccessee().GetAccesseeUserId() == remoteUserId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteUdid &&
            static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId()) == tokenId &&
            item.GetAccesser().GetAccesserDeviceId() == localUdid) {
            LOGI("Sink device unbind.");
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            pkgName = item.GetAccessee().GetAccesseeBundleName();
            continue;
        }
    }
    return pkgName;
}

std::vector<AccessControlProfile> DeviceProfileConnector::GetAllAccessControlProfile()
{
    std::vector<AccessControlProfile> profiles;
    if (DistributedDeviceProfileClient::GetInstance().GetAllAccessControlProfile(profiles) != DM_OK) {
        LOGE("DP failed.");
    }
    return profiles;
}

void DeviceProfileConnector::DeleteAccessControlById(int64_t accessControlId)
{
    DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(accessControlId);
}

IDeviceProfileConnector *CreateDpConnectorInstance()
{
    return &DeviceProfileConnector::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
