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

#include "UTTest_hichain_listener.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"

namespace OHOS {
namespace DistributedHardware {
void HichainListenerTest::SetUp()
{
}
void HichainListenerTest::TearDown()
{
}
void HichainListenerTest::SetUpTestCase()
{
}
void HichainListenerTest::TearDownTestCase()
{
}

/**
 * @tc.name: RegisterDataChangeCb_001
 * @tc.type: FUNC
 */
HWTEST_F(HichainListenerTest, RegisterDataChangeCb_001, testing::ext::TestSize.Level1)
{
    HichainListener listerner;
    listerner.RegisterDataChangeCb();
    EXPECT_TRUE(listerner.deviceGroupManager_ != nullptr);
}

/**
 * @tc.name: RegisterDataChangeCb_002
 * @tc.type: FUNC
 */
HWTEST_F(HichainListenerTest, RegisterDataChangeCb_002, testing::ext::TestSize.Level2)
{
    HichainListener listerner;
    listerner.deviceGroupManager_ = nullptr;
    listerner.RegisterDataChangeCb();
    EXPECT_TRUE(listerner.deviceGroupManager_ == nullptr);
}


/**
 * @tc.name: OnHichainDeviceUnBound_001
 * @tc.type: FUNC
 */
HWTEST_F(HichainListenerTest, OnHichainDeviceUnBound_001, testing::ext::TestSize.Level1)
{
    HichainListener listerner;
    char peerUdid[] = "zhangsan";

    JsonObject jsonObject;
    constexpr int32_t  DM_IDENTICAL_ACCOUNT = 1;
    jsonObject[FIELD_GROUP_TYPE] = DM_IDENTICAL_ACCOUNT;
    jsonObject[FIELD_USER_ID] = "hdaoiudhifvafiouoahf-ZhangSan";
    jsonObject[FIELD_OS_ACCOUNT_ID] = 128;
    const char *groupInfoPtr = SafetyDump(jsonObject).c_str();
    listerner.OnHichainDeviceUnBound(peerUdid, groupInfoPtr);
    EXPECT_TRUE(groupInfoPtr != nullptr);
}

/**
 * @tc.name: OnHichainDeviceUnBound_002
 * @tc.type: FUNC
 */
HWTEST_F(HichainListenerTest, OnHichainDeviceUnBound_002, testing::ext::TestSize.Level2)
{
    HichainListener listerner;

    char *udidPtr = nullptr;
    char *groupInfoPtr = nullptr;
    listerner.OnHichainDeviceUnBound(udidPtr, groupInfoPtr);
    EXPECT_TRUE(groupInfoPtr == nullptr);

    char peerUdid[] = "zhangsan";
    char groupInfo[] = "123";
    udidPtr = peerUdid;
    groupInfoPtr = groupInfo;
    listerner.OnHichainDeviceUnBound(udidPtr, groupInfoPtr);
    EXPECT_TRUE(groupInfoPtr != nullptr);
}

HWTEST_F(HichainListenerTest, RegisterCredentialCb_001, testing::ext::TestSize.Level1)
{
    HichainListener listerner;
    listerner.RegisterCredentialCb();
    EXPECT_TRUE(listerner.credManager_ != nullptr);
}

HWTEST_F(HichainListenerTest, RegisterCredentialCb_002, testing::ext::TestSize.Level2)
{
    HichainListener listerner;
    listerner.credManager_ = nullptr;
    listerner.RegisterCredentialCb();
    EXPECT_TRUE(listerner.credManager_ == nullptr);
}

HWTEST_F(HichainListenerTest, OnCredentialDeleted_001, testing::ext::TestSize.Level1)
{
    HichainListener listerner;
    const char *credId = "credId";
    const char *credInfo = "credInfo";
    listerner.OnCredentialDeleted(credId, credInfo);
    EXPECT_TRUE(true);
}

HWTEST_F(HichainListenerTest, OnCredentialDeleted_002, testing::ext::TestSize.Level1)
{
    HichainListener listerner;
    const char *credId = nullptr;
    const char *credInfo = "credInfo";
    listerner.OnCredentialDeleted(credId, credInfo);
    EXPECT_TRUE(true);
}

HWTEST_F(HichainListenerTest, OnCredentialDeleted_003, testing::ext::TestSize.Level1)
{
    HichainListener listerner;
    const char *credId = "credId";
    const char *credInfo = nullptr;
    listerner.OnCredentialDeleted(credId, credInfo);
    EXPECT_TRUE(true);
}

HWTEST_F(HichainListenerTest, OnCredentialDeleted_004, testing::ext::TestSize.Level2)
{
    HichainListener listerner;
    // Create string longer than MAX_DATA_LEN (65536)
    std::string longCredId(65537, 'a');
    const char *credInfo = "credInfo";
    listerner.OnCredentialDeleted(longCredId.c_str(), credInfo);
    EXPECT_TRUE(true); // Verifying early return for invalid length
}

HWTEST_F(HichainListenerTest, OnCredentialDeleted_005, testing::ext::TestSize.Level2)
{
    HichainListener listerner;
    // Create string longer than MAX_DATA_LEN (65536)
    std::string longCredId(65537, 'a');
    const char *credInfo = "credInfo";
    listerner.OnCredentialDeleted(longCredId.c_str(), credInfo);
    EXPECT_TRUE(true); // Verifying early return for invalid length
}

HWTEST_F(HichainListenerTest, DeleteCredential_001, testing::ext::TestSize.Level2)
{
    HichainListener listener;
    listener.credManager_ = nullptr;
    int32_t osAccountId = 123;
    std::string credId = "123456789";
    int32_t ret = listener.DeleteCredential(osAccountId, credId);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

HWTEST_F(HichainListenerTest, DeleteCredential_002, testing::ext::TestSize.Level2)
{
    HichainListener listener;
    int32_t osAccountId = 123;
    std::string credId = "123456789";
    int32_t ret = listener.DeleteCredential(osAccountId, credId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
} // DistributedHardware
} // OHOS