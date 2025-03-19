/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_crypto.h"

#include <cinttypes>
#include <iostream>

#include "dm_constants.h"
#include "dm_crypto.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t SALT_STRING_LENGTH = 16;
    const std::string SALT_DEFAULT = "salt_defsalt_def";
    const std::string UDID_SAMPLE = "3fde738fb2b8c910023d949166125bc9ed49e9e2fc8f4826d652b2839def2238";
}
void DmCryptoTest::SetUp()
{
}
void DmCryptoTest::TearDown()
{
}
void DmCryptoTest::SetUpTestCase()
{
}
void DmCryptoTest::TearDownTestCase()
{
}

/**
 * @tc.name: GetSecRandom_01
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoTest, GetSecRandom_01, testing::ext::TestSize.Level1)
{
    const size_t len = 8;
    uint8_t buffer[len] = {0};
    int32_t ret = Crypto::GetSecRandom(buffer, len);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: GetSecSalt_01
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoTest, GetSecSalt_01, testing::ext::TestSize.Level1)
{
    std::string salt = Crypto::GetSecSalt();
    EXPECT_EQ(salt.length(), SALT_STRING_LENGTH);
    EXPECT_NE(salt, SALT_DEFAULT);

    std::cout << "Random Salt: " << salt << std::endl;
}

/**
 * @tc.name: GetUdidHash_01
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoTest, GetUdidHash_01, testing::ext::TestSize.Level1)
{
    char udidHash[DEVICE_UUID_LENGTH] = {0};
    EXPECT_EQ(Crypto::GetUdidHash(UDID_SAMPLE, reinterpret_cast<uint8_t *>(udidHash)), DM_OK);

    std::string res(udidHash);
    std::cout << "udidHash sample: " << res << std::endl;
}

/**
 * @tc.name: GetHashWithSalt_01
 * @tc.type: FUNC
 */
HWTEST_F(DmCryptoTest, GetHashWithSalt_01, testing::ext::TestSize.Level1)
{
    std::string text1 = "c9ed49e9e2fc8f4826d652b2839d";
    std::string text2 = "aaadfasdfasdfc9ed49e9e2sadfasdffc8f4826d6asdfasdf52basdf2839d";
    std::string salt1 = Crypto::GetSecSalt();
    std::string salt2 = Crypto::GetSecSalt();

    std::string hash1 = Crypto::GetHashWithSalt(text1, salt1);
    std::string hash2 = Crypto::GetHashWithSalt(text1, salt2);

    EXPECT_STRNE(hash1.c_str(), hash2.c_str());
    std::cout << "hash1: " << hash1 << std::endl;
    std::cout << "hash2: " << hash2 << std::endl;

    std::string hash3 = Crypto::GetHashWithSalt(text1, salt1);
    std::string hash4 = Crypto::GetHashWithSalt(text2, salt1);
    EXPECT_STRNE(hash3.c_str(), hash4.c_str());
    std::cout << "hash1: " << hash3 << std::endl;
    std::cout << "hash2: " << hash4 << std::endl;

    std::string hash5 = Crypto::GetHashWithSalt(text2, salt1);
    EXPECT_STREQ(hash5.c_str(), hash4.c_str());
    std::cout << "hash5: " << hash5 << std::endl;
    std::cout << "hash4: " << hash4 << std::endl;

    std::string hash6 = Crypto::GetHashWithSalt(text1, salt2);
    EXPECT_STREQ(hash2.c_str(), hash6.c_str());

    std::cout << "hash2: " << hash2 << std::endl;
    std::cout << "hash6: " << hash6 << std::endl;
}
} // DistributedHardware
} // OHOS