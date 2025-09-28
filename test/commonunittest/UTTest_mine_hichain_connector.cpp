/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "UTTest_mine_hichain_connector.h"

#include "dm_common_util.h"
#include "dm_constants.h"
#include "mine_hichain_connector.h"

namespace OHOS {
namespace DistributedHardware {
void MiniHiChainAuthConnectorTest::SetUp()
{
}

void MiniHiChainAuthConnectorTest::TearDown()
{
}

void MiniHiChainAuthConnectorTest::SetUpTestCase()
{
}

void MiniHiChainAuthConnectorTest::TearDownTestCase()
{
}

HWTEST_F(MiniHiChainAuthConnectorTest, onFinish_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    int64_t requestId = 159357462;
    int operationCode = 0;
    char *returnData = nullptr;
    minHiChain->onFinish(requestId, operationCode, returnData);
    EXPECT_EQ(minHiChain->IsCredentialExist(), false);
}

HWTEST_F(MiniHiChainAuthConnectorTest, onFinish_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    int64_t requestId = 0;
    int operationCode = 0;
    char *returnData = nullptr;
    minHiChain->onFinish(requestId, operationCode, returnData);
    EXPECT_EQ(minHiChain->IsCredentialExist(), false);
}

HWTEST_F(MiniHiChainAuthConnectorTest, onError_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    int64_t requestId = 159357462;
    int operationCode = 0;
    int errorCode = 0;
    char *returnData = nullptr;
    minHiChain->onError(requestId, operationCode, errorCode, returnData);
    EXPECT_EQ(minHiChain->IsCredentialExist(), false);
}

HWTEST_F(MiniHiChainAuthConnectorTest, onError_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    int64_t requestId = 0;
    int operationCode = 0;
    int errorCode = 0;
    char *returnData = nullptr;
    minHiChain->onError(requestId, operationCode, errorCode, returnData);
    EXPECT_EQ(minHiChain->IsCredentialExist(), false);
}

HWTEST_F(MiniHiChainAuthConnectorTest, Init_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    int32_t ret = minHiChain->Init();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniHiChainAuthConnectorTest, UnInit_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    int32_t ret = minHiChain->Init();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniHiChainAuthConnectorTest, DeleteCredentialAndGroup_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    int32_t ret = minHiChain->DeleteCredentialAndGroup();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniHiChainAuthConnectorTest, CreateGroup_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    std::string reqJsonStr;
    int32_t ret = minHiChain->CreateGroup(reqJsonStr);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MiniHiChainAuthConnectorTest, CreateGroup_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    JsonObject jsonObject;
    jsonObject["userId"] = 123456789;
    std::string reqJsonStr = jsonObject.Dump();
    int32_t ret = minHiChain->CreateGroup(reqJsonStr);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MiniHiChainAuthConnectorTest, CreateGroup_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    JsonObject jsonObject;
    jsonObject["userId"] = "123456789";
    std::string reqJsonStr = jsonObject.Dump();
    int32_t ret = minHiChain->CreateGroup(reqJsonStr);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MiniHiChainAuthConnectorTest, RequestCredential_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    JsonObject jsonObject;
    jsonObject["userId"] = "123456789";
    std::string returnJsonStr = jsonObject.Dump();
    int32_t ret = minHiChain->RequestCredential(returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniHiChainAuthConnectorTest, CheckCredential_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    std::string reqJsonStr;
    std::string returnJsonStr;
    int32_t ret = minHiChain->CheckCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MiniHiChainAuthConnectorTest, CheckCredential_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    std::string reqJsonStr = "reqJsonStr";
    std::string returnJsonStr;
    int32_t ret = minHiChain->CheckCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniHiChainAuthConnectorTest, ImportCredential_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    std::string reqJsonStr;
    std::string returnJsonStr;
    int32_t ret = minHiChain->ImportCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MiniHiChainAuthConnectorTest, ImportCredential_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    JsonObject jsonObject;
    jsonObject["userId"] = 123456789;
    std::string reqJsonStr = jsonObject.Dump();
    std::string returnJsonStr;
    int32_t ret = minHiChain->ImportCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_HICHAIN_REGISTER_CALLBACK);
}

HWTEST_F(MiniHiChainAuthConnectorTest, DeleteCredential_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    std::string reqJsonStr;
    std::string returnJsonStr;
    int32_t ret = minHiChain->DeleteCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MiniHiChainAuthConnectorTest, DeleteCredential_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    std::string reqJsonStr = "reqJsonStr";
    std::string returnJsonStr;
    int32_t ret = minHiChain->DeleteCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniHiChainAuthConnectorTest, IsCredentialExist_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MineHiChainConnector> minHiChain = std::make_shared<MineHiChainConnector>();
    bool ret = minHiChain->IsCredentialExist();
    EXPECT_EQ(ret, false);
}
} // namespace DistributedHardware
} // namespace OHOS
