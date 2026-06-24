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

#include "UTTest_app_manager.h"
#include "bundle_mgr_mock.h"
#include "dm_constants.h"
#include "dm_system_ability_manager_mock.h"

using namespace OHOS::AppExecFwk;
using namespace testing;

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr size_t ARG_THIRD = 2;
constexpr size_t INVOKE_COUNT = 1;
constexpr size_t ERR_FAILED_VALUE = 11600101;
constexpr uint32_t VALUABLE_TOKEN_ID = 153;
constexpr uint32_t UNVALUABLE_TOKEN_ID = 0;
} // namespace
void AppManagerTest::SetUp()
{
    auto skeleton = IPCSkeletonInterface::GetOrCreateIPCSkeleton();
    skeleton_ = std::static_pointer_cast<IPCSkeletonMock>(skeleton);
    auto token = AccessTokenKitInterface::GetOrCreateAccessTokenKit();
    token_ = std::static_pointer_cast<AccessTokenKitMock>(token);
    auto client = ISystemAbilityManagerClient::GetOrCreateSAMgrClient();
    client_ = std::static_pointer_cast<SystemAbilityManagerClientMock>(client);
    auto accountManager = IOsAccountManager::GetOrCreateOsAccountManager();
    accountManager_ = std::static_pointer_cast<OsAccountManagerMock>(accountManager);
}

void AppManagerTest::TearDown()
{
    IPCSkeletonInterface::ReleaseIPCSkeleton();
    AccessTokenKitInterface::ReleaseAccessTokenKit();
    ISystemAbilityManagerClient::ReleaseSAMgrClient();
    IOsAccountManager::ReleaseAccountManager();
    skeleton_ = nullptr;
    token_ = nullptr;
    client_ = nullptr;
    accountManager_ = nullptr;
}

void AppManagerTest::SetUpTestCase()
{}

void AppManagerTest::TearDownTestCase()
{}

HWTEST_F(AppManagerTest, GetCallerName_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);

    EXPECT_CALL(*skeleton_, GetCallingTokenID()).WillRepeatedly(Return(VALUABLE_TOKEN_ID));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*token_, GetNativeTokenInfo(_, _)).WillOnce(Return(ERR_FAILED_VALUE));
    bool isSystemSA = true;
    std::string output;
    auto ret = AppManager::GetInstance().GetCallerName(isSystemSA, output);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_INVALID));
    ret = AppManager::GetInstance().GetCallerName(isSystemSA, output);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTest, GetNativeTokenIdByName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(token_ != nullptr);

    EXPECT_CALL(*token_, GetNativeTokenId(_)).WillOnce(Return(UNVALUABLE_TOKEN_ID));
    std::string processName;
    int64_t tokenId = 0;
    auto ret = AppManager::GetInstance().GetNativeTokenIdByName(processName, tokenId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*token_, GetNativeTokenId(_)).WillOnce(Return(VALUABLE_TOKEN_ID));
    ret = AppManager::GetInstance().GetNativeTokenIdByName(processName, tokenId);
    EXPECT_NE(ret, ERR_DM_DISCOVERY_FAILED);
    EXPECT_EQ(tokenId, DM_OK);
}

HWTEST_F(AppManagerTest, GetHapTokenIdByName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(token_ != nullptr);
    EXPECT_CALL(*token_, GetHapTokenID(_, _, _)).WillOnce(Return(UNVALUABLE_TOKEN_ID));
    int32_t userId = 0;
    std::string bundleName;
    int32_t instIndex = 0;
    int64_t tokenId = 0;
    auto ret = AppManager::GetInstance().GetHapTokenIdByName(userId, bundleName, instIndex, tokenId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*token_, GetHapTokenID(_, _, _)).WillOnce(Return(VALUABLE_TOKEN_ID));
    ret = AppManager::GetInstance().GetHapTokenIdByName(userId, bundleName, instIndex, tokenId);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(tokenId, VALUABLE_TOKEN_ID);
}

HWTEST_F(AppManagerTest, GetCallerProcessName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);
    size_t getCallingTokenIDCallCount = 0;
    size_t getNativeTokenInfoCallCount = 0;

    ON_CALL(*skeleton_, GetCallingTokenID())
        .WillByDefault(Invoke([&getCallingTokenIDCallCount]() {
            return (getCallingTokenIDCallCount++ == 0) ? UNVALUABLE_TOKEN_ID : VALUABLE_TOKEN_ID;
        }));
    ON_CALL(*token_, GetNativeTokenInfo(_, _))
        .WillByDefault(Invoke([&getNativeTokenInfoCallCount](AccessTokenID, NativeTokenInfo &) {
            return (getNativeTokenInfoCallCount++ == 0) ? ERR_FAILED_VALUE : ERR_OK;
        }));
    EXPECT_CALL(*skeleton_, GetCallingTokenID()).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillRepeatedly(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*token_, GetNativeTokenInfo(_, _)).Times(AtLeast(INVOKE_COUNT));
    for (size_t i = 0; i < 3; ++i) {
        std::string output;
        auto ret = AppManager::GetInstance().GetCallerProcessName(output);
        if (getCallingTokenIDCallCount > 1 && getNativeTokenInfoCallCount > 1) {
            EXPECT_EQ(ret, DM_OK);
        } else {
            EXPECT_EQ(ret, ERR_DM_FAILED);
        }
    }
}

HWTEST_F(AppManagerTest, GetCallerProcessName_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);
    size_t GetHapTokenInfoCallCount = 0;

    ON_CALL(*token_, GetHapTokenInfo(_, _))
        .WillByDefault(Invoke([&GetHapTokenInfoCallCount](AccessTokenID, HapTokenInfo &) {
            return (GetHapTokenInfoCallCount++ == 0) ? ERR_FAILED_VALUE : ERR_OK;
        }));

    EXPECT_CALL(*skeleton_, GetCallingTokenID()).WillRepeatedly(Return(VALUABLE_TOKEN_ID));
    EXPECT_CALL(*skeleton_, GetCallingFullTokenID()).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillRepeatedly(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*token_, GetHapTokenInfo(_, _)).Times(AtLeast(INVOKE_COUNT));
    std::string output;
    for (size_t i = 0; i < 2; ++i) {
        auto ret = AppManager::GetInstance().GetCallerProcessName(output);
        EXPECT_EQ(ret, ERR_DM_FAILED);
    }

    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_INVALID));
    auto ret = AppManager::GetInstance().GetCallerProcessName(output);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTest, GetBundleNameForSelf_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(client_ != nullptr);

    // Test GetBundleManagerProxy failed - return nullptr for systemAbilityManager
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .Times(INVOKE_COUNT)
        .WillOnce(Return(nullptr));

    std::string bundleName = "";
    auto result = AppManager::GetInstance().GetBundleNameForSelf(bundleName);
    EXPECT_EQ(result, ERR_DM_GET_BMS_FAILED);
}

HWTEST_F(AppManagerTest, GetCallerName_003, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);

    // Test TOKEN_NATIVE success branch
    EXPECT_CALL(*skeleton_, GetCallingTokenID()).WillRepeatedly(Return(VALUABLE_TOKEN_ID));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));

    NativeTokenInfo nativeTokenInfo;
    nativeTokenInfo.processName = "test_process";
    EXPECT_CALL(*token_, GetNativeTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(nativeTokenInfo), Return(ERR_OK)));

    bool isSystemSA = false;
    std::string output;
    auto ret = AppManager::GetInstance().GetCallerName(isSystemSA, output);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(output, "test_process");
}

HWTEST_F(AppManagerTest, GetCallerProcessName_003, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);

    // Test TOKEN_HAP success branch - system app
    EXPECT_CALL(*skeleton_, GetCallingTokenID()).WillRepeatedly(Return(VALUABLE_TOKEN_ID));
    EXPECT_CALL(*skeleton_, GetCallingFullTokenID()).WillOnce(Return(VALUABLE_TOKEN_ID));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));

    HapTokenInfo hapTokenInfo;
    hapTokenInfo.bundleName = "test_bundle";
    EXPECT_CALL(*token_, GetHapTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(hapTokenInfo), Return(ERR_OK)));

    std::string output;
    auto ret = AppManager::GetInstance().GetCallerProcessName(output);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    EXPECT_EQ(output, "test_bundle");
}

HWTEST_F(AppManagerTest, IsSystemApp_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(skeleton_ != nullptr);

    // Test with valid system app token ID
    uint64_t fullSystemAppTokenId = 0x01000000; // System app token ID pattern
    EXPECT_CALL(*skeleton_, GetCallingFullTokenID())
        .Times(INVOKE_COUNT)
        .WillOnce(Return(fullSystemAppTokenId));

    auto ret = AppManager::GetInstance().IsSystemApp();
    EXPECT_FALSE(ret);
}

HWTEST_F(AppManagerTest, GetBundleNameByTokenId_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(token_ != nullptr);

    // negative tokenId guard -> ERR_DM_FAILED
    std::string bundleName;
    int64_t tokenId = -1;
    auto ret = AppManager::GetInstance().GetBundleNameByTokenId(tokenId, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTest, GetBundleNameByTokenId_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(token_ != nullptr);

    // TOKEN_HAP success branch
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    HapTokenInfo tokenInfo;
    tokenInfo.bundleName = "hap_bundle";
    EXPECT_CALL(*token_, GetHapTokenInfo(_, _))
        .Times(INVOKE_COUNT)
        .WillOnce(DoAll(SetArgReferee<1>(tokenInfo), Return(ERR_OK)));
    std::string bundleName;
    auto ret = AppManager::GetInstance().GetBundleNameByTokenId(VALUABLE_TOKEN_ID, bundleName);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(bundleName, "hap_bundle");
}

HWTEST_F(AppManagerTest, GetBundleNameByTokenId_003, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(token_ != nullptr);

    // TOKEN_HAP failure branch
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*token_, GetHapTokenInfo(_, _))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ERR_FAILED_VALUE));
    std::string bundleName;
    auto ret = AppManager::GetInstance().GetBundleNameByTokenId(VALUABLE_TOKEN_ID, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTest, GetBundleNameByTokenId_004, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(token_ != nullptr);

    // TOKEN_NATIVE success branch
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    NativeTokenInfo tokenInfo;
    tokenInfo.processName = "native_proc";
    EXPECT_CALL(*token_, GetNativeTokenInfo(_, _))
        .Times(INVOKE_COUNT)
        .WillOnce(DoAll(SetArgReferee<1>(tokenInfo), Return(ERR_OK)));
    std::string bundleName;
    auto ret = AppManager::GetInstance().GetBundleNameByTokenId(VALUABLE_TOKEN_ID, bundleName);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(bundleName, "native_proc");
}

HWTEST_F(AppManagerTest, GetBundleNameByTokenId_005, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(token_ != nullptr);

    // TOKEN_NATIVE failure branch
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*token_, GetNativeTokenInfo(_, _))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ERR_FAILED_VALUE));
    std::string bundleName;
    auto ret = AppManager::GetInstance().GetBundleNameByTokenId(VALUABLE_TOKEN_ID, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTest, GetBundleNameByTokenId_006, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(token_ != nullptr);

    // unsupported token type (else branch)
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_INVALID));
    std::string bundleName;
    auto ret = AppManager::GetInstance().GetBundleNameByTokenId(VALUABLE_TOKEN_ID, bundleName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTest, GetTokenIdByBundleName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(token_ != nullptr);

    // native token id found first -> returns DM_OK via GetNativeTokenIdByName path
    EXPECT_CALL(*token_, GetNativeTokenId(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(VALUABLE_TOKEN_ID));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    int32_t userId = 1001;
    std::string bundleName = "nativeBundle";
    int64_t tokenId = 0;
    auto ret = AppManager::GetInstance().GetTokenIdByBundleName(userId, bundleName, tokenId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AppManagerTest, GetTokenIdByBundleName_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(token_ != nullptr);

    // native lookup fails -> hap fallback succeeds
    EXPECT_CALL(*token_, GetNativeTokenId(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(UNVALUABLE_TOKEN_ID));
    EXPECT_CALL(*token_, GetHapTokenID(_, _, _))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(VALUABLE_TOKEN_ID));
    int32_t userId = 1001;
    std::string bundleName = "hapBundle";
    int64_t tokenId = 0;
    auto ret = AppManager::GetInstance().GetTokenIdByBundleName(userId, bundleName, tokenId);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(tokenId, VALUABLE_TOKEN_ID);
}

HWTEST_F(AppManagerTest, GetTokenIdByBundleName_003, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(token_ != nullptr);

    // both native and hap lookup fail -> ERR_DM_FAILED
    EXPECT_CALL(*token_, GetNativeTokenId(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(UNVALUABLE_TOKEN_ID));
    EXPECT_CALL(*token_, GetHapTokenID(_, _, _))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(UNVALUABLE_TOKEN_ID));
    int32_t userId = 1001;
    std::string bundleName = "unknownBundle";
    int64_t tokenId = 0;
    auto ret = AppManager::GetInstance().GetTokenIdByBundleName(userId, bundleName, tokenId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(AppManagerTest, GetAppIdByPkgName_002, testing::ext::TestSize.Level1)
{
    // empty pkgName early-return guard -> ERR_DM_INPUT_PARA_INVALID
    std::string emptyPkgName;
    std::string appId;
    int32_t userId = 1001;
    auto ret = AppManager::GetInstance().GetAppIdByPkgName(emptyPkgName, appId, userId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // namespace DistributedHardware
} // namespace OHOS
