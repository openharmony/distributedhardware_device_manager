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
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>

#include "auth_manager_fuzzer.h"

#include "auth_manager.h"
#include "deviceprofile_connector.h"
#include "device_manager_service_listener.h"
#include "dm_anonymous.h"
#include "dm_auth_context.h"
#include "dm_auth_manager_base.h"
#include "auth_manager.h"
#include "dm_auth_state_machine.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"


namespace OHOS {
namespace DistributedHardware {
namespace {
    std::shared_ptr<DmAuthMessageProcessor> dmAuthMessageProcessor_ = std::make_shared<DmAuthMessageProcessor>();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSrcManager>(softbusConnector,
        hiChainConnector, listener, hiChainAuthConnector);
    std::shared_ptr<DmAuthContext> context_ = authManager->GetAuthContext();
    std::shared_ptr<AuthManager> authSinkManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector,
        listener, hiChainAuthConnector);
}

void ConvertSrcVersionFuzzTest(FuzzedDataProvider &fdp)
{
    std::string version = fdp.ConsumeRandomLengthString();
    std::string edition = fdp.ConsumeRandomLengthString();
    CleanNotifyCallback cleanNotifyCallback;
    authManager->RegisterCleanNotifyCallback(cleanNotifyCallback);
    authManager->SetAuthContext(context_);
    authManager->GetAuthContext();
    authManager->GeneratePincode();
    authManager->ConvertSrcVersion(version, edition);
}

void RegisterUiStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    authManager->RegisterUiStateCallback(pkgName);
}

void UnRegisterUiStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    authManager->UnRegisterUiStateCallback(pkgName);
}

void UnAuthenticateDeviceFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string udid = fdp.ConsumeRandomLengthString();
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    authManager->UnAuthenticateDevice(pkgName, udid, bindLevel);
}

void ImportAuthCodeFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string authCode = fdp.ConsumeRandomLengthString();
    authManager->ImportAuthCode(pkgName, authCode);
}

void UnBindDeviceFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    std::string udid = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    authManager->UnBindDevice(pkgName, udid, bindLevel, extra);
}

void StopAuthenticateDeviceFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    authManager->StopAuthenticateDevice(pkgName);
    authManager->OnScreenLocked();
}

void HandleDeviceNotTrustFuzzTest(FuzzedDataProvider &fdp)
{
    std::string udid = fdp.ConsumeRandomLengthString();
    authManager->HandleDeviceNotTrust(udid);
}

void RegisterAuthenticationTypeFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t authenticationType = fdp.ConsumeIntegral<int32_t>();
    authManager->RegisterAuthenticationType(authenticationType);
    authManager->GetReason();
}

void AuthDeviceSessionKeyFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    uint8_t sessionKey = fdp.ConsumeIntegral<uint8_t>();
    uint32_t sessionKeyLen = fdp.ConsumeIntegral<uint32_t>();
    authManager->AuthDeviceSessionKey(requestId, &sessionKey, sessionKeyLen);
}

void AuthDeviceRequestFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    int operationCode = fdp.ConsumeIntegral<int>();
    const char* reqParams = fdp.ConsumeRandomLengthString().c_str();
    char *ret = authManager->AuthDeviceRequest(requestId, operationCode, reqParams);
    if (ret != nullptr) {
        free(ret);
        ret = nullptr;
    }
}

void SetAuthTypeFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    authManager->SetAuthType(authType);
}

void IsAuthTypeSupportedFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    authManager->IsAuthTypeSupported(authType);
}

void CheckAuthParamVaildFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    authManager->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    authType = 1;
    authManager->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
}

void ParseHmlInfoInJsonObjectFuzzTest(FuzzedDataProvider &fdp)
{
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_CONN_SESSIONTYPE] = fdp.ConsumeRandomLengthString();
    jsonObject[PARAM_KEY_HML_ENABLE_160M] = fdp.ConsumeBool();
    jsonObject[PARAM_KEY_HML_ACTIONID] = fdp.ConsumeRandomLengthString();
    authManager->ParseHmlInfoInJsonObject(jsonObject);
}

void ParseJsonObjectFuzzTest(FuzzedDataProvider &fdp)
{
    JsonObject jsonObject;
    jsonObject[APP_OPERATION_KEY] = fdp.ConsumeRandomLengthString();
    jsonObject[CUSTOM_DESCRIPTION_KEY] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_APP_THUMBNAIL2] = fdp.ConsumeRandomLengthString();
    jsonObject[PARAM_CLOSE_SESSION_DELAY_SECONDS] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_PEER_PKG_NAME] = fdp.ConsumeRandomLengthString();
    jsonObject[DM_BUSINESS_ID] = fdp.ConsumeRandomLengthString();
    authManager->ParseJsonObject(jsonObject);
}

void GetBindLevelFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t bindLevel = fdp.ConsumeIntegral<int32_t>();
    authManager->GetBindLevel(bindLevel);
}

void GetAuthParamFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    authManager->GetAuthParam(pkgName, authType, deviceId, extra);
    JsonObject jsonObject;
    jsonObject[TAG_BIND_LEVEL] = fdp.ConsumeIntegral<int32_t>();
    extra = jsonObject.Dump();
    authManager->GetAuthParam(pkgName, authType, deviceId, extra);
}

void InitAuthStateFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    authManager->InitAuthState(pkgName, authType, deviceId, extra);
}

void AuthenticateDeviceFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t authType = fdp.ConsumeIntegral<int32_t>();
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    authManager->AuthenticateDevice(pkgName, authType, deviceId, extra);
}

void AuthDeviceErrorFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    int32_t errorCode = fdp.ConsumeIntegral<int32_t>();
    authManager->AuthDeviceError(requestId, errorCode);
    authSinkManager->AuthDeviceError(requestId, errorCode);
    requestId = context_->requestId;
    authManager->AuthDeviceError(requestId, errorCode);
}

void AuthDeviceFinishFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    authManager->AuthDeviceFinish(requestId);
    authSinkManager->AuthDeviceFinish(requestId);
}

void GetPinCodeFuzzTest(FuzzedDataProvider &fdp)
{
    std::string code = fdp.ConsumeRandomLengthString();
    authManager->GetPinCode(code);
}

void GetAuthCodeAndPkgNameFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string authCode = fdp.ConsumeRandomLengthString();
    authManager->GetAuthCodeAndPkgName(pkgName, authCode);
}

void IsAuthCodeReadyFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    authManager->IsAuthCodeReady(pkgName);
}

void ParseUltrasonicSideFuzzTest(FuzzedDataProvider &fdp)
{
    JsonObject jsonObject;
    jsonObject[TAG_ULTRASONIC_SIDE] = fdp.ConsumeRandomLengthString();
    authManager->ParseUltrasonicSide(jsonObject);
}

void BindTargetFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int sessionId = fdp.ConsumeIntegral<int>();
    uint64_t logicalSessionId = fdp.ConsumeIntegral<uint64_t>();
    authManager->BindTarget(pkgName, targetId, bindParam, sessionId, logicalSessionId);
    bindParam[PARAM_KEY_AUTH_TYPE] = "1";
    authManager->BindTarget(pkgName, targetId, bindParam, sessionId, logicalSessionId);
    authManager->GetBindTargetParams(pkgName, targetId, bindParam);
    authManager->SetBindTargetParams(targetId);
}

void OnSessionClosedFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    authSinkManager->OnSessionClosed(sessionId);
}

void OnDataReceivedFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string message = fdp.ConsumeRandomLengthString();
    authSinkManager->OnDataReceived(sessionId, message);
    authSinkManager->GetIsCryptoSupport();
}

void GetRemoteDeviceIdFuzzTest(FuzzedDataProvider &fdp)
{
    std::string deviceId = fdp.ConsumeRandomLengthString();
    authManager->GetRemoteDeviceId(deviceId);
}

void OnUserOperationFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t action = fdp.ConsumeIntegral<int32_t>();
    std::string params = fdp.ConsumeRandomLengthString();
    authSinkManager->OnUserOperation(action, params);
    action = USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT;
    authSinkManager->OnUserOperation(action, params);
    action = USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY;
    authSinkManager->OnUserOperation(action, params);
}

void OnSessionOpenedFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t sessionSide = fdp.ConsumeIntegral<int32_t>();
    int32_t result = fdp.ConsumeIntegral<int32_t>();
    authManager->OnSessionOpened(sessionId, sessionSide, result);
    authSinkManager->OnSessionOpened(sessionId, sessionSide, result);
}

void OnSrcSessionClosedFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    authManager->OnSessionClosed(sessionId);
}

void OnSrcDataReceivedFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string message = fdp.ConsumeRandomLengthString();
    authManager->OnDataReceived(sessionId, message);
    authManager->OnSessionDisable();
    authManager->GetIsCryptoSupport();
}

void OnSrcUserOperationFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t action = fdp.ConsumeIntegral<int32_t>();
    std::string params = fdp.ConsumeRandomLengthString();
    authManager->OnUserOperation(action, params);
    action = USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT;
    authManager->OnUserOperation(action, params);
    action = USER_OPERATION_TYPE_DONE_PINCODE_INPUT;
    authManager->OnUserOperation(action, params);
}

void AuthDeviceTransmitFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    uint8_t data = fdp.ConsumeIntegral<uint8_t>();
    uint32_t dataLen = fdp.ConsumeIntegral<uint32_t>();
    authManager->AuthDeviceTransmit(requestId, &data, dataLen);
}

void AuthSinkDeviceTransmitFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    uint8_t data = fdp.ConsumeIntegral<uint8_t>();
    uint32_t dataLen = fdp.ConsumeIntegral<uint32_t>();
    authSinkManager->AuthDeviceTransmit(requestId, &data, dataLen);
}

void AuthSinkDeviceSessionKeyFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    uint8_t sessionKey = fdp.ConsumeIntegral<uint8_t>();
    uint32_t sessionKeyLen = fdp.ConsumeIntegral<uint32_t>();
    authSinkManager->AuthDeviceSessionKey(requestId, &sessionKey, sessionKeyLen);
}

void AuthSinkDeviceRequestFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    int operationCode = fdp.ConsumeIntegral<int>();
    const char* reqParams = fdp.ConsumeRandomLengthString().c_str();
    char *ret = authSinkManager->AuthDeviceRequest(requestId, operationCode, reqParams);
    if (ret != nullptr) {
        free(ret);
        ret = nullptr;
    }
}

void HandleBusinessEventsFuzzTest(FuzzedDataProvider &fdp)
{
    std::string businessId = fdp.ConsumeRandomLengthString();
    int action = fdp.ConsumeIntegral<int>();
    authManager->HandleBusinessEvents(businessId, action);
}

void ParseProxyJsonObjectFuzzTest(FuzzedDataProvider &fdp)
{
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_IS_PROXY_BIND] = fdp.ConsumeRandomLengthString();
    jsonObject[PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT] = fdp.ConsumeRandomLengthString();
    jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = fdp.ConsumeRandomLengthString();
    authManager->ParseProxyJsonObject(jsonObject);
}

void GetBindLevelByBundleNameFuzzTest(FuzzedDataProvider &fdp)
{
    std::string bundleName = fdp.ConsumeRandomLengthString();
    int32_t requestId = fdp.ConsumeIntegral<int32_t>();
    int32_t bindLevel = 0;
    authManager->GetBindLevelByBundleName(bundleName, requestId, bindLevel);
}

void AuthManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    ConvertSrcVersionFuzzTest(fdp);
    RegisterUiStateCallbackFuzzTest(fdp);
    UnRegisterUiStateCallbackFuzzTest(fdp);
    UnAuthenticateDeviceFuzzTest(fdp);
    ImportAuthCodeFuzzTest(fdp);
    UnBindDeviceFuzzTest(fdp);
    StopAuthenticateDeviceFuzzTest(fdp);
    HandleDeviceNotTrustFuzzTest(fdp);
    RegisterAuthenticationTypeFuzzTest(fdp);
    AuthDeviceSessionKeyFuzzTest(fdp);
    AuthDeviceRequestFuzzTest(fdp);
    SetAuthTypeFuzzTest(fdp);
    IsAuthTypeSupportedFuzzTest(fdp);
    ParseHmlInfoInJsonObjectFuzzTest(fdp);
    CheckAuthParamVaildFuzzTest(fdp);
    ParseJsonObjectFuzzTest(fdp);
    GetBindLevelFuzzTest(fdp);
    GetAuthParamFuzzTest(fdp);
    InitAuthStateFuzzTest(fdp);
    AuthDeviceErrorFuzzTest(fdp);
    AuthDeviceFinishFuzzTest(fdp);
    GetAuthCodeAndPkgNameFuzzTest(fdp);
    GetPinCodeFuzzTest(fdp);
    AuthenticateDeviceFuzzTest(fdp);
    BindTargetFuzzTest(fdp);
    ParseUltrasonicSideFuzzTest(fdp);
    IsAuthCodeReadyFuzzTest(fdp);
    AuthSinkDeviceSessionKeyFuzzTest(fdp);
    OnSessionClosedFuzzTest(fdp);
    OnDataReceivedFuzzTest(fdp);
    GetRemoteDeviceIdFuzzTest(fdp);
    OnUserOperationFuzzTest(fdp);
    OnSessionOpenedFuzzTest(fdp);
    OnSrcSessionClosedFuzzTest(fdp);
    OnSrcUserOperationFuzzTest(fdp);
    AuthDeviceTransmitFuzzTest(fdp);
    AuthSinkDeviceTransmitFuzzTest(fdp);
    AuthSinkDeviceSessionKeyFuzzTest(fdp);
    AuthSinkDeviceRequestFuzzTest(fdp);
    HandleBusinessEventsFuzzTest(fdp);
    ParseProxyJsonObjectFuzzTest(fdp);
    GetBindLevelByBundleNameFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthManagerFuzzTest(data, size);
    return 0;
}
