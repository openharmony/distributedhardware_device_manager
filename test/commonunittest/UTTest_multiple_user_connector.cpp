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
}
} // namespace DistributedHardware
} // namespace OHOS
