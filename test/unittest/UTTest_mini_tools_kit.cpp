/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_mini_tools_kit.h"
#include "accesstoken_kit.h"
#include "dm_constants.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"


namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t TEST_MAX_NAME_LENGTH = 30;
    const std::string TEST_PKG_NAME = "test_pkg_name";
    const std::string TEST_SERVICE_NAME = "test_service_name";
} // namespace
void MiniToolsKitTest::SetUp()
{
    const int32_t permsNum = 3;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    const int32_t indexTwo = 2;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[indexTwo] = "ohos.permission.MONITOR_DEVICE_NETWORK_STATE";
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

void MiniToolsKitTest::TearDown()
{}

void MiniToolsKitTest::SetUpTestCase()
{}

void MiniToolsKitTest::TearDownTestCase()
{}

HWTEST_F(MiniToolsKitTest, InitDeviceManager_001, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerMini::GetInstance().InitDeviceManager(TEST_PKG_NAME);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniToolsKitTest, InitDeviceManager_002, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerMini::GetInstance().InitDeviceManager("");
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(MiniToolsKitTest, GetLocalDisplayDeviceName001, testing::ext::TestSize.Level1)
{
    std::string localName = "";
    int32_t ret = DeviceManagerMini::GetInstance().GetLocalDisplayDeviceName(TEST_PKG_NAME, TEST_MAX_NAME_LENGTH,
        localName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniToolsKitTest, UnInitDeviceManager_001, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerMini::GetInstance().UnInitDeviceManager(TEST_PKG_NAME);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MiniToolsKitTest, UnInitDeviceManager_002, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerMini::GetInstance().UnInitDeviceManager("");
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(MiniToolsKitTest, OnDmServiceDied_001, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerImplMini::GetInstance().OnDmServiceDied();
    EXPECT_EQ(ret, DM_OK);
}

} // namespace DistributedHardware
} // namespace OHOS
