/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ipc_client_manager_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "device_manager_ipc_interface_code.h"
#include "ipc_client_manager.h"

namespace OHOS {
namespace DistributedHardware {
void IpcClientManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    int32_t cmdCode = fdp.ConsumeIntegralInRange<int32_t>(REGISTER_DEVICE_MANAGER_LISTENER, IPC_MSG_BUTT);
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    req->SetPkgName(pkgName);

    std::shared_ptr<IpcClientManager> instance = std::make_shared<IpcClientManager>();
    instance->Init(pkgName);
    instance->UnInit(pkgName);
    instance->SendRequest(cmdCode, req, rsp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcClientManagerFuzzTest(data, size);

    return 0;
}
