/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dm_log.h"
#include "dm_softbus_adapter_crypto.h"
#include "multiple_user_connector.h"

using namespace OHOS::DistributedDeviceProfile;

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector);
DeviceProfileConnector::DeviceProfileConnector()
{
    LOGD("DeviceProfileConnector constructor.");
}

DeviceProfileConnector::~DeviceProfileConnector()
{
    LOGD("DeviceProfileConnector destructor.");
}

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
    LOGI("GetAppTrustDeviceList Start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
    std::map<std::string, DmAuthForm> deviceIdMap;
    for (auto &item : profiles) {
        std::string trustDeviceId = item.GetTrustDeviceId();
        if (trustDeviceId == deviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        InsertDeviceIdMap(item, pkgName, deviceId, deviceIdMap, trustDeviceId);
    }
    LOGI("GetAppTrustDeviceList size is %d.", deviceIdMap.size());
    return deviceIdMap;
}

void DeviceProfileConnector::InsertDeviceIdMap(AccessControlProfile profiles, const std::string &pkgName,
    const std::string &deviceId, std::map<std::string, DmAuthForm> &deviceIdMap, std::string trustDeviceId)
{
    if (profiles.GetBindType() == IDENTICAL_ACCOUNT) {
        deviceIdMap[trustDeviceId] = DmAuthForm::IDENTICAL_ACCOUNT;
    }
    if (profiles.GetBindLevel() == DEVICE) {
        if (profiles.GetBindType() == DM_POINT_TO_POINT) {
            deviceIdMap[trustDeviceId] = DmAuthForm::PEER_TO_PEER;
        }
        if (profiles.GetBindType() == DM_ACROSS_ACCOUNT) {
            deviceIdMap[trustDeviceId] = DmAuthForm::ACROSS_ACCOUNT;
        }
    }
    if (profiles.GetBindLevel() == APP) {
        if (profiles.GetAccesser().GetAccesserBundleName() == pkgName &&
            profiles.GetAccesser().GetAccesserDeviceId() == deviceId) {
            if (profiles.GetBindType() == DM_POINT_TO_POINT) {
                deviceIdMap[trustDeviceId] = DmAuthForm::PEER_TO_PEER;
            }
            if (profiles.GetBindType() == DM_ACROSS_ACCOUNT) {
                deviceIdMap[trustDeviceId] = DmAuthForm::ACROSS_ACCOUNT;
            }
        }
        if (profiles.GetAccessee().GetAccesseeBundleName() == pkgName &&
            profiles.GetAccessee().GetAccesseeDeviceId() == deviceId) {
            if (profiles.GetBindType() == DM_POINT_TO_POINT) {
                deviceIdMap[trustDeviceId] = DmAuthForm::PEER_TO_PEER;
            }
            if (profiles.GetBindType() == DM_ACROSS_ACCOUNT) {
                deviceIdMap[trustDeviceId] = DmAuthForm::ACROSS_ACCOUNT;
            }
        }
    }
}

int32_t DeviceProfileConnector::GetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool &isonline, int32_t &authForm)
{
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    for (auto &item : profiles) {
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (DmSoftbusAdapterCrypto::GetUdidHash(item.GetTrustDeviceId(), reinterpret_cast<uint8_t *>(deviceIdHash)) !=
            DM_OK) {
            LOGE("get deviceIdHash by deviceId: %s failed.", GetAnonyString(deviceIdHash).c_str());
            return ERR_DM_FAILED;
        }
        if (static_cast<std::string>(deviceIdHash) != discoveryInfo.remoteDeviceIdHash || item.GetStatus() != ACTIVE) {
            continue;
        }
        HandleDmAuthForm(item, discoveryInfo, isonline, authForm);
    }
    return DM_OK;
}

void DeviceProfileConnector::HandleDmAuthForm(AccessControlProfile profiles, DmDiscoveryInfo discoveryInfo,
    bool &isonline, int32_t &authForm)
{
    if (profiles.GetBindType() == DM_IDENTICAL_ACCOUNT) {
        isonline = true;
        authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    }
    if (profiles.GetBindType() == DM_POINT_TO_POINT) {
        if (profiles.GetBindLevel() == DEVICE) {
            LOGI("The found device is peer-to-peer device bind-level.");
            isonline = true;
            authForm = DmAuthForm::PEER_TO_PEER;
        }
        if (profiles.GetBindLevel() == APP) {
            if (discoveryInfo.pkgname == profiles.GetAccesser().GetAccesserBundleName() &&
                discoveryInfo.localDeviceId == profiles.GetAccesser().GetAccesserDeviceId()) {
                LOGI("The found device is peer-to-peer app bind-level.");
                isonline = true;
                authForm = DmAuthForm::PEER_TO_PEER;
            }
            if (discoveryInfo.pkgname == profiles.GetAccessee().GetAccesseeBundleName() &&
                discoveryInfo.localDeviceId == profiles.GetAccessee().GetAccesseeDeviceId()) {
                LOGI("The found device is peer-to-peer app bind-level.");
                isonline = true;
                authForm = DmAuthForm::PEER_TO_PEER;
            }
        }
    }
    if (profiles.GetBindType() == DM_ACROSS_ACCOUNT) {
        if (profiles.GetBindLevel() == DEVICE) {
            LOGI("The found device is acrooc-account device bind-level.");
            isonline = true;
            authForm = DmAuthForm::ACROSS_ACCOUNT;
        }
        if (profiles.GetBindLevel() == APP) {
            if (discoveryInfo.pkgname == profiles.GetAccesser().GetAccesserBundleName() &&
                discoveryInfo.localDeviceId == profiles.GetAccesser().GetAccesserDeviceId()) {
                LOGI("The found device is acrooc-account app bind-level.");
                isonline = true;
                authForm = DmAuthForm::ACROSS_ACCOUNT;
            }
            if (discoveryInfo.pkgname == profiles.GetAccessee().GetAccesseeBundleName() &&
                discoveryInfo.localDeviceId == profiles.GetAccessee().GetAccesseeDeviceId()) {
                LOGI("The found device is acrooc-account app bind-level.");
                isonline = true;
                authForm = DmAuthForm::ACROSS_ACCOUNT;
            }
        }
    }
}

uint32_t DeviceProfileConnector::CheckBindType(std::string trustDeviceId, std::string requestDeviceId)
{
    LOGI("CheckBindType start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("GetOfflineParamFromAcl start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            offlineParam.leftAclNumber--;
        } else if ((item.GetAccesser().GetAccesserDeviceId() == requestDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == trustDeviceId) ||
            (item.GetAccesser().GetAccesserDeviceId() == trustDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == requestDeviceId)) {
            priority = APP_PEER_TO_PEER_TYPE;
            offlineParam.pkgNameVec.push_back(item.GetAccesser().GetAccesserBundleName());
            if (item.GetAuthenticationType() == ALLOW_AUTH_ONCE) {
                DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
                offlineParam.leftAclNumber--;
            }
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
    queryParams["accountId"] = accountId;
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
    }
    LOGI("AccessControlProfile size is %d.", profiles.size());
    for (auto &item : profiles) {
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT && (item.GetAccesser().GetAccesserUserId() &&
            item.GetAccesser().GetAccesserAccountId() == accountId)) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
    return DM_OK;
}

DmOfflineParam DeviceProfileConnector::DeleteAccessControlList(std::string pkgName, std::string localDeviceId,
    std::string remoteDeviceId)
{
    LOGI("DeleteAccessControlList by pkgName, localDeviceId, remoteDeviceId.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("AccessControlProfile size is %d.", profiles.size());
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

int32_t DeviceProfileConnector::DeleteTimeOutAcl(const std::string &deviceId)
{
    LOGI("DeviceProfileConnector::DeleteTimeOutAcl");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() == deviceId && item.GetStatus() == ACTIVE &&
            item.GetAuthenticationType() == ALLOW_AUTH_ONCE) {
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
        }
    }
    return DM_OK;
}

int32_t DeviceProfileConnector::GetTrustNumber(const std::string &deviceId)
{
    LOGI("DeviceProfileConnector::DeleteTimeOutAcl");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
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
    LOGI("AccessControlProfile size is %d.", profiles.size());
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != remoteDeviceId && item.GetStatus() != ACTIVE) {
            continue;
        }
        if ((item.GetBindType() == DM_POINT_TO_POINT || item.GetBindType() == DM_ACROSS_ACCOUNT) &&
            item.GetBindLevel() == DEVICE && (item.GetAccesser().GetAccesserBundleName() == pkgName ||
            item.GetAccessee().GetAccesseeBundleName() == pkgName)) {
            LOGI("The pkgname %s is peer-to-peer device unbind.", pkgName.c_str());
            return true;
        } else if ((item.GetBindType() == DM_POINT_TO_POINT || item.GetBindType() == DM_ACROSS_ACCOUNT) &&
            item.GetBindLevel() == APP && item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == localDeviceId) {
            LOGI("The pkgname %s is peer-to-peer app unbind.", pkgName.c_str());
            return true;
        } else if ((item.GetBindType() == DM_POINT_TO_POINT || item.GetBindType() == DM_ACROSS_ACCOUNT) &&
            item.GetBindLevel() == APP && item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == localDeviceId) {
            LOGI("The pkgname %s is peer-to-peer app unbind.", pkgName.c_str());
            return true;
        }
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
