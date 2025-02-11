/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "UTTest_dm_service_load.h"

#include <unistd.h>

#include "dm_constants.h"
#include "dm_service_load.h"
#include "dm_system_ability_manager_mock.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using ::testing::_;
using ::testing::An;
using ::testing::Return;

namespace OHOS {
namespace DistributedHardware {
void DmServiceLoadTest::SetUp()
{
}

void DmServiceLoadTest::TearDown()
{
}

void DmServiceLoadTest::SetUpTestCase()
{
}

void DmServiceLoadTest::TearDownTestCase()
{
}

namespace {
HWTEST_F(DmServiceLoadTest, LoadDMService_001, testing::ext::TestSize.Level0)
{
    int32_t ret = DmServiceLoad::GetInstance().LoadDMService();
    ASSERT_EQ(ret, DM_OK);
    ret = DmServiceLoad::GetInstance().LoadDMService();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmServiceLoadTest, SetLoadFinish_001, testing::ext::TestSize.Level0)
{
    DmServiceLoad::GetInstance().SetLoadFinish();
    int32_t systemAbilityId = 1000;
    sptr<IRemoteObject> remoteObject = nullptr;
    DMLoadCallbackTest dmLoadCallback;
    dmLoadCallback.OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    dmLoadCallback.OnLoadSystemAbilityFail(systemAbilityId);
    ASSERT_EQ(DmServiceLoad::GetInstance().isDMServiceLoading_, false);
}

HWTEST_F(DmServiceLoadTest, LoadDMService_002, testing::ext::TestSize.Level2)
{
    auto iSystemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto mockAbilityMgr = sptr<SystemAbilityManagerMock>(new SystemAbilityManagerMock());
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockAbilityMgr;
    EXPECT_CALL(*mockAbilityMgr, LoadSystemAbility(_, An<const sptr<ISystemAbilityLoadCallback>&>()))
        .Times(1)
        .WillOnce(Return(ERR_DM_FAILED));

    int32_t ret = DmServiceLoad::GetInstance().LoadDMService();
    EXPECT_EQ(ret, ERR_DM_FAILED);
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr;
}
}
} // namespace DistributedHardware
} // namespace OHOS
