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
    GroupInfo groupInfo;
    groupInfo.groupName = "group1";
    groupInfo.groupId = "123456";
    groupInfo.groupOwner = "sdsd";
    groupInfo.groupType = 1;
    groupInfo.groupVisibility = 0;
    groupInfo.userId = "82898";

    GroupInfo groupInfo2;
    groupInfo2.groupName = "group2";
    groupInfo2.groupId = "123456";
    groupInfo2.groupOwner = "sdsd";
    groupInfo2.groupType = 1;
    groupInfo2.groupVisibility = 0;
    groupInfo2.userId = "82898";

    GroupInfo groupInfo3;
    groupInfo3.groupName = "group3";
    groupInfo3.groupId = "123456";
    groupInfo3.groupOwner = "sdsd";
    groupInfo3.groupType = 1;
    groupInfo3.groupVisibility = 0;
    groupInfo3.userId = "82898";

    groupList.push_back(groupInfo);
    groupList.push_back(groupInfo2);
    groupList.push_back(groupInfo3);
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
} // namespace DistributedHardware
} // namespace OHOS