/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "UTTest_dm_anonymous.h"

namespace OHOS {
namespace DistributedHardware {
void DmAnonymousTest::SetUp()
{
}
void DmAnonymousTest::TearDown()
{
}
void DmAnonymousTest::SetUpTestCase()
{
}
void DmAnonymousTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: GetAnonyString_001
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyString_001, testing::ext::TestSize.Level0)
{
    const std::string value = "valueTest";
    std::string str = GetAnonyString(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 8);
}

/**
 * @tc.name: GetAnonyString_002
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyString_002, testing::ext::TestSize.Level0)
{
    const std::string value = "va";
    std::string str = GetAnonyString(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 6);
}

/**
 * @tc.name: GetAnonyString_003
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyString_003, testing::ext::TestSize.Level0)
{
    const std::string value = "ohos.distributedhardware.devicemanager.resident";
    std::string str = GetAnonyString(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 14);
}

/**
 * @tc.name: GetAnonyInt32_001
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyInt32_001, testing::ext::TestSize.Level0)
{
    const int32_t value = 1;
    std::string str = GetAnonyInt32(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: GetAnonyInt32_002
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyInt32_002, testing::ext::TestSize.Level0)
{
    const int32_t value = 12;
    std::string str = GetAnonyInt32(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 2);
}

/**
 * @tc.name: GetAnonyInt32_003
 * @tc.desc: Return size of anony string
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetAnonyInt32_003, testing::ext::TestSize.Level0)
{
    const int32_t value = 123456;
    std::string str = GetAnonyInt32(value);
    int32_t ret = str.size();
    EXPECT_EQ(ret, 6);
}

/**
 * @tc.name: IsNumberString_001
 * @tc.desc: Return false if the string is not a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsNumberString_001, testing::ext::TestSize.Level0)
{
    const std::string inputString = "";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsNumberString_002
 * @tc.desc: Return false if the string is not a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsNumberString_002, testing::ext::TestSize.Level0)
{
    const std::string inputString = "123inputstring";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsNumberString_003
 * @tc.desc: Return true if the string is a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, IsNumberString_003, testing::ext::TestSize.Level0)
{
    const std::string inputString = "25633981";
    bool ret = IsNumberString(inputString);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetErrorString_001
 * @tc.desc: Return true if the string is a number
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmAnonymousTest, GetErrorString_001, testing::ext::TestSize.Level0)
{
    int failedReason = -20000;
    std::string errorMessage = "dm process execution failed.";
    std::string ret = GetErrorString(failedReason);
    EXPECT_EQ(ret, errorMessage);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS