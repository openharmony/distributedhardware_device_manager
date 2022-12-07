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

#include "UTTest_dm_dfx.h"

namespace OHOS {
namespace DistributedHardware {
void DmDfxTest::SetUp()
{
}
void DmDfxTest::TearDown()
{
}
void DmDfxTest::SetUpTestCase()
{
}
void DmDfxTest::TearDownTestCase()
{
}

namespace {

/**
 * @tc.name: ShowHelp_001
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, ShowHelp_001, testing::ext::TestSize.Level0)
{
    std::string result = "";
    int32_t ret = HiDumpHelper::GetInstance().ShowHelp(result);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ShowIllealInfomation_001
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, ShowIllealInfomation_001, testing::ext::TestSize.Level0)
{
    std::string result = "";
    int32_t ret = HiDumpHelper::GetInstance().ShowIllealInfomation(result);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetArgsType_001
 * @tc.desc: Return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, GetArgsType_001, testing::ext::TestSize.Level0)
{
    std::vector<std::string> args;
    std::vector<HidumperFlag> Flag;
    int32_t ret = HiDumpHelper::GetInstance().GetArgsType(args, Flag);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetArgsType_002
 * @tc.desc: Return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, GetArgsType_002, testing::ext::TestSize.Level0)
{
    std::string str = std::string(ARGS_HELP_INFO);
    std::vector<std::string> args;
    args.push_back(str);
    std::vector<HidumperFlag> Flag;
    int32_t ret = HiDumpHelper::GetInstance().GetArgsType(args, Flag);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SetNodeInfo_001
 * @tc.desc: HiDumpHelper::GetInstance().nodeInfo_.size() plus 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, SetNodeInfo_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    HiDumpHelper::GetInstance().SetNodeInfo(deviceInfo);
    EXPECT_EQ(HiDumpHelper::GetInstance().nodeInfos_.size(), 1);
}

/**
 * @tc.name: ShowAllLoadTrustedList_001
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, ShowAllLoadTrustedList_001, testing::ext::TestSize.Level0)
{
    std::string result = "";
    int32_t ret = HiDumpHelper::GetInstance().ShowAllLoadTrustedList(result);
    EXPECT_TRUE(!result.empty());
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ShowAllLoadTrustedList_002
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, ShowAllLoadTrustedList_002, testing::ext::TestSize.Level0)
{
    std::string result = "";
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceTypeId = 1;
    deviceInfo.range = 11;
    HiDumpHelper::GetInstance().nodeInfos_.push_back(deviceInfo);
    int32_t ret = HiDumpHelper::GetInstance().ShowAllLoadTrustedList(result);
    EXPECT_TRUE(!result.empty());
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetDeviceType_001
 * @tc.desc: set deviceTypeId id 8, Return string is not empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, GetDeviceType_001, testing::ext::TestSize.Level0)
{
    int32_t deviceTypeId = 8;
    std::string ret = HiDumpHelper::GetInstance().GetDeviceType(deviceTypeId);
    EXPECT_TRUE(!ret.empty());
}

/**
 * @tc.name: GetDeviceType_002
 * @tc.desc: set deviceTypeId id 10000, Return string is not empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, GetDeviceType_002, testing::ext::TestSize.Level0)
{
    int32_t deviceTypeId = 10000;
    std::string ret = HiDumpHelper::GetInstance().GetDeviceType(deviceTypeId);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: ProcessDump_001
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, ProcessDump_001, testing::ext::TestSize.Level0)
{
    HidumperFlag flag = HidumperFlag::HIDUMPER_GET_HELP;
    std::string result;
    int32_t ret = HiDumpHelper::GetInstance().ProcessDump(flag, result);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ProcessDump_002
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, ProcessDump_002, testing::ext::TestSize.Level0)
{
    HidumperFlag flag = HidumperFlag::HIDUMPER_GET_TRUSTED_LIST;
    std::string result;
    int32_t ret = HiDumpHelper::GetInstance().ProcessDump(flag, result);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ProcessDump_003
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, ProcessDump_003, testing::ext::TestSize.Level0)
{
    HidumperFlag flag = HidumperFlag::HIDUMPER_UNKNOWN;
    std::string result;
    int32_t ret = HiDumpHelper::GetInstance().ProcessDump(flag, result);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: HiDump_001
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, HiDump_001, testing::ext::TestSize.Level0)
{
    std::vector<std::string> args;
    std::string result;
    int32_t ret = HiDumpHelper::GetInstance().HiDump(args, result);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: HiDump_002
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, HiDump_002, testing::ext::TestSize.Level0)
{
    std::string str = "argsTest";
    std::vector<std::string> args;
    args.push_back(str);
    std::string result;
    int32_t ret = HiDumpHelper::GetInstance().HiDump(args, result);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: HiDump_003
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDfxTest, HiDump_003, testing::ext::TestSize.Level0)
{
    std::string str = std::string(HIDUMPER_GET_TRUSTED_LIST_INFO);
    std::vector<std::string> args;
    args.push_back(str);
    std::string result;
    int32_t ret = HiDumpHelper::GetInstance().HiDump(args, result);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS