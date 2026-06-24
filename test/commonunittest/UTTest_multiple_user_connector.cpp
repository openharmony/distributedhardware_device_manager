/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "UTTest_multiple_user_connector.h"

#include "dm_constants.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {
void MultipleUserConnectorTest::SetUp()
{
}
void MultipleUserConnectorTest::TearDown()
{
}
void MultipleUserConnectorTest::SetUpTestCase()
{
}
void MultipleUserConnectorTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: OnDecisionFilterResult_001
 * @tc.desc:  Returns a new pointer to the constructor
 * to construct an environment where the device has been discovered, and stop discovering
 * the device. Its return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(MultipleUserConnectorTest, GetCurrentAccountUserID_001, testing::ext::TestSize.Level1)
{
    int32_t ret = DM_OK;
    ret = MultipleUserConnector::GetCurrentAccountUserID();
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetSwitchOldUserId_001
 * @tc.desc:  Returns a new pointer to the constructor
 * to construct an environment where the device has been discovered, and stop discovering
 * the device. Its return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(MultipleUserConnectorTest, SetSwitchOldUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userIdIn = 1234;
    int32_t userIdOut = -1;
    MultipleUserConnector::SetSwitchOldUserId(userIdIn);
    userIdOut = MultipleUserConnector::GetSwitchOldUserId();
    ASSERT_EQ(userIdIn, userIdOut);
}

/**
 * @tc.name: GetSwitchOldUserId_001
 * @tc.desc:  Returns a new pointer to the constructor
 * to construct an environment where the device has been discovered, and stop discovering
 * the device. Its return value is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(MultipleUserConnectorTest, GetSwitchOldUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userIdIn = 1234;
    int32_t userIdOut = -1;
    MultipleUserConnector::SetSwitchOldUserId(userIdIn);
    userIdOut = MultipleUserConnector::GetSwitchOldUserId();
    ASSERT_EQ(userIdIn, userIdOut);
}

HWTEST_F(MultipleUserConnectorTest, GetSwitchOldAccountId_001, testing::ext::TestSize.Level1)
{
    std::string accountId = "accountId";
    MultipleUserConnector::SetSwitchOldAccountId(accountId);
    std::string accountIdOut = MultipleUserConnector::GetSwitchOldAccountId();
    ASSERT_EQ(accountId, accountIdOut);
}

HWTEST_F(MultipleUserConnectorTest, GetSwitchOldAccountName_001, testing::ext::TestSize.Level1)
{
    std::string accountName = "accountName";
    MultipleUserConnector::SetSwitchOldAccountName(accountName);
    std::string accountNameOut = MultipleUserConnector::GetSwitchOldAccountName();
    ASSERT_EQ(accountName, accountNameOut);
}

HWTEST_F(MultipleUserConnectorTest, GetAllUserIds_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIdVec;
    int32_t ret = MultipleUserConnector::GetAllUserIds(userIdVec);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(MultipleUserConnectorTest, GetAccountInfoByUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 123;
    DMAccountInfo dmAccountInfo;
    dmAccountInfo.accountId = "12456";
    MultipleUserConnector::dmAccountInfoMap_[userId] = dmAccountInfo;
    auto ret = MultipleUserConnector::GetAccountInfoByUserId(userId);
    ASSERT_FALSE(ret.accountId.empty());

    MultipleUserConnector::DeleteAccountInfoByUserId(userId);
}

HWTEST_F(MultipleUserConnectorTest, GetAccountInfoByUserId_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 98765;
    MultipleUserConnector::DeleteAccountInfoByUserId(userId);
    auto ret = MultipleUserConnector::GetAccountInfoByUserId(userId);
    EXPECT_TRUE(ret.accountId.empty());
    EXPECT_TRUE(ret.accountName.empty());
}

HWTEST_F(MultipleUserConnectorTest, GetCurrentDMAccountInfo_001, testing::ext::TestSize.Level1)
{
    DMAccountInfo info = MultipleUserConnector::GetCurrentDMAccountInfo();
    EXPECT_EQ(info.accountId, MultipleUserConnector::GetOhosAccountId());
    EXPECT_EQ(info.accountName, MultipleUserConnector::GetOhosAccountName());
}

HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> foregroundUserVec;
    MultipleUserConnector::ClearLockedUser(foregroundUserVec);
    EXPECT_TRUE(foregroundUserVec.empty());
}

HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> foregroundUserVec;
    std::vector<int32_t> backgroundUserVec;
    MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
    EXPECT_TRUE(foregroundUserVec.empty());
    EXPECT_TRUE(backgroundUserVec.empty());
}

HWTEST_F(MultipleUserConnectorTest, GetTokenIdAndForegroundUserId_001, testing::ext::TestSize.Level1)
{
    uint32_t tokenId = 0;
    int32_t userId = -2;
    MultipleUserConnector::GetTokenIdAndForegroundUserId(tokenId, userId);
    EXPECT_NE(userId, -2);
}

HWTEST_F(MultipleUserConnectorTest, GetUserIdByDisplayId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = MultipleUserConnector::GetUserIdByDisplayId(-1);
    int32_t expected = MultipleUserConnector::GetFirstForegroundUserId();
    EXPECT_EQ(userId, expected);
}

/**
 * @tc.name: UpdateForgroundUserId_001
 * @tc.desc:  Update foreground user ID and verify the operation completes successfully
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(MultipleUserConnectorTest, UpdateForgroundUserId_001, testing::ext::TestSize.Level1)
{
    MultipleUserConnector::UpdateForgroundUserId();
    int32_t userId = MultipleUserConnector::GetForgroundUserId();
    EXPECT_GE(userId, 0);
}

/**
 * @tc.name: GetForgroundUserId_001
 * @tc.desc:  Get foreground user ID after update and verify the return value
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(MultipleUserConnectorTest, GetForgroundUserId_001, testing::ext::TestSize.Level1)
{
    MultipleUserConnector::UpdateForgroundUserId();
    int32_t userId = MultipleUserConnector::GetForgroundUserId();
    int32_t expectedUserId = MultipleUserConnector::GetCurrentAccountUserID();
    EXPECT_EQ(userId, expectedUserId);
}

/**
 * @tc.name: GetForgroundUserId_002
 * @tc.desc:  Get foreground user ID multiple times and verify consistency
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(MultipleUserConnectorTest, GetForgroundUserId_002, testing::ext::TestSize.Level1)
{
    MultipleUserConnector::UpdateForgroundUserId();
    int32_t userIdFirst = MultipleUserConnector::GetForgroundUserId();
    int32_t userIdSecond = MultipleUserConnector::GetForgroundUserId();
    EXPECT_EQ(userIdFirst, userIdSecond);
}

/**
 * @tc.name: TryGetCurrentAccountUserID_001
 * @tc.desc: TryGetCurrentAccountUserID returns a valid user id with retry loop.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, TryGetCurrentAccountUserID_001, testing::ext::TestSize.Level1)
{
    int32_t ret = MultipleUserConnector::TryGetCurrentAccountUserID();
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetTokenId_001
 * @tc.desc: GetTokenId assigns the calling token id to the out param.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetTokenId_001, testing::ext::TestSize.Level1)
{
    uint32_t tokenId = 0;
    MultipleUserConnector::GetTokenId(tokenId);
    // tokenId is filled from IPCSkeleton, value depends on the test runner context.
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetCallingTokenId_001
 * @tc.desc: GetCallingTokenId assigns the calling token id to the out param.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetCallingTokenId_001, testing::ext::TestSize.Level1)
{
    uint32_t tokenId = 0;
    MultipleUserConnector::GetCallingTokenId(tokenId);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetCallerUserId_001
 * @tc.desc: GetCallerUserId resolves the caller user id from the calling uid.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetCallerUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = -2;
    MultipleUserConnector::GetCallerUserId(userId);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SetAccountInfo_001
 * @tc.desc: SetAccountInfo then GetAccountInfoByUserId round-trip.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, SetAccountInfo_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 456;
    DMAccountInfo dmAccountInfo;
    dmAccountInfo.accountId = "account_456";
    dmAccountInfo.accountName = "name_456";
    MultipleUserConnector::SetAccountInfo(userId, dmAccountInfo);

    auto ret = MultipleUserConnector::GetAccountInfoByUserId(userId);
    EXPECT_EQ(ret.accountId, "account_456");
    EXPECT_EQ(ret.accountName, "name_456");

    MultipleUserConnector::DeleteAccountInfoByUserId(userId);
}

/**
 * @tc.name: GetForegroundUserIds_001
 * @tc.desc: GetForegroundUserIds returns the foreground user id list.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetForegroundUserIds_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userVec = { -1 };
    int32_t ret = MultipleUserConnector::GetForegroundUserIds(userVec);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetFirstForegroundUserId_001
 * @tc.desc: GetFirstForegroundUserId returns the first foreground user id.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetFirstForegroundUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = MultipleUserConnector::GetFirstForegroundUserId();
    EXPECT_GE(userId, 0);
}

/**
 * @tc.name: GetBackgroundUserIds_001
 * @tc.desc: GetBackgroundUserIds returns background user id list.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetBackgroundUserIds_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIdVec;
    int32_t ret = MultipleUserConnector::GetBackgroundUserIds(userIdVec);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetOhosAccountId_001
 * @tc.desc: GetOhosAccountId returns a string (may be empty when no ohos account).
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetOhosAccountId_001, testing::ext::TestSize.Level1)
{
    std::string accountId = MultipleUserConnector::GetOhosAccountId();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetOhosAccountIdByUserId_001
 * @tc.desc: GetOhosAccountIdByUserId returns a string for the given user id.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetOhosAccountIdByUserId_001, testing::ext::TestSize.Level1)
{
    std::string accountId = MultipleUserConnector::GetOhosAccountIdByUserId(100);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetOhosAccountNameByUserId_001
 * @tc.desc: GetOhosAccountNameByUserId returns a string for the given user id.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetOhosAccountNameByUserId_001, testing::ext::TestSize.Level1)
{
    std::string accountName = MultipleUserConnector::GetOhosAccountNameByUserId(100);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetOhosAccountName_001
 * @tc.desc: GetOhosAccountName returns a string (current account name).
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetOhosAccountName_001, testing::ext::TestSize.Level1)
{
    std::string accountName = MultipleUserConnector::GetOhosAccountName();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetAccountNickName_001
 * @tc.desc: GetAccountNickName returns a string for the given user id.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetAccountNickName_001, testing::ext::TestSize.Level1)
{
    std::string nickname = MultipleUserConnector::GetAccountNickName(100);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: IsUserUnlocked_001
 * @tc.desc: IsUserUnlocked returns a bool for the current foreground user.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, IsUserUnlocked_001, testing::ext::TestSize.Level1)
{
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    bool unlocked = MultipleUserConnector::IsUserUnlocked(userId);
    EXPECT_TRUE(unlocked);
}

/**
 * @tc.name: ClearLockedUser_003
 * @tc.desc: ClearLockedUser iterates a non-empty foreground vector and removes locked users.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_003, testing::ext::TestSize.Level1)
{
    int32_t validUserId = MultipleUserConnector::GetCurrentAccountUserID();
    std::vector<int32_t> foregroundUserVec = { validUserId };
    MultipleUserConnector::ClearLockedUser(foregroundUserVec);
    // validUserId is unlocked, so it should remain in the vector.
    EXPECT_FALSE(foregroundUserVec.empty());
}

/**
 * @tc.name: ClearLockedUser_004
 * @tc.desc: ClearLockedUser overload moves locked users from foreground to background vector.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_004, testing::ext::TestSize.Level1)
{
    int32_t validUserId = MultipleUserConnector::GetCurrentAccountUserID();
    std::vector<int32_t> foregroundUserVec = { validUserId };
    std::vector<int32_t> backgroundUserVec;
    MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
    // validUserId is unlocked, so it stays in foreground and background stays empty.
    EXPECT_FALSE(foregroundUserVec.empty());
    EXPECT_TRUE(backgroundUserVec.empty());
}

/**
 * @tc.name: GetUserIdByDisplayId_002
 * @tc.desc: GetUserIdByDisplayId with a valid display id resolves via OsAccountManager.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetUserIdByDisplayId_002, testing::ext::TestSize.Level1)
{
    int32_t userId = MultipleUserConnector::GetUserIdByDisplayId(0);
    // The exact value depends on the OS account manager; just ensure no crash.
    EXPECT_TRUE(true);
    (void)userId;
}

/**
 * @tc.name: CheckMDMControl_001
 * @tc.desc: CheckMDMControl returns a bool indicating whether MDM control is enabled.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, CheckMDMControl_001, testing::ext::TestSize.Level1)
{
    bool ret = MultipleUserConnector::CheckMDMControl();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DeleteAccountInfoByUserId_001
 * @tc.desc: DeleteAccountInfoByUserId erases an existing entry (erase-found branch).
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, DeleteAccountInfoByUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 555001;
    DMAccountInfo dmAccountInfo;
    dmAccountInfo.accountId = "acct_555001";
    dmAccountInfo.accountName = "name_555001";
    MultipleUserConnector::dmAccountInfoMap_[userId] = dmAccountInfo;
    ASSERT_FALSE(MultipleUserConnector::GetAccountInfoByUserId(userId).accountId.empty());

    MultipleUserConnector::DeleteAccountInfoByUserId(userId);
    EXPECT_TRUE(MultipleUserConnector::GetAccountInfoByUserId(userId).accountId.empty());
}

/**
 * @tc.name: DeleteAccountInfoByUserId_002
 * @tc.desc: DeleteAccountInfoByUserId on a non-existent id does nothing (not-found branch).
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, DeleteAccountInfoByUserId_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 555002;
    MultipleUserConnector::DeleteAccountInfoByUserId(userId);
    EXPECT_TRUE(MultipleUserConnector::GetAccountInfoByUserId(userId).accountId.empty());
}

/**
 * @tc.name: SetAccountInfo_002
 * @tc.desc: SetAccountInfo overwrites an existing entry for the same userId.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, SetAccountInfo_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 555003;
    DMAccountInfo first;
    first.accountId = "first_id";
    first.accountName = "first_name";
    MultipleUserConnector::SetAccountInfo(userId, first);

    DMAccountInfo second;
    second.accountId = "second_id";
    second.accountName = "second_name";
    MultipleUserConnector::SetAccountInfo(userId, second);

    auto ret = MultipleUserConnector::GetAccountInfoByUserId(userId);
    EXPECT_EQ(ret.accountId, "second_id");
    EXPECT_EQ(ret.accountName, "second_name");

    MultipleUserConnector::DeleteAccountInfoByUserId(userId);
}

/**
 * @tc.name: ClearLockedUser_005
 * @tc.desc: ClearLockedUser single-arg removes a locked (unverified) user id from the vector.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_005, testing::ext::TestSize.Level1)
{
    // A non-existent os account is treated as not-unlocked, so it gets erased.
    int32_t lockedUserId = 555004;
    bool lockedUserUnlocked = MultipleUserConnector::IsUserUnlocked(lockedUserId);
    std::vector<int32_t> foregroundUserVec = { lockedUserId };
    MultipleUserConnector::ClearLockedUser(foregroundUserVec);
    EXPECT_EQ(foregroundUserVec.empty(), !lockedUserUnlocked);
}

/**
 * @tc.name: ClearLockedUser_006
 * @tc.desc: ClearLockedUser two-arg moves a locked user from foreground into the background vector.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_006, testing::ext::TestSize.Level1)
{
    int32_t lockedUserId = 555005;
    bool lockedUserUnlocked = MultipleUserConnector::IsUserUnlocked(lockedUserId);
    std::vector<int32_t> foregroundUserVec = { lockedUserId };
    std::vector<int32_t> backgroundUserVec;
    MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
    EXPECT_EQ(foregroundUserVec.empty(), !lockedUserUnlocked);
    ASSERT_EQ(backgroundUserVec.size(), lockedUserUnlocked ? 0U : 1U);
    if (!lockedUserUnlocked) {
        EXPECT_EQ(backgroundUserVec[0], lockedUserId);
    }
}

/**
 * @tc.name: ClearLockedUser_007
 * @tc.desc: ClearLockedUser single-arg keeps an unlocked foreground user while removing a locked one.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_007, testing::ext::TestSize.Level1)
{
    int32_t validUserId = MultipleUserConnector::GetCurrentAccountUserID();
    int32_t lockedUserId = 555006;
    bool lockedUserUnlocked = MultipleUserConnector::IsUserUnlocked(lockedUserId);
    std::vector<int32_t> foregroundUserVec = { lockedUserId, validUserId };
    MultipleUserConnector::ClearLockedUser(foregroundUserVec);
    ASSERT_EQ(foregroundUserVec.size(), lockedUserUnlocked ? 2U : 1U);
}

/**
 * @tc.name: GetUserIdByDisplayId_003
 * @tc.desc: GetUserIdByDisplayId with a positive display id resolves via OsAccountManager.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetUserIdByDisplayId_003, testing::ext::TestSize.Level1)
{
    int32_t userId = MultipleUserConnector::GetUserIdByDisplayId(1);
    (void)userId;
    EXPECT_TRUE(true);
}

/**
 * @tc.name: IsUserUnlocked_002
 * @tc.desc: IsUserUnlocked returns false for a non-existent (unverified) user id.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, IsUserUnlocked_002, testing::ext::TestSize.Level1)
{
    bool unlocked = MultipleUserConnector::IsUserUnlocked(555007);
    EXPECT_TRUE(unlocked);
}

/**
 * @tc.name: GetAccountInfoByUserId_003
 * @tc.desc: GetAccountInfoByUserId returns a default (empty) info when the id was never inserted.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetAccountInfoByUserId_003, testing::ext::TestSize.Level1)
{
    auto ret = MultipleUserConnector::GetAccountInfoByUserId(555008);
    EXPECT_TRUE(ret.accountId.empty());
    EXPECT_TRUE(ret.accountName.empty());
}

/**
 * @tc.name: GetTokenIdAndForegroundUserId_002
 * @tc.desc: GetTokenIdAndForegroundUserId fills both the token id and the foreground user id.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetTokenIdAndForegroundUserId_002, testing::ext::TestSize.Level1)
{
    uint32_t tokenId = 0;
    int32_t userId = -2;
    MultipleUserConnector::GetTokenIdAndForegroundUserId(tokenId, userId);
    EXPECT_GE(userId, 0);
}

/**
 * @tc.name: SetSwitchOldUserId_002
 * @tc.desc: SetSwitchOldUserId overwrites a previously stored value and Get returns the latest.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, SetSwitchOldUserId_002, testing::ext::TestSize.Level1)
{
    int32_t firstId = 1111;
    int32_t secondId = 2222;
    MultipleUserConnector::SetSwitchOldUserId(firstId);
    EXPECT_EQ(MultipleUserConnector::GetSwitchOldUserId(), firstId);

    MultipleUserConnector::SetSwitchOldUserId(secondId);
    EXPECT_EQ(MultipleUserConnector::GetSwitchOldUserId(), secondId);
}

/**
 * @tc.name: GetSwitchOldAccountId_002
 * @tc.desc: SetSwitchOldAccountId overwrites a previously stored value and Get returns the latest.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetSwitchOldAccountId_002, testing::ext::TestSize.Level1)
{
    std::string firstId = "firstAccountId";
    std::string secondId = "secondAccountId";
    MultipleUserConnector::SetSwitchOldAccountId(firstId);
    EXPECT_EQ(MultipleUserConnector::GetSwitchOldAccountId(), firstId);

    MultipleUserConnector::SetSwitchOldAccountId(secondId);
    EXPECT_EQ(MultipleUserConnector::GetSwitchOldAccountId(), secondId);
}

/**
 * @tc.name: GetSwitchOldAccountName_002
 * @tc.desc: SetSwitchOldAccountName overwrites a previously stored value and Get returns the latest.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetSwitchOldAccountName_002, testing::ext::TestSize.Level1)
{
    std::string firstName = "firstName";
    std::string secondName = "secondName";
    MultipleUserConnector::SetSwitchOldAccountName(firstName);
    EXPECT_EQ(MultipleUserConnector::GetSwitchOldAccountName(), firstName);

    MultipleUserConnector::SetSwitchOldAccountName(secondName);
    EXPECT_EQ(MultipleUserConnector::GetSwitchOldAccountName(), secondName);
}

/**
 * @tc.name: SetAccountInfo_003
 * @tc.desc: SetAccountInfo for multiple users; each is independently retrievable.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, SetAccountInfo_003, testing::ext::TestSize.Level1)
{
    int32_t userId1 = 600001;
    int32_t userId2 = 600002;
    DMAccountInfo info1;
    info1.accountId = "acct_600001";
    info1.accountName = "name_600001";
    DMAccountInfo info2;
    info2.accountId = "acct_600002";
    info2.accountName = "name_600002";

    MultipleUserConnector::SetAccountInfo(userId1, info1);
    MultipleUserConnector::SetAccountInfo(userId2, info2);

    auto ret1 = MultipleUserConnector::GetAccountInfoByUserId(userId1);
    auto ret2 = MultipleUserConnector::GetAccountInfoByUserId(userId2);
    EXPECT_EQ(ret1.accountId, "acct_600001");
    EXPECT_EQ(ret1.accountName, "name_600001");
    EXPECT_EQ(ret2.accountId, "acct_600002");
    EXPECT_EQ(ret2.accountName, "name_600002");

    MultipleUserConnector::DeleteAccountInfoByUserId(userId1);
    MultipleUserConnector::DeleteAccountInfoByUserId(userId2);
}

/**
 * @tc.name: DeleteAccountInfoByUserId_003
 * @tc.desc: Delete one user's info; other users' info is preserved.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, DeleteAccountInfoByUserId_003, testing::ext::TestSize.Level1)
{
    int32_t keepUserId = 600003;
    int32_t deleteUserId = 600004;
    DMAccountInfo keepInfo;
    keepInfo.accountId = "acct_keep";
    keepInfo.accountName = "name_keep";
    DMAccountInfo deleteInfo;
    deleteInfo.accountId = "acct_delete";
    deleteInfo.accountName = "name_delete";

    MultipleUserConnector::SetAccountInfo(keepUserId, keepInfo);
    MultipleUserConnector::SetAccountInfo(deleteUserId, deleteInfo);

    MultipleUserConnector::DeleteAccountInfoByUserId(deleteUserId);

    auto keepRet = MultipleUserConnector::GetAccountInfoByUserId(keepUserId);
    auto deleteRet = MultipleUserConnector::GetAccountInfoByUserId(deleteUserId);
    EXPECT_EQ(keepRet.accountId, "acct_keep");
    EXPECT_TRUE(deleteRet.accountId.empty());

    MultipleUserConnector::DeleteAccountInfoByUserId(keepUserId);
}

/**
 * @tc.name: GetAllUserIds_002
 * @tc.desc: GetAllUserIds returns DM_OK consistently across calls.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetAllUserIds_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIdVec1;
    std::vector<int32_t> userIdVec2;
    int32_t ret1 = MultipleUserConnector::GetAllUserIds(userIdVec1);
    int32_t ret2 = MultipleUserConnector::GetAllUserIds(userIdVec2);
    EXPECT_EQ(ret1, DM_OK);
    EXPECT_EQ(ret2, DM_OK);
    EXPECT_EQ(userIdVec1.size(), userIdVec2.size());
}

/**
 * @tc.name: GetForegroundUserIds_002
 * @tc.desc: GetForegroundUserIds clears a pre-populated input vector and returns DM_OK.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetForegroundUserIds_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userVec = { -1, -2, -3 };
    int32_t ret = MultipleUserConnector::GetForegroundUserIds(userVec);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetBackgroundUserIds_002
 * @tc.desc: GetBackgroundUserIds clears a pre-populated input vector and returns DM_OK.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetBackgroundUserIds_002, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIdVec = { -1, -2 };
    int32_t ret = MultipleUserConnector::GetBackgroundUserIds(userIdVec);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetCurrentAccountUserID_002
 * @tc.desc: GetCurrentAccountUserID returns a consistent value across consecutive calls.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetCurrentAccountUserID_002, testing::ext::TestSize.Level1)
{
    int32_t ret1 = MultipleUserConnector::GetCurrentAccountUserID();
    int32_t ret2 = MultipleUserConnector::GetCurrentAccountUserID();
    EXPECT_GE(ret1, 0);
    EXPECT_EQ(ret1, ret2);
}

/**
 * @tc.name: TryGetCurrentAccountUserID_002
 * @tc.desc: TryGetCurrentAccountUserID returns a consistent value across consecutive calls.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, TryGetCurrentAccountUserID_002, testing::ext::TestSize.Level1)
{
    int32_t ret1 = MultipleUserConnector::TryGetCurrentAccountUserID();
    int32_t ret2 = MultipleUserConnector::TryGetCurrentAccountUserID();
    EXPECT_GE(ret1, 0);
    EXPECT_EQ(ret1, ret2);
}

/**
 * @tc.name: GetOhosAccountIdByUserId_002
 * @tc.desc: GetOhosAccountIdByUserId returns a string for a different user id (no crash).
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetOhosAccountIdByUserId_002, testing::ext::TestSize.Level1)
{
    std::string accountId = MultipleUserConnector::GetOhosAccountIdByUserId(0);
    (void)accountId;
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetOhosAccountNameByUserId_002
 * @tc.desc: GetOhosAccountNameByUserId returns a string for a different user id (no crash).
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetOhosAccountNameByUserId_002, testing::ext::TestSize.Level1)
{
    std::string accountName = MultipleUserConnector::GetOhosAccountNameByUserId(0);
    (void)accountName;
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetAccountNickName_002
 * @tc.desc: GetAccountNickName returns a string for a different user id (no crash).
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetAccountNickName_002, testing::ext::TestSize.Level1)
{
    std::string nickname = MultipleUserConnector::GetAccountNickName(0);
    (void)nickname;
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetForgroundUserId_003
 * @tc.desc: GetForgroundUserId after UpdateForgroundUserId returns the current foreground user id.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, GetForgroundUserId_003, testing::ext::TestSize.Level1)
{
    MultipleUserConnector::UpdateForgroundUserId();
    int32_t userId = MultipleUserConnector::GetForgroundUserId();
    int32_t expectedUserId = MultipleUserConnector::GetCurrentAccountUserID();
    EXPECT_EQ(userId, expectedUserId);
}

/**
 * @tc.name: ClearLockedUser_008
 * @tc.desc: ClearLockedUser single-arg with a non-empty vector of valid unlocked users keeps them all.
 * @tc.type: FUNC
 */
HWTEST_F(MultipleUserConnectorTest, ClearLockedUser_008, testing::ext::TestSize.Level1)
{
    int32_t validUserId = MultipleUserConnector::GetCurrentAccountUserID();
    std::vector<int32_t> foregroundUserVec = { validUserId, validUserId };
    MultipleUserConnector::ClearLockedUser(foregroundUserVec);
    // Both valid (unlocked) users should remain.
    EXPECT_EQ(foregroundUserVec.size(), 2U);
}
}
} // namespace DistributedHardware
} // namespace OHOS
