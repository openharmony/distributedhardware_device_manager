/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_auth_manager.h"

#include "distributed_device_profile_client.h"
#include "dm_auth_state.h"

using namespace testing;
namespace OHOS {
namespace DistributedHardware {

void AuthManagerTest::SetUp()
{
    InitDeviceAuthService();
    softbusConnector = std::make_shared<SoftbusConnector>();
    deviceManagerServicelistener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, deviceManagerServicelistener,
        hiChainAuthConnector);
}

void AuthManagerTest::TearDown()
{
    softbusConnector = nullptr;
    deviceManagerServicelistener = nullptr;
    hiChainAuthConnector = nullptr;
    hiChainConnector = nullptr;
    authManager = nullptr;
}

void AuthManagerTest::SetUpTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

void AuthManagerTest::TearDownTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
}

HWTEST_F(AuthManagerTest, ParseJsonObject_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    authManager->ParseJsonObject(jsonObject);
    EXPECT_EQ(jsonObject[DM_BUSINESS_ID].Get<std::string>(), "");
}

HWTEST_F(AuthManagerTest, OnUserOperation_001, testing::ext::TestSize.Level1)
{
    int32_t action = UiAction::USER_OPERATION_TYPE_CANCEL_AUTH;
    std::string params = "testParams";
    int32_t ret = authManager->OnUserOperation(action, params);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthManagerTest, OnUserOperation_002, testing::ext::TestSize.Level1)
{
    authManager->SetAuthContext(nullptr);

    int32_t action = USER_OPERATION_TYPE_CANCEL_AUTH;
    std::string params = "";

    int32_t ret = authManager->OnUserOperation(action, params);

    EXPECT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(AuthManagerTest, OnUserOperation_003, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->businessId = "";

    int32_t action = USER_OPERATION_TYPE_CANCEL_AUTH;
    std::string params = "";

    int32_t ret = authManager->OnUserOperation(action, params);

    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthManagerTest, OnUserOperation_004, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->businessId = "testBusinessId";

    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutBusinessEvent(::testing::_))
        .WillOnce(::testing::Return(ERR_DM_FAILED));

    int32_t action = USER_OPERATION_TYPE_CANCEL_AUTH;
    std::string params = "";

    int32_t ret = authManager->OnUserOperation(action, params);

    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthManagerTest, OnUserOperation_007, testing::ext::TestSize.Level1)
{
    int32_t action = -1;
    std::string params = "";

    int32_t ret = authManager->OnUserOperation(action, params);

    EXPECT_EQ(ret, DM_OK);
}

/* *
 * @tc.name: ImportAuthCode_001
 * @tc.desc: Test ImportAuthCode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ImportAuthCode_001, testing::ext::TestSize.Level1)
{
    std::string authCode = "";
    std::string pkgName = "pkgName";
    int32_t ret = authManager->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/* *
 * @tc.name: ImportAuthCode_002
 * @tc.desc: Test ImportAuthCode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ImportAuthCode_002, testing::ext::TestSize.Level1)
{
    std::string authCode = "123456";
    std::string pkgName = "";
    int32_t ret = authManager->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/* *
 * @tc.name: ImportAuthCode_003
 * @tc.desc: Test ImportAuthCode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ImportAuthCode_003, testing::ext::TestSize.Level1)
{
    std::string authCode = "123456";
    std::string pkgName = "pkgName";
    int32_t ret = authManager->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, DM_OK);
}

/* *
 * @tc.name: IsAuthCodeReady_001
 * @tc.desc: Test IsAuthCodeReady
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, IsAuthCodeReady_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    authManager->context_->importAuthCode = "";
    authManager->context_->importPkgName = "importPkgName";
    bool ret = authManager->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

/* *
 * @tc.name: IsAuthCodeReady_002
 * @tc.desc: Test IsAuthCodeReady
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, IsAuthCodeReady_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    authManager->context_->importAuthCode = "importAuthCode";
    authManager->context_->importPkgName = "";
    bool ret = authManager->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

HWTEST_F(AuthManagerTest, ParseUltrasonicSide_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[TAG_ULTRASONIC_SIDE] = "0";
    authManager->ParseUltrasonicSide(jsonObject);
    jsonObject[TAG_ULTRASONIC_SIDE] = "1";
    authManager->ParseUltrasonicSide(jsonObject);
    EXPECT_NE(authManager->context_, nullptr);
}

HWTEST_F(AuthManagerTest, AuthSrcConfirmState_NegotiateUltrasonic_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    std::shared_ptr<DmAuthContext> context = authManager->GetAuthContext();

    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    authState->NegotiateUltrasonic(nullptr);
    authState->NegotiateUltrasonic(context);

    context->authType = DmAuthType::AUTH_TYPE_PIN_ULTRASONIC;
    authState->NegotiateUltrasonic(context);

    context->accessee.extraInfo = "123456";
    authState->NegotiateUltrasonic(context);

    JsonObject json;
    json["isSupportUltrasonic"] = "123456";
    context->accessee.extraInfo = json.Dump();
    authState->NegotiateUltrasonic(context);

    json["isSupportUltrasonic"] = true;
    context->accessee.extraInfo = json.Dump();
    authState->NegotiateUltrasonic(context);

    json["isSupportUltrasonic"] = false;
    context->accessee.extraInfo = json.Dump();
    authState->NegotiateUltrasonic(context);
    EXPECT_EQ(context->authType, DmAuthType::AUTH_TYPE_PIN);
}

HWTEST_F(AuthManagerTest, GetSrcUserIdByDisplayIdAndDeviceType_001, testing::ext::TestSize.Level1)
{
    int32_t displayId = 0;
    DmDeviceType deviceType = DmDeviceType::DEVICE_TYPE_CAR;
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(-1));
    int32_t ret = authManager->GetSrcUserIdByDisplayIdAndDeviceType(displayId, deviceType);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(AuthManagerTest, GetSrcUserIdByDisplayIdAndDeviceType_002, testing::ext::TestSize.Level1)
{
    int32_t displayId = 0;
    DmDeviceType deviceType = DmDeviceType::DEVICE_TYPE_PHONE;
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(-1));
    int32_t ret = authManager->GetSrcUserIdByDisplayIdAndDeviceType(displayId, deviceType);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(AuthManagerTest, GetSrcCarUserIdByDisplayId_001, testing::ext::TestSize.Level1)
{
    int32_t displayId = 0;
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(-1));
    int32_t ret = authManager->GetSrcCarUserIdByDisplayId(displayId);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(AuthManagerTest, GetSrcCarUserIdByDisplayId_002, testing::ext::TestSize.Level1)
{
    int32_t displayId = -1;
    authManager->bindParam_["bindCallerIsSystemSA"] = "1";
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    int32_t ret = authManager->GetSrcCarUserIdByDisplayId(displayId);
    EXPECT_EQ(ret, 100);
}

HWTEST_F(AuthManagerTest, GetSrcCarUserIdByDisplayId_003, testing::ext::TestSize.Level1)
{
    int32_t displayId = 6;
    authManager->bindParam_["bindCallerIsSystemSA"] = "1";
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    int32_t ret = authManager->GetSrcCarUserIdByDisplayId(displayId);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(AuthManagerTest, GetSrcCarUserIdByDisplayId_004, testing::ext::TestSize.Level1)
{
    int32_t displayId = 0;
    authManager->bindParam_["bindCallerIsSystemSA"] = "1";
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    int32_t ret = authManager->GetSrcCarUserIdByDisplayId(displayId);
    EXPECT_EQ(ret, 100);
}

HWTEST_F(AuthManagerTest, GetSrcCarUserIdByDisplayId_005, testing::ext::TestSize.Level1)
{
    int32_t displayId = 0;
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    int32_t ret = authManager->GetSrcCarUserIdByDisplayId(displayId);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(AuthManagerTest, GetSrcCarUserIdByDisplayId_006, testing::ext::TestSize.Level1)
{
    int32_t displayId = 0;
    authManager->context_->processInfo.userId = 100;
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    int32_t ret = authManager->GetSrcCarUserIdByDisplayId(displayId);
    EXPECT_EQ(ret, 100);
}

/* *
 * @tc.name: OnUserOperation_008
 * @tc.desc: Test OnUserOperation with USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, OnUserOperation_008, testing::ext::TestSize.Level1)
{
    int32_t action = USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    std::string params = "";
    int32_t ret = authManager->OnUserOperation(action, params);
    EXPECT_EQ(ret, DM_OK);
}

/* *
 * @tc.name: OnUserOperation_010
 * @tc.desc: Test OnUserOperation ALLOW_AUTH with non-empty businessId triggers HandleBusinessEvents
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, OnUserOperation_010, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->businessId = "testBusinessId";
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutBusinessEvent(::testing::_))
        .WillOnce(::testing::Return(DM_OK));
    int32_t action = USER_OPERATION_TYPE_ALLOW_AUTH;
    std::string params = "";
    int32_t ret = authManager->OnUserOperation(action, params);
    EXPECT_EQ(ret, DM_OK);
}

/* *
 * @tc.name: GeneratePincode_001
 * @tc.desc: Test GeneratePincode returns a valid 6-digit pin within range
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, GeneratePincode_001, testing::ext::TestSize.Level1)
{
    std::string pinCode = authManager->GeneratePincode();
    EXPECT_FALSE(pinCode.empty());
    int32_t code = std::atoi(pinCode.c_str());
    EXPECT_GE(code, 100000);
    EXPECT_LE(code, 999999);
}

/* *
 * @tc.name: GetReason_001
 * @tc.desc: Test GetReason returns the value set in context reason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, GetReason_001, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->reason = STOP_BIND;
    int32_t ret = authManager->GetReason();
    EXPECT_EQ(ret, STOP_BIND);
}

/* *
 * @tc.name: GetRemoteDeviceId_001
 * @tc.desc: Test GetRemoteDeviceId returns accessee deviceId in sink direction
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, GetRemoteDeviceId_001, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->direction = DmAuthDirection::DM_AUTH_SINK;
    authManager->GetAuthContext()->accesser.deviceId = "accesserDeviceId";
    authManager->GetAuthContext()->accessee.deviceId = "accesseeDeviceId";
    std::string deviceId;
    authManager->GetRemoteDeviceId(deviceId);
    EXPECT_EQ(deviceId, "accesserDeviceId");
}

/* *
 * @tc.name: GetRemoteDeviceId_002
 * @tc.desc: Test GetRemoteDeviceId returns accessee deviceId in source direction
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, GetRemoteDeviceId_002, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->direction = DmAuthDirection::DM_AUTH_SOURCE;
    authManager->GetAuthContext()->accesser.deviceId = "accesserDeviceId";
    authManager->GetAuthContext()->accessee.deviceId = "accesseeDeviceId";
    std::string deviceId;
    authManager->GetRemoteDeviceId(deviceId);
    EXPECT_EQ(deviceId, "accesseeDeviceId");
}

/* *
 * @tc.name: RegisterAuthenticationType_001
 * @tc.desc: Test RegisterAuthenticationType stores the authenticationType into confirmOperation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, RegisterAuthenticationType_001, testing::ext::TestSize.Level1)
{
    int32_t authType = USER_OPERATION_TYPE_ALLOW_AUTH;
    int32_t ret = authManager->RegisterAuthenticationType(authType);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(static_cast<int32_t>(authManager->GetAuthContext()->confirmOperation), authType);
}

/* *
 * @tc.name: OnScreenLocked_001
 * @tc.desc: Test OnScreenLocked early-returns when authType is import-auth-code compatible
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, OnScreenLocked_001, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager->OnScreenLocked();
    EXPECT_NE(authManager->GetAuthContext()->reason, ERR_DM_BIND_USER_CANCEL);
}

/* *
 * @tc.name: ParseServiceInfo_001
 * @tc.desc: Test ParseServiceInfo does not set isServiceBind when extra is invalid json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ParseServiceInfo_001, testing::ext::TestSize.Level1)
{
    std::string extra = "invalid_json";
    authManager->GetAuthContext()->isServiceBind = false;
    authManager->ParseServiceInfo(extra);
    EXPECT_FALSE(authManager->GetAuthContext()->isServiceBind);
}

/* *
 * @tc.name: ParseServiceInfo_002
 * @tc.desc: Test ParseServiceInfo sets isServiceBind when isServiceBind is true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ParseServiceInfo_002, testing::ext::TestSize.Level1)
{
    std::string extra = "{\"isServiceBind\":\"true\"}";
    authManager->GetAuthContext()->isServiceBind = false;
    authManager->ParseServiceInfo(extra);
    EXPECT_TRUE(authManager->GetAuthContext()->isServiceBind);
}

/* *
 * @tc.name: GetAuthCodeAndPkgName_001
 * @tc.desc: Test GetAuthCodeAndPkgName does nothing when importAuthCode is empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, GetAuthCodeAndPkgName_001, testing::ext::TestSize.Level1)
{
    authManager->GetAuthContext()->importAuthCode = "";
    authManager->GetAuthContext()->importPkgName = "pkgName";
    std::string pkgName = "origPkg";
    std::string authCode = "origCode";
    authManager->GetAuthCodeAndPkgName(pkgName, authCode);
    EXPECT_EQ(pkgName, "origPkg");
    EXPECT_EQ(authCode, "origCode");
}

/* *
 * @tc.name: ConvertSrcVersion_001
 * @tc.desc: Test ConvertSrcVersion returns edition when version empty and edition non-empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ConvertSrcVersion_001, testing::ext::TestSize.Level1)
{
    std::string result = AuthManagerBase::ConvertSrcVersion("", "5.1.0");
    EXPECT_EQ(result, "5.1.0");
}

/* *
 * @tc.name: ConvertSrcVersion_002
 * @tc.desc: Test ConvertSrcVersion returns default version when both version and edition empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ConvertSrcVersion_002, testing::ext::TestSize.Level1)
{
    std::string result = AuthManagerBase::ConvertSrcVersion("", "");
    EXPECT_EQ(result, std::string(DM_VERSION_5_1_0));
}

/* *
 * @tc.name: ConvertSrcVersion_003
 * @tc.desc: Test ConvertSrcVersion returns version when version non-empty and edition empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ConvertSrcVersion_003, testing::ext::TestSize.Level1)
{
    std::string result = AuthManagerBase::ConvertSrcVersion("3.0.0", "");
    EXPECT_EQ(result, "3.0.0");
}

/* *
 * @tc.name: ConvertSrcVersion_004
 * @tc.desc: Test ConvertSrcVersion returns empty when both version and edition non-empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ConvertSrcVersion_004, testing::ext::TestSize.Level1)
{
    std::string result = AuthManagerBase::ConvertSrcVersion("3.0.0", "5.1.0");
    EXPECT_EQ(result, "");
}

/* *
 * @tc.name: ParseAuthType_001
 * @tc.desc: Test ParseAuthType fails when authType key missing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ParseAuthType_001, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> bindParam;
    int32_t authType = -1;
    int32_t ret = authManager->ParseAuthType(bindParam, authType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/* *
 * @tc.name: ParseAuthType_002
 * @tc.desc: Test ParseAuthType fails when authType value is empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ParseAuthType_002, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> bindParam;
    bindParam[PARAM_KEY_AUTH_TYPE] = "";
    int32_t authType = -1;
    int32_t ret = authManager->ParseAuthType(bindParam, authType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/* *
 * @tc.name: ParseAuthType_003
 * @tc.desc: Test ParseAuthType fails when authType value length greater than 1
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ParseAuthType_003, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> bindParam;
    bindParam[PARAM_KEY_AUTH_TYPE] = "12";
    int32_t authType = -1;
    int32_t ret = authManager->ParseAuthType(bindParam, authType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/* *
 * @tc.name: ParseAuthType_004
 * @tc.desc: Test ParseAuthType fails when authType value is not a digit
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ParseAuthType_004, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> bindParam;
    bindParam[PARAM_KEY_AUTH_TYPE] = "a";
    int32_t authType = -1;
    int32_t ret = authManager->ParseAuthType(bindParam, authType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/* *
 * @tc.name: ParseAuthType_005
 * @tc.desc: Test ParseAuthType succeeds and sets authType for a single digit value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, ParseAuthType_005, testing::ext::TestSize.Level1)
{
    std::map<std::string, std::string> bindParam;
    bindParam[PARAM_KEY_AUTH_TYPE] = "1";
    int32_t authType = -1;
    int32_t ret = authManager->ParseAuthType(bindParam, authType);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(authType, 1);
}

/* *
 * @tc.name: CheckProcessNameInWhiteList_001
 * @tc.desc: Test CheckProcessNameInWhiteList returns false when processName is empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, CheckProcessNameInWhiteList_001, testing::ext::TestSize.Level1)
{
    bool ret = AuthManagerBase::CheckProcessNameInWhiteList("");
    EXPECT_FALSE(ret);
}

/* *
 * @tc.name: CheckProcessNameInWhiteList_002
 * @tc.desc: Test CheckProcessNameInWhiteList returns false when processName not in whitelist
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, CheckProcessNameInWhiteList_002, testing::ext::TestSize.Level1)
{
    bool ret = AuthManagerBase::CheckProcessNameInWhiteList("com.not.in.whitelist");
    EXPECT_FALSE(ret);
}

/* *
 * @tc.name: CheckProcessNameInProxyAdaptationList_001
 * @tc.desc: Test CheckProcessNameInProxyAdaptationList returns false when processName is empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, CheckProcessNameInProxyAdaptationList_001, testing::ext::TestSize.Level1)
{
    bool ret = AuthManagerBase::CheckProcessNameInProxyAdaptationList("");
    EXPECT_FALSE(ret);
}

/* *
 * @tc.name: CheckProcessNameInProxyAdaptationList_002
 * @tc.desc: Test CheckProcessNameInProxyAdaptationList returns true for gameservice_server
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, CheckProcessNameInProxyAdaptationList_002, testing::ext::TestSize.Level1)
{
    bool ret = AuthManagerBase::CheckProcessNameInProxyAdaptationList("gameservice_server");
    EXPECT_TRUE(ret);
}

/* *
 * @tc.name: CheckProcessNameInProxyAdaptationList_003
 * @tc.desc: Test CheckProcessNameInProxyAdaptationList returns false when processName not in list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, CheckProcessNameInProxyAdaptationList_003, testing::ext::TestSize.Level1)
{
    bool ret = AuthManagerBase::CheckProcessNameInProxyAdaptationList("com.unknown.process");
    EXPECT_FALSE(ret);
}

/* *
 * @tc.name: InsensibleSwitchingState_001
 * @tc.desc: Test Enable/Disable/NeedInsensibleSwitching state transitions
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, InsensibleSwitchingState_001, testing::ext::TestSize.Level1)
{
    authManager->EnableInsensibleSwitching();
    EXPECT_TRUE(authManager->NeedInsensibleSwitching());
    authManager->DisableInsensibleSwitching();
    EXPECT_FALSE(authManager->NeedInsensibleSwitching());
}

/* *
 * @tc.name: TransferReadyState_001
 * @tc.desc: Test SetTransferReady/IsTransferReady state transitions
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, TransferReadyState_001, testing::ext::TestSize.Level1)
{
    authManager->SetTransferReady(false);
    EXPECT_FALSE(authManager->IsTransferReady());
    authManager->SetTransferReady(true);
    EXPECT_TRUE(authManager->IsTransferReady());
}
} // namespace DistributedHardware
} // namespace OHOS
