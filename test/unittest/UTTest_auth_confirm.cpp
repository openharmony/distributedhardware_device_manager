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

#include "device_manager_service_listener.h"
#include "dm_auth_state.h"
#include "dm_crypto.h"
#include "UTTest_auth_confirm.h"

namespace OHOS {
namespace DistributedHardware {

using namespace testing;

namespace {
constexpr const char *TEST_DEVICE_ID = "deviceId";
constexpr const int32_t TEST_USER_ID = 0;
constexpr const int64_t TEST_TOKEN_ID = 0;
constexpr const char *TEST_CREDENTIAL_ID = "credentialId";
constexpr const char *TEST_IDENTIAL_CRED_ID = "identialCredId";
constexpr const char *TEST_SHARE_CRED_ID = "shareCredId";
constexpr const char *TEST_POINT_TO_POINT_CRED_ID = "p2pCredId";
constexpr const char *TEST_LNN_CRED_ID = "lnnCredId";

DistributedDeviceProfile::AccessControlProfile TestCreateAcl(const std::string credIdStr, int32_t bindType)
{
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    accesser.SetAccesserTokenId(TEST_TOKEN_ID);
    accesser.SetAccesserCredentialIdStr(credIdStr);

    DistributedDeviceProfile::Accessee accesee;
    accesee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    accesee.SetAccesseeUserId(TEST_USER_ID);
    accesee.SetAccesseeTokenId(TEST_TOKEN_ID);
    accesee.SetAccesseeCredentialIdStr(credIdStr);

    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetAccesser(accesser);
    profile.SetAccessee(accesee);
    profile.SetTrustDeviceId(TEST_DEVICE_ID);
    profile.SetBindType(bindType);
    return profile;
}

void TestSetContext(std::shared_ptr<DmAuthContext> context)
{
    context->accesser.deviceId = TEST_DEVICE_ID;
    context->accesser.userId = TEST_USER_ID;
    context->accesser.tokenId = TEST_TOKEN_ID;
    context->accesser.deviceIdHash = Crypto::Sha256(context->accesser.deviceId);
    context->accesser.tokenIdHash = Crypto::Sha256(std::to_string(context->accesser.tokenId));
    context->accessee.deviceId = TEST_DEVICE_ID;
    context->accessee.userId = TEST_USER_ID;
    context->accessee.tokenId = TEST_TOKEN_ID;
    context->accessee.deviceIdHash = Crypto::Sha256(context->accessee.deviceId);
    context->accessee.tokenIdHash = Crypto::Sha256(std::to_string(context->accessee.tokenId));
}
}

std::shared_ptr<DeviceProfileConnectorMock> AuthConfirmTest::deviceProfileConnectorMock = nullptr;
std::shared_ptr<HiChainAuthConnectorMock> AuthConfirmTest::dmHiChainAuthConnectorMock = nullptr;

void AuthConfirmTest::SetUpTestCase()
{
    deviceProfileConnectorMock = std::make_shared<DeviceProfileConnectorMock>();
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock;
    dmHiChainAuthConnectorMock = std::make_shared<HiChainAuthConnectorMock>();
    DmHiChainAuthConnector::dmHiChainAuthConnector = dmHiChainAuthConnectorMock;
}

void AuthConfirmTest::TearDownTestCase()
{
    deviceProfileConnectorMock = nullptr;
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    dmHiChainAuthConnectorMock = nullptr;
    DmHiChainAuthConnector::dmHiChainAuthConnector = nullptr;
}

void AuthConfirmTest::SetUp()
{
    LOGI("start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
}

void AuthConfirmTest::TearDown()
{
    LOGI("start.");
    softbusConnector = nullptr;
    listener = nullptr;
    hiChainAuthConnector = nullptr;
    hiChainConnector = nullptr;
    authManager = nullptr;
    context = nullptr;
    Mock::VerifyAndClearExpectations(deviceProfileConnectorMock.get());
    Mock::VerifyAndClearExpectations(dmHiChainAuthConnectorMock.get());
}

bool DmAuthState::IsScreenLocked()
{
    return false;
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcCredentialInfo_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    JsonObject jsonObject;

    context->accesser.accountIdHash = context->accessee.accountIdHash = "";
    authState->GetSrcCredentialInfo(context, jsonObject);

    context->accesser.accountIdHash = "0";
    context->accessee.accountIdHash = "1";
    EXPECT_CALL(*dmHiChainAuthConnectorMock, QueryCredentialInfo(_, _, _))
        .WillOnce(Return(DM_OK))
        .WillOnce(Return(DM_OK));
    authState->GetSrcCredentialInfo(context, jsonObject);

    context->accesser.accountIdHash = Crypto::Sha256("ohosAnonymousUid");
    EXPECT_CALL(*dmHiChainAuthConnectorMock, QueryCredentialInfo(_, _, _))
        .WillOnce(Return(DM_OK))
        .WillOnce(Return(DM_OK));
    authState->GetSrcCredentialInfo(context, jsonObject);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_IdenticalAccountAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    TestSetContext(context);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    EXPECT_FALSE(authState->IdenticalAccountAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_ShareAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    TestSetContext(context);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);

    EXPECT_FALSE(authState->ShareAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_Point2PointAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    TestSetContext(context);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    accesser.SetAccesserTokenId(TEST_TOKEN_ID);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    accessee.SetAccesseeUserId(TEST_USER_ID);
    accessee.SetAccesseeTokenId(TEST_TOKEN_ID);

    EXPECT_FALSE(authState->Point2PointAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_LnnAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    TestSetContext(context);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    accesser.SetAccesserTokenId(TEST_TOKEN_ID);
    accesser.SetAccesserBundleName("");

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    accessee.SetAccesseeUserId(TEST_USER_ID);
    accessee.SetAccesseeTokenId(TEST_TOKEN_ID);
    accessee.SetAccesseeBundleName("");

    EXPECT_FALSE(authState->LnnAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_CheckCredIdInAcl_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();

    DistributedDeviceProfile::AccessControlProfile profile = TestCreateAcl(TEST_CREDENTIAL_ID, DM_LNN);

    std::string jsonStr = R"({
        "credentialId": {
            "credType": 3,
            "authorizedAppList": [0, 0]
        }
    })";
    JsonObject credInfo(jsonStr);

    EXPECT_TRUE(authState->CheckCredIdInAcl(context, profile, credInfo, DM_LNN));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcCredType_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();

    std::string jsonCredStr = R"([
        {"credType": 1, "credId": "0"},
        {"credType": 2, "credId": "0"}
    ])";
    JsonObject credInfo(jsonCredStr);

    std::string aclJsonStr = R"({"lnnAcl":3,"pointTopointAcl":256})";
    JsonObject aclInfo(aclJsonStr);

    std::string credTypeJsonStr = R"({"identicalCredType":1,"shareCredType":2})";
    JsonObject credTypeJson;
    authState->GetSrcCredType(context, credInfo, aclInfo, credTypeJson);
    EXPECT_TRUE(credTypeJson.Dump() == credTypeJsonStr);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_CONFIRM_STATE);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_Action_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();
    EXPECT_EQ(authState->Action(context), ERR_DM_FAILED);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ShowConfigDialog_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    EXPECT_EQ(authState->ShowConfigDialog(context), STOP_BIND);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ReadServiceInfo_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();

    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->serviceInfoFound = true;

    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetAuthBoxType(static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM));

    srvInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ONETIME));
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->ReadServiceInfo(context);

    srvInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::CANCEL));
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->ReadServiceInfo(context);

    srvInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ALWAYS));
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->ReadServiceInfo(context);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_MatchFallBackCandidateList_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();

    authState->MatchFallBackCandidateList(context, DmAuthType::AUTH_TYPE_NFC);
    EXPECT_TRUE(context->authTypeList.empty());

    context->accessee.bundleName = "cast_engine_service";
    authState->MatchFallBackCandidateList(context, DmAuthType::AUTH_TYPE_NFC);
    EXPECT_EQ(context->authTypeList[0], DmAuthType::AUTH_TYPE_PIN);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ReadServiceInfo_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();

    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetAuthBoxType(static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM));
    srvInfo.SetExtraInfo("invalid_extra_info");

    srvInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ONETIME));
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->ReadServiceInfo(context);
    auto ret = context->confirmOperation == UiAction::USER_OPERATION_TYPE_CANCEL_AUTH ? true : false;
    EXPECT_FALSE(ret);

    srvInfo.SetExtraInfo(R"({"pinConsumerTokenId":123456})");
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->ReadServiceInfo(context);
    ret = context->confirmOperation == UiAction::USER_OPERATION_TYPE_CANCEL_AUTH ? true : false;
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ExtractPinConsumerTokenId_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    uint64_t tokenId = 123;
    std::string json = R"({"otherField":123})";
    bool ret = authState->ExtractPinConsumerTokenId(json, tokenId);
    EXPECT_FALSE(ret);
    EXPECT_EQ(tokenId, 0);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ExtractPinConsumerTokenId_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    uint64_t tokenId = 0;
    std::string json = R"({"pinConsumerTokenId":987654321})";
    bool ret = authState->ExtractPinConsumerTokenId(json, tokenId);
    EXPECT_TRUE(ret);
    EXPECT_EQ(tokenId, 987654321);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ReadServiceInfo_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->accessee.pkgName = "watch_system_service";
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetBundleName("com.huawei.hmos.wearlink");
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(
        testing::StrEq("com.huawei.hmos.wearlink"), _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->ReadServiceInfo(context);
    EXPECT_EQ(srvInfo.GetBundleName(), "com.huawei.hmos.wearlink");
}

/**
 * @tc.name: DmAuthState_GetOutputState_001
 * @tc.desc: GetOutputState maps AUTH_SRC_FINISH_STATE to STATUS_DM_AUTH_FINISH.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetOutputState_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    int32_t state = static_cast<int32_t>(DmAuthStateType::AUTH_SRC_FINISH_STATE);
    EXPECT_EQ(authState->GetOutputState(state),
        static_cast<int32_t>(DmAuthStatus::STATUS_DM_AUTH_FINISH));
}

/**
 * @tc.name: DmAuthState_GetOutputState_002
 * @tc.desc: GetOutputState returns STATUS_DM_AUTH_DEFAULT for an unmapped state value.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetOutputState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_EQ(authState->GetOutputState(9999), static_cast<int32_t>(DmAuthStatus::STATUS_DM_AUTH_DEFAULT));
}

/**
 * @tc.name: DmAuthState_GetOutputReplay_001
 * @tc.desc: GetOutputReplay maps DM_BIND_TRUST_TARGET to DM_OK for whitelisted bundle name.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetOutputReplay_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_EQ(authState->GetOutputReplay("CollaborationFwk", DM_BIND_TRUST_TARGET), DM_OK);
}

/**
 * @tc.name: DmAuthState_GetOutputReplay_002
 * @tc.desc: GetOutputReplay returns original replay when bundle name is not whitelisted.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetOutputReplay_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    int32_t replay = 12345;
    EXPECT_EQ(authState->GetOutputReplay("other_bundle", replay), replay);
}

/**
 * @tc.name: DmAuthState_GetOutputReplay_003
 * @tc.desc: GetOutputReplay returns original replay for whitelisted name but unmapped value.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetOutputReplay_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    int32_t replay = 999;
    EXPECT_EQ(authState->GetOutputReplay("cast_engine_service", replay), replay);
}

/**
 * @tc.name: DmAuthState_GetTaskTimeout_001
 * @tc.desc: GetTaskTimeout returns mapped clone timeout for import auth code and known task.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetTaskTimeout_001, testing::ext::TestSize.Level1)
{
    context = std::make_shared<DmAuthContext>();
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    int32_t defaultTimeout = 1000;
    int32_t ret = DmAuthState::GetTaskTimeout(context, AUTHENTICATE_TIMEOUT_TASK, defaultTimeout);
    EXPECT_EQ(ret, CLONE_AUTHENTICATE_TIMEOUT);
}

/**
 * @tc.name: DmAuthState_GetTaskTimeout_002
 * @tc.desc: GetTaskTimeout returns default timeout when task name is not in the map.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetTaskTimeout_002, testing::ext::TestSize.Level1)
{
    context = std::make_shared<DmAuthContext>();
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    int32_t defaultTimeout = 5000;
    int32_t ret = DmAuthState::GetTaskTimeout(context, "unknown_task_name", defaultTimeout);
    EXPECT_EQ(ret, defaultTimeout);
}

/**
 * @tc.name: DmAuthState_GetTaskTimeout_003
 * @tc.desc: GetTaskTimeout returns default timeout for non-import auth type.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetTaskTimeout_003, testing::ext::TestSize.Level1)
{
    context = std::make_shared<DmAuthContext>();
    context->authType = DmAuthType::AUTH_TYPE_PIN;
    int32_t defaultTimeout = 3000;
    int32_t ret = DmAuthState::GetTaskTimeout(context, AUTHENTICATE_TIMEOUT_TASK, defaultTimeout);
    EXPECT_EQ(ret, defaultTimeout);
}

/**
 * @tc.name: DmAuthState_IsImportAuthCodeCompatibility_001
 * @tc.desc: IsImportAuthCodeCompatibility returns true for PIN and NFC auth types.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_IsImportAuthCodeCompatibility_001, testing::ext::TestSize.Level1)
{
    EXPECT_TRUE(DmAuthState::IsImportAuthCodeCompatibility(DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE));
    EXPECT_TRUE(DmAuthState::IsImportAuthCodeCompatibility(DmAuthType::AUTH_TYPE_NFC));
    EXPECT_FALSE(DmAuthState::IsImportAuthCodeCompatibility(DmAuthType::AUTH_TYPE_PIN));
}

/**
 * @tc.name: DmAuthState_GetSysTimeMs_001
 * @tc.desc: GetSysTimeMs returns a non-zero positive millisecond timestamp.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GetSysTimeMs_001, testing::ext::TestSize.Level1)
{
    uint64_t t1 = DmAuthState::GetSysTimeMs();
    EXPECT_GT(t1, 0ULL);
    uint64_t t2 = DmAuthState::GetSysTimeMs();
    EXPECT_GE(t2, t1);
}

/**
 * @tc.name: DmAuthState_IsInFlagWhiteList_001
 * @tc.desc: IsInFlagWhiteList returns false for empty bundle name.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_IsInFlagWhiteList_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_FALSE(authState->IsInFlagWhiteList(""));
}

/**
 * @tc.name: DmAuthState_IsInFlagWhiteList_002
 * @tc.desc: IsInFlagWhiteList returns true for whitelisted bundle, false for unknown.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_IsInFlagWhiteList_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_TRUE(authState->IsInFlagWhiteList("wear_link_service"));
    EXPECT_FALSE(authState->IsInFlagWhiteList("unknown_bundle"));
}

/**
 * @tc.name: DmAuthState_IsNeedBind_001
 * @tc.desc: IsNeedBind returns true when context is nullptr.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_IsNeedBind_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_TRUE(authState->IsNeedBind(nullptr));
}

/**
 * @tc.name: DmAuthState_IsNeedAgreeCredential_001
 * @tc.desc: IsNeedAgreeCredential returns true when context is nullptr.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_IsNeedAgreeCredential_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_TRUE(authState->IsNeedAgreeCredential(nullptr));
}

/**
 * @tc.name: DmAuthState_HaveSameTokenId_001
 * @tc.desc: HaveSameTokenId returns false when tokenIdHashList size is less than 2.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_HaveSameTokenId_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    std::vector<std::string> tokenList = {"hashA"};
    EXPECT_FALSE(authState->HaveSameTokenId(context, tokenList));
}

/**
 * @tc.name: DmAuthState_HaveSameTokenId_002
 * @tc.desc: HaveSameTokenId returns true when both accesser and accessee hashes are in the list.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_HaveSameTokenId_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    context->accesser.tokenIdHash = "accHash";
    context->accessee.tokenIdHash = "aceHash";
    std::vector<std::string> tokenList = {"accHash", "aceHash"};
    EXPECT_TRUE(authState->HaveSameTokenId(context, tokenList));
}

/**
 * @tc.name: DmAuthState_IsAclHasCredential_001
 * @tc.desc: IsAclHasCredential returns true when accesser cred id is in cred info json.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_IsAclHasCredential_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    DistributedDeviceProfile::AccessControlProfile profile = TestCreateAcl(TEST_CREDENTIAL_ID, DM_LNN);
    std::string credInfoJson = R"({"credentialId":{"credType":3}})";
    std::string credId;
    EXPECT_TRUE(authState->IsAclHasCredential(profile, credInfoJson, credId));
    EXPECT_EQ(credId, TEST_CREDENTIAL_ID);
}

/**
 * @tc.name: DmAuthState_IsAclHasCredential_002
 * @tc.desc: IsAclHasCredential returns false when neither cred id is in cred info json.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_IsAclHasCredential_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    DistributedDeviceProfile::AccessControlProfile profile = TestCreateAcl(TEST_CREDENTIAL_ID, DM_LNN);
    std::string credInfoJson = R"({"otherCredId":{"credType":3}})";
    std::string credId;
    EXPECT_FALSE(authState->IsAclHasCredential(profile, credInfoJson, credId));
}

/**
 * @tc.name: DmAuthState_GenerateBindResultContent_001
 * @tc.desc: GenerateBindResultContent returns non-empty json content including empty deviceId branch.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthConfirmTest, DmAuthState_GenerateBindResultContent_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    context->direction = DM_AUTH_SOURCE;
    context->accessee.deviceId = "";
    context->accessee.networkId = "network123";
    std::string content = authState->GenerateBindResultContent(context);
    EXPECT_FALSE(content.empty());
    EXPECT_NE(content.find("network123"), std::string::npos);
}
HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_CONFIRM_STATE);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_NegotiateCredential_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    std::string jsonStr = R"({"identicalCredType":1,"shareCredType":2,"pointTopointCredType":256,"lnnCredType":3})";
    context = authManager->GetAuthContext();
    context->accessee.credTypeList = jsonStr;
    context->accesser.credTypeList = jsonStr;
    JsonObject jsonObject;
    authState->NegotiateCredential(context, jsonObject);
    EXPECT_TRUE(jsonObject["identicalCredType"].Get<int32_t>() == 1);
    EXPECT_TRUE(jsonObject["shareCredType"].Get<int32_t>() == 2);
    EXPECT_TRUE(jsonObject["pointTopointCredType"].Get<int32_t>() == 256);
    EXPECT_TRUE(jsonObject["lnnCredType"].Get<int32_t>() == 3);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_NegotiateAcl_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    std::string jsonStr = R"({"identicalAcl":1,"shareAcl":2,"pointTopointAcl":256,"lnnAcl":3})";
    context = authManager->GetAuthContext();
    context->accessee.aclTypeList = jsonStr;
    context->accesser.aclTypeList = jsonStr;
    JsonObject jsonObject;
    authState->NegotiateAcl(context, jsonObject);
    EXPECT_TRUE(jsonObject["identicalAcl"].Get<int32_t>() == 1);
    EXPECT_TRUE(jsonObject["shareAcl"].Get<int32_t>() == 2);
    EXPECT_TRUE(jsonObject["pointTopointAcl"].Get<int32_t>() == 256);
    EXPECT_TRUE(jsonObject["lnnAcl"].Get<int32_t>() == 3);
}

HWTEST_F(AuthConfirmTest, ProcessImportAuthInfo_AclLifeCycleDays_ParsedWhenPresent, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->accessee.tokenId = 123;
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetAuthBoxType(static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM));
    srvInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ALWAYS));
    srvInfo.SetExtraInfo(R"({"ACL_LIFE_CYCLE_DAYS":730})");

    auto state = std::make_shared<AuthSinkConfirmState>();
    state->ProcessImportAuthInfo(context, srvInfo);

    EXPECT_EQ(context->aclLifeCycleDays, 730);
    EXPECT_EQ(context->confirmOperation, UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS);
}

HWTEST_F(AuthConfirmTest, ProcessImportAuthInfo_AclLifeCycleDays_SentinelKeepsDefault, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetAuthBoxType(static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM));
    srvInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ALWAYS));
    srvInfo.SetExtraInfo(R"({"ACL_LIFE_CYCLE_DAYS":-1})");

    auto state = std::make_shared<AuthSinkConfirmState>();
    state->ProcessImportAuthInfo(context, srvInfo);

    EXPECT_EQ(context->aclLifeCycleDays, ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED);
}

HWTEST_F(AuthConfirmTest, ProcessImportAuthInfo_AclLifeCycleDays_MissingKeepsDefault, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->aclLifeCycleDays = 999;
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({})");

    auto state = std::make_shared<AuthSinkConfirmState>();
    state->ProcessImportAuthInfo(context, srvInfo);

    EXPECT_EQ(context->aclLifeCycleDays, ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED);
}

HWTEST_F(AuthConfirmTest, ProcessImportAuthInfo_AclLifeCycleDays_MalformedKeepsDefault, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->aclLifeCycleDays = 999;
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo("not-json");

    auto state = std::make_shared<AuthSinkConfirmState>();
    state->ProcessImportAuthInfo(context, srvInfo);

    EXPECT_EQ(context->aclLifeCycleDays, ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED);
}

HWTEST_F(AuthConfirmTest, ProcessImportAuthInfo_AclLifeCycleDays_StringValueKeepsDefault,
    testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"ACL_LIFE_CYCLE_DAYS":"730"})");

    auto state = std::make_shared<AuthSinkConfirmState>();
    state->ProcessImportAuthInfo(context, srvInfo);

    EXPECT_EQ(context->aclLifeCycleDays, ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED);
}

HWTEST_F(AuthConfirmTest, ProcessImportAuthInfo_AclLifeCycleDays_NegativeNonSentinelKeepsDefault,
    testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->aclLifeCycleDays = 999;
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"ACL_LIFE_CYCLE_DAYS":-2})");

    auto state = std::make_shared<AuthSinkConfirmState>();
    state->ProcessImportAuthInfo(context, srvInfo);

    EXPECT_EQ(context->aclLifeCycleDays, ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED);
}
}  // end namespace DistributedHardware
}  // end namespace OHOS
