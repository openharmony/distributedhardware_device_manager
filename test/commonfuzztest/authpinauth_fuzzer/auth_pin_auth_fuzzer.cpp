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

#include "auth_pin_auth_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "device_manager_service_listener.h"
#include "dm_auth_state.h"
#include "dm_auth_state_machine.h"
#include "dm_freeze_process.h"

namespace OHOS {
namespace DistributedHardware {
void AuthPinAuthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FreezeProcess freezeProcess;
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->srvExtarInfo = fdp.ConsumeRandomLengthString();
    context->transmitData = fdp.ConsumeRandomLengthString();
    context->requestId = fdp.ConsumeIntegral<int64_t>();
    context->accesser.userId = fdp.ConsumeIntegral<int32_t>();
    
    std::shared_ptr<DmAuthState> auth1 = std::make_shared<AuthSrcPinAuthStartState>();
    std::shared_ptr<DmAuthState> auth2 = std::make_shared<AuthSinkPinAuthDoneState>();
    std::shared_ptr<AuthSrcPinNegotiateStartState> auth3 = std::make_shared<AuthSrcPinNegotiateStartState>();
    std::shared_ptr<AuthSrcPinInputState> auth4 = std::make_shared<AuthSrcPinInputState>();
    std::shared_ptr<DmAuthState> auth5 = std::make_shared<AuthSrcReverseUltrasonicStartState>();
    std::shared_ptr<DmAuthState> auth6 = std::make_shared<AuthSrcForwardUltrasonicStartState>();
    std::shared_ptr<DmAuthState> auth7 = std::make_shared<AuthSinkReverseUltrasonicDoneState>();
    std::shared_ptr<DmAuthState> auth8 = std::make_shared<AuthSinkForwardUltrasonicDoneState>();
    auth1->GetStateType();
    auth2->GetStateType();
    int32_t credType = fdp.ConsumeIntegral<int32_t>();
    auth3->GetCredIdByCredType(context, credType);
    context->pinNegotiateStarted = false;
    auth3->ProcessPinBind(context);
    context->isAppCredentialVerified = true;
    context->accesser.isGenerateLnnCredential = false;
    context->listener = std::make_shared<DeviceManagerServiceListener>();
    auth4->ShowStartAuthDialog(context);
    auth5->GetStateType();
    auth6->GetStateType();
    auth7->GetStateType();
    auth8->GetStateType();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthPinAuthFuzzTest(data, size);
    return 0;
}
