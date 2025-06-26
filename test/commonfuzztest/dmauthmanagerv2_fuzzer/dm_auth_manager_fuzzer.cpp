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

#include "dm_auth_manager_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "auth_manager.h"
#include "device_manager_service_listener.h"
#include "dm_auth_context.h"
#include "dm_auth_state_machine.h"

namespace OHOS {
namespace DistributedHardware {

int32_t g_sessionId = 1;
int32_t g_sessionSide = 0;
int32_t g_result = 1;
int32_t g_authType = 1;
int32_t g_status = 1;
int32_t g_pinCode = 1;
int32_t g_action = 1;
int32_t g_userId = 1;
int32_t g_pageId = 1;
int32_t g_reason = 1;
int32_t g_state = 1;
int64_t g_requestId = 1;
int64_t g_operationCode = 1;
int64_t g_localSessionId = 1;

std::map<std::string, std::string> g_bindParam;
const char* PARAM_KEY_AUTH_TYPE = "AUTH_TYPE";
PeerTargetId g_targetId = {
    .deviceId = "deviceId",
    .brMac = "brMac",
    .bleMac = "bleMac",
    .wifiIp = "wifiIp",
};

// AuthSrcManager fuzz
void DmAuthSrcManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    FuzzedDataProvider fdp(data, size);
    std::string str(reinterpret_cast<const char*>(data), size);
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector,
        listener, hiChainAuthConnector);

    authManager->OnUserOperation(g_action, str);
    authManager->BindTarget(str, g_targetId, g_bindParam, g_sessionId, g_localSessionId);
    authManager->StopAuthenticateDevice(str);
    authManager->ImportAuthCode(str, str);
    authManager->RegisterUiStateCallback(str);
    authManager->UnRegisterUiStateCallback(str);
    authManager->UnAuthenticateDevice(str, str, bindLevel);
    authManager->UnBindDevice(str, str, bindLevel, str);
    authManager->HandleDeviceNotTrust(str);
    authManager->DeleteGroup(str, str);
    authManager->AuthDeviceTransmit(g_requestId, data, size);
    authManager->AuthDeviceSessionKey(g_requestId, data, size);
    authManager->AuthDeviceRequest(g_requestId, g_operationCode, str.c_str());
    authManager->OnDataReceived(g_sessionId, str);
    authManager->OnAuthDeviceDataReceived(g_sessionId, str);
}

// AuthSinkManager fuzz
void DmAuthSinkManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    FuzzedDataProvider fdp(data, size);
    std::string str(reinterpret_cast<const char*>(data), size);
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector,
        listener, hiChainAuthConnector);

    authManager->OnUserOperation(g_action, str);
    authManager->BindTarget(str, g_targetId, g_bindParam, g_sessionId, g_localSessionId);
    authManager->StopAuthenticateDevice(str);
    authManager->ImportAuthCode(str, str);
    authManager->RegisterUiStateCallback(str);
    authManager->UnRegisterUiStateCallback(str);
    authManager->UnAuthenticateDevice(str, str, bindLevel);
    authManager->UnBindDevice(str, str, bindLevel, str);
    authManager->HandleDeviceNotTrust(str);
    authManager->DeleteGroup(str, str);
    authManager->AuthDeviceTransmit(g_requestId, data, size);
    authManager->AuthDeviceSessionKey(g_requestId, data, size);
    authManager->AuthDeviceRequest(g_requestId, g_operationCode, str.c_str());
    authManager->OnDataReceived(g_sessionId, str);
    authManager->OnAuthDeviceDataReceived(g_sessionId, str);
}

void DmAuthManagerV2FuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    FuzzedDataProvider fdp(data, size);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t sessionSide = fdp.ConsumeIntegral<int32_t>();
    int32_t result = fdp.ConsumeIntegral<int32_t>();
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    int32_t errorCode = fdp.ConsumeIntegral<int32_t>();
    uint64_t logicalSessionId = fdp.ConsumeIntegral<uint64_t>();
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    PeerTargetId targetId = {
        .deviceId = fdp.ConsumeRandomLengthString(),
        .brMac = fdp.ConsumeRandomLengthString(),
        .bleMac = fdp.ConsumeRandomLengthString(),
        .wifiIp = fdp.ConsumeRandomLengthString(),
        .wifiPort = fdp.ConsumeIntegral<uint16_t>(),
    };
    std::map<std::string, std::string> bindParam = { { "key1", fdp.ConsumeRandomLengthString() },
        { "key2", fdp.ConsumeRandomLengthString() }, { PARAM_KEY_AUTH_TYPE, "1" } };
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    std::shared_ptr<AuthManager> authSrcManager =
        std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener, hiChainAuthConnector);
    std::shared_ptr<AuthManager> authSinkManager =
        std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener, hiChainAuthConnector);
    authSrcManager->BindTarget(pkgName, targetId, bindParam, sessionId, logicalSessionId);
    authSrcManager->GetAuthParam(pkgName, authType, deviceId, extra);
    authSinkManager->OnSessionOpened(sessionId, sessionSide, result);
    authSrcManager->OnSessionDisable();
    authSrcManager->GetIsCryptoSupport();
    authSinkManager->GetBindTargetParams(pkgName, targetId, bindParam);
    authSinkManager->SetBindTargetParams(targetId);
    authSinkManager->ClearSoftbusSessionCallback();
    authSinkManager->PrepareSoftbusSessionCallback();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmAuthSrcManagerFuzzTest(data, size);
    OHOS::DistributedHardware::DmAuthSinkManagerFuzzTest(data, size);
    OHOS::DistributedHardware::DmAuthManagerV2FuzzTest(data, size);
    return 0;
}