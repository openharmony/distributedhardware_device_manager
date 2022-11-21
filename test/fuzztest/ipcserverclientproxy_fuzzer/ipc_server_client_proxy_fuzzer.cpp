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

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include "ipc_server_client_proxy.h"
#include "ipc_server_client_proxy_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void IpcServerClientProxyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    int32_t cmdCode = UNREGISTER_DEVICE_MANAGER_LISTENER;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    sptr<IRemoteObject> remoteObject = nullptr;

    auto instance = new IpcServerClientProxy(remoteObject);
    if (instance != nullptr) {
        instance->SendCmd(cmdCode, req, rsp);
        delete instance;
    }
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcServerClientProxyFuzzTest(data, size);

    return 0;
}