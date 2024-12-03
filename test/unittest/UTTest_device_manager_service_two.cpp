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
using namespace testing;
using namespace testing::ext;
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

void SetSetDnPolicyPermission()
{
    const int32_t permsNum = 1;
    const int32_t indexZero = 0;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "collaboration_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

/**
 * @tc.name: GetTrustedDeviceList_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*appManagerMock_, GetAppId()).Times(::testing::AtLeast(1)).WillOnce(Return("appId"));
    DeviceManagerService::GetInstance().listener_ = std::make_shared<DeviceManagerServiceListener>();
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName);
    DeletePermission();
    const std::string extra;
    std::vector<DmDeviceInfo> deviceList;
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
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
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
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
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).Times(::testing::AtLeast(1)).WillOnce(Return(false));
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
    EXPECT_CALL(*softbusListenerMock_, GetDeviceSecurityLevel(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, networkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

/**
 * @tc.name: CheckAccessControl_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_201, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK)).WillOnce(Return(DM_OK));
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
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK)).WillOnce(Return(DM_OK));
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
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: InitAccountInfo_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, InitAccountInfo_201, testing::ext::TestSize.Level0)
{
    int32_t userId = 100;
    std::string commonEventType = "usual.event.USER_SWITCHED";
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, 101);
    commonEventType = "common.event.HWID_LOGIN";
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, 101);
    commonEventType = CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    int32_t currentUserId = -1;
    int32_t beforeUserId = 0;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, currentUserId, beforeUserId);
    commonEventType = CommonEventSupport::COMMON_EVENT_HWID_LOGOUT;
    currentUserId = 1;
    beforeUserId = 1;
    DMAccountInfo dmAccountInfo;
    dmAccountInfo.accountId = "accountId";
    dmAccountInfo.accountName = "accountName";
    EXPECT_CALL(*multipleUserConnectorMock_, GetAccountInfoByUserId(_)).WillOnce(Return(dmAccountInfo));
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, currentUserId, beforeUserId);
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
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    bool bRet = false;
    if (ret == DM_OK || ret == ERR_DM_UNSUPPORTED_METHOD || ret == SOFTBUS_IPC_ERR) {
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
    SetSetDnPolicyPermission();
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
    SetSetDnPolicyPermission();
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
    SetSetDnPolicyPermission();
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
    SetSetDnPolicyPermission();
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
    SetSetDnPolicyPermission();
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

    pkgName = "pkgName";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetDeviceScreenStatus(_, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

/**
 * @tc.name: GetTrustedDeviceList_204
 * @tc.desc:Set the intFlag of GetTrustedDeviceList_004 to true; Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_204, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string extra = "jdddd";
    std::vector<DmDeviceInfo> deviceList;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_)).WillOnce(Return(ERR_DM_FAILED));
    int ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

/**
 * @tc.name: GetLocalDeviceInfo_203
 * @tc.desc: The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_203, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetLocalDeviceInfo(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(info);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

/**
 * @tc.name: GetLocalDeviceInfo_204
 * @tc.desc: The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_204, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info;
    DeletePermission();
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetLocalDeviceInfo(_)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(info);
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.wifiIp = "wifi_Ip";
    std::map<std::string, std::string> bindParam;
    std::string key = PARAM_KEY_TARGET_ID;
    std::string value = "186";
    bindParam.insert(std::make_pair(key, value));
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    targetId.wifiIp = "178.168.1.2";
    EXPECT_CALL(*softbusListenerMock_, GetIPAddrTypeFromCache(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, SetLocalDeviceName_201, testing::ext::TestSize.Level0)
{
    std::string localDeviceName = "localDeviceName_201";
    std::string localDisplayName = "localDisplayName_201";
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    int32_t ret = DeviceManagerService::GetInstance().SetLocalDeviceName(localDeviceName, localDisplayName);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);

    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, SetLocalDeviceName(_, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().SetLocalDeviceName(localDeviceName, localDisplayName);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*softbusListenerMock_, SetLocalDeviceName(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().SetLocalDeviceName(localDeviceName, localDisplayName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "123456";
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCacheMock_, GetUdidByUdidHash(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(0));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "123456";
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCacheMock_, GetUdidByUdidHash(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(0));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCacheMock_, GetUdidByUdidHash(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(DM_IDENTICAL_ACCOUNT));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnBindDevice(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCacheMock_, GetUdidByUdidHash(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(DM_IDENTICAL_ACCOUNT));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnBindDevice(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, LoadHardwareFwkService_201, testing::ext::TestSize.Level0)
{
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().LoadHardwareFwkService();
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_)).WillOnce(Return(ERR_DM_FAILED));
    DeviceManagerService::GetInstance().LoadHardwareFwkService();
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

/**
 * @tc.name: UnAuthenticateDevice_201
 * @tc.desc: Set intFlag for UnAuthenticateDevice to true and pkgName to com.ohos.test; set deviceId null ，The return
 * value is SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "12345";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(0));
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnAuthenticateDevice(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnAuthenticateDevice(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, DM_OK);

    int32_t userId = 123456;
    std::string accountId = "accountId";
    std::string accountName = "accountName";
    std::multimap<std::string, int32_t> deviceMap;
    deviceMap.insert(std::make_pair("accountId", userId));
    deviceMap.insert(std::make_pair("accountName", 1));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetDeviceIdAndUserId(_, _)).WillOnce(Return(deviceMap));
    EXPECT_CALL(*cryptoMock_, GetAccountIdHash(_, _)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().HandleAccountLogout(userId, accountId, accountName);

    EXPECT_CALL(*deviceManagerServiceImplMock_, GetDeviceIdAndUserId(_, _)).WillOnce(Return(deviceMap));
    EXPECT_CALL(*cryptoMock_, GetAccountIdHash(_, _)).WillOnce(Return(ERR_DM_FAILED));
    DeviceManagerService::GetInstance().HandleAccountLogout(userId, accountId, accountName);

    int32_t curUserId = 0;
    int32_t preUserId = 1;
    std::map<std::string, int32_t> curUserDeviceMap;
    curUserDeviceMap["curUserId"] = userId;
    curUserDeviceMap["preUserId"] = userId;
    std::map<std::string, int32_t> preUserDeviceMap;
    preUserDeviceMap["accountId"] = userId;
    preUserDeviceMap["accountName"] = 1;
    EXPECT_CALL(*deviceManagerServiceImplMock_,
        GetDeviceIdAndBindLevel(_)).WillOnce(Return(curUserDeviceMap)).WillOnce(Return(preUserDeviceMap));
    EXPECT_CALL(*multipleUserConnectorMock_, GetBackgroundUserIds(_)).WillOnce(Return(ERR_DM_FAILED));
    DeviceManagerService::GetInstance().HandleUserSwitched(curUserId, preUserId);

    int32_t removeId = 123;
    deviceMap.insert(std::make_pair("removeId", removeId));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetDeviceIdAndUserId(_)).WillOnce(Return(deviceMap));
    DeviceManagerService::GetInstance().HandleUserRemoved(userId);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_205, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetTargetInfoFromCache(_, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    if (DeviceManagerService::GetInstance().discoveryMgr_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    DeviceManagerService::GetInstance().ClearDiscoveryCache(processInfo);
    DeviceManagerService::GetInstance().RemoveNotifyRecord(processInfo);

    std::vector<uint32_t> foregroundUserIds;
    std::vector<uint32_t> backgroundUserIds;
    std::string remoteUdid = "remoteUdid";
    if (DeviceManagerService::GetInstance().softbusListener_ == nullptr) {
        DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    }
    DeviceManagerService::GetInstance().ProcessSyncUserIds(foregroundUserIds, backgroundUserIds, remoteUdid);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

/**
 * @tc.name: AuthenticateDevice_205
 * @tc.desc: Set intFlag for GAuthenticateDevice to True and deviceId to null; Return ERR_DM_BIND_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_205, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 0;
    std::string deviceId = " 2345";
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetTargetInfoFromCache(_, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    std::vector<std::string> peerUdids;
    int32_t userId = 123456;
    if (DeviceManagerService::GetInstance().softbusListener_ == nullptr) {
        DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    }
    DeviceManagerService::GetInstance().SendUserRemovedBroadCast(peerUdids, userId);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, RegDevStateCallbackToService_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManagerService::GetInstance().RegDevStateCallbackToService(pkgName);
    EXPECT_EQ(ret, DM_OK);

    std::string msg;
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);
    msg = R"(
    {
        "processType" : 1,
        "g_authType" : 1,
        "userId" : "123"
    }))";
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    msg = R"(
    {
        "processType" : 1,
        "g_authType" : 1,
        "userId" : "123"
    })";
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    msg = R"(
    {
        "networkId" : "networkId_001",
        "g_authType" : 1,
        "userId" : "123"
    })";
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    nlohmann::json msgJsonObj;
    msgJsonObj["networkId"] = "networkId_001";
    msgJsonObj["discoverType"] = 0;
    msg = msgJsonObj.dump();
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(""), Return(DM_OK)));
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>("udid01"), Return(DM_OK)))
        .WillOnce(DoAll(SetArgReferee<1>("udid01"), Return(DM_OK)));
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    msgJsonObj["discoverType"] = 1;
    msg = msgJsonObj.dump();
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);
}

HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_205, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::vector<DmDeviceInfo> deviceList;
    int ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, deviceList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "jjdjk_pkgName";
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, deviceList);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);

    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, deviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    std::vector<DmDeviceInfo> onlineDeviceList;
    DmDeviceInfo dmDeviceInfo;
    dmDeviceInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    dmDeviceInfo.networkType = 1;
    onlineDeviceList.push_back(dmDeviceInfo);
    std::unordered_map<std::string, DmAuthForm> udidMap;
    udidMap.insert(std::make_pair("udid01", DmAuthForm::IDENTICAL_ACCOUNT));
    udidMap.insert(std::make_pair("udid02", DmAuthForm::ACROSS_ACCOUNT));
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(onlineDeviceList), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetAppTrustDeviceIdList(_)).WillOnce(Return(udidMap));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid01"), Return(DM_OK)));
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, deviceList);
    EXPECT_EQ(ret, DM_OK);

    pkgName = "ohos.deviceprofile";
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(onlineDeviceList), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetAppTrustDeviceIdList(_)).WillOnce(Return(udidMap));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid02"), Return(DM_OK)));
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, deviceList);
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, DmHiDumper_201, testing::ext::TestSize.Level0)
{
    std::vector<std::string> args;
    std::string result;
    std::vector<HidumperFlag> dumpflag;
    dumpflag.push_back(HidumperFlag::HIDUMPER_GET_DEVICE_STATE);
    dumpflag.push_back(HidumperFlag::HIDUMPER_GET_HELP);
    dumpflag.push_back(HidumperFlag::HIDUMPER_GET_TRUSTED_LIST);
    std::vector<DmDeviceInfo> deviceList;
    DmDeviceInfo dmDeviceInfo;
    dmDeviceInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    dmDeviceInfo.deviceTypeId = 0;
    deviceList.push_back(dmDeviceInfo);
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*hiDumpHelperMock_, GetArgsType(_, _)).WillOnce(DoAll(SetArgReferee<1>(dumpflag), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceList), Return(DM_OK)));
    int ret = DeviceManagerService::GetInstance().DmHiDumper(args, result);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*hiDumpHelperMock_, GetArgsType(_, _)).WillOnce(DoAll(SetArgReferee<1>(dumpflag), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceList), Return(ERR_DM_FAILED)));
    ret = DeviceManagerService::GetInstance().DmHiDumper(args, result);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
