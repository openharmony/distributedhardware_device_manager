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
std::map<std::string, std::string> g_bindParam;
const char* PARAM_KEY_AUTH_TYPE = "AUTH_TYPE";
PeerTargetId g_targetId = {
    .deviceId = "deviceId",
    .brMac = "brMac",
    .bleMac = "bleMac",
    .wifiIp = "wifiIp",
};

void DmAuthSrcManagerFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t action = fdp.ConsumeIntegral<int32_t>();
    int64_t requestId = fdp.ConsumeIntegral<int32_t>();
    int64_t operationCode = fdp.ConsumeIntegral<int32_t>();
    int64_t localSessionId = fdp.ConsumeIntegral<int32_t>();
    uint8_t data = fdp.ConsumeIntegral<uint8_t>();
    uint32_t dataLen = fdp.ConsumeIntegral<uint32_t>();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::string str = fdp.ConsumeRandomLengthString();
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector,
        listener, hiChainAuthConnector);
    authManager->OnUserOperation(action, str);
    authManager->BindTarget(str, g_targetId, g_bindParam, sessionId, localSessionId);
    authManager->StopAuthenticateDevice(str);
    authManager->ImportAuthCode(str, str);
    authManager->RegisterUiStateCallback(str);
    authManager->UnRegisterUiStateCallback(str);
    authManager->UnAuthenticateDevice(str, str, bindLevel);
    authManager->UnBindDevice(str, str, bindLevel, str);
    authManager->HandleDeviceNotTrust(str);
    authManager->DeleteGroup(str, str);
    authManager->AuthDeviceTransmit(requestId, &data, dataLen);
    authManager->AuthDeviceSessionKey(requestId, &data, dataLen);
    char *ret = authManager->AuthDeviceRequest(requestId, operationCode, str.c_str());
    if (ret != nullptr) {
        free(ret);
        ret = nullptr;
    }
    authManager->OnDataReceived(sessionId, str);
    authManager->OnAuthDeviceDataReceived(sessionId, str);
}

void DmAuthSinkManagerFuzzTest(FuzzedDataProvider &fdp)
{
    std::string str = fdp.ConsumeRandomLengthString();
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t action = fdp.ConsumeIntegral<int32_t>();
    int64_t requestId = fdp.ConsumeIntegral<int32_t>();
    int64_t operationCode = fdp.ConsumeIntegral<int32_t>();
    int64_t localSessionId = fdp.ConsumeIntegral<int32_t>();
    uint8_t data = fdp.ConsumeIntegral<uint8_t>();
    uint32_t dataLen = fdp.ConsumeIntegral<uint32_t>();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector,
        listener, hiChainAuthConnector);
    authManager->OnUserOperation(action, str);
    authManager->BindTarget(str, g_targetId, g_bindParam, sessionId, localSessionId);
    authManager->StopAuthenticateDevice(str);
    authManager->ImportAuthCode(str, str);
    authManager->RegisterUiStateCallback(str);
    authManager->UnRegisterUiStateCallback(str);
    authManager->UnAuthenticateDevice(str, str, bindLevel);
    authManager->UnBindDevice(str, str, bindLevel, str);
    authManager->HandleDeviceNotTrust(str);
    authManager->DeleteGroup(str, str);
    authManager->AuthDeviceTransmit(requestId, &data, dataLen);
    authManager->AuthDeviceSessionKey(requestId, &data, dataLen);
    char *ret = authManager->AuthDeviceRequest(requestId, operationCode, str.c_str());
    if (ret != nullptr) {
        free(ret);
        ret = nullptr;
    }
    authManager->OnDataReceived(sessionId, str);
    authManager->OnAuthDeviceDataReceived(sessionId, str);
}

void DmAuthManagerV2FuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
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
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::DmAuthSrcManagerFuzzTest(fdp);
    OHOS::DistributedHardware::DmAuthSinkManagerFuzzTest(fdp);
    OHOS::DistributedHardware::DmAuthManagerV2FuzzTest(fdp);
    return 0;
}