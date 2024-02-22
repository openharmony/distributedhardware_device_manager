/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {
void PremissionManagerTest::SetUp()
{
    const int32_t permsNum = 2;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dsoftbus_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
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

/**
 * @tc.name: PinAuthUi::GetCallerProcessName_001
 * @tc.desc: the return value is not DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PremissionManagerTest, GetCallerProcessName_001, testing::ext::TestSize.Level0)
{
    std::string processName;
    int32_t ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: PinAuthUi::CheckProcessNameValidOnAuthCode_001
 * @tc.desc: the return value is false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PremissionManagerTest, CheckProcessNameValidOnAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string processName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(processName);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: PinAuthUi::CheckProcessNameValidOnAuthCode_002
 * @tc.desc: the return value is true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PremissionManagerTest, CheckProcessNameValidOnAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string processName = "processName";
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(processName);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: PinAuthUi::CheckProcessNameValidOnPinHolder_001
 * @tc.desc: the return value is false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PremissionManagerTest, CheckProcessNameValidOnPinHolder_001, testing::ext::TestSize.Level0)
{
    std::string processName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: PinAuthUi::CheckProcessNameValidOnPinHolder_002
 * @tc.desc: the return value is true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PremissionManagerTest, CheckProcessNameValidOnPinHolder_002, testing::ext::TestSize.Level0)
{
    std::string processName = "processName";
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName);
    ASSERT_EQ(ret, false);
}
}
} // namespace DistributedHardware
} // namespace OHOS