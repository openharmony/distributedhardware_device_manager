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
}

namespace {
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
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    pkgName = "1com.ohos.test1";
    ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_TRUE(ret == SOFTBUS_IPC_ERR || ret == SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM || ret == DM_OK);
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
    commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN;
    DMAccountInfo dmAccountInfo;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, preUserId);
    commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT;
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
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);
}

/**
 * @tc.name: GetUdidByNetworkId_002
 * @tc.desc: Make not init for GetUdidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string netWorkId = "111";
    std::string udid = "2222";
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetUdidByNetworkId_003
 * @tc.desc: Make pkgName empty for GetUdidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string netWorkId = "";
    std::string udid = "";
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
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
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);
}

/**
 * @tc.name: GetUuidByNetworkId_002
 * @tc.desc: Make not init for GetUuidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string netWorkId = "12";
    std::string uuid = "21";
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetUuidByNetworkId_003
 * @tc.desc: Make pkgName empty for GetUuidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "";
    std::string uuid = "";
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
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
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
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
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, DM_OK);
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
 * @tc.name: GetDeviceInfo_001
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_001, testing::ext::TestSize.Level1)
{
    std::string networkId = "";
    DmDeviceInfo info;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, info);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
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
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName);
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
    DeviceManagerService::GetInstance().listener_ = nullptr;
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName);
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
    DeviceManagerService::GetInstance().UnRegisterCallerAppId(pkgName);
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
    DeviceManagerService::GetInstance().listener_ = nullptr;
    DeviceManagerService::GetInstance().UnRegisterCallerAppId(pkgName);
    EXPECT_EQ(DeviceManagerService::GetInstance().listener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t authType = 1;
    std::string deviceId;
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId;
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*softbusListenerMock_,
        GetTargetInfoFromCache(_, _, _)).WillOnce(Return(ERR_DM_BIND_INPUT_PARA_INVALID));
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_BIND_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId;
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId = "1234";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    std::string extra = "extra";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId;
    std::string extra = "extra";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_007, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId = "1234";
    std::string extra = "extra";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    std::string extra = "extra";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, OnSessionOpened_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    int sessionId = 0;
    int result = 0;
    void *data = nullptr;
    unsigned int dataLen = 0;
    int ret = DeviceManagerService::GetInstance().OnSessionOpened(sessionId, result);
    DeviceManagerService::GetInstance().OnBytesReceived(sessionId, data, dataLen);
    DeviceManagerService::GetInstance().OnSessionClosed(sessionId);
    EXPECT_NE(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceTest, MineRequestCredential_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, MineRequestCredential_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string returnJsonStr;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckCredential_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CheckCredential_002, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ImportCredential_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ImportCredential_005, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, DeleteCredential_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DeleteCredential_005, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, NotifyEvent_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    std::string msg = "";
    DeviceManagerService::GetInstance().HandleDeviceNotTrust(msg);
    msg =  R"(
    {
        "authType" : 1,
        "userId" : "123",
        "credentialData" : "cryptosupportData",
        "CRYPTOSUPPORT" : "cryptosupportTest",
        "credentialType" : 1,
        "credentialId" : "104",
        "NETWORK_ID" : "108",
        "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
        "serverPk" : "hello",
        "pkInfoSignature" : "world",
        "pkInfo" : "pkginfo",
        "peerDeviceId" : "3515656546"
    })";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().HandleDeviceNotTrust(msg);
    std::string commonEventType = "helloworld";
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().ScreenCommonEventCallback(commonEventType);
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, NotifyEvent_002, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, NotifyEvent_003, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t eventId = DM_NOTIFY_EVENT_ON_PINHOLDER_EVENT;
    std::string event;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, LoadHardwareFwkService_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().LoadHardwareFwkService();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string networkId;
    std::string uuid;
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string networkId;
    std::string uuid;
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string networkId = "network_id";
    std::string uuid = "13345689";
    if (DeviceManagerService::GetInstance().softbusListener_ == nullptr) {
        DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    }
    EXPECT_CALL(*softbusListenerMock_, GetUuidByNetworkId(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_002, testing::ext::TestSize.Level1)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(0);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_003, testing::ext::TestSize.Level1)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(1);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(2);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_005, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(1);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_006, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(2);
    EXPECT_NE(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_007, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(3);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string netWorkId;
    int32_t networkType = 0;
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string netWorkId;
    int32_t networkType = 0;
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string netWorkId = "netWorkId";
    int32_t networkType = 0;
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string netWorkId = "netWorkId";
    int32_t networkType = 0;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetNetworkTypeByNetworkId(_, _))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string authCode;
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string authCode;
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_002, testing::ext::TestSize.Level1)
{
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnloadDMServiceImplSo_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().UnloadDMServiceImplSo();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_TRUE(ret == SOFTBUS_IPC_ERR || ret == DM_OK || ret == SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL);
    ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    discoverParam[PARAM_KEY_META_TYPE] = "metaType";
    discoverParam[PARAM_KEY_SUBSCRIBE_ID] = "123456";
    discoverParam[PARAM_KEY_DISC_MEDIUM] =
        std::to_string(static_cast<int32_t>(DmExchangeMedium::DM_AUTO));
    discoverParam[PARAM_KEY_DISC_FREQ] =
        std::to_string(static_cast<int32_t>(DmExchangeFreq::DM_LOW));
    std::map<std::string, std::string> filterOptions;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_TRUE(ret == SOFTBUS_IPC_ERR || ret == DM_OK || ret == SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL);
    DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, StopDiscovering_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StopDiscovering_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StopDiscovering_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_NE(ret, ERR_DM_NO_PERMISSION);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, StartAdvertising_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StartAdvertising_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StartAdvertising_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_META_TYPE] = "metaType";
    advertiseParam[PARAM_KEY_PUBLISH_ID] = "123456";
    advertiseParam[PARAM_KEY_DISC_MODE] =
        std::to_string(static_cast<int32_t>(DmDiscoverMode::DM_DISCOVER_MODE_PASSIVE));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_NE(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "StopAdvertising_004";
    std::map<std::string, std::string> advertiseParam;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_TRUE(ret == SOFTBUS_DISCOVER_COAP_STOP_PUBLISH_FAIL || ret == SOFTBUS_ERR ||
                ret == ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "StopAdvertising_005";
    std::map<std::string, std::string> advertiseParam;
    std::string key = PARAM_KEY_META_TYPE;
    std::string value = "125";
    advertiseParam.insert(std::make_pair(key, value));
    key = PARAM_KEY_PUBLISH_ID;
    value = "128";
    advertiseParam.insert(std::make_pair(key, value));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_NE(ret, DM_OK);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_005, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.wifiIp = "";
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().dmServiceImpl_ = nullptr;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnbindTarget_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UnbindTarget_005, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnbindTarget_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}


HWTEST_F(DeviceManagerServiceTest, DpAclAdd_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string udid;
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DpAclAdd_002, testing::ext::TestSize.Level1)
{
    std::string udid = "udid";
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string networkId;
    int32_t securityLevel = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, networkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "12345";
    int32_t securityLevel = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "12345";
    int32_t securityLevel = -1;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId;
    int32_t securityLevel = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId;
    int32_t securityLevel = 0;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, IsSameAccount_001, testing::ext::TestSize.Level1)
{
    std::string udid = "";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(ERR_DM_INPUT_PARA_INVALID));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, IsSameAccount_002, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(ERR_DM_INPUT_PARA_INVALID));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, IsSameAccount_003, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DeletePermission();
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_FALSE(ret);
}


HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DeletePermission();
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> policy;
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().SetDnPolicy(pkgName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "openharmony123";
    std::map<std::string, std::string> policy;
    DeletePermission();
    int ret = DeviceManagerService::GetInstance().SetDnPolicy(pkgName, policy);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "openharmony123";
    std::map<std::string, std::string> policy;
    std::vector<std::string> peerUdids;
    int32_t userId = 1;
    uint64_t tokenId = 87;
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, USER);
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, APP);
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, 2);
    DeviceManagerService::GetInstance().SendDeviceUnBindBroadCast(peerUdids, userId);
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendDeviceUnBindBroadCast(peerUdids, userId);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().SetDnPolicy(pkgName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string networkId;
    int32_t screenStatus = 1;
    DeletePermission();
    int ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string networkId;
    int32_t screenStatus = 1;
    int ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgname";
    std::string networkId = "networkId_003";
    int32_t screenStatus = 1;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetDeviceScreenStatus(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgname";
    std::string networkId = "networkId_003";
    int32_t screenStatus = 1;
    int ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, HandleDeviceScreenStatusChange_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().HandleDeviceScreenStatusChange(deviceInfo);
    EXPECT_NE(DeviceManagerService::GetInstance().dmServiceImpl_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_002, testing::ext::TestSize.Level1)
{
    DmDeviceInfo info;
    DeletePermission();
    std::string msg = "";
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    bool result = DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_TRUE(result);
    msg =  R"(
    {
        "type" : 0,
        "userId" : 123,
        "accountId" : "28776",
        "tokenId" : 16,
        "peerUdid" : "104",
        "accountName" : "account_xiao"
    })";
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
    msg =  R"(
    {
        "type" : 1,
        "userId" : 128,
        "accountId" : "28778",
        "tokenId" : 17,
        "peerUdid" : "108",
        "accountName" : "account_li"
    })";
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
    msg =  R"(
    {
        "type" : 2,
        "userId" : 124,
        "accountId" : "28779",
        "tokenId" : 18,
        "peerUdid" : "110",
        "accountName" : "account_wang"
    })";
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(info);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkIdByUdid_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udid;
    std::string networkId = "networkIdTest_001";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkIdByUdid_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udid;
    std::string networkId = "networkIdTest_002";
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkIdByUdid_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_003";
    std::string udid = "sewdwed98897";
    std::string networkId = "networkIdTest_003";
    EXPECT_CALL(*softbusCacheMock_, GetNetworkIdFromCache(_, _))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, StopAuthenticateDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StopAuthenticateDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StopAuthenticateDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_003";
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_CALL(*deviceManagerServiceImplMock_, StopAuthenticateDevice(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_005";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    if (DeviceManagerService::GetInstance().pinHolder_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    if (DeviceManagerService::GetInstance().pinHolder_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    if (DeviceManagerService::GetInstance().pinHolder_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ShiftLNNGear_008
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to true; Return NOT DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ShiftLNNGear_008";
    std::string callerId = "ShiftLNNGear_008";
    bool isRefresh = true;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, ShiftLNNGear(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_003, testing::ext::TestSize.Level1)
{
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, IsSameAccount_004, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "networkId";
    int32_t securityLevel = 0;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetDeviceSecurityLevel(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string networkId = "network_id";
    std::string uuid = "13345689";
    if (DeviceManagerService::GetInstance().softbusListener_ == nullptr) {
        DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    }
    EXPECT_CALL(*softbusListenerMock_, GetUuidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*appManagerMock_, GetAppId()).WillOnce(Return("appId"));
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, DM_OK);
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
HWTEST_F(DeviceManagerServiceTest, ConvertUdidHashToAnoyDeviceId_001, testing::ext::TestSize.Level1)
{
    std::string udidHash;
    std::string result;
    EXPECT_CALL(*appManagerMock_, GetAppId()).WillOnce(Return(""));
    int32_t ret = DeviceManagerService::GetInstance().ConvertUdidHashToAnoyDeviceId(udidHash, result);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    EXPECT_CALL(*appManagerMock_, GetAppId()).WillOnce(Return("appId"));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().ConvertUdidHashToAnoyDeviceId(udidHash, result);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_CALL(*appManagerMock_, GetAppId()).WillOnce(Return("appId"));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().ConvertUdidHashToAnoyDeviceId(udidHash, result);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, GetUdidHashByAnoyDeviceId_001, testing::ext::TestSize.Level1)
{
    std::string udidHash;
    std::string anoyDeviceId = "anoyDeviceId";
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().GetUdidHashByAnoyDeviceId(anoyDeviceId, udidHash);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().GetUdidHashByAnoyDeviceId(anoyDeviceId, udidHash);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
#endif
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
