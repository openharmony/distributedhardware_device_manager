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
#include <cstdlib>
#include <random>
#include <vector>
#include <fuzzer/FuzzedDataProvider.h>

#include "device_manager_service_listener.h"
#include "dm_auth_manager.h"
#include "hichain_connector.h"

#include "on_finish_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {

class HiChainConnectorCallbackTest : public IHiChainConnectorCallback {
public:
    HiChainConnectorCallbackTest() {}
    virtual ~HiChainConnectorCallbackTest() {}
    void OnGroupCreated(int64_t requestId, const std::string &groupId) override
    {
        (void)requestId;
        (void)groupId;
    }
    void OnMemberJoin(int64_t requestId, int32_t status, int32_t operationCode) override
    {
        (void)requestId;
        (void)status;
        (void)operationCode;
    }
    std::string GetConnectAddr(std::string deviceId) override
    {
        (void)deviceId;
        return "";
    }
    int32_t GetPinCode(std::string &code) override
    {
        (void)code;
        return DM_OK;
    }
};

void OnFinishFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t) + sizeof(int32_t))) {
        return;
    }

    std::shared_ptr<HiChainConnector> hichainConnector = std::make_shared<HiChainConnector>();
    hichainConnector->RegisterHiChainCallback(std::make_shared<HiChainConnectorCallbackTest>());

    FuzzedDataProvider fdp(data, size);
    int64_t requestId = fdp.ConsumeIntegral<int64_t>();
    int operationCode = fdp.ConsumeIntegral<int32_t>();
    std::string str(reinterpret_cast<const char*>(data), size);
    const char *returnData = str.data();
    hichainConnector->onFinish(requestId, operationCode, returnData);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::OnFinishFuzzTest(data, size);

    return 0;
}