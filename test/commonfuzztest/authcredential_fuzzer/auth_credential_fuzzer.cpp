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

#include "auth_credential_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "device_manager_service_listener.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_state.h"
#include "dm_freeze_process.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint32_t SERVICE = 2;

void DerivativeSessionKeyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->IsProxyBind = true;
    context->accesser.userId = fdp.ConsumeIntegral<int32_t>();
    context->accesser.transmitSessionKeyId = fdp.ConsumeIntegral<int32_t>();
    DmProxyAuthContext dmProxyAuthContext;
    context->subjectProxyOnes.push_back(dmProxyAuthContext);
    std::shared_ptr<AuthSrcCredentialAuthDoneState> authPtr = std::make_shared<AuthSrcCredentialAuthDoneState>();
    context->authMessageProcessor = std::make_shared<DmAuthMessageProcessor>();
    authPtr->DerivativeSessionKey(context);
}

void DerivativeProxySessionKeyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->reUseCreId = fdp.ConsumeRandomLengthString();
    context->IsProxyBind = true;
    context->accesser.isAuthed = true;
    context->accesser.deviceIdHash = fdp.ConsumeRandomLengthString();
    context->accessee.deviceIdHash = fdp.ConsumeRandomLengthString();
    context->accesser.tokenIdHash = fdp.ConsumeRandomLengthString();
    context->accessee.tokenIdHash = fdp.ConsumeRandomLengthString();
    context->accesser.userId = fdp.ConsumeIntegral<int32_t>();
    context->accesser.transmitSkTimeStamp = fdp.ConsumeIntegral<int64_t>();
    context->accesser.transmitSessionKeyId = fdp.ConsumeIntegral<int32_t>();
    context->authMessageProcessor = std::make_shared<DmAuthMessageProcessor>();
    DmProxyAuthContext dmProxyAuthContext;
    context->subjectProxyOnes.push_back(dmProxyAuthContext);
    std::shared_ptr<AuthSrcCredentialAuthDoneState> authSrcPtr = std::make_shared<AuthSrcCredentialAuthDoneState>();
    authSrcPtr->DerivativeProxySessionKey(context);
    std::shared_ptr<AuthSinkCredentialAuthNegotiateState> authSinkPtr =
        std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    authSinkPtr->DerivativeSessionKey(context);
    std::shared_ptr<AuthCredentialAgreeState> authCrePtr = std::make_shared<AuthSrcCredentialExchangeState>();
    JsonObject jsonObject;
    authCrePtr->GenerateTokenIds(context, jsonObject);
}

void AuthCredentialFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FreezeProcess freezeProcess;
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->transmitData = fdp.ConsumeRandomLengthString();
    context->requestId = fdp.ConsumeIntegral<int64_t>();
    context->accesser.userId = fdp.ConsumeIntegral<int32_t>();
    context->accesser.isGenerateLnnCredential = true;
    context->isAppCredentialVerified = false;
    context->accesser.bindLevel = SERVICE;
    std::shared_ptr<DmAuthState> authFirst = std::make_shared<AuthSrcCredentialAuthNegotiateState>();
    std::shared_ptr<AuthSrcCredentialAuthDoneState> authSecond = std::make_shared<AuthSrcCredentialAuthDoneState>();
    std::shared_ptr<DmAuthState> authThird = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
    std::shared_ptr<DmAuthState> authForth = std::make_shared<AuthSrcCredentialExchangeState>();
    std::shared_ptr<DmAuthState> authFifth = std::make_shared<AuthSinkCredentialExchangeState>();
    std::shared_ptr<DmAuthState> authSixth = std::make_shared<AuthSrcCredentialAuthStartState>();

    authFirst->Action(context);
    authSecond->Action(context);
    context->isAppCredentialVerified = true;
    authSecond->Action(context);
    context->accesser.isGenerateLnnCredential = false;
    authFirst->GetStateType();
    authSecond->GetStateType();
    authThird->GetStateType();
    authForth->GetStateType();
    authFifth->GetStateType();
    authSixth->GetStateType();

    DerivativeSessionKeyFuzzTest(data, size);
    DerivativeProxySessionKeyFuzzTest(data, size);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthCredentialFuzzTest(data, size);
    return 0;
}