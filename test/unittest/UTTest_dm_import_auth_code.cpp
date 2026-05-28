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
        .processName = "CollaborationFwk",
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
/**
 * @tc.name: ImportAuthCode_001
 * @tc.desc: Test ImportAuthCode with valid parameters
 *           Step 1: Initialize device manager
 *           Step 2: Call ImportAuthCode with valid auth code
 *           Step 3: Verify return value is valid
 * @tc.type: FUNC
 */
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

/**
 * @tc.name: ExportAuthCode_001
 * @tc.desc: Test ExportAuthCode functionality
 *           Step 1: Initialize device manager
 *           Step 2: Call ExportAuthCode
 *           Step 3: Verify return value is valid
 * @tc.type: FUNC
 */
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

/**
 * @tc.name: PeerTargetIdEquals_001
 * @tc.desc: Test operator== with identical PeerTargetId objects
 *           Step 1: Prepare two PeerTargetId objects with same values
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_001, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdEquals_002
 * @tc.desc: Test operator== with different deviceId values
 *           Step 1: Prepare two PeerTargetId objects with different deviceId
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_002, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device456",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PeerTargetIdEquals_003
 * @tc.desc: Test operator== with different brMac values
 *           Step 1: Prepare two PeerTargetId objects with different brMac
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_003, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:56",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PeerTargetIdEquals_004
 * @tc.desc: Test operator== with different bleMac values
 *           Step 1: Prepare two PeerTargetId objects with different bleMac
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_004, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FE",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PeerTargetIdEquals_005
 * @tc.desc: Test operator== with different wifiIp values
 *           Step 1: Prepare two PeerTargetId objects with different wifiIp
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_005, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.2",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PeerTargetIdEquals_006
 * @tc.desc: Test operator== with different wifiPort values
 *           Step 1: Prepare two PeerTargetId objects with different wifiPort
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_006, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12346,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PeerTargetIdEquals_007
 * @tc.desc: Test operator== with different serviceId values
 *           Step 1: Prepare two PeerTargetId objects with different serviceId
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are not equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_007, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 200,
    };

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PeerTargetIdEquals_008
 * @tc.desc: Test operator== with default-initialized PeerTargetId objects
 *           Step 1: Prepare two default-initialized PeerTargetId objects
 *           Step 2: Compare using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdEquals_008, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1;
    PeerTargetId peerTargetId2;

    bool result = (peerTargetId1 == peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdLess_001
 * @tc.desc: Test operator< with deviceId comparison
 *           Step 1: Prepare PeerTargetId with smaller deviceId
 *           Step 2: Compare using operator<
 *           Step 3: Verify less than comparison returns true
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_001, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device456",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdLess_002
 * @tc.desc: Test operator< with brMac comparison
 *           Step 1: Prepare PeerTargetId with smaller brMac
 *           Step 2: Compare using operator<
 *           Step 3: Verify less than comparison returns true
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_002, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:54",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdLess_003
 * @tc.desc: Test operator< with bleMac comparison
 *           Step 1: Prepare PeerTargetId with smaller bleMac
 *           Step 2: Compare using operator<
 *           Step 3: Verify less than comparison returns true
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_003, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:EF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdLess_004
 * @tc.desc: Test operator< with wifiIp comparison
 *           Step 1: Prepare PeerTargetId with smaller wifiIp
 *           Step 2: Compare using operator<
 *           Step 3: Verify less than comparison returns true
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_004, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.0",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdLess_005
 * @tc.desc: Test operator< with wifiPort comparison
 *           Step 1: Prepare PeerTargetId with smaller wifiPort
 *           Step 2: Compare using operator<
 *           Step 3: Verify less than comparison returns true
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_005, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12344,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdLess_006
 * @tc.desc: Test operator< with serviceId comparison
 *           Step 1: Prepare PeerTargetId with smaller serviceId
 *           Step 2: Compare using operator<
 *           Step 3: Verify less than comparison returns true
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_006, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 99,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetIdLess_007
 * @tc.desc: Test operator< with equal PeerTargetId objects
 *           Step 1: Prepare two equal PeerTargetId objects
 *           Step 2: Compare using operator<
 *           Step 3: Verify less than comparison returns false
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_007, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    PeerTargetId peerTargetId2 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PeerTargetIdLess_008
 * @tc.desc: Test operator< with self comparison
 *           Step 1: Prepare PeerTargetId object
 *           Step 2: Compare with itself using operator<
 *           Step 3: Verify less than comparison returns false
 * @tc.type: FUNC
 */
HWTEST_F(DMImportAuthCodeTest, PeerTargetIdLess_008, testing::ext::TestSize.Level0)
{
    PeerTargetId peerTargetId1 = {
        .deviceId = "device123",
        .brMac = "00:11:22:33:44:55",
        .bleMac = "AA:BB:CC:DD:EE:FF",
        .wifiIp = "192.168.1.1",
        .wifiPort = 12345,
        .serviceId = 100,
    };

    bool result = (peerTargetId1 < peerTargetId1);
    EXPECT_FALSE(result);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS