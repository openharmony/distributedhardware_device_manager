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

#include "auth_message_processor.h"
#include "common_event_support.h"
#include "device_manager_service_listener.h"
#include "dm_auth_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "nlohmann/json.hpp"
#include "softbus_error_code.h"

static bool g_reportAuthOpenSessionReturnBoolValue = false;
static bool g_reportAuthConfirmBoxReturnBoolValue = false;

constexpr const char* DM_VERSION_4_1_5_1 = "4.1.5";
constexpr const char* DM_VERSION_5_0_1 = "5.0.1";
constexpr const char* DM_VERSION_5_0_2 = "5.0.2";

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
const int32_t CLONE_AUTHENTICATE_TIMEOUT = 10;

bool DmRadarHelper::ReportAuthOpenSession(struct RadarInfo &info)
{
    return g_reportAuthOpenSessionReturnBoolValue;
}

bool DmRadarHelper::ReportAuthConfirmBox(struct RadarInfo &info)
{
    return g_reportAuthConfirmBoxReturnBoolValue;
}

class SoftbusStateCallbackTest : public ISoftbusStateCallback {
public:
    SoftbusStateCallbackTest() {}
    virtual ~SoftbusStateCallbackTest() {}
    void OnDeviceOnline(std::string deviceId, int32_t authForm) {}
    void OnDeviceOffline(std::string deviceId) {}
    void DeleteOffLineTimer(std::string udidHash) {}
};

void DmAuthManagerTest::SetUp()
{
    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authMessageProcessor_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->softbusConnector_ = std::make_shared<SoftbusConnector>();
    authManager_->softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(
        std::shared_ptr<ISoftbusSessionCallback>(authManager_));
    authManager_->timer_ = std::make_shared<DmTimer>();
}
void DmAuthManagerTest::TearDown() {}
void DmAuthManagerTest::SetUpTestCase()
{
    softbusSessionMock_ = std::make_shared<SoftbusSessionMock>();
    DmSoftbusSession::dmSoftbusSession = softbusSessionMock_;
    appManagerMock_ = std::make_shared<AppManagerMock>();
    DmAppManager::dmAppManager = appManagerMock_;
}
void DmAuthManagerTest::TearDownTestCase()
{
    DmSoftbusSession::dmSoftbusSession = nullptr;
    softbusSessionMock_ = nullptr;
    DmAppManager::dmAppManager = nullptr;
    appManagerMock_ = nullptr;
}

namespace {
const int32_t MIN_PIN_CODE = 100000;
const int32_t MAX_PIN_CODE = 999999;

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

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_003, testing::ext::TestSize.Level0)
{
    std::string name = "test";
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    authManager_->SetAuthRequestState(authRequestState);
    authManager_->HandleAuthenticateTimeout(name);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseFinishState>();
    authManager_->SetAuthResponseState(authResponseState);
    ASSERT_TRUE(authManager_->authRequestState_ != nullptr);
}

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_004, testing::ext::TestSize.Level0)
{
    std::string name = "test";
    authManager_->authRequestContext_->reason = DM_OK;
    std::shared_ptr<AuthRequestInitStateMock> requestInitState = std::make_shared<AuthRequestInitStateMock>();
    EXPECT_CALL(*requestInitState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_NEGOTIATE));
    authManager_->authRequestState_ = requestInitState;
    authManager_->HandleAuthenticateTimeout(name);
    EXPECT_EQ(authManager_->authRequestContext_->reason, ERR_DM_TIME_OUT);

    std::shared_ptr<AuthResponseInitStateMock> authResponseInitState = std::make_shared<AuthResponseInitStateMock>();
    EXPECT_CALL(*authResponseInitState, GetStateType())
        .WillRepeatedly(testing::Return(AuthState::AUTH_RESPONSE_FINISH));
    authManager_->authResponseState_ = authResponseInitState;
    authManager_->HandleAuthenticateTimeout(name);
    EXPECT_EQ(authManager_->authRequestContext_->reason, ERR_DM_TIME_OUT);

    authManager_->authResponseState_ = nullptr;
    authManager_->HandleAuthenticateTimeout(name);
    EXPECT_EQ(authManager_->authRequestContext_->reason, ERR_DM_TIME_OUT);
}

HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_001, testing::ext::TestSize.Level0)
{
    std::string deviceId1;
    authManager_->AbilityNegotiate();
    g_reportAuthOpenSessionReturnBoolValue = false;
    int32_t ret = authManager_->EstablishAuthChannel(deviceId1);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_002, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    authManager_->authResponseContext_ = nullptr;
    authManager_->authRequestContext_ = nullptr;
    authManager_->authRequestState_ = nullptr;
    g_reportAuthOpenSessionReturnBoolValue = true;
    int32_t ret = authManager_->EstablishAuthChannel(deviceId);
    g_reportAuthOpenSessionReturnBoolValue = false;
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    g_reportAuthConfirmBoxReturnBoolValue = false;
    authManager_->remoteVersion_ = "4.1.5.2";
    authManager_->authResponseContext_->bindLevel = DEVICE;
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    g_reportAuthConfirmBoxReturnBoolValue = true;
    authManager_->remoteVersion_ = "4.1.5.2";
    authManager_->authResponseContext_->bindLevel = APP + 1;
    int32_t ret = authManager_->StartAuthProcess(action);
    g_reportAuthConfirmBoxReturnBoolValue = false;
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    authManager_->remoteVersion_ = "4.1.5.2";
    authManager_->authResponseContext_->bindLevel = INVALIED_TYPE;
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    authManager_->remoteVersion_ = "4.1.5.0";
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_005, testing::ext::TestSize.Level0)
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
    authManager_->isAddingMember_ = true;
    ret = authManager_->AddMember(pinCode);
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
    authManager_->AuthenticateFinish();
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
    int32_t code = 0;
    authManager_->GetPinCode(code);
    ASSERT_EQ(code, 123456);
}

HWTEST_F(DmAuthManagerTest, GetPinCode_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowConfigDialog();
    authManager_->ShowAuthInfoDialog();
    authManager_->ShowStartAuthDialog();
    int32_t code = 0;
    int32_t ret = authManager_->GetPinCode(code);
    ASSERT_NE(code, ERR_DM_TIME_OUT);
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
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager_->OnDataReceived(sessionId, message);
    int32_t ret = authManager_->SetPageId(pageId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetPageId_003, testing::ext::TestSize.Level0)
{
    int32_t pageId = 123;
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->SetPageId(pageId);
    ASSERT_NE(ret, DM_OK);
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

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string udid = "UnAuthenticateDevice_001";
    int32_t bindLevel = DEVICE;
    int32_t ret = authManager_->UnAuthenticateDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);

    pkgName = "com.ohos.test";
    authManager_->isAuthenticateDevice_ = false;
    ret = authManager_->UnAuthenticateDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);

    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_ = nullptr;
    ret = authManager_->UnAuthenticateDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);

    bindLevel = 0;
    ret = authManager_->UnAuthenticateDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnBindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string udid = "UnAuthenticateDevice_001";
    int32_t bindLevel = DEVICE;
    int32_t ret = authManager_->UnBindDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);

    pkgName = "com.ohos.test";
    authManager_->isAuthenticateDevice_ = false;
    ret = authManager_->UnBindDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);

    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_ = nullptr;
    ret = authManager_->UnBindDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);

    bindLevel = 0;
    ret = authManager_->UnBindDevice(pkgName, udid, bindLevel);
    EXPECT_NE(ret, DM_OK);
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

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_004, testing::ext::TestSize.Level0)
{
    authManager_->isCryptoSupport_ = true;
    authManager_->authRequestState_ = nullptr;
    std::shared_ptr<AuthResponseInitStateMock> authResponseInitState = std::make_shared<AuthResponseInitStateMock>();
    EXPECT_CALL(*authResponseInitState, GetStateType())
        .WillRepeatedly(testing::Return(AuthState::AUTH_RESPONSE_FINISH));
    authManager_->authResponseState_ = authResponseInitState;
    bool ret = authManager_->GetIsCryptoSupport();
    ASSERT_TRUE(ret);

    EXPECT_CALL(*authResponseInitState, GetStateType())
        .WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_NEGOTIATE_DONE));
    ret = authManager_->GetIsCryptoSupport();
    ASSERT_FALSE(ret);

    std::shared_ptr<AuthRequestFinishStateMock> requestFinishState = std::make_shared<AuthRequestFinishStateMock>();
    EXPECT_CALL(*requestFinishState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_NEGOTIATE));
    authManager_->authRequestState_ = requestFinishState;
    ret = authManager_->GetIsCryptoSupport();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*requestFinishState, GetStateType())
        .WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_NEGOTIATE_DONE));
    ret = authManager_->GetIsCryptoSupport();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*requestFinishState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_INIT));
    ret = authManager_->GetIsCryptoSupport();
    ASSERT_TRUE(ret);
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

HWTEST_F(DmAuthManagerTest, ResponseCredential001, testing::ext::TestSize.Level0)
{
    authManager_->ResponseCredential();
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
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->confirmOperation = USER_OPERATION_TYPE_ALLOW_AUTH;
    authManager_->authResponseContext_->haveCredential = true;
    authManager_->authResponseContext_->bindLevel = APP;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->softbusConnector_->deviceStateManagerCallback_ = std::make_shared<SoftbusStateCallbackTest>();
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);

    authManager_->authResponseContext_->confirmOperation = USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    authManager_->authResponseContext_->hostPkgName = "";
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);

    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);

    authManager_->authResponseContext_->bindLevel = SERVICE;
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);

    authManager_->authResponseContext_->haveCredential = false;
    authManager_->authResponseContext_->bindLevel = APP;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);

    authManager_->authResponseContext_->hostPkgName = "";
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);

    authManager_->authResponseContext_->bindLevel = SERVICE;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);

    authManager_->authResponseContext_->confirmOperation = USER_OPERATION_TYPE_DONE_PINCODE_INPUT;
    authManager_->SrcAuthDeviceFinish();
    EXPECT_TRUE(authManager_->softbusConnector_->processInfoVec_.size() > 0);
}

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->confirmOperation = USER_OPERATION_TYPE_ALLOW_AUTH;
    authManager_->authResponseContext_->haveCredential = true;
    authManager_->authResponseContext_->bindLevel = APP;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->softbusConnector_->deviceStateManagerCallback_ = std::make_shared<SoftbusStateCallbackTest>();
    authManager_->SrcAuthDeviceFinish();
    EXPECT_EQ(authManager_->authRequestContext_->reason, DM_OK);

    authManager_->authRequestContext_->reason = ERR_DM_FAILED;
    authManager_->authResponseContext_->haveCredential = false;
    authManager_->SrcAuthDeviceFinish();
    EXPECT_EQ(authManager_->authRequestContext_->reason, ERR_DM_FAILED);
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
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError002, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
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
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->requestId = 2;
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError004, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->requestId = 3;
    authManager_->authTimes_ = 3;
    int64_t requestId = 0;
    int32_t errorCode = ERR_DM_FAILED;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->authResponseContext_->state, AuthState::AUTH_REQUEST_JOIN);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError005, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->requestId = 3;
    authManager_->authTimes_ = 0;
    int64_t requestId = authManager_->authResponseContext_->requestId + 1;
    int32_t errorCode = DM_OK;
    uint32_t sessionKeyLen = 0;
    authManager_->AuthDeviceError(requestId, errorCode);
    authManager_->AuthDeviceSessionKey(requestId, nullptr, sessionKeyLen);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError006, testing::ext::TestSize.Level0)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->requestId = 3;
    authManager_->authTimes_ = 0;
    int64_t requestId = authManager_->authResponseContext_->requestId;
    int32_t errorCode = DM_OK;
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

HWTEST_F(DmAuthManagerTest, CompatiblePutAcl002, testing::ext::TestSize.Level0)
{
    authManager_->action_ = USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    authManager_->CompatiblePutAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, CompatiblePutAcl003, testing::ext::TestSize.Level0)
{
    authManager_->action_ = USER_OPERATION_TYPE_ALLOW_AUTH;
    authManager_->CompatiblePutAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiateExt001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    authManager_->ProcRespNegotiateExt(sessionId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, GenerateBindResultContent001, testing::ext::TestSize.Level0)
{
    auto ret = authManager_->GenerateBindResultContent();
    ASSERT_EQ(ret.empty(), false);
}

HWTEST_F(DmAuthManagerTest, GenerateBindResultContent002, testing::ext::TestSize.Level0)
{
    authManager_->remoteDeviceId_ = "test";
    auto ret = authManager_->GenerateBindResultContent();
    ASSERT_FALSE(ret.empty());
}


HWTEST_F(DmAuthManagerTest, OnScreenLocked001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->authRequestState_ = nullptr;
    authManager_->OnScreenLocked();

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_->authType = AUTH_TYPE_CRE;
    authManager_->OnScreenLocked();

    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->OnScreenLocked();

    authManager_->authResponseContext_->authType = AUTH_REQUEST_INIT;
    std::shared_ptr<AuthRequestFinishStateMock> requestFinishState = std::make_shared<AuthRequestFinishStateMock>();
    EXPECT_CALL(*requestFinishState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_NEGOTIATE));
    authManager_->authRequestState_ = requestFinishState;
    authManager_->OnScreenLocked();

    EXPECT_CALL(*requestFinishState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_INIT));
    authManager_->OnScreenLocked();

    EXPECT_CALL(*requestFinishState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_JOIN));
    authManager_->OnScreenLocked();

    EXPECT_CALL(*requestFinishState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_JOIN));
    authManager_->authUiStateMgr_ = nullptr;
    authManager_->OnScreenLocked();

    EXPECT_CALL(*requestFinishState, GetStateType())
        .WillRepeatedly(testing::Return(AuthState::AUTH_REQUEST_NEGOTIATE_DONE));
    authManager_->OnScreenLocked();

    EXPECT_CALL(*requestFinishState, GetStateType()).WillRepeatedly(testing::Return(AuthState::AUTH_RESPONSE_FINISH));
    authManager_->OnScreenLocked();
    EXPECT_EQ(authManager_->authResponseContext_->state, STATUS_DM_AUTH_DEFAULT);
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
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->PutAccessControlList();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->PutAccessControlList();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->localAccountId = "ohosAnonymousUid";
    authManager_->authResponseContext_->confirmOperation = USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    authManager_->PutAccessControlList();
    ASSERT_EQ(authManager_->authResponseContext_->isIdenticalAccount, false);
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

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_0012, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->msgType = 505;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
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

HWTEST_F(DmAuthManagerTest, ConvertSrcVersion_001, testing::ext::TestSize.Level0)
{
    std::string version = "";
    std::string edition = "test";
    EXPECT_EQ(authManager_->ConvertSrcVersion(version, edition), "test");

    edition = "";
    EXPECT_NE(authManager_->ConvertSrcVersion(version, edition), "test");

    version = "test";
    EXPECT_EQ(authManager_->ConvertSrcVersion(version, edition), "test");
}

HWTEST_F(DmAuthManagerTest, GetTaskTimeout_001, testing::ext::TestSize.Level0)
{
    int32_t taskTimeOut = 0;
    authManager_->SetAuthType(AUTH_TYPE_CRE);
    EXPECT_EQ(authManager_->GetTaskTimeout("test", taskTimeOut), taskTimeOut);

    taskTimeOut = 1000;

    authManager_->SetAuthType(AUTH_TYPE_IMPORT_AUTH_CODE);
    EXPECT_EQ(authManager_->GetTaskTimeout("test", taskTimeOut), taskTimeOut);

    authManager_->SetAuthType(AUTH_TYPE_IMPORT_AUTH_CODE);
    EXPECT_EQ(authManager_->GetTaskTimeout(AUTHENTICATE_TIMEOUT_TASK, taskTimeOut), CLONE_AUTHENTICATE_TIMEOUT);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaildExtra_001, testing::ext::TestSize.Level0)
{
    std::string extra = R"({"extra": {"bindLevel": "123"}})";
    nlohmann::json jsonObject;
    jsonObject["bindLevel"] = 1;
    int32_t ret = authManager_->CheckAuthParamVaildExtra(extra);
    EXPECT_EQ(ret, DM_OK);

    extra = jsonObject.dump();
    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(false));
    ret = authManager_->CheckAuthParamVaildExtra(extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(true));
    ret = authManager_->CheckAuthParamVaildExtra(extra);
    EXPECT_EQ(ret, DM_OK);

    jsonObject["bindLevel"] = 15;
    extra = jsonObject.dump();
    ret = authManager_->CheckAuthParamVaildExtra(extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::string extra = R"({"extra": {"bindLevel": 789}})";
    std::string pkgName = "ohos_test_004";
    std::string deviceId = "512156";
    authManager_->importPkgName_ = "ohos_test_004";
    authManager_->importAuthCode_ = "156161";
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    int32_t ret = authManager_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StopAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t ret = authManager_->StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    pkgName = "pkgName_001";
    int64_t requestId = 12;
    int32_t status = 0;
    int32_t sessionId = 1;
    std::string peerUdidHash;
    if (authManager_->timer_ == nullptr) {
        authManager_->timer_ = std::make_shared<DmTimer>();
    }
    authManager_->authRequestState_ = std::make_shared<AuthRequestInitState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestContext_->hostPkgName = pkgName;
    authManager_->authResponseContext_->hostPkgName = pkgName;
    authManager_->isAuthenticateDevice_ = true;

    authManager_->authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->MemberJoinAuthRequest(requestId, status);
    authManager_->authResponseContext_->authType == AUTH_TYPE_NFC;
    authManager_->authResponseContext_->requestId = requestId;
    status = 1;
    authManager_->authTimes_ = 3;
    authManager_->MemberJoinAuthRequest(requestId, status);
    status = 0;
    authManager_->authTimes_ = 2;
    authManager_->MemberJoinAuthRequest(requestId, status);
    status = 0;
    authManager_->HandleMemberJoinImportAuthCode(requestId, status);
    authManager_->NegotiateRespMsg(DM_VERSION_5_0_1);
    authManager_->NegotiateRespMsg(DM_VERSION_4_1_5_1);
    authManager_->NegotiateRespMsg(DM_VERSION_5_0_2);
    EXPECT_CALL(*softbusSessionMock_, GetPeerDeviceId(_, _)).WillOnce(Return(DM_OK));
    authManager_->GetPeerUdidHash(sessionId, peerUdidHash);
    ret = authManager_->StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GetBindLevel_001, testing::ext::TestSize.Level0)
{
    int32_t bindLevel = INVALIED_TYPE;
    std::string udid;
    authManager_->HandleDeviceNotTrust(udid);
    udid = "988989";
    authManager_->HandleDeviceNotTrust(udid);
    int32_t sessionId = 32166;
    authManager_->ProcIncompatible(sessionId);
    
    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(true));
    int32_t ret = authManager_->GetBindLevel(bindLevel);
    ASSERT_EQ(ret, DEVICE);

    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(false));
    ret = authManager_->GetBindLevel(bindLevel);
    ASSERT_EQ(ret, APP);

    authManager_->authResponseContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->importAuthCode_ = "importAuthCode";
    authManager_->ProcessAuthRequest(sessionId);

    authManager_->authResponseContext_->authType == AUTH_TYPE_NFC;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->reply = 0;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->isAuthCodeReady = true;
    authManager_->ProcessAuthRequest(sessionId);

    authManager_->authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->isAuthCodeReady == false;
    authManager_->ProcessAuthRequest(sessionId);

    bindLevel = SERVICE;
    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(false));
    ret = authManager_->GetBindLevel(bindLevel);
    ASSERT_EQ(ret, SERVICE);
}

HWTEST_F(DmAuthManagerTest, IsAuthFinish_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    bool ret = authManager_->IsAuthFinish();
    EXPECT_TRUE(ret);

    authManager_->authResponseContext_->reply = ERR_DM_AUTH_BUSINESS_BUSY;
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->authed = true;
    ret = authManager_->IsAuthFinish();
    EXPECT_TRUE(ret);

    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->authed = false;
    ret = authManager_->IsAuthFinish();
    EXPECT_TRUE(ret);

    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->isAuthCodeReady = false;
    ret = authManager_->IsAuthFinish();
    EXPECT_TRUE(ret);

    authManager_->authResponseContext_->isAuthCodeReady = true;
    ret = authManager_->IsAuthFinish();
    EXPECT_FALSE(ret);

    int32_t sessionId = 1;
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->importAuthCode_= "importAuthCode";
    authManager_->ProcessAuthRequestExt(sessionId);

    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->authed = true;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->ProcessAuthRequestExt(sessionId);
    authManager_->authResponseContext_->reply = ERR_DM_AUTH_BUSINESS_BUSY;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->isAuthCodeReady = true;
    authManager_->ProcessAuthRequestExt(sessionId);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
