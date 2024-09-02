/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "UTTest_dm_auth_manager_third.h"

#include "nlohmann/json.hpp"

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_auth_manager.h"
#include "auth_message_processor.h"
#include "device_manager_service_listener.h"
#include "softbus_error_code.h"

static std::string g_createSimpleMessageReturnDataStr = "{}";
static bool g_isIdenticalAccountReturnBoolValue = false;

namespace OHOS {
namespace DistributedHardware {
std::string AuthMessageProcessor::CreateSimpleMessage(int32_t msgType)
{
    return g_createSimpleMessageReturnDataStr;
}

bool DmAuthManager::IsIdenticalAccount()
{
    return g_isIdenticalAccountReturnBoolValue;
}

void DmAuthManagerTest::SetUp()
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    authManager_ = std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector, listener, hiChainAuthConnector);

    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->softbusConnector_ = std::make_shared<SoftbusConnector>();
    authManager_->softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(
        std::shared_ptr<ISoftbusSessionCallback>(authManager_));
    authManager_->timer_ = std::make_shared<DmTimer>();
}
void DmAuthManagerTest::TearDown()
{
    g_createSimpleMessageReturnDataStr = "{}";
    g_isIdenticalAccountReturnBoolValue = false;
}
void DmAuthManagerTest::SetUpTestCase() {}
void DmAuthManagerTest::TearDownTestCase() {}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiate001, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    int32_t sessionId = 0;
    authManager_->remoteDeviceId_ = "ProcRespNegotiate001";
    authManager_->ProcRespNegotiate(sessionId);

    jsonObject[TAG_CRYPTO_SUPPORT] = false;
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_CRE;
    authManager_->ProcRespNegotiate(sessionId);

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_SUPPORT] = true;
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->importAuthCode_ = "";
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = false;
    authManager_->ProcRespNegotiate(sessionId);

    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->ProcRespNegotiate(sessionId);

    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_NAME] = "test";
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->ProcRespNegotiate(sessionId);
    EXPECT_FALSE(authManager_->authResponseContext_->isOnline);
}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiate002, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    int32_t sessionId = 0;
    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_SUPPORT] = true;
    jsonObject[TAG_CRYPTO_NAME] = "test";
    jsonObject[TAG_CRYPTO_VERSION] = "test";
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    g_isIdenticalAccountReturnBoolValue = false;
    authManager_->remoteDeviceId_ = "ProcRespNegotiate002";
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->authResponseState_->context_->cryptoName = jsonObject[TAG_CRYPTO_NAME];
    authManager_->ProcRespNegotiate(sessionId);
    EXPECT_FALSE(authManager_->authResponseContext_->isOnline);

    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_SUPPORT] = true;
    jsonObject[TAG_CRYPTO_NAME] = "test";
    jsonObject[TAG_CRYPTO_VERSION] = "test";
    g_createSimpleMessageReturnDataStr = jsonObject.dump();
    g_isIdenticalAccountReturnBoolValue = false;
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->authResponseState_->context_->cryptoName = jsonObject[TAG_CRYPTO_NAME];
    authManager_->authResponseState_->context_->cryptoVer = jsonObject[TAG_CRYPTO_VERSION];
    authManager_->ProcRespNegotiate(sessionId);
    EXPECT_FALSE(authManager_->authResponseContext_->isOnline);
}
} // namespace DistributedHardware
} // namespace OHOS
