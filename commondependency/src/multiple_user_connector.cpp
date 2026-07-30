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

#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "dm_constants.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "account_info.h"
#include "ipc_skeleton.h"
#include "ohos_account_kits.h"
#ifdef OS_ACCOUNT_PART_EXISTS
#include "os_account_manager.h"
#include "os_account_subprofile_client.h"
using namespace OHOS::AccountSA;
#endif // OS_ACCOUNT_PART_EXISTS
#endif

namespace OHOS {
namespace DistributedHardware {
int32_t MultipleUserConnector::oldUserId_ = -1;
std::string MultipleUserConnector::accountId_ = "";
std::string MultipleUserConnector::accountName_ = "";
std::mutex MultipleUserConnector::lock_;
std::map<int32_t, std::map<int32_t, DMAccountInfo>> MultipleUserConnector::dmAccountInfoMap_ = {};
std::mutex MultipleUserConnector::dmAccountInfoMaplock_;
std::mutex MultipleUserConnector::currentForgroundUserIdLock_;
int32_t MultipleUserConnector::currentForgroundUserId_ = -1;
#ifndef OS_ACCOUNT_PART_EXISTS
const int32_t DEFAULT_OS_ACCOUNT_ID = 0; // 0 is the default id when there is no os_account part
#endif // OS_ACCOUNT_PART_EXISTS
const char* DM_MDM_CONSTRAINT = "constraint.distributed.transmission.outgoing";
const int32_t GET_USERID_MAX_NUM = 3;
const int32_t USLEEP_TIME_US_50000 = 50000; // 50ms

bool MultipleUserConnector::FillDMAccountInfoFromSubProfile(int32_t userId, int32_t subProfileId,
    DMAccountInfo &dmAccountInfo)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    (void)userId;
    (void)subProfileId;
    return false;
#elif OS_ACCOUNT_PART_EXISTS
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

int32_t MultipleUserConnector::GetCurrentAccountUserID(void)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return 0;
#elif OS_ACCOUNT_PART_EXISTS
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("error ret: %{public}d", ret);
        return -1;
    }
    return ids[0];
#else // OS_ACCOUNT_PART_EXISTS
    return DEFAULT_OS_ACCOUNT_ID;
#endif
}

DM_EXPORT int32_t MultipleUserConnector::TryGetCurrentAccountUserID(void)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return 0;
#elif OS_ACCOUNT_PART_EXISTS
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
#else // OS_ACCOUNT_PART_EXISTS
    return DEFAULT_OS_ACCOUNT_ID;
#endif
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
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return "";
#elif OS_ACCOUNT_PART_EXISTS
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOhosAccountInfo(accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
#else
    return "";
#endif
}

DM_EXPORT std::string MultipleUserConnector::GetOhosAccountIdByUserId(int32_t userId)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    (void)userId;
    return "";
#elif OS_ACCOUNT_PART_EXISTS
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
#else
    (void)userId;
    return "";
#endif
}

DM_EXPORT std::string MultipleUserConnector::GetOhosAccountNameByUserId(int32_t userId)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    (void)userId;
    return "";
#elif OS_ACCOUNT_PART_EXISTS
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.name_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.name_;
#else
    (void)userId;
    return "";
#endif
}

DM_EXPORT std::string MultipleUserConnector::GetOhosAccountName(void)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return "";
#elif OS_ACCOUNT_PART_EXISTS
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
#else
    return "";
#endif
}

void MultipleUserConnector::GetTokenIdAndForegroundUserId(uint32_t &tokenId, int32_t &userId)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
#else
    (void)tokenId;
#endif
    userId = GetFirstForegroundUserId();
}

DM_EXPORT void MultipleUserConnector::GetTokenId(uint32_t &tokenId)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
#else
    (void)tokenId;
#endif
}

void MultipleUserConnector::GetCallerUserId(int32_t &userId)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    (void)userId;
    return;
#elif OS_ACCOUNT_PART_EXISTS
    int32_t uid = OHOS::IPCSkeleton::GetCallingUid();
    ErrCode ret = OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (ret != 0) {
        LOGE("GetOsAccountLocalIdFromUid error ret: %{public}d", ret);
    }
    return;
#else // OS_ACCOUNT_PART_EXISTS
    (void)userId;
    return;
#endif
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

DM_EXPORT void MultipleUserConnector::SetAccountInfo(int32_t userId, int32_t subProfileId,
    DMAccountInfo dmAccountInfo)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    CHECK_SIZE_VOID(dmAccountInfoMap_);
    dmAccountInfoMap_[userId][subProfileId] = dmAccountInfo;
    LOGI("userId: %{public}d, subProfileId: %{public}d, accountId: %{public}s, accountName: %{public}s",
        userId, subProfileId, GetAnonyString(dmAccountInfo.accountId).c_str(),
        GetAnonyString(dmAccountInfo.accountName).c_str());
}

DM_EXPORT DMAccountInfo MultipleUserConnector::GetAccountInfo(int32_t userId, int32_t subProfileId)
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

DM_EXPORT void MultipleUserConnector::DeleteAccountInfoByUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(dmAccountInfoMaplock_);
    LOGI("userId: %{public}d", userId);
    if (dmAccountInfoMap_.find(userId) != dmAccountInfoMap_.end()) {
        dmAccountInfoMap_.erase(userId);
    }
}

DM_EXPORT void MultipleUserConnector::DeleteAccountInfo(int32_t userId, int32_t subProfileId)
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

DM_EXPORT int32_t MultipleUserConnector::GetForegroundUserIds(
    std::vector<int32_t> &userVec)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    userVec.push_back(DEFAULT_OS_ACCOUNT_ID);
    return DM_OK;
#elif OS_ACCOUNT_PART_EXISTS
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
#else // OS_ACCOUNT_PART_EXISTS
    userVec.push_back(DEFAULT_OS_ACCOUNT_ID);
    return DM_OK;
#endif
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
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return DM_OK;
#elif OS_ACCOUNT_PART_EXISTS
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
#else
    return DM_OK;
#endif
}

int32_t MultipleUserConnector::GetAllUserIds(std::vector<int32_t> &userIdVec)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return DM_OK;
#elif OS_ACCOUNT_PART_EXISTS
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
#else
    return DM_OK;
#endif
}

DM_EXPORT std::string MultipleUserConnector::GetAccountNickName(int32_t userId)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return "";
#elif OS_ACCOUNT_PART_EXISTS
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.nickname_;
#else
    return "";
#endif
}

bool MultipleUserConnector::IsUserUnlocked(int32_t userId)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return true;
#elif OS_ACCOUNT_PART_EXISTS
    bool isUserUnlocked = false;
    ErrCode ret = OsAccountManager::IsOsAccountVerified(userId, isUserUnlocked);
    if (ret != 0) {
        LOGE("error ret: %{public}d", ret);
        return false;
    }
    return isUserUnlocked;
#else
    return true;
#endif
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
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return dmAccountInfo;
#elif OS_ACCOUNT_PART_EXISTS
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

DM_EXPORT void MultipleUserConnector::GetCallingTokenId(uint32_t &tokenId)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
#else
    (void)tokenId;
#endif
}

DM_EXPORT int32_t MultipleUserConnector::GetUserIdByDisplayId(int32_t displayId)
{
    LOGI("displayId %{public}d", displayId);
    int32_t userId = -1;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
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
#endif
    return userId;
}

DM_EXPORT void MultipleUserConnector::UpdateForgroundUserId()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    {
        std::lock_guard<std::mutex> lock(currentForgroundUserIdLock_);
        currentForgroundUserId_ = userId;
    }
#endif
}

int32_t MultipleUserConnector::GetForgroundUserId(void)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return 0;
#elif OS_ACCOUNT_PART_EXISTS
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

#else // OS_ACCOUNT_PART_EXISTS
    return DEFAULT_OS_ACCOUNT_ID;
#endif
}

DM_EXPORT int32_t MultipleUserConnector::GetSubProfileIdByUserId(int32_t userId)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    (void)userId;
    return -1;
#elif OS_ACCOUNT_PART_EXISTS
    int32_t subProfileId = -1;
    ErrCode ret = OsAccountSubProfileClient::GetInstance().GetOsAccountForegroundSubProfileId(userId, subProfileId);
    if (ret != 0) {
        LOGE("GetOsAccountForegroundSubProfileId failed, userId %{public}d, ret %{public}d", userId, ret);
        return -1;
    }
    return subProfileId;
#else
    (void)userId;
    return -1;
#endif
}

DM_EXPORT void MultipleUserConnector::CacheAllUsersAccountInfo()
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return;
#elif OS_ACCOUNT_PART_EXISTS
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

#else
    return;
#endif
}

DM_EXPORT std::string MultipleUserConnector::GetAccountIdBySubProfileId(int32_t userId, int32_t subProfileId)
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

DM_EXPORT int32_t MultipleUserConnector::GetSubProfileIdByAccountId(int32_t userId, const std::string &accountId)
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

DM_EXPORT DMAccountInfo MultipleUserConnector::GetDMAccountInfoByUserId(int32_t userId)
{
    DMAccountInfo dmAccountInfo;
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    dmAccountInfo.accountId = GetOhosAccountIdByUserId(userId);
    dmAccountInfo.accountName = GetOhosAccountNameByUserId(userId);
    return dmAccountInfo;
#elif OS_ACCOUNT_PART_EXISTS
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
#else
    dmAccountInfo.accountId = GetOhosAccountIdByUserId(userId);
    dmAccountInfo.accountName = GetOhosAccountNameByUserId(userId);
#endif
    return dmAccountInfo;
}

DM_EXPORT int32_t MultipleUserConnector::GetAppIndexByUserId(int32_t userId)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return 0;
#elif OS_ACCOUNT_PART_EXISTS
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
#else
    return 0;
#endif
}

DM_EXPORT DMAccountInfo MultipleUserConnector::GetDMAccountInfoBySubProfileId(int32_t userId, int32_t subProfileId)
{
    DMAccountInfo dmAccountInfo;
#ifdef CAR_DEVICE_ENABLE
    FillDMAccountInfoFromSubProfile(userId, subProfileId, dmAccountInfo);
#endif
    return dmAccountInfo;
}
} // namespace DistributedHardware
} // namespace OHOS