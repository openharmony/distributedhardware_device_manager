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

#ifndef OHOS_DM_IPC_SERVER_STUB_H
#define OHOS_DM_IPC_SERVER_STUB_H

#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <vector>

#include "ipc_remote_broker.h"
#include "iremote_stub.h"
#include "single_instance.h"
#include "system_ability.h"

namespace OHOS {
namespace DistributedHardware {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class AppDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    /**
     * @tc.name: AppDeathRecipient::OnRemoteDied
     * @tc.desc: OnRemoteDied function of the App DeathRecipient
     * @tc.type: FUNC
     */
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    AppDeathRecipient() = default;
    ~AppDeathRecipient() = default;
};

class IpcServerStub : public SystemAbility, public IRemoteStub<IpcRemoteBroker> {
    DECLARE_SYSTEM_ABILITY(IpcServerStub);
    DECLARE_SINGLE_INSTANCE_BASE(IpcServerStub);

public:
    /**
     * @tc.name: IpcServerStub::OnStart
     * @tc.desc: OnStart of the IpcServerStub
     * @tc.type: FUNC
     */
    void OnStart() override;

    /**
     * @tc.name: IpcServerStub::OnStop
     * @tc.desc: OnStop of the IpcServerStub
     * @tc.type: FUNC
     */
    void OnStop() override;

    /**
     * @tc.name: IpcServerStub::OnRemoteRequest
     * @tc.desc: On Remote Request of the IpcServerStub
     * @tc.type: FUNC
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * @tc.name: IpcServerStub::SendCmd
     * @tc.desc: Send Cmd of the IpcServerStub
     * @tc.type: FUNC
     */
    int32_t SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp) override;

    /**
     * @tc.name: IpcServerStub::RegisterDeviceManagerListener
     * @tc.desc: Register DeviceManager Listener of the IpcServerStub
     * @tc.type: FUNC
     */
    int32_t RegisterDeviceManagerListener(std::string &pkgName, sptr<IRemoteObject> listener);

    /**
     * @tc.name: IpcServerStub::UnRegisterDeviceManagerListener
     * @tc.desc: UnRegister DeviceManager Listener of the IpcServerStub
     * @tc.type: FUNC
     */
    int32_t UnRegisterDeviceManagerListener(std::string &pkgName);

    /**
     * @tc.name: IpcServerStub::QueryServiceState
     * @tc.desc: Query Service State of the IpcServerStub
     * @tc.type: FUNC
     */
    ServiceRunningState QueryServiceState() const;

    /**
     * @tc.name: IpcServerStub::GetDmListener
     * @tc.desc: GetDmListener of the IpcServerStub
     * @tc.type: FUNC
     */
    const std::map<std::string, sptr<IRemoteObject>> &GetDmListener();

    /**
     * @tc.name: IpcServerStub::GetDmListener
     * @tc.desc: Get DmListener of the IpcServerStub
     * @tc.type: FUNC
     */
    const sptr<IpcRemoteBroker> GetDmListener(std::string pkgName) const;

    /**
     * @tc.name: IpcServerStub::Dump
     * @tc.desc: Dump of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    /**
     * @tc.name: IpcServerStub::OnAddSystemAbility
     * @tc.desc: OnAddSystemAbility of the IpcServerStub
     * @tc.type: FUNC
     */
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    /**
     * @tc.name: IpcServerStub::OnRemoveSystemAbility
     * @tc.desc: OnRemoveSystemAbility of the IpcServerStub
     * @tc.type: FUNC
     */
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    IpcServerStub();
    ~IpcServerStub() = default;
    bool Init();

private:
    bool registerToService_;
    ServiceRunningState state_;
    mutable std::mutex listenerLock_;
    std::map<std::string, sptr<AppDeathRecipient>> appRecipient_;
    std::map<std::string, sptr<IRemoteObject>> dmListener_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SERVER_STUB_H
