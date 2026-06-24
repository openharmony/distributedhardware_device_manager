/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UTTest_auth_manager_cred.h"
#include "device_manager_service_listener_3rd.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {

void AuthManagerCredTest::SetUpTestCase()
{
    LOGI("SetUpTestCase start");
}

void AuthManagerCredTest::TearDownTestCase()
{
    LOGI("TearDownTestCase start");
}

void AuthManagerCredTest::SetUp()
{
    LOGI("SetUp start");
    softbusConnector = std::make_shared<SoftbusConnector3rd>();
    listener = std::make_shared<DeviceManagerServiceListener3rd>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector3rd>();
}

void AuthManagerCredTest::TearDown()
{
    LOGI("TearDown start");
    softbusConnector = nullptr;
    listener = nullptr;
    hiChainAuthConnector = nullptr;
    authManagerCred = nullptr;
    authSrcManagerCred = nullptr;
    authSinkManagerCred = nullptr;
    context = nullptr;
}

HWTEST_F(AuthManagerCredTest, AuthManagerCredConstructor_001, testing::ext::TestSize.Level1)
{
    authManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    EXPECT_NE(authManagerCred, nullptr);
}

HWTEST_F(AuthManagerCredTest, AuthManagerCredDestructor_001, testing::ext::TestSize.Level1)
{
    authManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authManagerCred = nullptr;
    EXPECT_EQ(authManagerCred, nullptr);
}

HWTEST_F(AuthManagerCredTest, AuthDevice3rd_001, testing::ext::TestSize.Level1)
{
    authManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    std::map<std::string, std::string> authParam;
    int32_t sessionId = 1;
    uint64_t logicalSessionId = 12345;
    int32_t ret = authManagerCred->AuthDevice3rd(targetId, authParam, sessionId, logicalSessionId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(AuthManagerCredTest, AuthCredential_001, testing::ext::TestSize.Level1)
{
    authManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BIND_CALLER_UID] = "100";
    authParam[TAG_BIND_CALLER_TOKENID] = "123";
    authParam[TAG_BIND_CALLER_PROCESSNAME] = "testProcess";
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    int32_t sessionId = 1;
    uint64_t logicalSessionId = 12345;
    int32_t ret = authManagerCred->AuthCredential(targetId, authParam, sessionId, logicalSessionId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredConstructor_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    EXPECT_NE(authSrcManagerCred, nullptr);
}

HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredGetIsCryptoSupport_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    bool ret = authSrcManagerCred->GetIsCryptoSupport();
    EXPECT_EQ(ret, false);
}

HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceTransmit_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    int64_t requestId = 12345;
    uint8_t data[] = {1, 2, 3, 4, 5};
    uint32_t dataLen = 5;
    bool ret = authSrcManagerCred->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, false);
}

HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceRequest_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    int64_t requestId = 12345;
    int operationCode = 0;
    const char *reqParams = "test";
    char *ret = authSrcManagerCred->AuthDeviceRequest(requestId, operationCode, reqParams);
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredConstructor_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    EXPECT_NE(authSinkManagerCred, nullptr);
}

HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredGetIsCryptoSupport_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    bool ret = authSinkManagerCred->GetIsCryptoSupport();
    EXPECT_EQ(ret, false);
}

HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceTransmit_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    int64_t requestId = 12345;
    uint8_t data[] = {1, 2, 3, 4, 5};
    uint32_t dataLen = 5;
    bool ret = authSinkManagerCred->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, false);
}

HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceRequest_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    int64_t requestId = 12345;
    int operationCode = 0;
    const char *reqParams = "test";
    char *ret = authSinkManagerCred->AuthDeviceRequest(requestId, operationCode, reqParams);
    EXPECT_NE(ret, nullptr);
    if (ret != nullptr) {
        free(ret);
    }
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceRequest_002
 * @tc.desc: Cover AuthDeviceRequest branch where context_->accesser.openIdHash is non-empty so that
 *           the TAG_IS_OPEN_CRED_AUTH=true member is added to the response JSON.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceRequest_002, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    int32_t sessionId = 1;
    uint64_t logicalSessionId = 12345;
    authSinkManagerCred->AuthCredential(targetId, authParam, sessionId, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    int operationCode = 0;
    const char *reqParams = "test";
    char *ret = authSinkManagerCred->AuthDeviceRequest(requestId, operationCode, reqParams);
    EXPECT_NE(ret, nullptr);
    if (ret != nullptr) {
        free(ret);
    }
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceTransmit_002
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceTransmit success path where requestId matches
 *           context_->requestId (set by AuthCredential) and data is non-null so it returns true.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceTransmit_002, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    uint64_t logicalSessionId = 12345;
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    uint8_t data[] = {1, 2, 3, 4, 5};
    uint32_t dataLen = 5;
    bool ret = authSrcManagerCred->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceTransmit_003
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceTransmit null-data guard returning false even when
 *           requestId matches context_->requestId.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceTransmit_003, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    uint64_t logicalSessionId = 12345;
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    bool ret = authSrcManagerCred->AuthDeviceTransmit(requestId, nullptr, 0);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceTransmit_004
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceTransmit requestId-mismatch guard returning false
 *           when the supplied requestId differs from context_->requestId.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceTransmit_004, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    int64_t requestId = 99999;
    uint8_t data[] = {1, 2, 3, 4, 5};
    uint32_t dataLen = 5;
    bool ret = authSrcManagerCred->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceTransmit_002
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceTransmit success path where requestId matches
 *           context_->requestId so it returns true.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceTransmit_002, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    uint64_t logicalSessionId = 12345;
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    uint8_t data[] = {1, 2, 3, 4, 5};
    uint32_t dataLen = 5;
    bool ret = authSinkManagerCred->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceTransmit_003
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceTransmit requestId-mismatch guard returning false
 *           when the supplied requestId differs from context_->requestId.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceTransmit_003, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    int64_t requestId = 99999;
    uint8_t data[] = {1, 2, 3, 4, 5};
    uint32_t dataLen = 5;
    bool ret = authSinkManagerCred->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceSessionKey_001
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceSessionKey success path where requestId matches and
 *           sessionKey is non-null so the accesser sessionKey is filled and event notified.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceSessionKey_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    uint64_t logicalSessionId = 12345;
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    uint8_t sessionKey[] = {1, 2, 3, 4, 5};
    authSrcManagerCred->AuthDeviceSessionKey(requestId, sessionKey, sizeof(sessionKey));
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceSessionKey_002
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceSessionKey requestId-mismatch early-return guard.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceSessionKey_002, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    uint8_t sessionKey[] = {1, 2, 3, 4, 5};
    authSrcManagerCred->AuthDeviceSessionKey(99999, sessionKey, sizeof(sessionKey));
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceSessionKey_003
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceSessionKey null-sessionKey early-return guard.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceSessionKey_003, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    uint64_t logicalSessionId = 12345;
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    authSrcManagerCred->AuthDeviceSessionKey(requestId, nullptr, 0);
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceSessionKey_001
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceSessionKey success path where requestId matches and
 *           sessionKey is non-null so the accessee sessionKey is filled.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceSessionKey_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    uint64_t logicalSessionId = 12345;
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    uint8_t sessionKey[] = {1, 2, 3, 4, 5};
    authSinkManagerCred->AuthDeviceSessionKey(requestId, sessionKey, sizeof(sessionKey));
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceSessionKey_002
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceSessionKey requestId-mismatch early-return guard.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceSessionKey_002, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    uint8_t sessionKey[] = {1, 2, 3, 4, 5};
    authSinkManagerCred->AuthDeviceSessionKey(99999, sessionKey, sizeof(sessionKey));
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceSessionKey_003
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceSessionKey null-sessionKey early-return guard.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceSessionKey_003, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    uint64_t logicalSessionId = 12345;
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, logicalSessionId);
    int64_t requestId = static_cast<int64_t>(logicalSessionId);
    authSinkManagerCred->AuthDeviceSessionKey(requestId, nullptr, 0);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceFinish_001
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceFinish path that notifies the ON_FINISH event.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceFinish_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    authSrcManagerCred->AuthDeviceFinish(12345);
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceFinish_001
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceFinish path that notifies the ON_FINISH event.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceFinish_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    authSinkManagerCred->AuthDeviceFinish(12345);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceError_001
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceError requestId-mismatch early-return guard.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceError_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    authSrcManagerCred->AuthDeviceError(99999, ERR_DM_FAILED);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceError_002
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceError proofmismatch branch that maps the error code
 *           to ERR_DM_BIND_PIN_CODE_ERROR before notifying ON_FAIL.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceError_002, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    authSrcManagerCred->AuthDeviceError(12345, ERR_DM_HICHAIN_PROOFMISMATCH);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredAuthDeviceError_003
 * @tc.desc: Cover AuthSrcManagerCred::AuthDeviceError non-proofmismatch branch where the errorCode
 *           is used as-is as the context reason.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredAuthDeviceError_003, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSrcManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    authSrcManagerCred->AuthDeviceError(12345, ERR_DM_FAILED);
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceError_001
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceError requestId-mismatch early-return guard.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceError_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    authSinkManagerCred->AuthDeviceError(99999, ERR_DM_FAILED);
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredAuthDeviceError_002
 * @tc.desc: Cover AuthSinkManagerCred::AuthDeviceError success path that records the errorCode as
 *           the context reason and notifies ON_FAIL.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthDeviceError_002, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "testBusiness";
    authSinkManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    authSinkManagerCred->AuthDeviceError(12345, ERR_DM_FAILED);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredOnSessionOpened_001
 * @tc.desc: Cover AuthSrcManagerCred::OnSessionOpened logging-only callback execution.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredOnSessionOpened_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSrcManagerCred->OnSessionOpened(1, 0, DM_OK);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredOnSessionClosed_001
 * @tc.desc: Cover AuthSrcManagerCred::OnSessionClosed path that sets reason to ERR_DM_SESSION_CLOSED
 *           and transitions to the source finish state.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredOnSessionClosed_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSrcManagerCred->OnSessionClosed(1);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredOnSessionDisable_001
 * @tc.desc: Cover AuthSrcManagerCred::OnSessionDisable path that resets context_->sessionId to -1.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredOnSessionDisable_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSrcManagerCred->OnSessionDisable();
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredOnDataReceived_001
 * @tc.desc: Cover AuthSrcManagerCred::OnDataReceived parse-failure branch that sets reason to
 *           ERR_DM_PARSE_MESSAGE_FAILED and transitions to the source finish state.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredOnDataReceived_001, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSrcManagerCred->OnDataReceived(1, "");
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredOnSessionOpened_001
 * @tc.desc: Cover AuthSinkManagerCred::OnSessionOpened logging-only callback execution.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredOnSessionOpened_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSinkManagerCred->OnSessionOpened(1, 1, DM_OK);
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredOnSessionClosed_001
 * @tc.desc: Cover AuthSinkManagerCred::OnSessionClosed path that sets reason to ERR_DM_SESSION_CLOSED
 *           and transitions to the sink finish state.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredOnSessionClosed_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSinkManagerCred->OnSessionClosed(1);
    SUCCEED();
}

/**
 * @tc.name: AuthSinkManagerCredOnDataReceived_001
 * @tc.desc: Cover AuthSinkManagerCred::OnDataReceived parse-failure branch that sets reason to
 *           ERR_DM_PARSE_MESSAGE_FAILED and transitions to the sink finish state.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredOnDataReceived_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSinkManagerCred->OnDataReceived(1, "");
    SUCCEED();
}

/**
 * @tc.name: AuthCredential_002
 * @tc.desc: Cover AuthCredential with an empty authParam map, exercising the short-circuit that
 *           skips the invalid-param check and returns DM_OK.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthCredential_002, testing::ext::TestSize.Level1)
{
    authManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    int32_t sessionId = 1;
    uint64_t logicalSessionId = 12345;
    int32_t ret = authManagerCred->AuthCredential(targetId, authParam, sessionId, logicalSessionId);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthManagerCredRegisterCleanNotifyCallback_001
 * @tc.desc: Cover AuthManagerCred::RegisterCleanNotifyCallback path that stores the supplied
 *           callback into context_->cleanNotifyCallback.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthManagerCredRegisterCleanNotifyCallback_001, testing::ext::TestSize.Level1)
{
    authManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    CleanNotifyCallback callback = [](uint64_t logicalSessionId, int32_t result, ProcessInfo3rd processInfo) {};
    authManagerCred->RegisterCleanNotifyCallback(callback);
    SUCCEED();
}

/**
 * @tc.name: AuthCredential_003
 * @tc.desc: Cover AuthCredential exercising the full GetAuthParam branch set including
 *           TAG_PEER_PROCESS_NAME, TAG_PEER_BUSINESS_NAME, TAG_OPENID, TAG_OWNER_ID and
 *           TAG_CRED_TYPE, all of which are valid normal inputs.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthCredential_003, testing::ext::TestSize.Level1)
{
    authManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "testDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BIND_CALLER_UID] = "100";
    authParam[TAG_BIND_CALLER_TOKENID] = "123";
    authParam[TAG_BIND_CALLER_PROCESSNAME] = "callerProcess";
    authParam[TAG_BUSINESS_NAME] = "callerBusiness";
    authParam[TAG_PEER_PROCESS_NAME] = "peerProcess";
    authParam[TAG_PEER_BUSINESS_NAME] = "peerBusiness";
    authParam[TAG_OPENID] = "peerOpenId";
    authParam[TAG_OWNER_ID] = "owner123";
    authParam[TAG_CRED_TYPE] = "1";
    int32_t ret = authManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthSinkManagerCredAuthCredential_001
 * @tc.desc: Cover AuthSinkManagerCred::AuthCredential valid path on the sink side that initializes
 *           the sink auth state machine and returns DM_OK.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredAuthCredential_001, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    PeerTargetId3rd targetId;
    targetId.deviceId = "sinkDeviceId";
    std::map<std::string, std::string> authParam;
    authParam[TAG_BUSINESS_NAME] = "sinkBusiness";
    int32_t ret = authSinkManagerCred->AuthCredential(targetId, authParam, 1, 12345);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthSinkManagerCredOnSessionOpened_002
 * @tc.desc: Cover AuthSinkManagerCred::OnSessionOpened logging-only callback with a non-zero result,
 *           confirming the method returns normally for normal inputs.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSinkManagerCredOnSessionOpened_002, testing::ext::TestSize.Level1)
{
    authSinkManagerCred = std::make_shared<AuthSinkManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSinkManagerCred->OnSessionOpened(2, 1, ERR_DM_FAILED);
    SUCCEED();
}

/**
 * @tc.name: AuthSrcManagerCredOnSessionOpened_002
 * @tc.desc: Cover AuthSrcManagerCred::OnSessionOpened logging-only callback with a non-zero result,
 *           confirming the method returns normally for normal inputs.
 * @tc.type: FUNC
 * @tc.require: AR000H59TL
 */
HWTEST_F(AuthManagerCredTest, AuthSrcManagerCredOnSessionOpened_002, testing::ext::TestSize.Level1)
{
    authSrcManagerCred = std::make_shared<AuthSrcManagerCred>(softbusConnector, listener, hiChainAuthConnector);
    authSrcManagerCred->OnSessionOpened(3, 1, ERR_DM_FAILED);
    SUCCEED();
}
}
}