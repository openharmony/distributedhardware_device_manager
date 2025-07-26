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

#include "UTTest_hichain_connector.h"

#include <cstdlib>
#include <ctime>
#include <functional>
#include <securec.h>

#include "device_manager_service_listener.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_credential_manager.h"
#include "dm_log.h"
#include "dm_random.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
void HichainConnectorTest::SetUp()
{
}
void HichainConnectorTest::TearDown()
{
}
void HichainConnectorTest::SetUpTestCase()
{
}
void HichainConnectorTest::TearDownTestCase()
{
}

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
    std::string GetConnectAddr(std::string deviceId)
    {
        return "";
    }
    int32_t GetPinCode(std::string &code)
    {
        code = "123456";
        return DM_OK;
    }
};

namespace {
/**
 * @tc.name: CreateGroup_001
 * @tc.desc: Set the deviceGroupManager_ pointer to CreateGroup to NULlptr and return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, CreateGroup_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 123456;
    std::string groupName = "dfggg";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->deviceGroupManager_ = nullptr;
    int ret = hiChainConnector->CreateGroup(requestId, groupName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: CreateGroup_002
 * @tc.desc: Set CreateGroup to the correct process and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, CreateGroup_002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 123456;
    std::string groupName = "uuiioo";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->CreateGroup(requestId, groupName);
    EXPECT_EQ(ret, ERR_DM_CREATE_GROUP_FAILED);
}

/**
 * @tc.name: CreateGroup_003
 * @tc.desc: Set deviceGroupManager_ is nullptr return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, CreateGroup_003, testing::ext::TestSize.Level1)
{
    int64_t requestId = 159357;
    int32_t authType = 1;
    std::string userId = "userIdTest";
    JsonObject jsonOutObj;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->deviceGroupManager_ = nullptr;
    int32_t ret = hiChainConnector->CreateGroup(requestId, authType, userId, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: IsGroupInfoInvalid_001
 * @tc.desc: GroupType is GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP, group.groupVisibility is not GROUP_VISIBILITY_PUBLIC.
             Group.return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */

HWTEST_F(HichainConnectorTest, IsGroupInfoInvalid_001, testing::ext::TestSize.Level1)
{
    GroupInfo group;
    group.groupName = "dkdkkdkdk";
    group.groupId = 1;
    group.groupOwner = "ohos.distributedhardware.devicemanager";
    group.groupType = 7;
    group.groupVisibility = 1;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    bool ret = hiChainConnector->IsGroupInfoInvalid(group);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsGroupInfoInvalid_002
 * @tc.desc: GroupType is GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP, group.groupVisibility is GROUP_VISIBILITY_PUBLIC,
             Grou. groupOwner is not equal to DM_PKG_NAME. The value is true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, IsGroupInfoInvalid_002, testing::ext::TestSize.Level1)
{
    GroupInfo group;
    group.groupName = "test";
    group.groupId = 1;
    group.groupOwner = "ohos.disware";
    group.groupType = 1;
    group.groupVisibility = -1;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    bool ret = hiChainConnector->IsGroupInfoInvalid(group);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: DelMemberFromGroup_001
 * @tc.desc:set groupId, deviceId null and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DelMemberFromGroup_001, testing::ext::TestSize.Level1)
{
    std::string groupId;
    std::string deviceId;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->DelMemberFromGroup(groupId, deviceId);
    EXPECT_NE(ret, -1);
}

/**
 * @tc.name: DelMemberFromGroup_002
 * @tc.desc: The groupId "34451"; The deviceId = "123"; Can be deleted correctly
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DelMemberFromGroup_002, testing::ext::TestSize.Level1)
{
    std::string groupId = "34451";
    std::string deviceId = "123";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->DelMemberFromGroup(groupId, deviceId);
    EXPECT_NE(ret, -1);
}

/**
 * @tc.name: GenRequestId_001
 * @tc.desc:Call the GenRequestId function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GenRequestId_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->GenRequestId();
    ASSERT_NE(ret, 0);
}

/**
 * @tc.name: from_json_001
 * @tc.desc: Pass in arguments to the from_JSON function and convert it to the correct value
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, from_json_001, testing::ext::TestSize.Level1)
{
    GroupInfo groupInfo;
    groupInfo.groupName = "aaaa";
    groupInfo.groupId = "345678";
    groupInfo.groupOwner = "lllll";
    groupInfo.groupType = 5;
    groupInfo.groupVisibility = 5;
    JsonObject jsonObject;
    jsonObject[FIELD_GROUP_NAME] = groupInfo.groupName;
    jsonObject[FIELD_GROUP_ID] = groupInfo.groupId;
    jsonObject[FIELD_GROUP_OWNER] = groupInfo.groupOwner;
    jsonObject[FIELD_GROUP_TYPE] = groupInfo.groupType;
    jsonObject[FIELD_GROUP_VISIBILITY] = groupInfo.groupVisibility;
    GroupInfo groupInfo1;
    FromJson(jsonObject, groupInfo1);
    EXPECT_EQ(groupInfo1.groupName, "aaaa");
    EXPECT_EQ(groupInfo1.groupId, "345678");
    EXPECT_EQ(groupInfo1.groupOwner, "lllll");
    EXPECT_EQ(groupInfo1.groupType, 5);
    EXPECT_EQ(groupInfo1.groupVisibility, 5);
}

/**
 * @tc.name: from_json_002
 * @tc.desc: Pass in arguments to the from_JSON function and convert it to the correct value
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, from_json_002, testing::ext::TestSize.Level1)
{
    GroupInfo groupInfo;
    groupInfo.userId = "test";
    groupInfo.groupName = "test";
    JsonObject jsonObject;
    jsonObject[FIELD_GROUP_NAME] = 0;
    jsonObject[FIELD_GROUP_ID] = 0;
    jsonObject[FIELD_GROUP_OWNER] = 0;
    jsonObject[FIELD_GROUP_TYPE] = "test";
    jsonObject[FIELD_GROUP_VISIBILITY] = "test";
    jsonObject[FIELD_USER_ID] = "userId";
    FromJson(jsonObject, groupInfo);
    EXPECT_EQ(groupInfo.userId, "userId");

    jsonObject[FIELD_USER_ID] = "0";
    jsonObject.Erase(FIELD_GROUP_NAME);
    jsonObject.Erase(FIELD_GROUP_ID);
    jsonObject.Erase(FIELD_GROUP_OWNER);
    jsonObject.Erase(FIELD_GROUP_TYPE);
    jsonObject.Erase(FIELD_GROUP_VISIBILITY);
    GroupInfo groupInfo1;
    jsonObject[FIELD_GROUP_NAME] = "test";
    FromJson(jsonObject, groupInfo1);
    EXPECT_EQ(groupInfo1.groupName, "test");
}

/**
 * @tc.name: HiChainConnector_001
 * @tc.desc: Returns a new pointer to the HiChainConnector constructor new
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, HiChainConnector_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> m_HiChainConnector = std::make_shared<HiChainConnector>();
    ASSERT_NE(m_HiChainConnector, nullptr);
}

/**
 * @tc.name: HiChainConnector_002
 * @tc.desc: Give the HiChainConnector constructor new a new pointer and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, HiChainConnector_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> m_HiChainConnector = std::make_shared<HiChainConnector>();
    m_HiChainConnector.reset();
    EXPECT_EQ(m_HiChainConnector, nullptr);
}

/**
 * @tc.name:RegisterHiChainCallback_001
 * @tc.desc: Call the RegisterHiChainCallback function with a return value of DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, RegisterHiChainCallback_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AddMember_001
 * @tc.desc: set deviceGroupManager_ = nullptr;
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, AddMember_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->deviceGroupManager_ = nullptr;
    std::string deviceId;
    std::string connectInfo;
    int ret = hiChainConnector->AddMember(deviceId, connectInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: AddMember_002
 * @tc.desc: set deviceId and connectInfo = null;
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, AddMember_002, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    std::string connectInfo;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->AddMember(deviceId, connectInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: AddMember_003
 * @tc.desc: set deviceId and connectInfo = null;
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, AddMember_003, testing::ext::TestSize.Level1)
{
    std::string deviceId = "123456";
    std::string connectInfo = "dkdkk";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->AddMember(deviceId, connectInfo);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: AddMember_004
 * @tc.desc: set deviceId and connectInfo = null;
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, AddMember_004, testing::ext::TestSize.Level1)
{
    std::string deviceId = "deviceIdTest";
    std::string connectInfo = R"(
    {
        "DEVICEID" : "deviceId",
        "pinCode" : 1,
        "groupId" : "groupId",
        "REQUESTID" : "requestId",
        "GROUPNAME" : "groupName"
    }
    )";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->AddMember(deviceId, connectInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: onRequest_001
 * @tc.desc:set operationCode != GroupOperationCode::MEMBER_JOIN(3);  return nullptr;
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, onRequest_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 2;
    int32_t operationCode = 2;
    char *reqParams;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    char *ret = hiChainConnector->onRequest(requestId, operationCode, reqParams);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: onRequest_002
 * @tc.desc: operationCode = GroupOperationCode::MEMBER_JOIN(3)/hiChainConnectorCallback_ is nullptr; return nullptr;
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, onRequest_002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 2;
    int32_t operationCode = 3;
    char *reqParams;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->hiChainConnectorCallback_ = nullptr;
    char *ret = hiChainConnector->onRequest(requestId, operationCode, reqParams);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: onRequest_003
 * @tc.desc: Test the onRequest method of HiChainConnector to ensure it handles different
 *           return values from GetPinCode correctly.
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, onRequest_003, testing::ext::TestSize.Level1)
{
    int64_t requestId = 2;
    int32_t operationCode = 3;
    char *reqParams = nullptr;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<MockIHiChainConnectorCallback> mockCallback = std::make_shared<MockIHiChainConnectorCallback>();
    hiChainConnector->hiChainConnectorCallback_ = mockCallback;
    EXPECT_CALL(*mockCallback, GetPinCode(testing::_))
        .Times(1)
        .WillOnce(testing::Return(ERR_DM_FAILED));
    EXPECT_NE(hiChainConnector->onRequest(requestId, operationCode, reqParams), nullptr);

    EXPECT_CALL(*mockCallback, GetPinCode(testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    EXPECT_NE(hiChainConnector->onRequest(requestId, operationCode, reqParams), nullptr);
}

/**
 * @tc.name: GetConnectPara_001
 * @tc.desc: Test GetConnectPara method when hiChainConnectorCallback_ is set to nullptr,
 *           ensuring it returns an empty string.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetConnectPara_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->hiChainConnectorCallback_ = nullptr;
    std::string deviceId = "12345";
    std::string reqDeviceId = "12345";
    std::string ret = hiChainConnector->GetConnectPara(deviceId, reqDeviceId);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: GetConnectPara_002
 * @tc.desc: Test GetConnectPara method with an empty deviceId to ensure JSON parsing fails
 *           and returns an empty string.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetConnectPara_002, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    std::string reqDeviceId = "12345";
    std::shared_ptr<MockIHiChainConnectorCallback> mockCallback = std::make_shared<MockIHiChainConnectorCallback>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->hiChainConnectorCallback_ = mockCallback;
    EXPECT_CALL(*mockCallback, GetConnectAddr(testing::_))
        .Times(1)
        .WillOnce(testing::Return(R"({"key": "value"})"));
    std::string ret = hiChainConnector->GetConnectPara(deviceId, reqDeviceId);
    EXPECT_NE(ret, "");
}

/**
 * @tc.name: GetConnectPara_003
 * @tc.desc: Test GetConnectPara method with invalid JSON returned by hiChainConnectorCallback_
 *           to ensure it returns an empty string.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetConnectPara_003, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    std::string reqDeviceId = "12345";
    std::shared_ptr<MockIHiChainConnectorCallback> mockCallback = std::make_shared<MockIHiChainConnectorCallback>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->hiChainConnectorCallback_ = mockCallback;
    EXPECT_CALL(*mockCallback, GetConnectAddr(testing::_))
        .Times(1)
        .WillOnce(testing::Return("invalid json"));
    std::string ret = hiChainConnector->GetConnectPara(deviceId, reqDeviceId);
    EXPECT_EQ(ret, "invalid json");
}

/**
 * @tc.name: DeleteGroup_001
 * @tc.desc: set groupId = "34567", and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DeleteGroup_001, testing::ext::TestSize.Level1)
{
    std::string groupId = "34567";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->DeleteGroup(groupId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetRelatedGroups_001
 * @tc.desc: set DeviceId 123  groupList null and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetRelatedGroups_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "123";
    std::vector<GroupInfo> groupList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->GetRelatedGroups(deviceId, groupList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetRelatedGroupsExt_001
 * @tc.desc: set DeviceId = 12345,groupList null and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetRelatedGroupsExt_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "12345";
    std::vector<GroupInfo> groupList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->GetRelatedGroupsExt(deviceId, groupList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SyncGroups_001
 * @tc.desc: set deviceId = "34567", and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, SyncGroups_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "34567";
    std::vector<std::string> remoteGroupIdList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->SyncGroups(deviceId, remoteGroupIdList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetSyncGroupList_001
 * @tc.desc: set groupList null, and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetSyncGroupList_001, testing::ext::TestSize.Level1)
{
    std::vector<GroupInfo> groupList;
    std::vector<std::string> syncGroupList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->GetSyncGroupList(groupList, syncGroupList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetSyncGroupList_002
 * @tc.desc: set groupList not null, and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetSyncGroupList_002, testing::ext::TestSize.Level1)
{
    std::vector<GroupInfo> groupList;
    GroupInfo groupList1;
    groupList1.groupName = "groupName";
    groupList1.groupId = 1;
    groupList1.groupOwner = "ohos.distributedhardware.devicemanager";
    groupList1.groupType = 7;
    groupList1.groupVisibility = 1;
    groupList.push_back(groupList1);
    GroupInfo groupList2;
    groupList2.groupName = "hichainconnector";
    groupList2.groupId = "123456";
    groupList2.groupOwner = "doftbus";
    groupList2.groupType = 1;
    groupList2.groupVisibility = 2;
    groupList.push_back(groupList2);
    std::vector<std::string> syncGroupList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->GetSyncGroupList(groupList, syncGroupList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: IsGroupCreated_001
 * @tc.desc: return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, IsGroupCreated_001, testing::ext::TestSize.Level1)
{
    std::string groupName = "groupNameTest";
    GroupInfo groupInfo;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    bool ret = hiChainConnector->IsGroupCreated(groupName, groupInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsRedundanceGroup_001
 * @tc.desc: return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, IsRedundanceGroup_001, testing::ext::TestSize.Level1)
{
    const std::string userId = "userIdTest";
    int32_t authType = 1;
    std::vector<GroupInfo> groupList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    bool ret = hiChainConnector->IsRedundanceGroup(userId, authType, groupList);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: onFinish_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, onFinish_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 1;
    int operationCode = GroupOperationCode::MEMBER_JOIN;
    const char *returnData = "returnDataTest";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());

    hiChainConnector->onFinish(requestId, operationCode, returnData);

    operationCode = GroupOperationCode::GROUP_CREATE;
    hiChainConnector->onFinish(requestId, operationCode, returnData);

    operationCode = GroupOperationCode::MEMBER_DELETE;
    hiChainConnector->onFinish(requestId, operationCode, returnData);

    operationCode = GroupOperationCode::GROUP_DISBAND;
    hiChainConnector->onFinish(requestId, operationCode, returnData);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: onFinish_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, onFinish_002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 1;
    int operationCode = GroupOperationCode::MEMBER_JOIN;
    const char *returnData = "returnDataTest";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    EXPECT_EQ(ret, DM_OK);
    ret = hiChainConnector->UnRegisterHiChainCallback();
    EXPECT_EQ(ret, DM_OK);

    hiChainConnector->onFinish(requestId, operationCode, returnData);

    operationCode = GroupOperationCode::GROUP_CREATE;
    hiChainConnector->onFinish(requestId, operationCode, returnData);

    hiChainConnector->networkStyle_ = 1;
    hiChainConnector->hiChainResCallback_ = nullptr;
    hiChainConnector->onFinish(requestId, operationCode, returnData);

    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainConnector> hiChainConn = std::make_shared<HiChainConnector>();
    hiChainConnector->hiChainResCallback_ = std::make_shared<DmCredentialManager>(hiChainConn, listener);
    hiChainConnector->onFinish(requestId, operationCode, returnData);

    operationCode = GroupOperationCode::GROUP_DISBAND;
    hiChainConnector->onFinish(requestId, operationCode, returnData);

    hiChainConnector->hiChainResCallback_ = nullptr;
    hiChainConnector->onFinish(requestId, operationCode, returnData);

    EXPECT_EQ(hiChainConnector->hiChainConnectorCallback_, nullptr);
}

/**
 * @tc.name: onError_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, onError_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 1;
    int operationCode = GroupOperationCode::MEMBER_JOIN;
    int errorCode = 1;
    const char *errorReturn = "errorReturnTest";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    operationCode = GroupOperationCode::GROUP_CREATE;
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    operationCode = GroupOperationCode::MEMBER_DELETE;
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    operationCode = GroupOperationCode::GROUP_DISBAND;
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: onError_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, onError_002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 1;
    int operationCode = GroupOperationCode::MEMBER_JOIN;
    int errorCode = 1;
    const char *errorReturn = "errorReturnTest";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());
    EXPECT_EQ(ret, DM_OK);
    ret = hiChainConnector->UnRegisterHiChainCallback();
    EXPECT_EQ(ret, DM_OK);
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    operationCode = GroupOperationCode::GROUP_CREATE;
    hiChainConnector->networkStyle_ = 0;
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    hiChainConnector->networkStyle_ = 1;
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainConnector> hiChainConn = std::make_shared<HiChainConnector>();
    hiChainConnector->hiChainResCallback_ = std::make_shared<DmCredentialManager>(hiChainConn, listener);
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    operationCode = GroupOperationCode::GROUP_DISBAND;
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);

    hiChainConnector->hiChainResCallback_ = nullptr;
    hiChainConnector->onError(requestId, operationCode, errorCode, errorReturn);
    EXPECT_EQ(hiChainConnector->hiChainConnectorCallback_, nullptr);
}

/**
 * @tc.name: DeleteGroup_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DeleteGroup_002, testing::ext::TestSize.Level1)
{
    const int32_t userId = 1;
    std::string groupId = "groupIdTest";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->DeleteGroup(userId, groupId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteGroup_003
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DeleteGroup_003, testing::ext::TestSize.Level1)
{
    int64_t requestId = 1;
    std::string userId = "userIdTest";
    const int32_t authType = 1;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->DeleteRedundanceGroup(userId);
    int32_t ret = hiChainConnector->DeleteGroup(requestId, userId, authType);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: RegisterHiChainGroupCallback_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, RegisterHiChainGroupCallback_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<IDmGroupResCallback> callback = nullptr;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->RegisterHiChainGroupCallback(callback);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterHiChainGroupCallback_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, UnRegisterHiChainGroupCallback_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->UnRegisterHiChainGroupCallback();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: getRegisterInfo_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, getRegisterInfo_001, testing::ext::TestSize.Level1)
{
    std::string queryParams;
    std::string returnJsonStr;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->deviceGroupManager_ = nullptr;
    int32_t ret = hiChainConnector->getRegisterInfo(queryParams, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: getRegisterInfo_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, getRegisterInfo_002, testing::ext::TestSize.Level1)
{
    std::string queryParams;
    std::string returnJsonStr;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->getRegisterInfo(queryParams, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredential_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, ParseRemoteCredential_001, testing::ext::TestSize.Level1)
{
    int32_t groupType = 1;
    std::string userId;
    JsonObject jsonDeviceList;
    std::string params = "paramsTest";
    int32_t osAccountUserId = 0;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseRemoteCredential_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, ParseRemoteCredential_002, testing::ext::TestSize.Level1)
{
    int32_t groupType = 1;
    std::string userId = "1321231";
    JsonObject jsonDeviceList;
    std::string params = "paramsTest";
    int32_t osAccountUserId = 0;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseRemoteCredential_003
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, ParseRemoteCredential_003, testing::ext::TestSize.Level1)
{
    int32_t groupType = 1;
    std::string userId = "1321231";
    JsonObject jsonDeviceList;
    jsonDeviceList[FIELD_DEVICE_LIST] = "15264646";
    std::string params = "paramsTest";
    int32_t osAccountUserId = 0;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->ParseRemoteCredential(groupType, userId, jsonDeviceList, params, osAccountUserId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembers_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, addMultiMembers_001, testing::ext::TestSize.Level1)
{
    int32_t groupType = 1;
    std::string userId = "";
    JsonObject jsonDeviceList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->deviceGroupManager_ = nullptr;
    int32_t ret = hiChainConnector->addMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: addMultiMembers_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, addMultiMembers_002, testing::ext::TestSize.Level1)
{
    int32_t groupType = 1;
    std::string userId = "userIdTest";
    JsonObject jsonDeviceList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    EXPECT_NE(hiChainConnector->deviceGroupManager_, nullptr);
    int32_t ret = hiChainConnector->addMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: deleteMultiMembers_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, deleteMultiMembers_001, testing::ext::TestSize.Level1)
{
    int32_t groupType = 1;
    std::string userId = "userIdTest";
    JsonObject jsonDeviceList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->deviceGroupManager_ = nullptr;
    int32_t ret = hiChainConnector->deleteMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: deleteMultiMembers_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, deleteMultiMembers_002, testing::ext::TestSize.Level1)
{
    int32_t groupType = 1;
    std::string userId = "";
    JsonObject jsonDeviceList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    EXPECT_NE(hiChainConnector->deviceGroupManager_, nullptr);
    int32_t ret = hiChainConnector->deleteMultiMembers(groupType, userId, jsonDeviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: IsDevicesInP2PGroup_001
 * @tc.desc: return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, IsDevicesInP2PGroup_001, testing::ext::TestSize.Level1)
{
    std::string hostDevice = "hostDeviceTest";
    std::string peerDevice = "peerDeviceTest";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    bool ret = hiChainConnector->IsDevicesInP2PGroup(hostDevice, peerDevice);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UnRegisterHiChainCallback_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, UnRegisterHiChainCallback_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->UnRegisterHiChainCallback();
    EXPECT_EQ(hiChainConnector->hiChainConnectorCallback_, nullptr);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetGroupInfo_001
 * @tc.desc: return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetGroupInfo_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::string queryParams;
    std::vector<GroupInfo> groupList;
    bool ret = hiChainConnector->GetGroupInfo(queryParams, groupList);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetGroupInfo_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetGroupInfo_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t userId = 12;
    std::string queryParams;
    std::vector<GroupInfo> groupList;
    int32_t ret = hiChainConnector->GetGroupInfo(userId, queryParams, groupList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetGroupType_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetGroupType_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::string deviceId;
    auto ret = hiChainConnector->GetGroupType(deviceId);
    EXPECT_EQ(ret, DmAuthForm::INVALID_TYPE);
}

/**
 * @tc.name: DeleteGroupExt_001
 * @tc.desc: set groupId = "34567", and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DeleteGroupExt_001, testing::ext::TestSize.Level1)
{
    std::string groupId = "34567";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->DeleteGroupExt(groupId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteTimeOutGroup_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DeleteTimeOutGroup_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "13245631";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int ret = hiChainConnector->DeleteTimeOutGroup(deviceId.c_str());
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DealRedundanceGroup_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DealRedundanceGroup_001, testing::ext::TestSize.Level1)
{
    std::string userId = "13245631";
    int32_t authType = 1;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->DealRedundanceGroup(userId, authType);
    EXPECT_NE(hiChainConnector->deviceGroupManager_, nullptr);
}

/**
 * @tc.name: GetGroupId_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetGroupId_001, testing::ext::TestSize.Level1)
{
    std::string userId = "13245631";
    int32_t authType = 1;
    std::string groupId = "232310";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetGroupId(userId, authType, groupId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetJsonStr_001
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetJsonStr_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    std::string key;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    auto ret = hiChainConnector->GetJsonStr(jsonObj, key);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: GetJsonStr_002
 * @tc.desc: return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetJsonStr_002, testing::ext::TestSize.Level1)
{
    std::string key = "key";
    JsonObject jsonObj;
    jsonObj[key] = "232513";
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    auto ret = hiChainConnector->GetJsonStr(jsonObj, key);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: GetJsonInt_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetJsonInt_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    std::string key;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetJsonInt(jsonObj, key);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetJsonInt_002
 * @tc.desc: return 232513
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetJsonInt_002, testing::ext::TestSize.Level1)
{
    std::string key = "12";
    JsonObject jsonObj;
    jsonObj[key] = 232513;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetJsonInt(jsonObj, key);
    EXPECT_EQ(ret, 232513);
}

/**
 * @tc.name: GetGroupIdExt_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetGroupIdExt_001, testing::ext::TestSize.Level1)
{
    std::string userId = "12";
    int32_t groupType = 1;
    std::string groupId;
    std::string groupOwner;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetGroupIdExt(userId, groupType, groupId, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, ParseRemoteCredentialExt_001, testing::ext::TestSize.Level1)
{
    std::string credentialInfo;
    std::string params;
    std::string groupOwner;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, ParseRemoteCredentialExt_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    std::string credentialInfo = jsonObj.Dump();
    std::string params;
    std::string groupOwner;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_003
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, ParseRemoteCredentialExt_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["authType"] = 1;
    jsonObj[FIELD_USER_ID] = "156103";
    std::string credentialInfo = jsonObj.Dump();
    std::string params;
    std::string groupOwner;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ParseRemoteCredentialExt_004
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, ParseRemoteCredentialExt_004, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    jsonObj["authType"] = 4;
    jsonObj[FIELD_USER_ID] = "156103";
    std::string credentialInfo = jsonObj.Dump();
    std::string params;
    std::string groupOwner;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->ParseRemoteCredentialExt(credentialInfo, params, groupOwner);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: addMultiMembersExt_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, addMultiMembersExt_001, testing::ext::TestSize.Level1)
{
    std::string credentialInfo;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->deviceGroupManager_ = nullptr;
    int32_t ret = hiChainConnector->addMultiMembersExt(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: addMultiMembersExt_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, addMultiMembersExt_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    std::string credentialInfo = jsonObj.Dump();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->addMultiMembersExt(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetTrustedDevices_001
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetTrustedDevices_001, testing::ext::TestSize.Level1)
{
    std::string localDeviceUdid;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    auto ret = hiChainConnector->GetTrustedDevices(localDeviceUdid);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: GetTrustedDevicesUdid_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetTrustedDevicesUdid_001, testing::ext::TestSize.Level1)
{
    std::string jsonStr;
    std::vector<std::string> udidList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetTrustedDevicesUdid(jsonStr.c_str(), udidList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetTrustedDevicesUdid_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetTrustedDevicesUdid_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObj;
    std::string jsonStr = jsonObj.Dump();
    std::vector<std::string> udidList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetTrustedDevicesUdid(jsonStr.c_str(), udidList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetTrustedDevicesUdid_003
 * @tc.desc: Verify that the function returns DM_OK and correctly populates the udidList.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetTrustedDevicesUdid_003, testing::ext::TestSize.Level1)
{
    const char* jsonStr = R"({
        "device1": {
            "authId": "valid_udid_1"
        },
        "device2": {
            "authId": 12345
        },
        "device3": {
            "authId": "valid_udid_2"
        }
    })";

    std::vector<std::string> udidList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetTrustedDevicesUdid(jsonStr, udidList);
    EXPECT_EQ(ret, DM_OK);

    std::vector<std::string> expectedUdidList = {"valid_udid_1", "valid_udid_2"};
    EXPECT_EQ(udidList, expectedUdidList);
}

/**
 * @tc.name: DeleteAllGroup_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, DeleteAllGroup_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 1;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    hiChainConnector->DeleteAllGroup(userId);
    EXPECT_NE(hiChainConnector->deviceGroupManager_, nullptr);
}

/**
 * @tc.name: GetRelatedGroupsCommon_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(HichainConnectorTest, GetRelatedGroupsCommon_001, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    std::string  pkgName;
    std::vector<GroupInfo> groupList;
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t ret = hiChainConnector->GetRelatedGroupsCommon(deviceId, pkgName.c_str(), groupList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
