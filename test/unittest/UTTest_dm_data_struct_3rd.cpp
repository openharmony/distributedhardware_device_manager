/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UTTest_dm_data_struct_3rd.h"

namespace OHOS {
namespace DistributedHardware {

void DmDataStruct3rdTest::SetUp()
{
}

void DmDataStruct3rdTest::TearDown()
{
}

void DmDataStruct3rdTest::SetUpTestCase()
{
}

void DmDataStruct3rdTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: ProcessInfo3rdEquals_001
 * @tc.desc: Test operator== with identical ProcessInfo3rd objects
 *           Step 1: Prepare two ProcessInfo3rd objects with same values
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(DmDataStruct3rdTest, ProcessInfo3rdEquals_001, testing::ext::TestSize.Level1)
{
    ProcessInfo3rd key1;
    key1.tokenId = 12345;
    key1.uid = 67890;
    key1.processName = "com.example.process";
    key1.businessName = "com.example.business";
    key1.userId = 54321;

    ProcessInfo3rd key2;
    key2.tokenId = 12345;
    key2.uid = 67890;
    key2.processName = "com.example.process";
    key2.businessName = "com.example.business";
    key2.userId = 54321;

    bool result = (key1 == key2);
    EXPECT_TRUE(result);
}

HWTEST_F(DmDataStruct3rdTest, ProcessInfo3rdEquals_002, testing::ext::TestSize.Level1)
{
    ProcessInfo3rd key1;
    key1.tokenId = 12345;
    key1.uid = 67890;
    key1.processName = "com.example.process";
    key1.businessName = "com.example.business";
    key1.userId = 54321;

    ProcessInfo3rd key2;
    key2.tokenId = 12346;
    key2.uid = 67890;
    key2.processName = "com.example.process";
    key2.businessName = "com.example.business";
    key2.userId = 54321;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, ProcessInfo3rdEquals_003, testing::ext::TestSize.Level1)
{
    ProcessInfo3rd key1;
    key1.tokenId = 12345;
    key1.uid = 67890;
    key1.processName = "com.example.process";
    key1.businessName = "com.example.business";
    key1.userId = 54321;

    ProcessInfo3rd key2;
    key2.tokenId = 12345;
    key2.uid = 67899;
    key2.processName = "com.example.process";
    key2.businessName = "com.example.business";
    key2.userId = 54321;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, ProcessInfo3rdEquals_004, testing::ext::TestSize.Level1)
{
    ProcessInfo3rd key1;
    key1.tokenId = 12345;
    key1.uid = 67890;
    key1.processName = "com.example.process";
    key1.businessName = "com.example.business";
    key1.userId = 54321;

    ProcessInfo3rd key2;
    key2.tokenId = 12345;
    key2.uid = 67890;
    key2.processName = "com.example.process11";
    key2.businessName = "com.example.business";
    key2.userId = 54321;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, ProcessInfo3rdEquals_005, testing::ext::TestSize.Level1)
{
    ProcessInfo3rd key1;
    key1.tokenId = 12345;
    key1.uid = 67890;
    key1.processName = "com.example.process";
    key1.businessName = "com.example.business";
    key1.userId = 54321;

    ProcessInfo3rd key2;
    key2.tokenId = 12345;
    key2.uid = 67890;
    key2.processName = "com.example.process";
    key2.businessName = "com.example.business11";
    key2.userId = 54321;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, ProcessInfo3rdEquals_006, testing::ext::TestSize.Level1)
{
    ProcessInfo3rd key1;
    key1.tokenId = 12345;
    key1.uid = 67890;
    key1.processName = "com.example.process";
    key1.businessName = "com.example.business";
    key1.userId = 54321;

    ProcessInfo3rd key2;
    key2.tokenId = 12345;
    key2.uid = 67890;
    key2.processName = "com.example.process";
    key2.businessName = "com.example.business";
    key2.userId = 54333;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, ProcessInfo3rdEquals_007, testing::ext::TestSize.Level1)
{
    ProcessInfo3rd key1;
    key1.tokenId = 0;
    key1.uid = 0;
    key1.processName = "";
    key1.businessName = "";
    key1.userId = 0;

    ProcessInfo3rd key2;
    key2.tokenId = 0;
    key2.uid = 0;
    key2.processName = "";
    key2.businessName = "";
    key2.userId = 0;

    bool result = (key1 == key2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: PeerTargetId3rdEquals_001
 * @tc.desc: Test operator== with identical PeerTargetId3rd objects
 *           Step 1: Prepare two PeerTargetId3rd objects with same values
 *           Step 2: Compare objects using operator==
 *           Step 3: Verify they are equal
 * @tc.type: FUNC
 */
HWTEST_F(DmDataStruct3rdTest, PeerTargetId3rdEquals_001, testing::ext::TestSize.Level1)
{
    PeerTargetId3rd key1;
    key1.deviceId = "device123";
    key1.brMac = "00:11:22:33:44:55";
    key1.bleMac = "AA:BB:CC:DD:EE:FF";
    key1.wifiIp = "192.168.1.1";
    key1.wifiPort = 12345;

    PeerTargetId3rd key2;
    key2.deviceId = "device123";
    key2.brMac = "00:11:22:33:44:55";
    key2.bleMac = "AA:BB:CC:DD:EE:FF";
    key2.wifiIp = "192.168.1.1";
    key2.wifiPort = 12345;

    bool result = (key1 == key2);
    EXPECT_TRUE(result);
}

HWTEST_F(DmDataStruct3rdTest, PeerTargetId3rdEquals_002, testing::ext::TestSize.Level1)
{
    PeerTargetId3rd key1;
    key1.deviceId = "device123";
    key1.brMac = "00:11:22:33:44:55";
    key1.bleMac = "AA:BB:CC:DD:EE:FF";
    key1.wifiIp = "192.168.1.1";
    key1.wifiPort = 12345;

    PeerTargetId3rd key2;
    key2.deviceId = "device3333";
    key2.brMac = "00:11:22:33:44:55";
    key2.bleMac = "AA:BB:CC:DD:EE:FF";
    key2.wifiIp = "192.168.1.1";
    key2.wifiPort = 12345;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, PeerTargetId3rdEquals_003, testing::ext::TestSize.Level1)
{
    PeerTargetId3rd key1;
    key1.deviceId = "device123";
    key1.brMac = "00:11:22:33:44:55";
    key1.bleMac = "AA:BB:CC:DD:EE:FF";
    key1.wifiIp = "192.168.1.1";
    key1.wifiPort = 12345;

    PeerTargetId3rd key2;
    key2.deviceId = "device123";
    key2.brMac = "00:11:22:33:44:66";
    key2.bleMac = "AA:BB:CC:DD:EE:FF";
    key2.wifiIp = "192.168.1.1";
    key2.wifiPort = 12345;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, PeerTargetId3rdEquals_004, testing::ext::TestSize.Level1)
{
    PeerTargetId3rd key1;
    key1.deviceId = "device123";
    key1.brMac = "00:11:22:33:44:55";
    key1.bleMac = "AA:BB:CC:DD:EE:FF";
    key1.wifiIp = "192.168.1.1";
    key1.wifiPort = 12345;

    PeerTargetId3rd key2;
    key2.deviceId = "device123";
    key2.brMac = "00:11:22:33:44:55";
    key2.bleMac = "AA:BB:CC:DD:EE:FG";
    key2.wifiIp = "192.168.1.100";
    key2.wifiPort = 12365;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, PeerTargetId3rdEquals_005, testing::ext::TestSize.Level1)
{
    PeerTargetId3rd key1;
    key1.deviceId = "device123";
    key1.brMac = "00:11:22:33:44:55";
    key1.bleMac = "AA:BB:CC:DD:EE:FF";
    key1.wifiIp = "192.168.1.1";
    key1.wifiPort = 12345;

    PeerTargetId3rd key2;
    key2.deviceId = "device123";
    key2.brMac = "00:11:22:33:44:55";
    key2.bleMac = "AA:BB:CC:DD:EE:FF";
    key2.wifiIp = "192.168.1.100";
    key2.wifiPort = 12345;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, PeerTargetId3rdEquals_006, testing::ext::TestSize.Level1)
{
    PeerTargetId3rd key1;
    key1.deviceId = "device123";
    key1.brMac = "00:11:22:33:44:55";
    key1.bleMac = "AA:BB:CC:DD:EE:FF";
    key1.wifiIp = "192.168.1.1";
    key1.wifiPort = 12345;

    PeerTargetId3rd key2;
    key2.deviceId = "device123";
    key2.brMac = "00:11:22:33:44:55";
    key2.bleMac = "AA:BB:CC:DD:EE:FF";
    key2.wifiIp = "192.168.1.1";
    key2.wifiPort = 12365;

    bool result = (key1 == key2);
    EXPECT_FALSE(result);
}

HWTEST_F(DmDataStruct3rdTest, PeerTargetId3rdEquals_007, testing::ext::TestSize.Level1)
{
    PeerTargetId3rd key1;
    key1.deviceId = "";
    key1.brMac = "";
    key1.bleMac = "";
    key1.wifiIp = "";
    key1.wifiPort = 0;

    PeerTargetId3rd key2;
    key2.deviceId = "";
    key2.brMac = "";
    key2.bleMac = "";
    key2.wifiIp = "";
    key2.wifiPort = 0;

    bool result = (key1 == key2);
    EXPECT_TRUE(result);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
