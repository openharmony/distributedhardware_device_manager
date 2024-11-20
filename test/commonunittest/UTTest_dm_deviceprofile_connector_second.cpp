/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "UTTest_dm_deviceprofile_connector_second.h"

#include "dm_constants.h"
#include "deviceprofile_connector.h"
#include <iterator>

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void DeviceProfileConnectorSecondTest::SetUp()
{
}

void DeviceProfileConnectorSecondTest::TearDown()
{
}

void DeviceProfileConnectorSecondTest::SetUpTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
}

void DeviceProfileConnectorSecondTest::TearDownTestCase()
{
    distributedDeviceProfileClientMock_ = nullptr;
}

void AddAccessControlProfile(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "123456";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAccessControlProfile_201, testing::ext::TestSize.Level0)
{
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAccessControlProfileByUserId_201, testing::ext::TestSize.Level0)
{
    int32_t userId = DEVICE;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    profiles = DeviceProfileConnector::GetInstance().GetAccessControlProfileByUserId(userId);
    EXPECT_TRUE(profiles.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAuthForm_201, testing::ext::TestSize.Level0)
{
    DmAuthForm form = DmAuthForm::ACROSS_ACCOUNT;
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "pkgName";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().CheckAuthForm(form, profiles, discoveryInfo);
    EXPECT_EQ(ret, DmAuthForm::ACROSS_ACCOUNT);

    profiles.accesser_.SetAccesserBundleName("pkgName");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    ret = DeviceProfileConnector::GetInstance().CheckAuthForm(form, profiles, discoveryInfo);
    EXPECT_EQ(ret, DmAuthForm::ACROSS_ACCOUNT);

    profiles.SetBindLevel(SERVICE);
    ret = DeviceProfileConnector::GetInstance().CheckAuthForm(form, profiles, discoveryInfo);
    EXPECT_EQ(ret, DmAuthForm::INVALID_TYPE);

    profiles.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().CheckAuthForm(form, profiles, discoveryInfo);
    EXPECT_EQ(ret, DmAuthForm::ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorSecondTest, PutAccessControlList_201, testing::ext::TestSize.Level0)
{
    DmAclInfo aclInfo;
    DmAccesser dmAccesser;
    DmAccessee dmAccessee;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutAccessControlProfile(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckIdenticalAccount_201, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    std::string accountId;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    bool ret = DeviceProfileConnector::GetInstance().CheckIdenticalAccount(userId, accountId);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllAccessControlProfile_201, testing::ext::TestSize.Level0)
{
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAclForAccountLogOut_001, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_device_id";
    int32_t localUserId = 1;
    std::string peerUdid = "peer_device_id";
    int32_t peerUserId = 2;
    int32_t result = connector.DeleteAclForAccountLogOut(localUdid, localUserId, peerUdid, peerUserId);

    EXPECT_EQ(result, false);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAppBindLevel_001, testing::ext::TestSize.Level0)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "com.example.app";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles = {};
    std::string localUdid = "local_udid";
    std::string remoteUdid = "remote_udid";
    connector.DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid);

    EXPECT_EQ(offlineParam.processVec.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckIsSameAccount_001, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    caller.pkgName = "test_pkg";
    std::string srcUdid = "src_udid";
    DmAccessCallee callee;
    std::string sinkUdid = "non_identical_udid";

    EXPECT_EQ(connector.CheckIsSameAccount(caller, srcUdid, callee, sinkUdid), ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndBindLevel_001, testing::ext::TestSize.Level0)
{
    std::vector<int32_t> userIds = {4, 5, 6};
    std::string localUdid = "local_udid";
    auto result = connector.GetDeviceIdAndBindLevel(userIds, localUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclProfileByUserId_001, testing::ext::TestSize.Level0)
{
    std::string localUdid = "localDevice";
    int32_t userId = -1;
    std::string remoteUdid = "remoteDevice";
    auto result = connector.GetAclProfileByUserId(localUdid, userId, remoteUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclProfileByUserId_002, testing::ext::TestSize.Level0)
{
    std::string localUdid = "localDevice";
    int32_t userId = 1;
    std::string remoteUdid = "nonExistentDevice";
    auto result = connector.GetAclProfileByUserId(localUdid, userId, remoteUdid);
    
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclProfileByUserId_003, testing::ext::TestSize.Level0)
{
    std::string localUdid = "";
    int32_t userId = 0;
    std::string remoteUdid = "";
    auto result = connector.GetAclProfileByUserId(localUdid, userId, remoteUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetOfflineProcessInfo_001, testing::ext::TestSize.Level0)
{
    std::string localUdid = "invalid_device";
    std::vector<int32_t> localUserIds = {1, 2};
    std::string remoteUdid = "remote_device";
    std::vector<int32_t> remoteUserIds = {3, 4};
    std::vector<ProcessInfo> result =
        connector.GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid, remoteUserIds);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetOfflineProcessInfo_002, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_device";
    std::vector<int32_t> localUserIds = {1, 2};
    std::string remoteUdid = "remote_device";
    std::vector<int32_t> remoteUserIds = {99, 100};
    std::vector<ProcessInfo> result =
        connector.GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid, remoteUserIds);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetOfflineProcessInfo_003, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_device";
    std::vector<int32_t> localUserIds = {};
    std::string remoteUdid = "remote_device";
    std::vector<int32_t> remoteUserIds = {3, 4};
    std::vector<ProcessInfo> result =
        connector.GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid, remoteUserIds);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetUserIdAndBindLevel_001, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_udid";
    std::string peerUdid = "peer_udid";
    std::map<int32_t, int32_t> result = connector.GetUserIdAndBindLevel(localUdid, peerUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetUserIdAndBindLevel_002, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_udid";
    std::string peerUdid = "peer_udid";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(DM_OK));
    std::map<int32_t, int32_t> result = connector.GetUserIdAndBindLevel(localUdid, peerUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_001, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_udid_123";
    std::string peerUdid = "peer_udid_456";
    int32_t peerUserId = 789;
    std::string peerAccountHash = "invalid_hash";
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_002, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_udid_123";
    std::string peerUdid = "non_matching_udid";
    int32_t peerUserId = 789;
    std::string peerAccountHash = "valid_hash";
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_003, testing::ext::TestSize.Level0)
{
    std::string localUdid = "local_udid_123";
    std::string peerUdid = "peer_udid_456";
    int32_t peerUserId = -1;
    std::string peerAccountHash = "valid_hash";
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_004, testing::ext::TestSize.Level0)
{
    std::string localUdid = "";
    std::string peerUdid = "";
    int32_t peerUserId = 0;
    std::string peerAccountHash = "";
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUserId_001, testing::ext::TestSize.Level0)
{
    auto result = connector.GetDeviceIdAndUserId("device4", 4);
    EXPECT_EQ(result.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUserId_002, testing::ext::TestSize.Level0)
{
    auto result = connector.GetDeviceIdAndUserId("", 0);
    EXPECT_EQ(result.size(), 0);
}
} // namespace DistributedHardware
} // namespace OHOS
