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

#ifndef OHOS_IPC_SERVICE_STUB_3RD_H
#define OHOS_IPC_SERVICE_STUB_3RD_H

#include <set>

#include "ffrt.h"

#include "dm_single_instance_3rd.h"
#include "dm_log_3rd.h"
#include "device_manager_service_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "multiple_user_connector_3rd.h"
#include "ipc_interface_code_3rd.h"
#include "ipc_server_client_proxy_3rd.h"
#include "ipc_remote_broker_3rd.h"

#include "iremote_object.h"
#include "iipc_service_stub_3rd.h"

namespace OHOS { class MessageOption; }
namespace OHOS { class MessageParcel; }

namespace OHOS {
namespace DistributedHardware {

class AppDeathRecipient3rd : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    AppDeathRecipient3rd() = default;
    ~AppDeathRecipient3rd() override = default;
};

class IpcServiceStub3rd : public IIpcServiceStub3rd {
DM_DECLARE_SINGLE_INSTANCE_3RD_BASE_3RD(IpcServiceStub3rd);

public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    const ProcessInfo3rd GetDmListenerPkgName(const wptr<IRemoteObject> &remote) override;
    const sptr<IpcRemoteBroker3rd> GetDmListener(ProcessInfo3rd processInfo3rd) override;
    int OnAuth3rdAclSessionOpened(int sessionId, int result) override;
    void OnAuth3rdAclSessionClosed(int sessionId) override;
    void OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen) override;

    int OnAuth3rdSessionOpened(int sessionId, int result) override;
    void OnAuth3rdSessionClosed(int sessionId) override;
    void OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen) override;
    int32_t HandleUserRemoved(int32_t removedUserId) override;
    int32_t HandleAccountLogoutEvent(int32_t userId, const std::string &accountId) override;
private:
    IpcServiceStub3rd() = default;
    ~IpcServiceStub3rd() = default;
    int32_t InitDeviceManager(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t ImportPinCode3rd(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t GeneratePinCode(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t AuthPincode(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t RegisterDeviceManagerListener(const ProcessInfo3rd &processInfo3rd, sptr<IpcRemoteBroker3rd> listener);
    void AddSystemSA(const std::string &pkgName);
    void SetSaUserId(ProcessInfo3rd &processInfo3rd);

    int32_t AuthDevice3rd(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t QueryTrustRelation(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t DeleteTrustRelation(MessageParcel &data, MessageParcel &reply, MessageOption &option);

    mutable ffrt::mutex listenerLock_;
    std::map<ProcessInfo3rd, sptr<AppDeathRecipient3rd>> appRecipient3rd_;
    std::map<ProcessInfo3rd, sptr<IpcRemoteBroker3rd>> dmListener_;
    ffrt::mutex systemSAMtx_;
    std::set<std::string> systemSA_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IPC_SERVICE_STUB_3RD_H