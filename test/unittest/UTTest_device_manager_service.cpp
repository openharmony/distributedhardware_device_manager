/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
    const int32_t permsNum = 3;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    const int32_t indexTwo = 2;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[indexTwo] = "ohos.permission.ACCESS_SERVICE_DM";
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
 * @tc.desc: Start device discovery and return ERR_DM_NOT_INIT
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DmSubscribeInfo subscribeInfo;
    std::string extra;
    int ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
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
    std::string extra;
    int ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    pkgName = "1com.ohos.test1";
    ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
}

/**
 * @tc.name: StopDeviceDiscovery_001
 * @tc.desc: Stop device discovery and return ERR_DM_NOT_INIT
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, StopDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 1;
    int ret = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
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
 * @tc.desc: Publish device discovery and return ERR_DM_NOT_INIT
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, PublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test12";
    DmPublishInfo publishInfo;
    publishInfo.publishId = 1;
    publishInfo.mode = DM_DISCOVER_MODE_ACTIVE;
    publishInfo.freq = DM_HIGH;
    publishInfo.ranging = 1;
    int ret = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, DM_OK);
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
 * ERR_DM_DISCOVERY_REPEATED
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
    EXPECT_EQ(ret, ERR_DM_PUBLISH_FAILED);
}

/**
 * @tc.name: UnPublishDeviceDiscovery_001
 * @tc.desc: UnPublish device discovery and return ERR_DM_NOT_INIT
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerServiceTest, UnPublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t publishId = 1;
    int ret = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_EQ(ret, ERR_DM_PUBLISH_FAILED);
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
    std::string pkgName = "com.ohos.test";
    std::string callerId = "com.ohos.test";
    bool isRefresh = true;
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_TRUE(ret == DM_OK || ret == SOFTBUS_NOT_IMPLEMENT);
}

/**
 * @tc.name: ShiftLNNGear_004
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to false; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string callerId = "com.ohos.test";
    bool isRefresh = false;
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh);
    EXPECT_EQ(ret, DM_OK);
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
    std::string deviceId = "2345";
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
 * @tc.desc: Set intFlag for GAuthenticateDevice to True and deviceId to null; Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 0;
    std::string deviceId;
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_001
 * @tc.desc: 将UnAuthenticateDevice的intFlag设置为false，设置pkgName = "com.ohos.test";Return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "12345";
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
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
    std::string deviceId = "12345";
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, deviceId);
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
    std::string deviceId;
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: VerifyAuthentication_001
 * @tc.desc: Set intFlag for VerifyAuthentication to false and set authParam = "jdjjjj"，The return value is
 * ERR_DM_AUTH_NOT_START
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, VerifyAuthentication_001, testing::ext::TestSize.Level0)
{
    std::string authParam = "jdjjjj";
    int ret = DeviceManagerService::GetInstance().VerifyAuthentication(authParam);
    EXPECT_EQ(ret, ERR_DM_AUTH_NOT_START);
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
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUdidByNetworkId_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "111";
    std::string udid = "2222";
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, DM_OK);
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
    std::string pkgName = "";
    std::string netWorkId = "";
    std::string udid = "";
    int ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
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
 * ERR_DM_NOT_INIT
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetUuidByNetworkId_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string netWorkId = "12";
    std::string uuid = "21";
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, DM_OK);
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
    std::string pkgName = "";
    std::string netWorkId = "";
    std::string uuid = "";
    int ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetFaParam_001
 * @tc.desc: Make success for GetFaParam，The return value is
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetFaParam_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DmAuthParam authParam;
    int ret = DeviceManagerService::GetInstance().GetFaParam(pkgName, authParam);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetFaParam_002
 * @tc.desc: Make pkgName empty for GetFaParam, The return value is
 * ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetFaParam_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    DmAuthParam authParam;
    int ret = DeviceManagerService::GetInstance().GetFaParam(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetUserOperation_001
 * @tc.desc: Make success for SetUserOperation，The return value is
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, SetUserOperation_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t action = 0;
    const std::string param = "extra";
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, DM_OK);
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
 * @tc.name: RegisterDevStateCallback_001
 * @tc.desc: Make success for RegisterDevStateCallback，The return value is
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RegisterDevStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "extra";
    int32_t ret = DeviceManagerService::GetInstance().RegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDevStateCallback_002
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RegisterDevStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string extra = "extraTest";
    int32_t ret = DeviceManagerService::GetInstance().RegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_001
 * @tc.desc: Make success for UnRegisterDevStateCallback，The return value is
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterDevStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "extra";
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterDevStateCallback_002
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterDevStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string extra = "extraTest";
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
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
 * @tc.desc:The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_002, testing::ext::TestSize.Level0)
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
 * @tc.name: IsDMServiceImplSoLoaded_001
 * @tc.desc: DeviceManagerService::GetInstance().isImplsoLoaded_ is false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, IsDMServiceImplSoLoaded_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().LoadHardwareFwkService();
    bool ret = DeviceManagerService::GetInstance().IsDMServiceImplSoLoaded();
    EXPECT_EQ(ret, false);
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
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission();
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
 * @tc.name: GetAvailableDeviceList_001
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetAvailableDeviceList_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::vector<DmDeviceBasicInfo> deviceBasicInfoList;
    int32_t ret = DeviceManagerService::GetInstance().GetAvailableDeviceList(pkgName, deviceBasicInfoList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetAvailableDeviceList_002
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetAvailableDeviceList_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::vector<DmDeviceBasicInfo> deviceBasicInfoList;
    int32_t ret = DeviceManagerService::GetInstance().GetAvailableDeviceList(pkgName, deviceBasicInfoList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceNetworkId_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceNetworkId_001, testing::ext::TestSize.Level0)
{
    std::string networkId;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceNetworkId(networkId);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceId_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceId(pkgName, deviceId);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceName_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceName_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceName(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceType_001
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceType_001, testing::ext::TestSize.Level0)
{
    int32_t deviceType = 1;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceType(deviceType);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: StartDeviceDiscovery_004
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    uint16_t subscribeId = 1;
    std::string filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartDeviceDiscovery_005
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, StartDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 1;
    std::string filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: BindDevice_001
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, BindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: BindDevice_002
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, BindDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId;
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: BindDevice_003
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, BindDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnBindDevice_001
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnBindDevice_002
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId;
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnBindDevice_003
 * @tc.desc: The return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId = "1234";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

} // namespace
} // namespace DistributedHardware
} // namespace OHOS
