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

#include "accesstoken_kit_mock.h"
#include "dm_error_type.h"

using namespace OHOS::AppExecFwk;
using namespace testing;

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr size_t ERR_FAILED_VALUE = 11600101;
} // namespace
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
    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    auto tokenMock = std::static_pointer_cast<AccessTokenKitMock>(token);
    EXPECT_CALL(*tokenMock, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_INVALID));
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(tokenId, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    AccessTokenKitInterface::ReleaseAccessTokenKit();
}

HWTEST_F(AppManagerTwoTest, GetBundleNameByTokenId_003, testing::ext::TestSize.Level1)
{
    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    auto tokenMock = std::static_pointer_cast<AccessTokenKitMock>(token);

    // Test TOKEN_HAP success branch
    constexpr int64_t validHapTokenId = 153;
    EXPECT_CALL(*tokenMock, GetTokenTypeFlag(_))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));

    HapTokenInfo hapTokenInfo;
    hapTokenInfo.bundleName = "test_hap_bundle";
    EXPECT_CALL(*tokenMock, GetHapTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(hapTokenInfo), Return(ERR_OK)));

    std::string bundleName;
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(validHapTokenId, bundleName);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(bundleName, "test_hap_bundle");

    AccessTokenKitInterface::ReleaseAccessTokenKit();
}

HWTEST_F(AppManagerTwoTest, GetBundleNameByTokenId_004, testing::ext::TestSize.Level1)
{
    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    auto tokenMock = std::static_pointer_cast<AccessTokenKitMock>(token);

    // Test TOKEN_HAP branch but GetHapTokenInfo failed
    constexpr int64_t validHapTokenId = 153;
    EXPECT_CALL(*tokenMock, GetTokenTypeFlag(_))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*tokenMock, GetHapTokenInfo(_, _))
        .WillOnce(Return(ERR_FAILED_VALUE));

    std::string bundleName;
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(validHapTokenId, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    AccessTokenKitInterface::ReleaseAccessTokenKit();
}

HWTEST_F(AppManagerTwoTest, GetBundleNameByTokenId_005, testing::ext::TestSize.Level1)
{
    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    auto tokenMock = std::static_pointer_cast<AccessTokenKitMock>(token);

    // Test TOKEN_NATIVE success branch
    constexpr int64_t validNativeTokenId = 153;
    EXPECT_CALL(*tokenMock, GetTokenTypeFlag(_))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));

    NativeTokenInfo nativeTokenInfo;
    nativeTokenInfo.processName = "test_native_process";
    EXPECT_CALL(*tokenMock, GetNativeTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(nativeTokenInfo), Return(ERR_OK)));

    std::string bundleName;
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(validNativeTokenId, bundleName);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(bundleName, "test_native_process");

    AccessTokenKitInterface::ReleaseAccessTokenKit();
}

HWTEST_F(AppManagerTwoTest, GetBundleNameByTokenId_006, testing::ext::TestSize.Level1)
{
    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    auto tokenMock = std::static_pointer_cast<AccessTokenKitMock>(token);

    // Test TOKEN_NATIVE branch but GetNativeTokenInfo failed
    constexpr int64_t validNativeTokenId = 153;
    EXPECT_CALL(*tokenMock, GetTokenTypeFlag(_))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*tokenMock, GetNativeTokenInfo(_, _))
        .WillOnce(Return(ERR_FAILED_VALUE));

    std::string bundleName;
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(validNativeTokenId, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    AccessTokenKitInterface::ReleaseAccessTokenKit();
}

HWTEST_F(AppManagerTwoTest, GetBundleNameByTokenId_007, testing::ext::TestSize.Level1)
{
    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    auto tokenMock = std::static_pointer_cast<AccessTokenKitMock>(token);

    // Test TOKEN_INVALID branch
    constexpr int64_t validTokenId = 153;
    EXPECT_CALL(*tokenMock, GetTokenTypeFlag(_))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_INVALID));

    std::string bundleName;
    int32_t ret = AppManager::GetInstance().GetBundleNameByTokenId(validTokenId, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    AccessTokenKitInterface::ReleaseAccessTokenKit();
}
} // namespace DistributedHardware
} // namespace OHOS