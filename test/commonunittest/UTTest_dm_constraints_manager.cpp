/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UTTest_dm_constraints_manager.h"

#include "dm_constraints_manager.h"
#include "dm_error_type.h"

namespace OHOS {
namespace DistributedHardware {
void DmConstrainsManagerTest::SetUp()
{
}

void DmConstrainsManagerTest::TearDown()
{
}

void DmConstrainsManagerTest::SetUpTestCase()
{
}

void DmConstrainsManagerTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: SubscribeOsAccountConstraints_001
 * @tc.desc: Subscribe OS account constraints with empty constraint set
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, SubscribeOsAccountConstraints_001, testing::ext::TestSize.Level1)
{
    std::set<std::string> emptyConstraintSet;
    int32_t ret = DmConstrainsManager::GetInstance().SubscribeOsAccountConstraints(emptyConstraintSet);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SubscribeOsAccountConstraints_002
 * @tc.desc: Subscribe OS account constraints with valid constraint set
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, SubscribeOsAccountConstraints_002, testing::ext::TestSize.Level1)
{
    std::set<std::string> constraintSet;
    constraintSet.insert("constraint.distributed.transmission.outgoing");
    int32_t ret = DmConstrainsManager::GetInstance().SubscribeOsAccountConstraints(constraintSet);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: CheckOsAccountConstraintEnabled_001
 * @tc.desc: Check OS account constraint enabled with invalid userId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, CheckOsAccountConstraintEnabled_001, testing::ext::TestSize.Level1)
{
    int32_t userId = -1;
    std::string constraint = "constraint.distributed.transmission.outgoing";
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: CheckOsAccountConstraintEnabled_002
 * @tc.desc: Check OS account constraint enabled with valid userId and empty constraint
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, CheckOsAccountConstraintEnabled_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;
    std::string constraint = "";
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: CheckOsAccountConstraintEnabled_003
 * @tc.desc: Check OS account constraint enabled with valid userId and constraint
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, CheckOsAccountConstraintEnabled_003, testing::ext::TestSize.Level1)
{
    int32_t userId = 100;
    std::string constraint = "constraint.distributed.transmission.outgoing";
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: AddConstraint_001
 * @tc.desc: Add constraint with valid constraint data
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, AddConstraint_001, testing::ext::TestSize.Level1)
{
    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = 100;
    constrainData.constraint = "constraint.distributed.transmission.outgoing";
    constrainData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData);
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(
        constrainData.localId, constrainData.constraint);
    EXPECT_TRUE(isEnabled);
}

/**
 * @tc.name: AddConstraint_002
 * @tc.desc: Add constraint with disabled state
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, AddConstraint_002, testing::ext::TestSize.Level1)
{
    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = 101;
    constrainData.constraint = "constraint.distributed.transmission.outgoing";
    constrainData.isEnabled = false;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData);
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(
        constrainData.localId, constrainData.constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: DeleteConstraint_001
 * @tc.desc: Delete constraint with valid userId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DeleteConstraint_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 102;
    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = userId;
    constrainData.constraint = "constraint.distributed.transmission.outgoing";
    constrainData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData);
    DmConstrainsManager::GetInstance().DeleteConstraint(userId);
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(
        userId, constrainData.constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: DeleteConstraint_002
 * @tc.desc: Delete constraint with invalid userId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DeleteConstraint_002, testing::ext::TestSize.Level1)
{
    int32_t userId = -1;
    DmConstrainsManager::GetInstance().DeleteConstraint(userId);
    SUCCEED();
}

/**
 * @tc.name: DeleteConstraint_003
 * @tc.desc: Delete constraint and verify multiple constraints are removed
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DeleteConstraint_003, testing::ext::TestSize.Level1)
{
    int32_t userId = 103;
    std::string constraint1 = "constraint.distributed.transmission.outgoing";
    std::string constraint2 = "constraint.distributed.transmission.incoming";

    AccountSA::OsAccountConstraintStateData constrainData1;
    constrainData1.localId = userId;
    constrainData1.constraint = constraint1;
    constrainData1.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData1);

    AccountSA::OsAccountConstraintStateData constrainData2;
    constrainData2.localId = userId;
    constrainData2.constraint = constraint2;
    constrainData2.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData2);

    DmConstrainsManager::GetInstance().DeleteConstraint(userId);

    bool isEnabled1 = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint1);
    bool isEnabled2 = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint2);
    EXPECT_FALSE(isEnabled1);
    EXPECT_FALSE(isEnabled2);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_001
 * @tc.desc: Test DmOsAccountConstraintStateData equality operator
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_001, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 100;
    data1.constraint = "test.constraint";

    DmOsAccountConstraintStateData data2;
    data2.userId = 100;
    data2.constraint = "test.constraint";

    EXPECT_TRUE(data1 == data2);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_002
 * @tc.desc: Test DmOsAccountConstraintStateData less than operator
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_002, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 100;
    data1.constraint = "a.constraint";

    DmOsAccountConstraintStateData data2;
    data2.userId = 100;
    data2.constraint = "b.constraint";

    EXPECT_TRUE(data1 < data2);
    EXPECT_FALSE(data2 < data1);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_003
 * @tc.desc: Test DmOsAccountConstraintStateData less than operator with different userId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_003, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 99;
    data1.constraint = "z.constraint";

    DmOsAccountConstraintStateData data2;
    data2.userId = 100;
    data2.constraint = "a.constraint";

    EXPECT_TRUE(data1 < data2);
    EXPECT_FALSE(data2 < data1);
}

} // namespace
} // namespace DistributedHardware
} // namespace OHOS
