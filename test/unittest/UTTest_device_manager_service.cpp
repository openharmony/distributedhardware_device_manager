/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);

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
}

void DeviceManagerServiceTest::SetUpTestCase()
{
}

void DeviceManagerServiceTest::TearDownTestCase()
{
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

namespace {
/**
 * @tc.name: Init_001
 * @tc.desc: Init device manager service and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, Init_001, testing::ext::TestSize.Level0)
{
    int ret = DeviceManagerService::GetInstance().Init();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: StartDeviceDiscovery_001
 * @tc.desc: Start device discovery and return ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "com.ohos.test";
    DmSubscribeInfo subscribeInfo;
    std::string extra;
    int ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

/**
 * @tc.name: StartDeviceDiscovery_002
 * @tc.desc: Empty pkgName of StartDeviceDiscovery and return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DmSubscribeInfo subscribeInfo;
    std::string extra = "test";
    int ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartDeviceDiscovery_003
 * @tc.desc: Call StartDeviceDiscovery twice with pkgName not null and flag bit not false and return
 * ERR_DM_DISCOVERY_REPEATED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DmSubscribeInfo subscribeInfo;
    std::string extra = "test";
    int ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_REPEATED);
}

/**
 * @tc.name: StopDeviceDiscovery_001
 * @tc.desc: Stop device discovery and return ERR_DM_STOP_REFRESH_LNN_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, StopDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 1;
    int ret = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_STOP_REFRESH_LNN_FAILED);
}

/**
 * @tc.name:StopDeviceDiscovery_002
 * @tc.desc: StopDeviceDiscovery is initialized, pkgName is null, and its return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, StopDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    uint16_t subscribeId = 1;
    int ret = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: PublishDeviceDiscovery_001
 * @tc.desc: Publish device discovery and return ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DmPublishInfo publishInfo;
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: PublishDeviceDiscovery_003
 * @tc.desc: Call PublishDeviceDiscovery twice with pkgName not null and flag bit not false and return
 * ERR_DM_START_ADVERTISING_FAILED
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DmPublishInfo publishInfo;
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    pkgName = "1com.ohos.test1";
    ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, ERR_DM_START_ADVERTISING_FAILED);
}

/**
 * @tc.name: UnPublishDeviceDiscovery_001
 * @tc.desc: UnPublish device discovery and return ERR_DM_STOP_PUBLISH_LNN_FAILED
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, UnPublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t publishId = 1;
    int ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_EQ(ret, ERR_DM_STOP_PUBLISH_LNN_FAILED);
}

/**
 * @tc.name: UnPublishDeviceDiscovery_002
 * @tc.desc: UnPublishDeviceDiscovery is initialized, pkgName is null, and its return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, UnPublishDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t publishId = 1;
    DeviceManagerService::GetInstance().Init();
    int ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetTrustedDeviceList_001
 * @tc.desc:Set the intFlag of GetTrustedDeviceList to true and pkgName = null; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string extra = "jdddd";
    std::vector<DmDeviceInfo> deviceList;
    int ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ShiftLNNGear_001
 * @tc.desc:Set the pkgName to null, callerId not to null, and isRefresh to true; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string callerId = "com.ohos.test";
    bool isRefresh = true;
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ShiftLNNGear_002
 * @tc.desc:Set the callerId to null, pkgNamenot not to null, and isRefresh to true; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string callerId;
    bool isRefresh = true;
    std::vector<DmDeviceInfo> deviceList;
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ShiftLNNGear_003
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to true; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_003, testing::ext::TestSize.Level0)
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
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: ShiftLNNGear_004
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to false; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_004, testing::ext::TestSize.Level0)
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
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ShiftLNNGear_005
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to false; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string callerId = "com.ohos.test";
    bool isRefresh = false;
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ShiftLNNGear_006
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to true; Return NOT DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string callerId = "com.ohos.test";
    bool isRefresh = true;
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: AuthenticateDevice_001
 * @tc.desc: Set unsupport authType = 0 and return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 0;
    std::string deviceId = " 2345";
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_BIND_INPUT_PARA_INVALID);
}

/**
 * @tc.name: AuthenticateDevice_004
 * @tc.desc: Set intFlag for GAuthenticateDevice to True and deviceId to null; Return ERR_DM_NO_PERMISSION
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_004, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId;
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_004
 * @tc.desc: Set intFlag for UnAuthenticateDevice to true and pkgName to com.ohos.test; set deviceId null ，The return
 * value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "12345";
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetUdidByNetworkId_001
 * @tc.desc: Make success for GetUdidByNetworkId，The return value is
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "123";
    std::string udid = "123";
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetUdidByNetworkId_002
 * @tc.desc: Make not init for GetUdidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_004, testing::ext::TestSize.Level0)
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
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "12";
    std::string uuid = "12";
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetUuidByNetworkId_002
 * @tc.desc: Make not init for GetUuidByNetworkId，The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_004, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "com.ohos.test";
    int32_t action = 0;
    const std::string param = "extra";
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
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t action = 0;
    const std::string param = "extra";
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
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t action = 0;
    const std::string param = "extra";
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceInfo_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, RequestCredential_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, RequestCredential_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, RequestCredential_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, ImportCredential_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, ImportCredential_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, ImportCredential_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, RegisterCredentialCallback_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UnRegisterCredentialCallback_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UnRegisterCredentialCallback_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UninitSoftbusListener_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, UninitDMServiceListener_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, IsDMServiceImplReady_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_002, testing::ext::TestSize.Level0)
{
    std::string networkId = "networkIdTest";
    DmDeviceInfo info;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, info);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: CheckApiPermission_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_001, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(0);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDeviceManagerListener_001
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterDeviceManagerListener_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DeviceManagerService::GetInstance().RegisterDeviceManagerListener(pkgName);
    EXPECT_NE(DeviceManagerService::GetInstance().listener_, nullptr);
}

/**
 * @tc.name: RegisterDeviceManagerListener_002
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterDeviceManagerListener_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DeviceManagerService::GetInstance().listener_ = nullptr;
    DeviceManagerService::GetInstance().RegisterDeviceManagerListener(pkgName);
    EXPECT_NE(DeviceManagerService::GetInstance().listener_, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceManagerListener_001
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterDeviceManagerListener_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DeviceManagerService::GetInstance().UnRegisterDeviceManagerListener(pkgName);
    EXPECT_NE(DeviceManagerService::GetInstance().listener_, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceManagerListener_002
 * @tc.desc: Set pkgName null
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterDeviceManagerListener_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DeviceManagerService::GetInstance().listener_ = nullptr;
    DeviceManagerService::GetInstance().UnRegisterDeviceManagerListener(pkgName);
    EXPECT_NE(DeviceManagerService::GetInstance().listener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, GetAvailableDeviceList_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::vector<DmDeviceBasicInfo> deviceBasicInfoList;
    int32_t ret = DeviceManagerService::GetInstance().GetAvailableDeviceList(pkgName, deviceBasicInfoList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetAvailableDeviceList_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::vector<DmDeviceBasicInfo> deviceBasicInfoList;
    int32_t ret = DeviceManagerService::GetInstance().GetAvailableDeviceList(pkgName, deviceBasicInfoList);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceNetworkId_001, testing::ext::TestSize.Level0)
{
    std::string networkId;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceNetworkId(networkId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceId(pkgName, deviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceName_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceName(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceType_001, testing::ext::TestSize.Level0)
{
    int32_t deviceType = 1;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceType(deviceType);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    uint16_t subscribeId = 1;
    std::string filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 1;
    std::string filterOptions;
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    DeviceManagerService::GetInstance().discoveryMgr_ = std::make_shared<DiscoveryManager>(softbusListener, listener);
    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 1;
    std::string filterOptions;
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    DeviceManagerService::GetInstance().discoveryMgr_ = std::make_shared<DiscoveryManager>(softbusListener, listener);
    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_START_DISCOVERING_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t authType = 1;
    std::string deviceId;
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId;
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_BIND_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId;
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId = "1234";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, OnSessionOpened_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DeviceManagerServiceTest, OnPinHolderSessionOpened_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    int sessionId = 0;
    int result = 0;
    void *data = nullptr;
    unsigned int dataLen = 0;
    int ret = DeviceManagerService::GetInstance().OnPinHolderSessionOpened(sessionId, result);
    DeviceManagerService::GetInstance().OnPinHolderBytesReceived(sessionId, data, dataLen);
    DeviceManagerService::GetInstance().OnPinHolderSessionClosed(sessionId);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, MineRequestCredential_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, MineRequestCredential_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string returnJsonStr;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CheckCredential_002, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ImportCredential_004, testing::ext::TestSize.Level0)
{
    DeletePermission();
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ImportCredential_005, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, DeleteCredential_004, testing::ext::TestSize.Level0)
{
    DeletePermission();
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DeleteCredential_005, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, DmHiDumper_001, testing::ext::TestSize.Level0)
{
    std::vector<std::string> args;
    std::string result;
    int32_t ret = DeviceManagerService::GetInstance().DmHiDumper(args, result);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, NotifyEvent_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, NotifyEvent_002, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, LoadHardwareFwkService_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().LoadHardwareFwkService();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string networkId;
    std::string uuid;
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string networkId;
    std::string uuid;
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GenerateEncryptedUuid_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string uuid;
    std::string appId;
    std::string encryptedUuid;
    int32_t ret = DeviceManagerService::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GenerateEncryptedUuid_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string uuid;
    std::string appId;
    std::string encryptedUuid;
    int32_t ret = DeviceManagerService::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_002, testing::ext::TestSize.Level0)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(0);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_003, testing::ext::TestSize.Level0)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(1);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_004, testing::ext::TestSize.Level0)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(2);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_005, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(1);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_006, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(2);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_007, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(3);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::string netWorkId;
    int32_t networkType = 0;
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string netWorkId;
    int32_t networkType = 0;
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string netWorkId = "netWorkId";
    int32_t networkType = 0;
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::string authCode;
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string authCode = "authCode";
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string authCode;
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string authCode = "authCode";
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnloadDMServiceImplSo_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().UnloadDMServiceImplSo();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, IsDMServiceAdapterLoad_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().IsDMServiceAdapterLoad();
    bool ret = DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_EQ(ret, true);
}

HWTEST_F(DeviceManagerServiceTest, UnloadDMServiceAdapter_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().UnloadDMServiceAdapter();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, UnloadDMServiceAdapter_002, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().dmServiceImplExt_ = nullptr;
    DeviceManagerService::GetInstance().UnloadDMServiceAdapter();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_START_DISCOVERING_FAILED);
    DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
}

HWTEST_F(DeviceManagerServiceTest, StartDiscovering_004, testing::ext::TestSize.Level0)
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
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_START_DISCOVERING_FAILED);
    DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
}

HWTEST_F(DeviceManagerServiceTest, StopDiscovering_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StopDiscovering_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StopDiscovering_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_STOP_REFRESH_LNN_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, StartAdvertising_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StartAdvertising_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StartAdvertising_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_META_TYPE] = "metaType";
    advertiseParam[PARAM_KEY_PUBLISH_ID] = "123456";
    advertiseParam[PARAM_KEY_DISC_MODE] =
        std::to_string(static_cast<int32_t>(DmDiscoverMode::DM_DISCOVER_MODE_PASSIVE));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, StopAdvertising_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_STOP_PUBLISH_LNN_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_004, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_005, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnbindTarget_004, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UnbindTarget_005, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnbindTarget_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, DpAclAdd_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string udid;
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, DpAclAdd_002, testing::ext::TestSize.Level0)
{
    std::string udid = "udid";
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string networkId;
    int32_t securityLevel = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, networkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "12345";
    int32_t securityLevel = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "12345";
    int32_t securityLevel = -1;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId;
    int32_t securityLevel = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, IsSameAccount_001, testing::ext::TestSize.Level0)
{
    std::string udid = "";
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, IsSameAccount_002, testing::ext::TestSize.Level0)
{
    std::string udid = "udidTest";
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, IsSameAccount_003, testing::ext::TestSize.Level0)
{
    std::string udid = "udidTest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, InitSoftbusListener_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    DeviceManagerService::GetInstance().InitSoftbusListener();
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, HandleDeviceStatusChange_001, testing::ext::TestSize.Level0)
{
    DmDeviceState devState = DmDeviceState::DEVICE_INFO_READY;
    DmDeviceInfo devInfo;
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(devState, devInfo);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, OnUnbindSessionCloseed_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    DeviceManagerService::GetInstance().OnUnbindSessionCloseed(socket);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, OnUnbindBytesReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string data = "4152413541";
    DeviceManagerService::GetInstance().OnUnbindBytesReceived(socket, data.c_str(), data.size());
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, CheckRelatedDevice_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string udid;
    std::string bundleName;
    int32_t ret = DeviceManagerService::GetInstance().CheckRelatedDevice(udid, bundleName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, CheckRelatedDevice_002, testing::ext::TestSize.Level0)
{
    std::string udid = "123";
    std::string bundleName = "bundleName";
    int32_t ret = DeviceManagerService::GetInstance().CheckRelatedDevice(udid, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, CheckRelatedDevice_003, testing::ext::TestSize.Level0)
{
    std::string udid;
    std::string bundleName;
    int32_t ret = DeviceManagerService::GetInstance().CheckRelatedDevice(udid, bundleName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
