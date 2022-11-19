/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "pin_auth.h"

#include <memory>
#include <string>

#include "dm_constants.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "device_manager_service_listener.h"

#include "pinauth_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void PinAuthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<DmAuthManager> authManager =
            std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector);

    std::string authToken(reinterpret_cast<const char*>(data), size);
    std::string authParam(reinterpret_cast<const char*>(data), size);

    std::shared_ptr<PinAuth> pinauth = std::make_shared<PinAuth>();
    pinauth->ShowAuthInfo(authToken, authManager);
    pinauth->StartAuth(authToken, authManager);
    pinauth->VerifyAuthentication(authToken, authParam);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::PinAuthFuzzTest(data, size);

    return 0;
}
