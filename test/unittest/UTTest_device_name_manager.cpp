/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_device_name_manager.h"
#include "bundle_mgr_mock.h"
#include "datashare_result_set_mock.h"
#include "dm_constants.h"
#include "dm_system_ability_manager_mock.h"

using namespace testing;
using namespace OHOS::DataShare;

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t DEFAULT_USER_ID = -1;
constexpr int32_t DEFAULT_VALUABLE_USER_ID = 0;
const std::string SETTINGS_GENERAL_DEVICE_NAME = "settings.general.device_name";
} // namespace
void DeviceNameManagerTest::SetUp()
{
    multipleUserConnector_ = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnector_;
    auto client = ISystemAbilityManagerClient::GetOrCreateSAMgrClient();
    client_ = std::static_pointer_cast<SystemAbilityManagerClientMock>(client);
    helper_ = DataShareHelperMock::GetOrCreateInstance();
}

void DeviceNameManagerTest::TearDown()
{
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnector_ = nullptr;
    ISystemAbilityManagerClient::ReleaseSAMgrClient();
    client_ = nullptr;
    DataShareHelperMock::ReleaseInstance();
    helper_ = nullptr;
}

void DeviceNameManagerTest::SetUpTestCase()
{}

void DeviceNameManagerTest::TearDownTestCase()
{}

/**
 * @tc.name: Init_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, Init_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(multipleUserConnector_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    std::string subffixName = "手机";
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(DEFAULT_VALUABLE_USER_ID));
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(subffixName), Return(DataShare::E_OK)));

    DeviceNameManager::GetInstance().InitDeviceNameWhenSoftBusReady();
    DeviceNameManager::GetInstance().UnInit();
}

/**
 * @tc.name: InitDeviceNameWhenUserSwitch_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, InitDeviceNameWhenUserSwitch_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(multipleUserConnector_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    std::string subffixName = "手机";
    int32_t curUserId = DEFAULT_VALUABLE_USER_ID + 1;
    int32_t preUserId = DEFAULT_VALUABLE_USER_ID;
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(DEFAULT_VALUABLE_USER_ID));
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(subffixName), Return(DataShare::E_OK)));

    DeviceNameManager::GetInstance().InitDeviceNameWhenSoftBusReady();
    DeviceNameManager::GetInstance().InitDeviceNameWhenUserSwitch(curUserId, preUserId);
    DeviceNameManager::GetInstance().UnInit();
}

/**
 * @tc.name: InitDeviceNameWhenUserSwitch_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, InitDeviceNameWhenUserSwitch_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    std::string subffixName = "手机";
    int32_t curUserId = DEFAULT_USER_ID;
    int32_t preUserId = DEFAULT_USER_ID;

    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(subffixName), Return(DataShare::E_OK)));

    DeviceNameManager::GetInstance().InitDeviceNameWhenUserSwitch(curUserId, preUserId);
    DeviceNameManager::GetInstance().UnInit();
}

/**
 * @tc.name: InitDeviceNameWhenLogout_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, InitDeviceNameWhenLogout_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(multipleUserConnector_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    std::string subffixName = "手机";
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(DEFAULT_VALUABLE_USER_ID));
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));    
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(subffixName), Return(DataShare::E_OK)));

    DeviceNameManager::GetInstance().InitDeviceNameWhenLogout();
    DeviceNameManager::GetInstance().UnInit();
}

/**
 * @tc.name: InitDeviceNameWhenLogin_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, InitDeviceNameWhenLogin_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(multipleUserConnector_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    std::string subffixName = "手机";
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(DEFAULT_VALUABLE_USER_ID));
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GoToRow(_)).Times(AtLeast(1));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(subffixName), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, Close()).Times(AtLeast(1));

    DeviceNameManager::GetInstance().InitDeviceNameWhenLogin();
    DeviceNameManager::GetInstance().UnInit();
}

/**
 * @tc.name: InitDeviceNameWhenNickChange_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, InitDeviceNameWhenNickChange_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(multipleUserConnector_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    std::string subffixName = "手机";
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(DEFAULT_VALUABLE_USER_ID));
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(subffixName), Return(DataShare::E_OK)));

    DeviceNameManager::GetInstance().InitDeviceNameWhenNickChange();
    DeviceNameManager::GetInstance().UnInit();
}

/**
 * @tc.name: InitDeviceNameWhenNameChange_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, InitDeviceNameWhenNameChange_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(multipleUserConnector_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    std::string prefixName = "Mr.诸葛张三";
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));
    EXPECT_CALL(*multipleUserConnector_, GetAccountNickName(_)).WillRepeatedly(Return(prefixName));

    DeviceNameManager::GetInstance().InitDeviceNameWhenNameChange(DEFAULT_VALUABLE_USER_ID);
    DeviceNameManager::GetInstance().UnInit();
}

/**
 * @tc.name: GetLocalDisplayDeviceName_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(multipleUserConnector_ != nullptr);
    ASSERT_TRUE(helper_ != nullptr);

    size_t getStringInvokeCount = 0;
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(DEFAULT_VALUABLE_USER_ID));
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GoToRow(_)).Times(AtLeast(1));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(Invoke([&getStringInvokeCount](int, std::string &output) {
        std::string subffixName = "OH-3.2";
        if (getStringInvokeCount++ > 0) {
            output = subffixName;
        }
        return DM_OK;
    }));
    EXPECT_CALL(*resultSet, Close()).Times(AtLeast(1));
    std::string prefixName = "Mr.诸葛张三";
    EXPECT_CALL(*multipleUserConnector_, GetCallerUserId(_)).Times(AtLeast(1));
    EXPECT_CALL(*multipleUserConnector_, GetAccountNickName(_)).WillRepeatedly(Return(prefixName));
    for (size_t i = 0; i < 2; ++i) {
        int32_t maxNamelength = 24;
        std::string output;
        auto result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(maxNamelength, output);
        EXPECT_EQ(result, DM_OK);
    }
}

/**
 * @tc.name: GetLocalDisplayDeviceName_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_002, testing::ext::TestSize.Level2)
{
    std::string output;
    int32_t maxNamelength = -1;
    auto result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(maxNamelength, output);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);

    maxNamelength = 10;
    result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(maxNamelength, output);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);

    maxNamelength = 101;
    result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(maxNamelength, output);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}
} // DistributedHardware
} // OHOS
