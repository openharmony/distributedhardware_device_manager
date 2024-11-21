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

#include "UTTest_dm_deviceprofile_connector.h"

#include "dm_constants.h"
#include "dm_device_info.h"
#include "deviceprofile_connector.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceProfileConnectorTest::SetUp()
{
}

void DeviceProfileConnectorTest::TearDown()
{
}

void DeviceProfileConnectorTest::SetUpTestCase()
{
}

void DeviceProfileConnectorTest::TearDownTestCase()
{
}

void AddAccessControlProfileFirst(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
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

void AddAccessControlProfileSecond(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "localDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
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

void AddAccessControlProfileThird(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
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

void AddAccessControlProfileForth(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "localDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
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

void AddAccessControlProfileFifth(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
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

void GetAccessControlProfiles(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    AddAccessControlProfileFirst(accessControlProfiles);
    AddAccessControlProfileSecond(accessControlProfiles);
    AddAccessControlProfileThird(accessControlProfiles);
    AddAccessControlProfileForth(accessControlProfiles);
    AddAccessControlProfileFifth(accessControlProfiles);
}
HWTEST_F(DeviceProfileConnectorTest, GetAccessControlProfile_001, testing::ext::TestSize.Level0)
{
    auto ret = DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId_003";
    auto ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceAclParam_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryInfo discoveryInfo;
    bool isonline = true;
    int32_t authForm = 0;
    int32_t ret = DeviceProfileConnector::GetInstance().GetDeviceAclParam(discoveryInfo, isonline, authForm);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_001, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_IDENTICAL_ACCOUNT);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, IDENTICAL_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_002, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(DEVICE);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, PEER_TO_PEER);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_003, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("ohos_test");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "ohos_test";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, PEER_TO_PEER);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_004, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("ohos_test");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "ohos_test";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, PEER_TO_PEER);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_005, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(DEVICE);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_006, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("pkgName");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "pkgName";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_007, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "pkgName";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_008, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    uint32_t invalidType = 10;
    profiles.SetBindType(invalidType);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, INVALID_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckBindType_001, testing::ext::TestSize.Level0)
{
    std::string trustDeviceId = "trustDeviceId";
    std::string requestDeviceId = "requestDeviceId";
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    EXPECT_EQ(ret, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckBindType_002, testing::ext::TestSize.Level0)
{
    std::string trustDeviceId = "deviceId";
    std::string requestDeviceId = "requestDeviceId";
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    EXPECT_NE(ret, IDENTICAL_ACCOUNT_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckBindType_003, testing::ext::TestSize.Level0)
{
    std::string trustDeviceId = "deviceId";
    std::string requestDeviceId = "deviceId";
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    EXPECT_NE(ret, IDENTICAL_ACCOUNT_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindTypeByPkgName_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string requestDeviceId;
    std::string trustUdid;
    auto ret = DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindTypeByPkgName_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string requestDeviceId;
    std::string trustUdid = "123456";
    auto ret = DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_001, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_IDENTICAL_ACCOUNT);
    std::string pkgName;
    std::string requestDeviceId;
    std::vector<int32_t> bindTypeVec;
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, pkgName, requestDeviceId, bindTypeVec);
    EXPECT_EQ(bindTypeVec.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_002, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(DEVICE);
    std::string pkgName;
    std::string requestDeviceId;
    std::vector<int32_t> bindTypeVec;
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, pkgName, requestDeviceId, bindTypeVec);
    EXPECT_EQ(bindTypeVec.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_003, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("pkgName");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    std::string pkgName = "pkgName";
    std::string requestDeviceId = "localDeviceId";
    std::vector<int32_t> bindTypeVec;
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, pkgName, requestDeviceId, bindTypeVec);
    EXPECT_EQ(bindTypeVec.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_004, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    std::string pkgName = "pkgName";
    std::string requestDeviceId = "localDeviceId";
    std::vector<int32_t> bindTypeVec;
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, pkgName, requestDeviceId, bindTypeVec);
    EXPECT_EQ(bindTypeVec.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_005, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(DEVICE);
    std::string pkgName;
    std::string requestDeviceId;
    std::vector<int32_t> bindTypeVec;
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, pkgName, requestDeviceId, bindTypeVec);
    EXPECT_EQ(bindTypeVec.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_006, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("pkgName");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    std::string pkgName = "pkgName";
    std::string requestDeviceId = "localDeviceId";
    std::vector<int32_t> bindTypeVec;
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, pkgName, requestDeviceId, bindTypeVec);
    EXPECT_EQ(bindTypeVec.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_007, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    std::string pkgName = "pkgName";
    std::string requestDeviceId = "localDeviceId";
    std::vector<int32_t> bindTypeVec;
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, pkgName, requestDeviceId, bindTypeVec);
    EXPECT_EQ(bindTypeVec.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, CompareBindType_001, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId("deviceId");
    profile.SetStatus(INACTIVE);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.push_back(profile);
    std::string pkgName;
    std::vector<int32_t> sinkBindType;
    std::string localDeviceId;
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().CompareBindType(profiles, pkgName, sinkBindType, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, CompareBindType_002, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId("targetDeviceId");
    profile.SetStatus(ACTIVE);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.push_back(profile);
    std::string pkgName;
    std::vector<int32_t> sinkBindType;
    std::string localDeviceId;
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().CompareBindType(profiles, pkgName, sinkBindType, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, CompareBindType_003, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId("targetDeviceId");
    profile.SetStatus(INACTIVE);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.push_back(profile);
    std::string pkgName;
    std::vector<int32_t> sinkBindType;
    std::string localDeviceId;
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().CompareBindType(profiles, pkgName, sinkBindType, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_001, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_IDENTICAL_ACCOUNT);
    DmDiscoveryInfo paramInfo;
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 0;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, paramInfo, sinkBindType, bindTypeIndex, index);
    EXPECT_EQ(sinkBindType.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_002, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(DEVICE);
    DmDiscoveryInfo paramInfo;
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 0;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, paramInfo, sinkBindType, bindTypeIndex, index);
    EXPECT_EQ(sinkBindType.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_003, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("pkgName");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    DmDiscoveryInfo paramInfo;
    paramInfo.pkgname = "pkgName";
    paramInfo.localDeviceId = "localDeviceId";
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 0;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, paramInfo, sinkBindType, bindTypeIndex, index);
    EXPECT_EQ(sinkBindType.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_004, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo paramInfo;
    paramInfo.pkgname = "pkgName";
    paramInfo.localDeviceId = "localDeviceId";
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 0;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, paramInfo, sinkBindType, bindTypeIndex, index);
    EXPECT_EQ(sinkBindType.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_005, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(DEVICE);
    DmDiscoveryInfo paramInfo;
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 0;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, paramInfo, sinkBindType, bindTypeIndex, index);
    EXPECT_EQ(sinkBindType.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_006, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("pkgName");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    DmDiscoveryInfo paramInfo;
    paramInfo.pkgname = "pkgName";
    paramInfo.localDeviceId = "localDeviceId";
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 0;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, paramInfo, sinkBindType, bindTypeIndex, index);
    EXPECT_EQ(sinkBindType.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_007, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo paramInfo;
    paramInfo.pkgname = "pkgName";
    paramInfo.localDeviceId = "localDeviceId";
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 0;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, paramInfo, sinkBindType, bindTypeIndex, index);
    EXPECT_EQ(sinkBindType.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId;
    std::string targetDeviceId;
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId;
    std::string targetDeviceId = "123456";
    bindTypeVec.push_back(IDENTICAL_ACCOUNT_TYPE);
    bindTypeVec.push_back(DEVICE_PEER_TO_PEER_TYPE);
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "remoteDeviceId";
    bindTypeVec.push_back(DEVICE_ACROSS_ACCOUNT_TYPE);
    bindTypeVec.push_back(APP_ACROSS_ACCOUNT_TYPE);
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "remoteDeviceId";
    bindTypeVec.push_back(DEVICE_PEER_TO_PEER_TYPE);
    bindTypeVec.push_back(IDENTICAL_ACCOUNT_TYPE);
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_001, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "localDeviceId";
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, 100);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_002, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "123456";
    std::string targetDeviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, 100);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_003, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, 100);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, PutAccessControlList_001, testing::ext::TestSize.Level0)
{
    DmAclInfo aclInfo;
    DmAccesser dmAccesser;
    DmAccessee dmAccessee;
    int32_t userId = 123456;
    std::string localDeviceId = "deviceId";
    DeviceProfileConnector::GetInstance().DeleteAclForUserRemoved(localDeviceId, userId);
    int32_t ret = DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    std::string oldAccountId;
    std::string newAccountId;
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_002, testing::ext::TestSize.Level0)
{
    int32_t userId = 123456;
    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_003, testing::ext::TestSize.Level0)
{
    int32_t userId = 123456;
    std::string oldAccountId = "accountId";
    std::string newAccountId = "newAccountId";
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_004, testing::ext::TestSize.Level0)
{
    int32_t userId = 123456;
    std::string oldAccountId = "accountId";
    std::string newAccountId = "accountId";
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckIdenticalAccount_001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    std::string accountId;
    bool ret = DeviceProfileConnector::GetInstance().CheckIdenticalAccount(userId, accountId);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "123456";
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId";
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDevIdInAclForDevBind_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDevIdInAclForDevBind_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "123456";
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDevIdInAclForDevBind_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId";
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckDevIdInAclForDevBind_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteTimeOutAcl_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    uint32_t ret = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(deviceId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTrustNumber_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    int32_t ret = DeviceProfileConnector::GetInstance().GetTrustNumber(deviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, IsSameAccount_001, testing::ext::TestSize.Level0)
{
    std::string udid = "udid";
    int32_t ret = DeviceProfileConnector::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorTest, IsSameAccount_002, testing::ext::TestSize.Level0)
{
    std::string udid = "deviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, GetAuthForm_001, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    std::string trustDev = "";
    std::string reqDev = "";
    int32_t ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, INVALIED_TYPE);
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, IDENTICAL_ACCOUNT_TYPE);
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, DEVICE_PEER_TO_PEER_TYPE);
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, DEVICE_ACROSS_ACCOUNT_TYPE);
    profile.SetBindLevel(APP);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, APP_ACROSS_ACCOUNT_TYPE);
    profile.SetBindType(INVALIED_TYPE);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAccessControlList_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = INVALIED_TYPE;

    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance()
        .DeleteAccessControlList(pkgName, localDeviceId, remoteDeviceId, bindLevel);

    EXPECT_EQ(offlineParam.bindType, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAccessControlList_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = APP;

    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance()
        .DeleteAccessControlList(pkgName, localDeviceId, remoteDeviceId, bindLevel);

    EXPECT_EQ(offlineParam.bindType, APP);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAccessControlList_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = SERVICE;

    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance()
        .DeleteAccessControlList(pkgName, localDeviceId, remoteDeviceId, bindLevel);

    EXPECT_EQ(offlineParam.bindType, SERVICE);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAccessControlList_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = DEVICE;

    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance()
        .DeleteAccessControlList(pkgName, localDeviceId, remoteDeviceId, bindLevel);

    EXPECT_EQ(offlineParam.bindType, DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindLevel_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string localUdid = "localDeviceId";
    std::string udid = "remoteDeviceId";
    uint64_t tokenId = 0;
    int32_t bindLevel = INVALIED_TYPE;

    bindLevel = DeviceProfileConnector::GetInstance()
        .GetBindLevel(pkgName, localUdid, udid, tokenId);

    EXPECT_EQ(bindLevel, DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateBindType_001, testing::ext::TestSize.Level0)
{
    std::string udid = "deviceId";
    int32_t bindType = DEVICE;
    std::map<std::string, int32_t> deviceMap;
    deviceMap[udid] = APP;
    DeviceProfileConnector::GetInstance().UpdateBindType(udid, bindType, deviceMap);
    EXPECT_EQ(deviceMap[udid], DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateBindType_002, testing::ext::TestSize.Level0)
{
    std::string udid = "deviceId";
    int32_t bindType = DEVICE;
    std::map<std::string, int32_t> deviceMap;
    DeviceProfileConnector::GetInstance().UpdateBindType(udid, bindType, deviceMap);
    EXPECT_EQ(deviceMap[udid], DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAccountLogoutEvent_001, testing::ext::TestSize.Level0)
{
    int32_t remoteUserId = 0;
    int32_t bindType = DM_INVALIED_BINDTYPE;
    std::string remoteAccountHash = "remoteAccountHash";
    std::string remoteUdid = "1";
    std::string localUdid = "localDeviceId";

    bindType = DeviceProfileConnector::GetInstance().HandleAccountLogoutEvent(remoteUserId,
        remoteAccountHash, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_INVALIED_BINDTYPE);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDevUnBindEvent_001, testing::ext::TestSize.Level0)
{
    int32_t remoteUserId = 0;
    std::string remoteUdid = "remoteDeviceId";
    std::string localUdid = "localDeviceId";
    int32_t bindType = DM_INVALIED_BINDTYPE;

    bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(remoteUserId, remoteUdid, localUdid);
    EXPECT_EQ(bindType, SERVICE);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAppUnBindEvent_001, testing::ext::TestSize.Level0)
{
    int32_t remoteUserId = 0;
    int32_t tokenId = 0;
    std::string remoteUdid = "remoteDeviceId";
    std::string localUdid = "localDeviceId";
    std::string pkgName = "";
    ProcessInfo res;

    res = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId, localUdid);
    EXPECT_EQ(pkgName, res.pkgName);
}

HWTEST_F(DeviceProfileConnectorTest, SingleUserProcess_001, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    DmAccessCaller caller;
    DmAccessCallee callee;
    int32_t ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, false);
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    profile.accessee_.SetAccesseeBundleName("pkgName");
    profile.accessee_.SetAccesseeDeviceId("localDeviceId");
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(APP);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(SERVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(APP);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(SERVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindType(INVALIED_TYPE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, GetAccessControlProfileByUserId_001, testing::ext::TestSize.Level0)
{
    int32_t userId = DEVICE;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles = DeviceProfileConnector::GetInstance().GetAccessControlProfileByUserId(userId);
    EXPECT_GE(profiles.size(), 0);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAppBindLevel_001, testing::ext::TestSize.Level0)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "bundleName1";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "remoteDeviceId";
    std::string remoteUdid="localDeviceId";
    GetAccessControlProfiles(profiles);
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, APP);

    pkgName = "bundleName2";
    localUdid = "localDeviceId";
    remoteUdid="remoteDeviceId";
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, APP);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteDeviceBindLevel_001, testing::ext::TestSize.Level0)
{
    DmOfflineParam offlineParam;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "remoteDeviceId";
    std::string remoteUdid="localDeviceId";
    GetAccessControlProfiles(profiles);
    DeviceProfileConnector::GetInstance().DeleteDeviceBindLevel(offlineParam, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, DEVICE);

    localUdid = "localDeviceId";
    remoteUdid="remoteDeviceId";
    DeviceProfileConnector::GetInstance().DeleteDeviceBindLevel(offlineParam, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteServiceBindLevel_001, testing::ext::TestSize.Level0)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "bundleName1";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "remoteDeviceId";
    std::string remoteUdid="localDeviceId";
    GetAccessControlProfiles(profiles);
    DeviceProfileConnector::GetInstance().DeleteServiceBindLevel(offlineParam, pkgName, profiles,
        localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, SERVICE);

    pkgName = "bundleName2";
    localUdid = "localDeviceId";
    remoteUdid="remoteDeviceId";
    DeviceProfileConnector::GetInstance().DeleteServiceBindLevel(offlineParam, pkgName, profiles,
        localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, SERVICE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "remoteDeviceId";
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteTimeOutAcl_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "remoteDeviceId";
    uint32_t ret = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(deviceId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTrustNumber_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "remoteDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().GetTrustNumber(deviceId);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, IsSameAccount_003, testing::ext::TestSize.Level0)
{
    std::string udid = "123456";
    int32_t ret = DeviceProfileConnector::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckAccessControl_001, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    std::string srcUdid;
    DmAccessCallee callee;
    std::string sinkUdid;
    int32_t ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    srcUdid = "123456";
    sinkUdid = "123456";
    ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckIsSameAccount_001, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    std::string srcUdid;
    DmAccessCallee callee;
    std::string sinkUdid;
    int32_t ret = DeviceProfileConnector::GetInstance().CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    srcUdid = "123456";
    sinkUdid = "123456";
    ret = DeviceProfileConnector::GetInstance().CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAccountLogoutEvent_002, testing::ext::TestSize.Level0)
{
    int32_t remoteUserId = 0;
    int32_t bindType = DM_INVALIED_BINDTYPE;
    std::string remoteAccountHash = "remoteAccountHash";
    std::string remoteUdid = "123456";
    std::string localUdid = "localDeviceId";

    bindType = DeviceProfileConnector::GetInstance().HandleAccountLogoutEvent(remoteUserId,
        remoteAccountHash, remoteUdid, localUdid);
    EXPECT_NE(bindType, DM_IDENTICAL_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDevUnBindEvent_002, testing::ext::TestSize.Level0)
{
    int32_t remoteUserId = 0;
    std::string remoteUdid;
    std::string localUdid = "localDeviceId";
    int32_t bindType = DM_INVALIED_BINDTYPE;
    bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(remoteUserId, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_INVALIED_BINDTYPE);
}

HWTEST_F(DeviceProfileConnectorTest, GetAllAccessControlProfile_001, testing::ext::TestSize.Level0)
{
    int64_t accessControlId = 1;
    DeviceProfileConnector::GetInstance().DeleteAccessControlById(accessControlId);
    auto ret = DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetAclProfileByDeviceIdAndUserId_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    int32_t userId = 123456;
    auto ret = DeviceProfileConnector::GetInstance().GetAclProfileByDeviceIdAndUserId(deviceId, userId
    );
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAclForAccountLogOut_001, testing::ext::TestSize.Level0)
{
    std::string localUdid;
    int32_t localUserId = 444;
    std::string peerUdid = "deviceId";
    int32_t peerUserId = 555;
    bool ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId,
        peerUdid, peerUserId);
    EXPECT_FALSE(ret);

    localUdid = "deviceId";
    localUserId = 123456;
    peerUdid = "deviceIdee";
    peerUserId = 456;
    ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId, peerUdid, peerUserId);
    EXPECT_TRUE(ret);

    localUdid = "deviceId";
    localUserId = 123456;
    peerUdid = "deviceId";
    peerUserId = 123456;
    ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId, peerUdid, peerUserId);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_005, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "deviceId";
    int32_t userId = 123456;
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId,
        targetDeviceId, userId);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceIdAndBindLevel_001, testing::ext::TestSize.Level0)
{
    std::vector<int32_t> userIds;
    int32_t userId = 123456;
    userIds.push_back(userId);
    userIds.push_back(userId);
    userIds.push_back(userId);
    std::string localUdid = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndBindLevel(userIds, localUdid);
    EXPECT_FALSE(ret.empty());

    localUdid = "remoteDeviceId";
    ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndBindLevel(userIds, localUdid);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceIdAndUserId_001, testing::ext::TestSize.Level0)
{
    int32_t userId = 123456;
    std::string accountId;
    std::string localUdid = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(userId, accountId, localUdid);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, HandleUserSwitched_001, testing::ext::TestSize.Level0)
{
    int32_t currentUserId;
    std::string localUdid = "deviceId";
    int32_t beforeUserId = 123456;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleUserSwitched(localUdid, currentUserId, beforeUserId);
    EXPECT_EQ(ret, DM_OK);

    beforeUserId = 1234;
    currentUserId = 123456;
    ret = DeviceProfileConnector::GetInstance().HandleUserSwitched(localUdid, currentUserId, beforeUserId);
    EXPECT_EQ(ret, DM_OK);

    std::vector<int32_t> remoteUserIds;
    remoteUserIds.push_back(currentUserId);
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> localUserIds;
    localUserIds.push_back(currentUserId);
    std::string localUdid = "deviceId";
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(remoteUserIds, remoteUdid,
        localUserIds, localUdid);
}

HWTEST_F(DeviceProfileConnectorTest, GetOfflineProcessInfo_001, testing::ext::TestSize.Level0)
{
    std::string localUdid = "deviceId";
    std::vector<int32_t> localUserIds;
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> remoteUserIds;
    int32_t userId = 123456;
    remoteUserIds.push_back(userId);
    localUserIds.push_back(userId);
    auto ret = DeviceProfileConnector::GetInstance().GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid,
        remoteUserIds);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetUserIdAndBindLevel_001, testing::ext::TestSize.Level0)
{
    std::string localUdid = "deviceId";
    std::string peerUdid = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetUserIdAndBindLevel(localUdid, peerUdid);
    EXPECT_FALSE(ret.empty());

    int32_t userId = 123456;
    std::vector<int32_t> localUserIds;
    localUserIds.push_back(userId);
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> remoteFrontUserIds;
    remoteFrontUserIds.push_back(userId);
    std::vector<int32_t> remoteBackUserIds;
    remoteBackUserIds.push_back(userId);
    DeviceProfileConnector::GetInstance().UpdateACL(localUdid, localUserIds, remoteUdid, remoteFrontUserIds,
        remoteBackUserIds);
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceIdAndUserId_002, testing::ext::TestSize.Level0)
{
    std::string localUdid = "deviceId";
    int32_t localUserId = 123456;
    auto ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(localUdid, localUserId);
    EXPECT_FALSE(ret.empty());

    std::vector<int32_t> remoteUserIds;
    remoteUserIds.push_back(localUserId);
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> localUserIds;
    localUserIds.push_back(localUserId);
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(remoteUserIds, remoteUdid, localUserIds,
        localUdid);
}
} // namespace DistributedHardware
} // namespace OHOS
