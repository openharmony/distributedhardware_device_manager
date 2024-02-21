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

HWTEST_F(DeviceProfileConnectorTest, CompareBindType_001, testing::ext::TestSize.Level0)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string pkgName;
    std::vector<int32_t> sinkBindType;
    std::string localDeviceId;
    std::string targetDeviceId;
    auto ret = DeviceProfileConnector::GetInstance().CompareBindType(profiles, pkgName, sinkBindType, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
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
} // namespace DistributedHardware
} // namespace OHOS
