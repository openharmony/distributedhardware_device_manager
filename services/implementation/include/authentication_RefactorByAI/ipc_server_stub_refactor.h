/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_SERVER_STUB_REFACTOR_H
#define OHOS_DM_IPC_SERVER_STUB_REFACTOR_H

#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "ffrt.h"
#include "ipc_remote_broker.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "iremote_stub.h"
#include "system_ability.h"

#include "dm_device_info.h"
#include "dm_single_instance.h"
#include "iipc_service_stub_3rd.h"

namespace OHOS {
namespace DistributedHardware {

enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

constexpr int32_t MAX_PKGNAME_LENGTH = 256;
constexpr int32_t MAX_DUMP_FD = 1024;

class AppDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    AppDeathRecipient() = default;
    ~AppDeathRecipient() override = default;
};

class IpcServerStub : public SystemAbility, public IRemoteStub<IpcRemoteBroker> {
    DECLARE_SYSTEM_ABILITY(IpcServerStub);
    DM_DECLARE_SINGLE_INSTANCE_BASE(IpcServerStub);

public:
    void OnStart() override;
    void OnStop() override;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp) override;
    int32_t RegisterDeviceManagerListener(const ProcessInfo &processInfo, sptr<IpcRemoteBroker> listener);
    int32_t UnRegisterDeviceManagerListener(const ProcessInfo &processInfo);
    ServiceRunningState QueryServiceState() const;
    std::vector<ProcessInfo> GetAllProcessInfo();
    const sptr<IpcRemoteBroker> GetDmListener(ProcessInfo processInfo) const;
    const ProcessInfo GetDmListenerPkgName(const wptr<IRemoteObject> &remote) const;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    std::set<std::string> GetSystemSA();

    int OnAuth3rdAclSessionOpened(int sessionId, int result);
    void OnAuth3rdAclSessionClosed(int sessionId);
    void OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int OnAuth3rdSessionOpened(int sessionId, int result);
    void OnAuth3rdSessionClosed(int sessionId);
    void OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int OnAuthCred3rdSessionOpened(int sessionId, int result);
    void OnAuthCred3rdSessionClosed(int sessionId);
    void OnAuthCred3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen);
    int32_t HandleUserRemoved(int32_t removedUserId);
    int32_t HandleAccountLogoutEvent(int32_t userId, const std::string &accountId);
#ifdef CAR_DEVICE_ENABLE
    const sptr<IpcRemoteBroker> GetListenerByProcessInfo(ProcessInfo processInfo) const;
#endif
private:
    IpcServerStub();
    ~IpcServerStub() override;
    bool Init();
    void AddSystemSA(const std::string &pkgName);
    void RemoveSystemSA(const std::string &pkgName);
    std::string JoinPath(const std::string &prefixPath, const std::string &midPath,
        const std::string &subPath);
    std::string JoinPath(const std::string &prefixPath, const std::string &subPath);
    std::string AddDelimiter(const std::string &path);
    void ReclaimMemmgrFileMemForDM();
    void HandleSoftBusServerAdd();
    bool IsIpcServiceStub3rdReady();
    bool IsValidProcessInfo(const ProcessInfo &processInfo) const;
    void CleanupDeathRecipient(const ProcessInfo &processInfo);
    void Unload3rdService();

private:
    ffrt::mutex registerLock_;
    bool registerToService_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    mutable ffrt::mutex listenerLock_;
    ffrt::mutex systemSALock_;
    std::map<ProcessInfo, sptr<AppDeathRecipient>> appRecipient_;
    std::map<ProcessInfo, sptr<IpcRemoteBroker>> dmListener_;
    std::set<std::string> systemSA_;
    int64_t startBeginTime_ = 0;

private:
    void *ipcServiceStub3rdSoHandle_ = nullptr;
    bool ipcServiceStub3rdSoLoaded_ = false;
    ffrt::mutex ipcServiceStub3rdLoadLock_;
    std::shared_ptr<IIpcServiceStub3rd> ipcServiceStub3rd_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SERVER_STUB_REFACTOR_H
