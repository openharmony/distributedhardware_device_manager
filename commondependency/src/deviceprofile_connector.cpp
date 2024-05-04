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
IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector);
std::vector<AccessControlProfile> DeviceProfileConnector::GetAccessControlProfile()
{
    LOGI("GetAccessControlProfile start.");
    std::vector<AccessControlProfile> profiles;
    std::map<std::string, std::string> queryParams;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    queryParams["userId"] = std::to_string(userId);
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
    }
    return profiles;
}

std::map<std::string, DmAuthForm> DeviceProfileConnector::GetAppTrustDeviceList(const std::string &pkgName,
    const std::string &deviceId)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("DeviceProfileConnector::GetAppTrustDeviceList, AccessControlProfile size is %{public}zu", profiles.size());
    std::map<std::string, DmAuthForm> deviceIdMap;
    for (auto &item : profiles) {
        std::string trustDeviceId = item.GetTrustDeviceId();
        if (trustDeviceId == deviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        DmDiscoveryInfo discoveryInfo = {pkgName, deviceId};
        int32_t bindType = HandleDmAuthForm(item, discoveryInfo);
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
    LOGI("GetAppTrustDeviceList size is %{public}zu", deviceIdMap.size());
    return deviceIdMap;
}

int32_t DeviceProfileConnector::GetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool &isOnline, int32_t &authForm)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("DeviceProfileConnector::GetDeviceAclParam, AccessControlProfile size is %{public}zu", profiles.size());
    if (profiles.size() == 0) {
        return DM_OK;
    }
    std::vector<int32_t> bindTypes;
    for (auto &item : profiles) {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetUdidHash(item.GetTrustDeviceId(), reinterpret_cast<uint8_t *>(deviceIdHash)) !=
            DM_OK) {
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
        LOGI("GetDeviceAclParam, The found device is identical account device bind type.");
        return DM_OK;
    }
    if (std::count(bindTypes.begin(), bindTypes.end(), DmAuthForm::PEER_TO_PEER) > 0) {
        isOnline = true;
        authForm = DmAuthForm::PEER_TO_PEER;
        LOGI("GetDeviceAclParam, The found device is peer-to-peer device bind-level.");
        return DM_OK;
    }
    if (std::count(bindTypes.begin(), bindTypes.end(), DmAuthForm::ACROSS_ACCOUNT) > 0) {
        isOnline = true;
        authForm = DmAuthForm::ACROSS_ACCOUNT;
        LOGI("GetDeviceAclParam, The found device is across-account device bind-level.");
        return DM_OK;
    }
    authForm = DmAuthForm::INVALID_TYPE;
    return DM_OK;
}

int32_t DeviceProfileConnector::CheckAuthForm(DmAuthForm form, AccessControlProfile profiles,
    DmDiscoveryInfo discoveryInfo)
{
    LOGI("DeviceProfileConnector::CheckAuthForm %{public}d", form);
    if (profiles.GetBindLevel() == DEVICE || (profiles.GetBindLevel() == APP && discoveryInfo.pkgname == "")) {
        LOGI("The found device is device bind-level.");
        return form;
    }
    if (profiles.GetBindLevel() == APP) {
        if (discoveryInfo.pkgname == profiles.GetAccesser().GetAccesserBundleName() &&
            discoveryInfo.localDeviceId == profiles.GetAccesser().GetAccesserDeviceId()) {
            LOGI("The found device is app bind-level.");
            return form;
        }
        if (discoveryInfo.pkgname == profiles.GetAccessee().GetAccesseeBundleName() &&
            discoveryInfo.localDeviceId == profiles.GetAccessee().GetAccesseeDeviceId()) {
            LOGI("The found device is app bind-level.");
            return form;
        }
    }
    return DmAuthForm::INVALID_TYPE;
}

int32_t DeviceProfileConnector::HandleDmAuthForm(AccessControlProfile profiles, DmDiscoveryInfo discoveryInfo)
{
    if (profiles.GetBindType() == DM_IDENTICAL_ACCOUNT) {
        LOGI("The found device is identical account device bind type.");
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
    LOGI("CheckBindType start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    uint32_t highestPriority = INVALIED_TYPE;
    for (auto &item : profiles) {
        if (trustDeviceId != item.GetTrustDeviceId() || item.GetStatus() != ACTIVE) {
            continue;
        }
        uint32_t priority = INVALIED_TYPE;
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            priority = IDENTICAL_ACCOUNT_TYPE;
        } else if (item.GetBindLevel() == DEVICE) {
            priority = DEVICE_PEER_TO_PEER_TYPE;
        } else if (item.GetBindLevel() == APP && (item.GetAccesser().GetAccesserDeviceId() == requestDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == trustDeviceId)) {
            priority = APP_PEER_TO_PEER_TYPE;
        } else if (item.GetBindLevel() == APP && (item.GetAccessee().GetAccesseeDeviceId() == requestDeviceId &&
            item.GetAccesser().GetAccesserDeviceId() == trustDeviceId)) {
            priority = APP_PEER_TO_PEER_TYPE;
        }
        if (priority > highestPriority) {
            highestPriority = priority;
        }
    }
    return highestPriority;
}

std::vector<int32_t> DeviceProfileConnector::GetBindTypeByPkgName(std::string pkgName, std::string requestDeviceId,
    std::string trustUdid)
{
    LOGI("GetBindTypeByPkgName start.");
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
    LOGI("SyncAclByBindType start.");
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
    LOGI("GetPkgNameFromAcl start.");
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
    LOGI("DeviceProfileConnector::GetOfflineParamFromAcl, trustDeviceId = %{public}s and requestDeviceId = %{public}s",
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
    LOGI("DeviceProfileConnector::PutAccessControlList start.");
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
    if (DistributedDeviceProfileClient::GetInstance().PutAccessControlProfile(profile) != DM_OK) {
        LOGE("PutAccessControlProfile failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::DeleteAccessControlList(int32_t userId, std::string &accountId)
{
    LOGI("DeleteAccessControlList by userId and accountId.");
    std::vector<AccessControlProfile> profiles;
    std::map<std::string, std::string> queryParams;
    queryParams["userId"] = std::to_string(userId);
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
    }
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        LOGI("AccessControlProfile bindType is : %{public}d.", item.GetBindType());
        DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
    }
    return DM_OK;
}

DmOfflineParam DeviceProfileConnector::DeleteAccessControlList(std::string pkgName, std::string localDeviceId,
    std::string remoteDeviceId)
{
    LOGI("DeleteAccessControlList by pkgName, localDeviceId, remoteDeviceId.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    DmOfflineParam offlineParam;
    offlineParam.bindType = INVALIED_TYPE;
    offlineParam.leftAclNumber = 0;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteDeviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            LOGE("Identical account forbid unbind.");
            offlineParam.bindType = INVALIED_TYPE;
            return offlineParam;
        }
        if (item.GetTrustDeviceId() == remoteDeviceId) {
            offlineParam.leftAclNumber++;
            if (item.GetBindLevel() == DEVICE && item.GetBindType() != DM_IDENTICAL_ACCOUNT &&
                item.GetAccesser().GetAccesserBundleName() == pkgName) {
                LOGI("DeleteAccessControlList device unbind.");
                offlineParam.bindType = DEVICE_PEER_TO_PEER_TYPE;
            }
        }
    }
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteDeviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserDeviceId() == localDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == remoteDeviceId) ||
            (item.GetAccessee().GetAccesseeDeviceId() == localDeviceId &&
            item.GetAccesser().GetAccesserDeviceId() == remoteDeviceId)) {
            if (offlineParam.bindType == DEVICE_PEER_TO_PEER_TYPE) {
                DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
                offlineParam.leftAclNumber--;
            } else if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
                item.GetAccessee().GetAccesseeBundleName() == pkgName) {
                offlineParam.bindType = APP_PEER_TO_PEER_TYPE;
                DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
                offlineParam.leftAclNumber--;
                break;
            }
        }
    }
    return offlineParam;
}

int32_t DeviceProfileConnector::UpdateAccessControlList(int32_t userId, std::string &oldAccountId,
    std::string &newAccountId)
{
    LOGI("UpdateAccessControlList by userId and accountId.");
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
    LOGI("DeviceProfileConnector::CheckIdenticalAccount");
    std::vector<AccessControlProfile> profiles;
    std::map<std::string, std::string> queryParams;
    queryParams["userId"] = std::to_string(userId);
    queryParams["accountId"] = accountId;
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
int32_t DeviceProfileConnector::DeleteP2PAccessControlList(int32_t userId, std::string &accountId)
{
    LOGI("DeviceProfileConnector::DeleteP2PAccessControlList");
    std::vector<AccessControlProfile> profiles;
    std::map<std::string, std::string> queryParams;
    queryParams["userId"] = std::to_string(userId);
    queryParams["accountId"] = accountId;
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
    }
    for (auto &item : profiles) {
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT || item.GetStatus() != ACTIVE) {
            continue;
        }
        if ((item.GetAccesser().GetAccesserUserId() == userId &&
            item.GetAccesser().GetAccesserAccountId() == accountId) ||
            (item.GetAccessee().GetAccesseeUserId() == userId &&
            item.GetAccessee().GetAccesseeAccountId() == accountId)) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
    return DM_OK;
}

bool DeviceProfileConnector::CheckSrcDeviceIdInAcl(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DeviceProfileConnector::CheckSrcDeviceIdInAcl");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == deviceId && item.GetStatus() == ACTIVE &&
            item.GetBindLevel() == DEVICE && item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeUserId() == 0 && item.GetAccessee().GetAccesseeAccountId() == "") {
            return true;
        }
    }
    return false;
}

bool DeviceProfileConnector::CheckSinkDeviceIdInAcl(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DeviceProfileConnector::CheckSinkDeviceIdInAcl");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == deviceId && item.GetStatus() == ACTIVE &&
            item.GetBindLevel() == DEVICE && item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserUserId() == 0 && item.GetAccesser().GetAccesserAccountId() == "") {
            return true;
        }
    }
    return false;
}

bool DeviceProfileConnector::CheckDeviceIdInAcl(const std::string &pkgName, const std::string &deviceId)
{
    return (CheckSinkDeviceIdInAcl(pkgName, deviceId) || CheckSrcDeviceIdInAcl(pkgName, deviceId));
}

uint32_t DeviceProfileConnector::DeleteTimeOutAcl(const std::string &deviceId)
{
    LOGI("DeviceProfileConnector::DeleteTimeOutAcl");
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
    LOGI("DeviceProfileConnector::DeleteTimeOutAcl");
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

bool DeviceProfileConnector::CheckPkgnameInAcl(std::string pkgName, std::string localDeviceId,
    std::string remoteDeviceId)
{
    LOGI("DeviceProfileConnector::CheckPkgnameInAcl");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteDeviceId && item.GetStatus() != ACTIVE) {
            continue;
        }
        if ((item.GetBindType() == DM_POINT_TO_POINT || item.GetBindType() == DM_ACROSS_ACCOUNT) &&
            item.GetBindLevel() == DEVICE && (item.GetAccesser().GetAccesserBundleName() == pkgName ||
            item.GetAccessee().GetAccesseeBundleName() == pkgName)) {
            LOGI("The pkgname %{public}s is peer-to-peer device unbind.", pkgName.c_str());
            return true;
        } else if ((item.GetBindType() == DM_POINT_TO_POINT || item.GetBindType() == DM_ACROSS_ACCOUNT) &&
            item.GetBindLevel() == APP && item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == localDeviceId) {
            LOGI("The pkgname %{public}s is peer-to-peer app unbind.", pkgName.c_str());
            return true;
        } else if ((item.GetBindType() == DM_POINT_TO_POINT || item.GetBindType() == DM_ACROSS_ACCOUNT) &&
            item.GetBindLevel() == APP && item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == localDeviceId) {
            LOGI("The pkgname %{public}s is peer-to-peer app unbind.", pkgName.c_str());
            return true;
        }
    }
    return false;
}

int32_t DeviceProfileConnector::IsSameAccount(const std::string &udid)
{
    LOGI("DeviceProfileConnector::IsSameAccount start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("IsSameAccount profiles size is %{public}zu", profiles.size());
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

int32_t DeviceProfileConnector::CheckRelatedDevice(const std::string &udid, const std::string &bundleName)
{
    LOGI("DeviceProfileConnector::CheckRelatedDevice start, bundleName %{public}s, udid %{public}s", bundleName.c_str(),
        GetAnonyString(udid).c_str());
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("CheckRelatedDevice profiles size is %{public}zu", profiles.size());
    for (auto &item : profiles) {
        if (IsTrustDevice(item, udid, bundleName)) {
            LOGI("The udid %{public}s is trust.", GetAnonyString(udid).c_str());
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

bool DeviceProfileConnector::IsTrustDevice(AccessControlProfile profile, const std::string &udid,
    const std::string &bundleName)
{
    LOGI("DeviceProfileConnector::IsTrustDevice start, bundleName %{public}s, udid %{public}s", bundleName.c_str(),
        GetAnonyString(udid).c_str());
    std::string trustDeviceId = profile.GetTrustDeviceId();
    if (trustDeviceId == udid && profile.GetStatus() == ACTIVE) {
        if (profile.GetBindType() == DM_IDENTICAL_ACCOUNT) {  // 同账号
            LOGI("The udid %{public}s is identical bind.", GetAnonyString(udid).c_str());
            return true;
        }
        if (profile.GetBindType() == DM_POINT_TO_POINT || profile.GetBindType() == DM_ACROSS_ACCOUNT) {
            if (profile.GetBindLevel() == DEVICE) {  // 设备级
                LOGI("The udid %{public}s is device bind.", GetAnonyString(udid).c_str());
                return true;
            }
            if (profile.GetBindLevel() == APP && (profile.GetAccesser().GetAccesserBundleName() == bundleName ||
                profile.GetAccessee().GetAccesseeBundleName() == bundleName)) {  // 应用级
                LOGI("The udid %{public}s is app bind.", GetAnonyString(udid).c_str());
                return true;
            }
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
