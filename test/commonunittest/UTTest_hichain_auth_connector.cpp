/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "UTTest_hichain_auth_connector.h"

#include "dm_constants.h"
#include "hichain_auth_connector.h"

namespace OHOS {
namespace DistributedHardware {
void HiChainAuthConnectorTest::SetUp()
{
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

HWTEST_F(HiChainAuthConnectorTest, RegisterHiChainAuthCallback_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<IDmDeviceAuthCallback> callback = nullptr;
    int32_t ret = hiChain_->RegisterHiChainAuthCallback(callback);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, AuthDevice_001, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 0;
    int32_t osAccountId = 0;
    std::string udid;
    int64_t requestId = 0;
    int32_t ret = hiChain_->AuthDevice(pinCode, osAccountId, udid, requestId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, ProcessAuthData_001, testing::ext::TestSize.Level0)
{
    int32_t requestId = 0;
    std::string authData;
    int32_t osAccountId = 0;
    int32_t ret = hiChain_->ProcessAuthData(requestId, authData, osAccountId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(HiChainAuthConnectorTest, onTransmit_002, testing::ext::TestSize.Level0)
{
    int32_t requestId = 0;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    bool ret = hiChain_->onTransmit(requestId, data, dataLen);
    EXPECT_EQ(ret, false);
}

HWTEST_F(HiChainAuthConnectorTest, onRequest_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *reqParams = nullptr;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onRequest(requestId, operationCode, reqParams);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onFinish_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int operationCode = 0;
    char *returnData = nullptr;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onFinish(requestId, operationCode, returnData);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onError_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int operationCode = 0;
    int errorCode = 0;
    char *errorReturn = nullptr;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onError(requestId, operationCode, errorCode, errorReturn);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, onSessionKeyReturned_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    uint8_t *sessionKey = nullptr;
    uint32_t sessionKeyLen = 0;
    hiChain_->dmDeviceAuthCallback_ = nullptr;
    hiChain_->onSessionKeyReturned(requestId, sessionKey, sessionKeyLen);
    EXPECT_EQ(hiChain_->dmDeviceAuthCallback_, nullptr);
}

HWTEST_F(HiChainAuthConnectorTest, GenerateCredential_001, testing::ext::TestSize.Level0)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    int32_t ret = hiChain_->GenerateCredential(localUdid, osAccountId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, QueryCredential_001, testing::ext::TestSize.Level0)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    bool ret = hiChain_->QueryCredential(localUdid, osAccountId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(HiChainAuthConnectorTest, GetCredential_001, testing::ext::TestSize.Level0)
{
    std::string localUdid;
    int32_t osAccountId = 0;
    std::string publicKey;
    int32_t ret = hiChain_->GetCredential(localUdid, osAccountId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, ImportCredential_001, testing::ext::TestSize.Level0)
{
    int32_t localUdid = 0;
    std::string deviceId;
    std::string publicKey;
    int32_t ret = hiChain_->ImportCredential(localUdid, deviceId, publicKey);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(HiChainAuthConnectorTest, DeleteCredential_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    int32_t userId = 0;
    int32_t ret = hiChain_->DeleteCredential(deviceId, userId);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS