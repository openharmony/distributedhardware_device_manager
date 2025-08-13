/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <fuzzer/FuzzedDataProvider.h>

#include "device_manager_service_listener.h"
#include "dm_anonymous.h"
#include "dm_auth_manager.h"
#include "dm_constants.h"
#include "hichain_connector.h"

#include "hichain_connector_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* FIELD_OPERATION_CODE = "operationCode";
constexpr const char* FIELD_META_NODE_TYPE = "metaNodeType";
constexpr const char* FIELD_TYPE = "TType";

class HiChainConnectorCallbackTest : public IHiChainConnectorCallback {
public:
    HiChainConnectorCallbackTest() {}
    virtual ~HiChainConnectorCallbackTest() {}
    void OnGroupCreated(int64_t requestId, const std::string &groupId) override
    {
        (void)requestId;
        (void)groupId;
    }
    void OnMemberJoin(int64_t requestId, int32_t status, int32_t operationCode) override
    {
        (void)requestId;
        (void)status;
    }
    std::string GetConnectAddr(std::string deviceId) override
    {
        (void)deviceId;
        return "";
    }
    int32_t GetPinCode(std::string &code) override
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

void GenerateJsonObject(JsonObject &jsonObject, FuzzedDataProvider &fdp)
{
    jsonObject[FIELD_GROUP_NAME] = fdp.ConsumeRandomLengthString();
    jsonObject[FIELD_GROUP_ID] = fdp.ConsumeRandomLengthString();
    jsonObject[FIELD_GROUP_OWNER] = fdp.ConsumeRandomLengthString();
    jsonObject[FIELD_GROUP_TYPE] = fdp.ConsumeIntegral<int32_t>();
    jsonObject[FIELD_GROUP_VISIBILITY] = fdp.ConsumeIntegral<int32_t>();
    jsonObject[FIELD_USER_ID] = fdp.ConsumeRandomLengthString();
}

void AddAclInfo(std::vector<std::pair<int32_t, std::string>> &delACLInfoVec, std::vector<int32_t> &userIdVec)
{
    int32_t key = 12;
    std::string value = "acl_info1";
    delACLInfoVec.push_back(std::make_pair(key, value));
    userIdVec.push_back(key);
    int32_t key1 = 23;
    value = "acl_info2";
    delACLInfoVec.push_back(std::make_pair(key1, value));
    userIdVec.push_back(key);
    int32_t key2 = 25;
    value = "acl_info3";
    delACLInfoVec.push_back(std::make_pair(key2, value));
    userIdVec.push_back(key);
}

void HiChainConnectorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < (sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t)))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());

    FuzzedDataProvider fdp(data, size);
    std::string userId(reinterpret_cast<const char*>(data), size);
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    std::vector<GroupInfo> groupList;
    std::string queryParams(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string reqDeviceId(reinterpret_cast<const char*>(data), size);
    std::string hostDevice(reinterpret_cast<const char*>(data), size);
    std::vector<std::string> remoteGroupIdList;
    int32_t groupType = fdp.ConsumeIntegral<int32_t>();
    JsonObject jsonDeviceList;
    std::string groupOwner(reinterpret_cast<const char*>(data), size);
    std::string credentialInfo(reinterpret_cast<const char*>(data), size);
    std::string jsonStr(reinterpret_cast<const char*>(data), size);
    std::vector<std::string> udidList;
    std::string pkgNameStr(reinterpret_cast<const char*>(data), size);
    int32_t delUserid = fdp.ConsumeIntegral<int32_t>();

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
    JsonObject jsonDeviceList;
    GroupInfo groupInfo;
    std::vector<std::string> syncGroupList;
    hichainConnector->GetSyncGroupList(groupList, syncGroupList);
    hichainConnector->IsGroupInfoInvalid(groupInfo);
    hichainConnector->UnRegisterHiChainGroupCallback();
    hichainConnector->GetJsonStr(jsonDeviceList, "key");
}

void HiChainConnectorThirdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < (sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t)))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    std::string groupName = "groupName";
    GroupInfo groupInfo;
    std::string userId(reinterpret_cast<const char*>(data), size);
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    std::vector<GroupInfo> groupList;
    std::string queryParams(reinterpret_cast<const char*>(data), size);
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string reqDeviceId(reinterpret_cast<const char*>(data), size);
    JsonObject jsonOutObj;
    std::shared_ptr<IDmGroupResCallback> callback;
    std::string jsonStr(reinterpret_cast<const char*>(data), size);
    int32_t groupType = fdp.ConsumeIntegral<int32_t>();
    std::string reqParams(reinterpret_cast<const char*>(data), size);
    std::string credentialInfo(reinterpret_cast<const char*>(data), size);
    hichainConnector->deviceGroupManager_ = nullptr;
    hichainConnector->AddMember(deviceId, queryParams);
    hichainConnector->getRegisterInfo(queryParams, jsonStr);
    hichainConnector->addMultiMembers(groupType, userId, jsonOutObj);
    hichainConnector->addMultiMembersExt(credentialInfo);
    hichainConnector->deleteMultiMembers(groupType, userId, jsonOutObj);
    hichainConnector->GetGroupInfoCommon(authType, queryParams, pkgName.c_str(), groupList);
    hichainConnector->hiChainConnectorCallback_ = nullptr;
    hichainConnector->GetConnectPara(deviceId, reqDeviceId);
    char *ret = hichainConnector->onRequest(requestId, GroupOperationCode::MEMBER_JOIN, reqParams.c_str());
    if (ret != nullptr) {
        free(ret);
        ret = nullptr;
    }
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    if (hichainConnector->deviceGroupManager_ == nullptr) {
        hichainConnector->deviceGroupManager_ = GetGmInstance();
    }
    hichainConnector->IsGroupCreated(groupName, groupInfo);
    hichainConnector->GetGroupInfoExt(authType, queryParams, groupList);
    hichainConnector->GetGroupInfoCommon(authType, queryParams, pkgName.c_str(), groupList);
    hichainConnector->RegisterHiChainGroupCallback(callback);
    hichainConnector->GetJsonInt(jsonOutObj, "key");
    hichainConnector->deleteMultiMembers(groupType, userId, jsonOutObj);
    hichainConnector->DeleteAllGroupByUdid(reqParams);
}

void HiChainConnectorForthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t) + sizeof(int32_t))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    FuzzedDataProvider fdp(data, size);
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    std::string groupName(reinterpret_cast<const char*>(data), size);
    std::string groupId = "groupId_forth";
    std::string deviceId = "deviceId_forth";
    std::string returnData(reinterpret_cast<const char*>(data), size);
    std::string userId = "userId_forth";
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    int errCode = 102;
    std::vector<std::string> syncGroupList;
    std::vector<std::pair<int32_t, std::string>> delACLInfoVec;
    std::vector<int32_t> userIdVec;
    hichainConnector->DeleteGroupByACL(delACLInfoVec, userIdVec);
    std::vector<GroupInfo> groupList;
    AddGroupInfo(groupList);
    AddAclInfo(delACLInfoVec, userIdVec);
    hichainConnector->DeleteGroup(groupId);
    hichainConnector->DeleteGroupExt(groupId);
    hichainConnector->DeleteGroup(authType, groupId);
    hichainConnector->DeleteGroup(requestId, userId, authType);
    hichainConnector->DelMemberFromGroup(groupId, deviceId);
    hichainConnector->DeleteRedundanceGroup(userId);
    hichainConnector->DealRedundanceGroup(userId, authType);
    hichainConnector->DeleteGroupByACL(delACLInfoVec, userIdVec);
    hichainConnector->IsNeedDelete(groupName, authType, delACLInfoVec);
    hichainConnector->onFinish(requestId, GroupOperationCode::MEMBER_JOIN, returnData.c_str());
    hichainConnector->onError(requestId, GroupOperationCode::MEMBER_JOIN, errCode, returnData.c_str());
    char *ret = hichainConnector->onRequest(requestId, GroupOperationCode::MEMBER_JOIN, returnData.c_str());
    if (ret != nullptr) {
        free(ret);
        ret = nullptr;
    }
    hichainConnector->onFinish(requestId, GroupOperationCode::GROUP_CREATE, returnData.c_str());
    hichainConnector->onError(requestId, GroupOperationCode::GROUP_CREATE, errCode, returnData.c_str());
    hichainConnector->onFinish(requestId, GroupOperationCode::MEMBER_DELETE, returnData.c_str());
    hichainConnector->onError(requestId, GroupOperationCode::MEMBER_DELETE, errCode, returnData.c_str());
    hichainConnector->onFinish(requestId, GroupOperationCode::GROUP_DISBAND, returnData.c_str());
    hichainConnector->onError(requestId, GroupOperationCode::GROUP_DISBAND, errCode, returnData.c_str());
    hichainConnector->GenRequestId();
    hichainConnector->GetRelatedGroups(deviceId, groupList);
    hichainConnector->GetRelatedGroupsExt(deviceId, groupList);
    hichainConnector->GetSyncGroupList(groupList, syncGroupList);
    hichainConnector->GetGroupId(userId, authType, userId);
}

void HiChainConnectorFifthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    FuzzedDataProvider fdp(data, size);
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    std::string groupName = "groupName_fifth";
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    std::string params = "params";
    int32_t osAccountUserId = fdp.ConsumeIntegral<int32_t>();
    JsonObject jsonDeviceList;
    std::vector<std::pair<int32_t, std::string>> delACLInfoVec;
    std::vector<int32_t> userIdVec;
    std::vector<std::pair<int32_t, std::string>> delAclInfoVec1;
    std::string jsonStr = R"({"content": {"deviceid": "123"}}, authId: "123456"))";
    std::vector<std::string> udidList;
    int32_t key = 12;
    std::string value = "acl_info1";
    std::string credentialInfo = R"({"content": {"deviceid": "123"}}, authId: "123456"))";
    std::string groupOwner(reinterpret_cast<const char*>(data), size);
    delAclInfoVec1.push_back(std::make_pair(key, value));
    JsonObject jsonObj;
    jsonObj[AUTH_TYPE] = 1;
    jsonObj[FIELD_USER_ID] = "123456";
    jsonObj[FIELD_CREDENTIAL_TYPE] = 1;
    jsonObj[FIELD_OPERATION_CODE] = 1;
    jsonObj[FIELD_TYPE] = "filed_type";
    jsonObj[FIELD_DEVICE_LIST] = "device_list";
    hichainConnector->deviceGroupManager_ = nullptr;
    hichainConnector->CreateGroup(requestId, groupName);
    hichainConnector->CreateGroup(requestId, authType, groupName, jsonDeviceList);
    if (hichainConnector->deviceGroupManager_ == nullptr) {
        hichainConnector->deviceGroupManager_ = GetGmInstance();
    }
    hichainConnector->CreateGroup(requestId, groupName);
    hichainConnector->CreateGroup(requestId, authType, groupName, jsonDeviceList);
    hichainConnector->ParseRemoteCredential(authType, groupName, jsonDeviceList, params, osAccountUserId);
    hichainConnector->ParseRemoteCredential(authType, "", jsonDeviceList, params, osAccountUserId);
    hichainConnector->IsNeedDelete(groupName, authType, delACLInfoVec);
    hichainConnector->DeleteGroupByACL(delAclInfoVec1, userIdVec);
    hichainConnector->GetTrustedDevicesUdid(jsonStr.data(), udidList);
    jsonStr = R"({"content": {"deviceid": "123"}}, authId: "123456")";
    hichainConnector->GetTrustedDevicesUdid(jsonStr.data(), udidList);
    jsonStr = R"({"content": {"deviceid": "123"}}, localId: "123456")";
    hichainConnector->GetTrustedDevicesUdid(jsonStr.data(), udidList);
}

void HiChainConnectorSixthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    std::string groupOwner(reinterpret_cast<const char*>(data), size);
    JsonObject jsonObj;
    std::string deviceId = "deviceId";
    std::string key = "localDeviceId";
    jsonObj["deviceId"] = 1;
    hichainConnector->GetJsonInt(jsonObj, key);
    hichainConnector->GetJsonInt(jsonObj, key);
    jsonObj[key] = 1;
    jsonObj["deviceName"] = "devieName1";
    hichainConnector->GetJsonInt(jsonObj, "devieName");
    hichainConnector->AddMember(deviceId, jsonObj.Dump());
    jsonObj[TAG_DEVICE_ID] = "deviceId_001";
    jsonObj[PIN_CODE_KEY] = 1;
    jsonObj[TAG_GROUP_ID] = "groupId";
    jsonObj[TAG_REQUEST_ID] = 1;
    jsonObj[TAG_GROUP_NAME] = "groupName";
    hichainConnector->AddMember(deviceId, jsonObj.Dump());

    JsonObject jsonObjCre;
    std::string params;
    jsonObjCre[AUTH_TYPE] = 1;
    jsonObjCre["userId"] = "user_001";
    jsonObjCre[FIELD_CREDENTIAL_TYPE] = 1;
    jsonObjCre[FIELD_OPERATION_CODE] = 1;
    jsonObjCre[FIELD_META_NODE_TYPE] = "metaNode_002";
    jsonObjCre[FIELD_DEVICE_LIST] = "deviceList";
    std::string credentialInfo = jsonObjCre.Dump();
    hichainConnector->ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    int32_t groupType = *(reinterpret_cast<const int32_t*>(data));
    JsonObject jsonDeviceList;
    int32_t osAccountUserId = 0;
    std::string userId = "user_002";
    std::vector<std::string> fieldDeviceList = {"deviceList"};
    jsonDeviceList[FIELD_DEVICE_LIST] = fieldDeviceList;
    hichainConnector->ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    FuzzedDataProvider fdp(data, size);
    int32_t userIdInt = fdp.ConsumeIntegral<int32_t>();
    std::string groupId = fdp.ConsumeRandomLengthString();
    hichainConnector->DeleteGroupExt(userIdInt, groupId);
    GroupInfo groupInfo;
    GenerateJsonObject(jsonObj, fdp);
    FromJson(jsonObj, groupInfo);
}

void HiChainConnectorSevenhFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    std::vector<std::pair<int32_t, std::string>> delACLInfoVec;
    delACLInfoVec.push_back(std::make_pair(1, "aclinfo_001"));
    std::vector<int32_t> userIdVec;
    hichainConnector->DeleteGroupByACL(delACLInfoVec, userIdVec);
    std::string groupName = "";
    FuzzedDataProvider fdp(data, size);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    hichainConnector->IsNeedDelete(groupName, userId, delACLInfoVec);
    userId = 1;
    groupName = "aclinfo_001";
    hichainConnector->IsNeedDelete(groupName, userId, delACLInfoVec);
    std::vector<std::string> udidList;
    JsonObject jsonObject;
    std::string jsonStr = "dkocosdpa";
    hichainConnector->GetTrustedDevicesUdid(jsonStr.data(), udidList);
    jsonObject["authId"] = 1;
    jsonStr = jsonObject.Dump();
    hichainConnector->GetTrustedDevicesUdid(jsonStr.data(), udidList);

    jsonObject["authId"] = "authInfoId";
    jsonStr = jsonObject.Dump();
    hichainConnector->GetTrustedDevicesUdid(jsonStr.data(), udidList);
    hichainConnector->deviceGroupManager_ = nullptr;
    int32_t groupType = fdp.ConsumeIntegral<int32_t>();
    std::string usersId(reinterpret_cast<const char*>(data), size);
    JsonObject jsonDeviceList;
    hichainConnector->deleteMultiMembers(groupType, usersId, jsonDeviceList);
}

void GetGroupInfoExtFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t maxStringLength = 64;
    std::string queryParams = fdp.ConsumeRandomLengthString(maxStringLength);
    std::vector<GroupInfo> groupList;

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->GetGroupInfoExt(userId, queryParams, groupList);
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
    OHOS::DistributedHardware::HiChainConnectorFifthFuzzTest(data, size);
    OHOS::DistributedHardware::HiChainConnectorSixthFuzzTest(data, size);
    OHOS::DistributedHardware::HiChainConnectorSevenhFuzzTest(data, size);
    OHOS::DistributedHardware::GetGroupInfoExtFuzzTest(data, size);
    return 0;
}