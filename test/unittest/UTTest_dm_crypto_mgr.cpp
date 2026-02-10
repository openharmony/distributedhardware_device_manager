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
} // DistributedHardware
} // OHOS