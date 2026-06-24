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
/**
 * @tc.name: SubscribeOsAccountConstraints_002
 * @tc.desc: Subscribe OS account constraints with valid constraint set
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: CheckOsAccountConstraintEnabled_001
 * @tc.desc: Check OS account constraint enabled with invalid userId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: CheckOsAccountConstraintEnabled_002
 * @tc.desc: Check OS account constraint enabled with valid userId and empty constraint
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
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

/**
 * @tc.name: CheckOsAccountConstraintEnabled_004
 * @tc.desc: Check constraint that already exists in cache (cache-hit path of CheckOsAccountConstraintEnabled)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, CheckOsAccountConstraintEnabled_004, testing::ext::TestSize.Level1)
{
    int32_t userId = 200;
    std::string constraint = "constraint.distributed.cache.hit";
    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = userId;
    constrainData.constraint = constraint;
    constrainData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData);
    // cache-hit branch: returns cached isEnabled directly without consulting account manager.
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_TRUE(isEnabled);
}

/**
 * @tc.name: CheckOsAccountConstraintEnabled_005
 * @tc.desc: Check a different constraint for a cached userId (cache-miss on constraint within known userId)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, CheckOsAccountConstraintEnabled_005, testing::ext::TestSize.Level1)
{
    int32_t userId = 201;
    std::string knownConstraint = "constraint.distributed.known";
    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = userId;
    constrainData.constraint = knownConstraint;
    constrainData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData);
    // A different constraint for same userId: cache-miss path iterates map without matching.
    std::string unknownConstraint = "constraint.distributed.unknown";
    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, unknownConstraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: AddConstraint_003
 * @tc.desc: Add constraint then overwrite same (userId,constraint) key with different enabled state
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, AddConstraint_003, testing::ext::TestSize.Level1)
{
    int32_t userId = 202;
    std::string constraint = "constraint.distributed.overwrite";
    AccountSA::OsAccountConstraintStateData firstData;
    firstData.localId = userId;
    firstData.constraint = constraint;
    firstData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(firstData);

    // Overwrite the same map key with a disabled state.
    AccountSA::OsAccountConstraintStateData secondData;
    secondData.localId = userId;
    secondData.constraint = constraint;
    secondData.isEnabled = false;
    DmConstrainsManager::GetInstance().AddConstraint(secondData);

    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: DeleteConstraint_004
 * @tc.desc: Delete one userId while another userId's constraints are preserved (else/++it branch)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DeleteConstraint_004, testing::ext::TestSize.Level1)
{
    int32_t keepUserId = 203;
    int32_t deleteUserId = 204;
    std::string constraint = "constraint.distributed.preserve";

    AccountSA::OsAccountConstraintStateData keepData;
    keepData.localId = keepUserId;
    keepData.constraint = constraint;
    keepData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(keepData);

    AccountSA::OsAccountConstraintStateData deleteData;
    deleteData.localId = deleteUserId;
    deleteData.constraint = constraint;
    deleteData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(deleteData);

    DmConstrainsManager::GetInstance().DeleteConstraint(deleteUserId);

    bool keepEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(keepUserId, constraint);
    bool deleteEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(deleteUserId, constraint);
    EXPECT_TRUE(keepEnabled);
    EXPECT_FALSE(deleteEnabled);
}

/**
 * @tc.name: DeleteConstraint_005
 * @tc.desc: Delete userId when no matching entry exists in cache (no-op erase loop)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DeleteConstraint_005, testing::ext::TestSize.Level1)
{
    int32_t absentUserId = 9999;
    int32_t keepUserId = 205;
    std::string constraint = "constraint.distributed.noop.delete";

    AccountSA::OsAccountConstraintStateData keepData;
    keepData.localId = keepUserId;
    keepData.constraint = constraint;
    keepData.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(keepData);

    DmConstrainsManager::GetInstance().DeleteConstraint(absentUserId);
    bool keepEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(keepUserId, constraint);
    EXPECT_TRUE(keepEnabled);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_004
 * @tc.desc: Test DmOsAccountConstraintStateData equality returns false when userId differs
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_004, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 100;
    data1.constraint = "same.constraint";

    DmOsAccountConstraintStateData data2;
    data2.userId = 101;
    data2.constraint = "same.constraint";

    EXPECT_FALSE(data1 == data2);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_005
 * @tc.desc: Test DmOsAccountConstraintStateData equality returns false when constraint differs
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_005, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 100;
    data1.constraint = "constraint.a";

    DmOsAccountConstraintStateData data2;
    data2.userId = 100;
    data2.constraint = "constraint.b";

    EXPECT_FALSE(data1 == data2);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_006
 * @tc.desc: Test DmOsAccountConstraintStateData operator< returns false for equal objects
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_006, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 100;
    data1.constraint = "same.constraint";

    DmOsAccountConstraintStateData data2;
    data2.userId = 100;
    data2.constraint = "same.constraint";

    EXPECT_FALSE(data1 < data2);
    EXPECT_FALSE(data2 < data1);
}

/**
 * @tc.name: AddConstraint_004
 * @tc.desc: Add constraint for a userId that already has a different constraint (multiple keys per user)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, AddConstraint_004, testing::ext::TestSize.Level1)
{
    int32_t userId = 300;
    std::string constraint1 = "constraint.distributed.multi.a";
    std::string constraint2 = "constraint.distributed.multi.b";

    AccountSA::OsAccountConstraintStateData data1;
    data1.localId = userId;
    data1.constraint = constraint1;
    data1.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(data1);

    AccountSA::OsAccountConstraintStateData data2;
    data2.localId = userId;
    data2.constraint = constraint2;
    data2.isEnabled = false;
    DmConstrainsManager::GetInstance().AddConstraint(data2);

    bool enabled1 = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint1);
    bool enabled2 = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint2);
    EXPECT_TRUE(enabled1);
    EXPECT_FALSE(enabled2);
}

/**
 * @tc.name: DeleteConstraint_006
 * @tc.desc: Delete a userId that has multiple constraints; all constraints for that userId are removed
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DeleteConstraint_006, testing::ext::TestSize.Level1)
{
    int32_t userId = 301;
    std::string constraint1 = "constraint.distributed.delmulti.a";
    std::string constraint2 = "constraint.distributed.delmulti.b";

    AccountSA::OsAccountConstraintStateData data1;
    data1.localId = userId;
    data1.constraint = constraint1;
    data1.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(data1);

    AccountSA::OsAccountConstraintStateData data2;
    data2.localId = userId;
    data2.constraint = constraint2;
    data2.isEnabled = true;
    DmConstrainsManager::GetInstance().AddConstraint(data2);

    DmConstrainsManager::GetInstance().DeleteConstraint(userId);

    bool enabled1 = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint1);
    bool enabled2 = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint2);
    EXPECT_FALSE(enabled1);
    EXPECT_FALSE(enabled2);
}

/**
 * @tc.name: OnConstraintChanged_001
 * @tc.desc: DmOsAccountConstraintSubscriber::OnConstraintChanged adds constraint via the manager
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, OnConstraintChanged_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 302;
    std::string constraint = "constraint.distributed.subscriber.enabled";
    std::set<std::string> constraintSet = { constraint };
    DmOsAccountConstraintSubscriber subscriber(DmConstrainsManager::GetInstance(), constraintSet);

    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = userId;
    constrainData.constraint = constraint;
    constrainData.isEnabled = true;
    subscriber.OnConstraintChanged(constrainData);

    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_TRUE(isEnabled);
}

/**
 * @tc.name: OnConstraintChanged_002
 * @tc.desc: DmOsAccountConstraintSubscriber::OnConstraintChanged with disabled state is reflected in cache
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, OnConstraintChanged_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 303;
    std::string constraint = "constraint.distributed.subscriber.disabled";
    std::set<std::string> constraintSet = { constraint };
    DmOsAccountConstraintSubscriber subscriber(DmConstrainsManager::GetInstance(), constraintSet);

    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = userId;
    constrainData.constraint = constraint;
    constrainData.isEnabled = false;
    subscriber.OnConstraintChanged(constrainData);

    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: OnConstraintChanged_003
 * @tc.desc: OnConstraintChanged overwrites an existing (userId,constraint) entry with the new enabled state
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, OnConstraintChanged_003, testing::ext::TestSize.Level1)
{
    int32_t userId = 304;
    std::string constraint = "constraint.distributed.subscriber.overwrite";
    std::set<std::string> constraintSet = { constraint };
    DmOsAccountConstraintSubscriber subscriber(DmConstrainsManager::GetInstance(), constraintSet);

    AccountSA::OsAccountConstraintStateData firstData;
    firstData.localId = userId;
    firstData.constraint = constraint;
    firstData.isEnabled = true;
    subscriber.OnConstraintChanged(firstData);
    EXPECT_TRUE(DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint));

    AccountSA::OsAccountConstraintStateData secondData;
    secondData.localId = userId;
    secondData.constraint = constraint;
    secondData.isEnabled = false;
    subscriber.OnConstraintChanged(secondData);
    EXPECT_FALSE(DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint));
}

/**
 * @tc.name: CheckOsAccountConstraintEnabled_006
 * @tc.desc: Cache-hit returns the cached disabled state (false) without consulting account manager
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, CheckOsAccountConstraintEnabled_006, testing::ext::TestSize.Level1)
{
    int32_t userId = 305;
    std::string constraint = "constraint.distributed.cache.disabled";
    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = userId;
    constrainData.constraint = constraint;
    constrainData.isEnabled = false;
    DmConstrainsManager::GetInstance().AddConstraint(constrainData);

    bool isEnabled = DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(userId, constraint);
    EXPECT_FALSE(isEnabled);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_007
 * @tc.desc: operator< ordering: smaller userId but larger constraint string is still less
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_007, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 99;
    data1.constraint = "z.constraint";

    DmOsAccountConstraintStateData data2;
    data2.userId = 100;
    data2.constraint = "a.constraint";

    // userId 99 < 100, so data1 < data2 regardless of constraint.
    EXPECT_TRUE(data1 < data2);
    EXPECT_FALSE(data2 < data1);
}

/**
 * @tc.name: DmOsAccountConstraintStateData_008
 * @tc.desc: operator< uses constraint comparison when userId is equal
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmConstrainsManagerTest, DmOsAccountConstraintStateData_008, testing::ext::TestSize.Level1)
{
    DmOsAccountConstraintStateData data1;
    data1.userId = 100;
    data1.constraint = "alpha";

    DmOsAccountConstraintStateData data2;
    data2.userId = 100;
    data2.constraint = "beta";

    EXPECT_TRUE(data1 < data2);
    EXPECT_FALSE(data2 < data1);
}

} // namespace
} // namespace DistributedHardware
} // namespace OHOS
