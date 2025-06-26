/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_app_manager_two.h"

#include "dm_error_type.h"

using namespace OHOS::AppExecFwk;
using namespace testing;

namespace OHOS {
namespace DistributedHardware {
void AppManagerTwoTest::SetUp()
{
}

void AppManagerTwoTest::TearDown()
{
}

void AppManagerTwoTest::SetUpTestCase()
{
}

void AppManagerTwoTest::TearDownTestCase()
{
}

HWTEST_F(AppManagerTwoTest, GetTokenIdByBundleName_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    std::string bundleName = "";
    int64_t tokenId = 0;
    int32_t ret = AppManager::GetInstance().GetTokenIdByBundleName(userId, bundleName, tokenId);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AppManagerTwoTest, GetTokenIdByBundleName_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    std::string bundleName = "bundleName";
    int64_t tokenId = 0;
    int32_t ret = AppManager::GetInstance().GetTokenIdByBundleName(userId, bundleName, tokenId);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AppManagerTwoTest, GetBundleNameByTokenId_001, testing::ext::TestSize.Level1)
{
    int64_t tokenId = -1;
    std::string bundleName = "bundleName";
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(tokenId, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTwoTest, GetBundleNameByTokenId_002, testing::ext::TestSize.Level1)
{
    int64_t tokenId = 0;
    std::string bundleName = "bundleName";
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(tokenId, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
} // namespace DistributedHardware
} // namespace OHOS