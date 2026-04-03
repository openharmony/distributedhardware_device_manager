/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "multiple_user_connector_3rd.h"

#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"

#include "account_info.h"
#include "ipc_skeleton.h"
#include "ohos_account_kits.h"
#include "os_account_manager.h"

using namespace OHOS::AccountSA;

namespace OHOS {
namespace DistributedHardware {

void MultipleUserConnector3rd::GetCallingTokenId(uint32_t &tokenId)
{
    tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
}

const int32_t DEFAULT_OS_ACCOUNT_ID = 0;
void MultipleUserConnector3rd::GetCallerUserId(int32_t &userId)
{
    int32_t uid = OHOS::IPCSkeleton::GetCallingUid();
    ErrCode ret = OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (ret != 0) {
        LOGE("GetOsAccountLocalIdFromUid error ret: %{public}d", ret);
    }
    return;
}

int32_t MultipleUserConnector3rd::GetCurrentAccountUserID(void)
{
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("GetCurrentAccountUserID error ret: %{public}d", ret);
        return -1;
    }
    return ids[0];
}

std::string MultipleUserConnector3rd::GetOhosAccountIdByUserId(int32_t userId)
{
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOsAccountDistributedInfo(userId, accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
}

int32_t MultipleUserConnector3rd::GetFirstForegroundUserId(void)
{
    std::vector<int32_t> userVec;
    int32_t ret = GetForegroundUserIds(userVec);
    if (ret != DM_OK || userVec.size() == 0) {
        LOGE("get userid error ret: %{public}d.", ret);
        return -1;
    }
    return userVec[0];
}

int32_t MultipleUserConnector3rd::GetForegroundUserIds(
    std::vector<int32_t> &userVec)
{
    userVec.clear();
    std::vector<AccountSA::ForegroundOsAccount> accounts;
    ErrCode ret = OsAccountManager::GetForegroundOsAccounts(accounts);
    if (ret != 0 || accounts.empty()) {
        LOGE("error ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    for (auto &account : accounts) {
        userVec.push_back(account.localId);
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS