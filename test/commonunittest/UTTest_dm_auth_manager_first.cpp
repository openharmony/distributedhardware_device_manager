/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "json_object.h"
#include "softbus_error_code.h"
#include <memory>

static bool g_reportAuthOpenSessionReturnBoolValue = false;
static bool g_reportAuthConfirmBoxReturnBoolValue = false;

constexpr const char* DM_VERSION_4_1_5_1 = "4.1.5";
constexpr const char* DM_VERSION_5_0_1 = "5.0.1";
constexpr const char* DM_VERSION_5_0_2 = "5.0.2";
constexpr const char* AUTHENTICATE_TIMEOUT_TASK = "deviceManagerTimer:authenticate";

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {

namespace {
    const std::string PINCODE = "100001";
    constexpr int32_t MIN_PIN_CODE_VALUE = 10;
    constexpr int32_t MAX_PIN_CODE_VALUE = 9999999;
    constexpr int32_t INVALID_AUTHBOX_TYPE = 100;
}

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

void DmAuthManagerTest::TearDown()
{
    Mock::VerifyAndClearExpectations(softbusSessionMock_.get());
}

void DmAuthManagerTest::SetUpTestCase()
{
    softbusSessionMock_ = std::make_shared<SoftbusSessionMock>();
    DmSoftbusSession::dmSoftbusSession = softbusSessionMock_;
    appManagerMock_ = std::make_shared<AppManagerMock>();
    DmAppManager::dmAppManager = appManagerMock_;
    cryptoMock_ = std::make_shared<CryptoMock>();
    DmCrypto::dmCrypto = cryptoMock_;
    deviceProfileConnectorMock_ = std::make_shared<DeviceProfileConnectorMock>();
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock_;
    hiChainAuthConnectorMock_ = std::make_shared<HiChainAuthConnectorMock>();
    DmHiChainAuthConnector::dmHiChainAuthConnector = hiChainAuthConnectorMock_;
    multipleUserConnectorMock_ = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    cryptoMgrMock_ = std::make_shared<CryptoMgrMock>();
    DmCryptoMgr::dmCryptoMgr = cryptoMgrMock_;
}

void DmAuthManagerTest::TearDownTestCase()
{
    DmSoftbusSession::dmSoftbusSession = nullptr;
    softbusSessionMock_ = nullptr;
    DmAppManager::dmAppManager = nullptr;
    appManagerMock_ = nullptr;
    DmCrypto::dmCrypto = nullptr;
    cryptoMock_ = nullptr;
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    deviceProfileConnectorMock_ = nullptr;
    DmHiChainAuthConnector::dmHiChainAuthConnector = nullptr;
    hiChainAuthConnectorMock_ = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
    DmCryptoMgr::dmCryptoMgr = nullptr;
    cryptoMgrMock_ = nullptr;
}

namespace {
const std::string MIN_PIN_CODE = "100000";
const std::string MAX_PIN_CODE = "999999";

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_001, testing::ext::TestSize.Level1)
{
    std::string name = "test";
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestNetworkState>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager_->authResponseContext_ = nullptr;
    authManager_->SetAuthRequestState(authRequestState);
    authManager_->HandleAuthenticateTimeout(name);
    ASSERT_TRUE(authManager_->authResponseContext_ != nullptr);
}

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_002, testing::ext::TestSize.Level1)
{
    std::string name = "test";
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    authManager_->SetAuthRequestState(authRequestState);
    authManager_->HandleAuthenticateTimeout(name);
    ASSERT_TRUE(authManager_->authRequestState_ != nullptr);
}

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_003, testing::ext::TestSize.Level1)
{
    std::string name = "test";
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    authManager_->SetAuthRequestState(authRequestState);
    authManager_->HandleAuthenticateTimeout(name);
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseFinishState>();
    authManager_->SetAuthResponseState(authResponseState);
    ASSERT_TRUE(authManager_->authRequestState_ != nullptr);
}

HWTEST_F(DmAuthManagerTest, HandleAuthenticateTimeout_004, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_001, testing::ext::TestSize.Level1)
{
    std::string deviceId1;
    authManager_->AbilityNegotiate();
    g_reportAuthOpenSessionReturnBoolValue = false;
    int32_t ret = authManager_->EstablishAuthChannel(deviceId1);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_002, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, StartAuthProcess_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    g_reportAuthConfirmBoxReturnBoolValue = false;
    authManager_->remoteVersion_ = "4.1.5.2";
    authManager_->authResponseContext_->bindLevel = USER;
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_002, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, StartAuthProcess_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    authManager_->remoteVersion_ = "4.1.5.2";
    authManager_->authResponseContext_->bindLevel = INVALIED_TYPE;
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetAuthResponseState(authResponseState);
    int32_t action = 0;
    authManager_->remoteVersion_ = "4.1.5.0";
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartAuthProcess_005, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = nullptr;
    int32_t action = 1;
    int32_t ret = authManager_->StartAuthProcess(action);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(DmAuthManagerTest, CreateGroup_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseConfirmState>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    authManager_->SetAuthResponseState(authResponseState);
    authManager_->authResponseContext_->requestId = 111;
    authManager_->authResponseContext_->groupName = "111";
    int32_t ret = authManager_->CreateGroup();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, CreateGroup_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->CreateGroup();
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, AddMember_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseInitState>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    JsonObject jsonObject;
    authManager_->authResponseContext_->groupId = "111";
    authManager_->authResponseContext_->groupName = "222";
    authManager_->authResponseContext_->code = 123;
    authManager_->authResponseContext_->requestId = 234;
    authManager_->authResponseContext_->deviceId = "234";
    std::string pinCode = "444444";
    authManager_->hiChainConnector_->RegisterHiChainCallback(authManager_);
    authManager_->SetAuthResponseState(authResponseState);
    int32_t ret = authManager_->AddMember(pinCode);
    ASSERT_NE(ret, -1);
}

HWTEST_F(DmAuthManagerTest, AddMember_002, testing::ext::TestSize.Level1)
{
    std::string pinCode = "33333";
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->AddMember(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->isAddingMember_ = true;
    ret = authManager_->AddMember(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, JoinNetwork_001, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, JoinNetwork_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->AuthenticateFinish();
    int32_t ret = authManager_->JoinNetwork();
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, SetAuthResponseState_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthResponseState> authResponseState = std::make_shared<AuthResponseFinishState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    int32_t ret = authManager_->SetAuthResponseState(authResponseState);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetAuthResponseState_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AuthResponseState> authResponseState = nullptr;
    int32_t ret = authManager_->SetAuthResponseState(authResponseState);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, GetPinCode_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->code = "123456";
    std::string code = "0";
    authManager_->GetPinCode(code);
    ASSERT_EQ(code, "123456");
}

HWTEST_F(DmAuthManagerTest, GetPinCode_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowConfigDialog();
    authManager_->ShowAuthInfoDialog();
    authManager_->ShowStartAuthDialog();
    std::string code = "0";
    int32_t ret = authManager_->GetPinCode(code);
    ASSERT_NE(ret, ERR_DM_TIME_OUT);
}

HWTEST_F(DmAuthManagerTest, SetPageId_001, testing::ext::TestSize.Level1)
{
    int32_t pageId = 123;
    int32_t ret = authManager_->SetPageId(pageId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetPageId_002, testing::ext::TestSize.Level1)
{
    int32_t pageId = 123;
    authManager_->authResponseContext_ = nullptr;
    authManager_->authMessageProcessor_ = nullptr;
    const int32_t sessionId = 65;
    std::string message = "messageTest";
    int64_t requestId = 555;
    int32_t status = 2;
    JsonObject jsonObject;
    jsonObject[TAG_MSG_TYPE] = MSG_TYPE_AUTH_BY_PIN;
    authManager_->OnMemberJoin(requestId, status);
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseContext_->sessionId = sessionId;
    message = jsonObject.Dump();
    authManager_->authResponseState_ = nullptr;
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager_->OnDataReceived(sessionId, message);
    int32_t ret = authManager_->SetPageId(pageId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetPageId_003, testing::ext::TestSize.Level1)
{
    int32_t pageId = 123;
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->SetPageId(pageId);
    ASSERT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_001, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_002, testing::ext::TestSize.Level1)
{
    int32_t reason = 1234;
    int32_t state = 5678;
    int64_t requestId = 22;
    const std::string groupId = "{}";
    authManager_->OnGroupCreated(requestId, groupId);
    int32_t ret = authManager_->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetReasonAndFinish_003, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    int32_t reason = 12;
    int32_t state = 36;
    int32_t ret = authManager_->SetReasonAndFinish(reason, state);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, IsIdenticalAccount_001, testing::ext::TestSize.Level1)
{
    bool ret = authManager_->IsIdenticalAccount();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, GetAccountGroupIdHash_001, testing::ext::TestSize.Level1)
{
    auto ret = authManager_->GetAccountGroupIdHash();
    ASSERT_EQ(ret.empty(), true);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_001, testing::ext::TestSize.Level1)
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
    std::string ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_002, testing::ext::TestSize.Level1)
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
    std::string ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_003, testing::ext::TestSize.Level1)
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
    std::string ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, GeneratePincode_004, testing::ext::TestSize.Level1)
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
    std::string ret = authManager_->GeneratePincode();
    ASSERT_LE(ret, MAX_PIN_CODE);
    ASSERT_GE(ret, MIN_PIN_CODE);
}

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = -1;
    std::string deviceId = "113456";
    std::string extra = "extraTest";
    int32_t ret = authManager_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_FAILED);

    authType = 0;
    JsonObject jsonObject;
    jsonObject["bindLevel"] = 5;
    extra = jsonObject.Dump();
    ret = authManager_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_002, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_003, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udid = "UnAuthenticateDevice_001";
    int32_t bindLevel = USER;
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

HWTEST_F(DmAuthManagerTest, GenerateGroupName_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = nullptr;
    std::string ret = authManager_->GenerateGroupName();
    ASSERT_TRUE(ret.empty());
}

HWTEST_F(DmAuthManagerTest, UnBindDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udid = "UnBindDevice_002";
    int32_t bindLevel = USER;
    std::string extra = "extraTest";
    int32_t ret = authManager_->UnBindDevice(pkgName, udid, bindLevel, extra);
    EXPECT_NE(ret, DM_OK);

    pkgName = "com.ohos.test";
    authManager_->isAuthenticateDevice_ = false;
    ret = authManager_->UnBindDevice(pkgName, udid, bindLevel, extra);
    EXPECT_NE(ret, DM_OK);

    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_ = nullptr;
    ret = authManager_->UnBindDevice(pkgName, udid, bindLevel, extra);
    EXPECT_NE(ret, DM_OK);

    bindLevel = 0;
    ret = authManager_->UnBindDevice(pkgName, udid, bindLevel, extra);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GenerateGroupName_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->targetPkgName = "targetPkgNameTest";
    authManager_->authResponseContext_->localDeviceId = "localDeviceIdTest";
    authManager_->action_ = 6;
    std::string ret = authManager_->GenerateGroupName();
    ASSERT_TRUE(!ret.empty());
}

HWTEST_F(DmAuthManagerTest, GenerateGroupName_003, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->targetPkgName = "targetPkgNameTest";
    authManager_->authResponseContext_->localDeviceId = "localDeviceIdTest";
    authManager_->action_ = 7;
    std::string ret = authManager_->GenerateGroupName();
    ASSERT_TRUE(!ret.empty());
}

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseState_ = nullptr;
    bool ret = authManager_->GetIsCryptoSupport();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->authRequestState_ = nullptr;
    bool ret = authManager_->GetIsCryptoSupport();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_003, testing::ext::TestSize.Level1)
{
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    bool ret = authManager_->GetIsCryptoSupport();
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, GetIsCryptoSupport_004, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, OnUserOperation_001, testing::ext::TestSize.Level1)
{
    int32_t action = 0;
    std::string params = "paramsTest";
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, ERR_DM_AUTH_NOT_START);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_002, testing::ext::TestSize.Level1)
{
    int32_t action = 1;
    std::string params = "paramsTest1";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_003, testing::ext::TestSize.Level1)
{
    int32_t action = 2;
    std::string params = "paramsTest2";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_004, testing::ext::TestSize.Level1)
{
    int32_t action = 3;
    std::string params = "paramsTest3";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_005, testing::ext::TestSize.Level1)
{
    int32_t action = 4;
    std::string params = "paramsTest4";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_006, testing::ext::TestSize.Level1)
{
    int32_t action = 5;
    std::string params = "5";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation_007, testing::ext::TestSize.Level1)
{
    int32_t action = 1111;
    std::string params = "paramsTest1111";
    int32_t ret = authManager_->OnUserOperation(action, params);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, RequestCredential001, testing::ext::TestSize.Level1)
{
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->RequestCredential();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, GenerateCredential001, testing::ext::TestSize.Level1)
{
    std::string publicKey = "publicKey";
    authManager_->GenerateCredential(publicKey);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, RequestCredentialDone001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->RequestCredentialDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, RequestCredentialDone002, testing::ext::TestSize.Level1)
{
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->RequestCredential();
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->RequestCredentialDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, ImportCredential001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "deviceId";
    std::string publicKey = "publicKey";
    EXPECT_CALL(*hiChainAuthConnectorMock_, ImportCredential(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = authManager_->ImportCredential(deviceId, publicKey);
    ASSERT_NE(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, ResponseCredential001, testing::ext::TestSize.Level1)
{
    authManager_->ResponseCredential();
    ASSERT_EQ(authManager_->isAuthDevice_, false);

    authManager_->authResponseContext_->publicKey = "publicKey";
    EXPECT_CALL(*hiChainAuthConnectorMock_, ImportCredential(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    authManager_->ResponseCredential();
    ASSERT_EQ(authManager_->isAuthDevice_, false);

    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    EXPECT_CALL(*hiChainAuthConnectorMock_, ImportCredential(_, _, _, _)).WillOnce(Return(DM_OK));
    authManager_->ResponseCredential();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 11;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->authResponseState_ = nullptr;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    EXPECT_CALL(*softbusSessionMock_, SendData(_, _)).WillOnce(Return(ERR_DM_FAILED));
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit003, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->authRequestState_ = nullptr;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    EXPECT_CALL(*softbusSessionMock_, SendData(_, _)).WillOnce(Return(ERR_DM_FAILED));
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceTransmit004, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    EXPECT_CALL(*softbusSessionMock_, SendData(_, _)).WillOnce(Return(ERR_DM_FAILED));
    bool ret = authManager_->AuthDeviceTransmit(requestId, data, dataLen);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish001, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, SrcAuthDeviceFinish002, testing::ext::TestSize.Level1)
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

    authManager_->authResponseContext_->isOnline = false;
    authManager_->remoteVersion_ = "5.1.1";
    authManager_->SrcAuthDeviceFinish();
    EXPECT_EQ(authManager_->authRequestContext_->reason, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, SinkAuthDeviceFinish001, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish001, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish002, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish003, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError001, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = nullptr;
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError002, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->authResponseContext_->authType = 5;
    int64_t requestId = 0;
    int32_t errorCode = -1;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError003, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, AuthDeviceError004, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->requestId = 3;
    authManager_->authTimes_ = 3;
    int64_t requestId = 0;
    int32_t errorCode = ERR_DM_FAILED;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_EQ(authManager_->authResponseContext_->state, DmAuthStatus::STATUS_DM_AUTH_DEFAULT);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError005, testing::ext::TestSize.Level1)
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
    authManager_->GetSessionKeyIdSync(requestId);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError006, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, AuthDeviceSessionKey001, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    int64_t requestId = 0;
    uint8_t *sessionKey = nullptr;
    uint32_t sessionKeyLen = 0;
    authManager_->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
    authManager_->GetSessionKeyIdSync(requestId);
}

HWTEST_F(DmAuthManagerTest, CompatiblePutAcl001, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->CompatiblePutAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, CompatiblePutAcl002, testing::ext::TestSize.Level1)
{
    authManager_->action_ = USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    authManager_->CompatiblePutAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, CompatiblePutAcl003, testing::ext::TestSize.Level1)
{
    authManager_->action_ = USER_OPERATION_TYPE_ALLOW_AUTH;
    authManager_->CompatiblePutAcl();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiateExt001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    authManager_->ProcRespNegotiateExt(sessionId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, GenerateBindResultContent001, testing::ext::TestSize.Level1)
{
    auto ret = authManager_->GenerateBindResultContent();
    ASSERT_EQ(ret.empty(), false);
}

HWTEST_F(DmAuthManagerTest, GenerateBindResultContent002, testing::ext::TestSize.Level1)
{
    authManager_->remoteDeviceId_ = "test";
    auto ret = authManager_->GenerateBindResultContent();
    ASSERT_FALSE(ret.empty());
}


HWTEST_F(DmAuthManagerTest, OnScreenLocked001, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived002, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    JsonObject jsonObject;
    jsonObject[TAG_DATA] = 123;
    std::string message = SafetyDump(jsonObject);
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived003, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    JsonObject jsonObject;
    jsonObject[TAG_DATA] = "123";
    jsonObject[TAG_DATA_LEN] = "123";
    std::string message = SafetyDump(jsonObject);
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived004, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    JsonObject jsonObject;
    jsonObject[TAG_DATA] = "123";
    jsonObject[TAG_DATA_LEN] = 123;
    jsonObject[TAG_MSG_TYPE] = "123";
    std::string message = SafetyDump(jsonObject);
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived005, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    JsonObject jsonObject;
    jsonObject[TAG_DATA] = "123";
    jsonObject[TAG_DATA_LEN] = 123;
    jsonObject[TAG_MSG_TYPE] = 123;
    std::string message = SafetyDump(jsonObject);
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, DeleteGroup001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId;
    int32_t ret = authManager_->DeleteGroup(pkgName, deviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, DeleteGroup002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string deviceId;
    int32_t ret = authManager_->DeleteGroup(pkgName, deviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, PutAccessControlList001, testing::ext::TestSize.Level1)
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

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 200;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 200;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_003, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 501;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_004, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 501;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_005, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 90;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_006, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 90;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_007, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 104;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNetworkState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_008, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 104;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_009, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 503;
    authManager_->authRequestState_ = std::make_shared<AuthRequestCredential>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_010, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 503;
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateDoneState>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSourceMsg_0012, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->msgType = 505;
    authManager_->authRequestState_ = std::make_shared<AuthRequestReCheckMsg>();
    authManager_->ProcessSourceMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 80;
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 80;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_003, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 100;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_004, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 100;
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_005, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 104;
    authManager_->authResponseState_ = std::make_shared<AuthResponseShowState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, false);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_006, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 502;
    authManager_->authResponseState_ = std::make_shared<AuthResponseShowState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessSinkMsg_007, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->msgType = 504;
    authManager_->authResponseState_ = std::make_shared<AuthResponseAuthFinish>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);

    authManager_->authResponseState_ = std::make_shared<AuthResponseShowState>();
    authManager_->ProcessSinkMsg();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ConvertSrcVersion_001, testing::ext::TestSize.Level1)
{
    std::string version = "";
    std::string edition = "test";
    EXPECT_EQ(authManager_->ConvertSrcVersion(version, edition), "test");

    edition = "";
    EXPECT_NE(authManager_->ConvertSrcVersion(version, edition), "test");

    version = "test";
    EXPECT_EQ(authManager_->ConvertSrcVersion(version, edition), "test");
}

HWTEST_F(DmAuthManagerTest, GetTaskTimeout_001, testing::ext::TestSize.Level1)
{
    int32_t taskTimeOut = 0;
    authManager_->SetAuthType(AUTH_TYPE_CRE);
    EXPECT_EQ(authManager_->GetTaskTimeout("test", taskTimeOut), taskTimeOut);

    taskTimeOut = 1000;

    authManager_->SetAuthType(AUTH_TYPE_IMPORT_AUTH_CODE);
    EXPECT_EQ(authManager_->GetTaskTimeout("test", taskTimeOut), taskTimeOut);

    authManager_->SetAuthType(AUTH_TYPE_IMPORT_AUTH_CODE);
    EXPECT_EQ(authManager_->GetTaskTimeout(AUTHENTICATE_TIMEOUT_TASK, taskTimeOut), 20);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaildExtra_001, testing::ext::TestSize.Level1)
{
    std::string extra = R"({"extra": {"bindLevel": "123"}})";
    JsonObject jsonObject;
    jsonObject["bindLevel"] = 1;
    int32_t ret = authManager_->CheckAuthParamVaildExtra(extra, "");
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    extra = SafetyDump(jsonObject);
    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(false));
    ret = authManager_->CheckAuthParamVaildExtra(extra, "");
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(true));
    ret = authManager_->CheckAuthParamVaildExtra(extra, "");
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    jsonObject["bindLevel"] = 15;
    extra = SafetyDump(jsonObject);
    ret = authManager_->CheckAuthParamVaildExtra(extra, "");
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, AuthenticateDevice_004, testing::ext::TestSize.Level1)
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

    JsonObject jsonObject;
    jsonObject[PARAM_KEY_CONN_SESSIONTYPE] = "TML";
    deviceId = "sdcwafefawe";
    extra = jsonObject.Dump();
    ret = authManager_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, StopAuthenticateDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = authManager_->StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

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
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    authManager_->GetPeerUdidHash(sessionId, peerUdidHash);

    EXPECT_CALL(*softbusSessionMock_, GetPeerDeviceId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(ERR_DM_FAILED));
    authManager_->GetPeerUdidHash(sessionId, peerUdidHash);
    ret = authManager_->StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, IsAuthFinish_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    bool ret = authManager_->IsAuthFinish();
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

    authManager_->authResponseContext_->reply = ERR_DM_AUTH_BUSINESS_BUSY;
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
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    authManager_->ProcessAuthRequestExt(sessionId);
    authManager_->authResponseContext_->reply = ERR_DM_AUTH_BUSINESS_BUSY;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->isAuthCodeReady = true;
    authManager_->ProcessAuthRequestExt(sessionId);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_101, testing::ext::TestSize.Level1)
{
    int64_t requestId = 1;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->dmVersion = "5.0.2";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->RespNegotiate(requestId);
    authManager_->authResponseContext_->dmVersion = "5.0.1";
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->RespNegotiate(requestId);
    ASSERT_NE(authManager_->authResponseContext_, nullptr);

    authManager_->authResponseContext_->edition = "";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->timer_ = std::make_shared<DmTimer>();
    authManager_->RespNegotiate(requestId);
    ASSERT_NE(authManager_->authResponseContext_, nullptr);

    authManager_->authResponseContext_->bindLevel = 5;
    authManager_->RespNegotiate(requestId);
    ASSERT_NE(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, SendAuthRequest_101, testing::ext::TestSize.Level1)
{
    int64_t sessionId = 1;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->reply = ERR_DM_VERSION_INCOMPATIBLE;
    authManager_->SendAuthRequest(sessionId);

    authManager_->authResponseContext_->reply = ERR_DM_AUTH_CODE_INCORRECT;
    authManager_->authResponseContext_->bindLevel = 5;
    ASSERT_NE(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceError_007, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->requestId = 3;
    authManager_->authTimes_ = 0;
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    int64_t requestId = authManager_->authResponseContext_->requestId;
    int32_t errorCode = DM_OK;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_NE(authManager_->authRequestState_, nullptr);

    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->authTimes_ = 5;
    authManager_->AuthDeviceError(requestId, errorCode);
    ASSERT_NE(authManager_->authResponseState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, DeleteGroup_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    int32_t userId = 0;
    std::string deviceId;
    int32_t ret = authManager_->DeleteGroup(pkgName, userId, deviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, CompatiblePutAcl_004, testing::ext::TestSize.Level1)
{
    authManager_->action_ = USER_OPERATION_TYPE_ALLOW_AUTH;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(DM_OK));
    authManager_->CompatiblePutAcl();
    ASSERT_EQ(authManager_->authRequestState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiateExt002, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authed = true;
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->authResponseContext_->bundleName = "";
    std::vector<int32_t> bindType;
    bindType.push_back(1);
    bindType.push_back(0);
    EXPECT_CALL(*multipleUserConnectorMock_, GetOhosAccountId()).WillOnce(Return("remoteAccountId"));
    EXPECT_CALL(*deviceProfileConnectorMock_, GetBindTypeByPkgName(_, _, _)).WillOnce(Return(bindType));
    authManager_->authResponseContext_->remoteAccountId = "remoteAccountId";
    authManager_->authResponseContext_->localAccountId = "remoteAccountId";
    authManager_->ProcRespNegotiateExt(sessionId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, OnAuthDeviceDataReceived006, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    JsonObject jsonObject;
    jsonObject[TAG_DATA] = "123";
    jsonObject[TAG_DATA_LEN] = 123;
    jsonObject[TAG_MSG_TYPE] = 123;
    std::string message = jsonObject.Dump();
    authManager_->authResponseContext_ = nullptr;
    authManager_->OnAuthDeviceDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, GetBinderInfo_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->bundleName = "bundleName";
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = authManager_->GetBinderInfo();
    ASSERT_EQ(ret, DM_OK);

    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*appManagerMock_, GetHapTokenIdByName(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = authManager_->GetBinderInfo();
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*appManagerMock_, GetHapTokenIdByName(_, _, _, _)).WillOnce(Return(DM_OK));
    authManager_->authResponseContext_->bindLevel = APP;
    ret = authManager_->GetBinderInfo();
    ASSERT_EQ(ret, DM_OK);

    authManager_->authResponseContext_->bindLevel = USER;
    authManager_->SetProcessInfo();

    authManager_->authResponseContext_->bindLevel = SERVICE;
    authManager_->SetProcessInfo();

    authManager_->authResponseContext_->bindLevel = APP;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    authManager_->SetProcessInfo();

    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->SetProcessInfo();

    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->SetProcessInfo();

    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->RequestReCheckMsg();
}

HWTEST_F(DmAuthManagerTest, ResponseReCheckMsg_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->edition = "edition";
    authManager_->remoteVersion_ = "";
    authManager_->ResponseReCheckMsg();

    authManager_->authResponseContext_->edition = "";
    authManager_->remoteDeviceId_ = "remoteDeviceId_";
    authManager_->authResponseContext_->localDeviceId = authManager_->remoteDeviceId_;
    authManager_->authResponseContext_->remoteUserId = 1;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->authResponseContext_->localUserId = authManager_->authResponseContext_->remoteUserId;
    authManager_->authResponseContext_->bundleName = authManager_->authResponseContext_->hostPkgName;
    authManager_->authResponseContext_->localBindLevel = authManager_->authResponseContext_->bindLevel;
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).Times(::testing::AtLeast(1))
        .WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*appManagerMock_, GetHapTokenIdByName(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(DM_OK));
    authManager_->ResponseReCheckMsg();
    ASSERT_EQ(authManager_->authResponseContext_->localBindLevel, 1);
}

HWTEST_F(DmAuthManagerTest, RequestReCheckMsgDone_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseContext_->edition = "edition";
    authManager_->remoteVersion_ = "";
    authManager_->RequestReCheckMsgDone();

    authManager_->authResponseContext_->edition = "";
    authManager_->remoteDeviceId_ = "remoteDeviceId_";
    authManager_->authResponseContext_->localDeviceId = authManager_->remoteDeviceId_;
    authManager_->authRequestContext_->remoteUserId = 1;
    authManager_->authResponseContext_->peerBundleName = "peerBundleName";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->authResponseContext_->localUserId = authManager_->authRequestContext_->remoteUserId;
    authManager_->authResponseContext_->bundleName = authManager_->authResponseContext_->peerBundleName;
    authManager_->authResponseContext_->localBindLevel = authManager_->authResponseContext_->bindLevel;
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(DM_OK));
    authManager_->RequestReCheckMsgDone();
    ASSERT_EQ(authManager_->authResponseContext_->localBindLevel, 1);

    authManager_->ConverToFinish();
}

HWTEST_F(DmAuthManagerTest, SinkAuthDeviceFinish_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->softbusConnector_->RegisterSoftbusStateCallback(std::make_shared<SoftbusStateCallbackTest>());
    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authResponseContext_->haveCredential = false;
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->bindLevel = 3;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->isNeedProcCachedSrcReqMsg_ = true;
    authManager_->srcReqMsg_ = "srcReqMsg";
    authManager_->SinkAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);

    authManager_->authResponseContext_->haveCredential = true;
    JsonObject jsonObject;
    jsonObject["MSG_TYPE"] = MSG_TYPE_REQ_RECHECK_MSG;
    authManager_->srcReqMsg_ = jsonObject.Dump();
    authManager_->remoteVersion_ = "4.0.1";
    authManager_->SinkAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);

    authManager_->remoteVersion_ = "5.1.2";
    authManager_->SinkAuthDeviceFinish();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDeviceFinish004, testing::ext::TestSize.Level1)
{
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    int64_t requestId = 0;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->remoteVersion_ = "5.0.2";
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(DM_OK));
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);

    if (authManager_->authMessageProcessor_ == nullptr) {
        authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    }
    authManager_->remoteVersion_ = "5.1.2";
    authManager_->AuthDeviceFinish(requestId);
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, RequestCredentialDone_003, testing::ext::TestSize.Level1)
{
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->RequestCredential();
    authManager_->authRequestState_ = std::make_shared<AuthRequestNegotiateState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->publicKey = "";
    authManager_->RequestCredentialDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);

    authManager_->authResponseContext_->publicKey = "publicKey";
    EXPECT_CALL(*hiChainAuthConnectorMock_, ImportCredential(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    authManager_->RequestCredentialDone();
    ASSERT_EQ(authManager_->isAuthDevice_, false);
}

HWTEST_F(DmAuthManagerTest, AuthDevice_003, testing::ext::TestSize.Level1)
{
    std::string pinCode = "123456";
    authManager_->isAuthDevice_ = false;
    authManager_->authResponseContext_->authType = 5;
    EXPECT_CALL(*hiChainAuthConnectorMock_, AuthDevice(_, _, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = authManager_->AuthDevice(pinCode);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnBindDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udid = "udid";
    int32_t bindLevel = 1;
    std::string extra;
    int32_t ret = authManager_->UnBindDevice(pkgName, udid, bindLevel, extra);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    pkgName = "pkgName";
    DmOfflineParam offlineParam;
    offlineParam.bindType = INVALIED_TYPE;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    ret = authManager_->UnBindDevice(pkgName, udid, bindLevel, extra);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, DeleteAcl_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string localUdid = "localUdid";
    std::string remoteUdid = "remoteUdid";
    int32_t bindLevel = 1;
    std::string extra = "extra";
    DmOfflineParam offlineParam;
    offlineParam.bindType = APP_PEER_TO_PEER_TYPE;
    offlineParam.leftAclNumber = 1;
    authManager_->softbusConnector_->deviceStateManagerCallback_ = std::make_shared<SoftbusStateCallbackTest>();
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    int32_t ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    ASSERT_EQ(ret, DM_OK);

    offlineParam.leftAclNumber = 0;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    ASSERT_EQ(ret, DM_OK);

    bindLevel = 3;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    ASSERT_EQ(ret, DM_OK);

    offlineParam.leftAclNumber = 1;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    ASSERT_EQ(ret, DM_OK);

    bindLevel = 2;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    bindLevel = 1;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    ASSERT_EQ(ret, DM_OK);

    offlineParam.leftAclNumber = 0;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAccessControlList(_, _, _, _, _)).WillOnce(Return(offlineParam));
    ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StopAuthenticateDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestContext_->hostPkgName = pkgName;
    authManager_->authResponseContext_->hostPkgName = pkgName;
    authManager_->isAuthenticateDevice_ = true;
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    int32_t ret = authManager_->StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, DM_OK);

    authManager_->authRequestContext_->hostPkgName = "";
    ret = authManager_->StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, DM_OK);

    JsonObject jsonObject;
    jsonObject["PEER_BUNDLE_NAME"] = "";
    authManager_->authRequestContext_->hostPkgName = "hostPkgName";
    authManager_->ParseJsonObject(jsonObject);

    int32_t sessionId = 1;
    authManager_->remoteUdidHash_ = "remoteUdidhash";
    std::string udidHashTemp = "remoteUdidhash";
    EXPECT_CALL(*softbusSessionMock_, GetPeerDeviceId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(1))
    .WillOnce(WithArgs<1>(Invoke([udidHashTemp](unsigned char *udidHash) {
        memcpy_s(udidHash, (udidHashTemp.length() + 1), udidHashTemp.c_str(), (udidHashTemp.length()));
        return DM_OK;
    })));
    authManager_->DeleteOffLineTimer(sessionId);

    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    sessionId = 1;
    std::string message;
    authManager_->authResponseContext_->sessionId = sessionId;
    JsonObject jsonObject1;
    jsonObject1[TAG_MSG_TYPE] = 800;
    message = jsonObject1.Dump();
    authManager_->authResponseState_ = nullptr;
    authManager_->OnDataReceived(sessionId, message);

    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseNegotiateState>();
    authManager_->OnDataReceived(sessionId, message);
}

HWTEST_F(DmAuthManagerTest, RegisterAuthenticationType_001, testing::ext::TestSize.Level1)
{
    int32_t authenticationType = 1;
    int32_t ret = authManager_->RegisterAuthenticationType(authenticationType);
    ASSERT_EQ(ret, DM_OK);

    authenticationType = 0;
    ret = authManager_->RegisterAuthenticationType(authenticationType);
    ASSERT_EQ(ret, DM_OK);

    authManager_->authResponseState_ = std::make_shared<AuthResponseReCheckMsg>();
    authManager_->ProcessReqPublicKey();

    authManager_->authResponseState_ = std::make_shared<AuthResponseAuthFinish>();
    authManager_->ProcessReqPublicKey();
}

HWTEST_F(DmAuthManagerTest, CheckProcessNameInWhiteList_001, testing::ext::TestSize.Level1)
{
    std::string processName = "";
    bool ret = authManager_->CheckProcessNameInWhiteList(processName);
    ASSERT_FALSE(ret);

    processName = "processName";
    ret = authManager_->CheckProcessNameInWhiteList(processName);
    ASSERT_FALSE(ret);

    processName = "com.example.myapplication";
    ret = authManager_->CheckProcessNameInWhiteList(processName);
    ASSERT_TRUE(ret);
}

HWTEST_F(DmAuthManagerTest, GetCloseSessionDelaySeconds_001, testing::ext::TestSize.Level1)
{
    std::string delaySecondsStr = "";
    int32_t ret = authManager_->GetCloseSessionDelaySeconds(delaySecondsStr);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GetTokenIdByBundleName_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 1;
    std::string bundleName = "b********Info";
    int64_t tokenId = 0;
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = authManager_->GetTokenIdByBundleName(userId, bundleName, tokenId);
    ASSERT_EQ(ret, DM_OK);

    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*appManagerMock_, GetHapTokenIdByName(_, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = authManager_->GetTokenIdByBundleName(userId, bundleName, tokenId);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*appManagerMock_, GetHapTokenIdByName(_, _, _, _)).WillOnce(Return(DM_OK));
    ret = authManager_->GetTokenIdByBundleName(userId, bundleName, tokenId);
    ASSERT_EQ(ret, DM_OK);

    std::string deviceId = "de*******8";
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestContext_->connSessionType = CONN_SESSION_TYPE_HML;
    authManager_->JoinLnn(deviceId, false);

    int32_t errorCode = 0;
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->UpdateInputPincodeDialog(errorCode);

    authManager_->authResponseContext_->authType = AUTH_TYPE_NFC;
    errorCode = ERR_DM_HICHAIN_PROOFMISMATCH;
    authManager_->pincodeDialogEverShown_ = false;
    authManager_->authRequestContext_->hostPkgName = "hostPkgName";
    authManager_->importAuthCode_ = "14785";
    authManager_->importPkgName_ = "hostPkgName";
    authManager_->UpdateInputPincodeDialog(errorCode);
}

HWTEST_F(DmAuthManagerTest, CheckNeedShowAuthInfoDialog_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    int32_t errorCode = ERR_DM_HICHAIN_PROOFMISMATCH;
    authManager_->pincodeDialogEverShown_ = false;
    authManager_->authResponseContext_->authType = AUTH_TYPE_NFC;
    authManager_->serviceInfoProfile_.SetPinCode(PINCODE);
    authManager_->serviceInfoProfile_.SetPinExchangeType(
        static_cast<int32_t>(DMLocalServiceInfoPinExchangeType::FROMDP));
    bool ret = authManager_->CheckNeedShowAuthInfoDialog(errorCode);
    ASSERT_TRUE(ret);

    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    ret = authManager_->CheckNeedShowAuthInfoDialog(errorCode);
    ASSERT_FALSE(ret);

    EXPECT_CALL(*deviceProfileConnectorMock_, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));
    authManager_->GetLocalServiceInfoInDp();

    DistributedDeviceProfile::LocalServiceInfo localServiceInfo;
    localServiceInfo.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ONETIME));
    localServiceInfo.SetAuthBoxType(static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::STATE3));
    localServiceInfo.SetPinExchangeType(static_cast<int32_t>(DMLocalServiceInfoPinExchangeType::FROMDP));
    localServiceInfo.SetPinCode(PINCODE);
    EXPECT_CALL(*deviceProfileConnectorMock_, GetLocalServiceInfoByBundleNameAndPinExchangeType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(localServiceInfo), Return(DM_OK)));
    authManager_->GetLocalServiceInfoInDp();

    int64_t requestId = 1;
    uint8_t arrayPtr[] = {1, 2, 3, 4};
    uint8_t *sessionKey = arrayPtr;
    uint32_t sessionKeyLen = static_cast<uint32_t>(sizeof(arrayPtr) / sizeof(arrayPtr[0]));
    authManager_->authResponseContext_->requestId = 1;
    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    EXPECT_CALL(*cryptoMgrMock_, SaveSessionKey(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*deviceProfileConnectorMock_, PutSessionKey(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(1), Return(DM_OK)));
    authManager_->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
    authManager_->GetSessionKeyIdSync(requestId);
}

HWTEST_F(DmAuthManagerTest, IsPinCodeValid_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowConfigDialog();

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->serviceInfoProfile_.SetAuthBoxType(
        static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM));

    authManager_->authResponseContext_->authType = AUTH_TYPE_NFC;
    authManager_->serviceInfoProfile_.SetPinCode(PINCODE);
    authManager_->serviceInfoProfile_.SetPinExchangeType(
        static_cast<int32_t>(DMLocalServiceInfoPinExchangeType::FROMDP));
    authManager_->ShowConfigDialog();

    authManager_->authResponseContext_->isShowDialog = false;
    authManager_->serviceInfoProfile_.SetPinExchangeType(
        static_cast<int32_t>(DMLocalServiceInfoPinExchangeType::PINBOX));
    authManager_->ShowConfigDialog();

    authManager_->authResponseContext_->isShowDialog = true;
    authManager_->ShowConfigDialog();

    ASSERT_FALSE(authManager_->IsPinCodeValid(MIN_PIN_CODE_VALUE));
    ASSERT_FALSE(authManager_->IsPinCodeValid(MAX_PIN_CODE_VALUE));
    ASSERT_TRUE(authManager_->IsPinCodeValid(PINCODE));
}

HWTEST_F(DmAuthManagerTest, IsPinCodeValid_002, testing::ext::TestSize.Level1)
{
    std::string strPin = "";
    ASSERT_FALSE(authManager_->IsPinCodeValid(strPin));
    strPin = "pinCode";
    ASSERT_FALSE(authManager_->IsPinCodeValid(strPin));
}

HWTEST_F(DmAuthManagerTest, IsServiceInfoAuthTypeValid_001, testing::ext::TestSize.Level1)
{
    int32_t authType = 2;
    ASSERT_FALSE(authManager_->IsServiceInfoAuthTypeValid(authType));
}

HWTEST_F(DmAuthManagerTest, IsServiceInfoAuthBoxTypeValid_001, testing::ext::TestSize.Level1)
{
    int32_t authBoxType = 3;
    ASSERT_FALSE(authManager_->IsServiceInfoAuthBoxTypeValid(authBoxType));
}

HWTEST_F(DmAuthManagerTest, IsServiceInfoPinExchangeTypeValid_001, testing::ext::TestSize.Level1)
{
    int32_t pinExchangeType = 4;
    ASSERT_FALSE(authManager_->IsServiceInfoPinExchangeTypeValid(pinExchangeType));
}

HWTEST_F(DmAuthManagerTest, IsLocalServiceInfoValid_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo profile;
    profile.SetAuthType(2);
    ASSERT_FALSE(authManager_->IsLocalServiceInfoValid(profile));

    profile.SetAuthType(static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ONETIME));
    profile.SetAuthBoxType(3);
    ASSERT_FALSE(authManager_->IsLocalServiceInfoValid(profile));

    profile.SetAuthBoxType(static_cast<int32_t>(DMLocalServiceInfoAuthBoxType::STATE3));
    profile.SetPinExchangeType(4);
    ASSERT_FALSE(authManager_->IsLocalServiceInfoValid(profile));

    profile.SetPinExchangeType(static_cast<int32_t>(DMLocalServiceInfoPinExchangeType::FROMDP));
    profile.SetPinCode("");
    ASSERT_TRUE(authManager_->IsLocalServiceInfoValid(profile));

    profile.SetPinCode(PINCODE);
    ASSERT_TRUE(authManager_->IsLocalServiceInfoValid(profile));
}

HWTEST_F(DmAuthManagerTest, EstablishAuthChannel_003, testing::ext::TestSize.Level1)
{
    std::string deviceId = "d********3";
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestContext_->connSessionType = CONN_SESSION_TYPE_HML;
    int32_t ret = authManager_->EstablishAuthChannel(deviceId);
    ASSERT_EQ(ret, DM_OK);

    JsonObject jsonObject;
    jsonObject[PARAM_KEY_CONN_SESSIONTYPE] = "param_key_conn_sessionType";
    jsonObject[PARAM_KEY_HML_ENABLE_160M] = true;
    jsonObject[PARAM_KEY_HML_ACTIONID] = 0;
    authManager_->ParseHmlInfoInJsonObject(jsonObject);
}

HWTEST_F(DmAuthManagerTest, ParseHmlInfoInJsonObject_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_CONN_SESSIONTYPE] = CONN_SESSION_TYPE_HML;
    jsonObject[PARAM_KEY_HML_ACTIONID] = 0;
    authManager_->ParseHmlInfoInJsonObject(jsonObject);
    ASSERT_EQ(authManager_->authRequestContext_->hmlActionId, 0);

    jsonObject[PARAM_KEY_HML_ACTIONID] = 1;
    authManager_->ParseHmlInfoInJsonObject(jsonObject);
    ASSERT_EQ(authManager_->authRequestContext_->hmlActionId, 0);

    jsonObject[PARAM_KEY_HML_ACTIONID] = "1";
    authManager_->ParseHmlInfoInJsonObject(jsonObject);
    ASSERT_EQ(authManager_->authRequestContext_->hmlActionId, 1);

    authManager_->authRequestContext_->hmlActionId = 0;
    jsonObject[PARAM_KEY_CONN_SESSIONTYPE] = CONN_SESSION_TYPE_BLE;
    jsonObject[PARAM_KEY_HML_ACTIONID] = "1";
    authManager_->ParseHmlInfoInJsonObject(jsonObject);
    ASSERT_EQ(authManager_->authRequestContext_->hmlActionId, 0);
}

HWTEST_F(DmAuthManagerTest, CanUsePincodeFromDp_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo info;
    info.SetAuthBoxType((int32_t)DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM);
    info.SetAuthType((int32_t)DMLocalServiceInfoAuthType::TRUST_ONETIME);
    info.SetPinExchangeType((int32_t)DMLocalServiceInfoPinExchangeType::FROMDP);
    info.SetPinCode("******");
    authManager_->serviceInfoProfile_ = info;
    ASSERT_FALSE(authManager_->CanUsePincodeFromDp());

    info.SetPinCode("123456");
    authManager_->serviceInfoProfile_ = info;
    ASSERT_TRUE(authManager_->CanUsePincodeFromDp());

    info.SetAuthBoxType(INVALID_AUTHBOX_TYPE);
    info.SetPinCode("123456");
    authManager_->serviceInfoProfile_ = info;
    ASSERT_TRUE(authManager_->CanUsePincodeFromDp());

    info.SetPinExchangeType((int32_t)DMLocalServiceInfoPinExchangeType::ULTRASOUND);
    info.SetPinCode("123456");
    authManager_->serviceInfoProfile_ = info;
    ASSERT_FALSE(authManager_->CanUsePincodeFromDp());
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaildExtra_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_CONN_SESSIONTYPE] = CONN_SESSION_TYPE_HML;
    jsonObject[PARAM_KEY_HML_ENABLE_160M] = true;
    jsonObject[PARAM_KEY_HML_ACTIONID] = "kwjewkkl";
    std::string deviceId = "de*************12";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    authManager_->softbusConnector_->AddMemberToDiscoverMap(deviceId, deviceInfo);
    std::string strExtra = jsonObject.Dump();
    int32_t ret = authManager_->CheckAuthParamVaildExtra(strExtra, deviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    jsonObject[PARAM_KEY_HML_ACTIONID] = 0;
    strExtra = jsonObject.Dump();
    ret = authManager_->CheckAuthParamVaildExtra(strExtra, deviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    jsonObject[PARAM_KEY_HML_ACTIONID] = 1;
    strExtra = jsonObject.Dump();
    ret = authManager_->CheckAuthParamVaildExtra(strExtra, deviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    jsonObject[PARAM_KEY_HML_ACTIONID] = "1";
    jsonObject[TAG_BIND_LEVEL] = 1;
    strExtra = jsonObject.Dump();
    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(true));
    ret = authManager_->CheckAuthParamVaildExtra(strExtra, deviceId);
    ASSERT_EQ(ret, DM_OK);

    EXPECT_CALL(*appManagerMock_, IsSystemSA()).WillOnce(Return(false));
    ret = authManager_->CheckAuthParamVaildExtra(strExtra, deviceId);
    ASSERT_EQ(ret, DM_OK);

    std::string pkgName = "pkgName_pickerProxy_Info";
    int32_t authType = 1;
    std::string extra = "extra";
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    EXPECT_CALL(*appManagerMock_, GetNativeTokenIdByName(_, _)).WillOnce(Return(DM_OK));
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
}

HWTEST_F(DmAuthManagerTest, CheckHmlParamValid_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_HML_ACTIONID] = 1;
    bool ret = authManager_->CheckHmlParamValid(jsonObject);
    EXPECT_FALSE(ret);

    jsonObject[PARAM_KEY_HML_ACTIONID] = "kjsdkad";
    ret = authManager_->CheckHmlParamValid(jsonObject);
    EXPECT_FALSE(ret);

    jsonObject[PARAM_KEY_HML_ACTIONID] = "0";
    ret = authManager_->CheckHmlParamValid(jsonObject);
    EXPECT_FALSE(ret);

    jsonObject[PARAM_KEY_HML_ACTIONID] = "1";
    ret = authManager_->CheckHmlParamValid(jsonObject);
    EXPECT_TRUE(ret);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
