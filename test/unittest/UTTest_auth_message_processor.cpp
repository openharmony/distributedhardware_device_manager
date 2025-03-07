/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "UTTest_auth_message_processor.h"

#include "dm_log.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "auth_message_processor.h"
#include "softbus_connector.h"
#include "softbus_session.h"
#include "dm_auth_manager.h"
#include "device_manager_service_listener.h"
#include "auth_ui_state_manager.h"
#include <vector>

namespace OHOS {
namespace DistributedHardware {
constexpr const char* TAG_APP_THUMBNAIL = "APPTHUM";
constexpr const char* TAG_HOST = "HOST";

using namespace testing;
using namespace testing::ext;
class CryptoAdapterTest : public ICryptoAdapter {
public:
    CryptoAdapterTest() {}
    virtual ~CryptoAdapterTest() {}
    std::string GetName() override
    {
        return "";
    }
    std::string GetVersion() override
    {
        return "";
    }
    int32_t MbedTlsEncrypt(const uint8_t *plainText, int32_t plainTextLen, uint8_t *cipherText, int32_t cipherTextLen,
        int32_t *outLen) override
    {
        (void)plainText;
        (void)plainTextLen;
        (void)cipherText;
        (void)cipherTextLen;
        return DM_OK;
    }
    int32_t MbedTlsDecrypt(const uint8_t *cipherText, int32_t cipherTextLen, uint8_t *plainText, int32_t plainTextLen,
        int32_t *outLen) override
    {
        (void)cipherText;
        (void)cipherTextLen;
        (void)plainText;
        (void)plainTextLen;
        return DM_OK;
    }
};

void AuthMessageProcessorTest::SetUp()
{
}
void AuthMessageProcessorTest::TearDown()
{
}
void AuthMessageProcessorTest::SetUpTestCase()
{
    DmCryptoMgr::dmCryptoMgr = cryptoMgrMock_;
}
void AuthMessageProcessorTest::TearDownTestCase()
{
    DmCryptoMgr::dmCryptoMgr = nullptr;
    cryptoMgrMock_ = nullptr;
}

namespace {
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
/**
 * @tc.name: AuthMessageProcessor::AuthMessageProcessor_001
 * @tc.desc: 1 set cryptoAdapter_ to null
 *           2 call AuthMessageProcessor::AuthMessageProcessor_001 with cryptoAdapter_ = nullptr
 *           3 check ret is authMessageProcessor->CreateNegotiateMessage(jsonObj);
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, AuthMessageProcessor_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> Test =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    ASSERT_NE(Test, nullptr);
}

/**
 * @tc.name: AuthMessageProcessor::AuthMessageProcessor_001
 * @tc.desc: 1 set cryptoAdapter_ to null
 *           2 call AuthMessageProcessor::AuthMessageProcessor_001 with cryptoAdapter_ = nullptr
 *           3 check ret is authMessageProcessor->CreateNegotiateMessage(jsonObj);
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, AuthMessageProcessor_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> Test =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    Test.reset();
    EXPECT_EQ(Test, nullptr);
}

/**
 * @tc.name: AuthMessageProcessor::CreateNegotiateMessage_001
 * @tc.desc: 1 set cryptoAdapter_ to null
 *           2 call AuthMessageProcessor::CreateNegotiateMessage_001 with cryptoAdapter_ = nullptr
 *           3 check ret is authMessageProcessor->CreateNegotiateMessage(jsonObj);
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, CreateNegotiateMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(authManager);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    int32_t msgType = MSG_TYPE_NEGOTIATE;
    nlohmann::json jsonObj;
    jsonObj[TAG_VER] = DM_ITF_VER;
    jsonObj[TAG_MSG_TYPE] = msgType;
    jsonObj[TAG_AUTH_TYPE] = authMessageProcessor->authResponseContext_->authType;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->SetRequestContext(authRequestContext);
    authMessageProcessor->cryptoAdapter_ = nullptr;
    authMessageProcessor->authRequestContext_->authType = AUTH_TYPE_NFC;
    authManager->authRequestContext_->hostPkgName = "hostPkgName";
    authManager->importAuthCode_ = "123";
    authManager->importPkgName_ = "hostPkgName";
    authMessageProcessor->authMgr_ = authManager;
    authMessageProcessor->CreateNegotiateMessage(jsonObj);
    std::string str1 = SafetyDump(jsonObj);
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    authMessageProcessor->authRequestContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authMessageProcessor->CreateNegotiateMessage(jsonObj);

    nlohmann::json jsonObject;
    jsonObject[TAG_ACCOUNT_GROUPID] = "";
    jsonObject[TAG_VER] = DM_ITF_VER;
    jsonObject[TAG_MSG_TYPE] = msgType;
    jsonObject[TAG_CRYPTO_SUPPORT] = false;
    jsonObject[TAG_AUTH_TYPE] = authMessageProcessor->authResponseContext_->authType;
    jsonObject[TAG_REPLY] = authMessageProcessor->authResponseContext_->reply;
    jsonObject[TAG_LOCAL_DEVICE_ID] = authMessageProcessor->authResponseContext_->localDeviceId;
    jsonObject[TAG_HOST] = "";
    std::string str2 = SafetyDump(jsonObject);
    ASSERT_NE(str1, str2);
}

/**
 * @tc.name: AuthMessageProcessor::CreateSyncGroupMessage_001
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, CreateSyncGroupMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    nlohmann::json jsona;
    nlohmann::json jsonObj;
    authMessageProcessor->authRequestContext_->deviceId = "132416546";
    std::vector<std::string> syncGroupList;
    syncGroupList.push_back("1111");
    authMessageProcessor->authRequestContext_->syncGroupList = syncGroupList;
    jsona[TAG_DEVICE_ID] = authMessageProcessor->authRequestContext_->deviceId;
    jsona[TAG_GROUPIDS] = authMessageProcessor->authRequestContext_->syncGroupList;
    authMessageProcessor->CreateSyncGroupMessage(jsonObj);
    std::string str1 = SafetyDump(jsona);
    std::string str2 = SafetyDump(jsonObj);
    ASSERT_EQ(str1, str2);
}

/**
 * @tc.name: AuthMessageProcessor::CreateResponseAuthMessage_001
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, CreateResponseAuthMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    nlohmann::json jsonObj;
    authMessageProcessor->authResponseContext_->reply = 0;
    authMessageProcessor->authResponseContext_->deviceId = "132416546";
    authMessageProcessor->authResponseContext_->token = "11";
    nlohmann::json jsonb;
    jsonb[TAG_GROUP_ID] = "123456";
    authMessageProcessor->authResponseContext_->groupId =
        SafetyDump(jsonb);
    authMessageProcessor->authResponseContext_->authToken = "123456";
    authMessageProcessor->authResponseContext_->networkId = "11112222";
    authMessageProcessor->authResponseContext_->requestId = 222222;
    authMessageProcessor->authResponseContext_->groupName = "333333";
    jsona[TAG_TOKEN] = authMessageProcessor->authResponseContext_->token;
    jsona[TAG_REPLY] = authMessageProcessor->authResponseContext_->reply;
    jsona[TAG_DEVICE_ID] = authMessageProcessor->authResponseContext_->deviceId;
    jsona[TAG_AUTH_TOKEN] = authMessageProcessor->authResponseContext_->authToken;
    jsona[TAG_NET_ID] = authMessageProcessor->authResponseContext_->networkId;
    jsona[TAG_REQUEST_ID] = authMessageProcessor->authResponseContext_->requestId;
    jsona[TAG_GROUP_ID] = "123456";
    jsona[TAG_GROUP_NAME] = authMessageProcessor->authResponseContext_->groupName;
    authMessageProcessor->CreateResponseAuthMessage(jsonObj);
    std::string str1 = SafetyDump(jsona);
    std::string str2 = SafetyDump(jsonObj);
    ASSERT_EQ(str1, str2);
}

/**
 * @tc.name: AuthMessageProcessor::CreateResponseFinishMessage_001
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, CreateResponseFinishMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    nlohmann::json jsonObj;
    authMessageProcessor->authResponseContext_->reply = 1;
    jsona[TAG_REPLY] = authMessageProcessor->authResponseContext_->reply;
    jsona[TAG_AUTH_FINISH] = authMessageProcessor->authResponseContext_->isFinish;
    authMessageProcessor->CreateResponseFinishMessage(jsonObj);
    std::string str1 = SafetyDump(jsona);
    std::string str2 = SafetyDump(jsonObj);
    ASSERT_EQ(str1, str2);
}

/**
 * @tc.name: AuthMessageProcessor::ParseResponseFinishMessage_001
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseResponseFinishMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    authMessageProcessor->authResponseContext_->reply = 1;
    jsonObj[TAG_REPLY] = authMessageProcessor->authResponseContext_->reply;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseResponseFinishMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseResponseFinishMessage_002
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseResponseFinishMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_REPLY] = 22;
    jsonObj[TAG_AUTH_FINISH] = true;
    authMessageProcessor->ParseResponseFinishMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, jsonObj[TAG_REPLY]);
}

/**
 * @tc.name: AuthMessageProcessor::ParseResponseFinishMessage_003
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseResponseFinishMessage_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::string str = R"(
    {
        "REPLY" : "30000000000"
    }
    )";
    nlohmann::json jsonObj = nlohmann::json::parse(str, nullptr, false);
    authMessageProcessor->ParseResponseFinishMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_->reply, jsonObj[TAG_REPLY]);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_001
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    authResponseContext->reply = 0;
    authResponseContext->deviceId = "11111";
    authResponseContext->authToken = "123456";
    authResponseContext->networkId = "12345";
    authResponseContext->requestId = 2;
    authResponseContext->groupId = "23456";
    authResponseContext->groupName = "34567";
    authResponseContext->token = "11123";
    jsona[TAG_TOKEN] = authResponseContext->token;
    jsona[TAG_REPLY] = authResponseContext->reply;
    jsona[TAG_DEVICE_ID] = authResponseContext->deviceId;
    jsona[TAG_AUTH_TOKEN] = authResponseContext->authToken;
    jsona[TAG_NET_ID] = authResponseContext->networkId;
    jsona[TAG_REQUEST_ID] = authResponseContext->requestId;
    jsona[TAG_GROUP_ID] = authResponseContext->groupId;
    jsona[TAG_GROUP_NAME] = authResponseContext->groupName;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_002
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = "12";
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_003
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = 0;
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_004
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = "4153125";
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_005
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_005, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = "4153125";
    jsona[TAG_TOKEN] = 1513;
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_006
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_006, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = "4153125";
    jsona[TAG_TOKEN] = "1513";
    jsona[TAG_REQUEST_ID] = 1513;
    jsona[TAG_GROUP_ID] = "4521201";
    jsona[TAG_GROUP_NAME] = "4521201";
    jsona[TAG_AUTH_TOKEN] = "4521201";
    jsona[TAG_AUTH_TOKEN] = "1513152";
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_007
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_007, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = "4153125";
    jsona[TAG_TOKEN] = "1513";
    jsona[TAG_REQUEST_ID] = "1513";
    jsona[TAG_GROUP_ID] = 4521201;
    jsona[TAG_GROUP_NAME] = "4521201";
    jsona[TAG_AUTH_TOKEN] = "4521201";
    jsona[TAG_AUTH_TOKEN] = "1513152";
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_008
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_008, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = "4153125";
    jsona[TAG_TOKEN] = "1513";
    jsona[TAG_REQUEST_ID] = "1513";
    jsona[TAG_GROUP_ID] = "4521201";
    jsona[TAG_GROUP_NAME] = 4521201;
    jsona[TAG_AUTH_TOKEN] = "4521201";
    jsona[TAG_AUTH_TOKEN] = "1513152";
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_009
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_009, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = "4153125";
    jsona[TAG_TOKEN] = "1513";
    jsona[TAG_REQUEST_ID] = "1513";
    jsona[TAG_GROUP_ID] = "4521201";
    jsona[TAG_GROUP_NAME] = "4521201";
    jsona[TAG_AUTH_TOKEN] = 4521201;
    jsona[TAG_AUTH_TOKEN] = "1513152";
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthResponseMessage_0010
 * @tc.desc: Compare JSON before and after assi gnment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessage_0010, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsona;
    jsona[TAG_REPLY] = 0;
    jsona[TAG_DEVICE_ID] = "4153125";
    jsona[TAG_TOKEN] = "1513";
    jsona[TAG_REQUEST_ID] = "1513";
    jsona[TAG_GROUP_ID] = "4521201";
    jsona[TAG_GROUP_NAME] = "4521201";
    jsona[TAG_AUTH_TOKEN] = "4521201";
    jsona[TAG_AUTH_TOKEN] = 1513152;
    authMessageProcessor->ParseAuthResponseMessage(jsona);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthRequestMessage_001
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthRequestMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    nlohmann::json jsonThumbnail;
    authResponseContext->deviceId = "123";
    authResponseContext->reply = 0;
    authResponseContext->authType = 222;
    authResponseContext->networkId = "234";
    authResponseContext->groupId = "345";
    authResponseContext->groupName = "456";
    authResponseContext->requestId = 2333;
    jsonThumbnail[TAG_DEVICE_ID] = authResponseContext->deviceId;
    jsonThumbnail[TAG_REPLY] = authResponseContext->reply;
    jsonThumbnail[TAG_AUTH_TYPE] = authResponseContext->authType;
    jsonThumbnail[TAG_NET_ID] = authResponseContext->networkId;
    jsonThumbnail[TAG_GROUP_ID] = authResponseContext->groupId;
    jsonThumbnail[TAG_GROUP_NAME] = authResponseContext->groupName;
    jsonThumbnail[TAG_REQUEST_ID] = authResponseContext->requestId;
    int32_t ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthRequestMessage_002
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthRequestMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    nlohmann::json jsonThumbnail;
    jsonThumbnail[TAG_SLICE_NUM] = 1;
    jsonThumbnail[TAG_INDEX] = 0;
    jsonThumbnail[TAG_DEVICE_ID] = "123";
    jsonThumbnail[TAG_AUTH_TYPE] = 1;
    jsonThumbnail[TAG_CUSTOM_DESCRIPTION] = "123";
    jsonThumbnail[TAG_TOKEN] = "1234";
    jsonThumbnail[TAG_TARGET] = "12345";
    jsonThumbnail[TAG_APP_OPERATION] = "123456";
    jsonThumbnail[TAG_LOCAL_DEVICE_ID] = "localdeviceTest";
    jsonThumbnail[TAG_REQUESTER] = "AJ125S25S3E65F1A24T";
    jsonThumbnail[TAG_DEVICE_TYPE] = 1;
    int32_t ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, DM_OK);
    jsonThumbnail[TAG_INDEX] = 1;
    ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, DM_OK);
    jsonThumbnail[TAG_SLICE_NUM] = 10;
    ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, DM_OK);
    jsonThumbnail[TAG_SLICE_NUM] = 1;
    jsonThumbnail[TAG_IS_SHOW_DIALOG] = true;
    ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, DM_OK);
    jsonThumbnail[TAG_BIND_TYPE_SIZE] = 10001;
    ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, ERR_DM_FAILED);
    jsonThumbnail[TAG_BIND_TYPE_SIZE] = 1;
    ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthRequestMessage_003
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthRequestMessage_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    nlohmann::json jsonThumbnail;
    jsonThumbnail[TAG_SLICE_NUM] = 1;
    jsonThumbnail[TAG_INDEX] = 0;
    jsonThumbnail[TAG_DEVICE_ID] = 123;
    jsonThumbnail[TAG_AUTH_TYPE] = 1;
    jsonThumbnail[TAG_CUSTOM_DESCRIPTION] = "123";
    jsonThumbnail[TAG_TOKEN] = "1234";
    jsonThumbnail[TAG_TARGET] = "12345";
    jsonThumbnail[TAG_APP_OPERATION] = "123456";
    int32_t ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseAuthRequestMessage_004
 * @tc.desc: Compare JSON before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseAuthRequestMessage_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    nlohmann::json jsonThumbnail;
    jsonThumbnail[TAG_SLICE_NUM] = 1;
    jsonThumbnail[TAG_INDEX] = 0;
    jsonThumbnail[TAG_DEVICE_ID] = "123";
    jsonThumbnail[TAG_AUTH_TYPE] = 1;
    jsonThumbnail[TAG_CUSTOM_DESCRIPTION] = "123";
    jsonThumbnail[TAG_TOKEN] = "1234";
    jsonThumbnail[TAG_TARGET] = "12345";
    jsonThumbnail[TAG_APP_OPERATION] = "123456";
    jsonThumbnail[TAG_APP_THUMBNAIL] = "jsontest";
    jsonThumbnail[TAG_LOCAL_DEVICE_ID] = "localdeviceTest";
    jsonThumbnail[TAG_REQUESTER] = "iknbghkkj266SSjsjjdan21526";
    jsonThumbnail[TAG_DEVICE_TYPE] = 1;
    int32_t ret = authMessageProcessor->ParseAuthRequestMessage(jsonThumbnail);
    ASSERT_EQ(ret, ERR_DM_AUTH_MESSAGE_INCOMPLETE);
}

/**
 * @tc.name: AuthMessageProcessor::ParseNegotiateMessage_001
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseNegotiateMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_CRYPTO_SUPPORT] = "CRYPTOSUPPORT";
    jsonObj[TAG_CRYPTO_SUPPORT] = authMessageProcessor->authResponseContext_->cryptoSupport;
    authResponseContext->localDeviceId = "22";
    authResponseContext->authType = 1;
    authResponseContext->reply = 33;
    jsonObj[TAG_AUTH_TYPE] = authResponseContext->authType;
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_REPLY] = authResponseContext->reply;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseNegotiateMessage_002
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseNegotiateMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    authResponseContext->localDeviceId = "22";
    authResponseContext->authType = 1;
    authResponseContext->reply = 33;
    jsonObj[TAG_CRYPTO_NAME] = "CRYPTONAME";
    jsonObj[TAG_CRYPTO_NAME] = authResponseContext->cryptoSupport;
    jsonObj[TAG_AUTH_TYPE] = authResponseContext->authType;
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_REPLY] = authResponseContext->reply;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseNegotiateMessage_003
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseNegotiateMessage_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    authResponseContext->localDeviceId = "22";
    authResponseContext->authType = 1;
    authResponseContext->reply = 33;
    jsonObj[TAG_CRYPTO_VERSION] = "CRYPTOVERSION";
    jsonObj[TAG_CRYPTO_VERSION] = authResponseContext->cryptoSupport;
    jsonObj[TAG_AUTH_TYPE] = authResponseContext->authType;
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_REPLY] = authResponseContext->reply;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseNegotiateMessage_004
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseNegotiateMessage_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    authResponseContext->localDeviceId = "22";
    authResponseContext->authType = 1;
    authResponseContext->reply = 33;
    jsonObj[TAG_DEVICE_ID] = "DEVICEID";
    jsonObj[TAG_DEVICE_ID] = authResponseContext->deviceId;
    jsonObj[TAG_AUTH_TYPE] = authResponseContext->authType;
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_REPLY] = authResponseContext->reply;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseNegotiateMessage_005
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseNegotiateMessage_005, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    authResponseContext->localDeviceId = "22";
    authResponseContext->authType = 1;
    authResponseContext->reply = 33;
    jsonObj[TAG_LOCAL_DEVICE_ID] = "LOCALDEVICEID";
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_AUTH_TYPE] = authResponseContext->authType;
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_REPLY] = authResponseContext->reply;
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseNegotiateMessage_006
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseNegotiateMessage_006, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    authResponseContext->localDeviceId = "22";
    authResponseContext->authType = 1;
    authResponseContext->reply = 33;
    jsonObj[TAG_LOCAL_DEVICE_ID] = "LOCALDEVICEID";
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_AUTH_TYPE] = authResponseContext->authType;
    jsonObj[TAG_LOCAL_DEVICE_ID] = authResponseContext->localDeviceId;
    jsonObj[TAG_REPLY] = authResponseContext->reply;
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    jsonObj[TAG_CRYPTO_NAME] = "NAMETEST";
    jsonObj[TAG_CRYPTO_VERSION] = "1.0";
    jsonObj[TAG_ACCOUNT_GROUPID] = "GROUPID";
    authMessageProcessor->ParseNegotiateMessage(jsonObj);
    jsonObj[TAG_ACCOUNT_GROUPID] = 12;
    jsonObj[TAG_HOST] = "12";
    jsonObj[TAG_AUTH_TYPE] = "12";
    jsonObj[TAG_EDITION] = "edition";
    jsonObj[TAG_BUNDLE_NAME] = "bundleName";
    jsonObj[TAG_PEER_BUNDLE_NAME] = "peerbundleName";
    jsonObj[TAG_REMOTE_DEVICE_NAME] = "remoteDeviceName";
    authMessageProcessor->SetResponseContext(authResponseContext);
    authMessageProcessor->ParseNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::ParseRespNegotiateMessage_001
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseRespNegotiateMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    nlohmann::json jsonObj;
    jsonObj[TAG_IDENTICAL_ACCOUNT] = false;
    authMessageProcessor->ParseRespNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->isIdenticalAccount, jsonObj[TAG_IDENTICAL_ACCOUNT]);
}

/**
 * @tc.name: AuthMessageProcessor::ParseRespNegotiateMessage_002
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseRespNegotiateMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    nlohmann::json jsonObj;
    jsonObj[TAG_IDENTICAL_ACCOUNT] = "test";
    authMessageProcessor->ParseRespNegotiateMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_->isIdenticalAccount, jsonObj[TAG_IDENTICAL_ACCOUNT]);
}

/**
 * @tc.name: AuthMessageProcessor::ParseRespNegotiateMessage_003
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseRespNegotiateMessage_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_IDENTICAL_ACCOUNT] = true;
    jsonObj[TAG_REPLY] = 1231;
    jsonObj[TAG_LOCAL_DEVICE_ID] = "1212";
    jsonObj[TAG_IS_AUTH_CODE_READY] = true;
    jsonObj[TAG_ACCOUNT_GROUPID] = "1212";
    jsonObj[TAG_NET_ID] = "1212";
    jsonObj[TAG_TOKENID] = "1212";
    jsonObj[TAG_TARGET_DEVICE_NAME] = "1212";
    jsonObj[TAG_IMPORT_AUTH_CODE] = "1212";
    authMessageProcessor->ParseRespNegotiateMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->isIdenticalAccount, true);
}

/**
 * @tc.name: AuthMessageProcessor::ParseRespNegotiateMessage_004
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseRespNegotiateMessage_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_IDENTICAL_ACCOUNT] = 112;
    jsonObj[TAG_REPLY] = "1231";
    jsonObj[TAG_LOCAL_DEVICE_ID] = 1212;
    jsonObj[TAG_IS_AUTH_CODE_READY] = 1212;
    jsonObj[TAG_ACCOUNT_GROUPID] = 1212;
    jsonObj[TAG_NET_ID] = 1212;
    jsonObj[TAG_TOKENID] = 1212;
    jsonObj[TAG_TARGET_DEVICE_NAME] = 1212;
    authMessageProcessor->ParseRespNegotiateMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_->isIdenticalAccount, jsonObj[TAG_IDENTICAL_ACCOUNT]);
}
/**
 * @tc.name: AuthMessageProcessor::SetRequestContext_001
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, SetRequestContext_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->SetRequestContext(authRequestContext);
    ASSERT_EQ(authMessageProcessor->authRequestContext_, authRequestContext);
}

/**
 * @tc.name: AuthMessageProcessor::SetRequestContext_002
 * @tc.desc: Judge whether authrequestcontext is empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, SetRequestContext_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->SetRequestContext(nullptr);
    ASSERT_EQ(authMessageProcessor->authRequestContext_, nullptr);
}

/**
 * @tc.name: AuthMessageProcessor::SetResponseContext_001
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, SetResponseContext_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, authResponseContext);
}

/**
 * @tc.name: AuthMessageProcessor::SetResponseContext_002
 * @tc.desc: Judge whether authrequestcontext is empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, SetResponseContext_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(nullptr);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, nullptr);
}

/**
 * @tc.name: AuthMessageProcessor::GetResponseContext_001
 * @tc.desc: Compare authResponseContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, GetResponseContext_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<DmAuthResponseContext> authResponseContext = authMessageProcessor->GetResponseContext();
    ASSERT_EQ(authResponseContext, authMessageProcessor->authResponseContext_);
}

/**
 * @tc.name: AuthMessageProcessor::GetResponseContext_002
 * @tc.desc: Judge whether authrequestcontext is empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, GetResponseContext_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<DmAuthResponseContext> authResponseContext = authMessageProcessor->GetResponseContext();
    ASSERT_NE(authResponseContext, nullptr);
}

/**
 * @tc.name: AuthMessageProcessor::CreateSimpleMessage_001
 * @tc.desc: return the length of string is empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, CreateSimpleMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    int32_t msgType = MSG_TYPE_SYNC_GROUP;
    std::string ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_RESP_AUTH;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_REQ_AUTH_TERMINATE;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_RESP_AUTH_EXT;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_REQ_PUBLICKEY;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_NEGOTIATE;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_RESP_NEGOTIATE;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_UNKNOWN;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
    msgType = MSG_TYPE_REQ_RECHECK_MSG;
    ret = authMessageProcessor->CreateSimpleMessage(msgType);
    ASSERT_NE(ret.size(), 0);
}

/**
 * @tc.name: AuthMessageProcessor::GetRequestContext_001
 * @tc.desc: Compare authRequestContext before and after assignment
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, GetRequestContext_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthRequestContext> authRequestContext = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->SetRequestContext(authRequestContext);
    auto ret = authMessageProcessor->GetRequestContext();
    ASSERT_EQ(authMessageProcessor->authRequestContext_, ret);
}

/**
 * @tc.name: AuthMessageProcessor::ParseMessage_001
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "AUTHTYPE": 1,
        "CRYPTOSUPPORT": false,
        "ITF_VER": "1.1",
        "LOCALDEVICEID": "e68f0b9186386e87487564b02e91421f904eb9517f262721c9ada090477e35f5",
        "MSG_TYPE": 80,
        "REPLY": 2016
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseMessage_002
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "AUTHTYPE": 1,
        "CRYPTOSUPPORT": false,
        "ITF_VER": "1.1",
        "LOCALDEVICEID": "e68f0b9186386e87487564b02e91421f904eb9517f262721c9ada090477e35f5",
        "MSG_TYPE": 90,
        "REPLY": 2016
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseMessage_003
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseMessage_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "APPDESC": "Distributed Calc",
        "CUSTOMDESC": "customDescription",
        "APPICON": "",
        "APPNAME": "Distributed Calc",
        "APPOPERATION": "appoperrationTest",
        "AUTHTYPE":1,
        "DEVICEID": "e68f0b9186386e87487564b02e91421f904eb9517f262721c9ada090477e35f5",
        "LOCALDEVICEID": "test0b9186386e87487564b02etest1f904eb9517f262721c9ada090477etest",
        "DEVICETYPE": 1,
        "HOST": "com.example.distributedcalc",
        "INDEX": 0,
        "ITF_VER": "1.1",
        "MSG_TYPE": 100,
        "REQUESTER": "test0b9186386e87487564b02etest1f904eb9517f262721c9ada090477etest",
        "SLICE": 1,
        "TARGET": "com.example.distributedcalc",
        "THUMSIZE": 0,
        "TOKEN": "73141022",
        "VISIBILITY": 0
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseMessage_004
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseMessage_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "REPLY": 0,
        "DEVICEID": "e68f0b9186386e87487564b02e91421f904eb9517f262721c9ada090477e35f5",
        "TOKEN": "7314",
        "GROUPNAME": "com.example.test",
        "ITF_VER": "1.1",
        "MSG_TYPE": 200,
        "NETID": "147258963",
        "REQUESTID": 8448,
        "authToken": "com.example.distributedcalc62063A65EC8540074FF01413BDC3B6D7",
        "groupId" : "e68f0b9186386e87487564b02e91421f904eb9517f262721c9ada090477e35f5"
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseMessage_005
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseMessage_005, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "REPLY": 0,
        "ITF_VER": "1.1",
        "MSG_TYPE": 104
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthMessageProcessor::ParseMessage_006
 * @tc.desc: Return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AuthMessageProcessorTest, ParseMessage_006, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "REPLY": 1,
        "ITF_VER": "1.1.2",
        "MSG_TYPE": "104"
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AuthMessageProcessorTest, ParseMessage_007, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "REPLY": 1,
        "LOCALDEVICEID": "devId_4655198_test",
        "hostPkgname": "pkgname_dm_test"
        "MSG_TYPE": "501",
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AuthMessageProcessorTest, ParseMessage_008, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "publicKey": "publicKey_test",
        "MSG_TYPE": "502",
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    std::string message1 = R"(
    {
        "publicKey": "publicKey_test",
        "MSG_TYPE": "503",
    }
    )";
    ret = authMessageProcessor->ParseMessage(message);
    std::string message2 = R"(
    {
        "publicKey": "publicKey_test",
        "MSG_TYPE": "504",
    }
    )";
    ret = authMessageProcessor->ParseMessage(message2);
    std::string message3 = R"(
    {
        "publicKey": "publicKey_test",
        "MSG_TYPE": "600",
    }
    )";
    ret = authMessageProcessor->ParseMessage(message3);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AuthMessageProcessorTest, ParseMessage_009, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    std::shared_ptr<DmAuthResponseContext> authResponseContext = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->SetResponseContext(authResponseContext);
    std::string message = R"(
    {
        "REPLY": 1,
        "tokenId": "tokenId_123_test",
        "confirmOperation": "1",
        "REQUESTID": "1",
        "MSG_TYPE": "501",
    }
    )";
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    nlohmann::json jsonObj;
    jsonObj[TAG_LOCAL_ACCOUNTID] = "local_accountId_123";
    jsonObj[TAG_LOCAL_USERID] = 1;
    jsonObj[TAG_BIND_LEVEL] = 1;
    jsonObj[TAG_ISONLINE] = true;
    jsonObj[TAG_IDENTICAL_ACCOUNT] = true;
    jsonObj[TAG_AUTHED] = true;
    jsonObj[TAG_TOKENID] = 100;
        jsonObj[TAG_DMVERSION] = "1.1.1";
    jsonObj[TAG_HAVECREDENTIAL] = true;
    jsonObj[TAG_BIND_TYPE_SIZE] = 5;
    authMessageProcessor->ParsePkgNegotiateMessage(jsonObj);
}

HWTEST_F(AuthMessageProcessorTest, GetJsonObj_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authRequestContext_->groupVisibility = 0;
    authMessageProcessor->authRequestContext_->authType = 5;
    int32_t param = 1;
    authMessageProcessor->authRequestContext_->bindType.push_back(param);
    nlohmann::json jsonObj;
    authMessageProcessor->GetJsonObj(jsonObj);
    ASSERT_EQ(jsonObj[TAG_IS_SHOW_DIALOG], false);
}

HWTEST_F(AuthMessageProcessorTest, GetJsonObj_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authRequestContext_->groupVisibility = 0;
    authMessageProcessor->authRequestContext_->authType = 1;
    int32_t param = 1;
    authMessageProcessor->authRequestContext_->bindType.push_back(param);
    nlohmann::json jsonObj;
    authMessageProcessor->GetJsonObj(jsonObj);
    ASSERT_EQ(jsonObj[TAG_IS_SHOW_DIALOG], true);
}

HWTEST_F(AuthMessageProcessorTest, CreatePublicKeyMessageExt_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_->publicKey = "13213521";
    nlohmann::json jsonObj;
    authMessageProcessor->CreatePublicKeyMessageExt(jsonObj);
    ASSERT_EQ(jsonObj[TAG_PUBLICKEY], authMessageProcessor->authResponseContext_->publicKey);
}

HWTEST_F(AuthMessageProcessorTest, CreateResponseAuthMessageExt_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_->reply = 123;
    authMessageProcessor->authResponseContext_->token = "13213521";
    authMessageProcessor->authResponseContext_->confirmOperation = 456;
    authMessageProcessor->authResponseContext_->requestId = 521;
    nlohmann::json jsonObj;
    authMessageProcessor->CreateResponseAuthMessageExt(jsonObj);
    ASSERT_EQ(jsonObj[TAG_REPLY], authMessageProcessor->authResponseContext_->reply);
}

HWTEST_F(AuthMessageProcessorTest, CreateRespNegotiateMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = nullptr;
    nlohmann::json jsonObj;
    authMessageProcessor->CreateRespNegotiateMessage(jsonObj);
    ASSERT_EQ(jsonObj[TAG_CRYPTO_SUPPORT], false);
}

HWTEST_F(AuthMessageProcessorTest, CreateRespNegotiateMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    nlohmann::json jsonObj;
    authMessageProcessor->CreateRespNegotiateMessage(jsonObj);
    ASSERT_EQ(jsonObj[TAG_CRYPTO_SUPPORT], true);
}

HWTEST_F(AuthMessageProcessorTest, ParsePublicKeyMessageExt_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    nlohmann::json jsonObj;
    jsonObj[TAG_PUBLICKEY] = 2121;
    authMessageProcessor->ParsePublicKeyMessageExt(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->publicKey.empty(), true);
}

HWTEST_F(AuthMessageProcessorTest, ParsePublicKeyMessageExt_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    nlohmann::json jsonObj;
    jsonObj[TAG_PUBLICKEY] = "2121";
    authMessageProcessor->ParsePublicKeyMessageExt(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->publicKey.empty(), false);
}

HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessageExt_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    nlohmann::json jsonObj;
    jsonObj[TAG_REPLY] = 2121;
    jsonObj[TAG_TOKEN] = "1231";
    jsonObj[TAG_CONFIRM_OPERATION] = 1212;
    jsonObj[TAG_REQUEST_ID] = 1212;
    authMessageProcessor->ParseAuthResponseMessageExt(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, 2121);
}

HWTEST_F(AuthMessageProcessorTest, ParseAuthResponseMessageExt_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    nlohmann::json jsonObj;
    jsonObj[TAG_REPLY] = "2121";
    jsonObj[TAG_TOKEN] = 1231;
    jsonObj[TAG_CONFIRM_OPERATION] = "1212";
    jsonObj[TAG_REQUEST_ID] = "1212";
    authMessageProcessor->ParseAuthResponseMessageExt(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_->reply, 2121);
}

HWTEST_F(AuthMessageProcessorTest, GetAuthReqMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    nlohmann::json jsonObj;
    jsonObj[TAG_AUTH_TYPE] = 21;
    jsonObj[TAG_TOKEN] = "1231";
    jsonObj[TAG_DEVICE_ID] = "1212";
    jsonObj[TAG_TARGET] = "1212";
    jsonObj[TAG_LOCAL_DEVICE_ID] = "2121";
    jsonObj[TAG_APP_OPERATION] = "1231";
    jsonObj[TAG_CUSTOM_DESCRIPTION] = "1212";
    jsonObj[TAG_REQUESTER] = "1212";
    jsonObj[TAG_LOCAL_DEVICE_TYPE] = "2121";
    jsonObj[TAG_INDEX] = 1212;
    authMessageProcessor->GetAuthReqMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->authType, 21);
}

HWTEST_F(AuthMessageProcessorTest, GetAuthReqMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->cryptoAdapter_ = std::make_shared<CryptoAdapterTest>();
    nlohmann::json jsonObj;
    jsonObj[TAG_AUTH_TYPE] = "21";
    jsonObj[TAG_TOKEN] = 1231;
    jsonObj[TAG_DEVICE_ID] = 1212;
    jsonObj[TAG_TARGET] = 1212;
    jsonObj[TAG_LOCAL_DEVICE_ID] = 2121;
    jsonObj[TAG_APP_OPERATION] = 1231;
    jsonObj[TAG_CUSTOM_DESCRIPTION] = 1212;
    jsonObj[TAG_REQUESTER] = 1212;
    jsonObj[TAG_LOCAL_DEVICE_TYPE] = 2121;
    jsonObj[TAG_INDEX] = "1212";
    authMessageProcessor->GetAuthReqMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_->authType, 21);
}

HWTEST_F(AuthMessageProcessorTest, ParsePkgNegotiateMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_LOCAL_ACCOUNTID] = "21";
    jsonObj[TAG_LOCAL_USERID] = 1231;
    jsonObj[TAG_BIND_LEVEL] = 1212;
    jsonObj[TAG_ISONLINE] = true;
    jsonObj[TAG_IDENTICAL_ACCOUNT] = true;
    jsonObj[TAG_AUTHED] = true;
    jsonObj[TAG_TOKENID] = 1212;
    jsonObj[TAG_DMVERSION] = "1212";
    jsonObj[TAG_HAVECREDENTIAL] = true;
    jsonObj[TAG_BIND_TYPE_SIZE] = 5;
    authMessageProcessor->ParsePkgNegotiateMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_->authType, 21);
}

HWTEST_F(AuthMessageProcessorTest, ParsePkgNegotiateMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_LOCAL_ACCOUNTID] = 21;
    jsonObj[TAG_LOCAL_USERID] = "1231";
    jsonObj[TAG_BIND_LEVEL] = "1212";
    jsonObj[TAG_ISONLINE] = "1212";
    jsonObj[TAG_IDENTICAL_ACCOUNT] = "1212";
    jsonObj[TAG_AUTHED] = "1212";
    jsonObj[TAG_TOKENID] = "1212";
    jsonObj[TAG_DMVERSION] = 1212;
    jsonObj[TAG_HAVECREDENTIAL] = "1212";
    jsonObj[TAG_BIND_TYPE_SIZE] = "1212";
    authMessageProcessor->ParsePkgNegotiateMessage(jsonObj);
    jsonObj[TAG_BIND_TYPE_SIZE] = 1212;
    jsonObj[TAG_HOST_PKGLABEL] = "1212";
    authMessageProcessor->ParsePkgNegotiateMessage(jsonObj);
    jsonObj[TAG_BIND_TYPE_SIZE] = 121;
    jsonObj[TAG_HOST_PKGLABEL] = "1213";
    authMessageProcessor->ParsePkgNegotiateMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_->authType, 21);
}

HWTEST_F(AuthMessageProcessorTest, CreateDeviceAuthMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    int32_t msgType = 1;
    uint8_t param = 0;
    uint32_t dataLen = 0;
    std::string str = "1324213";
    std::string ret = authMessageProcessor->CreateDeviceAuthMessage(msgType, &param, dataLen);
    ASSERT_EQ(ret.empty(), false);

    std::shared_ptr<DeviceManagerServiceListener> listener_ = nullptr;
    std::shared_ptr<AuthUiStateManager> authUiStateManager = std::make_shared<AuthUiStateManager>(listener_);
    DmUiStateMsg msg = DmUiStateMsg::MSG_PIN_CODE_SUCCESS;
    authUiStateManager->UpdateUiState(msg);

    listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<AuthUiStateManager> authUiStateManager_ = std::make_shared<AuthUiStateManager>(listener_);
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    processInfo.userId = 123456;
    authUiStateManager_->pkgSet_.insert(processInfo);
    authUiStateManager_->UpdateUiState(msg);
}

HWTEST_F(AuthMessageProcessorTest, CreateReqReCheckMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_EDITION] = "edition";
    jsonObj[TAG_LOCAL_DEVICE_ID] = "1215";
    jsonObj[TAG_LOCAL_USERID] = 123;
    jsonObj[TAG_LOCAL_ACCOUNTID] = "localAccountId";
    jsonObj[TAG_TOKENID] = 1253;
    jsonObj[TAG_BUNDLE_NAME] = "bundleName";
    jsonObj[TAG_BIND_LEVEL] = 1;
    authMessageProcessor->cryptoMgr_ = std::make_shared<CryptoMgr>();
    EXPECT_CALL(*cryptoMgrMock_, EncryptMessage(_, _)).WillOnce(Return(DM_OK));
    authMessageProcessor->CreateReqReCheckMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_, nullptr);

    EXPECT_CALL(*cryptoMgrMock_, EncryptMessage(_, _)).WillOnce(Return(ERR_DM_FAILED));
    authMessageProcessor->CreateReqReCheckMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_, nullptr);
}

HWTEST_F(AuthMessageProcessorTest, ParseReqReCheckMessage_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_CRYPTIC_MSG] = "encryptStr";
    jsonObj[TAG_EDITION] = "edition";
    jsonObj[TAG_LOCAL_DEVICE_ID] = "1215";
    jsonObj[TAG_LOCAL_USERID] = 123;
    jsonObj[TAG_LOCAL_ACCOUNTID] = "localAccountId";
    jsonObj[TAG_TOKENID] = 1253;
    jsonObj[TAG_BUNDLE_NAME] = "bundleName";
    jsonObj[TAG_BIND_LEVEL] = 1;
    std::string decryptStr = "";
    authMessageProcessor->cryptoMgr_ = std::make_shared<CryptoMgr>();
    EXPECT_CALL(*cryptoMgrMock_, DecryptMessage(_, _)).WillOnce(Return(ERR_DM_FAILED));
    authMessageProcessor->ParseReqReCheckMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_, nullptr);

    EXPECT_CALL(*cryptoMgrMock_, DecryptMessage(_, _)).WillOnce(DoAll(SetArgReferee<1>(decryptStr), Return(DM_OK)));
    authMessageProcessor->ParseReqReCheckMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_, nullptr);

    decryptStr = jsonObj.dump();
    EXPECT_CALL(*cryptoMgrMock_, DecryptMessage(_, _)).Times(::testing::AtLeast(2))
        .WillOnce(DoAll(SetArgReferee<1>(decryptStr), Return(DM_OK)));
    authMessageProcessor->ParseReqReCheckMessage(jsonObj);
    ASSERT_NE(authMessageProcessor->authResponseContext_, nullptr);
}

HWTEST_F(AuthMessageProcessorTest, ParsePublicKeyMessageExt_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->encryptFlag_ = true;
    nlohmann::json jsonObj;
    jsonObj[TAG_PUBLICKEY] = "123456";
    jsonObj[TAG_CRYPTIC_MSG] = "cryptic";
    jsonObj[TAG_SESSIONKEY_ID] = 1;
    authMessageProcessor->cryptoMgr_ = std::make_shared<CryptoMgr>();
        
    authMessageProcessor->ParsePublicKeyMessageExt(jsonObj);
    std::string decryptStr = "";
    EXPECT_CALL(*cryptoMgrMock_, DecryptMessage(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(decryptStr), Return(DM_OK)));
    authMessageProcessor->ParsePublicKeyMessageExt(jsonObj);
    decryptStr = jsonObj.dump();
    EXPECT_CALL(*cryptoMgrMock_, DecryptMessage(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(decryptStr), Return(DM_OK)));
    authMessageProcessor->ParsePublicKeyMessageExt(jsonObj);

    EXPECT_CALL(*cryptoMgrMock_, DecryptMessage(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(decryptStr), Return(ERR_DM_FAILED)));
    authMessageProcessor->ParsePublicKeyMessageExt(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->publicKey.empty(), true);
}

HWTEST_F(AuthMessageProcessorTest, ParseMessage_010, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    jsonObj[TAG_MSG_TYPE] = 501;
    std::string message = "";
    message = jsonObj.dump();
    int32_t ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
    jsonObj[TAG_MSG_TYPE] = 502;
    message = jsonObj.dump();
    ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
    jsonObj[TAG_MSG_TYPE] = 504;
    message = jsonObj.dump();
    EXPECT_CALL(*cryptoMgrMock_, DecryptMessage(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(ERR_DM_FAILED));
    ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
    jsonObj[TAG_MSG_TYPE] = 700;
    message = jsonObj.dump();
    ret = authMessageProcessor->ParseMessage(message);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(AuthMessageProcessorTest, CreateResponseAuthMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    authMessageProcessor->authResponseContext_->reply = 0;
    authMessageProcessor->authResponseContext_->deviceId = "deviceId";
    authMessageProcessor->authResponseContext_->token = "123654";
    jsonObj[TAG_GROUP_ID] = 123;
    authMessageProcessor->CreateResponseAuthMessage(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_->reply, 0);
}

HWTEST_F(AuthMessageProcessorTest, CreateRespNegotiateMessage_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    nlohmann::json jsonObj;
    std::vector<int32_t> bindType;
    bindType.push_back(1);
    bindType.push_back(2);
    bindType.push_back(3);
    bindType.push_back(4);
    bindType.push_back(5);
    authMessageProcessor->authResponseContext_->bindType = bindType;
    authMessageProcessor->CreateRespNegotiateMessage(jsonObj);
    ASSERT_FALSE(authMessageProcessor->authResponseContext_->bindType.empty());
}

HWTEST_F(AuthMessageProcessorTest, CreatePublicKeyMessageExt_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_->publicKey = "1321352144564";
    authMessageProcessor->encryptFlag_ = true;
    nlohmann::json jsonObj;
    authMessageProcessor->cryptoMgr_ = std::make_shared<CryptoMgr>();
    EXPECT_CALL(*cryptoMgrMock_, EncryptMessage(_, _)).WillOnce(Return(ERR_DM_FAILED));
    authMessageProcessor->CreatePublicKeyMessageExt(jsonObj);
    std::string decryptStr = "1321352144564";
    EXPECT_CALL(*cryptoMgrMock_, EncryptMessage(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(decryptStr), Return(DM_OK)));
    authMessageProcessor->CreatePublicKeyMessageExt(jsonObj);
    ASSERT_EQ(jsonObj[TAG_CRYPTIC_MSG], authMessageProcessor->authResponseContext_->publicKey);
}

HWTEST_F(AuthMessageProcessorTest, CreateAuthRequestMessage_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authRequestContext_ = nullptr;
    auto ret = authMessageProcessor->CreateAuthRequestMessage();
    ASSERT_TRUE(ret.empty());
}

HWTEST_F(AuthMessageProcessorTest, GetJsonObj_010, testing::ext::TestSize.Level0)
{
    std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> data =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(data);
    authMessageProcessor->authResponseContext_ = nullptr;
    nlohmann::json jsonObj;
    authMessageProcessor->GetJsonObj(jsonObj);
    ASSERT_EQ(authMessageProcessor->authResponseContext_, nullptr);
    authMessageProcessor->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authMessageProcessor->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authMessageProcessor->authResponseContext_->bindType.push_back(101);
    authMessageProcessor->authResponseContext_->bindType.push_back(102);
    authMessageProcessor->authResponseContext_->bindType.push_back(103);
    authMessageProcessor->GetJsonObj(jsonObj);
    ASSERT_FALSE(authMessageProcessor->authResponseContext_->bindType.empty());
}

HWTEST_F(AuthMessageProcessorTest, IsPincodeImported_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthManager> authManager = nullptr;
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor = std::make_shared<AuthMessageProcessor>(authManager);
    authMessageProcessor->authMgr_ = authManager;
    bool ret = authMessageProcessor->IsPincodeImported();
    ASSERT_FALSE(ret);

    authManager = std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector_, listener, hiChainAuthConnector);
    authMessageProcessor->authMgr_ = authManager;
    ret = authMessageProcessor->IsPincodeImported();
    ASSERT_FALSE(ret);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
