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
#include "nlohmann/json.hpp"

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

HWTEST_F(CryptoMgrTest, EncryptAndDecryptMessage_001, testing::ext::TestSize.Level0)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_fm88";
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);

    nlohmann::json jsonObj;
    jsonObj[TAG_DEVICE_ID] = "51352xxxx";
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    std::string message = SafetyDump(jsonObj);
    std::string encryptData;
    std::string decryptData;
    ret = cryptoMgr->EncryptMessage(message, encryptData);
    ASSERT_EQ(ret, DM_OK);
    cryptoMgr->DecryptMessage(encryptData, decryptData);
    EXPECT_STREQ(decryptData.c_str(), message.c_str());
}

HWTEST_F(CryptoMgrTest, EncryptAndDecryptMessage_002, testing::ext::TestSize.Level0)
{
    auto cryptoMgr = std::make_shared<CryptoMgr>();
    uint8_t sessionKey[SESSION_KEY_LENGTH] = "sessionKey_fm88";
    auto ret = cryptoMgr->SaveSessionKey(sessionKey, SESSION_KEY_LENGTH);
    ASSERT_EQ(ret, DM_OK);

    nlohmann::json jsonObj;
    jsonObj[TAG_DEVICE_ID] = "51352xxxx";
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    std::string message = SafetyDump(jsonObj);
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

    nlohmann::json jsonObj;
    jsonObj[TAG_DEVICE_ID] = "51352xxxx";
    jsonObj[TAG_CRYPTO_SUPPORT] = true;
    std::string message = SafetyDump(jsonObj);
    std::string encryptData;
    std::string decryptData;
    auto ret = cryptoMgr->EncryptMessage(message, encryptData);
    EXPECT_NE(ret, DM_OK);
    cryptoMgr->DecryptMessage(encryptData, decryptData);
    EXPECT_NE(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
