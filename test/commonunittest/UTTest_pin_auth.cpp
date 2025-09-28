/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "UTTest_pin_auth.h"

#include <unistd.h>
#include <memory>

#include "dm_common_util.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "json_object.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void PinAuthTest::SetUp()
{
}

void PinAuthTest::TearDown()
{
}

void PinAuthTest::SetUpTestCase()
{
}

void PinAuthTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
std::shared_ptr<DmAuthManager> authManager =
    std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector, listener, hiChainAuthConnector);
/**
 * @tc.name: DmAuthManager::ShowAuthInfo_001
 * @tc.desc: Call unauthenticateddevice to check whether the return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PinAuthTest, ShowAuthInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<PinAuth> pinAuth = std::make_shared<PinAuth>();
    std::string authToken = "123456";
    int32_t ret = pinAuth->ShowAuthInfo(authToken, authManager);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::ShowAuthInfo_002
 * @tc.desc: Call unauthenticateddevice to check whether the return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PinAuthTest, ShowAuthInfo_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<PinAuth> pinAuth = std::make_shared<PinAuth>();
    JsonObject jsonObject;
    jsonObject[PIN_TOKEN] = 123456;
    std::string authToken = jsonObject.Dump();
    int32_t ret = pinAuth->ShowAuthInfo(authToken, authManager);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::ShowAuthInfo_003
 * @tc.desc: Call unauthenticateddevice to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PinAuthTest, ShowAuthInfo_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<PinAuth> pinAuth = std::make_shared<PinAuth>();
    JsonObject jsonObject;
    jsonObject[PIN_CODE_KEY] = 123456;
    std::string authToken = jsonObject.Dump();
    int32_t ret = pinAuth->ShowAuthInfo(authToken, nullptr);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::ShowAuthInfo_004
 * @tc.desc: Call unauthenticateddevice to check whether the return value is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PinAuthTest, ShowAuthInfo_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<PinAuth> pinAuth = std::make_shared<PinAuth>();
    JsonObject jsonObject;
    jsonObject[PIN_CODE_KEY] = 123456;
    std::string authToken = jsonObject.Dump();
    int32_t ret = pinAuth->ShowAuthInfo(authToken, authManager);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::StartAuth_001
 * @tc.desc: Call unauthenticateddevice to check whether the return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PinAuthTest, StartAuth_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<PinAuth> pinAuth = std::make_shared<PinAuth>();
    std::string authToken = "";
    int32_t ret = pinAuth->StartAuth(authToken, nullptr);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DmAuthManager::StartAuth_002
 * @tc.desc: Call unauthenticateddevice to check whether the return value is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PinAuthTest, StartAuth_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<PinAuth> pinAuth = std::make_shared<PinAuth>();
    std::string authToken = "";
    int32_t ret = pinAuth->StartAuth(authToken, authManager);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}
}
} // namespace DistributedHardware
} // namespace OHOS
