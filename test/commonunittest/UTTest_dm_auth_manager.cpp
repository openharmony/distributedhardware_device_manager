/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "UTTest_dm_auth_manager.h"

#include "nlohmann/json.hpp"

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_auth_manager.h"
#include "auth_message_processor.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DmAuthManagerTest::SetUp()
{
    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->softbusConnector_ = std::make_shared<SoftbusConnector>();
    authManager_->softbusConnector_->GetSoftbusSession()->
        RegisterSessionCallback(std::shared_ptr<ISoftbusSessionCallback>(authManager_));
    authManager_->timer_ = std::make_shared<DmTimer>();
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
const int32_t MIN_PIN_CODE = 100000;
const int32_t MAX_PIN_CODE = 999999;

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string networkId = "222";
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, networkId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string networkId = "222";
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->isAuthenticateDevice_ = true;
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, networkId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string networkId = "222";
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->isAuthenticateDevice_ = false;
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, networkId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string networkId = "networkId";
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, networkId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_001, testing::ext::TestSize.Level0)
{
    std::string name = "test";
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestNetworkState>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager_->authResponseContext_ = nullptr;
    authManager_->SetAuthRequestState(authRequestState);
    authManager_->HandleAuthenticateTimeout(name);
    ASSERT_TRUE(authManager_->authResponseContext_ != nullptr);
}

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_002, testing::ext::TestSize.Level0)
{
    std::string name = "test";
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    authManager_->SetAuthRequestState(authRequestState);
    authManager_->HandleAuthenticateTimeout(name);
    ASSERT_TRUE(authManager_->authRequestState_ != nullptr);
}

HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusSession> sessionSession = std::shared_ptr<SoftbusSession>();
    std::shared_ptr<DmAuthResponseContext> authRequestContext = std::make_shared<DmAuthResponseContext>();
    std::string deviceId1;
    int32_t ret = authManager_->EstablishAuthChannel(deviceId1);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    authManager_->StartAuthProcess(action);
    bool ret = authManager_->authRequestContext_->deviceName.empty();
    ASSERT_EQ(ret, true);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseInitState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager_->SetAuthResponseState(authResponseState);
    authManager_->authResponseContext_->sessionId = 111;
    authManager_->softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(authManager_);
    int32_t action = 1;
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    int32_t action = 1;
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(DmAuthManagerTest, CreateGroup_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    authManager_->SetAuthResponseState(authResponseState);
    authManager_->authResponseContext_->requestId = 111;
    authManager_->authResponseContext_->groupName = "111";
    int32_t ret = authManager_->CreateGroup();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, CreateGroup_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->CreateGroup();
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, AddMember_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseInitState>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    nlohmann::json jsonObject;
    authManager_->authResponseContext_->groupId = "111";
    authManager_->authResponseContext_->groupName = "222";
    authManager_->authResponseContext_->code = 123;
    authManager_->authResponseContext_->requestId = 234;
    authManager_->authResponseContext_->deviceId = "234";
    int32_t pinCode = 444444;
    authManager_->hiChainConnector_->RegisterHiChainCallback(authManager_);
    authManager_->SetAuthResponseState(authResponseState);
    int32_t ret = authManager_->AddMember(pinCode);
    ASSERT_NE(ret, -1);
}

HWTEST_F(DmAuthManagerTest, AddMember_002, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 33333;
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->AddMember(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, JoinNetwork_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    const int32_t sessionId = 65;
    const std::string message = "messageTest";
    int64_t requestId = 444;
    const std::string groupId = "{}";
    int32_t status = 1;
    authManager_->OnGroupCreated(requestId, groupId);
    authManager_->OnMemberJoin(requestId, status);
    authManager_->OnDataReceived(sessionId, message);
    authManager_->SetAuthRequestState(authRequestState);
    int32_t ret = authManager_->JoinNetwork();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, JoinNetwork_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    int32_t userId = 1;
    authManager_->UserSwitchEventCallback(userId);
    authManager_->AuthenticateFinish();
    authManager_->CancelDisplay();
    int32_t ret = authManager_->JoinNetwork();
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, SetAuthResponseState_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseFinishState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t ret = authManager_->SetAuthResponseState(authResponseState);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GetPinCode_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->code = 123456;
    int32_t ret = authManager_->GetPinCode();
    ASSERT_EQ(ret, 123456);
}

HWTEST_F(DmAuthManagerTest, GetPinCode_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowConfigDialog();
    authManager_->ShowAuthInfoDialog();
    authManager_->ShowStartAuthDialog();
    int32_t ret = authManager_->GetPinCode();
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(DmAuthManagerTest, SetPageId_001, testing::ext::TestSize.Level0)
{
    int32_t pageId = 123;
    int32_t ret = authManager_->SetPageId(pageId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetPageId_002, testing::ext::TestSize.Level0)
{
    int32_t pageId = 123;
    authManager_->authResponseContext_ = nullptr;
    authManager_->authMessageProcessor_ = nullptr;
    const int32_t sessionId = 65;
    const std::string message = "messageTest";
    int64_t requestId = 555;
    int32_t status = 2;
    authManager_->OnMemberJoin(requestId, status);
    authManager_->OnDataReceived(sessionId, message);
    int32_t ret = authManager_->SetPageId(pageId);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_001, testing::ext::TestSize.Level0)
{
    const int32_t sessionId = 78;
    int32_t reason = 123;
    int32_t state = 456;
    authManager_->OnSessionClosed(sessionId);
    authManager_->authResponseContext_ = nullptr;
    int64_t requestId = 333;
    const std::string groupId = "{}";
    authManager_->OnGroupCreated(requestId, groupId);
    int32_t ret = authManager_->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_002, testing::ext::TestSize.Level0)
{
    int32_t reason = 1234;
    int32_t state = 5678;
    int64_t requestId = 22;
    const std::string groupId = "{}";
    authManager_->OnGroupCreated(requestId, groupId);
    int32_t ret = authManager_->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_003, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    int32_t reason = 12;
    int32_t state = 36;
    int32_t ret = authManager_->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, IsIdenticalAccount_001, testing::ext::TestSize.Level0)
{
    bool ret = authManager_->IsIdenticalAccount();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, GetAccountGroupIdHash_001, testing::ext::TestSize.Level0)
{
    auto ret = authManager_->GetAccountGroupIdHash();
    ASSERT_EQ(ret.empty(), true);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_001, testing::ext::TestSize.Level0)
{
    int32_t openedSessionId = 66;
    int32_t sessionSide = 0;
    int32_t result = 3;
    const int32_t closedSessionId = 11;
    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = nullptr;
    authManager_->timer_ = nullptr;
    authManager_->OnSessionOpened(openedSessionId, sessionSide, result);
    authManager_->OnSessionClosed(closedSessionId);
    int32_t ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_002, testing::ext::TestSize.Level0)
{
    int32_t openedSessionId = 66;
    int32_t sessionSide = 0;
    int32_t result = 3;
    const int32_t closedSessionId = 11;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->timer_ = std::make_shared<DmTimer>();
    authManager_->OnSessionOpened(openedSessionId, sessionSide, result);
    authManager_->OnSessionClosed(closedSessionId);
    int32_t ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_003, testing::ext::TestSize.Level0)
{
    int32_t openedSessionId = 66;
    int32_t sessionSide = 1;
    int32_t result = 3;
    const int32_t closedSessionId = 11;
    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = nullptr;
    authManager_->timer_ = nullptr;
    authManager_->OnSessionOpened(openedSessionId, sessionSide, result);
    authManager_->OnSessionClosed(closedSessionId);
    int32_t ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_004, testing::ext::TestSize.Level0)
{
    int32_t openedSessionId = 66;
    int32_t sessionSide = 1;
    int32_t result = 3;
    const int32_t closedSessionId = 11;
    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = std::make_shared<AuthRequestInitState>();
    authManager_->timer_ = std::make_shared<DmTimer>();
    authManager_->OnSessionOpened(openedSessionId, sessionSide, result);
    authManager_->OnSessionClosed(closedSessionId);
    int32_t ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = -1;
    std::string deviceId = "113456";
    std::string extra = "extraTest";
    int32_t ret = authManager_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_FAILED);
}

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_002, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->importPkgName_ = "ohos_test";
    authManager_->importAuthCode_ = "156161";
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.emplace(deviceId, infoPtr);
    int32_t ret = authManager_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "deviceIdTest";
    std::string extra = "extraTest";
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    authManager_->timer_ = nullptr;
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.emplace(deviceId, infoPtr);
    int32_t ret = authManager_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GenerateGroupName_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    std::string ret = authManager_->GenerateGroupName();
    ASSERT_TRUE(ret.empty());
}

HWTEST_F(DmAuthManagerTest, GenerateGroupName_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->targetPkgName = "targetPkgNameTest";
    authManager_->authResponseContext_->localDeviceId = "localDeviceIdTest";
    authManager_->action_ = 6;
    std::string ret = authManager_->GenerateGroupName();
    ASSERT_TRUE(!ret.empty());
}

HWTEST_F(DmAuthManagerTest, GenerateGroupName_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->targetPkgName = "targetPkgNameTest";
    authManager_->authResponseContext_->localDeviceId = "localDeviceIdTest";
    authManager_->action_ = 7;
    std::string ret = authManager_->GenerateGroupName();
    ASSERT_TRUE(!ret.empty());
}

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseState_ = nullptr;
    bool ret = authManager_->GetIsCryptoSupport();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->authRequestState_ = nullptr;
    bool ret = authManager_->GetIsCryptoSupport();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    bool ret = authManager_->GetIsCryptoSupport();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_001, testing::ext::TestSize.Level0)
{
    int32_t action = 0;
    std::string params = "paramsTest";
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_002, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    std::string params = "paramsTest1";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_003, testing::ext::TestSize.Level0)
{
    int32_t action = 2;
    std::string params = "paramsTest2";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_004, testing::ext::TestSize.Level0)
{
    int32_t action = 3;
    std::string params = "paramsTest3";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_005, testing::ext::TestSize.Level0)
{
    int32_t action = 4;
    std::string params = "paramsTest4";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_006, testing::ext::TestSize.Level0)
{
    int32_t action = 5;
    std::string params = "5";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_007, testing::ext::TestSize.Level0)
{
    int32_t action = 1111;
    std::string params = "paramsTest1111";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnBindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string udidHash = "udidHash";
    int32_t ret = authManager_->UnBindDevice(pkgName, udidHash);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnBindDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string udidHash = "222";
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->isAuthenticateDevice_ = true;
    int32_t ret = authManager_->UnBindDevice(pkgName, udidHash);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnBindDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string udidHash = "222";
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->isAuthenticateDevice_ = false;
    int32_t ret = authManager_->UnBindDevice(pkgName, udidHash);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnBindDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string udidHash = "networkId";
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->UnBindDevice(pkgName, udidHash);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, RequestCredential001, testing::ext::TestSize.Level0)
{
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->RequestCredential();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, GenerateCredential001, testing::ext::TestSize.Level0)
{
    std::string publicKey = "publicKey";
    authManager_->GenerateCredential(publicKey);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, RequestCredentialDone001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->RequestCredentialDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, RequestCredentialDone002, testing::ext::TestSize.Level0)
{
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->RequestCredential();
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->RequestCredentialDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, ImportCredential001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::string publicKey = "publicKey";
    int32_t ret = authManager_->ImportCredential(deviceId, publicKey);
    ASSERT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, EstablishUnbindChannel001, testing::ext::TestSize.Level0)
{
    std::string deviceIdHash = "deviceIdHash";
    int32_t ret = authManager_->EstablishUnbindChannel(deviceIdHash);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, RequestSyncDeleteAcl001, testing::ext::TestSize.Level0)
{
    authManager_->RequestSyncDeleteAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, RequestSyncDeleteAcl002, testing::ext::TestSize.Level0)
{
    authManager_->timer_ = nullptr;
    authManager_->RequestSyncDeleteAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SrcSyncDeleteAclDonel001, testing::ext::TestSize.Level0)
{
    authManager_->isFinishOfLocal_ = true;
    authManager_->authResponseContext_->reply = 0;
    authManager_->SrcSyncDeleteAclDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SinkSyncDeleteAclDone001, testing::ext::TestSize.Level0)
{
    authManager_->isFinishOfLocal_ = true;
    authManager_->authResponseContext_->reply = 0;
    authManager_->SinkSyncDeleteAclDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SyncDeleteAclDone001, testing::ext::TestSize.Level0)
{
    authManager_->SyncDeleteAclDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SyncDeleteAclDone002, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = nullptr;
    authManager_->SyncDeleteAclDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, ResponseCredential001, testing::ext::TestSize.Level0)
{
    authManager_->ResponseCredential();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, ResponseSyncDeleteAcl001, testing::ext::TestSize.Level0)
{
    authManager_->ResponseSyncDeleteAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 11;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->authResponseState_ = nullptr;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->authRequestState_ = nullptr;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit004, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->SrcAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SinkAuthDeviceFinish001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->SinkAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    int64_t requestId = 0;
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError002, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = nullptr;
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceSessionKey001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    int64_t requestId = 0;
    uint8_t *sessionKey = nullptr;
    uint32_t sessionKeyLen = 0;
    authManager_->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, GetRemoteDeviceId001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    authManager_->GetRemoteDeviceId(deviceId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, CompatiblePutAcl001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->CompatiblePutAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, CommonEventCallback001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    int32_t userId = 0;
    authManager_->CommonEventCallback(userId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AccountIdLogoutEventCallback001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    authManager_->AccountIdLogoutEventCallback(userId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, UserSwitchEventCallback001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    authManager_->UserSwitchEventCallback(userId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, UserChangeEventCallback001, testing::ext::TestSize.Level0)
{
    int32_t userId = 0;
    authManager_->UserChangeEventCallback(userId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, HandleSyncDeleteTimeout001, testing::ext::TestSize.Level0)
{
    std::string name;
    authManager_->HandleSyncDeleteTimeout(name);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, DeleteAcl001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    int32_t ret = authManager_->DeleteAcl(pkgName, deviceId);
    ASSERT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ProRespNegotiateExt001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    authManager_->ProRespNegotiateExt(sessionId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, ProRespNegotiate001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    authManager_->ProRespNegotiate(sessionId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, GenerateBindResultContent001, testing::ext::TestSize.Level0)
{
    auto ret = authManager_->GenerateBindResultContent();
    ASSERT_EQ(ret.empty(), false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnUnbindSessionOpened001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    std::string name = "nameTest";
    std::string networkId = "159753681387291";
    std::string pkgName = "com.ohos.test";
    PeerSocketInfo info = {
        .name = const_cast<char*>(name.c_str()),
        .pkgName = const_cast<char*>(pkgName.c_str()),
        .networkId = const_cast<char*>(networkId.c_str()),
    };
    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = nullptr;
    authManager_->OnUnbindSessionOpened(sessionId, info);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, DeleteGroup001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    int32_t ret = authManager_->DeleteGroup(pkgName, deviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, DeleteGroup002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string deviceId;
    int32_t ret = authManager_->DeleteGroup(pkgName, deviceId);
    ASSERT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, PutAccessControlList001, testing::ext::TestSize.Level0)
{
    authManager_->PutAccessControlList();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, BindSocketFail_001, testing::ext::TestSize.Level0)
{
    authManager_->BindSocketFail();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, BindSocketSuccess_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = std::make_shared<AuthRequestDeleteInit>();
    authManager_->BindSocketSuccess(socket);
    ASSERT_EQ(authManager_->authRequestContext_->sessionId, socket);
}

HWTEST_F(DmAuthManagerTest, SyncDeleteAcl_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string deviceId = "deviceId";
    authManager_->SyncDeleteAcl(pkgName, deviceId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 200;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 501;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 90;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 104;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_005, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 503;
    authManager_->authRequestState_ = std::make_shared<AuthRequestCredential>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_006, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 505;
    authManager_->authRequestState_ = std::make_shared<AuthRequestSyncDeleteAcl>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_007, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 1000;
    authManager_->authRequestState_ = std::make_shared<AuthRequestSyncDeleteAcl>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 80;
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 80;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 100;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 100;
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_005, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 104;
    authManager_->authResponseState_ = std::make_shared<AuthResponseShowState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_006, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 502;
    authManager_->authResponseState_ = std::make_shared<AuthResponseShowState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_007, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 504;
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_008, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 505;
    authManager_->authResponseState_ = std::make_shared<AuthResponseSyncDeleteAclNone>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_009, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 1000;
    authManager_->authResponseState_ = std::make_shared<AuthResponseSyncDeleteAclNone>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->authResponseContext_ = nullptr;
    authManager_->OnDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_002, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authMessageProcessor_ = nullptr;
    authManager_->OnDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_003, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->OnDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_004, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_MSG_TYPE] = 200;
    std::string message = jsonObject.dump();
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = std::make_shared<AuthRequestSyncDeleteAcl>();
    authManager_->authResponseState_ = nullptr;
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_005, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_MSG_TYPE] = 200;
    std::string message = jsonObject.dump();
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseSyncDeleteAclNone>();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_007, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_MSG_TYPE] = 200;
    std::string message = jsonObject.dump();
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId;
    authManager_->authResponseContext_ = nullptr;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId;
    authManager_->authResponseState_ = nullptr;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId = "{}";
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_004, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId = "groupId";
    authManager_->authResponseContext_->isShowDialog = true;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_005, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId = "groupId";
    authManager_->authResponseContext_->isShowDialog = false;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authResponseContext_ = nullptr;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authUiStateMgr_ = nullptr;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 5;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_004, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_006, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 0;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_007, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 0;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseShowState>();
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_008, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 0;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, HandleMemberJoinImportAuthCode_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 0;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->HandleMemberJoinImportAuthCode(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, HandleMemberJoinImportAuthCode_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    int32_t status = 1;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->HandleMemberJoinImportAuthCode(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, HandleMemberJoinImportAuthCode_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    int32_t status = 1;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->HandleMemberJoinImportAuthCode(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->authResponseContext_ = nullptr;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_004, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_->dmVersion = "";
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, SendAuthRequest_001, testing::ext::TestSize.Level0)
{
    int64_t sessionId = 1;
    authManager_->authResponseContext_ = nullptr;
    authManager_->SendAuthRequest(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, SendAuthRequest_002, testing::ext::TestSize.Level0)
{
    int64_t sessionId = 1;
    authManager_->authResponseContext_->cryptoSupport = true;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->deviceId = "deviceId";
    authManager_->authResponseContext_->reply = -20018;
    authManager_->SendAuthRequest(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, SendAuthRequest_003, testing::ext::TestSize.Level0)
{
    int64_t sessionId = 1;
    authManager_->authResponseContext_->cryptoSupport = true;
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->deviceId = "deviceId";
    authManager_->authResponseContext_->reply = -20018;
    authManager_->SendAuthRequest(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthRequestContext_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->localAccountId = "123456789";
    authManager_->authRequestContext_->remoteAccountId = "123456789";
    authManager_->authRequestContext_->localAccountId = "123456789";
    authManager_->authResponseContext_->localUserId = 123456789;
    authManager_->GetAuthRequestContext();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessAuthRequestExt_001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    authManager_->authResponseContext_->reply = -20018;
    authManager_->ProcessAuthRequestExt(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessAuthRequestExt_002, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    authManager_->authResponseContext_->reply = -200;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->authed = false;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->ProcessAuthRequestExt(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessAuthRequestExt_004, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    authManager_->authResponseContext_->reply = -200;
    authManager_->authResponseContext_->authType = 5;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->isAuthCodeReady = false;
    authManager_->ProcessAuthRequestExt(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcess_001, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 0;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    int32_t ret = authManager_->ConfirmProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcess_002, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 6;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    int32_t ret = authManager_->ConfirmProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcess_003, testing::ext::TestSize.Level0)
{
    int32_t action = 3;
    authManager_->action_ = 4;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    int32_t ret = authManager_->ConfirmProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcessExt_001, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 0;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->authResponseContext_->isShowDialog = false;
    int32_t ret = authManager_->ConfirmProcessExt(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcessExt_002, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 6;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->authResponseContext_->isShowDialog = true;
    int32_t ret = authManager_->ConfirmProcessExt(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcessExt_003, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 4;
    int32_t ret = authManager_->ConfirmProcessExt(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "111111";
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "00000";
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "00000";
    authManager_->authResponseContext_->reply = 0;
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_005, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "11111";
    authManager_->authResponseContext_->reply = 1;
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetConnectAddr_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    auto ret = authManager_->GetConnectAddr(deviceId);
    ASSERT_EQ(ret.empty(), true);
}

HWTEST_F(DmAuthManagerTest, GetConnectAddr_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    infoPtr->addrNum = 1;
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.insert(std::pair<std::string,
        std::shared_ptr<DeviceInfo>>(deviceId, infoPtr));
    auto ret = authManager_->GetConnectAddr(deviceId);
    ASSERT_EQ(ret.empty(), false);
}

HWTEST_F(DmAuthManagerTest, SinkAuthenticateFinish_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager_->authPtr_ = authManager_->authenticationMap_[1];
    authManager_->isFinishOfLocal_ = true;
    authManager_->SinkAuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, SrcAuthenticateFinish_001, testing::ext::TestSize.Level0)
{
    authManager_->isFinishOfLocal_ = true;
    authManager_->authResponseContext_->state = 5;
    authManager_->authPtr_ = authManager_->authenticationMap_[1];
    authManager_->SrcAuthenticateFinish();
    usleep(600);
    ASSERT_EQ(authManager_->authRequestState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, SrcAuthenticateFinish_002, testing::ext::TestSize.Level0)
{
    authManager_->isFinishOfLocal_ = false;
    authManager_->authResponseContext_->state = 7;
    authManager_->authPtr_ = authManager_->authenticationMap_[1];
    authManager_->SrcAuthenticateFinish();
    usleep(600);
    ASSERT_EQ(authManager_->authRequestState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authUiStateMgr_ = nullptr;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authUiStateMgr_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager_->authRequestState_ = nullptr;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    authManager_->authUiStateMgr_ = nullptr;
    int32_t ret = authManager_->RegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, RegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    int32_t ret = authManager_->RegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    authManager_->authUiStateMgr_ = nullptr;
    int32_t ret = authManager_->UnRegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnRegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    int32_t ret = authManager_->UnRegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetAuthRequestState_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthRequestState> authRequestState = nullptr;
    int32_t ret = authManager_->SetAuthRequestState(authRequestState);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, SetAuthRequestState_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    int32_t ret = authManager_->SetAuthRequestState(authRequestState);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ShowConfigDialog_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowConfigDialog();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ShowConfigDialog_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = false;
    authManager_->ShowConfigDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, false);
}

HWTEST_F(DmAuthManagerTest, ShowConfigDialog_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = true;
    authManager_->ShowConfigDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, true);
}

HWTEST_F(DmAuthManagerTest, ShowAuthInfoDialog_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowAuthInfoDialog();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ShowAuthInfoDialog_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = false;
    authManager_->ShowAuthInfoDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, false);
}

HWTEST_F(DmAuthManagerTest, ShowAuthInfoDialog_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = true;
    authManager_->ShowAuthInfoDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, true);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    authManager_->importAuthCode_ = "";
    authManager_->importPkgName_ = "";
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, false);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    authManager_->importAuthCode_ = "123456";
    authManager_->importPkgName_ = "hostPkgName";
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->bindLevel, 1);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    authManager_->importAuthCode_ = "123456";
    authManager_->importPkgName_ = "hostPkgName";
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->dmVersion = "";
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->bindLevel, 0);
}

HWTEST_F(DmAuthManagerTest, ProcessPincode_001, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->ProcessPincode(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, ProcessPincode_002, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->isAuthDevice_ = true;
    int32_t ret = authManager_->ProcessPincode(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, ProcessPincode_003, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->authResponseContext_->dmVersion = "";
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->authUiStateMgr_ = nullptr;
    int32_t ret = authManager_->ProcessPincode(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, AuthDevice_001, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->isAuthDevice_ = true;
    int32_t ret = authManager_->AuthDevice(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, AuthDevice_002, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->isAuthDevice_ = false;
    authManager_->authResponseContext_->authType = 5;
    int32_t ret = authManager_->AuthDevice(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, ImportAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string authCode = "";
    std::string pkgName = "pkgName";
    int32_t ret = authManager_->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ImportAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string authCode = "123456";
    std::string pkgName = "";
    int32_t ret = authManager_->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ImportAuthCode_003, testing::ext::TestSize.Level0)
{
    std::string authCode = "123456";
    std::string pkgName = "pkgName";
    int32_t ret = authManager_->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, BindTarget_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, BindTarget_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, BindTarget_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.deviceId = "123456";
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "1"));
    bindParam.insert(std::pair<std::string, std::string>("CONN_ADDR_TYPE", "3"));
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, BindTarget_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.deviceId = "";
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "2"));
    bindParam.insert(std::pair<std::string, std::string>("CONN_ADDR_TYPE", "3"));
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseConnectAddr_001, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    targetId.wifiIp = "12131415";
    targetId.brMac = "D4G4E5G2B4G";
    targetId.bleMac = "1463158131321";
    std::string deviceId;
    std::string addrType = "123456";
    int32_t ret = authManager_->ParseConnectAddr(targetId, deviceId, addrType);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ParseConnectAddr_002, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    std::string deviceId;
    std::string addrType;
    int32_t ret = authManager_->ParseConnectAddr(targetId, deviceId, addrType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_001, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_002, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", ""));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_003, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "12"));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_004, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "S"));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_005, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "1"));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ParseExtraFromMap_001, testing::ext::TestSize.Level0)
{
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("BIND_EXTRA_DATA", "12"));
    std::string ret = authManager_->ParseExtraFromMap(bindParam);
    ASSERT_EQ(ret, "12");
}

HWTEST_F(DmAuthManagerTest, ParseExtraFromMap_002, testing::ext::TestSize.Level0)
{
    std::map<std::string, std::string> bindParam;
    std::string ret = authManager_->ParseExtraFromMap(bindParam);
    ASSERT_EQ(ret, "");
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    authManager_->importAuthCode_ = "";
    authManager_->importPkgName_ = "importPkgName_";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "importPkgName_";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "ohos_test";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DmAuthManagerTest, DeleteAuthCode_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    int32_t ret = authManager_->DeleteAuthCode();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "";
    authManager_->importPkgName_ = "importPkgName_";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "importPkgName_";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "123456";
    authManager_->importPkgName_ = "ohos_test";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, IsAuthTypeSupported_001, testing::ext::TestSize.Level0)
{
    int32_t authType = 7;
    bool ret = authManager_->IsAuthTypeSupported(authType);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthTypeSupported_002, testing::ext::TestSize.Level0)
{
    int32_t authType = 7;
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    bool ret = authManager_->IsAuthTypeSupported(authType);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_001, testing::ext::TestSize.Level0)
{
    int32_t authType = 5;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->importPkgName_ = "ohos_test";
    authManager_->importAuthCode_ = "156161";
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.emplace(deviceId, infoPtr);
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartNegotiate_001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 123;
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->StartNegotiate(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
