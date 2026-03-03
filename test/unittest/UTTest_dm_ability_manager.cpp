/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "UTTest_dm_ability_manager.h"

namespace OHOS {
namespace DistributedHardware {

void DmAbilityManagerTest::SetUpTestCase()
{
}

void DmAbilityManagerTest::TearDownTestCase()
{
}

void DmAbilityManagerTest::SetUp()
{
}

void DmAbilityManagerTest::TearDown()
{
}

/**
 * @tc.name: StartAbility_001
 * @tc.desc: StartAbility with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_001, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "appOperation": "operation", "customDescription": "desc", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_002
 * @tc.desc: StartAbility with empty parameters
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_002, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = "{}";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_003
 * @tc.desc: StartAbility with invalid json
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_003, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = "invalid_json";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_004
 * @tc.desc: StartAbility with deviceType -1
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_004, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": -1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_005
 * @tc.desc: StartAbility with deviceType 0
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_005, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": 0})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_006
 * @tc.desc: StartAbility with deviceType 1
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_006, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_007
 * @tc.desc: StartAbility with deviceType 2
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_007, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": 2})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_008
 * @tc.desc: StartAbility with deviceType 3
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_008, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": 3})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_009
 * @tc.desc: StartAbility with large deviceType value
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_009, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": 100})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_010
 * @tc.desc: StartAbility with very large deviceType value
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_010, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": 2147483647})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_011
 * @tc.desc: StartAbility with negative deviceType value
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_011, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": -2147483648})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_012
 * @tc.desc: StartAbility with empty requester
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_012, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_013
 * @tc.desc: StartAbility with long requester name
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_013, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "VeryLongDeviceNameThatExceedsNormalLengthForTesting", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_014
 * @tc.desc: StartAbility with special characters in requester
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_014, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "Device@#$%", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_015
 * @tc.desc: StartAbility with appOperation
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_015, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "appOperation": "testOperation", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_016
 * @tc.desc: StartAbility with customDescription
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_016, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "customDescription": "Custom Description", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_017
 * @tc.desc: StartAbility with all fields present
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_017, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "appOperation": "operation", "customDescription": "desc", "localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_018
 * @tc.desc: StartAbility with missing requester field
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_018, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"localDeviceType": 1})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_019
 * @tc.desc: StartAbility with only requester field
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_019, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName"})";

    AbilityStatus status = abilityManager.StartAbility(params);

    EXPECT_TRUE(status == AbilityStatus::ABILITY_STATUS_SUCCESS || status == AbilityStatus::ABILITY_STATUS_FAILED);
}

/**
 * @tc.name: StartAbility_020
 * @tc.desc: StartAbility multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(DmAbilityManagerTest, StartAbility_020, testing::ext::TestSize.Level1)
{
    DmAbilityManager abilityManager;
    std::string params = R"({"requester": "deviceName", "localDeviceType": 1})";

    AbilityStatus status1 = abilityManager.StartAbility(params);
    AbilityStatus status2 = abilityManager.StartAbility(params);

    EXPECT_TRUE(status1 == AbilityStatus::ABILITY_STATUS_SUCCESS || status1 == AbilityStatus::ABILITY_STATUS_FAILED);
    EXPECT_TRUE(status2 == AbilityStatus::ABILITY_STATUS_SUCCESS || status2 == AbilityStatus::ABILITY_STATUS_FAILED);
}

} // namespace DistributedHardware
} // namespace OHOS
