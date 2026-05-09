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
}
}