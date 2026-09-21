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

#include "multiple_user_connector.h"

#include "dm_error_type.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "account_info.h"
#include "ipc_skeleton.h"
#include "ohos_account_kits.h"
#ifdef OS_ACCOUNT_PART_EXISTS
#include "os_account_manager.h"
using namespace OHOS::AccountSA;
#endif // OS_ACCOUNT_PART_EXISTS

namespace OHOS {
namespace DistributedHardware {
int32_t MultipleUserConnector::oldUserId_ = -1;
std::string MultipleUserConnector::accountId_ = "";
std::string MultipleUserConnector::accountName_ = "";
std::mutex MultipleUserConnector::lock_;
std::map<int32_t, DMAccountInfo> MultipleUserConnector::dmAccountInfoMap_ = {};
std::mutex MultipleUserConnector::dmAccountInfoMaplock_;
#ifndef OS_ACCOUNT_PART_EXISTS
const int32_t DEFAULT_OS_ACCOUNT_ID = 0; // 0 is the default id when there is no os_account part
#endif // OS_ACCOUNT_PART_EXISTS
const char* DM_MDM_CONSTRAINT = "constraint.distributed.transmission.outgoing";

int32_t MultipleUserConnector::GetCurrentAccountUserID(void)
{
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("GetCurrentAccountUserID error ret: %{public}d", ret);
        return -1;
    }
    return ids[0];
}

DM_EXPORT bool MultipleUserConnector::CheckMDMControl()
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

DM_EXPORT std::string MultipleUserConnector::GetOhosAccountId(void)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOhosAccountInfo(accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("GetOhosAccountId error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
}

DM_EXPORT std::string MultipleUserConnector::GetOhosAccountIdByUserId(int32_t userId)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
}

DM_EXPORT std::string MultipleUserConnector::GetOhosAccountNameByUserId(int32_t userId)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.name_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.name_;
}

DM_EXPORT std::string MultipleUserConnector::GetOhosAccountName(void)
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

void MultipleUserConnector::GetTokenIdAndForegroundUserId(uint32_t &tokenId, int32_t &userId)
{
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    userId = GetFirstForegroundUserId();
}

DM_EXPORT void MultipleUserConnector::GetTokenId(uint32_t &tokenId)
{
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
}

void MultipleUserConnector::GetCallerUserId(int32_t &userId)
{
    int32_t uid = OHOS::IPCSkeleton::GetCallingUid();
    ErrCode ret = OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (ret != 0) {
        LOGE("GetOsAccountLocalIdFromUid error ret: %{public}d", ret);
    }
    return;
}

DM_EXPORT void MultipleUserConnector::SetSwitchOldUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(lock_);
    oldUserId_ = userId;
}

int32_t MultipleUserConnector::GetSwitchOldUserId(void)
{
    std::lock_guard<std::mutex> lock(lock_);
    return oldUserId_;
}

DM_EXPORT void MultipleUserConnector::SetSwitchOldAccountId(std::string accountId)
{
    std::lock_guard<std::mutex> lock(lock_);
    accountId_ = accountId;
}

std::string MultipleUserConnector::GetSwitchOldAccountId(void)
{
    std::lock_guard<std::mutex> lock(lock_);
    return accountId_;
}

void MultipleUserConnector::SetSwitchOldAccountName(std::string accountName)
{
    std::lock_guard<std::mutex> lock(lock_);
    accountName_ = accountName;
}

std::string MultipleUserConnector::GetSwitchOldAccountName(void)
{
    std::lock_guard<std::mutex> lock(lock_);
    return accountName_;
}

DM_EXPORT void MultipleUserConnector::SetAccountInfo(int32_t userId,
    DMAccountInfo dmAccountInfo)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    CHECK_SIZE_VOID(dmAccountInfoMap_);
    dmAccountInfoMap_[userId] = dmAccountInfo;
}

DM_EXPORT DMAccountInfo MultipleUserConnector::GetAccountInfoByUserId(int32_t userId)
{
    DMAccountInfo dmAccountInfo;
    {
        std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
        if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
            dmAccountInfo = dmAccountInfoMap_[userId];
            return dmAccountInfo;
        }
    }
    LOGE("userId is not exist.");
    return dmAccountInfo;
}

DM_EXPORT void MultipleUserConnector::DeleteAccountInfoByUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    LOGI("userId: %{public}d", userId);
    if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
        dmAccountInfoMap_.erase(userId);
    }
}

DM_EXPORT int32_t MultipleUserConnector::GetForegroundUserIds(
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

int32_t MultipleUserConnector::GetFirstForegroundUserId(void)
{
    std::vector<int32_t> userVec;
    int32_t ret = GetForegroundUserIds(userVec);
    if (ret != DM_OK || userVec.size() == 0) {
        LOGE("get userid error ret: %{public}d.", ret);
        return -1;
    }
    return userVec[0];
}

DM_EXPORT int32_t MultipleUserConnector::GetBackgroundUserIds(
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

int32_t MultipleUserConnector::GetAllUserIds(std::vector<int32_t> &userIdVec)
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

DM_EXPORT std::string MultipleUserConnector::GetAccountNickName(int32_t userId)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.nickname_;
}

bool MultipleUserConnector::IsUserUnlocked(int32_t userId)
{
    bool isUserUnlocked = false;
    ErrCode ret = OsAccountManager::IsOsAccountVerified(userId, isUserUnlocked);
    if (ret != 0) {
        LOGE("IsUserUnlocked error ret: %{public}d", ret);
        return false;
    }
    return isUserUnlocked;
}

DM_EXPORT void MultipleUserConnector::ClearLockedUser(
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

DM_EXPORT void MultipleUserConnector::ClearLockedUser(
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

DM_EXPORT DMAccountInfo MultipleUserConnector::GetCurrentDMAccountInfo()
{
    DMAccountInfo dmAccountInfo;
    dmAccountInfo.accountId = GetOhosAccountId();
    dmAccountInfo.accountName = GetOhosAccountName();
    return dmAccountInfo;
}

DM_EXPORT void MultipleUserConnector::GetCallingTokenId(uint32_t &tokenId)
{
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
}

DM_EXPORT int32_t MultipleUserConnector::GetUserIdByDisplayId(int32_t displayId)
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
} // namespace DistributedHardware
} // namespace OHOS
