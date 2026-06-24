/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_crypto_mgr.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
void CryptoMgrTest::SetUp()
{}

void CryptoMgrTest::TearDown()
{}

void CryptoMgrTest::SetUpTestCase()
{}

void CryptoMgrTest::TearDownTestCase()
{}

HWTEST_F(CryptoMgrTest, EncryptAndDecryptMessage_001, testing::ext::TestSize.Level1)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_fm88";
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);

    JsonObject jsonObj;
    jsonObj[TAG_DEVICE_ID] = "51352xxxx";
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    std::string message = jsonObj.Dump();
    std::string encryptData;
    std::string decryptData;
    ret = cryptoMgr->EncryptMessage(message, encryptData);
    ASSERT_EQ(ret, DM_OK);
    cryptoMgr->DecryptMessage(encryptData, decryptData);
    EXPECT_STREQ(decryptData.c_str(), message.c_str());
}

HWTEST_F(CryptoMgrTest, EncryptAndDecryptMessage_002, testing::ext::TestSize.Level1)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_fm88";
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);

    JsonObject jsonObj;
    jsonObj[TAG_DEVICE_ID] = "51352xxxx";
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    std::string message = jsonObj.Dump();
    std::string encryptData1;
    std::string encryptData2;
    std::string decryptData;
    ret = cryptoMgr->EncryptMessage(message, encryptData1);
    ASSERT_EQ(ret, DM_OK);
    ret = cryptoMgr->EncryptMessage(message, encryptData2);
    ASSERT_EQ(ret, DM_OK);
    cryptoMgr->DecryptMessage(encryptData1, decryptData);
    EXPECT_STREQ(decryptData.c_str(), message.c_str());
    cryptoMgr->DecryptMessage(encryptData2, decryptData);
    EXPECT_STREQ(decryptData.c_str(), message.c_str());
}

HWTEST_F(CryptoMgrTest, EncryptAndDecryptMessage_003, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_fm88";
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);

    std::string emptyMessage;
    std::string encryptData;
    std::string decryptData;
    ret = cryptoMgr->EncryptMessage(emptyMessage, encryptData);
    EXPECT_NE(ret, DM_OK);
    cryptoMgr->DecryptMessage(encryptData, decryptData);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(CryptoMgrTest, EncryptAndDecryptMessage_004, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    const size_t sessionKeyLen = 15;
    uint8_t sessionKey[sessionKeyLen] = {'\0'};
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, sessionKeyLen);
    ASSERT_EQ(ret, DM_OK);

    std::string emptyMessage;
    std::string encryptData;
    std::string decryptData;
    ret = cryptoMgr->EncryptMessage(emptyMessage, encryptData);
    EXPECT_NE(ret, DM_OK);
    cryptoMgr->DecryptMessage(encryptData, decryptData);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(CryptoMgrTest, EncryptAndDecryptMessage_005, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();

    JsonObject jsonObj;
    jsonObj[TAG_DEVICE_ID] = "51352xxxx";
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    std::string message = jsonObj.Dump();
    std::string encryptData;
    std::string decryptData;
    auto ret = cryptoMgr->EncryptMessage(message, encryptData);
    EXPECT_NE(ret, DM_OK);
    cryptoMgr->DecryptMessage(encryptData, decryptData);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(CryptoMgrTest, SaveSessionKey_001, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    // keyLen exceeds MAX_SESSION_KEY_LENGTH (512), hits the early-return branch.
    const size_t overLen = 513;
    uint8_t sessionKey[overLen] = {'\0'};
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, overLen);
    EXPECT_EQ(ret, ERR_DM_SAVE_SESSION_KEY_FAILED);
}

HWTEST_F(CryptoMgrTest, GetSessionKey_001, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    // Without saving a session key first, GetSessionKey derefs a nullptr-backed range;
    // cover the getter path after a valid SaveSessionKey so the returned vector is non-empty.
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_km99";
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);
    std::vector<unsigned char> key = cryptoMgr->GetSessionKey();
    EXPECT_EQ(key.size(), static_cast<size_t>(SESSION_KEY_LENGTH));
}

HWTEST_F(CryptoMgrTest, ClearSessionKey_001, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_lm11";
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);
    // Cover ClearSessionKey's null-guard: calling twice should be safe (key freed on first call).
    cryptoMgr->ClearSessionKey();
    cryptoMgr->ClearSessionKey();
    std::vector<unsigned char> key = cryptoMgr->GetSessionKey();
    EXPECT_EQ(key.empty(), true);
}

HWTEST_F(CryptoMgrTest, ProcessSessionKey_EncryptDecrypt_001, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    // ProcessSessionKey (copy-based key set) is distinct from SaveSessionKey; exercise the full
    // encrypt/decrypt round-trip through the ProcessSessionKey path.
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_pm22";
    auto ret = cryptoMgr->ProcessSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);

    JsonObject jsonObj;
    jsonObj[TAG_DEVICE_ID] = "51352yyyy";
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    std::string message = jsonObj.Dump();
    std::string encryptData;
    std::string decryptData;
    ret = cryptoMgr->EncryptMessage(message, encryptData);
    ASSERT_EQ(ret, DM_OK);
    ret = cryptoMgr->DecryptMessage(encryptData, decryptData);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_STREQ(decryptData.c_str(), message.c_str());
}

HWTEST_F(CryptoMgrTest, DecryptMessage_InvalidHex_001, testing::ext::TestSize.Level2)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_qm33";
    auto ret = cryptoMgr->ProcessSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);
    // Odd-length / too-short hex string falls into the inputMsgBytesLen < OVERHEAD_LEN branch.
    std::string invalidHex = "ab";
    std::string decryptData;
    ret = cryptoMgr->DecryptMessage(invalidHex, decryptData);
    EXPECT_NE(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
