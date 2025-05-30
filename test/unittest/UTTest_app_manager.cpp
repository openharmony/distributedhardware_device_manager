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

HWTEST_F(AppManagerTest, RegisterCallerAppId_success_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(accountManager_ != nullptr);

    EXPECT_CALL(*skeleton_, GetCallingTokenID())
        .Times(INVOKE_COUNT)
        .WillOnce(Return(VALUABLE_TOKEN_ID));
    EXPECT_CALL(*skeleton_, GetCallingUid())
        .Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .Times(INVOKE_COUNT)
        .WillOnce(Return(systemAbilityManager));
    EXPECT_CALL(*accountManager_, GetForegroundOsAccountLocalId(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ERR_OK));
    EXPECT_CALL(*bundleMgr, GetNameForUid(_, _))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ERR_OK));
    BundleInfo bundleInfo;
    bundleInfo.appId = "mock_appId";
    EXPECT_CALL(*bundleMgr, GetBundleInfoV9(_, _, _, _))
        .Times(INVOKE_COUNT)
        .WillOnce(DoAll(SetArgReferee<ARG_THIRD>(bundleInfo), Return(ERR_OK)));
    std::string packageName = "packageName";
    AppManager::GetInstance().RegisterCallerAppId(packageName);

    std::string appId;
    auto result = AppManager::GetInstance().GetAppIdByPkgName(packageName, appId);
    EXPECT_EQ(result, DM_OK);
    EXPECT_STREQ(bundleInfo.appId.c_str(), appId.c_str());
    AppManager::GetInstance().UnRegisterCallerAppId(packageName);
}

HWTEST_F(AppManagerTest, RegisterCallerAppId_failed_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);

    std::string emptyPackageName;
    std::string appId;
    AppManager::GetInstance().RegisterCallerAppId(emptyPackageName);
    AppManager::GetInstance().GetAppIdByPkgName(emptyPackageName, appId);
    AppManager::GetInstance().UnRegisterCallerAppId(emptyPackageName);

    EXPECT_CALL(*skeleton_, GetCallingTokenID())
        .Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    std::string packageName = "packageName";
    AppManager::GetInstance().RegisterCallerAppId(packageName);
    auto result = AppManager::GetInstance().GetAppIdByPkgName(packageName, appId);
    EXPECT_EQ(result, ERR_DM_FAILED);
    AppManager::GetInstance().UnRegisterCallerAppId(packageName);
}

HWTEST_F(AppManagerTest, GetAppId_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);
    ASSERT_TRUE(client_ != nullptr);
    ASSERT_TRUE(accountManager_ != nullptr);

    EXPECT_CALL(*skeleton_, GetCallingTokenID()).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*skeleton_, GetCallingUid()).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillRepeatedly(Return(ATokenTypeEnum::TOKEN_HAP));

    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    size_t getSAMgrcallCount = 0;
    size_t getSACallCount = 0;
    size_t getForegroundOsAccountLocalIdCallCount = 0;
    size_t getBInfoCallCount = 0;
    ON_CALL(*client_, GetSystemAbilityManager())
        .WillByDefault(Invoke([&getSAMgrcallCount, systemAbilityManager]() -> sptr<ISystemAbilityManager> {
            return (getSAMgrcallCount++ == 0) ? nullptr : systemAbilityManager;
        }));
    ON_CALL(*systemAbilityManager, GetSystemAbility(_))
        .WillByDefault(Invoke([&getSACallCount, bundleMgr](int32_t) -> sptr<IRemoteObject> {
            return (getSACallCount++ == 0) ? nullptr : bundleMgr;
        }));
    ON_CALL(*accountManager_, GetForegroundOsAccountLocalId(_))
        .WillByDefault(Invoke([&getForegroundOsAccountLocalIdCallCount](int32_t &) -> ErrCode {
            return (getForegroundOsAccountLocalIdCallCount++ == 0) ? ERR_FAILED_VALUE : ERR_OK;
        }));
    ON_CALL(*bundleMgr, GetBundleInfoV9(_, _, _, _))
        .WillByDefault(Invoke([&getBInfoCallCount](const std::string &, int32_t, BundleInfo &, int32_t) -> ErrCode {
            return (getBInfoCallCount++ == 0) ? ERR_FAILED_VALUE : ERR_OK;
        }));
    EXPECT_CALL(*client_, GetSystemAbilityManager()).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_)).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*accountManager_, GetForegroundOsAccountLocalId(_)).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*bundleMgr, GetNameForUid(_, _)).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*bundleMgr, GetBundleInfoV9(_, _, _, _)).Times(AtLeast(INVOKE_COUNT));
    for (size_t i = 0; i < 5; ++i) {
        auto appId = AppManager::GetInstance().GetAppId();
        EXPECT_TRUE(appId.empty());
    }
}

HWTEST_F(AppManagerTest, IsSystemSA_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);
    size_t getCallingTokenIDCallCount = 0;
    size_t getTokenTypeFlagCallCount = 0;

    ON_CALL(*skeleton_, GetCallingTokenID())
        .WillByDefault(Invoke([&getCallingTokenIDCallCount]() {
            return (getCallingTokenIDCallCount++ == 0) ? UNVALUABLE_TOKEN_ID : VALUABLE_TOKEN_ID;
        }));
    ON_CALL(*token_, GetTokenTypeFlag(_))
        .WillByDefault(Invoke([&getTokenTypeFlagCallCount](AccessTokenID) -> ATokenTypeEnum {
            return (getTokenTypeFlagCallCount++ == 0) ? ATokenTypeEnum::TOKEN_HAP : ATokenTypeEnum::TOKEN_NATIVE;
        }));
    EXPECT_CALL(*skeleton_, GetCallingTokenID()).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).Times(AtLeast(INVOKE_COUNT));
    for (size_t i = 0; i < 3; ++i) {
        auto ret = AppManager::GetInstance().IsSystemSA();
        if (getCallingTokenIDCallCount > 1 && getTokenTypeFlagCallCount > 1) {
            EXPECT_TRUE(ret);
        } else {
            EXPECT_FALSE(ret);
        }
    }
}

HWTEST_F(AppManagerTest, GetCallerName_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(skeleton_ != nullptr);
    ASSERT_TRUE(token_ != nullptr);
    size_t getCallingTokenIDCallCount = 0;
    size_t getHapTokenInfoCallCount = 0;

    ON_CALL(*skeleton_, GetCallingTokenID())
        .WillByDefault(Invoke([&getCallingTokenIDCallCount]() {
            return (getCallingTokenIDCallCount++ == 0) ? UNVALUABLE_TOKEN_ID : VALUABLE_TOKEN_ID;
        }));
    ON_CALL(*token_, GetHapTokenInfo(_, _))
        .WillByDefault(Invoke([&getHapTokenInfoCallCount](AccessTokenID, HapTokenInfo &) {
            return (getHapTokenInfoCallCount++ == 0) ? ERR_FAILED_VALUE : ERR_OK;
        }));
    EXPECT_CALL(*skeleton_, GetCallingTokenID()).Times(AtLeast(INVOKE_COUNT));
    EXPECT_CALL(*token_, GetTokenTypeFlag(_)).WillRepeatedly(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*token_, GetHapTokenInfo(_, _)).Times(AtLeast(INVOKE_COUNT));
    for (size_t i = 0; i < 3; ++i) {
        bool isSystemSA = false;
        std::string output;
        auto ret = AppManager::GetInstance().GetCallerName(isSystemSA, output);
        if (getCallingTokenIDCallCount > 1 && getHapTokenInfoCallCount > 1) {
            EXPECT_EQ(ret, DM_OK);
        } else {
            EXPECT_EQ(ret, ERR_DM_FAILED);
        }
    }
}

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
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(tokenId, VALUABLE_TOKEN_ID);
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

HWTEST_F(AppManagerTest, GetBundleNameForSelf_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(client_ != nullptr);
    auto bundleMgr = sptr<BundleMgrMock>(new (std::nothrow) BundleMgrMock());
    auto systemAbilityManager = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*systemAbilityManager, GetSystemAbility(_))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(bundleMgr));
    EXPECT_CALL(*client_, GetSystemAbilityManager())
        .Times(INVOKE_COUNT)
        .WillOnce(Return(systemAbilityManager));
    EXPECT_CALL(*bundleMgr, GetBundleInfoForSelf(_, _))
        .Times(INVOKE_COUNT)
        .WillOnce(Return(ERR_OK));

    std::string bundleName = "";
    auto result = AppManager::GetInstance().GetBundleNameForSelf(bundleName);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(bundleName, "");
}
} // namespace DistributedHardware
} // namespace OHOS
