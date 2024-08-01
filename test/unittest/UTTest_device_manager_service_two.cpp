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
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, Init_001, testing::ext::TestSize.Level0)
{
    int ret = DeviceManagerService::GetInstance().Init();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: InitSoftbusListener_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, InitSoftbusListener_001, testing::ext::TestSize.Level0)
{
    int ret = DeviceManagerService::GetInstance().InitSoftbusListener();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SubscribePublishCommonEvent_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, SubscribePublishCommonEvent_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().SubscribePublishCommonEvent();
    EXPECT_NE(DeviceManagerService::GetInstance().publshCommonEventManager_, nullptr);
}

/**
 * @tc.name: RegisterDeviceManagerListener_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RegisterDeviceManagerListener_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().listener_ = std::make_shared<DeviceManagerServiceListener>();
    int ret = DeviceManagerService::GetInstance().RegisterDeviceManagerListener();
    EXPECT_NE(DeviceManagerService::GetInstance().listener_ , nullptr);
}

/**
 * @tc.name: RegisterUiStateCallback_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

/**
 * @tc.name: UnRegisterUiStateCallback_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

/**
 * @tc.name: IsDMImplSoLoaded_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, IsDMImplSoLoaded_001, testing::ext::TestSize.Level0)
{
    bool ret = DeviceManagerService::GetInstance().IsDMImplSoLoaded();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetEncryptedUuidByNetworkId_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string networkId = "123";
    std::string uuid;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: CheckAccessControl_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_001, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckIsSameAccount_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, CheckIsSameAccount_001, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: InitAccountInfo_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, InitAccountInfo_001, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerService::GetInstance().InitAccountInfo();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: InitScreenLockEvent_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, InitScreenLockEvent_001, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerService::GetInstance().InitScreenLockEvent();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SubscribeAccountCommonEvent_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, SubscribeAccountCommonEvent_001, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().SubscribeAccountCommonEvent();
    EXPECT_NE(DeviceManagerService::GetInstance().accountCommonEventManager_, nullptr);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
