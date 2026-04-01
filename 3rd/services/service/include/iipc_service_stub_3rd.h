/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_I_IPC_SERVICE_STUB_3RD_H
#define OHOS_I_IPC_SERVICE_STUB_3RD_H

#include "device_manager_data_struct_3rd.h"
#include "iremote_object.h"
#include "ipc_remote_broker_3rd.h"

namespace OHOS { class MessageOption; }
namespace OHOS { class MessageParcel; }

namespace OHOS {
namespace DistributedHardware {
class IIpcServiceStub3rd {

public:
    virtual ~IIpcServiceStub3rd() {}

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) = 0;
    virtual const ProcessInfo3rd GetDmListenerPkgName(const wptr<IRemoteObject> &remote) = 0;
    virtual const sptr<IpcRemoteBroker3rd> GetDmListener(ProcessInfo3rd processInfo3rd) = 0;

    virtual int OnAuth3rdAclSessionOpened(int sessionId, int result) = 0;
    virtual void OnAuth3rdAclSessionClosed(int sessionId) = 0;
    virtual void OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen) = 0;

    virtual int OnAuth3rdSessionOpened(int sessionId, int result) = 0;
    virtual void OnAuth3rdSessionClosed(int sessionId) = 0;
    virtual void OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen) = 0;
    virtual int32_t HandleUserRemoved(int32_t removedUserId) = 0;
    virtual int32_t HandleAccountLogoutEvent(int32_t userId, const std::string &accountId) = 0;
};

using CreateIpcServiceStub3rdFuncPtr = IIpcServiceStub3rd *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_IPC_SERVICE_STUB_3RD_H