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
    int32_t GetPinCode(int32_t &code) override
    {
        if (pinCode == 0) {
            code = 0;
            return DM_OK;
        }
        code = ERR_DM_AUTH_NOT_START;
        return ERR_DM_AUTH_NOT_START;
    }
    void GetRemoteDeviceId(std::string &deviceId) override
    {
        (void)deviceId;
    }
private:
    int32_t pinCode = 0;
};

HWTEST_F(HiChainAuthConnectorTest, RegisterHiChainAuthCallback_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<IDmDeviceAuthCallback> callback = nullptr;
    int32_t ret = hiChain_->RegisterHiChainAuthCallback(callback);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, AuthDevice_001, testing::ext::TestSize.Level1)
{
    int32_t pinCode = 0;
    int32_t osAccountId = 0;
    std::string udid;
    int64_t requestId = 0;
    int32_t ret = hiChain_->AuthDevice(pinCode, osAccountId, udid, requestId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, ProcessAuthData_001, testing::ext::TestSize.Level1)
{
    int32_t requestId = 0;
    std::string authData;
    int32_t osAccountId = 0;
    int32_t ret = hiChain_->ProcessAuthData(requestId, authData, osAccountId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, onTransmit_001, testing::ext::TestSize.Level1)
{
    int32_t requestId = 0;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    bool ret = hiChain_->onTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, false);
}

HWTEST_F(HiChainAuthConnectorTest, onTransmit_002, testing::ext::TestSize.Level1)
{
    int32_t requestId = 0;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    bool ret = hiChain_->onTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, true);
}

HWTEST_F(HiChainAuthConnectorTest, onRequest_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *reqParams = nullptr;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onRequest(requestId, operationCode, reqParams);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onRequest_002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *reqParams = nullptr;
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    hiChain_->onRequest(requestId, operationCode, reqParams);
    EXPECT_NE(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onRequest_003, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    int32_t errorCode = -20024;
    char *reqParams = nullptr;
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    hiChain_->dmDeviceAuthCallback_->AuthDeviceError(requestId, errorCode);
    hiChain_->onRequest(requestId, operationCode, reqParams);
    EXPECT_NE(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onRequest_004, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *reqParams = nullptr;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<MockIDmDeviceAuthCallback> mockCallback = std::make_shared<MockIDmDeviceAuthCallback>();
    hiChainAuthConnector->dmDeviceAuthCallback_ = mockCallback;
    EXPECT_CALL(*mockCallback, GetPinCode(testing::_))
        .Times(1)
        .WillOnce(testing::Return(ERR_DM_FAILED));
    EXPECT_NE(hiChainAuthConnector->onRequest(requestId, operationCode, reqParams), nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onFinish_001, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *returnData = nullptr;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onFinish(requestId, operationCode, returnData);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onFinish_002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *returnData = nullptr;
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    hiChain_->onFinish(requestId, operationCode, returnData);
    EXPECT_NE(hiChain_->dmDeviceAuthCallback_, nullptr);
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

HWTEST_F(HiChainAuthConnectorTest, onError_002, testing::ext::TestSize.Level1)
{
    int64_t requestId = 0;
    int operationCode = 0;
    int errorCode = 0;
    char *errorReturn = nullptr;
    hiChain_->dmDeviceAuthCallback_ = std::make_shared<DmDeviceAuthCallbackTest>();
    hiChain_->onError(requestId, operationCode, errorCode, errorReturn);
    EXPECT_NE(hiChain_->dmDeviceAuthCallback_, nullptr);
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
    g_processCredentialReturnDataStr = "{invalid_json}";;
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
    std::string localUdid = SafetyDump(jsonObject);
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
    std::string localUdid = SafetyDump(jsonObject);
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
    std::string localUdid = SafetyDump(jsonObject);
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
    std::string localUdid = SafetyDump(jsonObject);
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
    std::string localUdid = SafetyDump(jsonObject);
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
    std::string localUdid = SafetyDump(jsonObject);
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
    std::string localUdid = SafetyDump(jsonObject);
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
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_002, testing::ext::TestSize.Level1)
{
    std::string deviceId = "864513535";
    int32_t userId = 0;
    int32_t peerUserId = 0;
    int32_t ret = hiChain_->DeleteCredential(deviceId, userId, peerUserId);
    EXPECT_EQ(ret, DM_OK);
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
