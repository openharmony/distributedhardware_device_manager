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

#include <iostream>
#include <string>
#include <unistd.h>

#include "dm_constants.h"
#include "dm_timer.h"
#include "UTTest_dm_timer.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* AUTHENTICATE_TIMEOUT_TASK = "deviceManagerTimer:authenticate";
constexpr int32_t MAX_TIME_OUT_VALUE = 600;
void TimeHeapTest::SetUp()
{
}

void TimeHeapTest::TearDown()
{
}

void TimeHeapTest::SetUpTestCase()
{
}

void TimeHeapTest::TearDownTestCase()
{
}

namespace {
static void TimeOut(std::string timerName) {}

/**
 * @tc.name: TimeHeapTest::StartTimer_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, StartTimer_001, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeout = 10;

    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->StartTimer("", timeout, TimeOut);
    EXPECT_EQ(ERR_DM_INPUT_PARA_INVALID, ret);

    ret = timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK), 700, TimeOut);
    EXPECT_EQ(ERR_DM_INPUT_PARA_INVALID, ret);

    ret = timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK), -20, TimeOut);
    EXPECT_EQ(ERR_DM_INPUT_PARA_INVALID, ret);

    ret = timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK), timeout, nullptr);
    EXPECT_EQ(ERR_DM_INPUT_PARA_INVALID, ret);
}

/**
 * @tc.name: TimeHeapTest::StartTimer_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, StartTimer_002, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    std::string name2 = "test2";
    int32_t timeOut = 10;
    int32_t timeOut2 = 40;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->StartTimer(name, timeOut, TimeOut);
    EXPECT_EQ(DM_OK, ret);

    ret = timer->StartTimer(name2, timeOut2, TimeOut);
    EXPECT_EQ(DM_OK, ret);
}

/**
 * @tc.name: TimeHeapTest::DeleteTimer_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DeleteTimer_001, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->DeleteTimer("");
    EXPECT_EQ(ERR_DM_INPUT_PARA_INVALID, ret);

    ret = timer->DeleteTimer(name);
    EXPECT_EQ(ERR_DM_FAILED, ret);
}

/**
 * @tc.name: TimeHeapTest::DeleteTimer_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DeleteTimer_002, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 10;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    timer->StartTimer(name, timeOut, TimeOut);
    int32_t ret = timer->DeleteTimer(name);
    EXPECT_EQ(DM_OK, ret);
}

/**
 * @tc.name: TimeHeapTest::DeleteAll_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DeleteAll_001, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 10;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->DeleteAll();
    EXPECT_EQ(DM_OK, ret);

    timer->StartTimer(name, timeOut, TimeOut);
    ret = timer->DeleteAll();
    EXPECT_EQ(DM_OK, ret);
}

HWTEST_F(TimeHeapTest, DeleteAll_002, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 20;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->StartTimer(name, timeOut, TimeOut);
    EXPECT_EQ(DM_OK, ret);
    timer->queue_ = nullptr;
    ret = timer->DeleteAll();
    EXPECT_EQ(DM_OK, ret);
}

HWTEST_F(TimeHeapTest, DeleteAll_003, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 20;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->StartTimer(name, timeOut, TimeOut);
    EXPECT_EQ(DM_OK, ret);
    timer->timerVec_[AUTHENTICATE_TIMEOUT_TASK] = nullptr;
    ret = timer->DeleteAll();
    EXPECT_EQ(DM_OK, ret);
}

HWTEST_F(TimeHeapTest, DeleteTimer_003, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 20;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    timer->StartTimer(name, timeOut, TimeOut);
    timer->queue_ = nullptr;
    int32_t ret = timer->DeleteTimer(name);
    EXPECT_EQ(DM_OK, ret);
}

HWTEST_F(TimeHeapTest, DeleteTimer_004, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 20;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    timer->StartTimer(name, timeOut, TimeOut);
    timer->timerVec_[AUTHENTICATE_TIMEOUT_TASK] = nullptr;
    int32_t ret = timer->DeleteTimer(name);
    EXPECT_EQ(DM_OK, ret);
}

/**
 * @tc.name: TimeHeapTest::StartTimer_003
 * @tc.desc: StartTimer with timeOut equal to MAX_TIME_OUT boundary (600) succeeds.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, StartTimer_003, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->StartTimer(name, MAX_TIME_OUT_VALUE, TimeOut);
    EXPECT_EQ(DM_OK, ret);
}

/**
 * @tc.name: TimeHeapTest::StartTimer_004
 * @tc.desc: StartTimer overwrites an existing timer name (re-entry into timerVec_[name]).
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, StartTimer_004, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 10;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->StartTimer(name, timeOut, TimeOut);
    EXPECT_EQ(DM_OK, ret);
    // overwrite the existing entry with a new handle
    ret = timer->StartTimer(name, timeOut, TimeOut);
    EXPECT_EQ(DM_OK, ret);
    EXPECT_EQ(timer->timerVec_.size(), static_cast<size_t>(1));
}

/**
 * @tc.name: TimeHeapTest::DeleteTimer_005
 * @tc.desc: DeleteTimer cancels a real running task (found item, non-null handle).
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DeleteTimer_005, testing::ext::TestSize.Level0)
{
    std::string name = std::string(AUTHENTICATE_TIMEOUT_TASK);
    int32_t timeOut = 300;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    int32_t ret = timer->StartTimer(name, timeOut, TimeOut);
    EXPECT_EQ(DM_OK, ret);
    // item found, non-null handle, queue non-null -> cancel path
    ret = timer->DeleteTimer(name);
    EXPECT_EQ(DM_OK, ret);
    EXPECT_EQ(timer->timerVec_.size(), static_cast<size_t>(0));
}

/**
 * @tc.name: TimeHeapTest::DeleteAll_004
 * @tc.desc: DeleteAll cancels real running tasks (non-null handle) across multiple entries.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DeleteAll_004, testing::ext::TestSize.Level0)
{
    std::string name1 = std::string(AUTHENTICATE_TIMEOUT_TASK);
    std::string name2 = "secondTimer";
    int32_t timeOut = 300;
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>();
    EXPECT_EQ(timer->StartTimer(name1, timeOut, TimeOut), DM_OK);
    EXPECT_EQ(timer->StartTimer(name2, timeOut, TimeOut), DM_OK);
    // both entries non-null handle, queue non-null -> cancel path for each
    int32_t ret = timer->DeleteAll();
    EXPECT_EQ(DM_OK, ret);
    EXPECT_EQ(timer->timerVec_.size(), static_cast<size_t>(0));
}
}
}
}
