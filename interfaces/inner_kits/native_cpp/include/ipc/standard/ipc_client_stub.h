/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_CLIENT_STUB_H
#define OHOS_DM_IPC_CLIENT_STUB_H

#include "ipc_remote_broker.h"
#include "iremote_stub.h"
namespace OHOS { class MessageOption; }
namespace OHOS { class MessageParcel; }
namespace OHOS::DistributedHardware { class IpcReq; }
namespace OHOS::DistributedHardware { class IpcRsp; }

namespace OHOS {
namespace DistributedHardware {
class IpcClientStub : public IRemoteStub<IpcRemoteBroker> {
public:
    IpcClientStub() {};
    ~IpcClientStub() {};
    /**
     * @tc.name: IpcClientStub::OnRemoteRequest
     * @tc.desc: Ipc Client Stub remote request
     * @tc.type: FUNC
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    /**
     * @tc.name: IpcClientStub::SendCmd
     * @tc.desc: Ipc Client Stub Send command
     * @tc.type: FUNC
     */
    int32_t SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_CLIENT_STUB_H
