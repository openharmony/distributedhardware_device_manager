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

#include <fuzzer/FuzzedDataProvider.h>
#include "ipc_cmd_register_fuzzer.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unistd.h>

#include "device_manager_ipc_interface_code.h"
#include "ipc_cmd_register.h"
#include "ipc_client_manager.h"
#include "ipc_register_listener_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
void IpcCmdRegisterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t cmdCode = UNREGISTER_DEVICE_MANAGER_LISTENER;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::string pkgNameStr = fdp.ConsumeRandomLengthString(50);
    req->SetPkgName(pkgNameStr);
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    MessageParcel data1;
    MessageParcel reply1;
    data1.WriteString(pkgNameStr);

    IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data1);
    IpcCmdRegister::GetInstance().ReadResponse(cmdCode, data1, rsp);
    IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data1, reply1);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcCmdRegisterFuzzTest(data, size);
    return 0;
}
