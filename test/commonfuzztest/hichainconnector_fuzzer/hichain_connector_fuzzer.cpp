/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <cstdlib>
#include <random>
#include <utility>
#include <vector>

#include "device_manager_service_listener.h"
#include "dm_auth_manager.h"
#include "hichain_connector.h"

#include "hichain_connector_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {

class HiChainConnectorCallbackTest : public IHiChainConnectorCallback {
public:
    HiChainConnectorCallbackTest() {}
    virtual ~HiChainConnectorCallbackTest() {}
    void OnGroupCreated(int64_t requestId, const std::string &groupId) override
    {
        (void)requestId;
        (void)groupId;
    }
    void OnMemberJoin(int64_t requestId, int32_t status) override
    {
        (void)requestId;
        (void)status;
    }
    std::string GetConnectAddr(std::string deviceId) override
    {
        (void)deviceId;
        return "";
    }
    int32_t GetPinCode(int32_t &code) override
    {
        (void)code;
        return DM_OK;
    }
};

void AddGroupInfo(std::vector<GroupInfo> &groupList)
{
    GroupInfo groupInfo1;
    groupInfo1.groupId = "234";
    groupInfo1.groupType = GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
    groupList.push_back(groupInfo1);
    GroupInfo groupInfo2;
    groupInfo2.groupId = "1485";
    groupInfo2.groupOwner = DM_PKG_NAME;
    groupList.push_back(groupInfo2);
}

void AddAclInfo(std::vector<std::pair<int32_t, std::string>> &delACLInfoVec)
{
    int32_t key = 12;
    std::string value = "acl_info1";
    delACLInfoVec.push_back(std::make_pair(key, value));
    key = 23;
    value = "acl_info2";
    delACLInfoVec.push_back(std::make_pair(key, value));
    key = 25;
    value = "acl_info3";
    delACLInfoVec.push_back(std::make_pair(key, value));
}

void HiChainConnectorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());

    std::string userId(reinterpret_cast<const char*>(data), size);
    int32_t authType = *(reinterpret_cast<const int32_t*>(data));
    std::vector<GroupInfo> groupList;
    std::string queryParams(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string reqDeviceId(reinterpret_cast<const char*>(data), size);
    std::string hostDevice(reinterpret_cast<const char*>(data), size);
    std::vector<std::string> remoteGroupIdList;
    int32_t groupType = *(reinterpret_cast<const int32_t*>(data));
    nlohmann::json jsonDeviceList;
    std::string groupOwner(reinterpret_cast<const char*>(data), size);
    std::string credentialInfo(reinterpret_cast<const char*>(data), size);
    std::string jsonStr(reinterpret_cast<const char*>(data), size);
    std::vector<std::string> udidList;
    std::string pkgNameStr(reinterpret_cast<const char*>(data), size);
    int32_t delUserid = *(reinterpret_cast<const int32_t*>(data));

    hichainConnector->IsRedundanceGroup(userId, authType, groupList);
    hichainConnector->GetGroupInfo(queryParams, groupList);
    hichainConnector->GetGroupInfo(delUserid, queryParams, groupList);
    hichainConnector->GetGroupType(deviceId);
    hichainConnector->AddMember(deviceId, queryParams);
    hichainConnector->GetConnectPara(deviceId, reqDeviceId);
    hichainConnector->IsDevicesInP2PGroup(hostDevice, reqDeviceId);
    hichainConnector->SyncGroups(deviceId, remoteGroupIdList);
    hichainConnector->DeleteTimeOutGroup(deviceId.data());
    hichainConnector->getRegisterInfo(queryParams, jsonStr);
    hichainConnector->GetGroupId(userId, groupType, queryParams);
    hichainConnector->addMultiMembers(groupType, userId, jsonDeviceList);
    hichainConnector->GetGroupIdExt(userId, groupType, queryParams, groupOwner);
    hichainConnector->ParseRemoteCredentialExt(credentialInfo, queryParams, groupOwner);
    hichainConnector->addMultiMembersExt(credentialInfo);
    hichainConnector->GetTrustedDevices(deviceId);
    hichainConnector->GetTrustedDevicesUdid(jsonStr.data(), udidList);
    hichainConnector->DeleteAllGroup(delUserid);
    hichainConnector->GetRelatedGroupsCommon(deviceId, pkgNameStr.data(), groupList);
    hichainConnector->UnRegisterHiChainCallback();
}

void HiChainConnectorSecondFuzzTest(const uint8_t* data, size_t size)
{
    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());

    std::vector<GroupInfo> groupList;
    nlohmann::json jsonDeviceList;
    GroupInfo groupInfo;
    std::vector<std::string> syncGroupList;
    hichainConnector->GetSyncGroupList(groupList, syncGroupList);
    hichainConnector->IsGroupInfoInvalid(groupInfo);
    hichainConnector->UnRegisterHiChainGroupCallback();
    hichainConnector->GetJsonStr(jsonDeviceList, "key");
}

void HiChainConnectorThirdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());

    int64_t requestId = *(reinterpret_cast<const int64_t*>(data));
    std::string groupName(reinterpret_cast<const char*>(data), size);
    GroupInfo groupInfo;
    std::string userId(reinterpret_cast<const char*>(data), size);
    int32_t usersId = *(reinterpret_cast<const int32_t*>(data));
    int32_t authType = *(reinterpret_cast<const int32_t*>(data));
    std::vector<GroupInfo> groupList;
    std::string queryParams(reinterpret_cast<const char*>(data), size);
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string groupId(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string reqDeviceId(reinterpret_cast<const char*>(data), size);
    nlohmann::json jsonOutObj;
    std::shared_ptr<IDmGroupResCallback> callback;
    std::string jsonStr(reinterpret_cast<const char*>(data), size);
    nlohmann::json jsonDeviceList;
    int32_t groupType = *(reinterpret_cast<const int32_t*>(data));
    std::string udid(reinterpret_cast<const char*>(data), size);
    int32_t switchUserId = *(reinterpret_cast<const int32_t*>(data));
    std::string reqParams(reinterpret_cast<const char*>(data), size);
    int operationCode = GroupOperationCode::MEMBER_JOIN;
    hichainConnector->deviceGroupManager_ = nullptr;
    hichainConnector->AddMember(deviceId, queryParams);
    hichainConnector->CreateGroup(requestId, groupName);
    hichainConnector->CreateGroup(requestId, authType, userId, jsonOutObj);
    hichainConnector->getRegisterInfo(queryParams, jsonStr);
    hichainConnector->addMultiMembers(groupType, userId, jsonDeviceList);
    hichainConnector->deleteMultiMembers(groupType, userId, jsonDeviceList);
    hichainConnector->GetGroupInfoCommon(usersId, queryParams, pkgName.c_str(), groupList);
    hichainConnector->hiChainResCallback_ = nullptr;
    hichainConnector->GetConnectPara(deviceId, reqDeviceId);
    hichainConnector->onRequest(requestId, operationCode, reqParams.c_str());
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    if (hichainConnector->deviceGroupManager_ == nullptr) {
        hichainConnector->deviceGroupManager_ = GetGmInstance();
    }
    hichainConnector->CreateGroup(requestId, groupName);
    hichainConnector->CreateGroup(requestId, authType, userId, jsonOutObj);
    hichainConnector->IsGroupCreated(groupName ,groupInfo);
    hichainConnector->GetGroupInfoExt(usersId, queryParams, groupList);
    hichainConnector->GetGroupInfoCommon(usersId, queryParams, pkgName.c_str(), groupList);
    hichainConnector->RegisterHiChainGroupCallback(callback);
    hichainConnector->GetJsonInt(jsonDeviceList, "key");
    hichainConnector->deleteMultiMembers(groupType, userId, jsonDeviceList);
    hichainConnector->DeleteAllGroupByUdid(udid);
    hichainConnector->DeleteP2PGroup(switchUserId);
}

void HiChainConnectorForthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());

    int64_t requestId = *(reinterpret_cast<const int64_t*>(data));
    std::string groupName(reinterpret_cast<const char*>(data), size);
    std::string groupId(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string returnData(reinterpret_cast<const char*>(data), size);
    std::string userId(reinterpret_cast<const char*>(data), size);
    int32_t authType = *(reinterpret_cast<const int32_t*>(data));
    int operationCode = GroupOperationCode::MEMBER_JOIN;
    int errCode = 102;
    std::vector<std::string> syncGroupList;
    std::vector<std::pair<int32_t, std::string>> delACLInfoVec;
    std::vector<int32_t> userIdVec;
    AddAclInfo(delACLInfoVec);
    hichainConnector->DeleteGroupByACL(delACLInfoVec, userIdVec);
    userIdVec.push_back(key);
    key = 32;
    userIdVec.push_back(key);
    std::vector<GroupInfo> groupList;
    AddGroupInfo(groupList);
    hichainConnector->DeleteGroup(groupId);
    hichainConnector->DeleteGroupExt(groupId);
    hichainConnector->DeleteGroup(authType, groupId);
    hichainConnector->DeleteGroup(requestId, userId, authType);
    hichainConnector->DelMemberFromGroup(groupId, deviceId);
    hichainConnector->DeleteRedundanceGroup(userId);
    hichainConnector->DealRedundanceGroup(userId, authType);
    hichainConnector->DeleteGroupByACL(delACLInfoVec, userIdVec);
    hichainConnector->IsNeedDelete(groupName, userId, delACLInfoVec);
    hichainConnector->onFinish(requestId, operationCode, returnData.c_str());
    hichainConnector->onError(requestId, operationCode, errCode, returnData.c_str());
    hichainConnector->onRequest(requestId, operationCode, returnData.c_str());
    operationCode = GroupOperationCode::GROUP_CREATE;
    hichainConnector->onFinish(requestId, operationCode, returnData.c_str());
    hichainConnector->onError(requestId, operationCode, errCode, returnData.c_str());
    operationCode == GroupOperationCode::MEMBER_DELETE
    hichainConnector->onFinish(requestId, operationCode, returnData.c_str());
    hichainConnector->onError(requestId, operationCode, errCode, returnData.c_str());
    operationCode == GroupOperationCode::GROUP_DISBAND;
    hichainConnector->onFinish(requestId, operationCode, returnData.c_str());
    hichainConnector->onError(requestId, operationCode, errCode, returnData.c_str());
    hichainConnector->GenRequestId();
    hichainConnector->GetRelatedGroups(deviceId, groupList);
    hichainConnector->GetRelatedGroupsExt(deviceId, groupList);
    hichainConnector->GetSyncGroupList(groupList, syncGroupList);
    hichainConnector->GetGroupId(userId, authType, userId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::HiChainConnectorFuzzTest(data, size);
    OHOS::DistributedHardware::HiChainConnectorSecondFuzzTest(data, size);
    OHOS::DistributedHardware::HiChainConnectorThirdFuzzTest(data, size);
    OHOS::DistributedHardware::HiChainConnectorForthFuzzTest(data, size);
    return 0;
}