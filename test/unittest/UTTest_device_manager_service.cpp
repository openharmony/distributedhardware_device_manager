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

#include "UTTest_device_manager_service.h"
#include "accesstoken_kit.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"
#include "softbus_error_code.h"

using namespace OHOS::Security::AccessToken;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);

void DeviceManagerServiceTest::SetUp()
{
    const int32_t permsNum = 4;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    const int32_t indexTwo = 2;
    const int32_t indexThree = 3;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[indexTwo] = "ohos.permission.ACCESS_SERVICE_DM";
    perms[indexThree] = "ohos.permission.MONITOR_DEVICE_NETWORK_STATE";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dsoftbus_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void DeviceManagerServiceTest::TearDown()
{
    Mock::VerifyAndClearExpectations(permissionManagerMock_.get());
    Mock::VerifyAndClearExpectations(softbusListenerMock_.get());
    Mock::VerifyAndClearExpectations(kVAdapterManagerMock_.get());
    Mock::VerifyAndClearExpectations(appManagerMock_.get());
    Mock::VerifyAndClearExpectations(deviceManagerServiceImplMock_.get());
    Mock::VerifyAndClearExpectations(softbusCacheMock_.get());
    Mock::VerifyAndClearExpectations(cryptoMock_.get());
    Mock::VerifyAndClearExpectations(multipleUserConnectorMock_.get());
    Mock::VerifyAndClearExpectations(dMCommToolMock_.get());
    Mock::VerifyAndClearExpectations(deviceProfileConnectorMock_.get());
    Mock::VerifyAndClearExpectations(deviceNameManagerMock_.get());
}

void DeviceManagerServiceTest::SetUpTestCase()
{
    DmPermissionManager::dmPermissionManager = permissionManagerMock_;
    DmSoftbusListener::dmSoftbusListener = softbusListenerMock_;
    DmKVAdapterManager::dmKVAdapterManager = kVAdapterManagerMock_;
    DmAppManager::dmAppManager = appManagerMock_;
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl = deviceManagerServiceImplMock_;
    DmSoftbusCache::dmSoftbusCache = softbusCacheMock_;
    DmCrypto::dmCrypto = cryptoMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    DmDMCommTool::dmDMCommTool = dMCommToolMock_;
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock_;
    DmDeviceNameManager::dmDeviceNameManager_ = deviceNameManagerMock_;
}

void DeviceManagerServiceTest::TearDownTestCase()
{
    DmPermissionManager::dmPermissionManager = nullptr;
    permissionManagerMock_ = nullptr;
    DmSoftbusListener::dmSoftbusListener = nullptr;
    softbusListenerMock_ = nullptr;
    DmKVAdapterManager::dmKVAdapterManager = nullptr;
    kVAdapterManagerMock_ = nullptr;
    DmAppManager::dmAppManager = nullptr;
    appManagerMock_ = nullptr;
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl = nullptr;
    deviceManagerServiceImplMock_ = nullptr;
    DmSoftbusCache::dmSoftbusCache = nullptr;
    softbusCacheMock_ = nullptr;
    DmCrypto::dmCrypto = nullptr;
    cryptoMock_ = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
    DmDMCommTool::dmDMCommTool = nullptr;
    dMCommToolMock_ = nullptr;
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    deviceProfileConnectorMock_ = nullptr;
    DmDeviceNameManager::dmDeviceNameManager_ = nullptr;
    deviceNameManagerMock_ = nullptr;
}

void DeletePermission()
{
    const int32_t permsNum = 1;
    const int32_t indexZero = 0;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "DeviceManagerServiceTest",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

/**
 * @tc.name: InitDMServiceListener_001
 * @tc.desc: Init device manager listener and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */

/**
 * @tc.name: InitDMServiceListener_001
 * @tc.desc: Init device manager listener and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, InitDMServiceListener_001, testing::ext::TestSize.Level1)
{
    int ret = DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
/**
 * @tc.name: PublishDeviceDiscovery_001
 * @tc.desc: Publish device discovery and return ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName = "com.ohos.test12";
    DmPublishInfo publishInfo;
    publishInfo.publishId = 1;
    publishInfo.mode = DM_DISCOVER_MODE_ACTIVE;
    publishInfo.freq = DM_HIGH;
    publishInfo.ranging = 1;
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
    DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishInfo.publishId);
}
/**
 * @tc.name: PublishDeviceDiscovery_002
 * @tc.desc: Empty pkgName of PublishDeviceDiscovery and return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DmPublishInfo publishInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: PublishDeviceDiscovery_003
 * @tc.desc: Call PublishDeviceDiscovery twice with pkgName not null and flag bit not false and return
 * SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmPublishInfo publishInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    pkgName = "1com.ohos.test1";
    ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
/**
 * @tc.name: PublishDeviceDiscovery_004
 * @tc.desc: Call PublishDeviceDiscovery twice with pkgName not null and flag bit not false and return
 * SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "PublishDeviceDiscovery_004";
    DmPublishInfo publishInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    pkgName = "1com.ohos.test1";
    ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_FALSE(ret == SOFTBUS_IPC_ERR || ret == SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM || ret == DM_OK);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
/**
 * @tc.name: UnPublishDeviceDiscovery_001
 * @tc.desc: UnPublish device discovery and return SOFTBUS_ERR
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, UnPublishDeviceDiscovery_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t publishId = 1;
    int ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_NE(ret, DM_OK);
}
/**
 * @tc.name: UnPublishDeviceDiscovery_002
 * @tc.desc: UnPublishDeviceDiscovery is initialized, pkgName is null, and its return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, UnPublishDeviceDiscovery_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t publishId = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
/**
 * @tc.name: UnPublishDeviceDiscovery_003
 * @tc.desc: UnPublishDeviceDiscovery is initialized, pkgName is null, and its return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, UnPublishDeviceDiscovery_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test003";
    int32_t publishId = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_TRUE(ret == SOFTBUS_DISCOVER_COAP_STOP_PUBLISH_FAIL || ret == SOFTBUS_ERR ||
                ret == ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
/**
 * @tc.name: UnPublishDeviceDiscovery_004
 * @tc.desc: UnPublishDeviceDiscovery is initialized, pkgName is null, and its return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, UnPublishDeviceDiscovery_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test003";
    int32_t publishId = 1;
    int32_t userId = 23;
    std::string accountId = "hello123";
    int32_t preUserId = 3;
    std::vector<std::string> peerUdids;
    std::string accountName = "openharmony123";
    std::map<std::string, int32_t> curUserDeviceMap;
    std::map<std::string, int32_t> preUserDeviceMap;
    std::multimap<std::string, int32_t> curMultiMap;
    std::string commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, preUserId);
    commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN;
    DMAccountInfo dmAccountInfo;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, preUserId);
    commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT;
    EXPECT_CALL(*multipleUserConnectorMock_, GetAccountInfoByUserId(_)).WillOnce(Return(dmAccountInfo));
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, preUserId);
    commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED;
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetDeviceIdAndUserId(_)).WillOnce(Return(curMultiMap));
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, preUserId);
    commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, preUserId);
    DeviceManagerService::GetInstance().HandleAccountLogout(userId, accountId, pkgName);
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetDeviceIdAndUserId(_)).WillOnce(Return(curMultiMap));
    DeviceManagerService::GetInstance().HandleUserRemoved(preUserId);
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendAccountLogoutBroadCast(peerUdids, accountId, accountName, userId);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    DeletePermission();
    int ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: GetTrustedDeviceList_001
 * @tc.desc:Set the intFlag of GetTrustedDeviceList to true and pkgName = null; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string extra = "jdddd";
    std::vector<DmDeviceInfo> deviceList;
    int ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: GetTrustedDeviceList_002
 * @tc.desc:Set the intFlag of GetTrustedDeviceList to true; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos_test";
    std::string extra = "jdddd";
    std::vector<DmDeviceInfo> deviceList;
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    int ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
/**
 * @tc.name: GetTrustedDeviceList_003
 * @tc.desc:Set the intFlag of GetTrustedDeviceList to true; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos_test";
    std::string extra = "jdddd";
    std::vector<DmDeviceInfo> deviceList;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_)).WillOnce(Return(DM_OK));
    int ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}
/**
 * @tc.name: ShiftLNNGear_001
 * @tc.desc:Set the pkgName to null, callerId not to null, and isRefresh to true; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string callerId = "com.ohos.test";
    bool isRefresh = true;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: ShiftLNNGear_002
 * @tc.desc:Set the callerId to null, pkgNamenot not to null, and isRefresh to true; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string callerId;
    bool isRefresh = true;
    std::vector<DmDeviceInfo> deviceList;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: ShiftLNNGear_003
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to true; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_003, testing::ext::TestSize.Level1)
{
    const int32_t permsNum = 1;
    const int32_t indexZero = 0;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "DeviceManagerServiceTest",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    std::string pkgName = "com.ohos.test";
    std::string callerId = "com.ohos.test";
    bool isRefresh = true;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    EXPECT_NE(ret, DM_OK);
}
/**
 * @tc.name: ShiftLNNGear_004
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to false; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_004, testing::ext::TestSize.Level1)
{
    const int32_t permsNum = 1;
    const int32_t indexZero = 0;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "DeviceManagerServiceTest",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    std::string pkgName = "com.ohos.test";
    std::string callerId = "com.ohos.test";
    bool isRefresh = false;
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: ShiftLNNGear_005
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to false; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string callerId = "com.ohos.test";
    bool isRefresh = false;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    EXPECT_EQ(ret, DM_OK);
}
/**
 * @tc.name: ShiftLNNGear_006
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to true; Return NOT DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ShiftLNNGear_006";
    std::string callerId = "ShiftLNNGear_006";
    bool isRefresh = true;
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
/**
 * @tc.name: ShiftLNNGear_007
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to true; Return NOT DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ShiftLNNGear_007";
    std::string callerId = "ShiftLNNGear_007";
    bool isRefresh = true;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, ShiftLNNGear(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
/**
 * @tc.name: AuthenticateDevice_001
 * @tc.desc: Set unsupport authType = 0 and return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 1;
    std::string deviceId;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: AuthenticateDevice_002
 * @tc.desc: Set intFlag for GAuthenticateDevice to True and pkgName to null; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string extra = "jdddd";
    int32_t authType = 0;
    std::string deviceId = " 2345";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: AuthenticateDevice_003
 * @tc.desc: Set intFlag for GAuthenticateDevice to True and deviceId to null; Return ERR_DM_BIND_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 0;
    std::string deviceId = " 2345";
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*softbusListenerMock_,
        GetTargetInfoFromCache(_, _, _)).WillOnce(Return(ERR_DM_BIND_INPUT_PARA_INVALID));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_BIND_INPUT_PARA_INVALID);
}
/**
 * @tc.name: AuthenticateDevice_004
 * @tc.desc: Set intFlag for GAuthenticateDevice to True and deviceId to null; Return ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 0;
    std::string deviceId = " 2345";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: UnAuthenticateDevice_001
 * @tc.desc: 将UnAuthenticateDevice的intFlag设置为false，设置pkgName = "com.ohos.test";Return ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName = "com.ohos.test";
    std::string networkId = "12345";
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: UnAuthenticateDevice_002
 * @tc.desc:  Set intFlag for UnAuthenticateDevice to True and pkgName to null; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string networkId = "12345";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: UnAuthenticateDevice_003
 * @tc.desc: Set intFlag for UnAuthenticateDevice to true and pkgName to com.ohos.test; set deviceId null ，The return
 * value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: UnAuthenticateDevice_004
 * @tc.desc: Set intFlag for UnAuthenticateDevice to true and pkgName to com.ohos.test; set deviceId null ，The return
 * value is SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "12345";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
/**
 * @tc.name: GetUdidByNetworkId_001
 * @tc.desc: Make success for GetUdidByNetworkId，The return value is
 * SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "123";
    std::string udid = "123";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(SOFTBUS_IPC_ERR));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);
}
/**
 * @tc.name: GetUdidByNetworkId_004
 * @tc.desc: Make pkgName empty for GetUdidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string netWorkId = "123";
    std::string udid = "123";
    DeletePermission();
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: GetUuidByNetworkId_001
 * @tc.desc: Make success for GetUuidByNetworkId，The return value is
 * SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "12";
    std::string uuid = "12";
    EXPECT_CALL(*softbusListenerMock_, GetUuidByNetworkId(_, _)).WillOnce(Return(SOFTBUS_IPC_ERR));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);
}
/**
 * @tc.name: GetUuidByNetworkId_004
 * @tc.desc: Make pkgName empty for GetUuidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "";
    std::string uuid = "";
    DeletePermission();
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: SetUserOperation_001
 * @tc.desc: Make success for SetUserOperation，The return value is
 * ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName = "com.ohos.test";
    int32_t action = 0;
    const std::string param = "{\"test\": \"extra\"}";;
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: SetUserOperation_002
 * @tc.desc: Make pkgName empty for SetUserOperation，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    int32_t action = 0;
    const std::string param = "{\"test\": \"extra\"}";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: SetUserOperation_003
 * @tc.desc: Make pkgName empty for SetUserOperation，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int32_t action = 0;
    const std::string param;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: SetUserOperation_004
 * @tc.desc: Make pkgName empty for SetUserOperation，The return value is
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int32_t action = 0;
    const std::string param = R"({"test":"extra"})";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: GetLocalDeviceInfo_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo info;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(info);
    EXPECT_EQ(ret, DM_OK);
}
/**
 * @tc.name: RequestCredential_001
 * @tc.desc:The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RequestCredential_001, testing::ext::TestSize.Level1)
{
    const std::string reqJsonStr = "test";
    std::string returnJsonStr = "returntest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
/**
 * @tc.name: RequestCredential_002
 * @tc.desc:The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RequestCredential_002, testing::ext::TestSize.Level1)
{
    const std::string reqJsonStr = "";
    std::string returnJsonStr = "returntest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: RequestCredential_003
 * @tc.desc:The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RequestCredential_003, testing::ext::TestSize.Level1)
{
    const std::string reqJsonStr = "test";
    std::string returnJsonStr = "returntest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: ImportCredential_001
 * @tc.desc:The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ImportCredential_001, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    const std::string credentialInfo = "credentialInfotest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
/**
 * @tc.name: ImportCredential_002
 * @tc.desc:The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ImportCredential_002, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "";
    const std::string credentialInfo = "";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: ImportCredential_003
 * @tc.desc:The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ImportCredential_003, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    const std::string credentialInfo = "credentialInfotest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: DeleteCredential_001
 * @tc.desc:The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_001, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    const std::string deleteInfo = "deleteInfotest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
/**
 * @tc.name: DeleteCredential_002
 * @tc.desc:The return value is ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_002, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    const std::string deleteInfo = "deleteInfotest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: DeleteCredential_003
 * @tc.desc:The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_003, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "";
    const std::string deleteInfo = "";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: RegisterCredentialCallback_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
/**
 * @tc.name: RegisterCredentialCallback_002
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: RegisterCredentialCallback_003
 * @tc.desc: The return value is ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RegisterCredentialCallback_003, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: UnRegisterCredentialCallback_001
 * @tc.desc:The return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterCredentialCallback_001, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
/**
 * @tc.name: UnRegisterCredentialCallback_002
 * @tc.desc:The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
/**
 * @tc.name: UnRegisterCredentialCallback_003
 * @tc.desc:The return value is ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterCredentialCallback_003, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: UninitSoftbusListener_001
 * @tc.desc: DeviceManagerService::GetInstance().softbusListener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UninitSoftbusListener_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().UninitSoftbusListener();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}
/**
 * @tc.name: UninitDMServiceListener_001
 * @tc.desc: DeviceManagerService::GetInstance().listener_ is nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UninitDMServiceListener_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().listener_ = std::make_shared<DeviceManagerServiceListener>();
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    EXPECT_EQ(DeviceManagerService::GetInstance().listener_, nullptr);
}
/**
 * @tc.name: IsDMServiceImplReady_001
 * @tc.desc: The return value is true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, IsDMServiceImplReady_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    bool ret = DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_EQ(ret, true);
}
/**
 * @tc.name: GetDeviceInfo_002
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_002, testing::ext::TestSize.Level1)
{
    std::string networkId = "networkIdTest";
    DmDeviceInfo info;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, info);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
/**
 * @tc.name: GetDeviceInfo_004
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_004, testing::ext::TestSize.Level1)
{
    std::string networkId = "networkIdTest4";
    DmDeviceInfo info;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, info);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
/**
 * @tc.name: CheckApiPermission_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(0);
    EXPECT_EQ(ret, DM_OK);
}
/**
 * @tc.name: RegisterCallerAppId_001
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterCallerAppId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t userId = 10001;
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName, userId);
    EXPECT_NE(DeviceManagerService::GetInstance().listener_, nullptr);
}
/**
 * @tc.name: RegisterCallerAppId_002
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterCallerAppId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t userId = 10001;
    DeviceManagerService::GetInstance().listener_ = nullptr;
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName, userId);
    EXPECT_EQ(DeviceManagerService::GetInstance().listener_, nullptr);
}
/**
 * @tc.name: UnRegisterDeviceManagerListener_001
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterCallerAppId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t userId = 10001;
    DeviceManagerService::GetInstance().UnRegisterCallerAppId(pkgName, userId);
    EXPECT_EQ(DeviceManagerService::GetInstance().listener_, nullptr);
}
/**
 * @tc.name: UnRegisterDeviceManagerListener_002
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterCallerAppId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t userId = 10001;
    DeviceManagerService::GetInstance().listener_ = nullptr;
    DeviceManagerService::GetInstance().UnRegisterCallerAppId(pkgName, userId);
    EXPECT_EQ(DeviceManagerService::GetInstance().listener_, nullptr);
}

} // namespace DistributedHardware
} // namespace OHOS
