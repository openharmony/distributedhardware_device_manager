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

HWTEST_F(DeviceProfileConnectorTest, GetAccessControlProfile_001, testing::ext::TestSize.Level0)
{
    auto ret = DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
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
    std::string trustDeviceId;
    std::string requestDeviceId;
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    EXPECT_EQ(ret, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindTypeByPkgName_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string requestDeviceId;
    std::string trustUdid;
    auto ret = DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    EXPECT_EQ(ret.empty(), true);
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

HWTEST_F(DeviceProfileConnectorTest, GetPkgNameFromAcl_001, testing::ext::TestSize.Level0)
{
    std::string localDeviceId;
    std::string targetDeviceId;
    auto ret = DeviceProfileConnector::GetInstance().GetPkgNameFromAcl(localDeviceId, targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetOfflineParamFromAcl_001, testing::ext::TestSize.Level0)
{
    std::string trustDeviceId;
    std::string requestDeviceId;
    auto ret = DeviceProfileConnector::GetInstance().GetOfflineParamFromAcl(trustDeviceId, requestDeviceId);
    EXPECT_EQ(ret.bindType, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, PutAccessControlList_001, testing::ext::TestSize.Level0)
{
    DmAclInfo aclInfo;
    DmAccesser dmAccesser;
    DmAccessee dmAccessee;
    int32_t ret = DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAccessControlList_001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    std::string accountId;
    int32_t ret = DeviceProfileConnector::GetInstance().DeleteAccessControlList(userId, accountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAccessControlList_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string localDeviceId;
    std::string remoteDeviceId;
    auto ret = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId, remoteDeviceId);
    EXPECT_EQ(ret.bindType, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    std::string oldAccountId;
    std::string newAccountId;
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckIdenticalAccount_001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    std::string accountId;
    bool ret = DeviceProfileConnector::GetInstance().CheckIdenticalAccount(userId, accountId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteP2PAccessControlList_001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    std::string accountId;
    int32_t ret = DeviceProfileConnector::GetInstance().DeleteP2PAccessControlList(userId, accountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDeviceIdInAcl_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDeviceIdInAcl(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDeviceIdInAcl_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDeviceIdInAcl(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckDeviceIdInAcl_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckDeviceIdInAcl(pkgName, deviceId);
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

HWTEST_F(DeviceProfileConnectorTest, CheckPkgnameInAcl_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string localDeviceId;
    std::string remoteDeviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckPkgnameInAcl(pkgName, localDeviceId, remoteDeviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckRelatedDevice_001, testing::ext::TestSize.Level0)
{
    std::string udid = "123";
    std::string bundleName = "bundleName";
    int32_t ret = DeviceProfileConnector::GetInstance().CheckRelatedDevice(udid, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorTest, IsTrustDevice_001, testing::ext::TestSize.Level0)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    std::string udid = "123";
    std::string bundleName = "bundleName";
    bool ret = DeviceProfileConnector::GetInstance().IsTrustDevice(profiles, udid, bundleName);
    EXPECT_EQ(ret, false);
}
} // namespace DistributedHardware
} // namespace OHOS
