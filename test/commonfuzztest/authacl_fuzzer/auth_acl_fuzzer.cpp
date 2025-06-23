/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "auth_acl_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "device_manager_service_listener.h"
#include "dm_auth_state.h"
#include "dm_freeze_process.h"

namespace OHOS {
namespace DistributedHardware {
void AuthAclFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string result = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DmAuthState> authSink = std::make_shared<AuthSinkDataSyncState>();
    std::shared_ptr<DmAuthState> authSrc = std::make_shared<AuthSrcDataSyncState>();

    authSink->GetStateType();
    authSrc->GetStateType();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthAclFuzzTest(data, size);
    return 0;
}