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
namespace {
constexpr int32_t MOCK_CONNECTOR_NOT_READY = -1;
}

bool HiChainConnector::GetGroupInfo(const int32_t userId, const std::string &queryParams,
    std::vector<GroupInfo> &groupList)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return false;
    }
    return DmHiChainConnector::dmHiChainConnector->GetGroupInfo(userId, queryParams, groupList);
}

bool HiChainConnector::IsDevicesInP2PGroup(const std::string &hostDevice, const std::string &peerDevice)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return false;
    }
    return DmHiChainConnector::dmHiChainConnector->IsDevicesInP2PGroup(hostDevice, peerDevice);
}

int32_t HiChainConnector::GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->GetRelatedGroups(deviceId, groupList);
}

int32_t HiChainConnector::GetRelatedGroups(int32_t userId, const std::string &deviceId,
    std::vector<GroupInfo> &groupList)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->GetRelatedGroups(userId, deviceId, groupList);
}

int32_t HiChainConnector::DeleteGroupByACL(std::vector<std::pair<int32_t, std::string>> &delACLInfoVec,
    std::vector<int32_t> &userIdVec)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->DeleteGroupByACL(delACLInfoVec, userIdVec);
}

int32_t HiChainConnector::DeleteTimeOutGroup(const std::string &peerUdid, int32_t userId)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->DeleteTimeOutGroup(peerUdid, userId);
}

int32_t HiChainConnector::GetRelatedGroupsExt(const std::string &deviceId, std::vector<GroupInfo> &groupList)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->GetRelatedGroupsExt(deviceId, groupList);
}

int32_t HiChainConnector::DeleteGroupExt(int32_t userId, const std::string &groupId)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->DeleteGroupExt(userId, groupId);
}

int32_t HiChainConnector::getRegisterInfo(const std::string &queryParams, std::string &returnJsonStr)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->GetRegisterInfo(queryParams, returnJsonStr);
}

int32_t HiChainConnector::CreateGroup(int64_t requestId, int32_t authType, const std::string &userId,
    JsonObject &jsonOutObj)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->CreateGroup(requestId, authType, userId, jsonOutObj);
}

int32_t HiChainConnector::addMultiMembers(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->AddMultiMembers(groupType, userId, jsonDeviceList);
}

int32_t HiChainConnector::addMultiMembersExt(const std::string &credentialInfo)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->AddMultiMembersExt(credentialInfo);
}

int32_t HiChainConnector::deleteMultiMembers(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->DeleteMultiMembers(groupType, userId, jsonDeviceList);
}

int32_t HiChainConnector::DeleteGroup(int64_t requestId, const std::string &userId, const int32_t authType)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->DeleteGroupByReqId(requestId, userId, authType);
}

int32_t HiChainConnector::RegisterHiChainGroupCallback(const std::shared_ptr<IDmGroupResCallback> &callback)
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->RegisterHiChainGroupCallback(callback);
}

int32_t HiChainConnector::UnRegisterHiChainGroupCallback()
{
    if (DmHiChainConnector::dmHiChainConnector == nullptr) {
        return MOCK_CONNECTOR_NOT_READY;
    }
    return DmHiChainConnector::dmHiChainConnector->UnRegisterHiChainGroupCallback();
}
} // namespace DistributedHardware
} // namespace OHOS
