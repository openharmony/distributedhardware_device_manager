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

#include "UTTest_device_manager_service_three.h"

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

void DeviceManagerServiceThreeTest::SetUp()
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

void DeviceManagerServiceThreeTest::TearDown()
{
}

void DeviceManagerServiceThreeTest::SetUpTestCase()
{
    DmDeviceManagerService::dmDeviceManagerService = deviceManagerServiceMock_;
    DmPermissionManager::dmPermissionManager = permissionManagerMock_;
    DmSoftbusListener::dmSoftbusListener = softbusListenerMock_;
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl = deviceManagerServiceImplMock_;
}

void DeviceManagerServiceThreeTest::TearDownTestCase()
{
    DmDeviceManagerService::dmDeviceManagerService = nullptr;
    deviceManagerServiceMock_ = nullptr;
    DmPermissionManager::dmPermissionManager = nullptr;
    permissionManagerMock_ = nullptr;
    DmSoftbusListener::dmSoftbusListener = nullptr;
    softbusListenerMock_ = nullptr;
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl = nullptr;
    deviceManagerServiceImplMock_ = nullptr;
}

namespace {

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
 * @tc.name: AuthenticateDevice_301
 * @tc.desc: Set unsupport authType = 0 and return ERR_DM_NOT_INIT
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceThreeTest, AuthenticateDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "jdddd";
    int32_t authType = 1;
    std::string deviceId = "deviceId";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

/**
 * @tc.name: UnAuthenticateDevice_301
 * @tc.desc: Set intFlag for UnAuthenticateDevice to true and pkgName to com.ohos.test; set deviceId null ，The return
 * value is SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceThreeTest, UnAuthenticateDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "12345";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

/**
 * @tc.name: SetUserOperation_301
 * @tc.desc: Make pkgName empty for SetUserOperation，The return value is
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceThreeTest, SetUserOperation_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int32_t action = 0;
    const std::string param = "extra";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

/**
 * @tc.name: RequestCredential_301
 * @tc.desc:The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceThreeTest, RequestCredential_301, testing::ext::TestSize.Level1)
{
    const std::string reqJsonStr = "test";
    std::string returnJsonStr = "returntest";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

/**
 * @tc.name: ImportCredential_301
 * @tc.desc:The return value is ERR_DM_NOT_INIT
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceThreeTest, ImportCredential_301, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    const std::string credentialInfo = "credentialInfotest";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);

    std::string reqJsonStr = "";
    std::string returnJsonStr = "";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

/**
 * @tc.name: DeleteCredential_301
 * @tc.desc:The return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceThreeTest, DeleteCredential_301, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    const std::string deleteInfo = "deleteInfotest";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

/**
 * @tc.name: RegisterCredentialCallback_301
 * @tc.desc: The return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceThreeTest, RegisterCredentialCallback_301, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "pkgNametest";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, BindDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnBindDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, MineRequestCredential_301, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckCredential_301, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterUiStateCallback_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnRegisterUiStateCallback_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, NotifyEvent_301, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, BindTarget_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);

    bindParam.insert(std::make_pair(PARAM_KEY_META_TYPE, pkgName));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, DpAclAdd_301, testing::ext::TestSize.Level1)
{
    std::string udid = "udid";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsSameAccount_301, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckIsSameAccount_301, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckAccessControl_301, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopAuthenticateDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_003";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthCode_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);

    std::vector<DmDeviceInfo> deviceList;
    DmDeviceInfo dmDeviceInfo;
    dmDeviceInfo.authForm = DmAuthForm::ACROSS_ACCOUNT;
    deviceList.push_back(dmDeviceInfo);
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*deviceManagerServiceMock_, GetTrustedDeviceList(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(deviceList), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().LoadHardwareFwkService();
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthCode_301, testing::ext::TestSize.Level1)
{
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);

    int32_t userId = 0;
    std::string accountId;
    std::string accountName;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterSoLoaded()).WillOnce(Return(false));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().HandleAccountLogout(userId, accountId, accountName);

    int32_t curUserId = 0;
    int32_t preUserId = 1;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().HandleUserSwitched(curUserId, preUserId);

    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().HandleUserRemoved(preUserId);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnbindTarget_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_301, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);

    std::string msg = "msg";
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnBindDevice_302, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    std::string extra;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterAuthenticationType_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> authParam;
    authParam.insert(std::make_pair(DM_AUTHENTICATION_TYPE, "123456"));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INIT_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceProfileInfoList_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DmDeviceProfileInfoFilterOptions filterOptions;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceIconInfo_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DmDeviceIconInfoFilterOptions filterOptions;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceIconInfo(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceInfo_301, testing::ext::TestSize.Level1)
{
    std::string networkId = "networkId";
    DmDeviceInfo deviceInfo;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceThreeTest, PutDeviceProfileInfoList_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::vector<DmDeviceProfileInfo> deviceProfileInfoList;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceNamePrefixs_301, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    auto ret = DeviceManagerService::GetInstance().GetDeviceNamePrefixs();
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceManagerServiceThreeTest, SetLocalDeviceName_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string deviceName = "deviceName";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyLocalDeviceName(_)).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().SetLocalDeviceName(pkgName, deviceName);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetRemoteDeviceName_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string deviceName = "deviceName";
    std::string deviceId = "d*********3";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidModifyRemoteDeviceName(_)).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceNetworkIdList_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    NetworkIdQueryFilter queryFilter;
    std::vector<std::string> networkIds{"uehd*****87"};
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceNetworkIdList(pkgName, queryFilter, networkIds);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
