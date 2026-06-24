/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "UTTest_dm_crypto_mgr.h"

#include "dm_constants.h"
#include "crypto_mgr.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;
}
void DmCryptoMgrTest::SetUp()
{
}
void DmCryptoMgrTest::TearDown()
{
}
void DmCryptoMgrTest::SetUpTestCase()
{
}
void DmCryptoMgrTest::TearDownTestCase()
{
}

/**
 * @tc.name: ProcessSessionKey_NullKey_Test
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoMgrTest, ProcessSessionKey_NullKey_Test, testing::ext::TestSize.Level1)
{
    CryptoMgr cryptoMgr;
    uint8_t *sessionKey = nullptr;
    uint32_t keyLen = 10;
    int32_t ret = cryptoMgr.ProcessSessionKey(sessionKey, keyLen);
    EXPECT_EQ(ret, ERR_DM_PROCESS_SESSION_KEY_FAILED);
}

/**
 * @tc.name: ProcessSessionKey_ExceedMaxLength_Test
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoMgrTest, ProcessSessionKey_ExceedMaxLength_Test, testing::ext::TestSize.Level1)
{
    CryptoMgr cryptoMgr;
    uint8_t sessionKey[MAX_SESSION_KEY_LENGTH + 1] = {0};
    uint32_t keyLen = MAX_SESSION_KEY_LENGTH + 1;
    int32_t ret = cryptoMgr.ProcessSessionKey(sessionKey, keyLen);
    EXPECT_EQ(ret, ERR_DM_PROCESS_SESSION_KEY_FAILED);
}

/**
 * @tc.name: ProcessSessionKey_ValidKey_Test
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoMgrTest, ProcessSessionKey_ValidKey_Test, testing::ext::TestSize.Level1)
{
    CryptoMgr cryptoMgr;
    uint8_t sessionKey[10] = {0};
    uint32_t keyLen = 10;
    int32_t ret = cryptoMgr.ProcessSessionKey(sessionKey, keyLen);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ProcessSessionKey_BoundaryMaxLength_Test
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoMgrTest, ProcessSessionKey_BoundaryMaxLength_Test, testing::ext::TestSize.Level2)
{
    // keyLen exactly equals MAX_SESSION_KEY_LENGTH (512): condition is keyLen > MAX, so this must
    // NOT take the early-return branch and should succeed.
    CryptoMgr cryptoMgr;
    uint8_t sessionKey[MAX_SESSION_KEY_LENGTH] = {0};
    uint32_t keyLen = MAX_SESSION_KEY_LENGTH;
    int32_t ret = cryptoMgr.ProcessSessionKey(sessionKey, keyLen);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ProcessSessionKey_Twice_Test
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoMgrTest, ProcessSessionKey_Twice_Test, testing::ext::TestSize.Level2)
{
    // Calling ProcessSessionKey twice exercises the internal ClearSessionKey path that frees an
    // already-allocated key before reallocating.
    CryptoMgr cryptoMgr;
    uint8_t sessionKey1[16] = {0};
    int32_t ret = cryptoMgr.ProcessSessionKey(sessionKey1, 16);
    EXPECT_EQ(ret, DM_OK);
    uint8_t sessionKey2[32] = {0};
    ret = cryptoMgr.ProcessSessionKey(sessionKey2, 32);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetSessionKey_AfterProcess_Test
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoMgrTest, GetSessionKey_AfterProcess_Test, testing::ext::TestSize.Level2)
{
    // Cover GetSessionKey getter returning the key material stored by ProcessSessionKey.
    CryptoMgr cryptoMgr;
    uint8_t sessionKey[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int32_t ret = cryptoMgr.ProcessSessionKey(sessionKey, 10);
    EXPECT_EQ(ret, DM_OK);
    std::vector<unsigned char> key = cryptoMgr.GetSessionKey();
    EXPECT_EQ(key.size(), static_cast<size_t>(10));
    EXPECT_EQ(key[0], static_cast<unsigned char>(1));
}

/**
 * @tc.name: ClearSessionKey_AfterProcess_Test
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoMgrTest, ClearSessionKey_AfterProcess_Test, testing::ext::TestSize.Level2)
{
    // Cover ClearSessionKey's nullptr-guard branch by clearing when no key was ever set, plus a
    // normal clear after ProcessSessionKey.
    CryptoMgr cryptoMgr;
    cryptoMgr.ClearSessionKey();
    uint8_t sessionKey[10] = {0};
    cryptoMgr.ProcessSessionKey(sessionKey, 10);
    cryptoMgr.ClearSessionKey();
    std::vector<unsigned char> key = cryptoMgr.GetSessionKey();
    EXPECT_EQ(key.empty(), true);
}
} // DistributedHardware
} // OHOS