/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 * @tc.name: RegisterCallerAppId_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterCallerAppId_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DeviceManagerService::GetInstance().listener_ = std::make_shared<DeviceManagerServiceListener>();
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName);
    EXPECT_EQ(DeviceManagerService::GetInstance().listener_ != nullptr, true);
}

/**
 * @tc.name: GetTrustedDeviceList_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_201, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    const std::string extra;
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetDeviceInfo_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_201, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string networkId = "123";
    DmDeviceInfo deviceIndo;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceIndo);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
    ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(deviceIndo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetLocalDeviceInfo_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, StopDeviceDiscovery_201, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    uint16_t subscribeId = 111;
    int32_t ret = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

/**
 * @tc.name: RegisterUiStateCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_201, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

/**
 * @tc.name: UnRegisterUiStateCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_201, testing::ext::TestSize.Level0)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

/**
 * @tc.name: IsDMImplSoLoaded_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, IsDMImplSoLoaded_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_META_TYPE] = "1";
    DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    std::map<std::string, std::string> advertiseParam2;
    advertiseParam[PARAM_KEY_PUBLISH_ID] = "12";
    DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam2);
    PeerTargetId targetId;
    targetId.deviceId = "123";
    std::map<std::string, std::string> bindParam;
    bindParam[PARAM_KEY_META_TYPE] = "1";
    DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    targetId.wifiIp = "192.168.1.1";
    std::map<std::string, std::string> bindParam2;
    DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam2);
    targetId.deviceId = "123";
    std::map<std::string, std::string> unbindParam;
    unbindParam[PARAM_KEY_META_TYPE] = "1";
    DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    DeviceManagerService::GetInstance().IsDMServiceImplReady();
    bool ret = DeviceManagerService::GetInstance().IsDMImplSoLoaded();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: RegisterPinHolderCallback_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "CollaborationFwk";
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: CreatePinHolder_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "CollaborationFwk";
    PeerTargetId targetId;
    DmPinType pinType = NUMBER_PIN_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetDeviceSecurityLevel_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string networkId = "123";
    int32_t securityLevel;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, networkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: CheckAccessControl_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_201, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckAccessControl_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_202, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DeletePermission();
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckIsSameAccount_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckIsSameAccount_201, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckIsSameAccount_202
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckIsSameAccount_202, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DeletePermission();
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: InitAccountInfo_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, InitAccountInfo_201, testing::ext::TestSize.Level0)
{
    int32_t userId = 100;
    std::string commonEventType = "usual.event.USER_SWITCHED";
    DeviceManagerService::GetInstance().AccountCommonEventCallback(userId, commonEventType);
    commonEventType = "common.event.HWID_LOGIN";
    DeviceManagerService::GetInstance().AccountCommonEventCallback(userId, commonEventType);
    int32_t ret = DeviceManagerService::GetInstance().InitAccountInfo();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: InitScreenLockEvent_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, InitScreenLockEvent_201, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerService::GetInstance().InitScreenLockEvent();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SubscribeAccountCommonEvent_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SubscribeAccountCommonEvent_201, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().SubscribeAccountCommonEvent();
    DeviceManagerService::GetInstance().SubscribeScreenLockEvent();
    std::string commonEventType = "usual.event.USER_SWITCHED";
    DeviceManagerService::GetInstance().ScreenCommonEventCallback(commonEventType);
    EXPECT_NE(DeviceManagerService::GetInstance().accountCommonEventManager_, nullptr);
}

/**
 * @tc.name: SetDnPolicy_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_201, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    bool bRet = false;
    if (ret == DM_OK || ret == ERR_DM_UNSUPPORTED_METHOD) {
        bRet = true;
    }
    ASSERT_EQ(bRet, true);
}

/**
 * @tc.name: SetDnPolicy_202
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_202, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_203
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_203, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_204
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_204, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_205
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_205, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "a100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_206
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_206, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100a";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.screenStatusTest";
    std::string networkId = "";
    int32_t screenStatus = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string networkId = "networkIdTest";
    int32_t screenStatus = -1;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS