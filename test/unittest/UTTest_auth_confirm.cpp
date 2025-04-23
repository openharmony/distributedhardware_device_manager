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

constexpr const char *TEST_DEVICE_ID = "deviceId";
constexpr const int32_t TEST_USER_ID = 0;
constexpr const int64_t TEST_TOKEN_ID = 0;
constexpr const char *TEST_CREDENTIAL_ID = "credentialId";

std::shared_ptr<DeviceProfileConnectorMock> AuthConfirmTest::deviceProfileConnectorMock = nullptr;

void AuthConfirmTest::SetUpTestCase()
{
    deviceProfileConnectorMock = std::make_shared<DeviceProfileConnectorMock>();
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock;
}

void AuthConfirmTest::TearDownTestCase()
{
    deviceProfileConnectorMock = nullptr;
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
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
}

bool DmAuthState::IsScreenLocked()
{
    return false;
}

// AuthSrcConfirmState
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
    EXPECT_EQ(authState->Action(context), ERR_DM_VERSION_INCOMPATIBLE);
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
    std::string jsonStr = R"(
        {
            "identicalCredType": true,
            "shareCredType": true,
            "pointTopointCredType": true,
            "lnnCredType": true
        }
    )";
    context = authManager->GetAuthContext();
    context->accessee.credTypeList = jsonStr;
    context->accesser.credTypeList = jsonStr;
    JsonObject jsonObject;

    authState->NegotiateCredential(context, jsonObject);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_NegotiateAcl_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    std::string jsonStr = R"(
        {
            "identicalAcl": true,
            "shareAcl": true,
            "pointTopointAcl": true,
            "lnnAcl": true
        }
    )";
    context = authManager->GetAuthContext();
    context->accessee.aclTypeList = jsonStr;
    context->accesser.aclTypeList = jsonStr;
    JsonObject jsonObject;

    authState->NegotiateCredential(context, jsonObject);
}

// get identical credential
HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcCredentialInfo_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    context->accesser.accountIdHash = context->accessee.accountIdHash = "";
    JsonObject jsonObject;
    authState->GetSrcCredentialInfo(context, jsonObject);
}

// get share credential
HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcCredentialInfo_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    context->accesser.accountIdHash = "0";
    context->accessee.accountIdHash = "1";
    JsonObject jsonObject;
    authState->GetSrcCredentialInfo(context, jsonObject);
}

// get point_to_point credential
HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcCredentialInfo_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    context->accesser.accountIdHash = Crypto::Sha256("ohosAnonymousUid");
    context->accessee.accountIdHash = Crypto::Sha256("ohosAnonymousUid");
    JsonObject jsonObject;
    authState->GetSrcCredentialInfo(context, jsonObject);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcAclInfo_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();

    context = authManager->GetAuthContext();
    context->accesser.deviceId = TEST_DEVICE_ID;
    context->accessee.deviceId = TEST_DEVICE_ID;
    context->accesser.deviceIdHash = Crypto::Sha256(context->accesser.deviceId);
    context->accessee.deviceIdHash = Crypto::Sha256(context->accessee.deviceId);

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
    JsonObject credInfo;
    JsonObject aclInf;
    authState->GetSrcAclInfo(context, credInfo, aclInf);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcAclInfoForP2P_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    context->accesser.deviceId = TEST_DEVICE_ID;
    context->accesser.userId = TEST_USER_ID;
    context->accesser.tokenId = TEST_TOKEN_ID;
    context->accessee.deviceId = TEST_DEVICE_ID;
    context->accessee.userId = TEST_USER_ID;
    context->accessee.tokenId = TEST_TOKEN_ID;
    context->accessee.deviceIdHash = Crypto::Sha256(context->accessee.deviceId);
    context->accessee.tokenIdHash = Crypto::Sha256(std::to_string(context->accessee.tokenId));

    DistributedDeviceProfile::AccessControlProfile profile;
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    accesser.SetAccesserTokenId(TEST_TOKEN_ID);
    accesser.SetAccesserCredentialIdStr(TEST_CREDENTIAL_ID);
    profile.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accesee;
    accesee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    accesee.SetAccesseeUserId(TEST_USER_ID);
    accesee.SetAccesseeTokenId(TEST_TOKEN_ID);
    accesee.SetAccesseeCredentialIdStr(TEST_CREDENTIAL_ID);
    profile.SetAccessee(accesee);

    std::string jsonStr = R"(
        {
            "credentialId": {
                "credType": 256,
                "authorizedAppList": [0, 0]
            }
        }
    )";
    JsonObject credInfo(jsonStr);
    JsonObject aclInf;

    authState->GetSrcAclInfoForP2P(context, profile, credInfo, aclInf);
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_IdenticalAccountAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();
    context->accesser.deviceId = TEST_DEVICE_ID;
    context->accesser.userId = TEST_USER_ID;
    context->accessee.deviceId = TEST_DEVICE_ID;
    context->accessee.deviceIdHash = Crypto::Sha256(context->accessee.deviceId);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    EXPECT_TRUE(authState->IdenticalAccountAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_ShareAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();

    context->accesser.deviceId = TEST_DEVICE_ID;
    context->accesser.userId = TEST_USER_ID;
    context->accessee.deviceId = TEST_DEVICE_ID;
    context->accessee.deviceIdHash = Crypto::Sha256(context->accessee.deviceId);

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);

    EXPECT_TRUE(authState->ShareAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_Point2PointAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();

    context->accesser.deviceId = TEST_DEVICE_ID;
    context->accesser.userId = TEST_USER_ID;
    context->accesser.tokenId = TEST_TOKEN_ID;

    context->accessee.deviceId = TEST_DEVICE_ID;
    context->accessee.tokenId = TEST_TOKEN_ID;
    context->accessee.deviceIdHash = Crypto::Sha256(context->accessee.deviceId);
    context->accessee.tokenIdHash = Crypto::Sha256(std::to_string(context->accessee.tokenId));

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    accesser.SetAccesserTokenId(TEST_TOKEN_ID);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    accessee.SetAccesseeUserId(TEST_USER_ID);
    accessee.SetAccesseeTokenId(TEST_TOKEN_ID);

    EXPECT_TRUE(authState->Point2PointAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_LnnAclCompare_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();

    context->accesser.deviceId = TEST_DEVICE_ID;
    context->accesser.userId = TEST_USER_ID;
    context->accesser.tokenId = TEST_TOKEN_ID;

    context->accessee.deviceId = TEST_DEVICE_ID;
    context->accessee.tokenId = TEST_TOKEN_ID;
    context->accessee.deviceIdHash = Crypto::Sha256(context->accessee.deviceId);
    context->accessee.tokenIdHash = Crypto::Sha256(std::to_string(context->accessee.tokenId));

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    accesser.SetAccesserTokenId(TEST_TOKEN_ID);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    accessee.SetAccesseeUserId(TEST_USER_ID);
    accessee.SetAccesseeTokenId(TEST_TOKEN_ID);

    EXPECT_TRUE(authState->LnnAclCompare(context, accesser, accessee));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_CheckCredIdInAcl_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();

    DistributedDeviceProfile::AccessControlProfile profile;
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserDeviceId(TEST_DEVICE_ID);
    accesser.SetAccesserUserId(TEST_USER_ID);
    accesser.SetAccesserTokenId(TEST_TOKEN_ID);
    accesser.SetAccesserCredentialIdStr(TEST_CREDENTIAL_ID);
    profile.SetAccesser(accesser);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeDeviceId(TEST_DEVICE_ID);
    accessee.SetAccesseeUserId(TEST_USER_ID);
    accessee.SetAccesseeTokenId(TEST_TOKEN_ID);
    accessee.SetAccesseeCredentialIdStr(TEST_CREDENTIAL_ID);
    profile.SetAccessee(accessee);

    std::string jsonStr = R"(
        {
            "credentialId": {
                "credType": 3,
                "authorizedAppList": [0, 0]
            }
        }
    )";
    JsonObject credInfo(jsonStr);

    uint32_t bindType = 3;

    EXPECT_TRUE(authState->CheckCredIdInAcl(context, profile, credInfo, bindType));
}

HWTEST_F(AuthConfirmTest, AuthSrcConfirmState_GetSrcCredType_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    std::shared_ptr<AuthSrcConfirmState> authState = std::make_shared<AuthSrcConfirmState>();
    context = authManager->GetAuthContext();

    std::string jsonStr = R"(
        [
            {
                "credType": 1,
                "credId": "credId1"
            },
            {
                "credType": 2,
                "credId": "credId2"
            },
            {
                "credType": "invalid",
                "credId": "credId3"
            },
            {
                "credType": 4,
                "credId": 12345
            }
        ]
    )";
    JsonObject credInfo(jsonStr);
    JsonObject aclInfo;
    JsonObject credTypeJson;
    authState->GetSrcCredType(context, credInfo, aclInfo, credTypeJson);
}

// AuthSinkConfirmState
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
    EXPECT_EQ(authState->ShowConfigDialog(context), DM_OK);
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

    authState->MatchFallBackCandidateList(context, AUTH_TYPE_PIN);
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

}  // end namespace DistributedHardware
}  // end namespace OHOS