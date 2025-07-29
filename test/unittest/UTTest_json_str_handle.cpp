/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_json_str_handle.h"

#include <algorithm>
#include <thread>
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {

namespace {
const std::string TAG_PEER_BUNDLE_NAME = "peerBundleName";
const std::string TAG_PEER_TOKENID = "peerTokenId";
const std::string TAG_PROXY = "proxy";
}  // namespace

void JsonStrHandleTest::SetUp() {}

void JsonStrHandleTest::TearDown() {}

void JsonStrHandleTest::SetUpTestCase() {}

void JsonStrHandleTest::TearDownTestCase() {}

/**
 * @tc.name: GetPeerAppInfoParseExtra_001
 * @tc.desc: GetPeerAppInfoParseExtra
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(JsonStrHandleTest, GetPeerAppInfoParseExtra_001, testing::ext::TestSize.Level1)
{
    std::string extra = "";
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    JsonStrHandle::GetInstance().GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);

    extra = "extra";
    peerTokenId = 1;
    peerBundleName = "peerBundleName";
    JsonStrHandle::GetInstance().GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    std::string extraInfo = "extraInfo";
    std::vector<int64_t> tokenIdVec = JsonStrHandle::GetInstance().GetProxyTokenIdByExtra(extraInfo);
    EXPECT_EQ(tokenIdVec.empty(), true);
}

/**
 * @tc.name: GetPeerAppInfoParseExtra_002
 * @tc.desc: Test GetPeerAppInfoParseExtra
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonStrHandleTest, GetPeerAppInfoParseExtra_002, testing::ext::TestSize.Level1)
{
    std::string extra = "{\"" + TAG_PEER_TOKENID + "\": 12345}";
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    handle->GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    EXPECT_EQ(peerBundleName, "");
}

/**
 * @tc.name: GetPeerAppInfoParseExtra_003
 * @tc.desc: Test GetPeerAppInfoParseExtra
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonStrHandleTest, GetPeerAppInfoParseExtra_003, testing::ext::TestSize.Level1)
{
    std::string extra = "{\"" + TAG_PEER_BUNDLE_NAME + "\": \"testBundle\"}";
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    handle->GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    EXPECT_EQ(peerTokenId, 0);
}

/**
 * @tc.name: GetPeerAppInfoParseExtra_004
 * @tc.desc: Test GetPeerAppInfoParseExtra
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonStrHandleTest, GetPeerAppInfoParseExtra_004, testing::ext::TestSize.Level1)
{
    std::string extra = "{\"" + TAG_PEER_BUNDLE_NAME + "\": 123, \"" + TAG_PEER_TOKENID + "\": 456}";
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    handle->GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    EXPECT_EQ(peerBundleName, "");
}

/**
 * @tc.name: GetPeerAppInfoParseExtra_005
 * @tc.desc: Test GetPeerAppInfoParseExtra
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonStrHandleTest, GetPeerAppInfoParseExtra_005, testing::ext::TestSize.Level1)
{
    std::string extra = "{\"" + TAG_PEER_BUNDLE_NAME + "\": \"testBundle\", \"" + TAG_PEER_TOKENID + "\": \"123\"}";
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    handle->GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    EXPECT_EQ(peerTokenId, 0);
}

/**
 * @tc.name: GetPeerAppInfoParseExtra_006
 * @tc.desc: Test GetPeerAppInfoParseExtra
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonStrHandleTest, GetPeerAppInfoParseExtra_006, testing::ext::TestSize.Level1)
{
    std::string extra = "{\"" + TAG_PEER_BUNDLE_NAME + "\": \"testBundle\", \"" + TAG_PEER_TOKENID + "\": 12345}";
    uint64_t peerTokenId = 0;
    std::string peerBundleName = "";
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    handle->GetPeerAppInfoParseExtra(extra, peerTokenId, peerBundleName);
    EXPECT_EQ(peerTokenId, 12345);
    EXPECT_EQ(peerBundleName, "testBundle");
}

/**
 * @tc.name: GetProxyTokenIdByExtra_001
 * @tc.desc: Test GetProxyTokenIdByExtra with empty extraInfo
 * @tc.type: FUNC
 * @tc.require: AR00000000
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    std::string emptyExtra;
    std::vector<int64_t> result = handle->GetProxyTokenIdByExtra(emptyExtra);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetProxyTokenIdByExtra_002
 * @tc.desc: Test GetProxyTokenIdByExtra with invalid JSON string
 * @tc.type: FUNC
 * @tc.require: AR00000000
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    std::string invalidJson = "{invalid}";
    std::vector<int64_t> result = handle->GetProxyTokenIdByExtra(invalidJson);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetProxyTokenIdByExtra_003
 * @tc.desc: Test GetProxyTokenIdByExtra with JSON missing proxy tag
 * @tc.type: FUNC
 * @tc.require: AR00000000
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    std::string noProxyTag = "{\"other\": \"value\"}";
    std::vector<int64_t> result = handle->GetProxyTokenIdByExtra(noProxyTag);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetProxyTokenIdByExtra_004
 * @tc.desc: Test GetProxyTokenIdByExtra with invalid proxy list string
 * @tc.type: FUNC
 * @tc.require: AR00000000
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    std::string invalidProxyList = "{\"" + TAG_PROXY + "\": \"not_a_valid_json_array\"}";
    std::vector<int64_t> result = handle->GetProxyTokenIdByExtra(invalidProxyList);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetProxyTokenIdByExtra_005
 * @tc.desc: Test GetProxyTokenIdByExtra with empty proxy list
 * @tc.type: FUNC
 * @tc.require: AR00000000
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    std::string emptyProxyList = "{\"" + TAG_PROXY + "\": \"[]\"}";
    std::vector<int64_t> result = handle->GetProxyTokenIdByExtra(emptyProxyList);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetProxyTokenIdByExtra_006
 * @tc.desc: Test GetProxyTokenIdByExtra with valid number list
 * @tc.type: FUNC
 * @tc.require: AR00000000
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    std::string validProxyList = "{\"" + TAG_PROXY + "\": \"[123, 456, 789]\"}";
    std::vector<int64_t> result = handle->GetProxyTokenIdByExtra(validProxyList);
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 123);
    EXPECT_EQ(result[1], 456);
    EXPECT_EQ(result[2], 789);
}

/**
 * @tc.name: GetProxyTokenIdByExtra_007
 * @tc.desc: Test GetProxyTokenIdByExtra with mixed type list
 * @tc.type: FUNC
 * @tc.require: AR00000000
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<JsonStrHandle> handle = std::make_shared<JsonStrHandle>();
    std::string mixedProxyList = "{\"" + TAG_PROXY + "\": \"[123, 456, null, 789]\"}";
    std::vector<int64_t> result = handle->GetProxyTokenIdByExtra(mixedProxyList);
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 123);
    EXPECT_EQ(result[1], 456);
    EXPECT_EQ(result[2], 789);
}
}  // namespace DistributedHardware
}  // namespace OHOS
