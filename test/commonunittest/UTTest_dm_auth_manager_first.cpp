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

#include "UTTest_dm_auth_manager_first.h"

#include "nlohmann/json.hpp"

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_auth_manager.h"
#include "auth_message_processor.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {

class SoftbusStateCallbackTest : public ISoftbusStateCallback {
public:
    SoftbusStateCallbackTest() {}
    virtual ~SoftbusStateCallbackTest() {}
    void OnDeviceOnline(std::string deviceId) {}
    void OnDeviceOffline(std::string deviceId) {}
};

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
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->isAuthenticateDevice_ = false;
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, networkId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string networkId = "222";
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->isAuthenticateDevice_ = true;
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, networkId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::string networkId = "222";
    authManager_->authRequestState_ = std::make_shared<AuthRequestReplyState>();
    authManager_->isAuthenticateDevice_ = false;
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, networkId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_005, testing::ext::TestSize.Level0)
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
    int32_t ret = authManager_->SetAuthResponseState(authResponseState);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetAuthResponseState_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = nullptr;
    int32_t ret = authManager_->SetAuthResponseState(authResponseState);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
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

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish002, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->softbusConnector_->RegisterSoftbusStateCallback(std::make_shared<SoftbusStateCallbackTest>());
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->confirmOperation = 0;
    authManager_->authResponseContext_->haveCredential = true;
    authManager_->authResponseContext_->bindLevel = 3;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->SrcAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish003, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->softbusConnector_->RegisterSoftbusStateCallback(std::make_shared<SoftbusStateCallbackTest>());
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->confirmOperation = 6;
    authManager_->authResponseContext_->haveCredential = true;
    authManager_->authResponseContext_->bindLevel = 3;
    authManager_->SrcAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish004, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->softbusConnector_->RegisterSoftbusStateCallback(std::make_shared<SoftbusStateCallbackTest>());
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->confirmOperation = 6;
    authManager_->authResponseContext_->haveCredential = false;
    authManager_->authResponseContext_->bindLevel = 3;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->SrcAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish005, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->softbusConnector_->RegisterSoftbusStateCallback(std::make_shared<SoftbusStateCallbackTest>());
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->haveCredential = true;
    authManager_->SrcAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish006, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->softbusConnector_->RegisterSoftbusStateCallback(std::make_shared<SoftbusStateCallbackTest>());
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->haveCredential = false;
    authManager_->SrcAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, SinkAuthDeviceFinish001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->softbusConnector_->RegisterSoftbusStateCallback(std::make_shared<SoftbusStateCallbackTest>());
    authManager_->authResponseContext_->haveCredential = false;
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->bindLevel = 3;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->SinkAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish002, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish003, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError001, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError002, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 5;
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError003, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->requestId = 2;
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

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived002, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_DATA] = 123;
    std::string message = jsonObject.dump();
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived003, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_DATA] = "123";
    jsonObject[TAG_DATA_LEN] = "123";
    std::string message = jsonObject.dump();
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived004, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_DATA] = "123";
    jsonObject[TAG_DATA_LEN] = 123;
    jsonObject[TAG_MSG_TYPE] = "123";
    std::string message = jsonObject.dump();
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived005, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_DATA] = "123";
    jsonObject[TAG_DATA_LEN] = 123;
    jsonObject[TAG_MSG_TYPE] = 123;
    std::string message = jsonObject.dump();
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
    authManager_->timer_ = nullptr;
    authManager_->OnUnbindSessionOpened(sessionId, info);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnUnbindSessionOpened002, testing::ext::TestSize.Level0)
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
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestDeleteInit>();
    authManager_->timer_ = nullptr;
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

HWTEST_F(DmAuthManagerTest, BindSocketSuccess_002, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->authRequestState_ = nullptr;
    authManager_->BindSocketSuccess(socket);
    ASSERT_NE(authManager_->authRequestContext_->sessionId, socket);
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
    authManager_->authResponseContext_->msgType = 200;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 501;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 501;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_005, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 90;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_006, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 90;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_007, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 104;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_008, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 104;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_009, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 503;
    authManager_->authRequestState_ = std::make_shared<AuthRequestCredential>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_010, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 503;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_0011, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 505;
    authManager_->authRequestState_ = std::make_shared<AuthRequestSyncDeleteAcl>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_0012, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 505;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_013, testing::ext::TestSize.Level0)
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
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
