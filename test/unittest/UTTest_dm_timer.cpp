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
static void TimeOut(void *data, std::string timerName) {}

/**
 * @tc.name: TimeHeapTest::Tick_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, Tick_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    timerHeap->hsize_ = 0;
    int32_t ret = timerHeap->Tick();
    EXPECT_EQ(DM_AUTH_NO_TIMER, ret);
}

/**
 * @tc.name: TimeHeapTest::Tick_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, Tick_002, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = 10;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    timerHeap->AddTimer(AUTHENTICATE_TIMEOUT_TASK, 10, TimeOut, timer);
    int32_t ret = timerHeap->Tick();
    EXPECT_EQ(DM_OK, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::Tick_003
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, Tick_003, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = 10;

    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    timerHeap->AddTimer(AUTHENTICATE_TIMEOUT_TASK, 10, TimeOut, timer);
    timerHeap->AddTimer(AUTHENTICATE_TIMEOUT_TASK, 20, TimeOut, timer);
    int32_t ret = timerHeap->Tick();
    EXPECT_EQ(DM_OK, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::MoveUp_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, MoveUp_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    std::shared_ptr<DmTimer> timer = nullptr;
    int32_t ret = timerHeap->MoveUp(timer);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::MoveUp_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, MoveUp_002, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = 10;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>(name, timeout + time(NULL), nullptr, TimeOut);
    timerHeap->hsize_ = 0;
    int32_t ret = timerHeap->MoveUp(timer);
    EXPECT_EQ(DM_INVALID_VALUE, ret);

    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::MoveUp_003
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, MoveUp_003, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = 10;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    std::shared_ptr<DmTimer> timer = std::make_shared<DmTimer>(name, timeout + time(NULL), nullptr, TimeOut);
    timerHeap->hsize_ = 1;
    int32_t ret = timerHeap->MoveUp(timer);
    EXPECT_EQ(DM_OK, ret);

    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::AddTimer_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, AddTimer_001, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = 10;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    int32_t ret = timerHeap->AddTimer(name, timeout, TimeOut, timer);
    EXPECT_EQ(DM_OK, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::AddTimer_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, AddTimer_002, testing::ext::TestSize.Level0)
{
    std::string name = "";
    int32_t timeout = 10;

    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    int32_t ret = timerHeap->AddTimer(name, timeout, TimeOut, timer);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::AddTimer_003
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, AddTimer_003, testing::ext::TestSize.Level0)
{
    std::string name = "timer";
    int32_t timeout = 10;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    int32_t ret = timerHeap->AddTimer(name, timeout, TimeOut, timer);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::AddTimer_004
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, AddTimer_004, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = -1;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    int32_t ret = timerHeap->AddTimer(name, timeout, TimeOut, timer);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::AddTimer_005
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, AddTimer_005, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = -1;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    int32_t ret = timerHeap->AddTimer(name, timeout, TimeOut, timer);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
    timerHeap->DelAll();
}

/**
 * @tc.name: TimeHeapTest::DelTimer_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DelTimer_001, testing::ext::TestSize.Level0)
{
    std::string name = "";

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    int32_t ret = timerHeap->DelTimer(name);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
}

/**
 * @tc.name: TimeHeapTest::DelTimer_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DelTimer_002, testing::ext::TestSize.Level0)
{
    std::string name = "timer";

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    int32_t ret = timerHeap->DelTimer(name);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
}

/**
 * @tc.name: TimeHeapTest::DelTimer_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DelTimer_003, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;

    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    int32_t ret = timerHeap->DelTimer(name);
    EXPECT_EQ(DM_INVALID_VALUE, ret);
}

/**
 * @tc.name: TimeHeapTest::DelTimer_004
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DelTimer_004, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = 10;

    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    timerHeap->AddTimer(name, timeout, TimeOut, timer);
    int32_t ret = timerHeap->DelTimer(name);
    EXPECT_EQ(DM_OK, ret);
}

/**
 * @tc.name: TimeHeapTest::DelAll_001
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DelAll_001, testing::ext::TestSize.Level0)
{
    std::string name = AUTHENTICATE_TIMEOUT_TASK;
    int32_t timeout = 10;

    DmTimer *timer = new DmTimer(name, timeout + time(NULL), nullptr, TimeOut);
    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    timerHeap->AddTimer(name, timeout, TimeOut, timer);
    int32_t ret = timerHeap->DelAll();
    EXPECT_EQ(DM_OK, ret);
    EXPECT_EQ(timerHeap->hsize_, 0);
}

/**
 * @tc.name: TimeHeapTest::DelAll_002
 * @tc.desc: Timeout event trigger
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(TimeHeapTest, DelAll_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<TimeHeap> timerHeap = std::make_shared<TimeHeap>();
    int32_t ret = timerHeap->DelAll();
    EXPECT_EQ(DM_OK, ret);
}
}
}
}
