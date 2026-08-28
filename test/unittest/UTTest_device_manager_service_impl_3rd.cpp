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

#include "UTTest_device_manager_service_impl_3rd.h"

#include "dm_constants_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_auth_message_3rd.h"
#include "idevice_manager_service_listener_3rd.h"
#include "json_object.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

namespace OHOS {
namespace DistributedHardware {

namespace {
constexpr int32_t TEST_SESSION_ID = 12345;
constexpr uint64_t TEST_LOGICAL_SESSION_ID = 123456789ULL;
constexpr int32_t TEST_REPLY = 0;
constexpr int32_t TEST_REASON = 3060;
constexpr int32_t TEST_ERROR_REPLY = ERR_DM_BIND_INPUT_PARA_INVALID;
constexpr uint32_t TEST_TOKEN_ID = 123456;
const std::string TEST_PROCESS_NAME = "com.ohos.test";
const std::string TEST_BUSINESS_NAME = "com.ohos.business";
}

class MockDeviceManagerServiceListener3rd : public IDeviceManagerServiceListener3rd {
public:
    ~MockDeviceManagerServiceListener3rd() override = default;
    void OnAuthResult(const ProcessInfo3rd &processInfo3rd, int32_t result, int32_t status,
        const std::string &authContent) override
    {
        (void)processInfo3rd;
        (void)result;
        (void)status;
        (void)authContent;
    }

    void OnAuthResult(const ProcessInfo3rd &processInfo3rd, int32_t result, int32_t status,
        std::vector<TrustDeviceInfo3rd> &deviceInfos, const std::string &authContent) override
    {
        (void)processInfo3rd;
        (void)result;
        (void)status;
        (void)deviceInfos;
        (void)authContent;
    }
};

void DeviceManagerServiceImpl3rdTest::SetUpTestCase()
{
}

void DeviceManagerServiceImpl3rdTest::TearDownTestCase()
{
}

void DeviceManagerServiceImpl3rdTest::SetUp()
{
    serviceImpl_ = std::make_shared<DeviceManagerServiceImpl3rd>();
}

void DeviceManagerServiceImpl3rdTest::TearDown()
{
}

HWTEST_F(DeviceManagerServiceImpl3rdTest, GetCredAuthMgrByMessage_001, testing::ext::TestSize.Level1)
{
    auto listener = std::make_shared<MockDeviceManagerServiceListener3rd>();
    serviceImpl_->Initialize(listener);
    
    JsonObject jsonObject;
    jsonObject[TAG_PEER_BUSINESS_NAME] = TEST_BUSINESS_NAME;
    jsonObject[TAG_PEER_PROCESS_NAME] = TEST_PROCESS_NAME;
    
    auto result = serviceImpl_->GetCredAuthMgrByMessage(
        static_cast<int32_t>(DmCredMessageType::CRED_RESP_NEGOTIATE),
        TEST_LOGICAL_SESSION_ID,
        TEST_SESSION_ID,
        jsonObject);
    
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(DeviceManagerServiceImpl3rdTest, GetCredAuthMgrByMessage_002, testing::ext::TestSize.Level1)
{
    auto listener = std::make_shared<MockDeviceManagerServiceListener3rd>();
    serviceImpl_->Initialize(listener);
    
    JsonObject jsonObject;
    jsonObject[TAG_PEER_BUSINESS_NAME] = TEST_BUSINESS_NAME;
    jsonObject[TAG_PEER_PROCESS_NAME] = TEST_PROCESS_NAME;
    
    {
        std::lock_guard<ffrt::mutex> lock(serviceImpl_->logicalSessionId2TokenIdMapMtx_);
        serviceImpl_->logicalSessionId2TokenIdMap_[TEST_LOGICAL_SESSION_ID] = TEST_TOKEN_ID;
    }
    
    auto result = serviceImpl_->GetCredAuthMgrByMessage(
        static_cast<int32_t>(DmCredMessageType::CRED_RESP_NEGOTIATE),
        TEST_LOGICAL_SESSION_ID,
        TEST_SESSION_ID,
        jsonObject);
    
    EXPECT_EQ(result, nullptr);
}
} // namespace DistributedHardware
} // namespace OHOS