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

/* *
 * @tc.name: RegisterUiStateCallback_001
 * @tc.desc: Test RegisterUiStateCallback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos_test";
    authManager->context_->authUiStateMgr = nullptr;
    int32_t ret = authManager->RegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/* *
 * @tc.name: RegisterUiStateCallback_002
 * @tc.desc: Test RegisterUiStateCallback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, RegisterUiStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos_test";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager->context_->authUiStateMgr = std::make_shared<AuthUiStateManager>(listener);
    int32_t ret = authManager->RegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

/* *
 * @tc.name: UnRegisterUiStateCallback_001
 * @tc.desc: Test UnRegisterUiStateCallback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos_test";
    authManager->context_->authUiStateMgr = nullptr;
    int32_t ret = authManager->UnRegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/* *
 * @tc.name: UnRegisterUiStateCallback_002
 * @tc.desc: Test UnRegisterUiStateCallback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, UnRegisterUiStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos_test";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager->context_->authUiStateMgr = std::make_shared<AuthUiStateManager>(listener);
    int32_t ret = authManager->UnRegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
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

/* *
 * @tc.name: IsAuthCodeReady_003
 * @tc.desc: Test IsAuthCodeReady
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, IsAuthCodeReady_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    authManager->context_->importAuthCode = "importAuthCode";
    authManager->context_->importPkgName = "importPkgName";
    bool ret = authManager->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

/* *
 * @tc.name: IsAuthCodeReady_004
 * @tc.desc: Test IsAuthCodeReady
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AuthManagerTest, IsAuthCodeReady_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos_test";
    authManager->context_->importAuthCode = "importAuthCode";
    authManager->context_->importPkgName = "ohos_test";
    bool ret = authManager->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, true);
}
} // namespace DistributedHardware
} // namespace OHOS
