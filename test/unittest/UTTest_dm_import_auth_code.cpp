/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "UTTest_dm_import_auth_code.h"
#include "dm_device_info.h"

#include <unistd.h>
#include "accesstoken_kit.h"
#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_skeleton.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {
void DMImportAuthCodeTest::SetUp()
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
        .processName = "com.huawei.msdp.hmringgenerator",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void DMImportAuthCodeTest::TearDown()
{
}

void DMImportAuthCodeTest::SetUpTestCase()
{
}

void DMImportAuthCodeTest::TearDownTestCase()
{
}

namespace {
HWTEST_F(DMImportAuthCodeTest, ImportAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.samples.devicemanager";
    std::string authCode = "123456";
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(pkgName, initcallback);
    ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    bool res = (ret == ERR_DM_NO_PERMISSION) || (ret == DM_OK);
    ASSERT_EQ(res, true);
    DeviceManager::GetInstance().UnInitDeviceManager(pkgName);
}

HWTEST_F(DMImportAuthCodeTest, ExportAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.samples.devicemanager";
    std::string authCode = "";
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(pkgName, initcallback);
    ret = DeviceManager::GetInstance().ExportAuthCode(authCode);
    bool res = (ret == ERR_DM_NO_PERMISSION) || (ret == DM_OK);
    ASSERT_EQ(res, true);
    DeviceManager::GetInstance().UnInitDeviceManager(pkgName);
}

HWTEST_F(DMImportAuthCodeTest, PeerTargetId_001, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId = {
        .deviceId = "pkgName",
        .brMac = "brMac",
        .bleMac = "bleMac",
        .wifiIp = "wifiIp",
        .wifiPort = 0,
    };

    PeerTargetId other;
    ASSERT_NE(other == peerTargetId, true);
    ASSERT_EQ(other < peerTargetId, true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS