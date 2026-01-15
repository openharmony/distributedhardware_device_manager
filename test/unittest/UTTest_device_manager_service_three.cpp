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
constexpr int8_t SERVICE_UNPUBLISHED_STATE = 0;
constexpr int32_t DP_PERMISSION_DENIED = 98566155;
constexpr int32_t SERVICE_PUBLISHED_STATE = 1;
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
    Mock::VerifyAndClearExpectations(deviceManagerServiceMock_.get());
    Mock::VerifyAndClearExpectations(permissionManagerMock_.get());
    Mock::VerifyAndClearExpectations(softbusListenerMock_.get());
    Mock::VerifyAndClearExpectations(deviceManagerServiceImplMock_.get());
    Mock::VerifyAndClearExpectations(deviceProfileConnectorMock_.get());
    Mock::VerifyAndClearExpectations(deviceNameManagerMock_.get());
}

void DeviceManagerServiceThreeTest::SetUpTestCase()
{
    DmDeviceManagerService::dmDeviceManagerService = deviceManagerServiceMock_;
    DmPermissionManager::dmPermissionManager = permissionManagerMock_;
    DmSoftbusListener::dmSoftbusListener = softbusListenerMock_;
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl = deviceManagerServiceImplMock_;
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock_;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    DmDeviceNameManager::dmDeviceNameManager_ = deviceNameManagerMock_;
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
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    deviceProfileConnectorMock_ = nullptr;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
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
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
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
    int ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, networkId);
    EXPECT_NE(ret, ERR_DM_NOT_INIT);
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
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action, param);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
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
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
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
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);

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
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
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
    int32_t ret = DeviceManagerService::GetInstance().RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, BindDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnBindDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, MineRequestCredential_301, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckCredential_301, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterUiStateCallback_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnRegisterUiStateCallback_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, NotifyEvent_301, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, BindTarget_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);

    bindParam.insert(std::make_pair(PARAM_KEY_META_TYPE, pkgName));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, DpAclAdd_301, testing::ext::TestSize.Level1)
{
    std::string udid = "udid";
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid);
    EXPECT_NE(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsSameAccount_301, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckIsSameAccount_301, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckAccessControl_301, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopAuthenticateDevice_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_003";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthCode_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthCode_301, testing::ext::TestSize.Level1)
{
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);

    int32_t userId = 0;
    std::string accountId;
    std::string accountName;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().HandleAccountLogout(userId, accountId, accountName);

    int32_t preUserId = 1;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().HandleUserRemoved(preUserId);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnbindTarget_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_301, testing::ext::TestSize.Level1)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
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
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterAuthenticationType_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> authParam;
    authParam.insert(std::make_pair(DM_AUTHENTICATION_TYPE, "123456"));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INIT_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceProfileInfoList_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DmDeviceProfileInfoFilterOptions filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceIconInfo_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DmDeviceIconInfoFilterOptions filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceIconInfo(pkgName, filterOptions);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceInfo_301, testing::ext::TestSize.Level1)
{
    std::string networkId = "networkId";
    DmDeviceInfo deviceInfo;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    EXPECT_NE(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceThreeTest, PutDeviceProfileInfoList_301, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::vector<DmDeviceProfileInfo> deviceProfileInfoList;
    int32_t ret = DeviceManagerService::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetDeviceNamePrefixs_301, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    auto ret = DeviceManagerService::GetInstance().GetDeviceNamePrefixs();
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DeviceManagerServiceThreeTest, OnPinHolderSessionOpened_001, testing::ext::TestSize.Level1)
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

HWTEST_F(DeviceManagerServiceThreeTest, IsDMImplSoLoaded_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    bool ret = DeviceManagerService::GetInstance().IsDMImplSoLoaded();
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, DmHiDumper_001, testing::ext::TestSize.Level1)
{
    std::vector<std::string> args;
    std::string result;
    int32_t ret = DeviceManagerService::GetInstance().DmHiDumper(args, result);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceThreeTest, GenerateEncryptedUuid_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string uuid;
    std::string appId;
    std::string encryptedUuid;
    int32_t ret = DeviceManagerService::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, GenerateEncryptedUuid_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string uuid;
    std::string appId;
    std::string encryptedUuid;
    int32_t ret = DeviceManagerService::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceThreeTest, HandleDeviceStatusChange_001, testing::ext::TestSize.Level1)
{
    DmDeviceState devState = DmDeviceState::DEVICE_INFO_READY;
    DmDeviceInfo devInfo;
    DeviceManagerService::GetInstance().HandleDeviceStatusChange(devState, devInfo, true);
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DeviceManagerServiceThreeTest, SendAppUnBindBroadCast_001, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    int32_t userId = 12;
    uint64_t tokenId = 23;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendAppUnBindBroadCast(peerUdids, userId, tokenId);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceThreeTest, SendAppUnBindBroadCast_002, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    int32_t userId = 12;
    uint64_t peerTokenId = 3;
    uint64_t tokenId = 23;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendAppUnBindBroadCast(peerUdids, userId, tokenId, peerTokenId);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceThreeTest, SendServiceUnBindBroadCast_001, testing::ext::TestSize.Level1)
{
    std::vector<std::string> peerUdids;
    int32_t userId = 12;
    uint64_t tokenId = 23;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendServiceUnBindBroadCast(peerUdids, userId, tokenId);
    EXPECT_NE(DeviceManagerService::GetInstance().softbusListener_, nullptr);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}

HWTEST_F(DeviceManagerServiceThreeTest, GetProxyInfosByParseExtra_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string extra = "extra";
    std::vector<std::pair<int64_t, int64_t>> agentToProxyVec;
    std::set<std::pair<std::string, std::string>> proxyInfos;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    proxyInfos = DeviceManagerService::GetInstance().GetProxyInfosByParseExtra(pkgName, extra, agentToProxyVec);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    EXPECT_NE(proxyInfos.empty(), true);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthCode_302, testing::ext::TestSize.Level1)
{
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_FALSE(ret == ERR_DM_INPUT_PARA_INVALID || ret == ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartDiscovering_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    std::map<std::string, std::string> discoverParam;
    discoverParam[PARAM_KEY_META_TYPE] = "testMetaType";
    std::map<std::string, std::string> filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).Times(AnyNumber()).WillRepeatedly(Return(false));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, DisableDiscoveryListener_005, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(false));

    DeviceManagerService::GetInstance().InitDMServiceListener();

    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_TRUE(ret == ERR_DM_NOT_INIT || ret == ERR_DM_NO_PERMISSION);

    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceThreeTest, DisableDiscoveryListener_006, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).Times(AnyNumber()).WillOnce(Return(true));

    DeviceManagerService::GetInstance().UninitDMServiceListener();

    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceThreeTest, DisableDiscoveryListener_007, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).Times(AnyNumber()).WillOnce(Return(true));

    DeviceManagerService::GetInstance().InitDMServiceListener();

    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);

    EXPECT_NE(ret, ERR_DM_POINT_NULL);

    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceThreeTest, StartAdvertising_004, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).Times(AnyNumber()).WillOnce(Return(true));

    DeviceManagerService::GetInstance().InitDMServiceListener();

    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    DeviceManagerService::GetInstance().UninitDMServiceListener();
}

HWTEST_F(DeviceManagerServiceThreeTest, StartAdvertising_005, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).Times(AnyNumber()).WillOnce(Return(true));

    DeviceManagerService::GetInstance().UninitDMServiceListener();

    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_302, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(ERR_DM_FAILED)));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_303, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "invalid_process";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(processName))
        .Times(AnyNumber()).WillOnce(Return(false));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_304, testing::ext::TestSize.Level1)
{
    std::string packName = "";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(processName))
        .Times(AnyNumber()).WillOnce(Return(true));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_305, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(processName))
        .Times(AnyNumber()).WillOnce(Return(true));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_306, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    std::string processName = "collaboration_service";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(processName))
        .Times(AnyNumber()).WillOnce(Return(true));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_307, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "invalid_number";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(processName))
        .Times(AnyNumber()).WillOnce(Return(true));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_308, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "invalid_number";
    std::string processName = "collaboration_service";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(processName))
        .Times(AnyNumber()).WillOnce(Return(true));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, SetDnPolicy_309, testing::ext::TestSize.Level1)
{
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::string processName = "collaboration_service";
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(processName))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad())
        .Times(AnyNumber()).WillOnce(Return(false));
    
    int32_t ret = DeviceManagerService::GetInstance().SetDnPolicy(packName, policy);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, SyncLocalAclListProcess_001, testing::ext::TestSize.Level1)
{
    DevUserInfo localDevUserInfo;
    DevUserInfo remoteDevUserInfo;
    std::string remoteAclList = "testAclList";
    bool isDelImmediately = false;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().SyncLocalAclListProcess(localDevUserInfo,
        remoteDevUserInfo, remoteAclList, isDelImmediately);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetAnonyLocalUdid_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string anonyUdid;
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));

    int32_t ret = DeviceManagerService::GetInstance().GetAnonyLocalUdid(pkgName, anonyUdid);

    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, GetAnonyLocalUdid_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string anonyUdid;
    
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    
    int32_t ret = DeviceManagerService::GetInstance().GetAnonyLocalUdid(pkgName, anonyUdid);
    EXPECT_TRUE(ret == DM_OK || ret == ERR_DM_FAILED);
    
    if (ret == DM_OK) {
        EXPECT_FALSE(anonyUdid.empty());
    }
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckSrcAccessControl_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;

    caller.pkgName = "com.ohos.test";
    caller.networkId = "net_src";
    callee.networkId = "net_sink";

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(false));

    bool ret = DeviceManagerService::GetInstance().CheckSrcAccessControl(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckSinkAccessControl_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    caller.pkgName = "com.ohos.test";
    caller.networkId = "net_src";
    callee.networkId = "net_sink";

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(false));

    bool ret = DeviceManagerService::GetInstance().CheckSinkAccessControl(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckSrcIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    caller.pkgName = "com.ohos.test";
    caller.networkId = "net_src";
    callee.networkId = "net_sink";

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(false));

    bool ret = DeviceManagerService::GetInstance().CheckSrcIsSameAccount(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, CheckSinkIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;

    caller.pkgName = "com.ohos.test";
    caller.networkId = "net_src";
    callee.networkId = "net_sink";

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(false));

    bool ret = DeviceManagerService::GetInstance().CheckSinkIsSameAccount(caller, callee);
    EXPECT_FALSE(ret);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopServiceDiscovery_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 12345;

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad())
        .Times(AnyNumber()).WillOnce(Return(false));

    int32_t ret = DeviceManagerService::GetInstance().StopServiceDiscovery(pkgName, discParam.discoveryServiceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnbindServiceTarget_005, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "com.ohos.test";
    const int64_t serviceId = 12345;

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnbindServiceTarget(pkgName, serviceId);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnbindServiceTarget_006, testing::ext::TestSize.Level1)
{
    const std::string pkgName = "com.ohos.test";
    const int64_t serviceId = 12345;

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).Times(AnyNumber()).WillOnce(Return(false));

    int32_t ret = DeviceManagerService::GetInstance().UnbindServiceTarget(pkgName, serviceId);
    EXPECT_EQ(ret, ERR_DM_NOT_INIT);
}

HWTEST_F(DeviceManagerServiceThreeTest, OpenAuthSessionWithPara_001, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 12345;
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().OpenAuthSessionWithPara(serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartServiceDiscovery_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 12345;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartServiceDiscovery(pkgName, discParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartServiceDiscovery_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "";
    discParam.discoveryServiceId = 12345;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartServiceDiscovery(pkgName, discParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartServiceDiscovery_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartServiceDiscovery(pkgName, discParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartServiceDiscovery_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 12345;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().StartServiceDiscovery(pkgName, discParam);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopServiceDiscovery_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    int32_t discServiceId = 12345;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopServiceDiscovery(pkgName, discServiceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopServiceDiscovery_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t discServiceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopServiceDiscovery(pkgName, discServiceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopServiceDiscovery_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t discServiceId = 12345;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().StopServiceDiscovery(pkgName, discServiceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterServiceInfo_002, testing::ext::TestSize.Level1)
{
    ServiceRegInfo serviceRegInfo;
    serviceRegInfo.serviceInfo.serviceId = 12345;
    serviceRegInfo.serviceInfo.serviceType = "";
    serviceRegInfo.serviceInfo.serviceName = "TestService";
    serviceRegInfo.serviceInfo.serviceDisplayName = "TestServiceDisplay";
    int32_t regServiceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterServiceInfo(serviceRegInfo, regServiceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterServiceInfo_003, testing::ext::TestSize.Level1)
{
    ServiceRegInfo serviceRegInfo;
    serviceRegInfo.serviceInfo.serviceId = 12345;
    serviceRegInfo.serviceInfo.serviceType = "TestService";
    serviceRegInfo.serviceInfo.serviceName = "";
    serviceRegInfo.serviceInfo.serviceDisplayName = "TestServiceDisplay";
    int32_t regServiceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterServiceInfo(serviceRegInfo, regServiceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterServiceInfo_004, testing::ext::TestSize.Level1)
{
    ServiceRegInfo serviceRegInfo;
    serviceRegInfo.serviceInfo.serviceId = 12345;
    serviceRegInfo.serviceInfo.serviceType = "TestService";
    serviceRegInfo.serviceInfo.serviceName = "TestName";
    serviceRegInfo.serviceInfo.serviceDisplayName = "TestServiceDisplay";
    int32_t regServiceId = 123456;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, CheckServiceHasRegistered(_, _, _)).Times(0);
    int32_t ret = DeviceManagerService::GetInstance().RegisterServiceInfo(serviceRegInfo, regServiceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterServiceInfo_005, testing::ext::TestSize.Level1)
{
    ServiceRegInfo serviceRegInfo;
    int32_t regServiceId = 12345;
    serviceRegInfo.serviceInfo.serviceType = "TestService";
    serviceRegInfo.serviceInfo.serviceName = "TestName";
    serviceRegInfo.serviceInfo.serviceDisplayName = "TestServiceDisplay";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, CheckServiceHasRegistered(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, GenerateRegServiceId(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().RegisterServiceInfo(serviceRegInfo, regServiceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterServiceInfo_006, testing::ext::TestSize.Level1)
{
    ServiceRegInfo serviceRegInfo;
    int32_t regServiceId = 12345;
    serviceRegInfo.serviceInfo.serviceType = "TestService";
    serviceRegInfo.serviceInfo.serviceName = "TestName";
    serviceRegInfo.serviceInfo.serviceDisplayName = "TestServiceDisplay";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, CheckServiceHasRegistered(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, GenerateRegServiceId(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceProfileConnectorMock_, PutServiceInfoProfile(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().RegisterServiceInfo(serviceRegInfo, regServiceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceThreeTest, UnRegisterServiceInfo_002, testing::ext::TestSize.Level1)
{
    int32_t regServiceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterServiceInfo(regServiceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PublishServiceParam param;
    int64_t serviceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, param, serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PublishServiceParam param;
    param.regServiceId = 0;
    int64_t serviceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, param, serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int64_t serviceId = 100;
    ServiceInfoProfile serviceInfotemp;
    serviceInfotemp.serviceName = "serviceName";
    serviceInfotemp.serviceType = "serviceType";
    serviceInfotemp.serviceDisplayName = "serviceDisplayName";
    serviceInfotemp.regServiceId = 191;
    PublishServiceParam publishServiceParam;
    publishServiceParam.regServiceId = 191;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByRegServiceId(_, _)).Times(AnyNumber())
        .WillOnce(DoAll(SetArgReferee<1>(serviceInfotemp), Return(ERR_DM_FAILED)));
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceParam, serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int64_t serviceId = 100;
    ServiceInfoProfile serviceInfotemp;
    serviceInfotemp.serviceName = "serviceName";
    serviceInfotemp.serviceType = "serviceType";
    serviceInfotemp.serviceDisplayName = "serviceDisplayName";
    serviceInfotemp.regServiceId = 120;
    serviceInfotemp.tokenId = 0;
    PublishServiceParam publishServiceParam;
    publishServiceParam.regServiceId = 191;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByRegServiceId(_, _)).Times(AnyNumber())
        .WillOnce(DoAll(SetArgReferee<1>(serviceInfotemp), Return(DM_OK)));
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceParam, serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int64_t serviceId = 100;
    ServiceInfoProfile serviceInfotemp;
    serviceInfotemp.serviceId = 100;
    serviceInfotemp.serviceName = "serviceName";
    serviceInfotemp.serviceType = "serviceType";
    serviceInfotemp.serviceDisplayName = "serviceDisplayName";
    serviceInfotemp.publishState = SERVICE_PUBLISHED_STATE;
    PublishServiceParam publishServiceParam;
    publishServiceParam.regServiceId = 191;
    serviceInfotemp.tokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByRegServiceId(_, _)).Times(AnyNumber())
        .WillOnce(DoAll(SetArgReferee<1>(serviceInfotemp), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).Times(0);
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceParam, serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int64_t serviceId = 100;
    ServiceInfoProfile serviceInfotemp;
    serviceInfotemp.serviceId = 0;
    serviceInfotemp.serviceName = "serviceName";
    serviceInfotemp.serviceType = "serviceType";
    serviceInfotemp.serviceDisplayName = "serviceDisplayName";
    PublishServiceParam publishServiceParam;
    publishServiceParam.regServiceId = 191;
    serviceInfotemp.tokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByRegServiceId(_, _)).Times(AnyNumber())
        .WillOnce(DoAll(SetArgReferee<1>(serviceInfotemp), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceMock_, GenerateServiceId(_)).Times(0);
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceParam, serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int64_t serviceId = 100;
    ServiceInfoProfile serviceInfotemp;
    serviceInfotemp.serviceId = 0;
    serviceInfotemp.serviceName = "serviceName";
    serviceInfotemp.serviceType = "serviceType";
    serviceInfotemp.serviceDisplayName = "serviceDisplayName";
    PublishServiceParam publishServiceParam;
    publishServiceParam.regServiceId = 191;
    serviceInfotemp.tokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByRegServiceId(_, _)).Times(AnyNumber())
        .WillOnce(DoAll(SetArgReferee<1>(serviceInfotemp), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceMock_, GenerateServiceId(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceProfileConnectorMock_, PutServiceInfoProfile(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceParam, serviceId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, StartPublishService_009, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int64_t serviceId = 100;
    ServiceInfoProfile serviceInfotemp;
    serviceInfotemp.serviceId = 0;
    serviceInfotemp.serviceName = "serviceName";
    serviceInfotemp.serviceType = "serviceType";
    serviceInfotemp.serviceDisplayName = "serviceDisplayName";
    PublishServiceParam publishServiceParam;
    publishServiceParam.regServiceId = 191;
    serviceInfotemp.tokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByRegServiceId(_, _)).Times(AnyNumber())
        .WillOnce(DoAll(SetArgReferee<1>(serviceInfotemp), Return(DM_OK)));
    EXPECT_CALL(*deviceManagerServiceMock_, GenerateServiceId(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceProfileConnectorMock_, PutServiceInfoProfile(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceAdapterResidentLoad()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().StartPublishService(pkgName, publishServiceParam, serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopPublishService_002, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopPublishService(serviceId);
    EXPECT_NE(ret, ERR_DM_BIND_TIMEOUT_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopPublishService_003, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 123456;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(serviceId, _)).Times(AnyNumber()).
        WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().StopPublishService(serviceId);
    EXPECT_NE(ret, ERR_DM_BIND_TIMEOUT_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopPublishService_004, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 123456;
    ServiceInfoProfile serviceInfo;
    serviceInfo.publishState = SERVICE_UNPUBLISHED_STATE;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(serviceId, _)).Times(AnyNumber()).
        WillOnce(DoAll(SetArgReferee<1>(serviceInfo), Return(DM_OK)));
    int32_t ret = DeviceManagerService::GetInstance().StopPublishService(serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopPublishService_005, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 123456;
    ServiceInfoProfile serviceInfo;
    serviceInfo.publishState = SERVICE_PUBLISHED_STATE;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(serviceId, _)).Times(AnyNumber()).
        WillOnce(DoAll(SetArgReferee<1>(serviceInfo), Return(DM_OK)));
    int32_t ret = DeviceManagerService::GetInstance().StopPublishService(serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, StopPublishService_006, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 123456;
    ServiceInfoProfile serviceInfo;
    serviceInfo.publishState = SERVICE_PUBLISHED_STATE;
    serviceInfo.tokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(serviceId, _)).Times(AnyNumber()).
        WillOnce(DoAll(SetArgReferee<1>(serviceInfo), Return(DM_OK)));
    int32_t ret = DeviceManagerService::GetInstance().StopPublishService(serviceId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerServiceThreeTest, ConvertServiceInfoProfileByRegInfo_001, testing::ext::TestSize.Level1)
{
    ServiceRegInfo regInfo;
    regInfo.serviceInfo.serviceId = 123;
    regInfo.serviceInfo.serviceType = "1";
    regInfo.serviceInfo.serviceName = "TestService";
    regInfo.serviceInfo.serviceDisplayName = "TestDisplayName";
    ServiceInfoProfile profile;
    int64_t tokenId = 0;
    int32_t ret = DeviceManagerService::GetInstance().ConvertServiceInfoProfileByRegInfo(regInfo, profile, tokenId);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(profile.serviceId, 123);
    EXPECT_EQ(profile.serviceType, std::to_string(1));
    EXPECT_EQ(profile.serviceName, "TestService");
    EXPECT_EQ(profile.serviceDisplayName, "TestDisplayName");
    EXPECT_FALSE(profile.deviceId.empty());
}

HWTEST_F(DeviceManagerServiceThreeTest, ConvertServiceInfoProfileByRegInfo_002, testing::ext::TestSize.Level1)
{
    ServiceRegInfo regInfo;
    ServiceInfoProfile profile;
    int64_t tokenId = 0;
    int32_t ret = DeviceManagerService::GetInstance().ConvertServiceInfoProfileByRegInfo(regInfo, profile, tokenId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceThreeTest, GenerateServiceId_002, testing::ext::TestSize.Level1)
{
    int64_t generatedServiceId = 123;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetServiceInfoProfileByServiceId(_, _)).Times(AnyNumber())
        .WillRepeatedly(::testing::Return(ERR_DM_SERVICE_INFO_NOT_EXIST));
    int32_t result = DeviceManagerService::GetInstance().GenerateServiceId(generatedServiceId);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthInfo_001, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ImportAuthInfo(dmAuthInfo);
    EXPECT_EQ(result, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthInfo_002, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(ERR_DM_FAILED));
    int32_t result = DeviceManagerService::GetInstance().ImportAuthInfo(dmAuthInfo);
    EXPECT_EQ(result,  ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthInfo_003, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ImportAuthInfo(dmAuthInfo);
    EXPECT_EQ(result, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthInfo_004, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsImportAuthInfoValid(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady())
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ImportAuthInfo(dmAuthInfo);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthInfo_005, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsImportAuthInfoValid(_))
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ImportAuthInfo(dmAuthInfo);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, ImportAuthInfo_006, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    dmAuthInfo.userId = 100;
    dmAuthInfo.pinConsumerPkgName = "com.ohos.test.pin";
    dmAuthInfo.bizSrcPkgName = "com.ohos.test.biz";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceImplMock_, ImportAuthInfo(_)).Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceManagerServiceMock_, IsImportAuthInfoValid(_)).Times(AnyNumber()).WillOnce(Return(true));
    int32_t result = DeviceManagerService::GetInstance().ImportAuthInfo(dmAuthInfo);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_001, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 6;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_002, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 6;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(ERR_DM_FAILED));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result,  ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_003, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 6;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_004, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 6;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsExportAuthInfoValid(_))
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_005, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 6;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsExportAuthInfoValid(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady())
        .Times(AnyNumber()).WillOnce(Return(false));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_006, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 5;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceMock_, IsExportAuthInfoValid(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_007, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 5;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .Times(AnyNumber()).WillOnce(Return(ERR_DM_FAILED));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, ExportAuthInfo_008, testing::ext::TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    uint32_t pinlength = 5;
    std::vector<int32_t> userIds = {100, 200};
    dmAuthInfo.userId = 300;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission())
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .Times(AnyNumber()).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_))
        .Times(AnyNumber()).WillOnce(Return(true));
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .Times(AnyNumber()).WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DM_OK)));
    int32_t result = DeviceManagerService::GetInstance().ExportAuthInfo(dmAuthInfo, pinlength);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsExportAuthInfoValid_001, TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(Return(ERR_DM_FAILED));
    bool result = DeviceManagerService::GetInstance().IsExportAuthInfoValid(dmAuthInfo);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsExportAuthInfoValid_002, TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    std::vector<int32_t> userIds = {100, 200};
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DM_OK)));
    dmAuthInfo.userId = 300;
    bool result = DeviceManagerService::GetInstance().IsExportAuthInfoValid(dmAuthInfo);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsExportAuthInfoValid_003, TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    std::vector<int32_t> userIds = {100};
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DM_OK)));
    dmAuthInfo.userId = 100;
    dmAuthInfo.pinConsumerPkgName = std::string(2025, 'a');
    bool result = DeviceManagerService::GetInstance().IsExportAuthInfoValid(dmAuthInfo);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsExportAuthInfoValid_004, TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    std::vector<int32_t> userIds = {100};
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DM_OK)));
    dmAuthInfo.userId = 100;
    dmAuthInfo.pinConsumerPkgName = "com.test.valid";
    dmAuthInfo.bizSrcPkgName = std::string(2025, 'b');
    bool result = DeviceManagerService::GetInstance().IsExportAuthInfoValid(dmAuthInfo);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsExportAuthInfoValid_005, TestSize.Level1)
{
    DmAuthInfo dmAuthInfo;
    std::vector<int32_t> userIds = {100};
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .Times(::testing::AtLeast(1)).WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DM_OK)));
    dmAuthInfo.userId = 100;
    dmAuthInfo.pinConsumerPkgName = "com.test.valid";
    bool result = DeviceManagerService::GetInstance().IsExportAuthInfoValid(dmAuthInfo);
    EXPECT_TRUE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_001, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName[0] = '\0';
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_002, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = "valid_meta";
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    info.authBoxType = DMLocalServiceInfoAuthBoxType::STATE3;
    info.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    info.description = "valid desc";
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_003, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123456";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = std::string(DM_MAX_PIN_CODE_LEN, 'a');
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    info.authBoxType = DMLocalServiceInfoAuthBoxType::STATE3;
    info.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_004, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123456";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = "valid_meta";
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = static_cast<DMLocalServiceInfoAuthType>(-1);
    info.authBoxType = DMLocalServiceInfoAuthBoxType::STATE3;
    info.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    info.description = "valid desc";
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_005, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123456";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = "valid_meta";
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    info.authBoxType = static_cast<DMLocalServiceInfoAuthBoxType>(
        static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::STATE3) - 1);
    info.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    info.description = "valid desc";
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_006, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123456";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = "valid_meta";
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    info.authBoxType = static_cast<DMLocalServiceInfoAuthBoxType>(
        static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::TWO_IN1) + 1);
    info.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    info.description = "valid desc";
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_007, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123456";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = "valid_meta";
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    info.authBoxType = DMLocalServiceInfoAuthBoxType::STATE3;
    info.pinExchangeType = static_cast<DMLocalServiceInfoPinExchangeType>(
        static_cast<int32_t>(DMLocalServiceInfoPinExchangeType::PINBOX) - 1);
    info.description = "valid desc";
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_008, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123456";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = "valid_meta";
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    info.authBoxType = DMLocalServiceInfoAuthBoxType::STATE3;
    info.pinExchangeType = static_cast<DMLocalServiceInfoPinExchangeType>(
        static_cast<int32_t>(DMLocalServiceInfoPinExchangeType::ULTRASOUND) + 1);
    info.description = "valid desc";
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, IsImportAuthInfoValid_009, TestSize.Level1)
{
    DmAuthInfo info;
    info.userId = 100;
    info.pinConsumerPkgName = "com.test.valid";
    string pinCodeTest = "123456";
    strncpy_s(info.pinCode, DM_MAX_PIN_CODE_LEN, pinCodeTest.c_str(), pinCodeTest.length());
    string metaTokenTest = "valid_meta";
    strncpy_s(info.metaToken, DM_MAX_META_TOKEN_LEN, metaTokenTest.c_str(), metaTokenTest.length());
    info.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    info.authBoxType = DMLocalServiceInfoAuthBoxType::STATE3;
    info.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    info.description = std::string(DM_MAX_PIN_CODE_LEN, 'd');
    bool result = DeviceManagerService::GetInstance().IsImportAuthInfoValid(info);
    EXPECT_FALSE(result);
}

HWTEST_F(DeviceManagerServiceThreeTest, InitTaskOfDelTimeOutAcl_002, TestSize.Level0)
{
    if (DeviceManagerService::GetInstance().discoveryMgr_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> aclInfos;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAllAuthOnceAclInfos(_))
        .WillOnce(DoAll(SetArgReferee<0>(aclInfos), Return(0)));
    DeviceManagerService::GetInstance().InitTaskOfDelTimeOutAcl();
}

HWTEST_F(DeviceManagerServiceThreeTest, InitTaskOfDelTimeOutAcl_003, TestSize.Level0)
{
    if (DeviceManagerService::GetInstance().discoveryMgr_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    AuthOnceAclInfo aclInfo;
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> aclInfos {aclInfo};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAllAuthOnceAclInfos(_))
        .WillOnce(DoAll(SetArgReferee<0>(aclInfos), Return(0)));
    EXPECT_CALL(*deviceManagerServiceMock_, IsDMServiceImplReady()).WillOnce(Return(false));
    DeviceManagerService::GetInstance().InitTaskOfDelTimeOutAcl();
}

HWTEST_F(DeviceManagerServiceThreeTest, StopDiscovering_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, EnableDiscoveryListener_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, EnableDiscoveryListener_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceThreeTest, DestroyPinHolder_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerServiceThreeTest, DestroyPinHolder_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));

    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(processName)).WillOnce(Return(true));

    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, DestroyPinHolder_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return((DM_OK)));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerServiceThreeTest, RegisterAuthenticationType_303, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.test.auth";
    std::map<std::string, std::string> authParam;
    authParam["OTHER_KEY"] = "test";

    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
