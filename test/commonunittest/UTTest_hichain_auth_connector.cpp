/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <securec.h>
#include "UTTest_hichain_auth_connector.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "hichain_auth_connector.h"

static int32_t g_processCredentialResultCode = -1;
static std::string g_processCredentialReturnDataStr = "";

namespace OHOS {
namespace DistributedHardware {
void HiChainAuthConnectorTest::SetUp()
{
    g_processCredentialResultCode = -1;
    g_processCredentialReturnDataStr = "";
}

void HiChainAuthConnectorTest::TearDown()
{
}

void HiChainAuthConnectorTest::SetUpTestCase()
{
}

void HiChainAuthConnectorTest::TearDownTestCase()
{
}

class DmDeviceAuthCallbackTest : public IDmDeviceAuthCallback {
public:
    DmDeviceAuthCallbackTest() {}
    virtual ~DmDeviceAuthCallbackTest() {}
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override
    {
        (void)requestId;
        (void)data;
        (void)dataLen;
        return true;
    }
    void AuthDeviceFinish(int64_t requestId) override
    {
        (void)requestId;
    }
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override
    {
        (void)requestId;
        pinCode = errorCode;
    }
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override
    {
        (void)requestId;
        (void)sessionKey;
        (void)sessionKeyLen;
    }
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams) override
    {
        (void)requestId;
        (void)operationCode;
        (void)reqParams;
        return nullptr;
    }
private:
    int32_t pinCode = 0;
};

HWTEST_F(HiChainAuthConnectorTest, onFinish_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *returnData = nullptr;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onFinish(requestId, operationCode, returnData);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onError_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    int errorCode = 0;
    char *errorReturn = nullptr;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onError(requestId, operationCode, errorCode, errorReturn);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onSessionKeyReturned_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    uint8_t *sessionKey = nullptr;
    uint32_t sessionKeyLen = 0;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onSessionKeyReturned(requestId, sessionKey, sessionKeyLen);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onSessionKeyReturned_003, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    uint8_t *sessionKey = nullptr;
    uint32_t sessionKeyLen = 0;
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    hiChain_->onSessionKeyReturned(requestId, sessionKey, sessionKeyLen);
    EXPECT_NE(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, GenerateCredential_001, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    int32_t ret = hiChain_->GenerateCredential(localUdid, osAccountId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GenerateCredential_002, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = "{invalid_json}";
    int32_t ret = hiChain_->GenerateCredential(localUdid, osAccountId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GenerateCredential_003, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": "not_an_int", "publicKey": "key"})";
    int32_t ret = hiChain_->GenerateCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GenerateCredential_004, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 1, "publicKey": 1)";
    int32_t ret = hiChain_->GenerateCredential(localUdid, osAccountId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GenerateCredential_005, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 1, "publicKey": "key"})";
    int32_t ret = hiChain_->GenerateCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GenerateCredential_006, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 0, "publicKey": "key"})";
    int32_t ret = hiChain_->GenerateCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "2131351352";
    int32_t osAccountId = 0;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject["result"] = 15;
    jsonObject["publicKey"] = 0;
    std::string localUdid = jsonObject.Dump();
    int32_t osAccountId = 1245;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject["result"] = 15;
    jsonObject["publicKey"] = 0;
    std::string localUdid = jsonObject.Dump();
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = "{invalid_json}";
    int32_t osAccountId = 1245;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_FALSE(ret);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_004, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject["result"] = 15;
    jsonObject["publicKey"] = 0;
    std::string localUdid = jsonObject.Dump();
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": "not_an_int", "publicKey": "key"})";
    int32_t osAccountId = 1245;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_TRUE(ret);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_005, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject["result"] = 15;
    jsonObject["publicKey"] = 0;
    std::string localUdid = jsonObject.Dump();
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": -1, "publicKey": "key"})";
    int32_t osAccountId = 1245;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_TRUE(ret);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_006, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject["result"] = 15;
    jsonObject["publicKey"] = 0;
    std::string localUdid = jsonObject.Dump();
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 1, "publicKey": 0})";
    int32_t osAccountId = 1245;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_FALSE(ret);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_007, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject["result"] = 15;
    jsonObject["publicKey"] = 0;
    std::string localUdid = jsonObject.Dump();
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 1, "publicKey": "string"})";
    int32_t osAccountId = 1245;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_TRUE(ret);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_008, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject["result"] = 15;
    jsonObject["publicKey"] = 0;
    std::string localUdid = jsonObject.Dump();
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 2, "publicKey": "string"})";
    int32_t osAccountId = 1245;
    int32_t peerOsAccountId = -1;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId, peerOsAccountId);
    EXPECT_TRUE(ret);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_001, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "GADGFADEFGSDA";
    int32_t osAccountId = 0;
    std::string publicKey = "4165145615";
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_003, testing::ext::TestSize.Level1)
{
    std::string publicKey = "test";
    std::string localUdid = "test";
    int32_t osAccountId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = "{invalid_json}";
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_004, testing::ext::TestSize.Level1)
{
    std::string publicKey = "test";
    std::string localUdid = "test";
    int32_t osAccountId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": "not_an_int", "publicKey": "key"})";
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_005, testing::ext::TestSize.Level1)
{
    std::string publicKey = "test";
    std::string localUdid = "test";
    int32_t osAccountId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": -1, "publicKey": "key"})";
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_006, testing::ext::TestSize.Level1)
{
    std::string publicKey = "test";
    std::string localUdid = "test";
    int32_t osAccountId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 1, "publicKey": 0})";
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_007, testing::ext::TestSize.Level1)
{
    std::string publicKey = "test";
    std::string localUdid = "test";
    int32_t osAccountId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 1, "publicKey": "string"})";
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_008, testing::ext::TestSize.Level1)
{
    std::string publicKey = "test";
    std::string localUdid = "test";
    int32_t osAccountId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 2, "publicKey": "string"})";
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ImportCredential_001, testing::ext::TestSize.Level1)
{
    int32_t localUdid = 0;
    std::string deviceId;
    std::string publicKey;
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->ImportCredential(localUdid, peerUserId, deviceId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ImportCredential_002, testing::ext::TestSize.Level1)
{
    int32_t localUdid = 0;
    std::string deviceId = "4513541351";
    std::string publicKey = "42125143613";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->ImportCredential(localUdid, peerUserId, deviceId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ImportCredential_003, testing::ext::TestSize.Level1)
{
    int32_t localUdid = 0;
    std::string deviceId = "test";
    std::string publicKey = "test";
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = "{invalid_json}";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->ImportCredential(localUdid, peerUserId, deviceId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ImportCredential_004, testing::ext::TestSize.Level1)
{
    int32_t localUdid = 0;
    std::string deviceId = "test";
    std::string publicKey = "test";
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": "not_an_int"})";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->ImportCredential(localUdid, peerUserId, deviceId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ImportCredential_005, testing::ext::TestSize.Level1)
{
    int32_t localUdid = 0;
    std::string deviceId = "test";
    std::string publicKey = "test";
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": -1})";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->ImportCredential(localUdid, peerUserId, deviceId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ImportCredential_006, testing::ext::TestSize.Level1)
{
    int32_t localUdid = 0;
    std::string deviceId = "test";
    std::string publicKey = "test";
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 0})";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->ImportCredential(localUdid, peerUserId, deviceId, publicKey);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_001, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    int32_t userId = 0;
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->DeleteCredential(deviceId, userId, peerUserId);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_002, testing::ext::TestSize.Level1)
{
    std::string deviceId = "864513535";
    int32_t userId = 0;
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->DeleteCredential(deviceId, userId, peerUserId);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_003, testing::ext::TestSize.Level1)
{
    std::string deviceId = "test";
    int32_t userId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = "{invalid_json}";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->DeleteCredential(deviceId, userId, peerUserId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_004, testing::ext::TestSize.Level1)
{
    std::string deviceId = "test";
    int32_t userId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": "not_an_int"})";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->DeleteCredential(deviceId, userId, peerUserId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_005, testing::ext::TestSize.Level1)
{
    std::string deviceId = "test";
    int32_t userId = 0;
    g_processCredentialResultCode = HC_SUCCESS;
    g_processCredentialReturnDataStr = R"({"result": 100})";
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->DeleteCredential(deviceId, userId, peerUserId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ProcessCredData_001, testing::ext::TestSize.Level1)
{
    int64_t authReqId = 0;
    std::string data;
    int32_t ret = hiChain_->ProcessCredData(authReqId, data);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, AddCredential_001, testing::ext::TestSize.Level1)
{
    int32_t osAccountId = 0;
    std::string authParams = "authParamsTest";
    std::string credId;
    int32_t ret = hiChain_->AddCredential(osAccountId, authParams, credId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, ExportCredential_001, testing::ext::TestSize.Level1)
{
    int32_t osAccountId = 0;
    std::string credId = "credIdTest";
    std::string publicKey;
    int32_t ret = hiChain_->ExportCredential(osAccountId, credId, publicKey);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, AgreeCredential_001, testing::ext::TestSize.Level1)
{
    int32_t osAccountId = 0;
    std::string selfCredId = "selfCredIdTest";
    std::string authParams;
    std::string credId;
    int32_t ret = hiChain_->AgreeCredential(osAccountId, selfCredId, authParams, credId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, AuthCredentialPinCode_001, testing::ext::TestSize.Level1)
{
    int32_t osAccountId = 0;
    int64_t authReqId = 1;
    std::string pinCode = "233";
    int32_t ret = hiChain_->AuthCredentialPinCode(osAccountId, authReqId, pinCode);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredentialInfo_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    JsonObject queryParam;
    JsonObject resultJson;
    int32_t ret = hiChain_->QueryCredentialInfo(userId, queryParam, resultJson);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredInfoByCredId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    std::string credId;
    JsonObject resultJson;
    int32_t ret = hiChain_->QueryCredInfoByCredId(userId, credId, resultJson);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterHiChainAuthCallback_001
 * @tc.desc: 1. set dmDeviceAuthCallback_ not null
 *           2. call UnRegisterHiChainAuthCallback
 *           3. check dmDeviceAuthCallback_ is null
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, UnRegisterHiChainAuthCallback_001, testing::ext::TestSize.Level1)
{
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    ASSERT_NE(hiChain_->dmDeviceAuthCallback_, nullptr);
    int32_t ret = hiChain_->UnRegisterHiChainAuthCallback();
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

/**
 * @tc.name: RegisterHiChainAuthCallbackById_001
 * @tc.desc: 1. call RegisterHiChainAuthCallbackById with valid id and callback
 *           2. call GetDeviceAuthCallback with same id
 *           3. check callback not null
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, RegisterHiChainAuthCallbackById_001, testing::ext::TestSize.Level1)
{
    int64_t id = 100;
    auto callback = std::make_shared<DmDeviceAuthCallbackTest>();
    int32_t ret = hiChain_->RegisterHiChainAuthCallbackById(id, callback);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterHiChainAuthCallbackById_001
 * @tc.desc: 1. call RegisterHiChainAuthCallbackById with valid id and callback
 *           2. call UnRegisterHiChainAuthCallbackById with same id
 *           3. call UnRegisterHiChainAuthCallbackById with unregistered id
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, UnRegisterHiChainAuthCallbackById_001, testing::ext::TestSize.Level1)
{
    int64_t id = 200;
    auto callback = std::make_shared<DmDeviceAuthCallbackTest>();
    hiChain_->RegisterHiChainAuthCallbackById(id, callback);
    int32_t ret = hiChain_->UnRegisterHiChainAuthCallbackById(id);
    EXPECT_EQ(ret, DM_OK);
    // branch: id not found in map
    int32_t retNotFound = hiChain_->UnRegisterHiChainAuthCallbackById(999);
    EXPECT_EQ(retNotFound, DM_OK);
}

/**
 * @tc.name: onTransmit_004
 * @tc.desc: 1. call onTransmit with dataLen exceeding HICHAIN_DATA_SIZE
 *           2. check return false
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, onTransmit_004, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    uint8_t data[] = {1};
    uint32_t dataLen = 10241; // exceeds HICHAIN_DATA_SIZE(10240)
    bool ret = hiChain_->onTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: onSessionKeyReturned_004
 * @tc.desc: 1. call onSessionKeyReturned with sessionKeyLen exceeding HICHAIN_DATA_SIZE
 *           2. check early return branch
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, onSessionKeyReturned_004, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    uint8_t sessionKey[] = {1};
    uint32_t sessionKeyLen = 10241; // exceeds HICHAIN_DATA_SIZE(10240)
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    hiChain_->onSessionKeyReturned(requestId, sessionKey, sessionKeyLen);
    EXPECT_NE(hiChain_->dmDeviceAuthCallback_, nullptr);
}

/**
 * @tc.name: DeleteCredential_006
 * @tc.desc: 1. call DeleteCredential(osAccountId, credId) two-arg overload with non-empty credId
 *           2. check ret is not DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_006, testing::ext::TestSize.Level1)
{
    int32_t osAccountId = 0;
    std::string credId = "credIdTest";
    int32_t ret = hiChain_->DeleteCredential(osAccountId, credId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_007
 * @tc.desc: 1. call DeleteCredential(osAccountId, credId) two-arg overload with empty credId
 *           2. check ret is not DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_007, testing::ext::TestSize.Level1)
{
    int32_t osAccountId = 0;
    std::string credId;
    int32_t ret = hiChain_->DeleteCredential(osAccountId, credId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: AddTokensToCredential_001
 * @tc.desc: 1. call AddTokensToCredential with valid params
 *           2. check ret is not DM_OK (GetCredMgrInstance fails in UT)
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, AddTokensToCredential_001, testing::ext::TestSize.Level1)
{
    std::string credId = "credIdTest";
    int32_t userId = 0;
    std::vector<std::string> tokenIds = {"token1", "token2"};
    int32_t ret = hiChain_->AddTokensToCredential(credId, userId, tokenIds);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: UpdateCredential_001
 * @tc.desc: 1. call UpdateCredential with valid params
 *           2. check ret is not DM_OK (GetCredMgrInstance fails in UT)
 * @tc.type: FUNC
 */
HWTEST_F(HiChainAuthConnectorTest, UpdateCredential_001, testing::ext::TestSize.Level1)
{
    std::string credId = "credIdTest";
    int32_t userId = 0;
    std::vector<std::string> tokenIds = {"token1"};
    int32_t ret = hiChain_->UpdateCredential(credId, userId, tokenIds);
    EXPECT_NE(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS

extern "C" __attribute__((constructor)) DEVICE_AUTH_API_PUBLIC int32_t ProcessCredential(int32_t operationCode,
    const char *requestParams, char **returnData)
{
    if (requestParams == nullptr || returnData == nullptr) {
        return -1;
    }

    if (g_processCredentialReturnDataStr.empty()) {
        *returnData = nullptr;
        return -1;
    }

    char *charArray = strdup(g_processCredentialReturnDataStr.c_str());
    if (charArray == nullptr) {
        return -1;
    }

    *returnData = charArray;
    return g_processCredentialResultCode;
}
