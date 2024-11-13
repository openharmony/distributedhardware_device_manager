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
} // namespace DistributedHardware
} // namespace OHOS
