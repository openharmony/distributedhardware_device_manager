/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <string>
#include <fuzzer/FuzzedDataProvider.h>
#include "device_manager_service_listener.h"
#include "auth_message_processor.h"
#include "dm_auth_manager.h"
#include "dm_auth_manager_fuzzer.h"
#include "dm_timer.h"
#include "hichain_connector.h"

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<SoftbusConnector> g_softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<IDeviceManagerServiceListener> g_listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainConnector> g_hiChainConnector = std::make_shared<HiChainConnector>();
std::shared_ptr<HiChainAuthConnector> g_hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
std::shared_ptr<AuthRequestState> g_authRequestState = std::make_shared<AuthRequestNetworkState>();
std::shared_ptr<AuthResponseState> g_authResponseState = std::make_shared<AuthResponseInitState>();
std::shared_ptr<DmAuthManager> g_authManager =
    std::make_shared<DmAuthManager>(g_softbusConnector, g_hiChainConnector, g_listener, g_hiChainAuthConnector);

int32_t g_sessionId = 1;
int32_t g_sessionSide = 0;
int32_t g_result = 1;
int32_t g_authType = 1;
int32_t g_status = 1;
std::string g_pinCode = "111111";
int32_t g_action = 1;
int32_t g_userId = 1;
int32_t g_pageId = 1;
int32_t g_reason = 1;
int32_t g_state = 1;
int64_t g_requestId = 1;
int64_t g_localSessionId = 1;

std::map<std::string, std::string> g_bindParam;

PeerTargetId g_targetId = {
    .deviceId = "deviceId",
    .brMac = "brMac",
    .bleMac = "bleMac",
    .wifiIp = "wifiIp",
};

void DmAuthManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string str(reinterpret_cast<const char*>(data), size);
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    g_authManager->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(g_authManager);
    g_authManager->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    g_authManager->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    g_authManager->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    g_authManager->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    g_authManager->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    g_authManager->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    g_authManager->softbusConnector_ = std::make_shared<SoftbusConnector>();
    g_authManager->softbusConnector_->GetSoftbusSession()->
        RegisterSessionCallback(std::shared_ptr<ISoftbusSessionCallback>(g_authManager));
    g_authManager->timer_ = std::make_shared<DmTimer>();

    g_authManager->InitAuthState(str, g_authType, str, str);
    g_authManager->OnSessionOpened(g_sessionId, g_sessionSide, g_result);
    g_authManager->AuthenticateDevice(str, g_authType, str, str);
    g_authManager->ImportAuthCode(str, str);
    g_authManager->BindTarget(str, g_targetId, g_bindParam, g_sessionId, g_localSessionId);
    g_authManager->ShowConfigDialog();
    g_authManager->ShowAuthInfoDialog();
    g_authManager->ShowStartAuthDialog();
    g_authManager->OnDataReceived(g_sessionId, str);
    g_authManager->OnGroupCreated(g_requestId, str);
    g_authManager->OnMemberJoin(g_requestId, g_status);
    g_authManager->StartNegotiate(g_sessionId);
    g_authManager->RespNegotiate(g_sessionId);
    g_authManager->SendAuthRequest(g_sessionId);
    g_authManager->SetAuthRequestState(g_authRequestState);
    g_authManager->SetAuthResponseState(g_authResponseState);
    g_authManager->StartAuthProcess(g_action);
    g_authManager->StartRespAuthProcess();
    g_authManager->CreateGroup();
    g_authManager->ProcessPincode(g_pinCode);
    g_authManager->SetPageId(g_pageId);
    g_authManager->SetReasonAndFinish(g_reason, g_state);
    g_authManager->IsIdenticalAccount();
    g_authManager->OnSessionClosed(g_sessionId);
    g_authManager->OnUserOperation(g_action, str);
    g_authManager->GetConnectAddr(str);
    g_authManager->HandleAuthenticateTimeout(str);
    g_authManager->RegisterUiStateCallback(str);
    g_authManager->UnRegisterUiStateCallback(str);
    g_authManager->UnAuthenticateDevice(str, str, bindLevel);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmAuthManagerFuzzTest(data, size);

    return 0;
}