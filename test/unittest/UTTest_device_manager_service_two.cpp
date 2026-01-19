/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <cstring>
#include <utility>

using namespace OHOS::Security::AccessToken;
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* PARAM_KEY_TARGET_ID = "TARGET_ID";
constexpr int32_t NUM_5 = 5;
constexpr int32_t SERVICE_PUBLISHED_STATE = 1;
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
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int32_t userId = 10001;
    EXPECT_CALL(*appManagerMock_, GetAppId()).Times(::testing::AtLeast(1)).WillOnce(Return("appId"));
    DeviceManagerService::GetInstance().listener_ = std::make_shared<DeviceManagerServiceListener>();
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName, userId);
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
HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_201, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_201, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_201, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, IsDMImplSoLoaded_201, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "CollaborationFwk";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: CreatePinHolder_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "CollaborationFwk";
    PeerTargetId targetId;
    DmPinType pinType = NUMBER_PIN_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).Times(::testing::AtLeast(1)).
        WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetDeviceSecurityLevel_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string networkId = "123";
    int32_t securityLevel;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetDeviceSecurityLevel(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, networkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

/**
 * @tc.name: CheckAccessControl_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, CheckAccessControl(_, _, _, _)).WillOnce(Return(true));
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckAccessControl_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_202, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, CheckIsSameAccount_201, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, CheckIsSameAccount(_, _, _, _)).WillOnce(Return(true));
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckIsSameAccount_202
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, CheckIsSameAccount_202, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, InitAccountInfo_201, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;
    std::string commonEventType = "usual.event.USER_SWITCHED";
    std::map<std::string, int32_t> curUserDeviceMap;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, 101);
    commonEventType = "common.event.HWID_LOGIN";
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, userId, 101);
    commonEventType = "usual.event.USER_SWITCHED";
    int32_t currentUserId = -1;
    int32_t beforeUserId = 0;
    DeviceManagerService::GetInstance().AccountCommonEventCallback(commonEventType, currentUserId, beforeUserId);
    commonEventType = "common.event.HWID_LOGOUT";
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
HWTEST_F(DeviceManagerServiceTest, InitScreenLockEvent_201, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerService::GetInstance().InitScreenLockEvent();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SubscribeAccountCommonEvent_201
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SubscribeAccountCommonEvent_201, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_201, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
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
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_202, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName;
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_203
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_203, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    SetSetDnPolicyPermission();
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_204
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_204, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_205
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_205, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "a100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetDnPolicy_206
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_206, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100a";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.screenStatusTest";
    std::string networkId = "";
    int32_t screenStatus = -1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    std::string networkId = "networkIdTest";
    int32_t screenStatus = -1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
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
HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_204, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_203, testing::ext::TestSize.Level1)
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
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_204, testing::ext::TestSize.Level1)
{
    DmDeviceInfo info;
    DeletePermission();
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetLocalDeviceInfo(_)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(info);
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, BindTarget_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.wifiIp = "wifi_Ip";
    std::map<std::string, std::string> bindParam;
    std::string key = PARAM_KEY_TARGET_ID;
    std::string value = "186";
    bindParam.insert(std::make_pair(key, value));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);

    targetId.wifiIp = "178.168.1.2";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetIPAddrTypeFromCache(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "123456";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(0));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_NE(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "123456";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(0));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_NE(ret, ERR_DM_POINT_NULL);

    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(DM_IDENTICAL_ACCOUNT));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnBindDevice(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_NE(ret, ERR_DM_POINT_NULL);

    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(DM_IDENTICAL_ACCOUNT));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnBindDevice(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_NE(ret, ERR_DM_POINT_NULL);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

/**
 * @tc.name: UnAuthenticateDevice_201
 * @tc.desc: Set intFlag for UnAuthenticateDevice to true and pkgName to com.ohos.test; set deviceId null ，The return
 * value is SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, UnAuthenticateDevice_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "12345";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(0));
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnAuthenticateDevice(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
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

    int32_t removeId = 123;
    deviceMap.insert(std::make_pair("removeId", removeId));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetDeviceIdAndUserId(_)).WillOnce(Return(deviceMap));
    DeviceManagerService::GetInstance().HandleUserRemoved(userId);
}

HWTEST_F(DeviceManagerServiceTest, BindDevice_205, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).Times(::testing::AtLeast(2)).WillRepeatedly(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetTargetInfoFromCache(_, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, DM_OK);

    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    if (DeviceManagerService::GetInstance().discoveryMgr_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    DeviceManagerService::GetInstance().ClearDiscoveryCache(processInfo);
    DeviceManagerService::GetInstance().RemoveNotifyRecord(processInfo);
}

/**
 * @tc.name: AuthenticateDevice_205
 * @tc.desc: Set intFlag for GAuthenticateDevice to True and deviceId to null; Return ERR_DM_BIND_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, AuthenticateDevice_205, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 0;
    std::string deviceId = " 2345";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetTargetInfoFromCache(_, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, DM_OK);

    std::vector<std::string> peerUdids;
    int32_t userId = 123456;
    if (DeviceManagerService::GetInstance().softbusListener_ == nullptr) {
        DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    }
    DeviceManagerService::GetInstance().SendUserRemovedBroadCast(peerUdids, userId);

    std::vector<int32_t> foregroundUserIds;
    std::vector<int32_t> backgroundUserIds;
    bool isNeedResponse = false;
    foregroundUserIds.push_back(1);
    foregroundUserIds.push_back(101);
    backgroundUserIds.push_back(102);
    backgroundUserIds.push_back(12);
    DeviceManagerService::GetInstance().SendUserIdsBroadCast(peerUdids, foregroundUserIds,
        backgroundUserIds, isNeedResponse);

    std::vector<UserIdInfo> remoteUserIdInfos;
    UserIdInfo userIdInfo(true, 1);
    remoteUserIdInfos.push_back(userIdInfo);
    UserIdInfo userIdInfo1(true, 102);
    remoteUserIdInfos.push_back(userIdInfo1);
    UserIdInfo userIdInfo2(false, 103);
    remoteUserIdInfos.push_back(userIdInfo2);
    UserIdInfo userIdInfo3(false, 104);
    remoteUserIdInfos.push_back(userIdInfo3);
    std::string remoteUdid = "remoteDeviceId";
    isNeedResponse = true;
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(Return(ERR_DM_FAILED)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*multipleUserConnectorMock_, GetBackgroundUserIds(_)).WillOnce(Return(ERR_DM_FAILED));
    DeviceManagerService::GetInstance().HandleUserIdsBroadCast(remoteUserIdInfos, remoteUdid, isNeedResponse);

    std::vector<int32_t> foregroundUserVec;
    foregroundUserVec.push_back(1);
    foregroundUserVec.push_back(102);
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(foregroundUserVec), Return(DM_OK))).WillOnce(Return(DM_OK));
    EXPECT_CALL(*multipleUserConnectorMock_, GetBackgroundUserIds(_)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().HandleUserIdsBroadCast(remoteUserIdInfos, remoteUdid, isNeedResponse);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, RegDevStateCallbackToService_201, testing::ext::TestSize.Level1)
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

    JsonObject msgJsonObj;
    msgJsonObj["networkId"] = "networkId_001";
    msgJsonObj["discoverType"] = 0b0100;
    msgJsonObj["ischange"] = true;
    msg = msgJsonObj.Dump();
    std::vector<int32_t> foregroundUserVec;
    foregroundUserVec.push_back(101);
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(foregroundUserVec), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(""), Return(DM_OK)));
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(foregroundUserVec), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>("udid01"), Return(DM_OK)));
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);
    Mock::VerifyAndClearExpectations(multipleUserConnectorMock_.get());

    msgJsonObj["discoverType"] = 0;
    msg = msgJsonObj.Dump();
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);
}

HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_205, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string extra = "extra";
    std::vector<DmDeviceInfo> deviceList;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "jjdjk_pkgName";
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);

    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    std::vector<DmDeviceInfo> onlineDeviceList;
    DmDeviceInfo dmDeviceInfo;
    dmDeviceInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    dmDeviceInfo.networkType = 1;
    onlineDeviceList.push_back(dmDeviceInfo);
    std::unordered_map<std::string, DmAuthForm> udidMap;
    udidMap.insert(std::make_pair("udid01", DmAuthForm::IDENTICAL_ACCOUNT));
    udidMap.insert(std::make_pair("udid02", DmAuthForm::ACROSS_ACCOUNT));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(onlineDeviceList), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetAppTrustDeviceIdList(_)).WillOnce(Return(udidMap));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid01"), Return(DM_OK)));
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, DM_OK);

    pkgName = "ohos.deviceprofile";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(onlineDeviceList), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetAppTrustDeviceIdList(_)).WillOnce(Return(udidMap));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid02"), Return(DM_OK)));
    ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, DmHiDumper_201, testing::ext::TestSize.Level1)
{
    std::vector<std::string> args;
    args.push_back(std::string("-getTrustlist"));
    std::string result;
    std::vector<DmDeviceInfo> deviceList;
    DmDeviceInfo dmDeviceInfo;
    dmDeviceInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    dmDeviceInfo.deviceTypeId = 0;
    deviceList.push_back(dmDeviceInfo);
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceList), Return(DM_OK)));
    int ret = DeviceManagerService::GetInstance().DmHiDumper(args, result);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*softbusListenerMock_, GetTrustedDeviceList(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceList), Return(ERR_DM_FAILED)));
    ret = DeviceManagerService::GetInstance().DmHiDumper(args, result);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_207, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100a";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10a";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(ERR_DM_FAILED)));
    ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(false));
    ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, GetTrustedDeviceList_206, testing::ext::TestSize.Level1)
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

HWTEST_F(DeviceManagerServiceTest, ParseCheckSumMsg_201, testing::ext::TestSize.Level1)
{
    std::string msg;
    std::string networkId;
    uint32_t discoveryType = 0;
    bool isChange = false;
    JsonObject msgJsonObj;
    msgJsonObj["networkId"] = "networkId001";
    msgJsonObj["discoverType"] = 1;
    msgJsonObj["ischange"] = false;
    msg = msgJsonObj.Dump();
    int ret = DeviceManagerService::GetInstance().ParseCheckSumMsg(msg, networkId, discoveryType, isChange);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_203, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string udidHash;
    std::string extra;
    int ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_204, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udidHash;
    std::string extra;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    udidHash = "udidHash";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_NE(ret, DM_OK);

    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(0));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_NE(ret, DM_OK);

    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnBindDevice(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_NE(ret, DM_OK);

    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnBindDevice(_, _, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, UnBindDevice_205, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string udidHash = "udidHash";
    std::string extra = "extra";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceImplMock_, GetBindLevel(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*deviceManagerServiceImplMock_, UnBindDevice(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, udidHash, extra);
    EXPECT_NE(ret, DM_OK);

    std::vector<UserIdInfo> remoteUserIdInfos;
    std::string remoteUdid;
    bool isNeedResponse = false;
    DeviceManagerService::GetInstance().HandleUserIdsBroadCast(remoteUserIdInfos, remoteUdid, isNeedResponse);

    std::vector<std::string> peerUdids;
    int32_t userId = 123;
    uint64_t tokenId = 1;
    int32_t bindLevel = 1;
    uint64_t peerTokenId = 1;
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, bindLevel, peerTokenId);
    bindLevel = 3;
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, bindLevel, peerTokenId);
    bindLevel = 2;
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, bindLevel, peerTokenId);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, SendAppUnBindBroadCast_201, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    int32_t userId = 12;
    uint64_t tokenId = 23;
    uint64_t peerTokenId = 1;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendAppUnBindBroadCast(peerUdids, userId, tokenId, peerTokenId);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, GetAnonyLocalUdid_201, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string anonyUdid;
    int32_t ret = DeviceManagerService::GetInstance().GetAnonyLocalUdid(pkgName, anonyUdid);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);

    const char *peerUdid = "peerUdid";
    GroupInformation groupInfo;
    DeviceManagerService::GetInstance().HandleDeviceUnBind(peerUdid, groupInfo);
}

HWTEST_F(DeviceManagerServiceTest, GetAnonyLocalUdid_202, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string anonyUdid;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetAnonyLocalUdid(pkgName, anonyUdid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, StopAuthenticateDevice_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_004";
    EXPECT_CALL(*deviceManagerServiceImplMock_, StopAuthenticateDevice(_)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, IsDMServiceAdapterSoLoaded_201, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isAdapterResidentSoLoaded_ = false;
    bool ret = DeviceManagerService::GetInstance().IsDMServiceAdapterSoLoaded();
    EXPECT_FALSE(ret);

    DeviceManagerService::GetInstance().isAdapterResidentSoLoaded_ = true;
    DeviceManagerService::GetInstance().dmServiceImplExtResident_ = nullptr;
    ret = DeviceManagerService::GetInstance().IsDMServiceAdapterSoLoaded();
    EXPECT_FALSE(ret);

    DeviceManagerService::GetInstance().isAdapterResidentSoLoaded_ = false;
    DeviceManagerService::GetInstance().IsDMServiceAdapterResidentLoad();
    ret = DeviceManagerService::GetInstance().IsDMServiceAdapterSoLoaded();
    EXPECT_FALSE(ret);

    DeviceManagerService::GetInstance().UnloadDMServiceAdapterResident();
}

HWTEST_F(DeviceManagerServiceTest, RegisterAuthenticationType_201, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> authParam;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);

    std::string networkId;
    std::vector<int32_t> foregroundUserIds;
    std::vector<int32_t> backgroundUserIds;
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(""), Return(DM_OK)));
    DeviceManagerService::GetInstance().ProcessCheckSumByBT(networkId, foregroundUserIds, backgroundUserIds);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>("udid01"), Return(DM_OK)));
    DeviceManagerService::GetInstance().ProcessCheckSumByBT(networkId, foregroundUserIds, backgroundUserIds);

    foregroundUserIds.push_back(101);
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>("udid01"), Return(DM_OK)));
    DeviceManagerService::GetInstance().ProcessCheckSumByBT(networkId, foregroundUserIds, backgroundUserIds);

    DeviceManagerService::GetInstance().localNetWorkId_ = "";
    networkId = "networkId001";
    std::string str = "deviceId";
    DmDeviceInfo deviceInfo;
    foregroundUserIds.push_back(102);
    memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, str.c_str(), str.length());
    backgroundUserIds.push_back(201);
    backgroundUserIds.push_back(202);
    DeviceManagerService::GetInstance().ProcessCheckSumByWifi(networkId, foregroundUserIds, backgroundUserIds);
}

HWTEST_F(DeviceManagerServiceTest, RegisterAuthenticationType_202, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    
    std::map<std::string, std::string> authParam;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    authParam.insert(std::make_pair(DM_AUTHENTICATION_TYPE, "authentication"));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    authParam[DM_AUTHENTICATION_TYPE] = "123456";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    std::string msg;
    JsonObject msgJsonObj;
    msgJsonObj["networkId"] = "networkId";
    msgJsonObj["discoverType"] = 123;
    msgJsonObj["ischange"] = false;
    msg = msgJsonObj.Dump();
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    msgJsonObj["ischange"] = true;
    msg = msgJsonObj.Dump();
    std::vector<int32_t> foregroundUserIds;
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(foregroundUserIds), Return(ERR_DM_INPUT_PARA_INVALID)));
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    foregroundUserIds.push_back(101);
    std::vector<int32_t> backgroundUserIds;
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(foregroundUserIds), Return(DM_OK)));
    EXPECT_CALL(*multipleUserConnectorMock_, GetBackgroundUserIds(_)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*dMCommToolMock_, SendUserIds(_, _, _)).WillRepeatedly(Return(ERR_DM_FAILED));
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);

    backgroundUserIds.push_back(102);
    msgJsonObj["discoverType"] = 1;
    msg = msgJsonObj.Dump();
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(foregroundUserIds), Return(DM_OK)));
    EXPECT_CALL(*multipleUserConnectorMock_, GetBackgroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(backgroundUserIds), Return(DM_OK)));
    DeviceManagerService::GetInstance().HandleUserIdCheckSumChange(msg);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceProfileInfoList_201, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    OHOS::DistributedHardware::DmDeviceProfileInfoFilterOptions filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceProfileInfoList_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    OHOS::DistributedHardware::DmDeviceProfileInfoFilterOptions filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceIconInfo_201, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    OHOS::DistributedHardware::DmDeviceIconInfoFilterOptions filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceIconInfo(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceIconInfo_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    OHOS::DistributedHardware::DmDeviceIconInfoFilterOptions filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceIconInfo(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceTest, PutDeviceProfileInfoList_201, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    std::vector<DmDeviceProfileInfo> deviceProfileInfoList;
    int32_t ret = DeviceManagerService::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, PutDeviceProfileInfoList_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::vector<DmDeviceProfileInfo> deviceProfileInfoList;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidPutDeviceProfileInfoList(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    EXPECT_TRUE(ret == ERR_DM_UNSUPPORTED_METHOD || ret == ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, SetLocalDisplayNameToSoftbus_201, testing::ext::TestSize.Level1)
{
    std::string displayName = "displayName";
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    int32_t ret = DeviceManagerService::GetInstance().SetLocalDisplayNameToSoftbus(displayName);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);

    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, SetLocalDisplayName(_)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().SetLocalDisplayNameToSoftbus(displayName);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*softbusListenerMock_, SetLocalDisplayName(_)).WillOnce(Return(ERR_DM_NO_PERMISSION));
    ret = DeviceManagerService::GetInstance().SetLocalDisplayNameToSoftbus(displayName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDisplayDeviceName_201, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName = "packName";
    int32_t maxNameLength = 1;
    std::string displayName = "displayName";
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDisplayDeviceName(pkgName, maxNameLength, displayName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDisplayDeviceName_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "packName";
    int32_t maxNameLength = 1;
    std::string displayName = "";
    std::string deviceName = "displayName";
    EXPECT_CALL(*deviceNameManagerMock_, GetLocalDisplayDeviceName(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(deviceName), Return(0)));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDisplayDeviceName(pkgName, maxNameLength, displayName);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(displayName, deviceName);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceNamePrefixs_201, testing::ext::TestSize.Level1)
{
    auto ret = DeviceManagerService::GetInstance().GetDeviceNamePrefixs();
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceManagerServiceTest, GetAllTrustedDeviceList_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    std::string extra = "extra";
    std::vector<DmDeviceInfo> deviceList;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetAllTrustedDeviceList(_, _, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, DM_OK);

    EXPECT_CALL(*softbusListenerMock_, GetAllTrustedDeviceList(_, _, _)).WillOnce(Return(ERR_DM_INPUT_PARA_INVALID));
    ret = DeviceManagerService::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    DeviceManagerService::GetInstance().hichainListener_ = nullptr;
    DeviceManagerService::GetInstance().InitHichainListener();
}

HWTEST_F(DeviceManagerServiceTest, GetAllTrustedDeviceList_202, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName = "pkgName";
    std::string extra = "extra";
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManagerService::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);

    int32_t actionId = 1;
    std::string bindParam = "bindParamData";
    DeviceManagerService::GetInstance().AddHmlInfoToBindParam(actionId, bindParam);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceInfo_202, testing::ext::TestSize.Level1)
{
    std::string networkId = "networkId";
    DmDeviceInfo deviceInfo;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("peerDeviceId"), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetDeviceInfo(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("peerDeviceId"), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetDeviceInfo(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("peerDeviceId"), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetDeviceInfo(_, _)).WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("peerDeviceId"), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetDeviceInfo(_, _))
        .WillOnce(Return (DM_OK))
        .WillOnce(Return(DM_OK));
    ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("peerDeviceId"), Return(DM_OK)));
    ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    EXPECT_EQ(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, InitDPLocalServiceInfo_201, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo serviceInfo;
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfo;
    bool ret = DeviceManagerService::GetInstance().InitDPLocalServiceInfo(serviceInfo, dpLocalServiceInfo);
    EXPECT_TRUE(ret);

    DeviceManagerService::GetInstance().InitServiceInfo(dpLocalServiceInfo, serviceInfo);
    std::vector<DistributedDeviceProfile::LocalServiceInfo> dpLocalServiceInfos;
    dpLocalServiceInfos.push_back(dpLocalServiceInfo);
    std::vector<DMLocalServiceInfo> serviceInfos;
    DeviceManagerService::GetInstance().InitServiceInfos(dpLocalServiceInfos, serviceInfos);
}

HWTEST_F(DeviceManagerServiceTest, RegisterLocalServiceInfo_201, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo serviceInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, PutLocalServiceInfo(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().RegisterLocalServiceInfo(serviceInfo);
    EXPECT_EQ(ret, DM_OK);

    std::string localUdid = "U*******jkjk2";
    std::vector<std::string> deviceVec{"deviceInfo"};
    std::vector<int32_t> foregroundUserIds{10, 11};
    std::vector<int32_t> backgroundUserIds{1, 2};
    DeviceManagerService::GetInstance().InitDMServiceListener();
    DeviceManagerService::GetInstance().hichainListener_ = std::make_shared<HichainListener>();
    DeviceManagerService::GetInstance().UpdateAclAndDeleteGroup(localUdid, deviceVec,
        foregroundUserIds, backgroundUserIds);

    std::string pkgName = "pkgName";
    DeviceManagerService::GetInstance().ClearPublishIdCache(pkgName);
    DeviceManagerService::GetInstance().hichainListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, UnRegisterLocalServiceInfo_201, testing::ext::TestSize.Level1)
{
    std::string bundleName = "bund******98";
    int32_t pinExchangeType = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteLocalServiceInfo(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterLocalServiceInfo(bundleName, pinExchangeType);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UpdateLocalServiceInfo_201, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo serviceInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, UpdateLocalServiceInfo(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().UpdateLocalServiceInfo(serviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalServiceInfoByBundleNameAndPinExchangeType_201,
    testing::ext::TestSize.Level1)
{
    std::string bundleName = "bund******98";
    int32_t pinExchangeType = 1;
    DMLocalServiceInfo serviceInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName,
            pinExchangeType, serviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, InitDPLocalServiceInfo_001, testing::ext::TestSize.Level0)
{
    DMLocalServiceInfo dmServiceInfo;
    dmServiceInfo.bundleName = "testbundle";
    dmServiceInfo.extraInfo = "testextra";
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfo;
    DeviceManagerService::GetInstance().InitDPLocalServiceInfo(dmServiceInfo, dpLocalServiceInfo);
    EXPECT_TRUE(dmServiceInfo.bundleName == dpLocalServiceInfo.GetBundleName());
    EXPECT_TRUE(dmServiceInfo.extraInfo == dpLocalServiceInfo.GetExtraInfo());
}

HWTEST_F(DeviceManagerServiceTest, InitServiceInfo_001, testing::ext::TestSize.Level0)
{
    DMLocalServiceInfo dmServiceInfo;
    DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfo;
    dpLocalServiceInfo.SetBundleName("testbundle");
    dpLocalServiceInfo.SetExtraInfo("testextra");
    DeviceManagerService::GetInstance().InitServiceInfo(dpLocalServiceInfo, dmServiceInfo);
    EXPECT_TRUE(dmServiceInfo.bundleName == dpLocalServiceInfo.GetBundleName());
    EXPECT_TRUE(dmServiceInfo.extraInfo == dpLocalServiceInfo.GetExtraInfo());
}

HWTEST_F(DeviceManagerServiceTest, InitServiceInfos_001, testing::ext::TestSize.Level0)
{
    std::vector<DMLocalServiceInfo> dmServiceInfos;
    std::vector<DistributedDeviceProfile::LocalServiceInfo> dpLocalServiceInfos;
    for (int k = 0; k < NUM_5; k++) {
        DistributedDeviceProfile::LocalServiceInfo dpLocalServiceInfo;
        dpLocalServiceInfo.SetBundleName(std::string("testbundle") + std::to_string(k));
        dpLocalServiceInfo.SetExtraInfo(std::string("testextra") + std::to_string(k));
        dpLocalServiceInfos.emplace_back(dpLocalServiceInfo);
    }
    DeviceManagerService::GetInstance().InitServiceInfos(dpLocalServiceInfos, dmServiceInfos);
    for (int k = 0; k < NUM_5; k++) {
        DMLocalServiceInfo &dmServiceInfo = dmServiceInfos[k];
        DistributedDeviceProfile::LocalServiceInfo &dpLocalServiceInfo = dpLocalServiceInfos[k];
        EXPECT_TRUE(dmServiceInfo.bundleName == dpLocalServiceInfo.GetBundleName());
        EXPECT_TRUE(dmServiceInfo.extraInfo == dpLocalServiceInfo.GetExtraInfo());
    }
}

HWTEST_F(DeviceManagerServiceTest, RestoreLocalDeviceName_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().RestoreLocalDeviceName(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, RestoreLocalDeviceName_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().RestoreLocalDeviceName(pkgName);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyLocalDeviceName(_)).WillOnce(Return(false));
    ret = DeviceManagerService::GetInstance().RestoreLocalDeviceName(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyLocalDeviceName(_)).WillOnce(Return(true));
    ret = DeviceManagerService::GetInstance().RestoreLocalDeviceName(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, SetLocalDeviceName_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string deviceName = "deviceName";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().SetLocalDeviceName(pkgName, deviceName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, SetLocalDeviceName_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string deviceName = "deviceName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().SetLocalDeviceName(pkgName, deviceName);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyLocalDeviceName(_)).WillOnce(Return(false));
    ret = DeviceManagerService::GetInstance().SetLocalDeviceName(pkgName, deviceName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyLocalDeviceName(_)).WillOnce(Return(true));
#if !defined(DEVICE_MANAGER_COMMON_FLAG)
    ret = DeviceManagerService::GetInstance().SetLocalDeviceName(pkgName, deviceName);
#else
    DeviceManagerService::GetInstance().InitDMServiceListener();
    ret = DeviceManagerService::GetInstance().SetLocalDeviceName(pkgName, deviceName);
#endif
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, SetRemoteDeviceName_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string deviceName = "deviceName";
    std::string deviceId = "d*********9";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, SetRemoteDeviceName_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string deviceName = "deviceName";
    std::string deviceId = "d*********9";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyRemoteDeviceName(_)).WillOnce(Return(false));
    ret = DeviceManagerService::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);

    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyRemoteDeviceName(_)).WillOnce(Return(true));
    ret = DeviceManagerService::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceNetworkIdList_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    NetworkIdQueryFilter queryFilter;
    std::vector<std::string> networkIds{"uehd*****87"};
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceNetworkIdList(pkgName, queryFilter, networkIds);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);

    int32_t stopUserId = 1;
    std::string stopEventUdid = "ud*********4";
    std::vector<std::string> acceptEventUdids{"acc**********7"};
    DeviceManagerService::GetInstance().InitDMServiceListener();
    DeviceManagerService::GetInstance().HandleUserStop(stopUserId, stopEventUdid, acceptEventUdids);
    DeviceManagerService::GetInstance().HandleUserStop(stopUserId, stopEventUdid);

    std::string localUdid = "local*******76";
    std::vector<std::string> peerUdids;
    DeviceManagerService::GetInstance().NotifyRemoteLocalUserStop(localUdid, peerUdids, stopUserId);
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendUserStopBroadCast(peerUdids, stopUserId);
    std::string remoteUdid = "re********7";
    DeviceManagerService::GetInstance().HandleUserStopBroadCast(stopUserId, remoteUdid);

    std::map<std::string, std::string> wifiDevices;
    wifiDevices.insert(std::make_pair("wikjdmcsk", "deviceInfowifi"));
    EXPECT_CALL(*dMCommToolMock_, SendUserStop(_, _)).WillOnce(Return(ERR_DM_FAILED));
    DeviceManagerService::GetInstance().NotifyRemoteLocalUserStopByWifi(localUdid, wifiDevices, stopUserId);

    EXPECT_CALL(*dMCommToolMock_, SendUserStop(_, _)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().NotifyRemoteLocalUserStopByWifi(localUdid, wifiDevices, stopUserId);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceTest, GetDeviceNetworkIdList_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    NetworkIdQueryFilter queryFilter;
    std::vector<std::string> networkIds{"uehd*****87"};
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceNetworkIdList(pkgName, queryFilter, networkIds);
    EXPECT_NE(ret, DM_OK);

    int32_t stopUserId = 1;
    std::map<std::string, int32_t> deviceMap;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    DeviceManagerService::GetInstance().HandleUserStopEvent(stopUserId);

    std::vector<std::string> peerUdids;
    std::vector<std::string> bleUdids;
    std::map<std::string, std::string> wifiDevices;
    DeviceManagerService::GetInstance().DivideNotifyMethod(peerUdids, bleUdids, wifiDevices);

    peerUdids.push_back("u********23");
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    DeviceManagerService::GetInstance().DivideNotifyMethod(peerUdids, bleUdids, wifiDevices);

    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusCacheMock_, GetNetworkIdFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(DM_OK)));
    DeviceManagerService::GetInstance().DivideNotifyMethod(peerUdids, bleUdids, wifiDevices);

    EXPECT_CALL(*softbusCacheMock_, GetNetworkIdFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("net********8"), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetNetworkTypeByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(4), Return(ERR_DM_FAILED)));
    DeviceManagerService::GetInstance().DivideNotifyMethod(peerUdids, bleUdids, wifiDevices);

    EXPECT_CALL(*softbusCacheMock_, GetNetworkIdFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("net********8"), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetNetworkTypeByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(4), Return(DM_OK)));
    DeviceManagerService::GetInstance().DivideNotifyMethod(peerUdids, bleUdids, wifiDevices);

    EXPECT_CALL(*softbusCacheMock_, GetNetworkIdFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("net********8"), Return(DM_OK)));
    EXPECT_CALL(*softbusListenerMock_, GetNetworkTypeByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(2), Return(DM_OK)));
    DeviceManagerService::GetInstance().DivideNotifyMethod(peerUdids, bleUdids, wifiDevices);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceTest, SendShareTypeUnBindBroadCast_001, testing::ext::TestSize.Level1)
{
    const char *credId = "testCredId";
    int32_t localUserId = 1001;
    std::vector<std::string> peerUdids = {"peerUdid1", "peerUdid2"};

    DeviceManagerService::GetInstance().SendShareTypeUnBindBroadCast(credId, localUserId, peerUdids);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceTest, HandleCredentialDeleted_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*deviceManagerServiceImplMock_, HandleCredentialDeleted(_, _, _, _, _)).Times(0);

    DeviceManagerService::GetInstance().HandleCredentialDeleted(nullptr, "credInfo");
}

HWTEST_F(DeviceManagerServiceTest, HandleCredentialDeleted_004, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).Times(0);
    EXPECT_CALL(*softbusListenerMock_, SendAclChangedBroadcast(_)).Times(0);

    DeviceManagerService::GetInstance().HandleCredentialDeleted("credId", "credInfo");
}

HWTEST_F(DeviceManagerServiceTest, HandleCredentialDeleted_005, testing::ext::TestSize.Level1)
{
    const char *credId = "testCredId";
    const char *credInfo = R"({"deviceId": "remoteUdid", "userId": 1})";
    std::string localUdid = "localUdid";
    std::string remoteUdid = "remoteUdid";
    bool isSendBroadCast = false;

    EXPECT_CALL(*deviceManagerServiceImplMock_, HandleCredentialDeleted(StrEq(credId), StrEq(credInfo), _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(remoteUdid), SetArgReferee<4>(isSendBroadCast)));

    DeviceManagerService::GetInstance().HandleCredentialDeleted(credId, credInfo);
}

HWTEST_F(DeviceManagerServiceTest, HandleCredentialDeleted_006, testing::ext::TestSize.Level1)
{
    const char *credId = "testCredId";
    const char *credInfo = R"({"deviceId": "remoteUdid", "userId": 1})";
    std::string localUdid = "localUdid";
    std::string remoteUdid = "remoteUdid";
    bool isSendBroadCast = true;

    EXPECT_CALL(*deviceManagerServiceImplMock_, HandleCredentialDeleted(StrEq(credId), StrEq(credInfo), _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(remoteUdid), SetArgReferee<4>(isSendBroadCast)));

    DeviceManagerService::GetInstance().HandleCredentialDeleted(credId, credInfo);
}

HWTEST_F(DeviceManagerServiceTest, HandleShareUnbindBroadCast_001, testing::ext::TestSize.Level1)
{
    std::string credId = "123456";
    int32_t userId = 1001;
    std::string localUdid = "localUdid";
    EXPECT_CALL(*deviceManagerServiceImplMock_, HandleShareUnbindBroadCast(_, _, _)).Times(1);

    DeviceManagerService::GetInstance().HandleShareUnbindBroadCast(userId, credId);
}

HWTEST_F(DeviceManagerServiceTest, HandleShareUnbindBroadCast_002, testing::ext::TestSize.Level1)
{
    std::string credId = "";
    int32_t userId = 1001;

    EXPECT_CALL(*deviceManagerServiceImplMock_, HandleShareUnbindBroadCast(_, _, _)).Times(0);

    DeviceManagerService::GetInstance().HandleShareUnbindBroadCast(userId, credId);
}

HWTEST_F(DeviceManagerServiceTest, HandleShareUnbindBroadCast_003, testing::ext::TestSize.Level1)
{
    std::string credId = "123456";
    int32_t userId = 1001;

    EXPECT_TRUE(DeviceManagerService::GetInstance().IsDMServiceImplReady());

    DeviceManagerService::GetInstance().HandleShareUnbindBroadCast(userId, credId);
}

HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceName_201, testing::ext::TestSize.Level1)
{
    std::string deviceName = "123";
    std::string displayName = "";
    EXPECT_CALL(*permissionManagerMock_, CheckReadLocalDeviceName()).WillOnce(Return(true));
    EXPECT_CALL(*deviceNameManagerMock_, GetLocalDisplayDeviceName(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(deviceName), Return(0)));
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceName(displayName);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(displayName, deviceName);

    displayName = "";
    std::string marketName = "456";
    EXPECT_CALL(*permissionManagerMock_, CheckReadLocalDeviceName()).WillOnce(Return(false));
    EXPECT_CALL(*deviceNameManagerMock_, GetLocalMarketName()).WillOnce(Return(marketName));
    ret = DeviceManagerService::GetInstance().GetLocalDeviceName(displayName);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(displayName, marketName);
}

HWTEST_F(DeviceManagerServiceTest, ValidateUnBindDeviceParams_201, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    std::string deviceId = "";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ValidateUnBindDeviceParams(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, ValidateUnBindDeviceParams_202, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos.test.pkgName";
    std::string deviceId = "deviceId";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().ValidateUnBindDeviceParams(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, ValidateUnBindDeviceParams_203, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    std::string deviceId = "";
    std::string extra;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ValidateUnBindDeviceParams(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, ValidateUnBindDeviceParams_204, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos.test.pkgName";
    std::string deviceId = "deviceId";
    std::string extra;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().ValidateUnBindDeviceParams(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, SendUninstAppByWifi_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 1001;
    int32_t tokenId = 1002;
    std::string networkId = "networkId123";
    EXPECT_CALL(*dMCommToolMock_, SendUninstAppObj(_, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().SendUninstAppByWifi(userId, tokenId, networkId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, SendUninstAppByWifi_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 1001;
    int32_t tokenId = 1002;
    std::string networkId = "networkId123";
    EXPECT_CALL(*dMCommToolMock_, SendUninstAppObj(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().SendUninstAppByWifi(userId, tokenId, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, GetNotifyRemoteUnBindAppWay_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 1001;
    int32_t tokenId = 11;
    std::map<std::string, std::string> wifiDevices;
    bool isBleWay = false;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().GetNotifyRemoteUnBindAppWay(userId, tokenId, wifiDevices, isBleWay);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}
HWTEST_F(DeviceManagerServiceTest, GetNotifyRemoteUnBindAppWay_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 1001;
    int32_t tokenId = 11;
    std::map<std::string, std::string> wifiDevices;
    bool isBleWay = false;
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    DeviceManagerService::GetInstance().GetNotifyRemoteUnBindAppWay(userId, tokenId, wifiDevices, isBleWay);
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
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

HWTEST_F(DeviceManagerServiceTest, BindServiceTarget_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.serviceId = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().BindServiceTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, BindServiceTarget_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.serviceId = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindServiceTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindServiceTarget_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos.distributedhardware.devicemanager";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.serviceId = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindServiceTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindServiceTarget_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.serviceId = 0;
    int32_t ret = DeviceManagerService::GetInstance().BindServiceTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, BindServiceTarget_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.serviceId = 123;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindServiceTarget(pkgName, targetId, bindParam);
    EXPECT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, BindServiceTarget_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    targetId.serviceId = 1;
    std::map<std::string, std::string> bindParam;
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindServiceTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnbindServiceTarget_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    int64_t serviceId = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnbindServiceTarget(pkgName, serviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnbindServiceTarget_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnbindServiceTarget(pkgName, serviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceTest, UnbindServiceTarget_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1;
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnbindServiceTarget(pkgName, serviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, UnbindServiceTarget_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().UnbindServiceTarget(pkgName, serviceId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, UpdateServiceInfo_001, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 10020;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().UpdateServiceInfo(serviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, UpdateServiceInfo_002, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 10020;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceProfileConnectorMock_, PutServiceInfoProfile(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().UpdateServiceInfo(serviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceTest, UpdateServiceInfo_003, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 10020;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceProfileConnectorMock_, PutServiceInfoProfile(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().UpdateServiceInfo(serviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceTest, SetLocalDeviceName_203, testing::ext::TestSize.Level1)
{
    std::string pkgName = "packName";
    std::string deviceName = "deviceName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().SetLocalDeviceName(pkgName, deviceName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, SetRemoteDeviceName_203, testing::ext::TestSize.Level1)
{
    std::string pkgName = "packName";
    std::string deviceId = "123456";
    std::string deviceName = "deviceName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, RestoreLocalDeviceName_203, testing::ext::TestSize.Level1)
{
    std::string pkgName = "packName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RestoreLocalDeviceName(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceTest, BindServiceTarget_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.serviceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindServiceTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
