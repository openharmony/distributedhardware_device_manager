/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <string>
#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "dm_credential_impl_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
class CredentialCallbackFuzzTest : public CredentialCallback {
public:
    virtual ~CredentialCallbackFuzzTest() {}
    virtual void OnCredentialResult(int32_t &action, const std::string &credentialResult) override {}
};

void DeviceManagerCredentialFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string szData(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<CredentialCallbackFuzzTest> callback = std::make_shared<CredentialCallbackFuzzTest>();
    std::string returnJsonStr;

    DeviceManager::GetInstance().RequestCredential(szData, szData, returnJsonStr);
    DeviceManager::GetInstance().ImportCredential(szData, szData);
    DeviceManager::GetInstance().DeleteCredential(szData, szData);
    DeviceManager::GetInstance().RegisterCredentialCallback(szData, callback);
    DeviceManager::GetInstance().UnRegisterCredentialCallback(szData);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceManagerCredentialFuzzTest(data, size);

    return 0;
}
