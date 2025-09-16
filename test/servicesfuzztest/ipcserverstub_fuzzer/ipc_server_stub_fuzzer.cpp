/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <thread>
#include <unistd.h>

#include "ipc_client_stub.h"
#include "dm_device_info.h"
#include "ipc_server_stub.h"
#include "device_manager_impl.h"
#include "dm_constants.h"
#include "if_system_ability_manager.h"
#include "ipc_cmd_register.h"
#include "ipc_remote_broker.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "ipc_server_stub_fuzzer.h"
#include "iremote_object.h"

namespace OHOS {
namespace DistributedHardware {
void IpcServerStubFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
    MessageParcel data1;
    MessageParcel reply;
    MessageOption option;
    ProcessInfo processInfo;
    std::string pkgName = fdp.ConsumeRandomLengthString();
    processInfo.pkgName = pkgName;
    sptr<IpcRemoteBroker> listener = sptr<IpcServerStub>(new IpcServerStub());
    std::shared_ptr<IpcReq> req = nullptr;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    IpcServerStub::GetInstance().OnRemoteRequest(code, data1, reply, option);
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(processInfo, listener);
    IpcServerStub::GetInstance().GetDmListener(processInfo);
    IpcServerStub::GetInstance().SendCmd(code, req, rsp);
    IpcServerStub::GetInstance().UnRegisterDeviceManagerListener(processInfo);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcServerStubFuzzTest(data, size);
    return 0;
}
