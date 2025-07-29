/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_distributed_hardware_load.h"

#include <iostream>

#include "dm_log.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
void DmDistributedHardwareLoadTest::SetUp() {}

void DmDistributedHardwareLoadTest::TearDown() {}

void DmDistributedHardwareLoadTest::SetUpTestCase() {}

void DmDistributedHardwareLoadTest::TearDownTestCase() {}

HWTEST_F(DmDistributedHardwareLoadTest, LoadDistributedHardwareFwk_001, testing::ext::TestSize.Level0)
{
    DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
    DmDistributedHardwareLoad::GetInstance().InitDistributedHardwareLoadCount();
    uint32_t ret = DmDistributedHardwareLoad::GetInstance().GetDistributedHardwareLoadCount();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDistributedHardwareLoadTest, InitDistributedHardwareLoadCount_001, testing::ext::TestSize.Level0)
{
    DmDistributedHardwareLoad::GetInstance().InitDistributedHardwareLoadCount();
    uint32_t ret = DmDistributedHardwareLoad::GetInstance().distributedHardwareLoadCount_;
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDistributedHardwareLoadTest, GetDistributedHardwareLoadCount_001, testing::ext::TestSize.Level0)
{
    DmDistributedHardwareLoad::GetInstance().GetDistributedHardwareLoadCount();
    uint32_t ret = DmDistributedHardwareLoad::GetInstance().distributedHardwareLoadCount_;
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDistributedHardwareLoadTest, OnLoadSystemAbilitySuccess_001, testing::ext::TestSize.Level0)
{
    int32_t systemAbilityId = 4800;
    sptr<IRemoteObject> remoteObject = nullptr;
    sptr<DistributedHardwareLoadCallback> distributedHardwareLoadCallback_(new DistributedHardwareLoadCallback());
    distributedHardwareLoadCallback_->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    uint32_t ret = DmDistributedHardwareLoad::GetInstance().distributedHardwareLoadCount_;
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDistributedHardwareLoadTest, Sha256_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<Crypto> crypto = std::make_shared<Crypto>();
    std::string text;
    bool isUpper = false;
    std::string ret = crypto->Sha256(text, isUpper);
    EXPECT_NE(ret.empty(), true);
}

HWTEST_F(DmDistributedHardwareLoadTest, GetCryptoAdapter_001, testing::ext::TestSize.Level0)
{
    std::string soName = "soName";
    auto ret = DmAdapterManager::GetInstance().GetCryptoAdapter(soName);
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(DmDistributedHardwareLoadTest, GetUdidHash_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    auto ret = Crypto::GetUdidHash(deviceId, reinterpret_cast<uint8_t *>(deviceIdHash));
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDistributedHardwareLoadTest, GetUdidHash_002, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    unsigned char *udidHash = nullptr;
    auto ret = Crypto::GetUdidHash(deviceId, udidHash);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmDistributedHardwareLoadTest, GetGroupIdHash_001, testing::ext::TestSize.Level0)
{
    std::string groupId = "123456789";
    std::string str = Crypto::GetGroupIdHash(groupId);
    EXPECT_EQ(str.empty(), false);
}

/**
 * @tc.name: ConvertHexStringToBytes_001
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_001, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 2;
    uint32_t outBufLen = 0;
    const char *inBuf = "12";
    unsigned char *outBuf = nullptr;
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ConvertHexStringToBytes_002
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_002, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 0;
    uint32_t outBufLen = 1;
    const char *inBuf = nullptr;
    unsigned char outBuf[1] = {0};
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ConvertHexStringToBytes_003
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_003, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 1;
    uint32_t outBufLen = 1;
    const char *inBuf = "1";
    unsigned char outBuf[1] = {0};
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ConvertHexStringToBytes_004
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_004, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 2;
    uint32_t outBufLen = 2;
    const char *inBuf = "1G";
    unsigned char outBuf[2] = {0};
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ConvertHexStringToBytes_005
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_005, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 2;
    uint32_t outBufLen = 2;
    const char *inBuf = "G1";
    unsigned char outBuf[2] = {0};
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ConvertHexStringToBytes_006
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_006, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 2;
    uint32_t outBufLen = 2;
    const char *inBuf = "a1";
    unsigned char outBuf[2] = {0};
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ConvertHexStringToBytes_007
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_007, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 2;
    uint32_t outBufLen = 2;
    const char *inBuf = "Aa";
    unsigned char outBuf[2] = {0};
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ConvertHexStringToBytes_008
 * @tc.desc: Test ConvertHexStringToBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DmDistributedHardwareLoadTest, ConvertHexStringToBytes_008, testing::ext::TestSize.Level0)
{
    uint32_t inLen = 2;
    uint32_t outBufLen = 2;
    const char *inBuf = "AA";
    unsigned char outBuf[2] = {0};
    auto ret = Crypto::ConvertHexStringToBytes(outBuf, outBufLen, inBuf, inLen);
    EXPECT_EQ(ret, DM_OK);
}
}  // namespace DistributedHardware
}  // namespace OHOS
