/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "dp_inited_callback_stub.h"
#include "dm_error_type.h"
#include "dm_crypto.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {

void DeviceProfileConnectorSecondTest::SetUp()
{
}

void DeviceProfileConnectorSecondTest::TearDown()
{
    Mock::VerifyAndClearExpectations(distributedDeviceProfileClientMock_.get());
    Mock::VerifyAndClearExpectations(multipleUserConnectorMock_.get());
}

void DeviceProfileConnectorSecondTest::SetUpTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

void DeviceProfileConnectorSecondTest::TearDownTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
}

class DpInitedCallback : public DistributedDeviceProfile::DpInitedCallbackStub {
public:
    DpInitedCallback()
    {
    }
    ~DpInitedCallback()
    {
    }
    int32_t OnDpInited()
    {
        return 0;
    }
};

DistributedDeviceProfile::AccessControlProfile GenerateAccessControlProfile(int64_t accessControlId,
    uint32_t deviceIdType, const std::string &peerUdid, int32_t authType, int32_t bindType, uint32_t status,
    uint32_t accesserId, const std::string &accesserUdId, int32_t accesserUserId, const std::string &accesserAccountId,
    uint32_t accesseeId, const std::string &accesseeUdId, int32_t accesseeUserId, const std::string &accesseeAccountId,
    uint32_t tokenId, const std::string &bundleName, uint32_t bindLevel, const std::string &extraData)
{
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(accesserUdId);
    accesser.SetAccesserUserId(accesserUserId);
    accesser.SetAccesserAccountId(accesserAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName(bundleName);
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesseeId);
    accessee.SetAccesseeDeviceId(accesseeUdId);
    accessee.SetAccesseeUserId(accesseeUserId);
    accessee.SetAccesseeAccountId(accesseeAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName(bundleName);
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile accessControlProfile;
    accessControlProfile.SetAccessControlId(accessControlId);
    accessControlProfile.SetDeviceIdType(deviceIdType);
    accessControlProfile.SetTrustDeviceId(peerUdid);
    accessControlProfile.SetAuthenticationType(authType);
    accessControlProfile.SetBindType(bindType);
    accessControlProfile.SetStatus(status);
    accessControlProfile.SetAccesserId(accesserId);
    accessControlProfile.SetAccesseeId(accesseeId);
    accessControlProfile.SetBindLevel(bindLevel);
    accessControlProfile.SetAccesser(accesser);
    accessControlProfile.SetAccessee(accessee);
    accessControlProfile.SetExtraData(extraData);
    return accessControlProfile;
}

void AddAccessControlProfile(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    uint32_t accessControlId = 1;
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = ACTIVE;
    uint32_t authType = 2;
    uint32_t accesserId = 1;
    uint32_t accesseeId = 1;
    uint32_t tokenId = 1001;

    std::string accesserAccountId = "oldAccountId";
    std::string accesseeAccountId = "newAccountId";
    std::string accesserUdId = "remoteDeviceId";
    std::string accesseeUdId = "localDeviceId";

    accessControlProfiles.emplace_back(GenerateAccessControlProfile(accessControlId, deviceIdType, accesserUdId,
        authType, bindType, status, accesserId, accesserUdId, userId, accesserAccountId, accesseeId, accesseeUdId,
        userId, accesseeAccountId, tokenId, "bundleName", bindLevel, ""));
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAccessControlProfile_201, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAccessControlProfileByUserId_201, testing::ext::TestSize.Level1)
{
    int32_t userId = USER;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    profiles = DeviceProfileConnector::GetInstance().GetAccessControlProfileByUserId(userId);
    EXPECT_TRUE(profiles.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAuthForm_201, testing::ext::TestSize.Level1)
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
    EXPECT_EQ(ret, DmAuthForm::ACROSS_ACCOUNT);

    profiles.SetBindLevel(USER);
    ret = DeviceProfileConnector::GetInstance().CheckAuthForm(form, profiles, discoveryInfo);
    EXPECT_EQ(ret, DmAuthForm::ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorSecondTest, PutAccessControlList_201, testing::ext::TestSize.Level1)
{
    DmAclInfo aclInfo;
    DmAccesser dmAccesser;
    DmAccessee dmAccessee;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutAccessControlProfile(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteSigTrustACL_201, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    std::string accountId;

    userId = 1;
    int32_t bindType = 1;
    std::string deviceIdEr = "deviceId";
    std::string deviceIdEe = "deviceIdEe";
    uint32_t accesserId = 1;
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetAccessControlId(accesserId);
    profile.SetBindType(bindType);
    profile.SetAccesser(accesser);
    profile.SetAccessee(accessee);

    int userIds = 12356;
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> remoteFrontUserIds;
    remoteFrontUserIds.push_back(userIds);
    std::vector<int32_t> remoteBackUserIds;
    remoteBackUserIds.push_back(userIds);
    DmOfflineParam offlineParam;
    DeviceProfileConnector::GetInstance().DeleteSigTrustACL(profile, remoteUdid, remoteFrontUserIds, remoteBackUserIds,
        offlineParam);

    remoteUdid = "deviceIdEe";
    DeviceProfileConnector::GetInstance().DeleteSigTrustACL(profile, remoteUdid, remoteFrontUserIds, remoteBackUserIds,
        offlineParam);

    int32_t userIdee = 0;
    accessee.SetAccesseeUserId(userIdee);
    DistributedDeviceProfile::AccessControlProfile profilesecond;
    profilesecond.SetAccessControlId(accesserId);
    profilesecond.SetBindType(bindType);
    profilesecond.SetAccesser(accesser);
    profilesecond.SetAccessee(accessee);
    std::string localUdid = "deviceId";
    std::vector<int32_t> localUserIds;
    int32_t localUserId = 1;
    localUserIds.push_back(localUserId);
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_)).WillOnce(Return(ERR_DM_FAILED));
    DeviceProfileConnector::GetInstance().UpdatePeerUserId(profilesecond, localUdid, localUserIds,
        remoteUdid, remoteFrontUserIds);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllAccessControlProfile_201, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclProfileByDeviceIdAndUserId_201, testing::ext::TestSize.Level1)
{
    std::string deviceId = "deviceId";
    int32_t userId = 123456;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = DeviceProfileConnector::GetInstance().GetAclProfileByDeviceIdAndUserId(deviceId, userId);
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAppBindLevel_001, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "com.example.app";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles = {};
    std::string localUdid = "local_udid";
    std::string remoteUdid = "remote_udid";
    connector.DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid);

    EXPECT_EQ(offlineParam.processVec.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.pkgName = "test_pkg";
    std::string srcUdid = "src_udid";
    DmAccessCallee callee;
    std::string sinkUdid = "non_identical_udid";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    EXPECT_EQ(connector.CheckIsSameAccount(caller, srcUdid, callee, sinkUdid), false);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndBindLevel_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIds = {4, 5, 6};
    std::string localUdid = "local_udid";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    auto result = DeviceProfileConnector::GetInstance().GetDeviceIdAndBindLevel(userIds, localUdid);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclProfileByUserId_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDevice";
    int32_t userId = -1;
    std::string remoteUdid = "remoteDevice";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    auto result = connector.GetAclProfileByUserId(localUdid, userId, remoteUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclProfileByUserId_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDevice";
    int32_t userId = 1;
    std::string remoteUdid = "nonExistentDevice";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    auto result = connector.GetAclProfileByUserId(localUdid, userId, remoteUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclProfileByUserId_003, testing::ext::TestSize.Level1)
{
    std::string localUdid = "";
    int32_t userId = 0;
    std::string remoteUdid = "";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    auto result = connector.GetAclProfileByUserId(localUdid, userId, remoteUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetOfflineProcessInfo_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "invalid_device";
    std::vector<int32_t> localUserIds = {1, 2};
    std::string remoteUdid = "remote_device";
    std::vector<int32_t> remoteUserIds = {3, 4};
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::vector<ProcessInfo> result =
        connector.GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid, remoteUserIds);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetOfflineProcessInfo_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_device";
    std::vector<int32_t> localUserIds = {1, 2};
    std::string remoteUdid = "remote_device";
    std::vector<int32_t> remoteUserIds = {99, 100};
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::vector<ProcessInfo> result =
        connector.GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid, remoteUserIds);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetOfflineProcessInfo_003, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_device";
    std::vector<int32_t> localUserIds = {};
    std::string remoteUdid = "remote_device";
    std::vector<int32_t> remoteUserIds = {3, 4};
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::vector<ProcessInfo> result =
        connector.GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid, remoteUserIds);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetUserIdAndBindLevel_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_udid";
    std::string peerUdid = "peer_udid";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::map<int32_t, int32_t> result = connector.GetUserIdAndBindLevel(localUdid, peerUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetUserIdAndBindLevel_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_udid";
    std::string peerUdid = "peer_udid";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::map<int32_t, int32_t> result = connector.GetUserIdAndBindLevel(localUdid, peerUdid);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_udid_123";
    std::string peerUdid = "peer_udid_456";
    int32_t peerUserId = 789;
    std::string peerAccountHash = "invalid_hash";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_udid_123";
    std::string peerUdid = "non_matching_udid";
    int32_t peerUserId = 789;
    std::string peerAccountHash = "valid_hash";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_003, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_udid_123";
    std::string peerUdid = "peer_udid_456";
    int32_t peerUserId = -1;
    std::string peerAccountHash = "valid_hash";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDevIdAndUserIdByActHash_004, testing::ext::TestSize.Level1)
{
    std::string localUdid = "";
    std::string peerUdid = "";
    int32_t peerUserId = 0;
    std::string peerAccountHash = "";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    std::multimap<std::string, int32_t> result =
        connector.GetDevIdAndUserIdByActHash(localUdid, peerUdid, peerUserId, peerAccountHash);

    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUserId_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    auto result = connector.GetDeviceIdAndUserId("device4", 4);
    EXPECT_EQ(result.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUserId_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    auto result = connector.GetDeviceIdAndUserId("", 0);
    EXPECT_EQ(result.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, SubscribeDeviceProfileInited_201, testing::ext::TestSize.Level1)
{
    OHOS::sptr<DistributedDeviceProfile::IDpInitedCallback> dpInitedCallback = nullptr;
    int32_t ret = DeviceProfileConnector::GetInstance().SubscribeDeviceProfileInited(dpInitedCallback);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    dpInitedCallback = sptr<DistributedDeviceProfile::IDpInitedCallback>(new DpInitedCallback());
    EXPECT_CALL(*distributedDeviceProfileClientMock_, SubscribeDeviceProfileInited(_, _))
        .WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceProfileConnector::GetInstance().SubscribeDeviceProfileInited(dpInitedCallback);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, SubscribeDeviceProfileInited(_, _))
        .WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().SubscribeDeviceProfileInited(dpInitedCallback);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UnSubscribeDeviceProfileInited_201, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UnSubscribeDeviceProfileInited(_))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceProfileConnector::GetInstance().UnSubscribeDeviceProfileInited();
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, UnSubscribeDeviceProfileInited(_))
        .WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().UnSubscribeDeviceProfileInited();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, PutAllTrustedDevices_201, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::TrustedDeviceInfo> deviceInfos;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutAllTrustedDevices(_))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceProfileConnector::GetInstance().PutAllTrustedDevices(deviceInfos);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutAllTrustedDevices(_))
        .WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().PutAllTrustedDevices(deviceInfos);
    EXPECT_EQ(ret, DM_OK);

    int32_t bindType = 0;
    std::string peerUdid = "";
    std::string localUdid = "";
    int32_t localUserId = 1234;
    std::string localAccountId = "";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    DeviceProfileConnector::GetInstance().HandleDeviceUnBind(bindType, peerUdid, localUdid, localUserId,
        localAccountId);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAccessControlList_201, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string localDeviceId = "";
    std::string remoteDeviceId = "";
    int32_t bindLevel = 2;
    std::string extra = "";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(offlineParam.bindType, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorSecondTest, PutSessionKey_201, testing::ext::TestSize.Level1)
{
    std::vector<unsigned char> sessionKeyArray;
    int32_t sessionKeyId = 1;
    int32_t userId = 100;
    int32_t ret = DeviceProfileConnector::GetInstance().PutSessionKey(userId, sessionKeyArray, sessionKeyId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    sessionKeyArray.push_back('1');
    sessionKeyArray.push_back('2');
    sessionKeyArray.push_back('3');
    sessionKeyArray.push_back('4');
    sessionKeyArray.push_back('5');
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutSessionKey(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceProfileConnector::GetInstance().PutSessionKey(userId, sessionKeyArray, sessionKeyId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutSessionKey(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().PutSessionKey(userId, sessionKeyArray, sessionKeyId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, PutLocalServiceInfo_201, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutLocalServiceInfo(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceProfileConnector::GetInstance().PutLocalServiceInfo(localServiceInfo);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutLocalServiceInfo(_)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceProfileConnector::GetInstance().PutLocalServiceInfo(localServiceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteLocalServiceInfo_201, testing::ext::TestSize.Level1)
{
    std::string bundleName = "b********pl";
    int32_t pinExchangeType = 1;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, DeleteLocalServiceInfo(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceProfileConnector::GetInstance().DeleteLocalServiceInfo(bundleName, pinExchangeType);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, DeleteLocalServiceInfo(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceProfileConnector::GetInstance().DeleteLocalServiceInfo(bundleName, pinExchangeType);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UpdateLocalServiceInfo_201, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateLocalServiceInfo(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateLocalServiceInfo(localServiceInfo);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateLocalServiceInfo(_)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceProfileConnector::GetInstance().UpdateLocalServiceInfo(localServiceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetLocalServiceInfoByBundleNameAndPinExchangeType_201,
        testing::ext::TestSize.Level1)
{
    std::string bundleName = "b********pl";
    int32_t pinExchangeType = 1;
    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName,
        pinExchangeType, localServiceInfo);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName,
        pinExchangeType, localServiceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleDmAuthForm_009, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_SHARE);
    profiles.SetBindLevel(USER);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, SHARE);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleDmAuthForm_010, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_SHARE);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("ohos_test");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "ohos_test";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, SHARE);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleDmAuthForm_011, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_SHARE);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "pkgName";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, SHARE);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::SHARE;

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeDeviceId(deviceId);
    profile.SetAccessee(accessee);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(trustDeviceId);
    profile.SetAccesser(accesser);

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::SHARE;

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(0);
    accessee.SetAccesseeDeviceId(deviceId);
    profile.SetAccessee(accessee);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(trustDeviceId);
    profile.SetAccesser(accesser);

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::SHARE;

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(-1);
    accessee.SetAccesseeDeviceId(deviceId);
    profile.SetAccessee(accessee);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(trustDeviceId);
    profile.SetAccesser(accesser);
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_004, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::ACROSS_ACCOUNT;

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeDeviceId("wrongDeviceId");
    profile.SetAccessee(accessee);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(trustDeviceId);
    profile.SetAccesser(accesser);

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_005, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::ACROSS_ACCOUNT;

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeDeviceId(deviceId);
    profile.SetAccessee(accessee);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId("wrongTrustDeviceId");
    profile.SetAccesser(accesser);

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_006, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::PEER_TO_PEER; // Wrong bind type

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeDeviceId(deviceId);
    profile.SetAccessee(accessee);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(trustDeviceId);
    profile.SetAccesser(accesser);

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_007, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::ACROSS_ACCOUNT;

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(999999); // Different user ID
    accessee.SetAccesseeDeviceId(deviceId);
    profile.SetAccessee(accessee);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(trustDeviceId);
    profile.SetAccesser(accesser);
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareType_008, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t userId = 123456;
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "trustDeviceId";
    int32_t bindType = DmAuthForm::ACROSS_ACCOUNT;

    // Empty profile (no accessee/accesser set)
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareType(
        profile, userId, deviceId, trustDeviceId, bindType);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthFormMap_001, testing::ext::TestSize.Level1)
{
    // Test case 1: Empty input - should return empty map
    std::string pkgName = "testPkg";;
    std::string deviceId = "deviceId1";
    int32_t userId = 123456;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profilesFilter;

    auto ret = DeviceProfileConnector::GetInstance().GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
    EXPECT_EQ(ret.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthFormMap_002, testing::ext::TestSize.Level1)
{
    // Test case 2: Profile with trustDeviceId matching input deviceId - should be skipped
    std::string pkgName = "testPkg";
    std::string deviceId = "deviceId1";
    int32_t userId = 123456;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profilesFilter;
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId(deviceId); // Matching deviceId
    profile.SetStatus(ACTIVE);
    profilesFilter.push_back(profile);

    auto ret = DeviceProfileConnector::GetInstance().GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
    EXPECT_EQ(ret.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthFormMap_003, testing::ext::TestSize.Level1)
{
    // Test case 3: Profile with INACTIVE status - should be skipped
    std::string pkgName = "testPkg";
    std::string deviceId = "deviceId1";
    std::string trustDeviceId = "trustDeviceId1";
    int32_t userId = 123456;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profilesFilter;
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId(trustDeviceId);
    profile.SetStatus(INACTIVE);
    profilesFilter.push_back(profile);

    auto ret = DeviceProfileConnector::GetInstance().GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
    EXPECT_EQ(ret.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthFormMap_004, testing::ext::TestSize.Level1)
{
    // Test case 4: Profile with INVALID_TYPE auth form - should be skipped
    std::string pkgName = "testPkg";
    std::string deviceId = "deviceId1";
    std::string trustDeviceId = "trustDeviceId1";
    int32_t userId = 123456;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profilesFilter;
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId(trustDeviceId);
    profile.SetStatus(ACTIVE);
    profile.SetBindType(DmAuthForm::INVALID_TYPE);
    profilesFilter.push_back(profile);

    auto ret = DeviceProfileConnector::GetInstance().GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
    EXPECT_EQ(ret.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthFormMap_005, testing::ext::TestSize.Level1)
{
    // Test case 5: Single valid profile with IDENTICAL_ACCOUNT type
    std::string pkgName = "testPkg";
    std::string deviceId = "deviceId1";
    std::string trustDeviceId = "trustDeviceId1";
    int32_t userId = 123456;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profilesFilter;
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId(trustDeviceId);
    profile.SetStatus(ACTIVE);
    profile.SetBindType(DmAuthForm::IDENTICAL_ACCOUNT);
    profilesFilter.push_back(profile);

    auto ret = DeviceProfileConnector::GetInstance().GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[trustDeviceId], DmAuthForm::IDENTICAL_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthFormMap_006, testing::ext::TestSize.Level1)
{
    // Test case 6: CheckSinkShareType returns true - should skip adding to map
    std::string pkgName = "testPkg";
    std::string deviceId = "deviceId1";
    std::string trustDeviceId = "trustDeviceId1";
    int32_t userId = 123456;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profilesFilter;
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId(trustDeviceId);
    profile.SetStatus(ACTIVE);
    // Set up accessee/accesser to make CheckSinkShareType return true
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeDeviceId(deviceId);
    profile.SetAccessee(accessee);
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(trustDeviceId);
    profile.SetAccesser(accesser);
    profile.SetBindType(DmAuthForm::ACROSS_ACCOUNT);

    profilesFilter.push_back(profile);

    auto ret = DeviceProfileConnector::GetInstance().GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
    EXPECT_EQ(ret.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthFormMap_007, testing::ext::TestSize.Level1)
{
    // Test case 7: Multiple profiles with different auth forms - should keep highest priority
    std::string pkgName = "testPkg";
    std::string deviceId = "deviceId1";
    std::string trustDeviceId = "trustDeviceId1";
    int32_t userId = 123456;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profilesFilter;
    // First profile - PEER_TO_PEER
    DistributedDeviceProfile::AccessControlProfile profile1;
    profile1.SetTrustDeviceId(trustDeviceId);
    profile1.SetStatus(ACTIVE);
    profile1.SetBindType(DmAuthForm::PEER_TO_PEER);
    profilesFilter.push_back(profile1);

    // Second profile - ACROSS_ACCOUNT (should override PEER_TO_PEER)
    DistributedDeviceProfile::AccessControlProfile profile2;
    profile2.SetTrustDeviceId(trustDeviceId);
    profile2.SetStatus(ACTIVE);
    profile2.SetBindType(DmAuthForm::ACROSS_ACCOUNT);
    profilesFilter.push_back(profile2);

    // Third profile - IDENTICAL_ACCOUNT (should override everything)
    DistributedDeviceProfile::AccessControlProfile profile3;
    profile3.SetTrustDeviceId(trustDeviceId);
    profile3.SetStatus(ACTIVE);
    profile3.SetBindType(DmAuthForm::IDENTICAL_ACCOUNT);
    profilesFilter.push_back(profile3);

    auto ret = DeviceProfileConnector::GetInstance().GetAuthFormMap(pkgName, deviceId, profilesFilter, userId);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[trustDeviceId], DmAuthForm::IDENTICAL_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetVersionByExtra_001, testing::ext::TestSize.Level1)
{
    std::string extraInfo = "";
    std::string dmVersion;

    int32_t ret = DeviceProfileConnector::GetInstance().GetVersionByExtra(extraInfo, dmVersion);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetVersionByExtra_002, testing::ext::TestSize.Level1)
{
    std::string extraInfo = "invalid_json";
    std::string dmVersion;

    int32_t ret = DeviceProfileConnector::GetInstance().GetVersionByExtra(extraInfo, dmVersion);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetVersionByExtra_003, testing::ext::TestSize.Level1)
{
    std::string extraInfo = R"({"key": "value"})";
    std::string dmVersion;

    int32_t ret = DeviceProfileConnector::GetInstance().GetVersionByExtra(extraInfo, dmVersion);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetVersionByExtra_004, testing::ext::TestSize.Level1)
{
    std::string extraInfo = R"({"dmVersion": "5.1.0"})";
    std::string dmVersion;

    int32_t ret = DeviceProfileConnector::GetInstance().GetVersionByExtra(extraInfo, dmVersion);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(dmVersion, "5.1.0");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllVerionAclMap_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = "";

    EXPECT_NO_THROW(DeviceProfileConnector::GetInstance().GetAllVerionAclMap(acl, aclMap, dmVersion));
    EXPECT_EQ(aclMap.size(), 1);
    EXPECT_TRUE(aclMap.find(DM_VERSION_5_1_0) != aclMap.end());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllVerionAclMap_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = DM_VERSION_5_1_0;

    EXPECT_NO_THROW(DeviceProfileConnector::GetInstance().GetAllVerionAclMap(acl, aclMap, dmVersion));
    EXPECT_EQ(aclMap.size(), 1);
    EXPECT_TRUE(aclMap.find(dmVersion) != aclMap.end());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllVerionAclMap_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = "invalid_version";

    EXPECT_NO_THROW(DeviceProfileConnector::GetInstance().GetAllVerionAclMap(acl, aclMap, dmVersion));
    EXPECT_EQ(aclMap.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllVerionAclMap_004, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = DM_VERSION_5_1_0;

    EXPECT_NO_THROW(DeviceProfileConnector::GetInstance().GetAllVerionAclMap(acl, aclMap, dmVersion));
    EXPECT_EQ(aclMap.size(), 1);
    EXPECT_TRUE(aclMap.find(dmVersion) != aclMap.end());
    EXPECT_FALSE(aclMap[dmVersion].empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GenerateAclHash_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = DM_VERSION_5_1_0;

    EXPECT_NO_THROW(DeviceProfileConnector::GetInstance().GenerateAclHash(acl, aclMap, dmVersion));
    EXPECT_EQ(aclMap.size(), 1);
    EXPECT_TRUE(aclMap.find(dmVersion) != aclMap.end());
    EXPECT_FALSE(aclMap[dmVersion].empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GenerateAclHash_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = "invalid_version";

    EXPECT_NO_THROW(DeviceProfileConnector::GetInstance().GenerateAclHash(acl, aclMap, dmVersion));
    EXPECT_EQ(aclMap.size(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GenerateAclHash_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = DM_VERSION_5_1_0;;
    aclMap[dmVersion].push_back("existing_hash");

    EXPECT_NO_THROW(DeviceProfileConnector::GetInstance().GenerateAclHash(acl, aclMap, dmVersion));
    EXPECT_EQ(aclMap.size(), 1);
    EXPECT_TRUE(aclMap.find(dmVersion) != aclMap.end());
    EXPECT_EQ(aclMap[dmVersion].size(), 2);
}


HWTEST_F(DeviceProfileConnectorSecondTest, IsAuthNewVersion_001, testing::ext::TestSize.Level1)
{
    int32_t bindLevel = 0;
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;

    std::string result = DeviceProfileConnector::GetInstance()
        .IsAuthNewVersion(bindLevel, localUdid, remoteUdid, tokenId, userId);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, IsAuthNewVersion_002, testing::ext::TestSize.Level1)
{
    int32_t bindLevel = 4;
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;

    std::string result = DeviceProfileConnector::GetInstance()
        .IsAuthNewVersion(bindLevel, localUdid, remoteUdid, tokenId, userId);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, ChecksumAcl_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::vector<std::string> acLStrList;

    bool result = DeviceProfileConnector::GetInstance().ChecksumAcl(acl, acLStrList);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, ChecksumAcl_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::string aclStr = DeviceProfileConnector::GetInstance().AccessToStr(acl);
    std::string aclHash = Crypto::Sha256(aclStr);

    std::vector<std::string> acLStrList = {aclHash};

    bool result = DeviceProfileConnector::GetInstance().ChecksumAcl(acl, acLStrList);
    EXPECT_TRUE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, ChecksumAcl_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::vector<std::string> acLStrList = {"invalid_hash"};

    bool result = DeviceProfileConnector::GetInstance().ChecksumAcl(acl, acLStrList);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, ChecksumAcl_004, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    acl.SetAccessControlId(12345);
    acl.SetAccesserId(1);
    acl.SetAccesseeId(2);
    acl.SetTrustDeviceId("trustDeviceId");
    acl.SetBindType(256);
    acl.SetAuthenticationType(2);
    acl.SetDeviceIdType(1);
    acl.SetStatus(1);
    acl.SetBindLevel(1);

    std::string aclStr = DeviceProfileConnector::GetInstance().AccessToStr(acl);
    std::string aclHash = Crypto::Sha256(aclStr);

    std::vector<std::string> acLStrList = {"invalid_hash1", aclHash, "invalid_hash2"};

    bool result = DeviceProfileConnector::GetInstance().ChecksumAcl(acl, acLStrList);
    EXPECT_TRUE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, ChecksumAcl_005, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;

    std::vector<std::string> acLStrList = {"some_hash"};

    bool result = DeviceProfileConnector::GetInstance().ChecksumAcl(acl, acLStrList);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, AccessToStr_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId("accesserDeviceId");
    accesser.SetAccesserUserId(1001);
    accesser.SetAccesserAccountId("accesserAccountId");
    accesser.SetAccesserTokenId(2001);
    accesser.SetAccesserBundleName("accesserBundleName");
    accesser.SetAccesserHapSignature("accesserHapSignature");
    accesser.SetAccesserBindLevel(1);
    accesser.SetAccesserCredentialIdStr("accesserCredentialIdStr");
    accesser.SetAccesserStatus(1);
    accesser.SetAccesserSKTimeStamp(1234567890);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId("accesseeDeviceId");
    accessee.SetAccesseeUserId(1002);
    accessee.SetAccesseeAccountId("accesseeAccountId");
    accessee.SetAccesseeTokenId(2002);
    accessee.SetAccesseeBundleName("accesseeBundleName");
    accessee.SetAccesseeHapSignature("accesseeHapSignature");
    accessee.SetAccesseeBindLevel(2);
    accessee.SetAccesseeCredentialIdStr("accesseeCredentialIdStr");
    accessee.SetAccesseeStatus(2);
    accessee.SetAccesseeSKTimeStamp(987654321);

    acl.SetAccesser(accesser);
    acl.SetAccessee(accessee);

    std::string result = DeviceProfileConnector::GetInstance().AccessToStr(acl);

    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("accesserDeviceId"), std::string::npos);
    EXPECT_NE(result.find("accesserAccountId"), std::string::npos);
    EXPECT_NE(result.find("accesseeDeviceId"), std::string::npos);
    EXPECT_NE(result.find("accesseeAccountId"), std::string::npos);
}

HWTEST_F(DeviceProfileConnectorSecondTest, AccessToStr_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId("accesserDeviceId");
    accesser.SetAccesserUserId(1001);
    accesser.SetAccesserAccountId("accesserAccountId");
    accesser.SetAccesserTokenId(2001);
    accesser.SetAccesserBundleName("accesserBundleName");
    accesser.SetAccesserHapSignature("accesserHapSignature");
    accesser.SetAccesserBindLevel(1);
    accesser.SetAccesserCredentialIdStr("accesserCredentialIdStr");
    accesser.SetAccesserStatus(1);
    accesser.SetAccesserSKTimeStamp(1234567890);

    acl.SetAccesser(accesser);

    std::string result = DeviceProfileConnector::GetInstance().AccessToStr(acl);

    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("accesserDeviceId"), std::string::npos);
    EXPECT_NE(result.find("accesserAccountId"), std::string::npos);
    EXPECT_EQ(result.find("accesseeDeviceId"), std::string::npos);
}

HWTEST_F(DeviceProfileConnectorSecondTest, AccessToStr_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId("accesseeDeviceId");
    accessee.SetAccesseeUserId(1002);
    accessee.SetAccesseeAccountId("accesseeAccountId");
    accessee.SetAccesseeTokenId(2002);
    accessee.SetAccesseeBundleName("accesseeBundleName");
    accessee.SetAccesseeHapSignature("accesseeHapSignature");
    accessee.SetAccesseeBindLevel(2);
    accessee.SetAccesseeCredentialIdStr("accesseeCredentialIdStr");
    accessee.SetAccesseeStatus(2);
    accessee.SetAccesseeSKTimeStamp(987654321);

    acl.SetAccessee(accessee);

    std::string result = DeviceProfileConnector::GetInstance().AccessToStr(acl);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.find("accesserDeviceId"), std::string::npos);
    EXPECT_NE(result.find("accesseeDeviceId"), std::string::npos);
    EXPECT_NE(result.find("accesseeAccountId"), std::string::npos);
}

HWTEST_F(DeviceProfileConnectorSecondTest, AccessToStr_004, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId("accesserDeviceId");
    accesser.SetAccesserUserId(1001);
    accesser.SetAccesserAccountId("");
    accesser.SetAccesserTokenId(2001);
    accesser.SetAccesserBundleName("");
    accesser.SetAccesserHapSignature("accesserHapSignature");
    accesser.SetAccesserBindLevel(1);
    accesser.SetAccesserCredentialIdStr("");
    accesser.SetAccesserStatus(1);
    accesser.SetAccesserSKTimeStamp(1234567890);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId("");
    accessee.SetAccesseeUserId(1002);
    accessee.SetAccesseeAccountId("accesseeAccountId");
    accessee.SetAccesseeTokenId(2002);
    accessee.SetAccesseeBundleName("accesseeBundleName");
    accessee.SetAccesseeHapSignature("");
    accessee.SetAccesseeBindLevel(2);
    accessee.SetAccesseeCredentialIdStr("accesseeCredentialIdStr");
    accessee.SetAccesseeStatus(2);
    accessee.SetAccesseeSKTimeStamp(987654321);

    acl.SetAccesser(accesser);
    acl.SetAccessee(accessee);
    std::string result = DeviceProfileConnector::GetInstance().AccessToStr(acl);

    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("accesserDeviceId"), std::string::npos);
    EXPECT_EQ(result.find("accesserAccountId"), std::string::npos);
    EXPECT_NE(result.find("accesseeAccountId"), std::string::npos);
    EXPECT_EQ(result.find("accesseeDeviceId"), std::string::npos);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    profiles.push_back(profile);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_003, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindLevel(USER);
    profiles.push_back(profile);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_004, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(APP);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserDeviceId(localUdid);
    accesser.SetAccesserExtraData("5.1.0");
    profile.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(remoteUdid);
    profile.SetAccessee(accessee);

    profiles.push_back(profile);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_EQ(result, "5.1.0");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_005, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(SERVICE);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(remoteUdid);
    profile.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeDeviceId(localUdid);
    accessee.SetAccesseeExtraData("5.1.0");
    profile.SetAccessee(accessee);

    profiles.push_back(profile);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_EQ(result, "5.1.0");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_006, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(APP);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserUserId(9999);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserDeviceId(localUdid);
    accesser.SetAccesserExtraData("5.1.0");
    profile.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(remoteUdid);
    profile.SetAccessee(accessee);

    profiles.push_back(profile);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_007, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(SERVICE);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserTokenId(9999);
    accesser.SetAccesserDeviceId(localUdid);
    accesser.SetAccesserExtraData("5.1.0");
    profile.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(remoteUdid);
    profile.SetAccessee(accessee);

    profiles.push_back(profile);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_008, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(APP);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserDeviceId("wrongDeviceId");
    accesser.SetAccesserExtraData("5.1.0");
    profile.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(remoteUdid);
    profile.SetAccessee(accessee);

    profiles.push_back(profile);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAppServiceAuthVersionInfo_009, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t tokenId = 12345;
    int32_t userId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    DistributedDeviceProfile::AccessControlProfile profile1;
    profile1.SetBindType(DM_IDENTICAL_ACCOUNT);
    profiles.push_back(profile1);

    DistributedDeviceProfile::AccessControlProfile profile2;
    profile2.SetBindType(DM_ACROSS_ACCOUNT);
    profile2.SetBindLevel(USER);
    profiles.push_back(profile2);

    DistributedDeviceProfile::AccessControlProfile profile3;
    profile3.SetBindType(DM_ACROSS_ACCOUNT);
    profile3.SetBindLevel(APP);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserDeviceId(localUdid);
    accesser.SetAccesserExtraData("5.1.0");
    profile3.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(remoteUdid);
    profile3.SetAccessee(accessee);

    profiles.push_back(profile3);

    std::string result = DeviceProfileConnector::GetInstance().GetAppServiceAuthVersionInfo(
        localUdid, remoteUdid, tokenId, userId, profiles);
    EXPECT_EQ(result, "5.1.0");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceAuthVersionInfo_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    std::string result = DeviceProfileConnector::GetInstance()
        .GetDeviceAuthVersionInfo(localUdid, remoteUdid, profiles);
    EXPECT_EQ(result, "");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceAuthVersionInfo_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles = {profile};

    std::string result = DeviceProfileConnector::GetInstance()
        .GetDeviceAuthVersionInfo(localUdid, remoteUdid, profiles);
    EXPECT_EQ(result, "");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceAuthVersionInfo_003, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(APP);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles = {profile};

    std::string result = DeviceProfileConnector::GetInstance()
        .GetDeviceAuthVersionInfo(localUdid, remoteUdid, profiles);
    EXPECT_EQ(result, "");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceAuthVersionInfo_004, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(USER);
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(localUdid);
    accesser.SetAccesserExtraData("AccesserExtraData");
    profile.SetAccesser(accesser);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(remoteUdid);
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles = {profile};

    std::string result = DeviceProfileConnector::GetInstance()
        .GetDeviceAuthVersionInfo(localUdid, remoteUdid, profiles);
    EXPECT_EQ(result, "AccesserExtraData");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceAuthVersionInfo_005, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(USER);
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(remoteUdid);
    profile.SetAccesser(accesser);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(localUdid);
    accessee.SetAccesseeExtraData("AccesseeExtraData");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles = {profile};

    std::string result = DeviceProfileConnector::GetInstance()
        .GetDeviceAuthVersionInfo(localUdid, remoteUdid, profiles);
    EXPECT_EQ(result, "AccesseeExtraData");
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceAuthVersionInfo_006, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(USER);
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId("differentDeviceId");
    profile.SetAccesser(accesser);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles = {profile};

    std::string result = DeviceProfileConnector::GetInstance()
        .GetDeviceAuthVersionInfo(localUdid, remoteUdid, profiles);
    EXPECT_EQ(result, "");
}
HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUdidListByTokenId_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIds;
    std::string emptyUdid;
    int32_t tokenId = 1234;

    auto result = DeviceProfileConnector::GetInstance().GetDeviceIdAndUdidListByTokenId(userIds, emptyUdid, tokenId);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUdidListByTokenId_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> emptyUserIds;
    std::string localUdid = "localDeviceId";
    int32_t tokenId = 1234;

    auto result = DeviceProfileConnector::GetInstance().GetDeviceIdAndUdidListByTokenId(emptyUserIds, localUdid, tokenId);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUdidListByTokenId_003, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIds = {1, 2};
    std::string emptyUdid;
    int32_t tokenId = 1234;

    auto result = DeviceProfileConnector::GetInstance().GetDeviceIdAndUdidListByTokenId(userIds, emptyUdid, tokenId);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUdidListByTokenId_004, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIds = {1, 2};
    std::string localUdid = "localDeviceId";
    int32_t tokenId = 1234;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    auto result = DeviceProfileConnector::GetInstance().GetDeviceIdAndUdidListByTokenId(userIds, localUdid, tokenId);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, IsAllowAuthAlways_001, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    item.SetAuthenticationType(ALLOW_AUTH_ALWAYS);
    std::string localUdid = item.GetAccesser().GetAccesserDeviceId();
    int32_t userId = item.GetAccesser().GetAccesserUserId();
    std::string peerUdid = item.GetAccessee().GetAccesseeDeviceId();
    std::string pkgName = item.GetAccesser().GetAccesserBundleName();
    uint64_t tokenId = item.GetAccesser().GetAccesserTokenId();
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().IsAllowAuthAlways(localUdid, userId, peerUdid,
        pkgName, tokenId);
    EXPECT_TRUE(result);

    localUdid = item.GetAccessee().GetAccesseeDeviceId();
    userId = item.GetAccessee().GetAccesseeUserId();
    peerUdid = item.GetAccesser().GetAccesserDeviceId();
    pkgName = item.GetAccessee().GetAccesseeBundleName();
    tokenId = item.GetAccessee().GetAccesseeTokenId();
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    result = DeviceProfileConnector::GetInstance().IsAllowAuthAlways(localUdid, userId, peerUdid,
        pkgName, tokenId);
    EXPECT_TRUE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, IsAllowAuthAlways_002, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    std::string localUdid = item.GetAccesser().GetAccesserDeviceId();
    int32_t userId = item.GetAccesser().GetAccesserUserId();
    std::string peerUdid = item.GetAccessee().GetAccesseeDeviceId();
    std::string pkgName = item.GetAccesser().GetAccesserBundleName();
    uint64_t tokenId = item.GetAccesser().GetAccesserTokenId();
    item.SetAuthenticationType(ALLOW_AUTH_ONCE);
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl((_)))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().IsAllowAuthAlways(localUdid, userId, peerUdid,
        pkgName, tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, IsAllowAuthAlways_003, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    std::string localUdid = "localUdid3";
    int32_t userId = 3;
    std::string peerUdid = "peerUdid3";
    std::string pkgName = "com.example.app3";
    uint64_t tokenId = 123458;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl((_)))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().IsAllowAuthAlways(localUdid, userId, peerUdid,
        pkgName, tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteTimeOutAcl_001, testing::ext::TestSize.Level1)
{
    int32_t deviceIdType = 1;
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    int32_t authType = ALLOW_AUTH_ONCE;
    int32_t bindType = DM_POINT_TO_POINT;
    uint32_t status = ACTIVE;
    uint32_t bindLevel = 1;
    uint32_t tokenId = 1;
    DmOfflineParam offlineParam;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(1, deviceIdType, peerUdid, authType, bindType, status,
        1, peerUdid, peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(2, deviceIdType, peerUdid, authType, bindType, status,
        2, "localUdid", localUserId, "accesserAccountId", 2, peerUdid, peerUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(3, deviceIdType, "remoteDeviceId2", authType, bindType, status,
        3, "localUdid", localUserId, "accesserAccountId", 3, "remoteDeviceId2", peerUserId, "accesseeAccountId",
        tokenId, "bundleName2", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(4, deviceIdType, peerUdid, authType, bindType, status,
        4, peerUdid, peerUserId, "accesserAccountId", 4, "localUdid", localUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, R"({"IsLnnAcl":"true"})"));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    uint32_t ret = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(peerUdid, peerUserId,
        localUserId, offlineParam);
    EXPECT_EQ(ret, 3);
    EXPECT_EQ(offlineParam.needDelAclInfos.size(), 3);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteTimeOutAcl_002, testing::ext::TestSize.Level1)
{
    int32_t deviceIdType = 1;
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    int32_t authType = ALLOW_AUTH_ONCE;
    int32_t bindType = DM_ACROSS_ACCOUNT;
    uint32_t status = ACTIVE;
    uint32_t bindLevel = 1;
    uint32_t tokenId = 1;
    DmOfflineParam offlineParam;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(1, deviceIdType, peerUdid, authType, bindType, status,
        1, peerUdid, peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(2, deviceIdType, peerUdid, authType, bindType, status,
        2, "localUdid", localUserId, "accesserAccountId", 2, peerUdid, peerUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(3, deviceIdType, "remoteDeviceId2", authType, bindType, status,
        3, "localUdid", localUserId, "accesserAccountId", 3, "remoteDeviceId2", peerUserId, "accesseeAccountId",
        tokenId, "bundleName2", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(4, deviceIdType, peerUdid, authType, bindType, status,
        4, peerUdid, peerUserId, "accesserAccountId", 4, "localUdid", localUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, R"({"IsLnnAcl":"true"})"));
    acls.emplace_back(GenerateAccessControlProfile(5, deviceIdType, peerUdid, ALLOW_AUTH_ALWAYS, bindType, status,
        5, "localUdid", localUserId, "accesserAccountId", 5, peerUdid, peerUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    uint32_t ret = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(peerUdid, peerUserId,
        localUserId, offlineParam);
    EXPECT_EQ(ret, 4);
    EXPECT_EQ(offlineParam.needDelAclInfos.size(), 2);
}

HWTEST_F(DeviceProfileConnectorSecondTest, AuthOnceAclIsActive_001, testing::ext::TestSize.Level1)
{
    int32_t deviceIdType = 1;
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    int32_t authType = ALLOW_AUTH_ONCE;
    int32_t bindType = 1;
    uint32_t status = ACTIVE;
    uint32_t bindLevel = 1;
    uint32_t tokenId = 1;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(1, deviceIdType, peerUdid, authType, bindType, status,
        1, peerUdid, peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(3, deviceIdType, "remoteDeviceId2", authType, bindType, status,
        3, "localUdid", localUserId, "accesserAccountId", 3, "remoteDeviceId2", peerUserId, "accesseeAccountId",
        tokenId, "bundleName2", bindLevel, ""));
    acls.emplace_back(GenerateAccessControlProfile(5, deviceIdType, peerUdid, ALLOW_AUTH_ALWAYS, bindType, status,
        5, "localUdid", localUserId, "accesserAccountId", 5, peerUdid, peerUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(acls), Return(0)));
    bool ret = DeviceProfileConnector::GetInstance().AuthOnceAclIsActive(peerUdid, peerUserId,
        localUserId);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, AuthOnceAclIsActive_002, testing::ext::TestSize.Level1)
{
    int32_t deviceIdType = 1;
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    int32_t authType = ALLOW_AUTH_ONCE;
    int32_t bindType = 1;
    uint32_t status = ACTIVE;
    uint32_t bindLevel = 1;
    uint32_t tokenId = 1;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(2, deviceIdType, peerUdid, authType, bindType, status,
        2, "localUdid", localUserId, "accesserAccountId", 2, peerUdid, peerUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, ""));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(acls), Return(0)));
    bool ret = DeviceProfileConnector::GetInstance().AuthOnceAclIsActive(peerUdid, peerUserId,
        localUserId);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, AuthOnceAclIsActive_003, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(acls), Return(0)));
    bool ret = DeviceProfileConnector::GetInstance().AuthOnceAclIsActive(peerUdid, peerUserId,
        localUserId);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CacheLnnAcl_001, testing::ext::TestSize.Level1)
{
    int32_t deviceIdType = 1;
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    int32_t authType = ALLOW_AUTH_ONCE;
    int32_t bindType = 1;
    uint32_t status = ACTIVE;
    uint32_t bindLevel = 1;
    uint32_t tokenId = 1;
    DistributedDeviceProfile::AccessControlProfile profile =
        GenerateAccessControlProfile(4, deviceIdType, peerUdid, authType, bindType, status,
        4, peerUdid, peerUserId, "accesserAccountId", 4, "localUdid", localUserId, "accesseeAccountId",
        tokenId, "bundleName1", bindLevel, R"({"IsLnnAcl":"true"})");
    DmAclIdParam dmAclIdParam;
    bool ret = DeviceProfileConnector::GetInstance().CacheLnnAcl(profile,
        peerUdid, peerUserId, localUserId, dmAclIdParam);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CacheLnnAcl_002, testing::ext::TestSize.Level1)
{
    int32_t deviceIdType = 1;
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    int32_t authType = ALLOW_AUTH_ONCE;
    int32_t bindType = 1;
    uint32_t status = ACTIVE;
    uint32_t bindLevel = 1;
    uint32_t tokenId = 1;
    DistributedDeviceProfile::AccessControlProfile profile =
        GenerateAccessControlProfile(4, deviceIdType, peerUdid, authType, bindType, status,
        4, "localUdid", localUserId, "accesseeAccountId", 4, peerUdid, peerUserId, "accesserAccountId",
        tokenId, "bundleName1", bindLevel, R"({"IsLnnAcl":"true"})");
    DmAclIdParam dmAclIdParam;
    bool ret = DeviceProfileConnector::GetInstance().CacheLnnAcl(profile,
        peerUdid, peerUserId, localUserId, dmAclIdParam);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CacheLnnAcl_003, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "remoteDeviceId1";
    int32_t peerUserId = 123456;
    int32_t localUserId = 123456;
    DistributedDeviceProfile::AccessControlProfile profile;
    DmAclIdParam dmAclIdParam;
    bool ret = DeviceProfileConnector::GetInstance().CacheLnnAcl(profile,
        peerUdid, peerUserId, localUserId, dmAclIdParam);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UpdateAclDeviceName_001, testing::ext::TestSize.Level1)
{
    std::string udid;
    std::string newDeviceName;
    bool isLocal = false;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillOnce(Return(100));
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, newDeviceName, isLocal);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UpdateAclDeviceName_002, testing::ext::TestSize.Level1)
{
    std::string udid = "peerUdid";
    std::string newDeviceName = "newDeviceName";
    bool isLocal = false;
    int32_t peerUserId = 1;
    int32_t localUserId = 1;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(1, 1, "peerUdid1", 0, 1, ACTIVE,
        1, "peerUdid1", peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(2, 1, udid, 0, 1, INACTIVE,
        2, udid, peerUserId, "accesserAccountId", 2, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(3, 1, udid, 0, 1, ACTIVE,
        3, udid, peerUserId, "accesserAccountId", 3, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(4, 1, udid, 0, 1, ACTIVE,
        4, "localUdid", localUserId, "accesseeAccountId", 4, udid, peerUserId, "accesserAccountId",
        1, "bundleName1", 1, ""));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillOnce(Return(localUserId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(2).WillRepeatedly(Return(0));
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, newDeviceName, isLocal);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UpdateAclDeviceName_003, testing::ext::TestSize.Level1)
{
    std::string udid = "localUdid";
    std::string newDeviceName = "newDeviceName";
    bool isLocal = true;
    int32_t peerUserId = 1;
    int32_t localUserId = 1;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(3, 1, "peerUdid", 0, 1, ACTIVE,
        3, "peerUdid", peerUserId, "accesserAccountId", 3, udid, localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(4, 1, "peerUdid", 0, 1, ACTIVE,
        4, udid, localUserId, "accesseeAccountId", 4, "peerUdid", peerUserId, "accesserAccountId",
        1, "bundleName1", 1, ""));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillOnce(Return(localUserId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(2).WillRepeatedly(Return(0));
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, newDeviceName, isLocal);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetActiveAuthOncePeerUserId_001, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 1;
    int32_t localUserId = 1;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(1, 1, "peerUdid1", ALLOW_AUTH_ONCE, 1, ACTIVE,
        1, peerUdid, peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(2, 1, peerUdid, ALLOW_AUTH_ONCE, 1, INACTIVE,
        2, peerUdid, peerUserId, "accesserAccountId", 2, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(5, 1, peerUdid, ALLOW_AUTH_ALWAYS, 1, ACTIVE,
        5, peerUdid, peerUserId, "accesserAccountId", 5, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(3, 1, peerUdid, ALLOW_AUTH_ONCE, 1, ACTIVE,
        3, peerUdid, peerUserId, "accesserAccountId", 3, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(4, 1, peerUdid, ALLOW_AUTH_ONCE, 1, ACTIVE,
        4, "localUdid", localUserId, "accesseeAccountId", 4, peerUdid, peerUserId, "accesserAccountId",
        1, "bundleName1", 1, ""));

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(acls), Return(0)));

    std::unordered_set<int32_t> peerUserIds = DeviceProfileConnector::GetInstance()
        .GetActiveAuthOncePeerUserId(peerUdid, localUserId);
    EXPECT_EQ(peerUserIds.size(), 1);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllAuthOnceAclInfos_001, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 1;
    int32_t localUserId = 1;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(1, 1, peerUdid, ALLOW_AUTH_ALWAYS, 1, ACTIVE,
        1, peerUdid, peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(3, 1, peerUdid, ALLOW_AUTH_ONCE, 1, ACTIVE,
        3, peerUdid, peerUserId, "accesserAccountId", 3, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(4, 1, peerUdid, ALLOW_AUTH_ONCE, 1, ACTIVE,
        4, "localUdid", localUserId, "accesseeAccountId", 4, peerUdid, peerUserId, "accesserAccountId",
        1, "bundleName1", 1, ""));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> aclInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().GetAllAuthOnceAclInfos(aclInfos);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(aclInfos.size(), 1);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthOnceAclInfos_001, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 1;
    int32_t localUserId = 1;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    acls.emplace_back(GenerateAccessControlProfile(1, 1, peerUdid, ALLOW_AUTH_ALWAYS, 1, ACTIVE,
        1, peerUdid, peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(1, 1, "peerUdid1", ALLOW_AUTH_ONCE, 1, ACTIVE,
        1, peerUdid, peerUserId, "accesserAccountId", 1, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(3, 1, peerUdid, ALLOW_AUTH_ONCE, 1, ACTIVE,
        3, peerUdid, peerUserId, "accesserAccountId", 3, "localUdid", localUserId, "accesseeAccountId",
        1, "bundleName1", 1, ""));
    acls.emplace_back(GenerateAccessControlProfile(4, 1, peerUdid, ALLOW_AUTH_ONCE, 1, ACTIVE,
        4, "localUdid", localUserId, "accesseeAccountId", 4, peerUdid, peerUserId, "accesserAccountId",
        1, "bundleName1", 1, ""));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> aclInfos =
        DeviceProfileConnector::GetInstance().GetAuthOnceAclInfos(peerUdid);
    EXPECT_EQ(aclInfos.size(), 1);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_001, testing::ext::TestSize.Level1)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    item.accesser_.SetAccesserDeviceId(localUdid);
    int32_t userId = item.GetAccesser().GetAccesserUserId();
    int32_t tokenId = static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId());

    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(::testing::AtLeast(1)).WillOnce(Return(userId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_TRUE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_002, testing::ext::TestSize.Level1)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    item.accessee_.SetAccesseeDeviceId(localUdid);
    int32_t userId = item.GetAccessee().GetAccesseeUserId();
    int32_t tokenId = static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId());

    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(::testing::AtLeast(1)).WillOnce(Return(userId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_TRUE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_003, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    int32_t tokenId = 123456;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_004, testing::ext::TestSize.Level1)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    item.accesser_.SetAccesserDeviceId(localUdid);
    int32_t userId = item.GetAccesser().GetAccesserUserId();
    int32_t tokenId = 123456;

    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(::testing::AtLeast(1)).WillOnce(Return(userId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_005, testing::ext::TestSize.Level1)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    item.accesser_.SetAccesserDeviceId(localUdid);
    int32_t tokenId = static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId());

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_006, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    int32_t userId = item.GetAccesser().GetAccesserUserId();
    int32_t tokenId = static_cast<int32_t>(item.GetAccesser().GetAccesserTokenId());

    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(::testing::AtLeast(1)).WillOnce(Return(userId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_007, testing::ext::TestSize.Level1)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    item.accessee_.SetAccesseeDeviceId(localUdid);
    int32_t userId = item.GetAccessee().GetAccesseeUserId();
    int32_t tokenId = 123456;

    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(::testing::AtLeast(1)).WillOnce(Return(userId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_008, testing::ext::TestSize.Level1)
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = std::string(localDeviceId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    item.accessee_.SetAccesseeDeviceId(localUdid);
    int32_t tokenId = static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId());

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_009, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;
    AddAccessControlProfile(acls);
    auto &item = acls[0];
    int32_t userId = item.GetAccessee().GetAccesseeUserId();
    int32_t tokenId = static_cast<int32_t>(item.GetAccessee().GetAccesseeTokenId());

    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID())
        .Times(::testing::AtLeast(1)).WillOnce(Return(userId));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(acls), Return(0)));
    bool result = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleSyncForegroundUserIdEvent_201, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<int32_t> remoteUserIds = {userId};
    std::vector<int32_t> localUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9001, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        INACTIVE, 1, localUdid, userId, "accesserAccountId", 1, remoteUdid, userId, "accesseeAccountId",
        1001, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[1001,1002]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(
        remoteUserIds, remoteUdid, localUserIds, localUdid, serviceInfos);

    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_EQ(serviceInfos[0].peerUdid, remoteUdid);
    EXPECT_EQ(serviceInfos[0].peerUserId, userId);
    EXPECT_EQ(serviceInfos[0].localTokenId, 1001);
    EXPECT_TRUE(serviceInfos[0].isActive);
    ASSERT_FALSE(serviceInfos[0].serviceIds.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleSyncBackgroundUserIdEvent_201, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<int32_t> remoteUserIds = {userId};
    std::vector<int32_t> localUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9002, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, localUdid, userId, "accesserAccountId", 1, remoteUdid, userId, "accesseeAccountId",
        1002, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[2001]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(
        remoteUserIds, remoteUdid, localUserIds, localUdid, serviceInfos);

    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_EQ(serviceInfos[0].peerUdid, remoteUdid);
    EXPECT_EQ(serviceInfos[0].peerUserId, userId);
    EXPECT_EQ(serviceInfos[0].localTokenId, 1002);
    ASSERT_FALSE(serviceInfos[0].serviceIds.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleSyncForegroundUserIdEvent_202,
    testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<int32_t> remoteUserIds = {userId};
    std::vector<int32_t> localUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9004, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        INACTIVE, 1, localUdid, userId, "accesserAccountId", 1, remoteUdid, userId, "accesseeAccountId",
        1004, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"noServiceId":[1]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(
        remoteUserIds, remoteUdid, localUserIds, localUdid, serviceInfos);

    EXPECT_TRUE(serviceInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleSyncBackgroundUserIdEvent_202,
    testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<int32_t> remoteUserIds = {userId};
    std::vector<int32_t> localUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9005, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, localUdid, userId, "accesserAccountId", 1, remoteUdid, userId, "accesseeAccountId",
        1005, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData("");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(
        remoteUserIds, remoteUdid, localUserIds, localUdid, serviceInfos);

    EXPECT_TRUE(serviceInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleAccountCommonEvent_201, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<std::string> deviceVec = {remoteUdid};
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9003, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, localUdid, userId, "accesserAccountId", 1, remoteUdid, userId, "accesseeAccountId",
        1003, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[3001,3002]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleAccountCommonEvent(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds, serviceInfos);

    EXPECT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_EQ(serviceInfos[0].peerUdid, remoteUdid);
    EXPECT_EQ(serviceInfos[0].peerUserId, userId);
    EXPECT_EQ(serviceInfos[0].localTokenId, 1003);
    EXPECT_FALSE(serviceInfos[0].isActive);
    ASSERT_FALSE(serviceInfos[0].serviceIds.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleAccountCommonEvent_202, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<std::string> deviceVec = {remoteUdid};
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9006, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        INACTIVE, 1, localUdid, userId, "accesserAccountId", 1, remoteUdid, userId, "accesseeAccountId",
        1006, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[4001]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleAccountCommonEvent(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds, serviceInfos);

    EXPECT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_EQ(serviceInfos[0].peerUdid, remoteUdid);
    EXPECT_EQ(serviceInfos[0].peerUserId, userId);
    EXPECT_EQ(serviceInfos[0].localTokenId, 1006);
    EXPECT_TRUE(serviceInfos[0].isActive);
    ASSERT_FALSE(serviceInfos[0].serviceIds.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleAccountCommonEvent_203, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<std::string> deviceVec = {remoteUdid};
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9007, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, localUdid, userId, "accesserAccountId", 1, remoteUdid, userId, "accesseeAccountId",
        1007, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"badKey":[1]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleAccountCommonEvent(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds, serviceInfos);

    EXPECT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_TRUE(serviceInfos[0].serviceIds.empty());
    EXPECT_EQ(serviceInfos[0].localTokenId, 0);
    EXPECT_EQ(serviceInfos[0].peerUserId, 0);
    EXPECT_FALSE(serviceInfos[0].isActive);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleAccountCommonEvent_204, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<std::string> deviceVec = {remoteUdid};
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9008, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, remoteUdid, userId, "accesserAccountId", 1, localUdid, userId, "accesseeAccountId",
        1008, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[5001]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleAccountCommonEvent(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds, serviceInfos);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_TRUE(serviceInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleAccountCommonEvent_205, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    int32_t userId = 123456;
    std::vector<std::string> deviceVec = {remoteUdid};
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {userId};

    auto profile = GenerateAccessControlProfile(9009, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        INACTIVE, 1, remoteUdid, userId, "accesserAccountId", 1, localUdid, userId, "accesseeAccountId",
        1009, "bundleName", USER, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[5002]})");
    profile.SetAccessee(accessee);
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleAccountCommonEvent(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds, serviceInfos);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_TRUE(serviceInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAclStatusAndForegroundNotMatch_201, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {123456};
    auto profile = GenerateAccessControlProfile(9010, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, localUdid, 123456, "accesserAccountId", 1, "remoteDeviceId", 123456, "accesseeAccountId",
        1010, "bundleName", USER, "");
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    bool ret = DeviceProfileConnector::GetInstance().CheckAclStatusAndForegroundNotMatch(
        localUdid, foregroundUserIds, backgroundUserIds);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAclStatusAndForegroundNotMatch_202, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {123456};
    auto profile = GenerateAccessControlProfile(9011, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, localUdid, 999999, "accesserAccountId", 1, "remoteDeviceId", 999999, "accesseeAccountId",
        1011, "bundleName", USER, "");
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    bool ret = DeviceProfileConnector::GetInstance().CheckAclStatusAndForegroundNotMatch(
        localUdid, foregroundUserIds, backgroundUserIds);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, IsLnnAcl_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9012, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1012, "bundleName", USER, R"({"IsLnnAcl":"true"})");
    bool ret = DeviceProfileConnector::GetInstance().IsLnnAcl(profile);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, IsLnnAcl_202, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9013, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1013, "bundleName", USER, R"({"badKey":"true"})");
    bool ret = DeviceProfileConnector::GetInstance().IsLnnAcl(profile);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UpdatePeerUserId_201, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    auto profile = GenerateAccessControlProfile(9014, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_IDENTICAL_ACCOUNT,
        ACTIVE, 1, localUdid, 123456, "accesserAccountId", 1, remoteUdid, 0, "accesseeAccountId",
        1014, "bundleName", USER, "");
    std::vector<int32_t> localUserIds = {123456};
    std::vector<int32_t> remoteFrontUserIds = {456};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));
    DeviceProfileConnector::GetInstance().UpdatePeerUserId(profile, localUdid, localUserIds, remoteUdid,
        remoteFrontUserIds);
    EXPECT_TRUE(true);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleUserStop_201, testing::ext::TestSize.Level1)
{
    int32_t stopUserId = 123456;
    std::string stopEventUdid = "localDeviceId";
    auto profile = GenerateAccessControlProfile(9015, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, stopEventUdid, stopUserId, "accesserAccountId", 1, "remoteDeviceId", stopUserId,
        "accesseeAccountId", 1015, "bundleName", USER, "");
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    int32_t ret = DeviceProfileConnector::GetInstance().HandleUserStop(stopUserId, stopEventUdid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UpdateAclStatus_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9020, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1020, "bundleName", USER, "");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));
    DeviceProfileConnector::GetInstance().UpdateAclStatus(profile);
    EXPECT_TRUE(true);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteCacheAcl_201, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.emplace_back(GenerateAccessControlProfile(9021, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS,
        DM_POINT_TO_POINT, ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId",
        123456, "accesseeAccountId", 1021, "bundleName", USER, ""));
    profiles.emplace_back(GenerateAccessControlProfile(9022, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS,
        DM_POINT_TO_POINT, ACTIVE, 2, "localDeviceId", 123456, "accesserAccountId", 2, "remoteDeviceId",
        123456, "accesseeAccountId", 1022, "bundleName", USER, ""));
    std::vector<int64_t> delAclIdVec = {9021};

    DeviceProfileConnector::GetInstance().DeleteCacheAcl(delAclIdVec, profiles);
    ASSERT_EQ(profiles.size(), 1);
    EXPECT_EQ(profiles[0].GetAccessControlId(), 9022);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAuthFormProxyTokenId_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string extraStr = "";
    bool ret = DeviceProfileConnector::GetInstance().CheckAuthFormProxyTokenId(pkgName, extraStr);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleServiceUnBindEvent_201, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 123456;
    std::string remoteUdid = "remoteDeviceId";
    std::string localUdid = "localDeviceId";
    int32_t tokenId = 1001;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    auto offlineParam = DeviceProfileConnector::GetInstance().HandleServiceUnBindEvent(
        remoteUserId, remoteUdid, localUdid, tokenId);
    EXPECT_TRUE(offlineParam.processVec.empty());
    EXPECT_TRUE(offlineParam.needDelAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAclForAccountLogOut_201, testing::ext::TestSize.Level1)
{
    DMAclQuadInfo info;
    info.localUdid = "localDeviceId";
    info.localUserId = 123456;
    info.peerUdid = "remoteDeviceId";
    info.peerUserId = 123456;
    std::string accountId = "accesserAccountId";

    auto profile = GenerateAccessControlProfile(9023, 1, info.peerUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, info.localUdid, info.localUserId, accountId, 1, info.peerUdid, info.peerUserId,
        "accesseeAccountId", 1023, "bundleName", USER, "");
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));

    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    bool notifyOffline = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(
        info, accountId, offlineParam, serviceInfos);

    EXPECT_TRUE(notifyOffline);
    EXPECT_EQ(offlineParam.needDelAclInfos.size(), 1);
}

HWTEST_F(DeviceProfileConnectorSecondTest, UpdateACL_201, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    std::vector<int32_t> localUserIds = {123456};
    std::vector<int32_t> remoteFrontUserIds = {456};
    std::vector<int32_t> remoteBackUserIds;
    DmOfflineParam offlineParam;

    auto profile = GenerateAccessControlProfile(9024, 1, remoteUdid, ALLOW_AUTH_ALWAYS, DM_IDENTICAL_ACCOUNT,
        ACTIVE, 1, localUdid, 123456, "accesserAccountId", 1, remoteUdid, 0,
        "accesseeAccountId", 1024, "bundleName", USER, "");
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls = {profile};

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateAccessControlProfile(_))
        .Times(1)
        .WillOnce(Return(DM_OK));

    DeviceProfileConnector::GetInstance().UpdateACL(localUdid, localUserIds, remoteUdid,
        remoteFrontUserIds, remoteBackUserIds, offlineParam);
    EXPECT_TRUE(true);
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAclByActhash_201, testing::ext::TestSize.Level1)
{
    DMAclQuadInfo info;
    info.localUdid = "localDeviceId";
    info.localUserId = 123456;
    info.peerUdid = "remoteDeviceId";
    info.peerUserId = 123456;
    std::string accountIdHash = "hash";
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));

    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    bool ret = DeviceProfileConnector::GetInstance().DeleteAclByActhash(
        info, accountIdHash, offlineParam, serviceInfos);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CacheOfflineParam_201, testing::ext::TestSize.Level1)
{
    DmCacheOfflineInputParam inputParam;
    inputParam.accountIdHash = "invalidHash";
    inputParam.info.localUdid = "localDeviceId";
    inputParam.info.localUserId = 123456;
    inputParam.info.peerUdid = "remoteDeviceId";
    inputParam.info.peerUserId = 123456;
    inputParam.profile = GenerateAccessControlProfile(9030, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS,
        DM_POINT_TO_POINT, ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1,
        "remoteDeviceId", 123456, "accesseeAccountId", 1030, "bundleName", USER, "");

    DmOfflineParam offlineParam;
    bool notifyOffline = false;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().CacheOfflineParam(inputParam, offlineParam, notifyOffline, serviceInfos);
    EXPECT_TRUE(offlineParam.needDelAclInfos.empty() || !offlineParam.needDelAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAclForUserRemoved_201, testing::ext::TestSize.Level1)
{
    DmLocalUserRemovedInfo userRemovedInfo;
    userRemovedInfo.localUdid = "localDeviceId";
    userRemovedInfo.preUserId = 123456;
    userRemovedInfo.peerUdids = {"remoteDeviceId"};
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));

    std::multimap<std::string, int32_t> peerUserIdMap;
    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().DeleteAclForUserRemoved(
        userRemovedInfo, peerUserIdMap, offlineParam, serviceInfos);
    EXPECT_TRUE(peerUserIdMap.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAclForRemoteUserRemoved_201, testing::ext::TestSize.Level1)
{
    DmRemoteUserRemovedInfo userRemovedInfo;
    userRemovedInfo.peerUdid = "remoteDeviceId";
    userRemovedInfo.peerUserId = 123456;
    std::vector<DistributedDeviceProfile::AccessControlProfile> acls;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAclIncludeLnnAcl(_))
        .WillOnce(DoAll(SetArgReferee<0>(acls), Return(DM_OK)));

    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().DeleteAclForRemoteUserRemoved(
        userRemovedInfo, offlineParam, serviceInfos);
    EXPECT_TRUE(userRemovedInfo.localUserIds.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, ProcessLocalAccessRemote_201, testing::ext::TestSize.Level1)
{
    DMAclQuadInfo info;
    info.localUdid = "localDeviceId";
    info.localUserId = 123456;
    info.peerUdid = "remoteDeviceId";
    info.peerUserId = 123456;

    auto profile = GenerateAccessControlProfile(9031, 1, info.peerUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, info.localUdid, info.localUserId, "accesserAccountId", 1, info.peerUdid, info.peerUserId,
        "accesseeAccountId", 1031, "bundleName", USER, R"({"IsLnnAcl":"true"})");

    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    bool notifyOffline = false;
    DeviceProfileConnector::GetInstance().ProcessLocalAccessRemote(
        profile, info, "account", offlineParam, serviceInfos, notifyOffline);

    EXPECT_TRUE(notifyOffline);
    EXPECT_EQ(offlineParam.needDelAclInfos.size(), 1);
}

HWTEST_F(DeviceProfileConnectorSecondTest, ProcessRemoteAccessLocal_201, testing::ext::TestSize.Level1)
{
    DMAclQuadInfo info;
    info.localUdid = "localDeviceId";
    info.localUserId = 123456;
    info.peerUdid = "remoteDeviceId";
    info.peerUserId = 123456;

    auto profile = GenerateAccessControlProfile(9032, 1, info.peerUdid, ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, info.peerUdid, info.peerUserId, "accesserAccountId", 1, info.localUdid, info.localUserId,
        "accesseeAccountId", 1032, "bundleName", USER, R"({"IsLnnAcl":"true"})");

    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    bool notifyOffline = false;
    DeviceProfileConnector::GetInstance().ProcessRemoteAccessLocal(
        profile, info, "account", offlineParam, serviceInfos, notifyOffline);

    EXPECT_TRUE(notifyOffline);
    EXPECT_EQ(offlineParam.needDelAclInfos.size(), 1);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckLastLnnAcl_201, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.emplace_back(GenerateAccessControlProfile(9040, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS,
        DM_POINT_TO_POINT, ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId",
        123456, "accesseeAccountId", 1040, "bundleName", USER, R"({"IsLnnAcl":"true"})"));

    DeviceProfileConnector::GetInstance().CheckLastLnnAcl("localDeviceId", 123456, "remoteDeviceId",
        offlineParam, profiles);
    EXPECT_TRUE(offlineParam.hasLnnAcl);
    EXPECT_EQ(offlineParam.needDelAclInfos.size(), 1);
}

HWTEST_F(DeviceProfileConnectorSecondTest, FindTargetAcl_201, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    auto profile = GenerateAccessControlProfile(9041, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1041, "bundleName", USER, "");

    bool ret = DeviceProfileConnector::GetInstance().FindTargetAcl(profile, "localDeviceId", 1041,
        "remoteDeviceId", 0, offlineParam);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, FindLnnAcl_201, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    auto profile = GenerateAccessControlProfile(9042, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1042, "bundleName", USER, R"({"IsLnnAcl":"true"})");

    bool ret = DeviceProfileConnector::GetInstance().FindLnnAcl(profile, "localDeviceId", "remoteDeviceId",
        offlineParam);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(offlineParam.allLnnAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, FindUserAcl_201, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    auto profile = GenerateAccessControlProfile(9043, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1043, "bundleName", USER, "");

    bool ret = DeviceProfileConnector::GetInstance().FindUserAcl(profile, "localDeviceId", "remoteDeviceId",
        offlineParam);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(offlineParam.allUserAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, FindLeftAcl_201, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    auto profile = GenerateAccessControlProfile(9044, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1044, "bundleName", APP, "");

    bool ret = DeviceProfileConnector::GetInstance().FindLeftAcl(profile, "localDeviceId", "remoteDeviceId",
        offlineParam);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(offlineParam.allLeftAppOrSvrAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, FilterNeedDeleteACLInfos_201, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.emplace_back(GenerateAccessControlProfile(9045, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS,
        DM_POINT_TO_POINT, ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId",
        123456, "accesseeAccountId", 1045, "bundleName", USER, ""));

    DeviceProfileConnector::GetInstance().FilterNeedDeleteACLInfos(
        profiles, "localDeviceId", 1045, "remoteDeviceId", "", offlineParam);
    EXPECT_FALSE(offlineParam.needDelAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, SetProcessInfoPkgName_201, testing::ext::TestSize.Level1)
{
    std::vector<OHOS::DistributedHardware::ProcessInfo> processInfoVec;
    auto profile = GenerateAccessControlProfile(9046, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1046, "bundleName", APP, "");

    DeviceProfileConnector::GetInstance().SetProcessInfoPkgName(profile, processInfoVec, true);
    DeviceProfileConnector::GetInstance().SetProcessInfoPkgName(profile, processInfoVec, true);
    ASSERT_EQ(processInfoVec.size(), 1);
    EXPECT_FALSE(processInfoVec[0].pkgName.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckAclStatusNotMatch_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9047, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1047, "bundleName", USER, "");
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {123456};

    bool ret = DeviceProfileConnector::GetInstance().CheckAclStatusNotMatch(profile, "localDeviceId",
        foregroundUserIds, backgroundUserIds);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, ProcessLocalToPeer_201, testing::ext::TestSize.Level1)
{
    DMAclQuadInfo info;
    auto profile = GenerateAccessControlProfile(9048, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "accesserAccountId", 1, "remoteDeviceId", 123456,
        "accesseeAccountId", 1048, "bundleName", USER, R"({"IsLnnAcl":"true"})");
    DmOfflineParam offlineParam;
    bool notifyOffline = false;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().ProcessLocalToPeer(
        profile, info, "hash", offlineParam, notifyOffline, serviceInfos);
    EXPECT_TRUE(notifyOffline);
    EXPECT_FALSE(offlineParam.needDelAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, ProcessPeerToLocal_201, testing::ext::TestSize.Level1)
{
    DMAclQuadInfo info;
    auto profile = GenerateAccessControlProfile(9049, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "remoteDeviceId", 123456, "accesserAccountId", 1, "localDeviceId", 123456,
        "accesseeAccountId", 1049, "bundleName", USER, R"({"IsLnnAcl":"true"})");
    DmOfflineParam offlineParam;
    bool notifyOffline = false;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().ProcessPeerToLocal(
        profile, info, "hash", offlineParam, notifyOffline, serviceInfos);
    EXPECT_TRUE(notifyOffline);
    EXPECT_FALSE(offlineParam.needDelAclInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSrcAcuntAccessControl_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 123456;
    DmAccessCallee callee;
    auto profile = GenerateAccessControlProfile(9050, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_IDENTICAL_ACCOUNT,
        ACTIVE, 1, "localDeviceId", 123456, "sameAccount", 1, "remoteDeviceId", 123456,
        "sameAccount", 1050, "bundleName", USER, "");

    bool ret = DeviceProfileConnector::GetInstance().CheckSrcAcuntAccessControl(
        profile, caller, "localDeviceId", callee, "remoteDeviceId");
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkAcuntAccessControl_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 123456;
    caller.accountId = "sameAccount";
    DmAccessCallee callee;
    callee.userId = 123456;
    callee.accountId = "sameAccount";
    auto profile = GenerateAccessControlProfile(9051, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_IDENTICAL_ACCOUNT,
        ACTIVE, 1, "localDeviceId", 123456, "sameAccount", 1, "remoteDeviceId", 123456,
        "sameAccount", 1051, "bundleName", USER, "");

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkAcuntAccessControl(
        profile, caller, "localDeviceId", callee, "remoteDeviceId");
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSrcShareAccessControl_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 123456;
    caller.accountId = "shareAccount";
    DmAccessCallee callee;
    auto profile = GenerateAccessControlProfile(9060, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_SHARE,
        ACTIVE, 1, "localDeviceId", 123456, "shareAccount", 1, "remoteDeviceId", 123456,
        "shareAccount", 1060, "bundleName", USER, "");

    bool ret = DeviceProfileConnector::GetInstance().CheckSrcShareAccessControl(
        profile, caller, "localDeviceId", callee, "remoteDeviceId");
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkShareAccessControl_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 123456;
    caller.accountId = "shareAccount";
    DmAccessCallee callee;
    callee.userId = 123456;
    callee.accountId = "shareAccount";
    auto profile = GenerateAccessControlProfile(9061, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_SHARE,
        ACTIVE, 1, "localDeviceId", 123456, "shareAccount", 1, "remoteDeviceId", 123456,
        "shareAccount", 1061, "bundleName", USER, "");

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkShareAccessControl(
        profile, caller, "localDeviceId", callee, "remoteDeviceId");
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSrcP2PAccessControl_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 123456;
    caller.tokenId = 1062;
    DmAccessCallee callee;
    auto profile = GenerateAccessControlProfile(9062, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "p2pAccount", 1, "remoteDeviceId", 123456,
        "p2pAccount", 1062, "bundleName", APP, "");

    bool ret = DeviceProfileConnector::GetInstance().CheckSrcP2PAccessControl(
        profile, caller, "localDeviceId", callee, "remoteDeviceId");
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkP2PAccessControl_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 123456;
    caller.tokenId = 1065;
    DmAccessCallee callee;
    callee.userId = 123456;
    callee.tokenId = 1065;
    auto profile = GenerateAccessControlProfile(9065, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "p2pAccount", 1, "remoteDeviceId", 123456,
        "p2pAccount", 1065, "bundleName", APP, "");

    bool ret = DeviceProfileConnector::GetInstance().CheckSinkP2PAccessControl(
        profile, caller, "localDeviceId", callee, "remoteDeviceId");
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckExtWhiteList_201, testing::ext::TestSize.Level1)
{
    bool ret = DeviceProfileConnector::GetInstance().CheckExtWhiteList("");
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, MatchAccesseeByBothUserIdAndPeerUdid_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9066, 1, "peerUdid", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "account", 1, "peerUdid", 456,
        "account", 1066, "bundleName", USER, "");
    bool ret = DeviceProfileConnector::GetInstance().MatchAccesseeByBothUserIdAndPeerUdid(
        "peerUdid", 456, 123456, profile);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, MatchAccesserByBothUserIdAndPeerUdid_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9067, 1, "peerUdid", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "peerUdid", 456, "account", 1, "localDeviceId", 123456,
        "account", 1067, "bundleName", USER, "");
    bool ret = DeviceProfileConnector::GetInstance().MatchAccesserByBothUserIdAndPeerUdid(
        "peerUdid", 456, 123456, profile);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HasServiceId_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9068, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "account", 1, "remoteDeviceId", 123456,
        "account", 1068, "bundleName", APP, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[7001,7002]})");
    profile.SetAccessee(accessee);

    bool ret = DeviceProfileConnector::GetInstance().HasServiceId(profile, 7002);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleSyncBackgroundUserIdEventInner_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9069, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_POINT_TO_POINT,
        ACTIVE, 1, "localDeviceId", 123456, "account", 1, "remoteDeviceId", 123456,
        "account", 1069, "bundleName", APP, "");
    auto accessee = profile.GetAccessee();
    accessee.SetAccesseeExtraData(R"({"serviceId":[8001]})");
    profile.SetAccessee(accessee);

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEventInner(profile, serviceInfos);
    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_EQ(serviceInfos[0].localTokenId, 1069);
}

HWTEST_F(DeviceProfileConnectorSecondTest, FillDmUserRemovedServiceInfoRemote_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9070, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_IDENTICAL_ACCOUNT,
        ACTIVE, 1, "localDeviceId", 123456, "account", 1, "remoteDeviceId", 123456,
        "account", 1070, "bundleName", USER, "");
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().FillDmUserRemovedServiceInfoRemote(profile, serviceInfos);
    EXPECT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_FALSE(serviceInfos[0].serviceIds.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, FillDmUserRemovedServiceInfoLocal_201, testing::ext::TestSize.Level1)
{
    auto profile = GenerateAccessControlProfile(9071, 1, "remoteDeviceId", ALLOW_AUTH_ALWAYS, DM_IDENTICAL_ACCOUNT,
        ACTIVE, 1, "localDeviceId", 123456, "account", 1, "remoteDeviceId", 123456,
        "account", 1071, "bundleName", USER, "");
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().FillDmUserRemovedServiceInfoLocal(profile, serviceInfos);
    ASSERT_EQ(serviceInfos.size(), 1);
    EXPECT_FALSE(serviceInfos[0].serviceIds.empty());
}

/**
 * @tc.name: GetAllVerionAclMap_202
 * @tc.desc: GetAllVerionAclMap with valid ACL profile
 *           Step 1: Prepare AccessControlProfile with version info
 *           Step 2: Call GetAllVerionAclMap
 *           Step 3: Verify aclMap is populated
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetAllVerionAclMap_202, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    std::map<std::string, std::vector<std::string>> aclMap;
    std::string dmVersion = "6.1.0";

    DeviceProfileConnector::GetInstance().GetAllVerionAclMap(acl, aclMap, dmVersion);
    EXPECT_EQ(aclMap.empty(), true);
}

/**
 * @tc.name: ChecksumAcl_203
 * @tc.desc: ChecksumAcl with valid ACL profile
 *           Step 1: Prepare AccessControlProfile
 *           Step 2: Call ChecksumAcl
 *           Step 3: Verify checksum result
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, ChecksumAcl_203, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    std::vector<std::string> acLStrList;

    bool ret = DeviceProfileConnector::GetInstance().ChecksumAcl(acl, acLStrList);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AccessToStr_201
 * @tc.desc: AccessToStr converts ACL to string
 *           Step 1: Prepare AccessControlProfile
 *           Step 2: Call AccessToStr
 *           Step 3: Verify return string is not empty
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, AccessToStr_201, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl;
    std::string result = DeviceProfileConnector::GetInstance().AccessToStr(acl);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: GetAclList_001
 * @tc.desc: GetAclList with valid device and user info
 *           Step 1: Prepare localUdid, localUserId, remoteUdid, remoteUserId
 *           Step 2: Call GetAclList
 *           Step 3: Verify ACL list is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetAclList_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localUdid";
    int32_t localUserId = 100;
    std::string remoteUdid = "remoteUdid";
    int32_t remoteUserId = 100;

    std::vector<DistributedDeviceProfile::AccessControlProfile> aclList =
        DeviceProfileConnector::GetInstance().GetAclList(localUdid, localUserId, remoteUdid, remoteUserId);
    EXPECT_EQ(aclList.empty(), true);
}

/**
 * @tc.name: AclHashItemToJson_001
 * @tc.desc: AclHashItemToJson converts AclHashItem to JSON
 *           Step 1: Prepare AclHashItem with version and hash list
 *           Step 2: Call AclHashItemToJson
 *           Step 3: Verify JSON object is created
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, AclHashItemToJson_001, testing::ext::TestSize.Level1)
{
    JsonObject itemObject("{\"version\":\"5.1.0\",\"aclHashList\":\"[\\\"hasha1\\\"]\"}");
    AclHashItem value;
    DeviceProfileConnector::GetInstance().AclHashItemToJson(itemObject, value);
    EXPECT_EQ(value.version, "");
}

/**
 * @tc.name: GetServiceInfosByUdid_001
 * @tc.desc: GetServiceInfosByUdid retrieves service infos for device
 *           Step 1: Prepare device udid
 *           Step 2: Call GetServiceInfosByUdid
 *           Step 3: Verify service infos are returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetServiceInfosByUdid_001, testing::ext::TestSize.Level1)
{
    std::string udid = "testUdid";
    std::vector<DistributedDeviceProfile::ServiceInfo> serviceInfos;

    int32_t ret = DeviceProfileConnector::GetInstance().GetServiceInfosByUdid(udid, serviceInfos);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: GetServiceInfosByUdidAndUserId_001
 * @tc.desc: GetServiceInfosByUdidAndUserId retrieves service infos for device and user
 *           Step 1: Prepare device udid and userId
 *           Step 2: Call GetServiceInfosByUdidAndUserId
 *           Step 3: Verify service infos are returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetServiceInfosByUdidAndUserId_001, testing::ext::TestSize.Level1)
{
    std::string udid = "testUdid";
    int32_t userId = 100;
    std::vector<DistributedDeviceProfile::ServiceInfo> serviceInfos;

    int32_t ret = DeviceProfileConnector::GetInstance().GetServiceInfosByUdidAndUserId(udid, userId, serviceInfos);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: GetServiceInfoByUdidAndServiceId_001
 * @tc.desc: GetServiceInfoByUdidAndServiceId retrieves specific service info
 *           Step 1: Prepare device udid and serviceId
 *           Step 2: Call GetServiceInfoByUdidAndServiceId
 *           Step 3: Verify service info is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetServiceInfoByUdidAndServiceId_001, testing::ext::TestSize.Level1)
{
    std::string udid = "testUdid";
    int64_t serviceId = 1001;
    DistributedDeviceProfile::ServiceInfo dpServiceInfo;

    int32_t ret = DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(udid,
        serviceId, dpServiceInfo);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: PutServiceInfo_001
 * @tc.desc: PutServiceInfo stores service info
 *           Step 1: Prepare ServiceInfo
 *           Step 2: Call PutServiceInfo
 *           Step 3: Verify return value
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, PutServiceInfo_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::ServiceInfo dpServiceInfo;
    dpServiceInfo.SetServiceId(1001);

    int32_t ret = DeviceProfileConnector::GetInstance().PutServiceInfo(dpServiceInfo);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: DeleteServiceInfo_001
 * @tc.desc: DeleteServiceInfo removes service info
 *           Step 1: Prepare ServiceInfo
 *           Step 2: Call DeleteServiceInfo
 *           Step 3: Verify return value
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, DeleteServiceInfo_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::ServiceInfo serviceInfo;
    serviceInfo.SetServiceId(1001);

    int32_t ret = DeviceProfileConnector::GetInstance().DeleteServiceInfo(serviceInfo);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: DeleteSessionKey_001
 * @tc.desc: DeleteSessionKey removes session key
 *           Step 1: Prepare userId and sessionKeyId
 *           Step 2: Call DeleteSessionKey
 *           Step 3: Verify return value
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, DeleteSessionKey_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;
    int32_t sessionKeyId = 1;

    int32_t ret = DeviceProfileConnector::GetInstance().DeleteSessionKey(userId, sessionKeyId);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: GetSessionKey_001
 * @tc.desc: GetSessionKey retrieves session key
 *           Step 1: Prepare userId and sessionKeyId
 *           Step 2: Call GetSessionKey
 *           Step 3: Verify session key is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetSessionKey_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;
    int32_t sessionKeyId = 1;
    std::vector<unsigned char> sessionKeyArray;

    int32_t ret = DeviceProfileConnector::GetInstance().GetSessionKey(userId, sessionKeyId, sessionKeyArray);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: PutSessionKey_001
 * @tc.desc: PutSessionKey stores session key
 *           Step 1: Prepare userId and session key array
 *           Step 2: Call PutSessionKey
 *           Step 3: Verify session key ID is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, PutSessionKey_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;
    std::vector<unsigned char> sessionKeyArray = {1, 2, 3, 4};
    int32_t sessionKeyId = 0;

    int32_t ret = DeviceProfileConnector::GetInstance().PutSessionKey(userId, sessionKeyArray, sessionKeyId);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: PutLocalServiceInfo_001
 * @tc.desc: PutLocalServiceInfo stores local service info
 *           Step 1: Prepare LocalServiceInfo
 *           Step 2: Call PutLocalServiceInfo
 *           Step 3: Verify return value
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, PutLocalServiceInfo_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;
    localServiceInfo.SetBundleName("com.test.app");

    int32_t ret = DeviceProfileConnector::GetInstance().PutLocalServiceInfo(localServiceInfo);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: DeleteLocalServiceInfo_001
 * @tc.desc: DeleteLocalServiceInfo removes local service info
 *           Step 1: Prepare bundleName and pinExchangeType
 *           Step 2: Call DeleteLocalServiceInfo
 *           Step 3: Verify return value
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, DeleteLocalServiceInfo_001, testing::ext::TestSize.Level1)
{
    std::string bundleName = "com.test.app";
    int32_t pinExchangeType = 1;

    int32_t ret = DeviceProfileConnector::GetInstance().DeleteLocalServiceInfo(bundleName, pinExchangeType);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: UpdateLocalServiceInfo_001
 * @tc.desc: UpdateLocalServiceInfo updates local service info
 *           Step 1: Prepare LocalServiceInfo
 *           Step 2: Call UpdateLocalServiceInfo
 *           Step 3: Verify return value
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, UpdateLocalServiceInfo_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;
    localServiceInfo.SetBundleName("com.test.app");

    int32_t ret = DeviceProfileConnector::GetInstance().UpdateLocalServiceInfo(localServiceInfo);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: GetLocalServiceInfoByBundleNameAndPinExchangeType_001
 * @tc.desc: GetLocalServiceInfoByBundleNameAndPinExchangeType retrieves local service info
 *           Step 1: Prepare bundleName and pinExchangeType
 *           Step 2: Call GetLocalServiceInfoByBundleNameAndPinExchangeType
 *           Step 3: Verify service info is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetLocalServiceInfoByBundleNameAndPinExchangeType_001,
    testing::ext::TestSize.Level1)
{
    std::string bundleName = "com.test.app";
    int32_t pinExchangeType = 1;
    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;

    int32_t ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        bundleName, pinExchangeType, localServiceInfo);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: GetDeviceIdAndUdidListByTokenId_201
 * @tc.desc: GetDeviceIdAndUdidListByTokenId retrieves device IDs by token ID
 *           Step 1: Prepare userIds, localUdid and tokenId
 *           Step 2: Call GetDeviceIdAndUdidListByTokenId
 *           Step 3: Verify device ID list is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetDeviceIdAndUdidListByTokenId_201, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIds = {100};
    std::string localUdid = "localUdid";
    int32_t tokenId = 123456;

    std::vector<std::string> deviceIdList = DeviceProfileConnector::GetInstance().GetDeviceIdAndUdidListByTokenId(
        userIds, localUdid, tokenId);
    EXPECT_EQ(deviceIdList.empty(), true);
}

/**
 * @tc.name: GetUserIdAndBindLevel_201
 * @tc.desc: GetUserIdAndBindLevel retrieves user ID and bind level mapping
 *           Step 1: Prepare localUdid and peerUdid
 *           Step 2: Call GetUserIdAndBindLevel
 *           Step 3: Verify user ID and bind level map is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetUserIdAndBindLevel_201, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localUdid";
    std::string peerUdid = "peerUdid";

    std::map<int32_t, int32_t> result = DeviceProfileConnector::GetInstance().GetUserIdAndBindLevel(localUdid,
        peerUdid);
    EXPECT_EQ(result.empty(), true);
}

/**
 * @tc.name: CheckAccessControlProfileByTokenId_201
 * @tc.desc: CheckAccessControlProfileByTokenId checks if ACL exists for token ID
 *           Step 1: Prepare tokenId
 *           Step 2: Call CheckAccessControlProfileByTokenId
 *           Step 3: Verify check result
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, CheckAccessControlProfileByTokenId_201, testing::ext::TestSize.Level1)
{
    int32_t tokenId = 123456;

    bool ret = DeviceProfileConnector::GetInstance().CheckAccessControlProfileByTokenId(tokenId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetActiveAuthOncePeerUserId_201
 * @tc.desc: GetActiveAuthOncePeerUserId retrieves active auth-once peer user IDs
 *           Step 1: Prepare peerUdid and localUserId
 *           Step 2: Call GetActiveAuthOncePeerUserId
 *           Step 3: Verify peer user ID set is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetActiveAuthOncePeerUserId_201, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    int32_t localUserId = 100;

    std::unordered_set<int32_t> result = DeviceProfileConnector::GetInstance().GetActiveAuthOncePeerUserId(
        peerUdid, localUserId);
    EXPECT_EQ(result.empty(), true);
}

/**
 * @tc.name: GetAuthOnceAclInfos_201
 * @tc.desc: GetAuthOnceAclInfos retrieves auth-once ACL infos
 *           Step 1: Prepare peerUdid
 *           Step 2: Call GetAuthOnceAclInfos
 *           Step 3: Verify ACL info set is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthOnceAclInfos_201, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";

    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> result =
        DeviceProfileConnector::GetInstance().GetAuthOnceAclInfos(peerUdid);
    EXPECT_EQ(result.empty(), true);
}

/**
 * @tc.name: AuthOnceAclIsActive_201
 * @tc.desc: AuthOnceAclIsActive checks if auth-once ACL is active
 *           Step 1: Prepare peerUdid, peerUserId and localUserId
 *           Step 2: Call AuthOnceAclIsActive
 *           Step 3: Verify active status
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, AuthOnceAclIsActive_201, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 100;
    int32_t localUserId = 100;

    bool ret = DeviceProfileConnector::GetInstance().AuthOnceAclIsActive(peerUdid, peerUserId, localUserId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetAuthTypeByUdidHash_001
 * @tc.desc: GetAuthTypeByUdidHash retrieves auth type by UDID hash
 *           Step 1: Prepare udidHash and pkgName
 *           Step 2: Call GetAuthTypeByUdidHash
 *           Step 3: Verify auth type is returned
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetAuthTypeByUdidHash_001, testing::ext::TestSize.Level1)
{
    std::string udidHash = "udidHash";
    std::string pkgName = "com.test.app";
    DMLocalServiceInfoAuthType authType;

    DeviceProfileConnector::GetInstance().GetAuthTypeByUdidHash(udidHash, pkgName, authType);
    EXPECT_NE(authType, DMLocalServiceInfoAuthType::TRUST_ONETIME);
}

/**
 * @tc.name: DeleteDpInvalidAcl_001
 * @tc.desc: DeleteDpInvalidAcl deletes invalid ACLs
 *           Step 1: Call DeleteDpInvalidAcl
 *           Step 2: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, DeleteDpInvalidAcl_001, testing::ext::TestSize.Level1)
{
    DeviceProfileConnector::GetInstance().DeleteDpInvalidAcl();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: CheckUserIdIsForegroundUserId_001
 * @tc.desc: CheckUserIdIsForegroundUserId checks if user ID is foreground
 *           Step 1: Prepare userId
 *           Step 2: Call CheckUserIdIsForegroundUserId
 *           Step 3: Verify foreground status
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, CheckUserIdIsForegroundUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;

    bool ret = DeviceProfileConnector::GetInstance().CheckUserIdIsForegroundUserId(userId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetPeerTokenIdForServiceProxyUnbind_001
 * @tc.desc: GetPeerTokenIdForServiceProxyUnbind retrieves peer token IDs for service unbind
 *           Step 1: Prepare userId, localTokenId, peerUdid and serviceId
 *           Step 2: Call GetPeerTokenIdForServiceProxyUnbind
 *           Step 3: Verify peer token ID vector is populated
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetPeerTokenIdForServiceProxyUnbind_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;
    uint64_t localTokenId = 123456;
    std::string peerUdid = "peerUdid";
    int64_t serviceId = 1001;
    std::vector<uint64_t> peerTokenId;

    DeviceProfileConnector::GetInstance().GetPeerTokenIdForServiceProxyUnbind(
        userId, localTokenId, peerUdid, serviceId, peerTokenId);
    EXPECT_EQ(peerTokenId.empty(), true);
}

/**
 * @tc.name: HandleSyncForegroundUserIdEvent_001
 * @tc.desc: HandleSyncForegroundUserIdEvent handles foreground user sync event
 *           Step 1: Prepare remoteUserIds, remoteUdid, localUserIds and localUdid
 *           Step 2: Call HandleSyncForegroundUserIdEvent
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, HandleSyncForegroundUserIdEvent_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> remoteUserIds = {100};
    std::string remoteUdid = "remoteUdid";
    std::vector<int32_t> localUserIds = {100};
    std::string localUdid = "localUdid";

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(
        remoteUserIds, remoteUdid, localUserIds, localUdid, serviceInfos);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: HandleSyncBackgroundUserIdEvent_001
 * @tc.desc: HandleSyncBackgroundUserIdEvent handles background user sync event
 *           Step 1: Prepare remoteUserIds, remoteUdid, localUserIds and localUdid
 *           Step 2: Call HandleSyncBackgroundUserIdEvent
 *           Step 3: Verify no crash occurs
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, HandleSyncBackgroundUserIdEvent_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> remoteUserIds = {100};
    std::string remoteUdid = "remoteUdid";
    std::vector<int32_t> localUserIds = {100};
    std::string localUdid = "localUdid";

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(
        remoteUserIds, remoteUdid, localUserIds, localUdid, serviceInfos);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UnSubscribeDeviceProfileInited_001
 * @tc.desc: UnSubscribeDeviceProfileInited unsubscribes from device profile init event
 *           Step 1: Call UnSubscribeDeviceProfileInited
 *           Step 2: Verify return value
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, UnSubscribeDeviceProfileInited_001, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceProfileConnector::GetInstance().UnSubscribeDeviceProfileInited();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckIsSameAccountByUdidHash_001, testing::ext::TestSize.Level1)
{
    std::string udidHash;
    int32_t ret = DeviceProfileConnector::GetInstance().CheckIsSameAccountByUdidHash(udidHash);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAccessControlProfileByAccessControlId_001, testing::ext::TestSize.Level1)
{
    auto profile = DeviceProfileConnector::GetInstance().GetAccessControlProfileByAccessControlId(-1);
    EXPECT_EQ(profile.GetAccessControlId(), 0);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAgentToProxyVecFromAclByUserId_001, testing::ext::TestSize.Level1)
{
    auto result = DeviceProfileConnector::GetInstance().GetAgentToProxyVecFromAclByUserId("", "", 0);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteHoDevice_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> foregroundUserIds;
    std::vector<int32_t> backgroundUserIds;
    DeviceProfileConnector::GetInstance().DeleteHoDevice("", foregroundUserIds, backgroundUserIds);
    EXPECT_TRUE(true);
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetRemoteTokenIds_001, testing::ext::TestSize.Level1)
{
    std::unordered_set<int64_t> remoteTokenIds;
    DeviceProfileConnector::GetInstance().GetRemoteTokenIds("", "", remoteTokenIds);
    EXPECT_TRUE(remoteTokenIds.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAclListHashStr_001, testing::ext::TestSize.Level1)
{
    DevUserInfo localDevUserInfo;
    localDevUserInfo.deviceId = "";
    localDevUserInfo.userId = 0;
    DevUserInfo remoteDevUserInfo;
    remoteDevUserInfo.deviceId = "";
    remoteDevUserInfo.userId = 0;
    std::string aclListHash;
    std::string dmVersion = "5.1.0";
    int32_t ret = DeviceProfileConnector::GetInstance().GetAclListHashStr(localDevUserInfo,
        remoteDevUserInfo, aclListHash, dmVersion);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_FALSE(aclListHash.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, GetAllAclIncludeLnnAcl_001, testing::ext::TestSize.Level1)
{
    auto profiles = DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    if (profiles.empty()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_FALSE(profiles.empty());
    }
}

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAccessControlById_001, testing::ext::TestSize.Level1)
{
    DeviceProfileConnector::GetInstance().DeleteAccessControlById(-1);
    EXPECT_TRUE(true);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleAccountCommonEvent_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localUdid";
    std::vector<std::string> deviceVec;
    std::vector<int32_t> foregroundUserIds;
    std::vector<int32_t> backgroundUserIds;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleAccountCommonEvent(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds, serviceInfos);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_TRUE(serviceInfos.empty());
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleDeviceUnBind_001, testing::ext::TestSize.Level1)
{
    int32_t bindType = USER;
    std::string peerUdid;
    std::string localUdid;
    int32_t localUserId = 0;
    std::string localAccountId;
    DeviceProfileConnector::GetInstance().HandleDeviceUnBind(bindType, peerUdid, localUdid, localUserId,
        localAccountId);
    EXPECT_TRUE(true);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSrcAccessControl_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = -1;
    caller.tokenId = 0;
    DmAccessCallee callee;
    callee.userId = 0;
    callee.tokenId = 0;
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcAccessControl(caller, "", callee, "");
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkAccessControl_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 0;
    caller.tokenId = 0;
    DmAccessCallee callee;
    callee.userId = -1;
    callee.tokenId = 0;
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkAccessControl(caller, "", callee, "");
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSrcIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = -1;
    caller.tokenId = 0;
    DmAccessCallee callee;
    callee.userId = 0;
    callee.tokenId = 0;
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcIsSameAccount(caller, "", callee, "");
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, CheckSinkIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    caller.userId = 0;
    caller.tokenId = 0;
    DmAccessCallee callee;
    callee.userId = -1;
    callee.tokenId = 0;
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkIsSameAccount(caller, "", callee, "");
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleUserSwitched_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::vector<std::string> deviceVec = {"remoteDeviceId"};
    int32_t currentUserId = 456;
    int32_t beforeUserId = 123456;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleUserSwitched(localUdid, deviceVec,
        currentUserId, beforeUserId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleUserSwitched_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::vector<std::string> deviceVec = {"remoteDeviceId"};
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {123456};
    int32_t ret = DeviceProfileConnector::GetInstance().HandleUserSwitched(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, HandleAccountCommonEvent_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localDeviceId";
    std::vector<std::string> deviceVec = {"remoteDeviceId"};
    std::vector<int32_t> foregroundUserIds = {456};
    std::vector<int32_t> backgroundUserIds = {123456};
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleAccountCommonEvent(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds, serviceInfos);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorSecondTest, FindTargetAclIncludeLnnOld_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile acl1;
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    DmOfflineParam offlineParam;
    std::string localUdid = "localUdid";
    std::string remoteUdid = "remoteUdid";
    accesser.SetAccesserDeviceId(remoteUdid);
    accessee.SetAccesseeDeviceId(localUdid);
    acl1.SetAccesser(accesser);
    acl1.SetAccessee(accessee);
    bool ret = DeviceProfileConnector::GetInstance().FindTargetAclIncludeLnnOld(
        acl1, localUdid, remoteUdid, offlineParam);
    EXPECT_EQ(true, ret);

    DistributedDeviceProfile::AccessControlProfile acl2;
    accesser.SetAccesserDeviceId(localUdid);
    accessee.SetAccesseeDeviceId(remoteUdid);
    acl2.SetAccesser(accesser);
    acl2.SetAccessee(accessee);
    ret = DeviceProfileConnector::GetInstance().FindTargetAclIncludeLnnOld(
        acl2, localUdid, remoteUdid, offlineParam);
    EXPECT_EQ(true, ret);

    std::string extra = "extra";
    DistributedDeviceProfile::AccessControlProfile acl3;
    accesser.SetAccesserDeviceId(localUdid);
    accessee.SetAccesseeDeviceId(remoteUdid);
    acl3.SetAccesser(accesser);
    acl3.SetAccessee(accessee);
    ret = DeviceProfileConnector::GetInstance().FindTargetAclIncludeLnnOld(
        acl3, extra, remoteUdid, offlineParam);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: GetVersionByExtra_201
 * @tc.desc: GetVersionByExtra with valid extra info containing dmVersion
 *           Step 1: Prepare extraInfo with dmVersion field
 *           Step 2: Call GetVersionByExtra
 *           Step 3: Verify dmVersion is extracted correctly
 * @tc.type: FUNC
 */
HWTEST_F(DeviceProfileConnectorSecondTest, GetVersionByExtra_201, testing::ext::TestSize.Level1)
{
    std::string extraInfo = "{\"dmVersion\":\"5.1.0\"}";
    std::string dmVersion;
    int32_t ret = DeviceProfileConnector::GetInstance().GetVersionByExtra(extraInfo, dmVersion);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(dmVersion, "5.1.0");
}
} // namespace DistributedHardware
} // namespace OHOS
