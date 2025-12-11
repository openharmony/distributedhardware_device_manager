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
    LOGI("AuthConfirmTest::SetUp start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
}

void AuthConfirmTest::TearDown()
{
    LOGI("AuthConfirmTest::TearDown start.");
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

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetStateType_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_CONFIRM_STATE);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_Action_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context->accessee.dmVersion = "6.0.0";
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_Action_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context->accessee.dmVersion = DM_VERSION_5_1_0;
    std::vector<DistributedDeviceProfile::AccessControlProfile> allProfiles;
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId(context->accesser.deviceId);
    profile.SetBindType(0);
    allProfiles.push_back(profile);
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    allProfiles.push_back(profile);
    profile.SetBindType(DM_SHARE);
    allProfiles.push_back(profile);
    profile.SetBindType(DM_POINT_TO_POINT);
    allProfiles.push_back(profile);
    EXPECT_CALL(*deviceProfileConnectorMock, GetAllAclIncludeLnnAcl()).WillOnce(Return(allProfiles));
    EXPECT_EQ(authState->Action(context), DM_OK);
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

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ProcessBindAuthorize_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();

    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->importAuthCode = "123456";
    context->importPkgName = "pkgName";
    context->pkgName = "pkgName";

    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetAuthBoxType(static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM));

    srvInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ONETIME));
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .Times(::testing::AtLeast(1))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));

    EXPECT_EQ(authState->ProcessBindAuthorize(context), DM_OK);
}

HWTEST_F(AuthConfirmTest, AuthSinkConfirmState_ProcessNoBindAuthorize_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSinkConfirmState> authState = std::make_shared<AuthSinkConfirmState>();
    context = authManager->GetAuthContext();

    EXPECT_EQ(authState->ProcessNoBindAuthorize(context), ERR_DM_FAILED);

    context->accessee.credTypeList = R"({"identicalCredType": true})";
    EXPECT_EQ(authState->ProcessNoBindAuthorize(context), DM_OK);

    context->accessee.credTypeList = R"({"shareCredType": true})";
    EXPECT_EQ(authState->ProcessNoBindAuthorize(context), DM_OK);

    context->accessee.credTypeList = R"({"pointTopointCredType": true})";
    EXPECT_EQ(authState->ProcessNoBindAuthorize(context), DM_OK);

    context->accessee.credTypeList = R"({"lnnCredType": true})";
    EXPECT_EQ(authState->ProcessNoBindAuthorize(context), DM_OK);
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
}  // end namespace DistributedHardware
}  // end namespace OHOS
