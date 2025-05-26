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

#include "UTTest_device_manager_service_impl_first.h"
#include "softbus_error_code.h"
#include "common_event_support.h"
#include "deviceprofile_connector.h"
#include "distributed_device_profile_client.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::DistributedDeviceProfile;
namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceImplFirstTest::SetUp()
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->Initialize(listener_);
}

void DeviceManagerServiceImplFirstTest::TearDown()
{
}

void DeviceManagerServiceImplFirstTest::SetUpTestCase()
{
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    DmSoftbusConnector::dmSoftbusConnector = softbusConnectorMock_;
    DmDmDeviceStateManager::dmDeviceStateManager = dmDeviceStateManagerMock_;
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl = deviceManagerServiceImplMock_;
}

void DeviceManagerServiceImplFirstTest::TearDownTestCase()
{
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    deviceProfileConnectorMock_ = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    softbusConnectorMock_ = nullptr;
    DmDmDeviceStateManager::dmDeviceStateManager = nullptr;
    dmDeviceStateManagerMock_ = nullptr;
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl = nullptr;
    deviceManagerServiceImplMock_ = nullptr;
}

namespace {
HWTEST_F(DeviceManagerServiceImplFirstTest, GetDeviceIdAndUserId_101, testing::ext::TestSize.Level1)
{
    int32_t userId = 1;
    std::string accountId = "accountId";
    auto ret = deviceManagerServiceImpl_->GetDeviceIdAndUserId(userId, accountId);
    EXPECT_TRUE(ret.empty());

    ret = deviceManagerServiceImpl_->GetDeviceIdAndUserId(userId);
    EXPECT_TRUE(ret.empty());

    std::string localUdid = "deviceId";
    int32_t localUserId = 123456;
    std::string peerUdid = "remoteUdid";
    int32_t peerUserId = 1;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAclForAccountLogOut(_, _, _, _, _)).WillOnce(Return(true));
    if (deviceManagerServiceImpl_->softbusConnector_ == nullptr) {
        deviceManagerServiceImpl_->Initialize(listener_);
    }

    if (deviceManagerServiceImpl_->deviceStateMgr_ == nullptr) {
        deviceManagerServiceImpl_->Initialize(listener_);
    }
    deviceManagerServiceImpl_->HandleIdentAccountLogout(localUdid, localUserId, peerUdid, peerUserId);

    std::vector<uint32_t> foregroundUserIds;
    std::vector<uint32_t> backgroundUserIds;
    std::string remoteUdid = "deviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_)).WillOnce(Return(ERR_DM_FAILED));
    deviceManagerServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, false);

    std::vector<int32_t> localUserIds;
    localUserIds.push_back(101);
    localUserIds.push_back(102);
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(localUserIds), Return(DM_OK)));
    deviceManagerServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, false);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, CheckSharePeerSrc_001, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    std::string localUdid = "localUdid";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));

    bool result = deviceManagerServiceImpl_->CheckSharePeerSrc(peerUdid, localUdid);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, CheckSharePeerSrc_002, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    std::string localUdid = "localUdid";
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);
    profile.SetTrustDeviceId(peerUdid);
    Accesser accesser;
    accesser.SetAccesserDeviceId(peerUdid);
    profile.SetAccesser(accesser);

    Accessee accessee;
    accessee.SetAccesseeDeviceId(localUdid);
    profile.SetAccessee(accessee);
    profiles.push_back(profile);
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    
    bool result = deviceManagerServiceImpl_->CheckSharePeerSrc(peerUdid, localUdid);
    EXPECT_TRUE(result);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, CheckSharePeerSrc_003, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    std::string localUdid = "localUdid";
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);
    profile.SetTrustDeviceId(peerUdid);
    Accesser accesser;
    accesser.SetAccesserDeviceId(localUdid);
    profile.SetAccesser(accesser);
    profile.GetAccessee().SetAccesseeDeviceId(peerUdid);
    profiles.push_back(profile);
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    
    bool result = deviceManagerServiceImpl_->CheckSharePeerSrc(peerUdid, localUdid);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, CheckSharePeerSrc_004, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    std::string localUdid = "localUdid";
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    profile.SetTrustDeviceId(peerUdid);
    Accesser accesser;
    accesser.SetAccesserDeviceId(peerUdid);
    profile.SetAccesser(accesser);
    Accessee accessee;
    accessee.SetAccesseeDeviceId(localUdid);
    profile.SetAccessee(accessee);
    profiles.push_back(profile);
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    
    bool result = deviceManagerServiceImpl_->CheckSharePeerSrc(peerUdid, localUdid);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, CheckSharePeerSrc_005, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "peerUdid";
    std::string localUdid = "localUdid";
    std::string trustDevideId = "trustDevideId";

    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);
    profile.SetTrustDeviceId(trustDevideId);
    profiles.push_back(profile);
   
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));

    bool result = deviceManagerServiceImpl_->CheckSharePeerSrc(peerUdid, localUdid);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleCredentialDeleted_001, testing::ext::TestSize.Level1)
{
    const char *credId = "123456";
    const char *credInfo = "invalid_json";
    std::string localUdid = "localUdid";
    std::string remoteUdid;
    bool isShareType = false;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;

    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(0);
    
    deviceManagerServiceImpl_->HandleCredentialDeleted(credId, credInfo, localUdid, remoteUdid, isShareType);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleCredentialDeleted_002, testing::ext::TestSize.Level1)
{
    const char *credId = "123456";
    const char *credInfo = R"({"deviceId": "remoteUdid", "userId": 1})";
    std::string localUdid = "localUdid";
    std::string remoteUdid;
    bool isShareType = false;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_POINT_TO_POINT);
    profiles.push_back(profile);

    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_)).Times(0);

    deviceManagerServiceImpl_->HandleCredentialDeleted(credId, credInfo, localUdid, remoteUdid, isShareType);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleCredentialDeleted_003, testing::ext::TestSize.Level1)
{
    const char *credId = "123456";
    const char *credInfo = R"({"deviceId": "remoteUdid", "userId": 1})";
    std::string localUdid = "localUdid";
    std::string remoteUdid;
    bool isShareType = false;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);

    profile.GetAccesser().SetAccesserCredentialId(atoi(credId));
    profile.GetAccesser().SetAccesserDeviceId(localUdid);
    profile.GetAccessee().SetAccesseeDeviceId("remoteUdid");
    profile.GetAccessee().SetAccesseeUserId(1);

    profiles.push_back(profile);
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(1);

    deviceManagerServiceImpl_->HandleCredentialDeleted(credId, credInfo, localUdid, remoteUdid, isShareType);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleCredentialDeleted_004, testing::ext::TestSize.Level1)
{
    const char *credId = "123456";
    const char *credInfo = R"({"deviceId": "remoteUdid", "userId": 1})";
    std::string localUdid = "localUdid";
    std::string remoteUdid;
    bool isShareType = false;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);

    profile.GetAccessee().SetAccesseeCredentialId(atoi(credId));
    profile.GetAccessee().SetAccesseeDeviceId(localUdid);
    profile.GetAccesser().SetAccesserDeviceId("remoteUdid");
    profile.GetAccesser().SetAccesserUserId(1);

    profiles.push_back(profile);

    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(1);
    
    deviceManagerServiceImpl_->HandleCredentialDeleted(credId, credInfo, localUdid, remoteUdid, isShareType);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleShareUnbindBroadCast_001, testing::ext::TestSize.Level1)
{
    std::string credId = "12345";
    std::string localUdid = "localId";
    int32_t userId = 123456;
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(0);
    
    deviceManagerServiceImpl_->HandleShareUnbindBroadCast(credId, userId, localUdid);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleShareUnbindBroadCast_002, testing::ext::TestSize.Level1)
{
    std::string credId = "12345";
    std::string localUdid = "localId";
    int32_t userId = 123456;
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    profiles.push_back(profile);
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(0);
    
    deviceManagerServiceImpl_->HandleShareUnbindBroadCast(credId, userId, localUdid);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleShareUnbindBroadCast_003, testing::ext::TestSize.Level1)
{
    std::string credId = "12345";
    std::string localUdid = "localId";
    int32_t userId = 123456;
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);
    
    Accesser accesser;
    accesser.SetAccesserCredentialId(atoi(credId.c_str()));
    accesser.SetAccesserUserId(userId);
    profile.SetAccesser(accesser);
    Accessee accessee;
    accessee.SetAccesseeDeviceId(localUdid);
    profile.SetAccessee(accessee);
    
    profiles.push_back(profile);
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(1);
    
    deviceManagerServiceImpl_->HandleShareUnbindBroadCast(credId, userId, localUdid);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleShareUnbindBroadCast_004, testing::ext::TestSize.Level1)
{
    std::string credId = "12345";
    std::string localUdid = "localUdid";
    int32_t userId = 123456;

    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);
    
    Accesser accesser;
    accesser.SetAccesserDeviceId(localUdid);
    profile.SetAccesser(accesser);
    Accessee accessee;
    accessee.SetAccesseeCredentialId(atoi(credId.c_str()));
    accessee.SetAccesseeUserId(userId);
    profile.SetAccessee(accessee);
    
    profiles.push_back(profile);

    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(1);

    deviceManagerServiceImpl_->HandleShareUnbindBroadCast(credId, userId, localUdid);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, HandleShareUnbindBroadCast_005, testing::ext::TestSize.Level1)
{
    std::string credId = "12345";
    std::string localUdid = "localUdid";
    int32_t userId = 123456;
    
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    AccessControlProfile profile;
    profile.SetBindType(DM_SHARE);
    
    Accesser accesser;
    accesser.SetAccesserCredentialId(9999);
    accesser.SetAccesserDeviceId("peer123");
    accesser.SetAccesserUserId(1002);
    profile.SetAccesser(accesser);
    
    Accessee accessee;
    accessee.SetAccesseeDeviceId("peer456");
    accessee.SetAccesseeUserId(1003);
    profile.SetAccessee(accessee);
    
    profiles.push_back(profile);
    
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
        .WillOnce(Return(profiles));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlById(_))
        .Times(0);
    
    deviceManagerServiceImpl_->HandleShareUnbindBroadCast(credId, userId, localUdid);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, SetOnlineProcessInfo_001, testing::ext::TestSize.Level1)
{
    uint32_t bindType = IDENTICAL_ACCOUNT_TYPE;
    ProcessInfo processInfo;
    DmDeviceInfo devInfo;
    std::string requestDeviceId = "requestDeviceId";
    std::string trustDeviceId = "trustDeviceId";
    DmDeviceState devState = DEVICE_STATE_ONLINE;

    EXPECT_CALL(*softbusConnectorMock_, SetProcessInfo(_)).Times(1);
    EXPECT_CALL(*dmDeviceStateManagerMock_, HandleDeviceStatusChange(_, _)).Times(1);

    deviceManagerServiceImpl_->SetOnlineProcessInfo(
        bindType, processInfo, devInfo, requestDeviceId, trustDeviceId, devState);

    EXPECT_EQ(devInfo.authForm, DmAuthForm::IDENTICAL_ACCOUNT);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, SetOnlineProcessInfo_002, testing::ext::TestSize.Level1)
{
    uint32_t bindType = DEVICE_PEER_TO_PEER_TYPE;
    ProcessInfo processInfo;
    DmDeviceInfo devInfo;
    std::string requestDeviceId = "requestDeviceId";
    std::string trustDeviceId = "trustDeviceId";
    DmDeviceState devState = DEVICE_STATE_ONLINE;

    EXPECT_CALL(*softbusConnectorMock_, SetProcessInfo(_)).Times(1);
    EXPECT_CALL(*dmDeviceStateManagerMock_, HandleDeviceStatusChange(_, _)).Times(1);

    deviceManagerServiceImpl_->SetOnlineProcessInfo(
        bindType, processInfo, devInfo, requestDeviceId, trustDeviceId, devState);

    EXPECT_EQ(devInfo.authForm, DmAuthForm::PEER_TO_PEER);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, SetOnlineProcessInfo_003, testing::ext::TestSize.Level1)
{
    uint32_t bindType = DEVICE_ACROSS_ACCOUNT_TYPE;
    ProcessInfo processInfo;
    DmDeviceInfo devInfo;
    std::string requestDeviceId = "requestDeviceId";
    std::string trustDeviceId = "trustDeviceId";
    DmDeviceState devState = DEVICE_STATE_ONLINE;

    EXPECT_CALL(*softbusConnectorMock_, SetProcessInfo(_)).Times(1);
    EXPECT_CALL(*dmDeviceStateManagerMock_, HandleDeviceStatusChange(_, _)).Times(1);

    deviceManagerServiceImpl_->SetOnlineProcessInfo(
        bindType, processInfo, devInfo, requestDeviceId, trustDeviceId, devState);

    EXPECT_EQ(devInfo.authForm, DmAuthForm::ACROSS_ACCOUNT);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, SetOnlineProcessInfo_004, testing::ext::TestSize.Level1)
{
    uint32_t bindType = APP_PEER_TO_PEER_TYPE;
    ProcessInfo processInfo;
    DmDeviceInfo devInfo;
    std::string requestDeviceId = "requestDeviceId";
    std::string trustDeviceId = "trustDeviceId";
    DmDeviceState devState = DEVICE_STATE_ONLINE;

    std::vector<ProcessInfo> processInfoVec = {processInfo};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetProcessInfoFromAclByUserId(_, _, _))
        .WillOnce(Return(processInfoVec));
    EXPECT_CALL(*softbusConnectorMock_, SetProcessInfoVec(_)).Times(1);
    EXPECT_CALL(*dmDeviceStateManagerMock_, HandleDeviceStatusChange(_, _)).Times(1);

    deviceManagerServiceImpl_->SetOnlineProcessInfo(
        bindType, processInfo, devInfo, requestDeviceId, trustDeviceId, devState);

    EXPECT_EQ(devInfo.authForm, DmAuthForm::PEER_TO_PEER);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, SetOnlineProcessInfo_005, testing::ext::TestSize.Level1)
{
    uint32_t bindType = APP_ACROSS_ACCOUNT_TYPE;
    ProcessInfo processInfo;
    DmDeviceInfo devInfo;
    std::string requestDeviceId = "requestDeviceId";
    std::string trustDeviceId = "trustDeviceId";
    DmDeviceState devState = DEVICE_STATE_ONLINE;

    std::vector<ProcessInfo> processInfoVec = {processInfo};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetProcessInfoFromAclByUserId(_, _, _))
        .WillOnce(Return(processInfoVec));
    EXPECT_CALL(*softbusConnectorMock_, SetProcessInfoVec(_)).Times(1);
    EXPECT_CALL(*dmDeviceStateManagerMock_, HandleDeviceStatusChange(_, _)).Times(1);

    deviceManagerServiceImpl_->SetOnlineProcessInfo(
        bindType, processInfo, devInfo, requestDeviceId, trustDeviceId, devState);

    EXPECT_EQ(devInfo.authForm, DmAuthForm::ACROSS_ACCOUNT);
}

HWTEST_F(DeviceManagerServiceImplFirstTest, SetOnlineProcessInfo_006, testing::ext::TestSize.Level1)
{
    uint32_t bindType = SHARE_TYPE;
    ProcessInfo processInfo;
    DmDeviceInfo devInfo;
    std::string requestDeviceId = "requestDeviceId";
    std::string trustDeviceId = "trustDeviceId";
    DmDeviceState devState = DEVICE_STATE_ONLINE;

    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfile())
    .Times(1).WillOnce(Return(std::vector<DistributedDeviceProfile::AccessControlProfile>()));

    EXPECT_CALL(*deviceManagerServiceImplMock_, CheckSharePeerSrc(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*softbusConnectorMock_, SetProcessInfo(_)).Times(1);
    EXPECT_CALL(*dmDeviceStateManagerMock_, HandleDeviceStatusChange(_, _)).Times(1);

    deviceManagerServiceImpl_->SetOnlineProcessInfo(
        bindType, processInfo, devInfo, requestDeviceId, trustDeviceId, devState);

    EXPECT_EQ(devInfo.authForm, DmAuthForm::SHARE);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
