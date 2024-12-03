/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ipc_cmd_register_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(IpcCmdRegister);
int32_t IpcCmdRegister::SetRequest(int32_t cmdCode, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    return DmIpcCmdRegister::dmIpcCmdRegister->SetRequest(cmdCode, pBaseReq, data);
}

int32_t IpcCmdRegister::OnIpcCmd(int32_t cmdCode, MessageParcel &data, MessageParcel &reply)
{
    return DmIpcCmdRegister::dmIpcCmdRegister->OnIpcCmd(cmdCode, data, reply);
}
} // namespace DistributedHardware
} // namespace OHOS