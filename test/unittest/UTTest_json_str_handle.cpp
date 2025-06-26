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
void JsonStrHandleTest::SetUp()
{
}

void JsonStrHandleTest::TearDown()
{
}

void JsonStrHandleTest::SetUpTestCase()
{
}

void JsonStrHandleTest::TearDownTestCase()
{
}

namespace {
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
    std::vector<int64_t> tokenIdVec =
        JsonStrHandle::GetInstance().GetProxyTokenIdByExtra(extraInfo);
    EXPECT_EQ(tokenIdVec.empty(), true);
}

/**
 * @tc.name: GetProxyTokenIdByExtra_001
 * @tc.desc: GetProxyTokenIdByExtra
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(JsonStrHandleTest, GetProxyTokenIdByExtra_001, testing::ext::TestSize.Level1)
{
    std::string extraInfo = "";
    std::vector<int64_t> tokenIdVec =
        JsonStrHandle::GetInstance().GetProxyTokenIdByExtra(extraInfo);
    EXPECT_EQ(tokenIdVec.empty(), true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS