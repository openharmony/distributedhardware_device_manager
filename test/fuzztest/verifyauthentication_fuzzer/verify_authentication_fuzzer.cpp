/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "verify_authentication_fuzzer.h"

#include "device_manager_impl.h"

namespace OHOS {
namespace DistributedHardware {

class MockVerifyAuthCallback : public VerifyAuthCallback {
public:
    void OnVerifyAuthResult(const std::string &deviceId, int32_t resultCode, int32_t flag)
    {
        (void)deviceId;
        (void)resultCode;
        (void)flag;
    }
};

void VerifyAuthenticationFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string authPara(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<VerifyAuthCallback> callback = std::make_shared<MockVerifyAuthCallback>();
    DeviceManagerImpl::GetInstance().VerifyAuthentication(pkgName, authPara, callback);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::VerifyAuthenticationFuzzTest(data, size);

    return 0;
}
