/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "dm_log.h"

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "account_info.h"
#include "ohos_account_kits.h"
#ifdef OS_ACCOUNT_PART_EXISTS
#include "os_account_manager.h"
using namespace OHOS::AccountSA;
#endif // OS_ACCOUNT_PART_EXISTS
#endif

namespace OHOS {
namespace DistributedHardware {
int32_t MultipleUserConnector::oldUserId_ = -1;
std::string MultipleUserConnector::accountId_ = "";
std::string MultipleUserConnector::accountName_ = "";
std::mutex MultipleUserConnector::lock_;
#ifndef OS_ACCOUNT_PART_EXISTS
const int32_t DEFAULT_OS_ACCOUNT_ID = 0; // 0 is the default id when there is no os_account part
#endif // OS_ACCOUNT_PART_EXISTS

int32_t MultipleUserConnector::GetCurrentAccountUserID(void)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return 0;
#elif OS_ACCOUNT_PART_EXISTS
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("GetCurrentAccountUserID error ret: %{public}d", ret);
        return -1;
    }
    return ids[0];
#else // OS_ACCOUNT_PART_EXISTS
    return DEFAULT_OS_ACCOUNT_ID;
#endif
}

std::string MultipleUserConnector::GetOhosAccountId(void)
{
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return "";
#elif OS_ACCOUNT_PART_EXISTS
    OhosAccountInfo accountInfo;
    ErrCode ret = OhosAccountKits::GetInstance().GetOhosAccountInfo(accountInfo);
    if (ret != 0 || accountInfo.uid_ == "") {
        LOGE("GetOhosAccountId error ret: %{public}d", ret);
        return "";
    }
    return accountInfo.uid_;
#else
    return "";
#endif
}

std::string MultipleUserConnector::GetOhosAccountName(void)
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

void MultipleUserConnector::SetSwitchOldUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(lock_);
    oldUserId_ = userId;
}

int32_t MultipleUserConnector::GetSwitchOldUserId(void)
{
    std::lock_guard<std::mutex> lock(lock_);
    return oldUserId_;
}

void MultipleUserConnector::SetSwitchOldAccountId(std::string accountId)
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
} // namespace DistributedHardware
} // namespace OHOS
