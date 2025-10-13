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
#include "UTTest_auth_negotiate.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

constexpr const char* TEST_STRING = "test_string";
constexpr int32_t TEST_NEGATIVE = -1;
constexpr int32_t TEST_POSITIVE = 1;
void AuthNegotiateTest::SetUpTestCase()
{
    LOGI("AuthNegotiateTest::SetUpTestCase start.");

    DmSoftbusConnector::dmSoftbusConnector = softbusConnectorMock;
    DmSoftbusSession::dmSoftbusSession = softbusSessionMock;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
}

void AuthNegotiateTest::TearDownTestCase()
{
    LOGI("AuthNegotiateTest::TearDownTestCase done.");
    softbusConnectorMock = nullptr;
    softbusSessionMock = nullptr;
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    DmSoftbusSession::dmSoftbusSession = nullptr;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
}

void AuthNegotiateTest::SetUp()
{
    LOGI("AuthNegotiateTest::SetUp start.");
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
    LOGI("AuthNegotiateTest::TearDown done.");
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

HWTEST_F(AuthNegotiateTest, AuthSrcStartState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcStartState> authState = std::make_shared<AuthSrcStartState>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_START_STATE);
}

HWTEST_F(AuthNegotiateTest, AuthSrcStartState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcStartState> authState = std::make_shared<AuthSrcStartState>();
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthNegotiateTest, AuthSrcNegotiateStateMachine_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcNegotiateStateMachine> authState = std::make_shared<AuthSrcNegotiateStateMachine>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_NEGOTIATE_STATE);
}

HWTEST_F(AuthNegotiateTest, AuthSrcNegotiateStateMachine_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSrcNegotiateStateMachine> authState = std::make_shared<AuthSrcNegotiateStateMachine>();

    EXPECT_CALL(*softbusConnectorMock, GetSoftbusSession)
        .WillOnce(Return(softbusSession));

    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_NEGOTIATE_STATE);
}

HWTEST_F(AuthNegotiateTest, AuthSinkNegotiateStateMachine_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthSinkNegotiateStateMachine> authState = std::make_shared<AuthSinkNegotiateStateMachine>();
    context->accesser.dmVersion = DM_VERSION_5_1_0;
    context->accessee.displayId = -1;
    EXPECT_CALL(*softbusConnectorMock, GetSoftbusSession).WillOnce(Return(softbusSession));
    EXPECT_CALL(*tokenMock, GetNativeTokenId(_)).WillOnce(Return(1));
    EXPECT_EQ(authState->Action(context), ERR_DM_GET_TOKENID_FAILED);
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
}
}