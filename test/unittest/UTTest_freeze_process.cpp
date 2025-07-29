/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_freeze_process.h"

#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
void FreezeProcessTest::SetUp()
{
}

void FreezeProcessTest::TearDown()
{
}

void FreezeProcessTest::SetUpTestCase()
{
}

void FreezeProcessTest::TearDownTestCase()
{
}

namespace {
constexpr int64_t DATA_REFRESH_INTERVAL = 20 * 60;
}
HWTEST_F(FreezeProcessTest, UpdateFreezeRecord, testing::ext::TestSize.Level0)
{
    int64_t nowTime = 1633072800 ;
    BindFailedEvents bindFailedEventsCache;
    bindFailedEventsCache.failedTimeStamps = {1633072700, 1633072800, 1633072900};
    bindFailedEventsCache.freezeTimeStamps = {1633072600};
    FreezeProcess freezeProcess;
    freezeProcess.bindFailedEventsCache_ = bindFailedEventsCache;
    int32_t result = freezeProcess.UpdateFreezeRecord();
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToBindFailedEvents_WhenJsonIsEmpty, testing::ext::TestSize.Level0)
{
    std::string emptyResult = "";
    BindFailedEvents bindFailedEventsObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToBindFailedEvents(emptyResult, bindFailedEventsObj);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToBindFailedEvents_WhenJsonIsInvalid, testing::ext::TestSize.Level0)
{
    std::string invalidJsonResult = "invalid_json";
    BindFailedEvents bindFailedEventsObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToBindFailedEvents(invalidJsonResult, bindFailedEventsObj);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToBindFailedEvents_WhenFailedTimestampsExist, testing::ext::TestSize.Level0)
{
    std::string validJsonResult = R"({"failedTimeStamps": [123456, 789012]})";
    BindFailedEvents bindFailedEventsObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToBindFailedEvents(validJsonResult, bindFailedEventsObj);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(bindFailedEventsObj.failedTimeStamps.size(), 2);
    EXPECT_EQ(bindFailedEventsObj.failedTimeStamps[0], 123456);
    EXPECT_EQ(bindFailedEventsObj.failedTimeStamps[1], 789012);
    EXPECT_EQ(bindFailedEventsObj.freezeTimeStamps.size(), 0);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToBindFailedEvents_WhenFreezeTimestampsExist, testing::ext::TestSize.Level0)
{
    std::string validJsonResult = R"({"freezeTimeStamps": [345678, 901234]})";
    BindFailedEvents bindFailedEventsObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToBindFailedEvents(validJsonResult, bindFailedEventsObj);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(bindFailedEventsObj.failedTimeStamps.size(), 0);
    EXPECT_EQ(bindFailedEventsObj.freezeTimeStamps.size(), 2);
    EXPECT_EQ(bindFailedEventsObj.freezeTimeStamps[0], 345678);
    EXPECT_EQ(bindFailedEventsObj.freezeTimeStamps[1], 901234);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToBindFailedEvents_WhenAllExist, testing::ext::TestSize.Level0)
{
    std::string validJsonResult = R"({"failedTimeStamps": [123456, 789012],
        "freezeTimeStamps": [345678, 901234]})";
    BindFailedEvents bindFailedEventsObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToBindFailedEvents(validJsonResult, bindFailedEventsObj);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(bindFailedEventsObj.failedTimeStamps.size(), 2);
    EXPECT_EQ(bindFailedEventsObj.failedTimeStamps[0], 123456);
    EXPECT_EQ(bindFailedEventsObj.failedTimeStamps[1], 789012);
    EXPECT_EQ(bindFailedEventsObj.freezeTimeStamps.size(), 2);
    EXPECT_EQ(bindFailedEventsObj.freezeTimeStamps[0], 345678);
    EXPECT_EQ(bindFailedEventsObj.freezeTimeStamps[1], 901234);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToDeviceFreezeState_WhenJsonIsEmpty, testing::ext::TestSize.Level0)
{
    std::string emptyResult = "";
    DeviceFreezeState freezeStateObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToDeviceFreezeState(emptyResult, freezeStateObj);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToDeviceFreezeState_WhenJsonIsInvalid, testing::ext::TestSize.Level0)
{
    std::string invalidJsonResult = "invalid_json";
    DeviceFreezeState freezeStateObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToDeviceFreezeState(invalidJsonResult, freezeStateObj);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToDeviceFreezeState_WhenStartTimestampsExist, testing::ext::TestSize.Level0)
{
    std::string validJsonResult = "{\"startFreezeTimeStamp\":1234567890}";
    DeviceFreezeState freezeStateObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToDeviceFreezeState(validJsonResult, freezeStateObj);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(freezeStateObj.startFreezeTimeStamp, 1234567890);
    EXPECT_EQ(freezeStateObj.stopFreezeTimeStamp, 0);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToDeviceFreezeState_WhenStopTimestampsExist, testing::ext::TestSize.Level0)
{
    std::string validJsonResult = "{\"stopFreezeTimeStamp\":9876543210}";
    DeviceFreezeState freezeStateObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToDeviceFreezeState(validJsonResult, freezeStateObj);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(freezeStateObj.startFreezeTimeStamp, 0);
    EXPECT_EQ(freezeStateObj.stopFreezeTimeStamp, 9876543210);
}

HWTEST_F(FreezeProcessTest, ConvertJsonToDeviceFreezeState_WhenAllExist, testing::ext::TestSize.Level0)
{
    std::string validJsonResult = "{\"startFreezeTimeStamp\":1234567890,\"stopFreezeTimeStamp\":9876543210}";
    DeviceFreezeState freezeStateObj;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.ConvertJsonToDeviceFreezeState(validJsonResult, freezeStateObj);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(freezeStateObj.startFreezeTimeStamp, 1234567890);
    EXPECT_EQ(freezeStateObj.stopFreezeTimeStamp, 9876543210);
}

/* *
 * @tc.name: CleanBindFailedEvents_001
 * @tc.desc: Test CleanBindFailedEvents
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FreezeProcessTest, CleanBindFailedEvents_001, testing::ext::TestSize.Level0)
{
    int64_t reservedDataTimeStamp = 12;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.CleanBindFailedEvents(reservedDataTimeStamp);

    EXPECT_EQ(result, DM_OK);
}

/* *
 * @tc.name: CleanFreezeState_001
 * @tc.desc: Test CleanFreezeState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FreezeProcessTest, CleanFreezeState_001, testing::ext::TestSize.Level0)
{
    int64_t reservedDataTimeStamp = 12;
    FreezeProcess freezeProcess;
    int32_t result = freezeProcess.CleanFreezeState(reservedDataTimeStamp);

    EXPECT_EQ(result, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
