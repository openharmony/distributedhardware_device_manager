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

#include "hichain_connector_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
bool HiChainConnector::GetGroupInfo(const int32_t userId, const std::string &queryParams,
    std::vector<GroupInfo> &groupList)
{
    return DmHiChainConnector::dmHiChainConnector->GetGroupInfo(userId, queryParams, groupList);
}

bool HiChainConnector::IsDevicesInP2PGroup(const std::string &hostDevice, const std::string &peerDevice)
{
    return DmHiChainConnector::dmHiChainConnector->IsDevicesInP2PGroup(hostDevice, peerDevice);
}

int32_t HiChainConnector::GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList)
{
    return DmHiChainConnector::dmHiChainConnector->GetRelatedGroups(deviceId, groupList);
}

int32_t HiChainConnector::GetRelatedGroups(int32_t userId, const std::string &deviceId,
    std::vector<GroupInfo> &groupList)
{
    return DmHiChainConnector::dmHiChainConnector->GetRelatedGroups(userId, deviceId, groupList);
}

int32_t HiChainConnector::DeleteGroupByACL(std::vector<std::pair<int32_t, std::string>> &delACLInfoVec,
    std::vector<int32_t> &userIdVec)
{
    return DmHiChainConnector::dmHiChainConnector->DeleteGroupByACL(delACLInfoVec, userIdVec);
}

int32_t HiChainConnector::DeleteTimeOutGroup(const std::string &peerUdid, int32_t userId)
{
    return DmHiChainConnector::dmHiChainConnector->DeleteTimeOutGroup(peerUdid, userId);
}

int32_t HiChainConnector::GetRelatedGroupsExt(const std::string &deviceId, std::vector<GroupInfo> &groupList)
{
    return DmHiChainConnector::dmHiChainConnector->GetRelatedGroupsExt(deviceId, groupList);
}

int32_t HiChainConnector::DeleteGroupExt(int32_t userId, const std::string &groupId)
{
    return DmHiChainConnector::dmHiChainConnector->DeleteGroupExt(userId, groupId);
}
} // namespace DistributedHardware
} // namespace OHOS