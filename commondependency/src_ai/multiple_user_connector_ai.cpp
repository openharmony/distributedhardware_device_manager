/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "multiple_user_connector_ai.h"

#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "account_info.h"
#include "ipc_skeleton.h"
#include "ohos_account_kits.h"
#ifdef OS_ACCOUNT_PART_EXISTS
#include "os_account_manager.h"
#include "os_account_subprofile_client.h"
using namespace OHOS::AccountSA;
#endif // OS_ACCOUNT_PART_EXISTS

namespace OHOS {
namespace DistributedHardware {
int32_t MultipleUserConnectorAi::oldUserId_ = -1;
std::string MultipleUserConnectorAi::accountId_ = "";
std::string MultipleUserConnectorAi::accountName_ = "";
std::mutex MultipleUserConnectorAi::lock_;
std::map<int32_t, std::map<int32_t, DMAccountInfo>> MultipleUserConnectorAi::dmAccountInfoMap_ = {};
std::mutex MultipleUserConnectorAi::dmAccountInfoMaplock_;
std::mutex MultipleUserConnectorAi::currentForgroundUserIdLock_;
int32_t MultipleUserConnectorAi::currentForgroundUserId_ = -1;
#ifndef OS_ACCOUNT_PART_EXISTS
const int32_t DEFAULT_OS_ACCOUNT_ID = 0; // 0 is the default id when there is no os_account part
#endif // OS_ACCOUNT_PART_EXISTS
const char* DM_MDM_CONSTRAINT = "constraint.distributed.transmission.outgoing";
const int32_t GET_USERID_MAX_NUM = 3;
const int32_t USLEEP_TIME_US_50000 = 50000; // 50ms

bool MultipleUserConnectorAi::FillDMAccountInfoFromSubProfile(int32_t userId, int32_t subProfileId,
    DMAccountInfo &dmAccountInfo)
{
#if OS_ACCOUNT_PART_EXISTS
#ifdef CAR_DEVICE_ENABLE
    OhosAccountInfo accountInfo;
    OsAccountSubspaceResult result;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountSubProfile(userId, subProfileId,
        result, accountInfo);
    if (ret == 0 && accountInfo.uid_ != "") {
        dmAccountInfo.accountId = accountInfo.uid_;
        dmAccountInfo.accountName = accountInfo.name_;
        return true;
    }
#else
    (void)userId;
    (void)subProfileId;
    dmAccountInfo.accountId = GetOhosAccountIdByUserId(userId);
    dmAccountInfo.accountName = GetOhosAccountNameByUserId(userId);
    return true;
#endif
#endif
    return false;
}

int32_t MultipleUserConnectorAi::GetCurrentAccountUserID(void)
{
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("error ret: %{public}d", ret);
        return -1;
    }
    return ids[0];
}

DM_EXPORT int32_t MultipleUserConnectorAi::TryGetCurrentAccountUserID(void)
{
    int32_t retryNum = 0;
    std::vector<int> ids;
    ErrCode ret = 0;
    while (retryNum < GET_USERID_MAX_NUM) {
        ret = OsAccountManager::QueryActiveOsAccountIds(ids);
        if (ret == 0 && !ids.empty()) {
            break;
        }
        usleep(USLEEP_TIME_US_50000);
        retryNum++;
    }
    if (ret != 0 || ids.empty()) {
        LOGE("error ret: %{public}d", ret);
        return -1;
    }
    return ids[0];
}

DM_EXPORT bool MultipleUserConnectorAi::CheckMDMControl()
{
    bool isMDMControl = false;
#ifdef OS_ACCOUNT_PART_EXISTS
    int32_t activeAccountId = GetCurrentAccountUserID();
    int32_t ret =
        AccountSA::OsAccountManager::CheckOsAccountConstraintEnabled(activeAccountId, DM_MDM_CONSTRAINT, isMDMControl);
    if (ret != ERR_OK) {
        LOGE("CheckOsAccountConstraintEnabled failed, ret %{public}d.", ret);
        return false;
    }
    LOGI("CheckOsAccountConstraintEnabled success, isMDMControl %{public}d.", isMDMControl);
#endif
    return isMDMControl;
}

DM_EXPORT std::string MultipleUserConnectorAi::GetOhosAccountId(void)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOhosAccountInfo(accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
}

DM_EXPORT std::string MultipleUserConnectorAi::GetOhosAccountIdByUserId(int32_t userId)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
}

DM_EXPORT std::string MultipleUserConnectorAi::GetOhosAccountNameByUserId(int32_t userId)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.name_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.name_;
}

DM_EXPORT std::string MultipleUserConnectorAi::GetOhosAccountName(void)
{
    auto accountInfo = OhosAccountKits::GetInstance().QueryOhosAccountInfo();
    if (!accountInfo.first) {
        LOGE("QueryOhosAccountInfo failed.");
        return "";
    }
    if (accountInfo.second.name_.empty()) {
        LOGE("QueryOhosAccountInfo name empty.");
        return "";
    }
    return accountInfo.second.name_;
}

void MultipleUserConnectorAi::GetTokenIdAndForegroundUserId(uint32_t &tokenId, int32_t &userId)
{
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    userId = GetFirstForegroundUserId();
}

DM_EXPORT void MultipleUserConnectorAi::GetTokenId(uint32_t &tokenId)
{
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
}

void MultipleUserConnectorAi::GetCallerUserId(int32_t &userId)
{
    int32_t uid = OHOS::IPCSkeleton::GetCallingUid();
    ErrCode ret = OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (ret != 0) {
        LOGE("GetOsAccountLocalIdFromUid error ret: %{public}d", ret);
    }
    return;
}

DM_EXPORT void MultipleUserConnectorAi::SetSwitchOldUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(lock_);
    oldUserId_ = userId;
}

int32_t MultipleUserConnectorAi::GetSwitchOldUserId(void)
{
    std::lock_guard<std::mutex> lock(lock_);
    return oldUserId_;
}

DM_EXPORT void MultipleUserConnectorAi::SetSwitchOldAccountId(std::string accountId)
{
    std::lock_guard<std::mutex> lock(lock_);
    accountId_ = accountId;
}

std::string MultipleUserConnectorAi::GetSwitchOldAccountId(void)
{
    std::lock_guard<std::mutex> lock(lock_);
    return accountId_;
}

void MultipleUserConnectorAi::SetSwitchOldAccountName(std::string accountName)
{
    std::lock_guard<std::mutex> lock(lock_);
    accountName_ = accountName;
}

std::string MultipleUserConnectorAi::GetSwitchOldAccountName(void)
{
    std::lock_guard<std::mutex> lock(lock_);
    return accountName_;
}

DM_EXPORT void MultipleUserConnectorAi::SetAccountInfo(int32_t userId, int32_t subProfileId,
    DMAccountInfo dmAccountInfo)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    CHECK_SIZE_VOID(dmAccountInfoMap_);
    dmAccountInfoMap_[userId][subProfileId] = dmAccountInfo;
    LOGI("userId: %{public}d, subProfileId: %{public}d, accountId: %{public}s, accountName: %{public}s",
        userId, subProfileId, GetAnonyString(dmAccountInfo.accountId).c_str(),
        GetAnonyString(dmAccountInfo.accountName).c_str());
}

DM_EXPORT DMAccountInfo MultipleUserConnectorAi::GetAccountInfo(int32_t userId, int32_t subProfileId)
{
    DMAccountInfo dmAccountInfo;
    {
        std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
        if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
            auto& subMap = dmAccountInfoMap_[userId];
            if (subMap.find(subProfileId) != subMap.end()) {
                dmAccountInfo = subMap[subProfileId];
                return dmAccountInfo;
            }
        }
    }
    LOGE("userId %{public}d, subProfileId %{public}d is not exist.", userId, subProfileId);
    return dmAccountInfo;
}

DM_EXPORT void MultipleUserConnectorAi::DeleteAccountInfoByUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    LOGI("userId: %{public}d", userId);
    if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
        dmAccountInfoMap_.erase(userId);
    }
}

DM_EXPORT void MultipleUserConnectorAi::DeleteAccountInfo(int32_t userId, int32_t subProfileId)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    LOGI("userId: %{public}d, subProfileId: %{public}d", userId, subProfileId);
    if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
        auto& subMap = dmAccountInfoMap_[userId];
        if (subMap.find(subProfileId) != subMap.end()) {
            subMap.erase(subProfileId);
            if (subMap.empty()) {
                dmAccountInfoMap_.erase(userId);
            }
        }
    }
}

DM_EXPORT int32_t MultipleUserConnectorAi::GetForegroundUserIds(
    std::vector<int32_t> &userVec)
{
    userVec.clear();
    std::vector<AccountSA::ForegroundOsAccount> accounts;
    ErrCode ret = OsAccountManager::GetForegroundOsAccounts(accounts);
    if (ret != 0 || accounts.empty()) {
        LOGE("error ret: %{public}d", ret);
        return ret;
    }
    for (auto &account : accounts) {
        userVec.push_back(account.localId);
    }
    return DM_OK;
}

int32_t MultipleUserConnectorAi::GetFirstForegroundUserId(void)
{
    std::vector<int32_t> userVec;
    int32_t ret = GetForegroundUserIds(userVec);
    if (ret != DM_OK || userVec.size() == 0) {
        LOGE("get userid error ret: %{public}d.", ret);
        return -1;
    }
    return userVec[0];
}

DM_EXPORT int32_t MultipleUserConnectorAi::GetBackgroundUserIds(
    std::vector<int32_t> &userIdVec)
{
    userIdVec.clear();
    std::vector<OsAccountInfo> allOsAccounts;
    ErrCode ret = OsAccountManager::QueryAllCreatedOsAccounts(allOsAccounts);
    if (ret != 0) {
        LOGE("Get all created accounts error, ret: %{public}d", ret);
        return ret;
    }

    std::vector<AccountSA::ForegroundOsAccount> foregroundAccounts;
    ret = OsAccountManager::GetForegroundOsAccounts(foregroundAccounts);
    if (ret != 0) {
        LOGE("Get foreground accounts error ret: %{public}d", ret);
        return ret;
    }

    std::vector<int32_t> allUserIds;
    std::vector<int32_t> foregroundUserIds;
    for (const auto &u : allOsAccounts) {
        allUserIds.push_back(u.GetLocalId());
    }
    for (const auto &u : foregroundAccounts) {
        foregroundUserIds.push_back(u.localId);
    }

    for (const auto &userId : allUserIds) {
        if (std::find(foregroundUserIds.begin(), foregroundUserIds.end(), userId) == foregroundUserIds.end()) {
            userIdVec.push_back(userId);
        }
    }
    return DM_OK;
}

int32_t MultipleUserConnectorAi::GetAllUserIds(std::vector<int32_t> &userIdVec)
{
    userIdVec.clear();
    std::vector<OsAccountInfo> allOsAccounts;
    ErrCode ret = OsAccountManager::QueryAllCreatedOsAccounts(allOsAccounts);
    if (ret != 0) {
        LOGE("Get all created accounts error, ret: %{public}d", ret);
        return ret;
    }

    for (const auto &u : allOsAccounts) {
        userIdVec.push_back(u.GetLocalId());
    }
    return DM_OK;
}

DM_EXPORT std::string MultipleUserConnectorAi::GetAccountNickName(int32_t userId)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.nickname_;
}

bool MultipleUserConnectorAi::IsUserUnlocked(int32_t userId)
{
    bool isUserUnlocked = false;
    ErrCode ret = OsAccountManager::IsOsAccountVerified(userId, isUserUnlocked);
    if (ret != 0) {
        LOGE("error ret: %{public}d", ret);
        return false;
    }
    return isUserUnlocked;
}

DM_EXPORT void MultipleUserConnectorAi::ClearLockedUser(
    std::vector<int32_t> &foregroundUserVec)
{
    for (auto iter = foregroundUserVec.begin(); iter != foregroundUserVec.end();) {
        if (!IsUserUnlocked(*iter)) {
            iter = foregroundUserVec.erase(iter);
        } else {
            ++iter;
        }
    }
}

DM_EXPORT void MultipleUserConnectorAi::ClearLockedUser(
    std::vector<int32_t> &foregroundUserVec, std::vector<int32_t> &backgroundUserVec)
{
    for (auto iter = foregroundUserVec.begin(); iter != foregroundUserVec.end();) {
        if (!IsUserUnlocked(*iter)) {
            backgroundUserVec.push_back(*iter);
            iter = foregroundUserVec.erase(iter);
        } else {
            ++iter;
        }
    }
}

DM_EXPORT DMAccountInfo MultipleUserConnectorAi::GetCurrentDMAccountInfo()
{
    DMAccountInfo dmAccountInfo;
#if OS_ACCOUNT_PART_EXISTS
    int32_t userId = GetCurrentAccountUserID();
#ifdef CAR_DEVICE_ENABLE
    dmAccountInfo.subProfileId = GetSubProfileIdByUserId(userId);
    if (dmAccountInfo.subProfileId < 0) {
        dmAccountInfo.accountId = GetOhosAccountIdByUserId(userId);
        dmAccountInfo.accountName = GetOhosAccountNameByUserId(userId);
        return dmAccountInfo;
    }
    FillDMAccountInfoFromSubProfile(userId, dmAccountInfo.subProfileId, dmAccountInfo);
#else
    dmAccountInfo.subProfileId = 0;
    dmAccountInfo.accountId = GetOhosAccountId();
    dmAccountInfo.accountName = GetOhosAccountName();
#endif
#endif
    return dmAccountInfo;
}

DM_EXPORT void MultipleUserConnectorAi::GetCallingTokenId(uint32_t &tokenId)
{
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
}

DM_EXPORT int32_t MultipleUserConnectorAi::GetUserIdByDisplayId(int32_t displayId)
{
    LOGI("displayId %{public}d", displayId);
    int32_t userId = -1;
    if (displayId == -1) {
        userId = GetFirstForegroundUserId();
        return userId;
    }
#ifdef OS_ACCOUNT_PART_EXISTS
    int32_t ret = OHOS::AccountSA::OsAccountManager::
        GetForegroundOsAccountLocalId(static_cast<uint64_t>(displayId), userId);
    if (ret != DM_OK) {
        LOGE("GetForegroundOsAccountLocalId failed ret %{public}d.", ret);
    }
#endif // OS_ACCOUNT_PART_EXISTS
    return userId;
}

DM_EXPORT void MultipleUserConnectorAi::UpdateForgroundUserId()
{
    int32_t userId = MultipleUserConnectorAi::GetCurrentAccountUserID();
    {
        std::lock_guard<std::mutex> lock(currentForgroundUserIdLock_);
        currentForgroundUserId_ = userId;
    }
}

int32_t MultipleUserConnectorAi::GetForgroundUserId(void)
{
    int32_t userId = -1;
    {
        std::lock_guard<std::mutex> lock(currentForgroundUserIdLock_);
        userId = currentForgroundUserId_;
    }
    if (userId == -1) {
        std::vector<int> ids;
        ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
        if (ret != 0 || ids.empty()) {
            LOGE("error ret: %{public}d", ret);
            return -1;
        }
        {
            std::lock_guard<std::mutex> lock(currentForgroundUserIdLock_);
            currentForgroundUserId_ = ids[0];
        }
        return ids[0];
    } else {
        return userId;
    }
}

DM_EXPORT int32_t MultipleUserConnectorAi::GetSubProfileIdByUserId(int32_t userId)
{
    int32_t subProfileId = -1;
    ErrCode ret = OsAccountSubProfileClient::GetInstance().GetOsAccountForegroundSubProfileId(userId, subProfileId);
    if (ret != 0) {
        LOGE("GetOsAccountForegroundSubProfileId failed, userId %{public}d, ret %{public}d", userId, ret);
        return -1;
    }
    return subProfileId;
}

DM_EXPORT void MultipleUserConnectorAi::CacheAllUsersAccountInfo()
{
    std::vector<int32_t> userIdVec;
    int32_t ret = GetAllUserIds(userIdVec);
    if (ret != DM_OK || userIdVec.empty()) {
        LOGE("GetAllUserIds failed or empty, ret: %{public}d", ret);
        return;
    }
    LOGI("CacheAllUsersAccountInfo: total %{public}zu users", userIdVec.size());
    for (int32_t userId : userIdVec) {
        if (userId <= 0) {
            continue;
        }
        DMAccountInfo dmAccountInfo;
#ifdef CAR_DEVICE_ENABLE
        dmAccountInfo.subProfileId = GetSubProfileIdByUserId(userId);
        if (dmAccountInfo.subProfileId < 0) {
            LOGI("User %{public}d has no valid subProfileId, skip cache", userId);
            continue;
        }
        FillDMAccountInfoFromSubProfile(userId, dmAccountInfo.subProfileId, dmAccountInfo);
#else
        dmAccountInfo.subProfileId = 0;
        dmAccountInfo.accountId = GetOhosAccountIdByUserId(userId);
        dmAccountInfo.accountName = GetOhosAccountNameByUserId(userId);
#endif
        SetAccountInfo(userId, dmAccountInfo.subProfileId, dmAccountInfo);
        LOGI("Cached user %{public}d: accountId %{public}s, subProfileId %{public}d, accountName %{public}s",
            userId, GetAnonyString(dmAccountInfo.accountId).c_str(), dmAccountInfo.subProfileId,
            GetAnonyString(dmAccountInfo.accountName).c_str());
    }
}

DM_EXPORT std::string MultipleUserConnectorAi::GetAccountIdBySubProfileId(int32_t userId, int32_t subProfileId)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
        auto& subMap = dmAccountInfoMap_[userId];
        if (subMap.find(subProfileId) != subMap.end()) {
            return subMap[subProfileId].accountId;
        }
    }
    LOGE("GetAccountIdBySubProfileId failed: userId %{public}d, subProfileId %{public}d", userId, subProfileId);
    return "";
}

DM_EXPORT int32_t MultipleUserConnectorAi::GetSubProfileIdByAccountId(int32_t userId, const std::string &accountId)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
        auto& subMap = dmAccountInfoMap_[userId];
        for (const auto& pair : subMap) {
            if (pair.second.accountId == accountId) {
                return pair.first;
            }
        }
    }
    LOGE("GetSubProfileIdByAccountId failed: userId %{public}d, accountId %{public}s",
        userId, GetAnonyString(accountId).c_str());
    return -1;
}

DM_EXPORT DMAccountInfo MultipleUserConnectorAi::GetDMAccountInfoByUserId(int32_t userId)
{
    DMAccountInfo dmAccountInfo;
    int32_t subProfileId = GetSubProfileIdByUserId(userId);
    if (subProfileId < 0) {
        LOGI("User %{public}d has no valid subProfileId, fallback to GetOhosAccountIdByUserId", userId);
        dmAccountInfo.accountId = GetOhosAccountIdByUserId(userId);
        dmAccountInfo.accountName = GetOhosAccountNameByUserId(userId);
        return dmAccountInfo;
    }
    dmAccountInfo.subProfileId = subProfileId;
    if (FillDMAccountInfoFromSubProfile(userId, subProfileId, dmAccountInfo)) {
        LOGI("GetDMAccountInfoByUserId: userId %{public}d, subProfileId %{public}d, accountId %{public}s",
            userId, subProfileId, GetAnonyString(dmAccountInfo.accountId).c_str());
    } else {
        LOGE("FillDMAccountInfoFromSubProfile failed, fallback to GetOhosAccountIdByUserId");
        dmAccountInfo.accountId = GetOhosAccountIdByUserId(userId);
        dmAccountInfo.accountName = GetOhosAccountNameByUserId(userId);
    }
    return dmAccountInfo;
}

DM_EXPORT int32_t MultipleUserConnectorAi::GetAppIndexByUserId(int32_t userId)
{
    int32_t subProfileId = GetSubProfileIdByUserId(userId);
    if (subProfileId < 0) {
        LOGI("User %{public}d has no valid subProfileId, return default appIndex 0", userId);
        return 0;
    }
    OhosAccountInfo accountInfo;
    OsAccountSubspaceResult result;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountSubProfile(userId, subProfileId, result, accountInfo);
    if (ret == 0) {
        int32_t appIndex = result.index;
        LOGI("GetAppIndexByUserId: userId %{public}d, subProfileId %{public}d, appIndex %{public}d",
            userId, subProfileId, appIndex);
        return appIndex;
    } else {
        LOGE("GetOsAccountSubProfile failed, ret %{public}d, return default appIndex 0", ret);
        return 0;
    }
}

DM_EXPORT DMAccountInfo MultipleUserConnectorAi::GetDMAccountInfoBySubProfileId(int32_t userId, int32_t subProfileId)
{
    DMAccountInfo dmAccountInfo;
#ifdef CAR_DEVICE_ENABLE
    FillDMAccountInfoFromSubProfile(userId, subProfileId, dmAccountInfo);
#endif
    return dmAccountInfo;
}

DM_EXPORT std::string MultipleUserConnectorAi::GetAccountIdByUserId(int32_t userId)
{
    int32_t subProfileId = -1;
    ErrCode ret = OsAccountSubProfileClient::GetInstance().GetOsAccountForegroundSubProfileId(userId, subProfileId);
    if (ret != 0) {
        LOGE("GetOsAccountForegroundSubProfileId failed, userId %{public}d, ret %{public}d", userId, ret);
        return "";
    }
    OhosAccountInfo accountInfo;
    OsAccountSubspaceResult result;
    ret = OhosAccountKits::GetInstance().GetOsAccountSubProfile(userId, subProfileId, result, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        return "";
    }
    return accountInfo.uid_;
}
} // namespace DistributedHardware
} // namespace OHOS