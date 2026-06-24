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
#include "dm_crypto.h"
#include "dm_auth_context.h"
#include "dm_auth_state.h"
#include "dm_negotiate_process.h"
#include "UTTest_auth_negotiate.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

constexpr const char* TEST_STRING = "test_string";
constexpr int32_t TEST_NEGATIVE = -1;
constexpr int32_t TEST_POSITIVE = 1;
void AuthNegotiateTest::SetUpTestCase()
{
    LOGI("start.");

    DmSoftbusConnector::dmSoftbusConnector = softbusConnectorMock;
    DmSoftbusSession::dmSoftbusSession = softbusSessionMock;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    DmAppManager::dmAppManager = appManagerMock_;
}

void AuthNegotiateTest::TearDownTestCase()
{
    LOGI("done.");
    softbusConnectorMock = nullptr;
    softbusSessionMock = nullptr;
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    DmSoftbusSession::dmSoftbusSession = nullptr;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
    DmAppManager::dmAppManager = nullptr;
    appManagerMock_ = nullptr;
}

void AuthNegotiateTest::SetUp()
{
    LOGI("start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    softbusSession = std::make_shared<SoftbusSession>();

    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    tokenMock = std::static_pointer_cast<AccessTokenKitMock>(token);

    Mock::VerifyAndClearExpectations(&*softbusConnectorMock);
    Mock::VerifyAndClearExpectations(&*softbusSessionMock);
    Mock::VerifyAndClearExpectations(&*distributedDeviceProfileClientMock_);
}

void AuthNegotiateTest::TearDown()
{
    LOGI("done.");
    softbusConnector = nullptr;
    listener = nullptr;
    hiChainAuthConnector = nullptr;
    authManager = nullptr;
    context = nullptr;
    softbusSession = nullptr;

    Mock::VerifyAndClearExpectations(&*tokenMock);
    tokenMock = nullptr;
}

int32_t GetBusinessEventMockTrue(DistributedDeviceProfile::BusinessEvent &event)
{
    event.SetBusinessValue("{\"business_id\":\"test_business_id\",\"is_in_anti_disturbance_mode\":true}");
    return DM_OK;
}

int32_t GetBusinessEventMockEmpty(DistributedDeviceProfile::BusinessEvent &event)
{
    event.SetBusinessValue("");
    return DM_OK;
}

int32_t GetBusinessEventMockFalse(DistributedDeviceProfile::BusinessEvent &event)
{
    event.SetBusinessValue("{\"business_id\":\"test_business_id\",\"is_in_anti_disturbance_mode\":false}");
    return DM_OK;
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    context->accesser.dmVersion = DM_VERSION_5_1_0;
    context->accessee.displayId = -1;
    EXPECT_CALL(*softbusConnectorMock, GetSoftbusSession).WillOnce(Return(softbusSession));
    EXPECT_CALL(*tokenMock, GetNativeTokenId(_)).Times(0);
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    JsonItemObject credObj;
    JsonObject aclInfo;
    aclInfo["pointTopointAcl"] = 1;
    JsonObject credTypeJson;
    int32_t credType = 1;
    std::vector<std::string> deleteCredInfo;

    authState->GetSinkCredTypeForP2P(context, credObj, aclInfo, credTypeJson, credType, deleteCredInfo);
    EXPECT_EQ(credTypeJson["pointTopointCredType"].Get<int32_t>(), credType);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();

    DistributedDeviceProfile::AccessControlProfile profile;
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserTokenId(123);
    accessee.SetAccesseeTokenId(456);
    profile.SetAccesser(accesser);
    profile.SetAccessee(accessee);
    profile.SetBindLevel(1);

    JsonObject credIdJson;
    credIdJson[FILED_CRED_TYPE] = DM_POINT_TO_POINT;
    std::vector<std::string> appList = {"123", "456"};
    credIdJson[FILED_AUTHORIZED_APP_LIST] = appList;
    credIdJson[FILED_AUTHORIZED_SCOPE] = DM_AUTH_SCOPE_USER;
    JsonObject credInfo;
    std::string test_cred_id = "123";
    credInfo.Insert(test_cred_id, credIdJson);

    bool checkResult = false;
    authState->CheckCredIdInAclForP2P(context, test_cred_id, profile, credInfo, DM_POINT_TO_POINT, checkResult);
    GTEST_LOG_(INFO) << "checkResult=" << checkResult;
    EXPECT_TRUE(checkResult);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    context->accessee.deviceId = "1";
    context->accessee.userId = 0;
    context->accesser.deviceIdHash = Crypto::Sha256("1");

    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserDeviceId("1");
    accesser.SetAccesserUserId(0);
    accessee.SetAccesseeDeviceId("1");
    accessee.SetAccesseeUserId(0);

    bool result = authState->IdenticalAccountAclCompare(context, accesser, accessee);
    EXPECT_FALSE(result);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.deviceId = "1";
    context->accessee.userId = 0;
    context->accesser.deviceIdHash = Crypto::Sha256("1");

    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserDeviceId("1");
    accesser.SetAccesserUserId(0);
    accessee.SetAccesseeDeviceId("1");
    accessee.SetAccesseeUserId(0);

    bool result = authState->ShareAclCompare(context, accesser, accessee);
    EXPECT_FALSE(result);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_008, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.deviceId = "1";
    context->accessee.userId = 0;
    context->accesser.deviceIdHash = Crypto::Sha256("1");

    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserDeviceId("1");
    accesser.SetAccesserUserId(0);
    accesser.SetAccesserTokenId(0);
    accesser.SetAccesserBundleName("");
    accessee.SetAccesseeDeviceId("1");
    accessee.SetAccesseeUserId(0);
    accessee.SetAccesseeTokenId(0);
    accessee.SetAccesseeBundleName("");

    bool result = authState->LnnAclCompare(context, accesser, accessee);

    EXPECT_FALSE(result);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_Action_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();

    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->businessId = "test_business_id";

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetBusinessEvent(::testing::_))
        .WillOnce(::testing::Invoke(GetBusinessEventMockTrue));

    int32_t result = authState->Action(context);

    EXPECT_EQ(result, ERR_DM_ANTI_DISTURB_MODE);
    EXPECT_EQ(context->reason, ERR_DM_ANTI_DISTURB_MODE);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_Action_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();

    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->businessId = "test_business_id";

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetBusinessEvent(::testing::_))
        .WillOnce(::testing::Invoke(GetBusinessEventMockFalse));

    int32_t result = authState->Action(context);

    EXPECT_NE(result, ERR_DM_ANTI_DISTURB_MODE);
    EXPECT_NE(context->reason, ERR_DM_ANTI_DISTURB_MODE);
}

HWTEST_F(AuthNegotiateTest, IsAntiDisturbanceMode_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "";
    businessId.clear();
    EXPECT_FALSE(authState->IsAntiDisturbanceMode(businessId));
}

HWTEST_F(AuthNegotiateTest, IsAntiDisturbanceMode_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetBusinessEvent(::testing::_))
        .WillOnce(::testing::Return(ERR_DM_FAILED));

    EXPECT_FALSE(authState->IsAntiDisturbanceMode(businessId));
}

HWTEST_F(AuthNegotiateTest, IsAntiDisturbanceMode_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetBusinessEvent(::testing::_))
        .WillOnce(::testing::Invoke(GetBusinessEventMockEmpty));

    EXPECT_FALSE(authState->IsAntiDisturbanceMode(businessId));
}

HWTEST_F(AuthNegotiateTest, IsAntiDisturbanceMode_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";

    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetBusinessEvent(::testing::_))
        .WillOnce(::testing::Invoke(GetBusinessEventMockTrue));

    EXPECT_TRUE(authState->IsAntiDisturbanceMode(businessId));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "invalid_json";
    EXPECT_FALSE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "{\"is_in_anti_disturbance_mode\":true}";
    EXPECT_FALSE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "{\"business_id\":\"wrong_id\",\"is_in_anti_disturbance_mode\":true}";
    EXPECT_FALSE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "{\"business_id\":\"test_business_id\"}";
    EXPECT_FALSE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "{\"business_id\":\"test_business_id\",\"is_in_anti_disturbance_mode\":true}";
    EXPECT_TRUE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "{\"business_id\":\"test_business_id\",\"is_in_anti_disturbance_mode\":false}";
    EXPECT_FALSE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "{\"business_id\":123,\"is_in_anti_disturbance_mode\":true}";

    EXPECT_FALSE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, ParseAndCheckAntiDisturbanceMode_008, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::string businessId = "test_business_id";
    std::string businessValue = "{\"business_id\":\"test_business_id\",\"is_in_anti_disturbance_mode\":123}";

    EXPECT_FALSE(authState->ParseAndCheckAntiDisturbanceMode(businessId, businessValue));
}

HWTEST_F(AuthNegotiateTest, SinkNegotiateService_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    int result = authState->SinkNegotiateService(nullptr);
    EXPECT_EQ(result, ERR_DM_POINT_NULL);
}

HWTEST_F(AuthNegotiateTest, SinkNegotiateService_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.serviceId = 0;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    int result = authState->SinkNegotiateService(context);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(AuthNegotiateTest, GetSinkUserIdByDeviceType_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.displayId = 0;
    DmDeviceType deviceType = DmDeviceType::DEVICE_TYPE_CAR;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(-1));
    int result = authState->GetSinkUserIdByDeviceType(context, deviceType);
    EXPECT_EQ(result, -1);
}

HWTEST_F(AuthNegotiateTest, GetSinkUserIdByDeviceType_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.displayId = 0;
    DmDeviceType deviceType = DmDeviceType::DEVICE_TYPE_PAD;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    int result = authState->GetSinkUserIdByDeviceType(context, deviceType);
    EXPECT_EQ(result, 100);
}

HWTEST_F(AuthNegotiateTest, GetSinkCarUserId_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.displayId = 0;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(-1));
    int result = authState->GetSinkCarUserId(context);
    EXPECT_EQ(result, -1);
}

HWTEST_F(AuthNegotiateTest, GetSinkCarUserId_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.displayId = -1;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(DM_OK));
    int result = authState->GetSinkCarUserId(context);
    EXPECT_EQ(result, 100);
}

HWTEST_F(AuthNegotiateTest, GetSinkCarUserId_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.displayId = 6;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(DM_OK));
    int result = authState->GetSinkCarUserId(context);
    EXPECT_EQ(result, -1);
}

HWTEST_F(AuthNegotiateTest, GetSinkCarUserId_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.displayId = 0;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(DM_OK));
    int result = authState->GetSinkCarUserId(context);
    EXPECT_EQ(result, 100);
}

HWTEST_F(AuthNegotiateTest, GetSinkCarUserId_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->accessee.displayId = 0;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_CALL(*multipleUserConnectorMock_, GetUserIdByDisplayId(_)).WillOnce(Return(100));
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int result = authState->GetSinkCarUserId(context);
    EXPECT_EQ(result, 100);
}

/**
 * @tc.name: NegotiateProcess_HandleNegotiateResult_001
 * @tc.desc: HandleNegotiateResult should return ERR_DM_POINT_NULL when context is nullptr.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_HandleNegotiateResult_001, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(NegotiateProcess::GetInstance().HandleNegotiateResult(nullptr), ERR_DM_POINT_NULL);
}

/**
 * @tc.name: NegotiateProcess_HandleNegotiateResult_002
 * @tc.desc: No cred no acl with input pincode maps to PinBindAndCredAuth on source side.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */

/**
 * @tc.name: NegotiateProcess_HandleProxyNegotiateResult_001
 * @tc.desc: HandleProxyNegotiateResult with null context returns ERR_DM_POINT_NULL.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_HandleProxyNegotiateResult_001, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(NegotiateProcess::GetInstance().HandleProxyNegotiateResult(nullptr, DM_OK), ERR_DM_POINT_NULL);
}

/**
 * @tc.name: NegotiateProcess_HandleProxyNegotiateResult_002
 * @tc.desc: HandleProxyNegotiateResult returns DM_OK when not proxy bind (early return).
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_HandleProxyNegotiateResult_002, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->IsProxyBind = false;
    EXPECT_EQ(NegotiateProcess::GetInstance().HandleProxyNegotiateResult(ctx, DM_OK), DM_OK);
}

/**
 * @tc.name: NegotiateProcess_HandleServiceNegotiateResult_001
 * @tc.desc: HandleServiceNegotiateResult with null context returns ERR_DM_POINT_NULL.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_HandleServiceNegotiateResult_001, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(NegotiateProcess::GetInstance().HandleServiceNegotiateResult(nullptr, DM_OK), ERR_DM_POINT_NULL);
}

/**
 * @tc.name: NegotiateProcess_HandleServiceNegotiateResult_002
 * @tc.desc: HandleServiceNegotiateResult returns DM_OK when subjectServiceOnes is empty.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_HandleServiceNegotiateResult_002, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->IsProxyBind = true;
    ctx->subjectServiceOnes.clear();
    EXPECT_EQ(NegotiateProcess::GetInstance().HandleServiceNegotiateResult(ctx, DM_OK), DM_OK);
}

/**
 * @tc.name: NegotiateProcess_IsExistTheTokenId_001
 * @tc.desc: IsExistTheTokenId returns false when extraData is empty.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_IsExistTheTokenId_001, testing::ext::TestSize.Level1)
{
    EXPECT_FALSE(NegotiateProcess::GetInstance().IsExistTheTokenId("", "hashValue"));
}

/**
 * @tc.name: NegotiateProcess_IsExistTheTokenId_002
 * @tc.desc: IsExistTheTokenId returns false when extraData is discarded json.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_IsExistTheTokenId_002, testing::ext::TestSize.Level1)
{
    EXPECT_FALSE(NegotiateProcess::GetInstance().IsExistTheTokenId("invalid_json", "hashValue"));
}

/**
 * @tc.name: NegotiateProcess_IsExistTheTokenId_003
 * @tc.desc: IsExistTheTokenId returns false when proxy object is discarded (no proxy field).
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_IsExistTheTokenId_003, testing::ext::TestSize.Level1)
{
    std::string extra = R"({"otherField":123})";
    EXPECT_FALSE(NegotiateProcess::GetInstance().IsExistTheTokenId(extra, "hashValue"));
}

/**
 * @tc.name: NegotiateProcess_IsNeedSetProxyRelationShip_001
 * @tc.desc: IsNeedSetProxyRelationShip returns true when p2p acl profile not present in proxyAccesser.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, NegotiateProcess_IsNeedSetProxyRelationShip_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SOURCE;
    DmProxyAuthContext proxyContext;
    proxyContext.proxyAccesser.tokenIdHash = "hashA";
    proxyContext.proxyAccessee.tokenIdHash = "hashB";
    EXPECT_TRUE(NegotiateProcess::GetInstance().IsNeedSetProxyRelationShip(ctx, proxyContext));
}

/**
 * @tc.name: DmAuthContext_GetDeviceId_001
 * @tc.desc: GetDeviceId returns local/remote device id correctly for source direction.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_GetDeviceId_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SOURCE;
    ctx->accesser.deviceId = "localDev";
    ctx->accessee.deviceId = "remoteDev";
    EXPECT_EQ(ctx->GetDeviceId(DM_AUTH_LOCAL_SIDE), "localDev");
    EXPECT_EQ(ctx->GetDeviceId(DM_AUTH_REMOTE_SIDE), "remoteDev");
}

/**
 * @tc.name: DmAuthContext_GetDeviceId_002
 * @tc.desc: GetDeviceId returns local/remote device id correctly for sink direction.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_GetDeviceId_002, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SINK;
    ctx->accesser.deviceId = "localDev";
    ctx->accessee.deviceId = "remoteDev";
    EXPECT_EQ(ctx->GetDeviceId(DM_AUTH_LOCAL_SIDE), "remoteDev");
    EXPECT_EQ(ctx->GetDeviceId(DM_AUTH_REMOTE_SIDE), "localDev");
}

/**
 * @tc.name: DmAuthContext_GetUserId_001
 * @tc.desc: GetUserId returns local/remote userId correctly for sink direction.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_GetUserId_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SINK;
    ctx->accesser.userId = 100;
    ctx->accessee.userId = 200;
    EXPECT_EQ(ctx->GetUserId(DM_AUTH_LOCAL_SIDE), 200);
    EXPECT_EQ(ctx->GetUserId(DM_AUTH_REMOTE_SIDE), 100);
}

/**
 * @tc.name: DmAuthContext_GetCredentialId_001
 * @tc.desc: GetCredentialId returns transmit and lnn credential ids by scope/side.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_GetCredentialId_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SOURCE;
    ctx->accesser.transmitCredentialId = "transA";
    ctx->accesser.lnnCredentialId = "lnnA";
    ctx->accessee.transmitCredentialId = "transB";
    ctx->accessee.lnnCredentialId = "lnnB";
    EXPECT_EQ(ctx->GetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_APP), "transA");
    EXPECT_EQ(ctx->GetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_LNN), "lnnA");
    EXPECT_EQ(ctx->GetCredentialId(DM_AUTH_REMOTE_SIDE, DM_AUTH_SCOPE_APP), "transB");
}

/**
 * @tc.name: DmAuthContext_GetPublicKey_001
 * @tc.desc: GetPublicKey returns transmit and lnn public keys by scope/side on sink side.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_GetPublicKey_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SINK;
    ctx->accesser.transmitPublicKey = "pkTransA";
    ctx->accesser.lnnPublicKey = "pkLnnA";
    ctx->accessee.transmitPublicKey = "pkTransB";
    EXPECT_EQ(ctx->GetPublicKey(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_APP), "pkTransB");
    EXPECT_EQ(ctx->GetPublicKey(DM_AUTH_REMOTE_SIDE, DM_AUTH_SCOPE_LNN), "pkLnnA");
}

/**
 * @tc.name: DmAuthContext_SetCredentialId_001
 * @tc.desc: SetCredentialId writes transmit and lnn credential ids into accesser/accessee.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_SetCredentialId_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SOURCE;
    ctx->SetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_APP, "newTrans");
    ctx->SetCredentialId(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_LNN, "newLnn");
    EXPECT_EQ(ctx->accesser.transmitCredentialId, "newTrans");
    EXPECT_EQ(ctx->accesser.lnnCredentialId, "newLnn");
    ctx->SetCredentialId(DM_AUTH_REMOTE_SIDE, DM_AUTH_SCOPE_APP, "remoteTrans");
    EXPECT_EQ(ctx->accessee.transmitCredentialId, "remoteTrans");
}

/**
 * @tc.name: DmAuthContext_SetPublicKey_001
 * @tc.desc: SetPublicKey writes transmit and lnn public keys on sink direction.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_SetPublicKey_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SINK;
    ctx->SetPublicKey(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_APP, "localTransPk");
    ctx->SetPublicKey(DM_AUTH_LOCAL_SIDE, DM_AUTH_SCOPE_LNN, "localLnnPk");
    ctx->SetPublicKey(DM_AUTH_REMOTE_SIDE, DM_AUTH_SCOPE_LNN, "remoteLnnPk");
    EXPECT_EQ(ctx->accessee.transmitPublicKey, "localTransPk");
    EXPECT_EQ(ctx->accessee.lnnPublicKey, "localLnnPk");
    EXPECT_EQ(ctx->accesser.lnnPublicKey, "remoteLnnPk");
}

/**
 * @tc.name: DmAuthContext_GetAccountId_001
 * @tc.desc: GetAccountId returns local/remote account id based on direction.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_GetAccountId_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    ctx->direction = DM_AUTH_SOURCE;
    ctx->accesser.accountId = "accA";
    ctx->accessee.accountId = "accB";
    EXPECT_EQ(ctx->GetAccountId(DM_AUTH_LOCAL_SIDE), "accA");
    EXPECT_EQ(ctx->GetAccountId(DM_AUTH_REMOTE_SIDE), "accB");
}

/**
 * @tc.name: DmAuthContext_GetAclBundleName_001
 * @tc.desc: GetAclBundleName returns pkgName when non-empty, otherwise bundleName.
 * @tc.type: FUNC
 * @tc.require: I9JUK2
 */
HWTEST_F(AuthNegotiateTest, DmAuthContext_GetAclBundleName_001, testing::ext::TestSize.Level1)
{
    auto ctx = std::make_shared<DmAuthContext>();
    DmProxyAuthContext proxyContext;
    proxyContext.proxyAccesser.pkgName = "srcPkg";
    proxyContext.proxyAccesser.bundleName = "srcBundle";
    proxyContext.proxyAccessee.pkgName = "";
    proxyContext.proxyAccessee.bundleName = "sinkBundle";
    EXPECT_EQ(ctx->GetAclBundleName(DM_AUTH_SOURCE, proxyContext), "srcPkg");
    EXPECT_EQ(ctx->GetAclBundleName(DM_AUTH_SINK, proxyContext), "sinkBundle");
}
}
}