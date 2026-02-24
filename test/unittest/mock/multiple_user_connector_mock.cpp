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

#include "multiple_user_connector_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
int32_t MultipleUserConnector::GetCurrentAccountUserID(void)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetCurrentAccountUserID();
}

int32_t MultipleUserConnector::GetFirstForegroundUserId(void)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetFirstForegroundUserId();
}

DMAccountInfo MultipleUserConnector::GetAccountInfoByUserId(int32_t userId)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetAccountInfoByUserId(userId);
}

int32_t MultipleUserConnector::GetBackgroundUserIds(std::vector<int32_t> &userIdVec)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetBackgroundUserIds(userIdVec);
}

int32_t MultipleUserConnector::GetForegroundUserIds(std::vector<int32_t> &userVec)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetForegroundUserIds(userVec);
}

std::string MultipleUserConnector::GetOhosAccountId(void)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetOhosAccountId();
}

int32_t MultipleUserConnector::GetUserIdByDisplayId(int32_t displayId)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetUserIdByDisplayId(displayId);
}

void MultipleUserConnector::GetTokenIdAndForegroundUserId(uint32_t &tokenId, int32_t &userId)
{
    DmMultipleUserConnector::dmMultipleUserConnector->GetTokenIdAndForegroundUserId(tokenId, userId);
}

std::string MultipleUserConnector::GetOhosAccountIdByUserId(int32_t userId)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetOhosAccountIdByUserId(userId);
}

std::string MultipleUserConnector::GetAccountNickName(int32_t userId)
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetAccountNickName(userId);
}

void MultipleUserConnector::GetCallerUserId(int32_t &userId)
{
    DmMultipleUserConnector::dmMultipleUserConnector->GetCallerUserId(userId);
}

void MultipleUserConnector::SetAccountInfo(int32_t userId, DMAccountInfo dmAccountInfo)
{
    DmMultipleUserConnector::dmMultipleUserConnector->SetAccountInfo(userId, dmAccountInfo);
}

DMAccountInfo MultipleUserConnector::GetCurrentDMAccountInfo()
{
    return DmMultipleUserConnector::dmMultipleUserConnector->GetCurrentDMAccountInfo();
}
} // namespace DistributedHardware
} // namespace OHOS