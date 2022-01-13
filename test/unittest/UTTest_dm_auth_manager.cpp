/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dm_auth_manager.h"

#include "nlohmann/json.hpp"

#include "dm_log.h"
#include "dm_constants.h"
#include "auth_message_processor.h"
#include "UTTest_dm_auth_manager.h"

namespace OHOS {
namespace DistributedHardware {
void DmAuthManagerTest::SetUp()
{
}
void DmAuthManagerTest::TearDown()
{
}
void DmAuthManagerTest::SetUpTestCase()
{
}
void DmAuthManagerTest::TearDownTestCase()
{
}

namespace {
    std::string AUTHENTICATE_TIMEOUT_TASK = "authenticateTimeoutTask";
    std::string NEGOTIATE_TIMEOUT_TASK = "negotiateTimeoutTask";
    std::string CONFIRM_TIMEOUT_TASK = "confirmTimeoutTask";
    std::string INPUT_TIMEOUT_TASK = "inputTimeoutTask";
    std::string ADD_TIMEOUT_TASK = "addTimeoutTask";

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_001
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_AUTH_BUSINESS_BUSY
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestInitState>();
    std::shared_ptr<DmAuthRequestContext> context = std::make_shared<DmAuthRequestContext>();
    std::string pkgName = "111";
    int32_t authType = 1;
    std::string deviceId = "222";
    std::string extra = "333";
    authRequestState->SetAuthContext(context);
    authManager->SetAuthRequestState(authRequestState);
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_AUTH_BUSINESS_BUSY);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_002
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_AUTH_BUSINESS_BUSY
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseInitState>();
    std::shared_ptr<DmAuthResponseContext> context = std::make_shared<DmAuthResponseContext>();
    const std::string pkgName = "111";
    int32_t authType = 1;
    const std::string deviceId = "222";
    const std::string extra = "333";
    authResponseState->SetAuthContext(context);
    authManager->SetAuthResponseState(authResponseState);
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_AUTH_BUSINESS_BUSY);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_003
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_INPUT_PARA_EMPTY
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    std::string pkgName = "111";
    int32_t authType = 1;
    std::string deviceId = "222";
    std::string extra = "";
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_INPUT_PARA_EMPTY);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_004
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_INPUT_PARA_EMPTY
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    nlohmann::json json;
    const std::string pkgName = "111";
    int32_t authType = 4;
    const std::string deviceId = "222";
    const std::string extra = "targetPkgName";
    authRequestContext->targetPkgName =  "targetPkgName";
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_INPUT_PARA_EMPTY);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_005
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_005, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    nlohmann::json json;
    const std::string pkgName = "111";
    int32_t authType = 1;
    const std::string deviceId = "222";
    const std::string extra = "targetPkgName";
    authRequestContext->targetPkgName =  "targetPkgName";
    json[TARGET_PKG_NAME_KEY] =  authRequestContext->targetPkgName;
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_006
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_006, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    nlohmann::json json;
    const std::string pkgName = "111";
    int32_t authType = 1;
    const std::string deviceId = "222";
    const std::string extra = "appName";
    authRequestContext->targetPkgName =  "appName";
    json[APP_NAME_KEY] =  authRequestContext->targetPkgName;
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_007
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_007, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    nlohmann::json json;
    const std::string pkgName = "111";
    int32_t authType = 1;
    const std::string deviceId = "222";
    const std::string extra = "appDescription";
    authRequestContext->targetPkgName =  "appDescription";
    json[APP_DESCRIPTION_KEY] =  authRequestContext->targetPkgName;
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_008
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_008, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    nlohmann::json json;
    const std::string pkgName = "111";
    int32_t authType = 1;
    const std::string deviceId = "222";
    const std::string extra = "appThumbnail";
    authRequestContext->targetPkgName =  "appThumbnail";
    json[APP_THUMBNAIL] =  authRequestContext->targetPkgName;
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::AuthenticateDevice_009
 * @tc.desc: Call authenticatedevice to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AuthenticateDevice_009, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    nlohmann::json json;
    const std::string pkgName = "111";
    int32_t authType = 1;
    const std::string deviceId = "222";
    const std::string extra = "appIcon";
    authRequestContext->targetPkgName =  "appIcon";
    json[APP_ICON_KEY] =  authRequestContext->targetPkgName;
    int32_t ret = authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::UnAuthenticateDevice_001
 * @tc.desc: Call unauthenticateddevice to check whether the return value is DM_ FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    std::string pkgName = "";
    std::string deviceId = "222";
    int32_t ret = authManager->UnAuthenticateDevice(pkgName, deviceId);
    ASSERT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::VerifyAuthentication_001
 * @tc.desc: Call verifyauthentication to check whether the return value is DM_ FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, VerifyAuthentication_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestNetworkState>();
    std::shared_ptr<DmTimer> negotiateStartTimer = std::make_shared<DmTimer>(INPUT_TIMEOUT_TASK);
    authManager->timerMap_[INPUT_TIMEOUT_TASK] = negotiateStartTimer;
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager->authResponseContext_=nullptr;
    authManager->SetAuthRequestState(authRequestState);
    int32_t ret = authManager->HandleAuthenticateTimeout();
    ASSERT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::HandleAuthenticateTimeout_001
 * @tc.desc: authResponseContext_= nullptr; Call handleauthenticatemeout to check whether the return value is DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestNetworkState>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager->authResponseContext_=nullptr;
    authManager->SetAuthRequestState(authRequestState);
    int32_t ret = authManager->HandleAuthenticateTimeout();
    ASSERT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::HandleAuthenticateTimeout_002
 * @tc.desc: authResponseContext_= nullptr; Call handleauthenticatemeout to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager->SetAuthRequestState(authRequestState);
    int32_t ret = authManager->HandleAuthenticateTimeout();
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::EstablishAuthChannel_001
 * @tc.desc: Call establishauthchannel to check whether the return value is DM_ FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthResponseContext> authRequestContext = std::make_shared<DmAuthResponseContext>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    std::string deviceId1;
    int32_t ret = authManager->EstablishAuthChannel(deviceId1);
    ASSERT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::SendAuthRequest_001
 * @tc.desc: Cryptosupport = true call sendauthrequest to check whether the return value is DM_ FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SendAuthRequest_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestNegotiateState>();
    std::shared_ptr<DmTimer> negotiateStartTimer = std::make_shared<DmTimer>(NEGOTIATE_TIMEOUT_TASK);
    authManager->timerMap_[NEGOTIATE_TIMEOUT_TASK] = negotiateStartTimer;
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();;
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager->authResponseContext_->cryptoSupport = true;
    authManager->SetAuthRequestState(authRequestState);
    int32_t sessionId=1;
    int32_t ret = authManager->SendAuthRequest(sessionId);
    ASSERT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::StartAuthProcess_001
 * @tc.desc: Whether the return value of calling startauthprocess is DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, StartAuthProcess_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    int32_t ret = authManager->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::StartAuthProcess_002
 * @tc.desc: Whether the return value of calling startauthprocess is DM_ OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, StartAuthProcess_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->SetAuthResponseState(authResponseState);
    authManager->authResponseContext_->sessionId=111;
    int32_t action = 1;
    int32_t ret = authManager->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::CreateGroup_001
 * @tc.desc: Whether the return value of calling creategroup is DM_ OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, CreateGroup_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->SetAuthResponseState(authResponseState);
    authManager->authResponseContext_->requestId=111;
    authManager->authResponseContext_->groupName="111";
    int32_t action = 1;
    int32_t ret = authManager->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::AddMember_001
 * @tc.desc: Whether the return value of calling addmember is DM_ OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, AddMember_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseInitState>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmTimer> joinStartTimer = std::make_shared<DmTimer>(ADD_TIMEOUT_TASK);
    authManager->timerMap_[ADD_TIMEOUT_TASK] = joinStartTimer;
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObject;
    authManager->authResponseContext_->groupId="111";
    authManager->authResponseContext_->groupName="222";
    authManager->authResponseContext_->code=123;
    authManager->authResponseContext_->requestId=234;
    authManager->authResponseContext_->deviceId="234";
    jsonObject[TAG_GROUP_ID] = authManager->authResponseContext_->groupId;
    jsonObject[TAG_GROUP_NAME] = authManager->authResponseContext_->groupName;
    jsonObject[PIN_CODE_KEY] = authManager->authResponseContext_->code;
    jsonObject[TAG_REQUEST_ID] = authManager->authResponseContext_->requestId;
    jsonObject[TAG_DEVICE_ID] = authManager->authResponseContext_->deviceId;
    std::string deviceId = "44444";
    authManager->SetAuthResponseState(authResponseState);
    int32_t ret = authManager->AddMember(deviceId);
    ASSERT_EQ(ret, DM_OK);
    sleep(15);
}

/**
 * @tc.name: DmAuthManager::JoinNetwork_001
 * @tc.desc: Whether the return value of calling joinnetwork is DM_ OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, JoinNetwork_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    std::shared_ptr<DmTimer> authenticateStartTimer = std::make_shared<DmTimer>(AUTHENTICATE_TIMEOUT_TASK);
    authManager->timerMap_[AUTHENTICATE_TIMEOUT_TASK] = authenticateStartTimer;
    authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager->SetAuthRequestState(authRequestState);
    int32_t ret = authManager->JoinNetwork();
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::GetIsCryptoSupport_001
 * @tc.desc: authResponseState_ Equal to nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    authManager->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    bool isCryptoSupport = false;
    int32_t ret = authManager->GetIsCryptoSupport(isCryptoSupport);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::SetAuthResponseState_001
 * @tc.desc: Is the authresponsestate assignment successful
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, SetAuthResponseState_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseFinishState>();
    authManager->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager->SetAuthResponseState(authResponseState);
    int32_t ret = authManager->SetAuthResponseState(authResponseState);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DmAuthManager::GetPinCode_001
 * @tc.desc: Return authresponsecontext - > code
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAuthManagerTest, GetPinCode_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmAuthManager> authManager = std::make_shared<DmAuthManager>(softbusConnector, listener);
    authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();;
    int32_t ret = authManager->GetPinCode();
    ASSERT_EQ(ret, authManager->authResponseContext_->code);
}
}
}
}
