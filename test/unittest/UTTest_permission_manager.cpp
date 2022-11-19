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

#include "UTTest_permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
void PremissionManagerTest::SetUp()
{
}

void PremissionManagerTest::TearDown()
{
}

void PremissionManagerTest::SetUpTestCase()
{
}

void PremissionManagerTest::TearDownTestCase()
{
}

namespace {

/**
 * @tc.name: PinAuthUi::CheckPermission_001
 * @tc.desc: the return value is true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PremissionManagerTest, CheckPermission_001, testing::ext::TestSize.Level0)
{
    bool ret = PermissionManager::GetInstance().CheckPermission();
    ASSERT_EQ(ret, true);
}
}
} // namespace DistributedHardware
} // namespace OHOS