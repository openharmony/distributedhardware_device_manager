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
#include "dm_device_info.h"
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
    std::string accountId = MultipleUserConnector::GetOhosAccountId();
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    queryParams["userId"] = std::to_string(userId);
    queryParams["accountId"] = accountId;
    if (DistributedDeviceProfileClient::GetInstance().GetAccessControlProfile(queryParams, profiles) != DM_OK) {
        LOGE("DP GetAccessControlProfile failed.");
    }
    return profiles;
}

std::vector<std::string> DeviceProfileConnector::GetAppTrustDeviceList(const std::string &pkgName,
    const std::string &deviceId)
{
    LOGI("GetAppTrustDeviceList Start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
    std::vector<std::string> deviceIdVec;
    for (auto &item : profiles) {
        std::string trustDeviceId = item.GetTrustDeviceId();
        if (trustDeviceId == deviceId || item.GetStatus() != ACTIVE) {
            continue;
        }
        if (item.GetBindType() == IDENTICAL_ACCOUNT) {
            LOGI("The device %s is identical account.", GetAnonyString(std::string(trustDeviceId)).c_str());
            deviceIdVec.push_back(trustDeviceId);
        } else if (item.GetBindLevel() == DEVICE) {
            LOGI("The device %s is device-level bind.", GetAnonyString(std::string(trustDeviceId)).c_str());
            deviceIdVec.push_back(trustDeviceId);
        } else if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == deviceId) {
            LOGI("The device %s is pkg-level bind accesser.", GetAnonyString(std::string(trustDeviceId)).c_str());
            deviceIdVec.push_back(trustDeviceId);
        } else if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == deviceId) {
            LOGI("The device %s is pkg-level bind accessee.", GetAnonyString(std::string(trustDeviceId)).c_str());
            deviceIdVec.push_back(trustDeviceId);
        }
    }
    LOGI("GetAppTrustDeviceList size is %d.", deviceIdVec.size());
    return deviceIdVec;
}

int32_t DeviceProfileConnector::GetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool &isonline, int32_t &authForm)
{
    LOGI("GetDeviceAclParam start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
    for (auto &item : profiles) {
        if (item.GetStatus() != ACTIVE) {
            continue;
        }
        char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (DmSoftbusAdapterCrypto::GetUdidHash(item.GetTrustDeviceId(), (uint8_t *)deviceIdHash) != DM_OK) {
            LOGE("get deviceIdHash by deviceId: %s failed.", GetAnonyString(deviceIdHash).c_str());
            return ERR_DM_FAILED;
        }
        std::string trustDeviceIdHash = static_cast<std::string>(deviceIdHash);
        if (trustDeviceIdHash != discoveryInfo.remoteDeviceIdHash || item.GetStatus() != ACTIVE) {
            continue;
        }
        if (item.GetBindType() == IDENTICAL_ACCOUNT) {
            LOGI("The found device is identical account.");
            isonline = true;
            authForm = DmAuthForm::IDENTICAL_ACCOUNT;
        } else if (item.GetBindLevel() == DEVICE) {
            LOGI("The found device is device bind-level.");
            isonline = true;
            authForm = DmAuthForm::PEER_TO_PEER;
        } else if ((discoveryInfo.pkgname == item.GetAccesser().GetAccesserBundleName() &&
            discoveryInfo.localDeviceId == item.GetAccesser().GetAccesserDeviceId()) ||
            (discoveryInfo.pkgname == item.GetAccessee().GetAccesseeBundleName() &&
            discoveryInfo.localDeviceId == item.GetAccessee().GetAccesseeDeviceId())) {
            LOGI("The found device is pkg bind-level.");
            isonline = true;
            authForm = DmAuthForm::PEER_TO_PEER;
        }
    }
    return DM_OK;
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
            priority = DEVICE_LEVEL_BIND_TYPE;
        } else if (item.GetBindLevel() == APP && (item.GetAccesser().GetAccesserDeviceId() == requestDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == trustDeviceId)) {
            priority = APP_LEVEL_BIND_TYPE;
        } else if (item.GetBindLevel() == APP && (item.GetAccessee().GetAccesseeDeviceId() == requestDeviceId &&
            item.GetAccesser().GetAccesserDeviceId() == trustDeviceId)) {
            priority = APP_LEVEL_BIND_TYPE;
        }
        if (priority > highestPriority) {
            highestPriority = priority;
        }
    }
    return highestPriority;
}

std::vector<int32_t> DeviceProfileConnector::GetBindTypeByPkgName(std::string pkgName, std::string requestDeviceId,
    std::string trustUdidHash)
{
    LOGI("GetBindTypeByPkgName start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
    std::vector<int32_t> bindTypeVec;
    for (auto &item : profiles) {
        char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (DmSoftbusAdapterCrypto::GetUdidHash(item.GetTrustDeviceId(), (uint8_t *)udidHash) != DM_OK) {
            LOGE("get udidhash by udid: %s failed.", GetAnonyString(udidHash).c_str());
        }
        std::string trustDeviceIdHash = static_cast<std::string>(udidHash);
        if (trustDeviceIdHash != trustUdidHash || item.GetStatus() == INACTIVE) {
            continue;
        }
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            bindTypeVec.push_back(IDENTICAL_ACCOUNT_TYPE);
        } else if (item.GetBindLevel() == DEVICE) {
            bindTypeVec.push_back(DEVICE_LEVEL_BIND_TYPE);
        } else if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
            item.GetAccesser().GetAccesserDeviceId() == requestDeviceId) {
            bindTypeVec.push_back(APP_LEVEL_BIND_TYPE);
        } else if (item.GetAccessee().GetAccesseeBundleName() == pkgName &&
            item.GetAccessee().GetAccesseeDeviceId() == requestDeviceId) {
            bindTypeVec.push_back(APP_LEVEL_BIND_TYPE);
        }
    }
    return bindTypeVec;
}

std::vector<int32_t> DeviceProfileConnector::SyncAclByBindType(std::string pkgName, std::vector<int32_t> bindTypeVec,
    std::string localDeviceId, std::string targetDeviceId)
{
    LOGI("SyncAclByBindType start.");
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    std::vector<int32_t> bindType;
    for (uint32_t index = 0; index < profiles.size(); index++) {
        if (profiles[index].GetTrustDeviceId() != targetDeviceId || profiles[index].GetStatus() != ACTIVE) {
            continue;
        }
        if (profiles[index].GetBindType() == DM_IDENTICAL_ACCOUNT) {
            sinkBindType.push_back(IDENTICAL_ACCOUNT_TYPE);
            bindTypeIndex.push_back(index);
        } else if (profiles[index].GetBindLevel() == DEVICE) {
            sinkBindType.push_back(DEVICE_LEVEL_BIND_TYPE);
            bindTypeIndex.push_back(index);
        } else if ((profiles[index].GetAccesser().GetAccesserBundleName() == pkgName &&
            profiles[index].GetAccesser().GetAccesserDeviceId() == targetDeviceId) ||
            (profiles[index].GetAccessee().GetAccesseeBundleName() == pkgName &&
            profiles[index].GetAccessee().GetAccesseeDeviceId() == localDeviceId)) {
            sinkBindType.push_back(APP_LEVEL_BIND_TYPE);
            bindTypeIndex.push_back(index);
        }
    }
    for (uint32_t sinkIndex = 0; sinkIndex < sinkBindType.size(); sinkIndex++) {
        bool deleteAclFlag = true;
        for (uint32_t srcIndex = 0; srcIndex < bindTypeVec.size(); srcIndex++) {
            if (sinkBindType[sinkIndex] == bindTypeVec[srcIndex]) {
                deleteAclFlag = false;
                bindType.push_back(bindTypeVec[sinkIndex]);
            }
        }
        if (deleteAclFlag) {
            uint32_t deleteIndex = profiles[bindTypeIndex[sinkIndex]].GetAccessControlId();
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
        uint32_t priority = APP_LEVEL_BIND_TYPE;
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT) {
            priority = IDENTICAL_ACCOUNT_TYPE;
        } else if (item.GetBindType() == DEVICE && item.GetAuthenticationType() == ALLOW_AUTH_ALWAYS) {
            priority = DEVICE_LEVEL_BIND_TYPE;
        } else if (item.GetBindLevel() == DEVICE && item.GetAuthenticationType() == ALLOW_AUTH_ONCE) {
            priority = DEVICE_LEVEL_BIND_TYPE;
            offlineParam.pkgNameVec.push_back(item.GetAccesser().GetAccesserBundleName());
            DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
            offlineParam.leftAclNumber--;
        } else if ((item.GetAccesser().GetAccesserDeviceId() == requestDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == trustDeviceId) ||
            (item.GetAccesser().GetAccesserDeviceId() == trustDeviceId &&
            item.GetAccessee().GetAccesseeDeviceId() == requestDeviceId)) {
            priority = APP_LEVEL_BIND_TYPE;
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
    std::vector<AccessControlProfile> profiles = GetAccessControlProfile();
    LOGI("AccessControlProfile size is %d.", profiles.size());
    for (auto &item : profiles) {
        if ((item.GetAccesser().GetAccesserUserId() == userId &&
            item.GetAccesser().GetAccesserAccountId() == accountId) ||
            (item.GetAccessee().GetAccesseeUserId() == userId &&
            item.GetAccessee().GetAccesseeAccountId() == accountId)) {
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
        if (item.GetBindType() == DM_IDENTICAL_ACCOUNT && item.GetStatus() == ACTIVE) {
            LOGE("Identical account forbid unbind.");
            offlineParam.bindType = INVALIED_TYPE;
            return offlineParam;
        }
        if (item.GetTrustDeviceId() == remoteDeviceId && item.GetStatus() == ACTIVE) {
            offlineParam.leftAclNumber++;
            if (item.GetBindLevel() == DEVICE && item.GetBindType() != DM_IDENTICAL_ACCOUNT &&
                item.GetAccesser().GetAccesserBundleName() == pkgName) {
                LOGI("DeleteAccessControlList device unbind.");
                offlineParam.bindType = DEVICE_LEVEL_BIND_TYPE;
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
            if (offlineParam.bindType == DEVICE_LEVEL_BIND_TYPE) {
                DistributedDeviceProfileClient::GetInstance().DeleteAccessControlProfile(item.GetAccessControlId());
                offlineParam.leftAclNumber--;
            } else if (item.GetAccesser().GetAccesserBundleName() == pkgName &&
                item.GetAccessee().GetAccesseeBundleName() == pkgName) {
                offlineParam.bindType = APP_LEVEL_BIND_TYPE;
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
} // namespace DistributedHardware
} // namespace OHOS
