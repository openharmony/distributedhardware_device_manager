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
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(subffixName), Return(DataShare::E_OK)));

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

HWTEST_F(DeviceNameManagerTest, ReleaseDataShareHelper_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DataShareHelperMock> helper = nullptr;
    bool ret = DeviceNameManager::GetInstance().ReleaseDataShareHelper(helper);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceNameManagerTest, ReleaseDataShareHelper_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(false));
    bool ret = DeviceNameManager::GetInstance().ReleaseDataShareHelper(helper_);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceNameManagerTest, ReleaseDataShareHelper_003, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));
    bool ret = DeviceNameManager::GetInstance().ReleaseDataShareHelper(helper_);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DeviceNameManagerTest, MakeUri_001, testing::ext::TestSize.Level1)
{
    std::string proxyUri = "";
    std::string key = "key";
    auto ret = DeviceNameManager::GetInstance().MakeUri(proxyUri, key);
    EXPECT_EQ(ret, Uri(proxyUri + "&key=" + key));
}

HWTEST_F(DeviceNameManagerTest, GetProxyUriStr_001, testing::ext::TestSize.Level1)
{
    std::string tableName = "SETTINGSDATA";
    int32_t userId = 12;
    auto ret = DeviceNameManager::GetInstance().GetProxyUriStr(tableName, userId);
    EXPECT_EQ(ret, "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true");
}

HWTEST_F(DeviceNameManagerTest, GetProxyUriStr_002, testing::ext::TestSize.Level1)
{
    std::string tableName = "tableName";
    int32_t userId = 12;
    auto ret = DeviceNameManager::GetInstance().GetProxyUriStr(tableName, userId);
    EXPECT_EQ(ret, "datashare:///com.ohos.settingsdata/entry/settingsdata/tableName100?Proxy=true");
}

HWTEST_F(DeviceNameManagerTest, SetValue_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);

    std::string tableName = "tableName";
    int32_t userId = 12;
    std::string key = "key";
    std::string value = "value";
    int32_t ret = -1;
    EXPECT_CALL(*helper_, Update(_, _, _)).WillRepeatedly(Return(ret));
    EXPECT_CALL(*helper_, Insert(_, _)).WillRepeatedly(Return(ret));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().SetValue(tableName, userId, key, value);
    EXPECT_EQ(result, -1);
}

HWTEST_F(DeviceNameManagerTest, SetValue_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);

    std::string tableName = "tableName";
    int32_t userId = 12;
    std::string key = "key";
    std::string value = "value";
    int32_t ret = 1;
    EXPECT_CALL(*helper_, Update(_, _, _)).WillRepeatedly(Return(ret));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().SetValue(tableName, userId, key, value);
    EXPECT_EQ(result, 1);
}

HWTEST_F(DeviceNameManagerTest, GetValue_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);

    std::string tableName = "tableName";
    int32_t userId = 12;
    std::string key = "key";
    std::string value = "value";
    std::shared_ptr<DataShareResultSet> resultSet = nullptr;
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().GetValue(tableName, userId, key, value);
    EXPECT_EQ(result, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceNameManagerTest, GetValue_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);

    std::string tableName = "tableName";
    int32_t userId = 12;
    std::string key = "key";
    std::string value = "value";
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GoToRow(_)).Times(AtLeast(1));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(Return(DataShare::E_OK));
    EXPECT_CALL(*resultSet, Close()).Times(AtLeast(1));
    auto result = DeviceNameManager::GetInstance().GetValue(tableName, userId, key, value);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(DeviceNameManagerTest, GetValue_003, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);

    std::string tableName = "tableName";
    int32_t userId = 12;
    std::string key = "key";
    std::string value = "value";
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, GoToRow(_)).Times(AtLeast(1));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(Return(6666));
    EXPECT_CALL(*resultSet, Close()).Times(AtLeast(1));
    auto result = DeviceNameManager::GetInstance().GetValue(tableName, userId, key, value);
    EXPECT_EQ(result, 6666);
}

HWTEST_F(DeviceNameManagerTest, GetValue_004, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);

    std::string tableName = "tableName";
    int32_t userId = 12;
    std::string key = "key";
    std::string value = "value";
    auto resultSet = std::make_shared<DataShareResultSetMock>(nullptr);
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(0), Return(DataShare::E_OK)));
    EXPECT_CALL(*resultSet, Close()).Times(AtLeast(1));
    auto result = DeviceNameManager::GetInstance().GetValue(tableName, userId, key, value);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(DeviceNameManagerTest, GetRemoteObj_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(client_ != nullptr);
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillRepeatedly(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .WillRepeatedly(Return(systemAbilityManager));
    auto ret = DeviceNameManager::GetInstance().GetRemoteObj();
    EXPECT_NE(ret, nullptr);
}

HWTEST_F(DeviceNameManagerTest, SetDeviceName_001, testing::ext::TestSize.Level1)
{
    std::string deviceName = "";
    auto ret = DeviceNameManager::GetInstance().SetDeviceName(deviceName);
    EXPECT_EQ(ret, ERR_DM_NAME_EMPTY);
}

HWTEST_F(DeviceNameManagerTest, SetDeviceName_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    std::string deviceName = "deviceName";
    int32_t ret = 1;
    EXPECT_CALL(*helper_, Update(_, _, _)).WillRepeatedly(Return(ret));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));
    auto result = DeviceNameManager::GetInstance().SetDeviceName(deviceName);
    EXPECT_EQ(result, 1);
}

HWTEST_F(DeviceNameManagerTest, GetDeviceName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);

    std::string deviceName = "deviceName";
    std::shared_ptr<DataShareResultSet> resultSet = nullptr;
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().GetDeviceName(deviceName);
    EXPECT_EQ(result, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceNameManagerTest, SetDisplayDeviceName_001, testing::ext::TestSize.Level1)
{
    std::string deviceName = "";
    int32_t userId = 12;
    auto result = DeviceNameManager::GetInstance().SetDisplayDeviceName(deviceName, userId);
    EXPECT_EQ(result, ERR_DM_NAME_EMPTY);
}

HWTEST_F(DeviceNameManagerTest, SetDisplayDeviceName_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    std::string deviceName = "deviceName";
    int32_t userId = 12;
    int32_t ret = 1;
    EXPECT_CALL(*helper_, Update(_, _, _)).WillRepeatedly(Return(ret));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().SetDisplayDeviceName(deviceName, userId);
    EXPECT_EQ(result, 1);
}

HWTEST_F(DeviceNameManagerTest, SetDisplayDeviceNameState_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    std::string state = "state";
    int32_t userId = 12;
    int32_t ret = 1;
    EXPECT_CALL(*helper_, Update(_, _, _)).WillRepeatedly(Return(ret));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().SetDisplayDeviceNameState(state, userId);
    EXPECT_EQ(result, 1);
}

HWTEST_F(DeviceNameManagerTest, GetDisplayDeviceName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    std::string deviceName = "deviceName";
    int32_t userId = 12;
    
    std::shared_ptr<DataShareResultSet> resultSet = nullptr;
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().GetDisplayDeviceName(userId, deviceName);
    EXPECT_EQ(result, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceNameManagerTest, SetUserDefinedDeviceName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    std::string deviceName = "deviceName";
    int32_t userId = 12;
    
    int32_t ret = 1;
    EXPECT_CALL(*helper_, Update(_, _, _)).WillRepeatedly(Return(ret));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));

    auto result = DeviceNameManager::GetInstance().SetUserDefinedDeviceName(deviceName, userId);
    EXPECT_EQ(result, 1);
}

HWTEST_F(DeviceNameManagerTest, SubstrByBytes_001, testing::ext::TestSize.Level1)
{
    std::string str = "str";
    int32_t maxNumBytes = 12;
    auto result = DeviceNameManager::GetInstance().SubstrByBytes(str, maxNumBytes);
    EXPECT_EQ(result, str);
}

HWTEST_F(DeviceNameManagerTest, SubstrByBytes_002, testing::ext::TestSize.Level1)
{
    std::string str = "HelloWorld";
    int32_t maxNumBytes = 5;
    auto result = DeviceNameManager::GetInstance().SubstrByBytes(str, maxNumBytes);
    EXPECT_EQ(result, "Hello");
}

HWTEST_F(DeviceNameManagerTest, GetUserDefinedDeviceName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(helper_ != nullptr);
    std::string deviceName = "deviceName";
    int32_t userId = 12;
    std::shared_ptr<DataShareResultSet> resultSet = nullptr;
    EXPECT_CALL(*helper_, Query(_, _, _, _)).WillRepeatedly(Return(resultSet));
    EXPECT_CALL(*helper_, Release()).WillRepeatedly(Return(true));
    auto result = DeviceNameManager::GetInstance().GetUserDefinedDeviceName(userId, deviceName);
    EXPECT_EQ(result, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_003, testing::ext::TestSize.Level1)
{
    std::string prefixName = "My";
    std::string subffixName = "Device";
    int32_t maxNameLength = 20;
    std::string result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(prefixName,
        subffixName, maxNameLength);
    EXPECT_EQ(result, "My的Device");
}

HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_004, testing::ext::TestSize.Level1)
{
    std::string prefixName = "MyVeryLong";
    std::string subffixName = "DeviceName";
    int32_t maxNameLength = 15;
    std::string result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(prefixName,
        subffixName, maxNameLength);
    EXPECT_EQ(result, "My...DeviceName");
}

HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_005, testing::ext::TestSize.Level1)
{
    std::string prefixName = "My";
    std::string subffixName = "Device";
    int32_t maxNameLength = 0;
    std::string result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(prefixName,
        subffixName, maxNameLength);
    EXPECT_EQ(result, "My的Device");
}

HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_006, testing::ext::TestSize.Level1)
{
    std::string prefixName = "My";
    std::string subffixName = "VeryLongDeviceNameThatExceedsLimit";
    int32_t maxNameLength = 30;
    std::string result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(prefixName,
        subffixName, maxNameLength);
    EXPECT_EQ(result, "My的VeryLongDevi...");
}

HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_007, testing::ext::TestSize.Level1)
{
    std::string prefixName = "";
    std::string subffixName = "Device";
    int32_t maxNameLength = 10;
    std::string result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(prefixName,
        subffixName, maxNameLength);
    EXPECT_EQ(result, "Device");
}

HWTEST_F(DeviceNameManagerTest, GetLocalDisplayDeviceName_008, testing::ext::TestSize.Level1)
{
    std::string prefixName = "";
    std::string subffixName = "VeryLongDeviceName";
    int32_t maxNameLength = 10;
    std::string result = DeviceNameManager::GetInstance().GetLocalDisplayDeviceName(prefixName,
        subffixName, maxNameLength);
    EXPECT_EQ(result, "VeryLon...");
}

HWTEST_F(DeviceNameManagerTest, DependsIsReady_001, testing::ext::TestSize.Level1)
{
    bool srcDataShareReady = DeviceNameManager::GetInstance().isDataShareReady_;
    DeviceNameManager::GetInstance().isDataShareReady_ = false;
    bool result = DeviceNameManager::GetInstance().DependsIsReady();
    EXPECT_FALSE(result);
    DeviceNameManager::GetInstance().isDataShareReady_ = srcDataShareReady;
}

HWTEST_F(DeviceNameManagerTest, DependsIsReady_002, testing::ext::TestSize.Level1)
{
    bool srcDataShareReady = DeviceNameManager::GetInstance().isDataShareReady_;
    bool srcAccountSysReady = DeviceNameManager::GetInstance().isAccountSysReady_;
    DeviceNameManager::GetInstance().isDataShareReady_ = true;
    DeviceNameManager::GetInstance().isAccountSysReady_ = false;
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(DEFAULT_USER_ID));
    bool result = DeviceNameManager::GetInstance().DependsIsReady();
    EXPECT_FALSE(result);
    DeviceNameManager::GetInstance().isDataShareReady_ = srcDataShareReady;
    DeviceNameManager::GetInstance().isAccountSysReady_ = srcAccountSysReady;
}

HWTEST_F(DeviceNameManagerTest, DependsIsReady_003, testing::ext::TestSize.Level1)
{
    bool srcDataShareReady = DeviceNameManager::GetInstance().isDataShareReady_;
    bool srcAccountSysReady = DeviceNameManager::GetInstance().isAccountSysReady_;
    sptr<IRemoteObject> srcRemoteObj = DeviceNameManager::GetInstance().remoteObj_;
    DeviceNameManager::GetInstance().isDataShareReady_ = true;
    DeviceNameManager::GetInstance().isAccountSysReady_ = false;
    DeviceNameManager::GetInstance().remoteObj_ = nullptr;
    EXPECT_CALL(*multipleUserConnector_, GetCurrentAccountUserID()).WillRepeatedly(Return(0));
    bool result = DeviceNameManager::GetInstance().DependsIsReady();
    EXPECT_TRUE(DeviceNameManager::GetInstance().isAccountSysReady_);
    EXPECT_FALSE(result);
    DeviceNameManager::GetInstance().isDataShareReady_ = srcDataShareReady;
    DeviceNameManager::GetInstance().isAccountSysReady_ = srcAccountSysReady;
    DeviceNameManager::GetInstance().remoteObj_ = srcRemoteObj;
}
} // DistributedHardware
} // OHOS
