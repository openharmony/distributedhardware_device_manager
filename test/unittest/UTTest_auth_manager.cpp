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
}

void AuthManagerTest::TearDownTestCase()
{
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
}

HWTEST_F(AuthManagerTest, HandleBusinessEvents_001, testing::ext::TestSize.Level1)
{
    std::string businessId = "testBusinessId";
    int32_t action = USER_OPERATION_TYPE_CANCEL_AUTH;
    EXPECT_CALL(*distributedDeviceProfileClientMock_,
        PutBusinessEvent(::testing::_)).WillOnce(::testing::Return(DM_OK));

    int32_t ret = authManager->HandleBusinessEvents(businessId, action);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthManagerTest, HandleBusinessEvents_002, testing::ext::TestSize.Level1)
{
    std::string businessId = "testBusinessId";
    int32_t action = USER_OPERATION_TYPE_CANCEL_AUTH;

    EXPECT_CALL(*distributedDeviceProfileClientMock_,
        PutBusinessEvent(::testing::_)).WillOnce(::testing::Return(ERR_DM_AUTH_NOT_START));

    int32_t ret = authManager->HandleBusinessEvents(businessId, action);
    EXPECT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(AuthManagerTest, HandleBusinessEvents_003, testing::ext::TestSize.Level1)
{
    std::string businessId = "testBusinessId";
    int32_t action = USER_OPERATION_TYPE_ALLOW_AUTH;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutBusinessEvent(::testing::_))
        .WillOnce(::testing::Return(DM_OK));

    int32_t result = authManager->HandleBusinessEvents(businessId, action);

    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(AuthManagerTest, HandleBusinessEvents_004, testing::ext::TestSize.Level1)
{
    std::string businessId = "";
    int32_t action = USER_OPERATION_TYPE_CANCEL_AUTH;

    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutBusinessEvent(::testing::_))
        .WillOnce(::testing::Return(DM_OK));

    int32_t result = authManager->HandleBusinessEvents(businessId, action);

    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(AuthManagerTest, ParseJsonObject_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[DM_BUSINESS_ID] = "testBusinessId";

    authManager->ParseJsonObject(jsonObject);
    EXPECT_EQ(jsonObject[DM_BUSINESS_ID].Get<std::string>(), "testBusinessId");
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

    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AuthManagerTest, OnUserOperation_005, testing::ext::TestSize.Level1)
{
    int32_t action = USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT;
    std::string params = "";

    int32_t ret = authManager->OnUserOperation(action, params);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_NE(authManager->GetAuthContext()->reason, DM_OK);
}

HWTEST_F(AuthManagerTest, OnUserOperation_006, testing::ext::TestSize.Level1)
{
    int32_t action = USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY;
    std::string params = "";

    int32_t ret = authManager->OnUserOperation(action, params);

    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(authManager->GetAuthContext()->reason, ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY);
}

HWTEST_F(AuthManagerTest, OnUserOperation_007, testing::ext::TestSize.Level1)
{
    int32_t action = -1;
    std::string params = "";

    int32_t ret = authManager->OnUserOperation(action, params);

    EXPECT_EQ(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
