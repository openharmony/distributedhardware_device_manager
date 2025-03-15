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
#include "UTTest_softbus_session.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "json_object.h"
#include "softbus_connector.h"
#include "softbus_session.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusSessionTest::SetUp()
{
}
void SoftbusSessionTest::TearDown()
{
}
void SoftbusSessionTest::SetUpTestCase()
{
}
void SoftbusSessionTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<SoftbusSession> softbusSession = std::make_shared<SoftbusSession>();
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
std::shared_ptr<DmAuthManager> discoveryMgr =
    std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector, listener, hiChainAuthConnector);

/**
 * @tc.name: OpenAuthSession_001
 * @tc.desc: set deviceId =null, return sessionId(1)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, OpenAuthSession_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "";
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int ret = softbusSession->OpenAuthSession(deviceId);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: OpenAuthSession_002
 * @tc.desc: set deviceId = "123456";and return sessionId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, OpenAuthSession_002, testing::ext::TestSize.Level1)
{
    std::string deviceId = "123456";
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int ret = softbusSession->OpenAuthSession(deviceId);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: SendData_001
 * @tc.desc: set message null and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, SendData_001, testing::ext::TestSize.Level1)
{
    std::string message = "";
    int32_t sessionId = -1;
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int ret = softbusSession->SendData(sessionId, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SendData_002
 * @tc.desc: set sessionId = 0, go to the SendBytes'smaster and return SOFTBUS_TRANS_SESSION_SERVER_NOINIT
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, SendData_002, testing::ext::TestSize.Level1)
{
    int32_t msgType = 2;
    JsonObject jsonObj;
    jsonObj[TAG_VER] = DM_ITF_VER;
    jsonObj[TAG_MSG_TYPE] = msgType;
    std::string message = SafetyDump(jsonObj);
    int32_t sessionId = 0;
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    softbusSession->RegisterSessionCallback(std::shared_ptr<ISoftbusSessionCallback>(discoveryMgr));
    int ret = softbusSession->SendData(sessionId, message);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_SERVER_NOINIT);
}

/**
 * @tc.name: SendData_003
 * @tc.desc: set jsonObject[TAG_MSG_TYPE] is string and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, SendData_003, testing::ext::TestSize.Level1)
{
    std::string message = R"(
    {
        "MSG_TYPE": "messageTest"
    }
    )";
    int32_t sessionId = 0;
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int32_t ret = softbusSession->SendData(sessionId, message);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SoftbusSession_001
 * @tc.desc: set SoftbusSession to make a new pointer, and it not nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, SoftbusSession_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusSession> m_SoftbusSession = std::make_shared<SoftbusSession>();
    ASSERT_NE(m_SoftbusSession, nullptr);
}

/**
 * @tc.name: SoftbusSession_002
 * @tc.desc: set SoftbusSession to make a new pointer, it not nullptr and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, SoftbusSession_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusSession> m_SoftbusSession = std::make_shared<SoftbusSession>();
    m_SoftbusSession.reset();
    EXPECT_EQ(m_SoftbusSession, nullptr);
}

/**
 * @tc.name: CloseAuthSession_001
 * @tc.desc: set sessionId = 3, and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, CloseAuthSession_001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 3;
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int ret = softbusSession->CloseAuthSession(sessionId);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetPeerDeviceId_001
 * @tc.desc: set sessionId = 3 and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, GetPeerDeviceId_001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 3;
    std::string peerDevId;
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int ret = softbusSession->GetPeerDeviceId(sessionId, peerDevId);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_SERVER_NOINIT);
}

/**
 * @tc.name: RegisterSessionCallback_001
 * @tc.desc: set info to null and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, RegisterSessionCallback_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<ISoftbusSessionCallback> callback;
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int ret = softbusSession->RegisterSessionCallback(callback);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterSessionCallback_001
 * @tc.desc: set info to null and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, UnRegisterSessionCallback_001, testing::ext::TestSize.Level1)
{
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    int ret = softbusSession->UnRegisterSessionCallback();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnSessionOpened_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusSessionTest, OnSessionOpened_001, testing::ext::TestSize.Level1)
{
    if (softbusSession == nullptr) {
        softbusSession = std::make_shared<SoftbusSession>();
    }
    softbusSession->RegisterSessionCallback(discoveryMgr);
    int sessionId = 1;
    int result = 0;
    void *data = nullptr;
    unsigned int dataLen = 1;
    softbusSession->OnBytesReceived(sessionId, data, dataLen);
    softbusSession->OnBytesReceived(sessionId, data, -1);
    sessionId = -1;
    softbusSession->OnBytesReceived(sessionId, data, dataLen);
    int ret = softbusSession->OnSessionOpened(sessionId, result);
    softbusSession->OnSessionClosed(sessionId);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
