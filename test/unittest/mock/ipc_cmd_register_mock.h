/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_IPC_CMD_REGISTER_MOCK_H
#define OHOS_IPC_CMD_REGISTER_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "ipc_cmd_register.h"

namespace OHOS {
namespace DistributedHardware {
class DmIpcCmdRegister {
public:
    virtual ~DmIpcCmdRegister() = default;
public:
    virtual int32_t SetRequest(int32_t cmdCode, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data) = 0;
    virtual int32_t OnIpcCmd(int32_t cmdCode, MessageParcel &data, MessageParcel &reply) = 0;
public:
    static inline std::shared_ptr<DmIpcCmdRegister> dmIpcCmdRegister = nullptr;
};

class IpcCmdRegisterMock : public DmIpcCmdRegister {
public:
    MOCK_METHOD(int32_t, SetRequest, (int32_t, std::shared_ptr<IpcReq>, MessageParcel &));
    MOCK_METHOD(int32_t, OnIpcCmd, (int32_t, MessageParcel &, MessageParcel &));
};
}
}
#endif
