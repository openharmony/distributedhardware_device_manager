/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "UTTest_dm_comm_tool.h"

namespace OHOS {
namespace DistributedHardware {
void DMCommToolTest::SetUp()
{
    dmCommTool = DMCommTool::GetInstance();
    dmCommTool->Init();
}
void DMCommToolTest::TearDown()
{
}
void DMCommToolTest::SetUpTestCase()
{
}
void DMCommToolTest::TearDownTestCase()
{
}

/**
 * @tc.name: GetEventHandler_NotNull
 * @tc.type: FUNC
 */
HWTEST_F(DMCommToolTest, GetEventHandler_NotNull, testing::ext::TestSize.Level0)
{
    auto handler = dmCommTool->GetEventHandler();
    EXPECT_NE(handler, nullptr);
}

/**
 * @tc.name: GetDMTransportPtr_NotNull
 * @tc.type: FUNC
 */
HWTEST_F(DMCommToolTest, GetDMTransportPtr_NotNull, testing::ext::TestSize.Level0)
{
    auto transportPtr = dmCommTool->GetDMTransportPtr();
    EXPECT_NE(transportPtr, nullptr);
}
} // DistributedHardware
} // OHOS