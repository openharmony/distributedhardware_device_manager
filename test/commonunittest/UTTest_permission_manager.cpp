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
void PermissionManagerTest::SetUp()
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
constexpr int32_t PAKAGE_NAME_SIZE_MAX = 256;

#define SYSTEM_SA_WHITE_LIST_NUM (4)
constexpr const static char systemSaWhiteList[SYSTEM_SA_WHITE_LIST_NUM][PAKAGE_NAME_SIZE_MAX] = {
    "Samgr_Networking",
    "ohos.distributeddata.service",
    "ohos.dslm",
    "ohos.deviceprofile",
};

void PermissionManagerTest::TearDown()
{
}

void PermissionManagerTest::SetUpTestCase()
{
}

void PermissionManagerTest::TearDownTestCase()
{
}

namespace {

/**
 * @tc.name: PinAuthUi::CheckPermission_001
 * @tc.desc: the return value is true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PermissionManagerTest, CheckPermission_001, testing::ext::TestSize.Level0)
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
HWTEST_F(PermissionManagerTest, GetCallerProcessName_001, testing::ext::TestSize.Level0)
{
    std::string processName;
    int32_t ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: PinAuthUi::CheckWhiteListSystemSA_001
 * @tc.desc: the return value is false
 * @tc.type：FUNC
 * @tc.require: AR000GHSJK
*/
HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_EQ(ret, false);
    pkgName = "pkgName";
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: PinAuthUi::CheckWhiteListSystemSA_002
 * @tc.desc: the return value is true
 * @tc.type：FUNC
 * @tc.require: AR000GHSJK
*/
HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_002, testing::ext::TestSize.Level0)
{
    std::string pkgName1(systemSaWhiteList[0]);
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName1);
    ASSERT_EQ(ret, true);
    std::string pkgName2(systemSaWhiteList[1]);
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName2);
    ASSERT_EQ(ret, true);
    std::string pkgName3(systemSaWhiteList[2]);
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName3);
    ASSERT_EQ(ret, true);
    std::string pkgName4(systemSaWhiteList[3]);
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName4);
    ASSERT_EQ(ret, true);
}

HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_101, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_102, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos.dhardware";
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckMonitorPermission_001, testing::ext::TestSize.Level0)
{
    bool ret = PermissionManager::GetInstance().CheckMonitorPermission();
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, GetWhiteListSystemSA_001, testing::ext::TestSize.Level0)
{
    auto ret = PermissionManager::GetInstance().GetWhiteListSystemSA();
    ASSERT_FALSE(ret.empty());
}
}
} // namespace DistributedHardware
} // namespace OHOS