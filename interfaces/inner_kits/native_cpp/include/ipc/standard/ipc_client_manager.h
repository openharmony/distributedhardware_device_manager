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

#ifndef OHOS_DM_IPC_CLIENT_MANAGER_H
#define OHOS_DM_IPC_CLIENT_MANAGER_H

#include "ipc_client.h"
#include "ipc_client_stub.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    /**
     * @tc.name: DmDeathRecipient::OnRemoteDied
     * @tc.desc: ipc client initialization
     * @tc.type: FUNC
     */
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    DmDeathRecipient() = default;
    ~DmDeathRecipient() override = default;
};

class IpcClientManager : public IpcClient {
    friend class DmDeathRecipient;
    DECLARE_IPC_INTERFACE(IpcClientManager);
public:
    class SystemAbilityListener : public SystemAbilityStatusChangeStub {
    public:
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    };

public:
    /**
     * @tc.name: IpcClientManager::Init
     * @tc.desc: ipc client Manager initialization
     * @tc.type: FUNC
     */
    virtual int32_t Init(const std::string &pkgName) override;
    /**
     * @tc.name: IpcClientManager::UnInit
     * @tc.desc: ipc client Manager Uninitialization
     * @tc.type: FUNC
     */
    virtual int32_t UnInit(const std::string &pkgName) override;
    /**
     * @tc.name: IpcClientManager::SendRequest
     * @tc.desc: ipc client Manager Send Request
     * @tc.type: FUNC
     */
    virtual int32_t SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp) override;

    virtual int32_t OnDmServiceDied() override;
private:
    int32_t ClientInit();
    void SubscribeDMSAChangeListener();
private:
    std::mutex lock_;
    std::map<std::string, sptr<IpcClientStub>> dmListener_;
    sptr<IpcRemoteBroker> dmInterface_ { nullptr };
    sptr<DmDeathRecipient> dmRecipient_ { nullptr };
    std::atomic<bool> isSubscribeDMSAChangeListener = false;
    sptr<SystemAbilityListener> saListenerCallback = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_CLIENT_MANAGER_H
