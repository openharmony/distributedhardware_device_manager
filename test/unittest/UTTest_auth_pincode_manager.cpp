/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UTTest_auth_pincode_manager.h"

#include "auth_pincode_manager_3rd.h"
#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_message_processor_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_constants_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "idevice_manager_service_listener_3rd.h"
#include "device_manager_service_listener_3rd.h"
#include "softbus_connector_3rd.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

namespace {
std::shared_ptr<SoftbusConnector3rd> mockSoftbusConnector_;
std::shared_ptr<IDeviceManagerServiceListener3rd> mockListener_;
std::shared_ptr<HiChainAuthConnector3rd> mockHiChainAuthConnector_;
}

void AuthPincodeManagerTest::SetUpTestCase()
{
    LOGI("AuthPincodeManagerTest::SetUpTestCase");
}

void AuthPincodeManagerTest::TearDownTestCase()
{
    LOGI("AuthPincodeManagerTest::TearDownTestCase");
}

void AuthPincodeManagerTest::SetUp()
{
    LOGI("AuthPincodeManagerTest::SetUp");
    mockSoftbusConnector_ = std::make_shared<SoftbusConnector3rd>();
    mockListener_ = std::make_shared<DeviceManagerServiceListener3rd>();
    mockHiChainAuthConnector_ = std::make_shared<HiChainAuthConnector3rd>();
}

void AuthPincodeManagerTest::TearDown()
{
    LOGI("AuthPincodeManagerTest::TearDown");
    mockSoftbusConnector_ = nullptr;
    mockListener_ = nullptr;
    mockHiChainAuthConnector_ = nullptr;
}

/**
 * @tc.name: AuthPincodeSrcManager_001
 * @tc.desc: Test AuthPincodeSrcManager construction and initialization
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);
    EXPECT_EQ(srcManager->GetIsCryptoSupport(), false);
}

/**
 * @tc.name: AuthPincodeSinkManager_001
 * @tc.desc: Test AuthPincodeSinkManager construction and initialization
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSinkManager_001, TestSize.Level1)
{
    auto sinkManager = std::make_shared<AuthPincodeSinkManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(sinkManager, nullptr);
    EXPECT_EQ(sinkManager->GetIsCryptoSupport(), false);
}

/**
 * @tc.name: AuthPincodeSrcManager_AuthPincode_001
 * @tc.desc: Test AuthPincode with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_AuthPincode_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";

    std::map<std::string, std::string> authParam;
    authParam[TAG_BIND_CALLER_PROCESSNAME] = "test_process";
    authParam[TAG_BUSINESS_NAME] = "test_business";
    authParam[TAG_BIND_CALLER_UID] = "1001";
    authParam[TAG_BIND_CALLER_TOKENID] = "12345";
    authParam[TAG_BIND_CALLER_BIND_LEVEL] = "1";

    int32_t sessionId = 100;
    uint64_t logicalSessionId = 12345;

    int32_t ret = srcManager->AuthPincode(targetId, authParam, sessionId, logicalSessionId);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeSinkManager_AuthPincode_001
 * @tc.desc: Test AuthPincode Sink with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSinkManager_AuthPincode_001, TestSize.Level1)
{
    auto sinkManager = std::make_shared<AuthPincodeSinkManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";

    std::map<std::string, std::string> authParam;
    authParam[TAG_BIND_CALLER_PROCESSNAME] = "test_process";
    authParam[TAG_BUSINESS_NAME] = "test_business";
    authParam[TAG_BIND_CALLER_UID] = "1001";
    authParam[TAG_BIND_CALLER_TOKENID] = "12345";

    int32_t sessionId = 100;
    uint64_t logicalSessionId = 12345;

    int32_t ret = sinkManager->AuthPincode(targetId, authParam, sessionId, logicalSessionId);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeManager_ImportAuthCodeAndUid_001
 * @tc.desc: Test ImportAuthCodeAndUid with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeManager_ImportAuthCodeAndUid_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);

    std::string businessName = "test_business";
    std::string authCode = "123456";
    int32_t uid = 1001;

    int32_t ret = srcManager->ImportAuthCodeAndUid(businessName, authCode, uid);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthPincodeManager_ImportAuthCodeAndUid_002
 * @tc.desc: Test ImportAuthCodeAndUid with empty parameters
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeManager_ImportAuthCodeAndUid_002, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);

    std::string businessName = "";
    std::string authCode = "";
    int32_t uid = 1001;

    int32_t ret = srcManager->ImportAuthCodeAndUid(businessName, authCode, uid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: AuthPincodeSrcManager_OnSessionOpened_001
 * @tc.desc: Test OnSessionOpened callback
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_OnSessionOpened_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);

    int32_t sessionId = 100;
    int32_t sessionSide = 1;
    int32_t result = DM_OK;

    srcManager->OnSessionOpened(sessionId, sessionSide, result);
}

/**
 * @tc.name: AuthPincodeSinkManager_OnSessionOpened_001
 * @tc.desc: Test Sink OnSessionOpened callback
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSinkManager_OnSessionOpened_001, TestSize.Level1)
{
    auto sinkManager = std::make_shared<AuthPincodeSinkManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(sinkManager, nullptr);

    int32_t sessionId = 100;
    int32_t sessionSide = 1;
    int32_t result = DM_OK;

    sinkManager->OnSessionOpened(sessionId, sessionSide, result);
}

/**
 * @tc.name: AuthPincodeSrcManager_OnSessionClosed_001
 * @tc.desc: Test OnSessionClosed callback
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_OnSessionClosed_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);

    int32_t sessionId = 100;
    srcManager->OnSessionClosed(sessionId);
}

/**
 * @tc.name: AuthPincodeSinkManager_OnSessionClosed_001
 * @tc.desc: Test Sink OnSessionClosed callback
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSinkManager_OnSessionClosed_001, TestSize.Level1)
{
    auto sinkManager = std::make_shared<AuthPincodeSinkManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(sinkManager, nullptr);

    int32_t sessionId = 100;
    sinkManager->OnSessionClosed(sessionId);
}

/**
 * @tc.name: AuthPincodeSrcManager_OnSessionDisable_001
 * @tc.desc: Test OnSessionDisable callback
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_OnSessionDisable_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);

    srcManager->OnSessionDisable();
}

/**
 * @tc.name: AuthPincodeSrcManager_AuthDeviceTransmit_001
 * @tc.desc: Test AuthDeviceTransmit with valid data
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_AuthDeviceTransmit_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";
    std::map<std::string, std::string> authParam;
    srcManager->AuthPincode(targetId, authParam, 100, 12345);

    int64_t requestId = 12345;
    std::string testData = "test_data";
    const uint8_t *data = reinterpret_cast<const uint8_t*>(testData.c_str());
    uint32_t dataLen = testData.length();

    bool ret = srcManager->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: AuthPincodeSrcManager_AuthDeviceTransmit_002
 * @tc.desc: Test AuthDeviceTransmit with invalid request ID
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_AuthDeviceTransmit_002, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);

    int64_t requestId = 99999;
    std::string testData = "test_data";
    const uint8_t *data = reinterpret_cast<const uint8_t*>(testData.c_str());
    uint32_t dataLen = testData.length();

    bool ret = srcManager->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: AuthPincodeSinkManager_AuthDeviceTransmit_001
 * @tc.desc: Test Sink AuthDeviceTransmit with valid data
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSinkManager_AuthDeviceTransmit_001, TestSize.Level1)
{
    auto sinkManager = std::make_shared<AuthPincodeSinkManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";
    std::map<std::string, std::string> authParam;
    sinkManager->AuthPincode(targetId, authParam, 100, 12345);

    int64_t requestId = 12345;
    std::string testData = "test_data";
    const uint8_t *data = reinterpret_cast<const uint8_t*>(testData.c_str());
    uint32_t dataLen = testData.length();

    bool ret = sinkManager->AuthDeviceTransmit(requestId, data, dataLen);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: AuthPincodeSrcManager_AuthDeviceError_001
 * @tc.desc: Test AuthDeviceError callback
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_AuthDeviceError_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";
    std::map<std::string, std::string> authParam;
    int32_t ret = srcManager->AuthPincode(targetId, authParam, 100, 12345);
    EXPECT_EQ(ret, DM_OK);

    int64_t requestId = 12345;
    int32_t errorCode = ERR_DM_FAILED;

    srcManager->AuthDeviceError(requestId, errorCode);
}

/**
 * @tc.name: AuthPincodeSrcManager_AuthDeviceError_002
 * @tc.desc: Test AuthDeviceError with proof mismatch error
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_AuthDeviceError_002, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";
    std::map<std::string, std::string> authParam;
    int32_t ret = srcManager->AuthPincode(targetId, authParam, 100, 12345);
    EXPECT_EQ(ret, DM_OK);

    int64_t requestId = 12345;
    int32_t errorCode = ERR_DM_HICHAIN_PROOFMISMATCH;

    srcManager->AuthDeviceError(requestId, errorCode);
}

/**
 * @tc.name: AuthPincodeSrcManager_AuthDeviceFinish_001
 * @tc.desc: Test AuthDeviceFinish callback
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_AuthDeviceFinish_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";
    std::map<std::string, std::string> authParam;
    int32_t ret = srcManager->AuthPincode(targetId, authParam, 100, 12345);
    EXPECT_EQ(ret, DM_OK);

    int64_t requestId = 12345;
    srcManager->AuthDeviceFinish(requestId);
}

/**
 * @tc.name: AuthPincodeSrcManager_AuthDeviceSessionKey_001
 * @tc.desc: Test AuthDeviceSessionKey with valid session key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSrcManager_AuthDeviceSessionKey_001, TestSize.Level1)
{
    auto srcManager = std::make_shared<AuthPincodeSrcManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(srcManager, nullptr);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";
    std::map<std::string, std::string> authParam;
    int32_t ret = srcManager->AuthPincode(targetId, authParam, 100, 12345);
    EXPECT_EQ(ret, DM_OK);

    int64_t requestId = 12345;
    std::vector<uint8_t> keyData = {1, 2, 3, 4, 5, 6, 7, 8};
    const uint8_t *sessionKey = keyData.data();
    uint32_t sessionKeyLen = keyData.size();

    srcManager->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
}

/**
 * @tc.name: AuthPincodeSinkManager_AuthDeviceSessionKey_001
 * @tc.desc: Test Sink AuthDeviceSessionKey with valid session key
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, AuthPincodeSinkManager_AuthDeviceSessionKey_001, TestSize.Level1)
{
    auto sinkManager = std::make_shared<AuthPincodeSinkManager>(mockSoftbusConnector_,
        mockListener_, mockHiChainAuthConnector_);
    ASSERT_NE(sinkManager, nullptr);

    PeerTargetId3rd targetId;
    targetId.deviceId = "test_device_id";
    std::map<std::string, std::string> authParam;
    int32_t ret = sinkManager->AuthPincode(targetId, authParam, 100, 12345);
    EXPECT_EQ(ret, DM_OK);

    int64_t requestId = 12345;
    std::vector<uint8_t> keyData = {1, 2, 3, 4, 5, 6, 7, 8};
    const uint8_t *sessionKey = keyData.data();
    uint32_t sessionKeyLen = keyData.size();

    sinkManager->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
}

/**
 * @tc.name: DmAuthPincodeMessageProcessor_001
 * @tc.desc: Test DmAuthPincodeMessageProcessor construction
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, DmAuthPincodeMessageProcessor_001, TestSize.Level1)
{
    auto msgProcessor = std::make_shared<DmAuthPincodeMessageProcessor3rd>();
    ASSERT_NE(msgProcessor, nullptr);
}

/**
 * @tc.name: DmAuthPincodeStateMachine_001
 * @tc.desc: Test DmAuthPincodeStateMachine construction
 * @tc.type: FUNC
 */
HWTEST_F(AuthPincodeManagerTest, DmAuthPincodeStateMachine_001, TestSize.Level1)
{
    auto context = std::make_shared<DmAuthPincodeContext>();
    context->direction = DM_AUTH_PINCODE_SOURCE;
    auto stateMachine = std::make_shared<DmAuthPincodeStateMachine3rd>(context);
    ASSERT_NE(stateMachine, nullptr);

    stateMachine->Stop();
}

} // namespace DistributedHardware
} // namespace OHOS