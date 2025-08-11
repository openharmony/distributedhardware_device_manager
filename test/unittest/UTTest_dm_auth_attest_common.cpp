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

#include "gtest/gtest.h"
#include "dm_auth_cert.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "UTTest_dm_auth_attest_common.h"

namespace OHOS {
namespace DistributedHardware {

void DmAuthAttestCommonTest::SetUp()
{
}
void DmAuthAttestCommonTest::TearDown()
{
}
void DmAuthAttestCommonTest::SetUpTestCase()
{
}
void DmAuthAttestCommonTest::TearDownTestCase()
{
}

/**
 * @tc.name: SerializeDmCertChain_001
 * @tc.type: FUNC
 */
HWTEST_F(DmAuthAttestCommonTest, SerializeDmCertChain_001, testing::ext::TestSize.Level1) {
    AuthAttestCommon authAttestCommon;
    DmCertChain *chain = nullptr;
    std::string result = authAttestCommon.SerializeDmCertChain(chain);
    EXPECT_EQ(result, "{}");
}

/**
 * @tc.name: DeserializeDmCertChain_001
 * @tc.type: FUNC
 */
HWTEST_F(DmAuthAttestCommonTest, DeserializeDmCertChain_001, testing::ext::TestSize.Level1) {
    AuthAttestCommon authAttestCommon;
    std::string data = R"({"certCount":2,"cert":["cert1","cert2"]})";
    DmCertChain *outChain = nullptr;

    bool result = authAttestCommon.DeserializeDmCertChain(data, outChain);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: DeserializeDmCertChain_002
 * @tc.type: FUNC
 */
HWTEST_F(DmAuthAttestCommonTest, DeserializeDmCertChain_002, testing::ext::TestSize.Level1) {
    AuthAttestCommon authAttestCommon;
    std::string data = "";
    DmCertChain outChain;

    bool result = authAttestCommon.DeserializeDmCertChain(data, &outChain);

    EXPECT_FALSE(result);
}
}
}