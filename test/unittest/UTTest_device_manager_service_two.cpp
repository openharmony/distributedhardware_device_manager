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
 * @tc.name: InitSoftbusListener_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, InitSoftbusListener_201, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    int32_t ret = DeviceManagerService::GetInstance().InitSoftbusListener();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDeviceManagerListener_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterDeviceManagerListener_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DeviceManagerService::GetInstance().listener_ = std::make_shared<DeviceManagerServiceListener>();
    DeviceManagerService::GetInstance().RegisterDeviceManagerListener(pkgName);
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
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
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
}

/**
 * @tc.name: GetLocalDeviceInfo_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_201, testing::ext::TestSize.Level0)
{
    DeletePermission();
    DmDeviceInfo deviceIndo;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(deviceIndo);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
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
    DeviceManagerService::GetInstance().IsDMServiceImplReady();
    bool ret = DeviceManagerService::GetInstance().IsDMImplSoLoaded();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetEncryptedUuidByNetworkId_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string networkId = "123";
    std::string uuid;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: StopAdvertising_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, StopAdvertising_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_META_TYPE] = "1";
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: StopAdvertising_202
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, StopAdvertising_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_PUBLISH_ID] = "12";
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: BindTarget_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, BindTarget_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.deviceId = "123";
    std::map<std::string, std::string> bindParam;
    bindParam[PARAM_KEY_META_TYPE] = "1";
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: BindTarget_202
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, BindTarget_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.wifiIp = "192.168.1.1";
    targetId.deviceId = "123";
    std::map<std::string, std::string> bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnbindTarget_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, UnbindTarget_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.wifiIp = "192.168.1.1";
    targetId.deviceId = "123";
    std::map<std::string, std::string> unbindParam;
    unbindParam[PARAM_KEY_META_TYPE] = "1";
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterPinHolderCallback_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "CollaborationFwk";
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: CreatePinHolder_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "CollaborationFwk";
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName);
    EXPECT_EQ(ret, DM_OK);
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
    EXPECT_EQ(ret, DM_OK);
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
 * @tc.name: CheckAccessControl_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_202, testing::ext::TestSize.Level0)
{
    DeletePermission();
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_EQ(ret, false);
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
    DeletePermission();
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: InitAccountInfo_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, InitAccountInfo_201, testing::ext::TestSize.Level0)
{
    int32_t userId = 100;
    std::string commonEventType = CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(userId, commonEventType);
    std::string commonEventType = EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN;
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
    std::string commonEventType = CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    DeviceManagerService::GetInstance().ScreenCommonEventCallback(commonEventType);
    EXPECT_NE(DeviceManagerService::GetInstance().accountCommonEventManager_, nullptr);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
