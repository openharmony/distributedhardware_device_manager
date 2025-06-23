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
}

void DeviceProfileConnectorSecondTest::SetUpTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
}

void DeviceProfileConnectorSecondTest::TearDownTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
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
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAccessControlProfile(_, _)).WillOnce(Return(ERR_DM_FAILED));

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

HWTEST_F(DeviceProfileConnectorSecondTest, DeleteAclForAccountLogOut_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "local_device_id";
    int32_t localUserId = 1;
    std::string peerUdid = "peer_device_id";
    int32_t peerUserId = 2;
    DmOfflineParam offlineParam;
    DMAclQuadInfo info = {localUdid, localUserId, peerUdid, peerUserId};
    std::string accoutId = "accountId";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetAllAccessControlProfile(_)).WillOnce(Return(DM_OK));
    int32_t result = connector.DeleteAclForAccountLogOut(info, accoutId, offlineParam);

    EXPECT_EQ(result, false);
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
        .Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    auto result = connector.GetDeviceIdAndBindLevel(userIds, localUdid);
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
} // namespace DistributedHardware
} // namespace OHOS
