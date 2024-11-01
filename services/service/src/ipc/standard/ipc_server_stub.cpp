/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "ipc_server_stub.h"

#include "if_system_ability_manager.h"
#include "ipc_cmd_register.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "kv_adapter_manager.h"
#endif
#ifdef SUPPORT_MEMMGR
#include "mem_mgr_client.h"
#include "mem_mgr_proxy.h"
#endif // SUPPORT_MEMMGR

#include "string_ex.h"
#include "system_ability_definition.h"
#include "device_manager_ipc_interface_code.h"
#include "device_manager_service.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(IpcServerStub);

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&IpcServerStub::GetInstance());
constexpr int32_t DM_IPC_THREAD_NUM = 32;

IpcServerStub::IpcServerStub() : SystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, true)
{
    registerToService_ = false;
    state_ = ServiceRunningState::STATE_NOT_START;
    accountBootListener_ = std::make_shared<AccountBootListener>();
}

void IpcServerStub::OnStart()
{
    LOGI("IpcServerStub::OnStart start");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGI("IpcServerStub has already started.");
        return;
    }

    IPCSkeleton::SetMaxWorkThreadNum(DM_IPC_THREAD_NUM);

    LOGI("called:AddAbilityListener begin!");
    AddSystemAbilityListener(DISTRIBUTED_HARDWARE_SA_ID);
#ifdef SUPPORT_MEMMGR
    AddSystemAbilityListener(MEMORY_MANAGER_SA_ID);
#endif // SUPPORT_MEMMGR
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
    AddSystemAbilityListener(SCREENLOCK_SERVICE_ID);
    AddSystemAbilityListener(SOFTBUS_SERVER_SA_ID);
    LOGI("called:AddAbilityListener end!");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    AddSystemAbilityListener(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
#endif
    DeviceManagerService::GetInstance().SubscribePackageCommonEvent();
}

void IpcServerStub::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        DeviceManagerService::GetInstance().InitSoftbusListener();
        if (!Init()) {
            LOGE("failed to init IpcServerStub");
            state_ = ServiceRunningState::STATE_NOT_START;
            return;
        }
        state_ = ServiceRunningState::STATE_RUNNING;
        std::string localDisplayName = accountBootListener_->GetLocalDisplayName();
        if (!localDisplayName.empty()) {
            LOGI("Already have display deviceName=%{public}s", localDisplayName.c_str());
            DeviceManagerService::GetInstance().SetLocalDeviceName(localDisplayName);
            return;
        }
        accountBootListener_->SetSaTriggerFlag(SaTriggerFlag::DM_SA_READY);
        return;
    }

#ifdef SUPPORT_MEMMGR
    if (systemAbilityId == MEMORY_MANAGER_SA_ID) {
        int pid = getpid();
        Memory::MemMgrClient::GetInstance().NotifyProcessStatus(pid, 1, 1, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        return;
    }
#endif // SUPPORT_MEMMGR

    if (systemAbilityId == SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN) {
        MultipleUserConnector::SetSwitchOldUserId(MultipleUserConnector::GetCurrentAccountUserID());
        MultipleUserConnector::SetSwitchOldAccountId(MultipleUserConnector::GetOhosAccountId());
        MultipleUserConnector::SetSwitchOldAccountName(MultipleUserConnector::GetOhosAccountName());
        DeviceManagerService::GetInstance().InitAccountInfo();
        return;
    }

    if (systemAbilityId == SCREENLOCK_SERVICE_ID) {
        DeviceManagerService::GetInstance().InitScreenLockEvent();
        return;
    }

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (systemAbilityId == DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID) {
        KVAdapterManager::GetInstance().ReInit();
        accountBootListener_->SetSaTriggerFlag(SaTriggerFlag::DATA_SHARE_SA_REDDY);
        return;
    }
#endif
}

void IpcServerStub::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("OnRemoveSystemAbility systemAbilityId:%{public}d removed!", systemAbilityId);
    if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        DeviceManagerService::GetInstance().UninitSoftbusListener();
    } else if (systemAbilityId == DISTRIBUTED_HARDWARE_SA_ID) {
        DeviceManagerService::GetInstance().LoadHardwareFwkService();
    }
}

bool IpcServerStub::Init()
{
    LOGI("IpcServerStub::Init ready to init.");
    DeviceManagerService::GetInstance().InitDMServiceListener();
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            LOGE("IpcServerStub::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
        KVAdapterManager::GetInstance().Init();
    }
    return true;
}

void IpcServerStub::OnStop()
{
    LOGI("IpcServerStub::OnStop ready to stop service.");
    DeviceManagerService::GetInstance().UninitDMServiceListener();
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
#ifdef SUPPORT_MEMMGR
    int pid = getpid();
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(pid, 1, 0, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
#endif // SUPPORT_MEMMGR
    LOGI("IpcServerStub::OnStop end.");
}

int32_t IpcServerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
        LOGI("ReadInterfaceToken fail!");
        return ERR_DM_IPC_READ_FAILED;
    }
    int32_t ret = IpcCmdRegister::GetInstance().OnIpcCmd(static_cast<int32_t>(code), data, reply);
    if (ret == ERR_DM_UNSUPPORTED_IPC_COMMAND) {
        LOGW("unsupported code: %{public}d", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}

int32_t IpcServerStub::SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcServerStub::SendCmd error: Invalid para, cmdCode: %{public}d", (int32_t)cmdCode);
        return IPCObjectStub::OnRemoteRequest(cmdCode, data, reply, option);
    }

    if (IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data) != DM_OK) {
        LOGE("set request cmd failed");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    int32_t ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    if (ret == ERR_DM_UNSUPPORTED_IPC_COMMAND) {
        LOGW("unsupported code: %{public}d", cmdCode);
        return IPCObjectStub::OnRemoteRequest(cmdCode, data, reply, option);
    }
    return IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
}

ServiceRunningState IpcServerStub::QueryServiceState() const
{
    return state_;
}

int32_t IpcServerStub::RegisterDeviceManagerListener(std::string &pkgName, sptr<IpcRemoteBroker> listener)
{
    if (pkgName.empty() || listener == nullptr) {
        LOGE("RegisterDeviceManagerListener error: input parameter invalid.");
        return ERR_DM_POINT_NULL;
    }

    LOGI("Register device manager listener for package name: %{public}s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto iter = dmListener_.find(pkgName);
    if (iter != dmListener_.end()) {
        LOGI("Listener already exists");
        auto recipientIter = appRecipient_.find(pkgName);
        if (recipientIter == appRecipient_.end()) {
            LOGI("AppRecipient not exists");
            dmListener_.erase(pkgName);
        } else {
            auto listener = iter->second;
            auto appRecipient = recipientIter->second;
            listener->AsObject()->RemoveDeathRecipient(appRecipient);
            appRecipient_.erase(pkgName);
            dmListener_.erase(pkgName);
        }
    }

    sptr<AppDeathRecipient> appRecipient = sptr<AppDeathRecipient>(new AppDeathRecipient());
    if (!listener->AsObject()->AddDeathRecipient(appRecipient)) {
        LOGE("AddDeathRecipient Failed");
    }
    dmListener_[pkgName] = listener;
    appRecipient_[pkgName] = appRecipient;
    LOGD("Register listener complete.");
    return DM_OK;
}

int32_t IpcServerStub::UnRegisterDeviceManagerListener(std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("IpcServerStub::UnRegisterDeviceManagerListener In, pkgName: %{public}s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto listenerIter = dmListener_.find(pkgName);
    if (listenerIter == dmListener_.end()) {
        LOGI("Listener not exists");
        return DM_OK;
    }
    auto recipientIter = appRecipient_.find(pkgName);
    if (recipientIter == appRecipient_.end()) {
        LOGI("AppRecipient not exists");
        dmListener_.erase(pkgName);
        return DM_OK;
    }
    auto listener = listenerIter->second;
    auto appRecipient = recipientIter->second;
    listener->AsObject()->RemoveDeathRecipient(appRecipient);
    appRecipient_.erase(pkgName);
    dmListener_.erase(pkgName);
    return DM_OK;
}

int32_t IpcServerStub::SendALL(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    for (const auto &iter : dmListener_) {
        auto pkgName = iter.first;
        auto listener = iter.second;
        req->SetPkgName(pkgName);
        if (listener == nullptr) {
            LOGE("IpcServerStub::SendALL, listener is nullptr, pkgName : %{public}s.", pkgName.c_str());
            continue;
        }
        listener->SendCmd(cmdCode, req, rsp);
    }
    return DM_OK;
}

std::vector<std::string> IpcServerStub::GetAllPkgName()
{
    std::vector<std::string> PkgNameVec;
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    for (const auto &iter : dmListener_) {
        PkgNameVec.push_back(iter.first);
    }
    return PkgNameVec;
}

const sptr<IpcRemoteBroker> IpcServerStub::GetDmListener(std::string pkgName) const
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return nullptr;
    }
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto iter = dmListener_.find(pkgName);
    if (iter == dmListener_.end()) {
        return nullptr;
    }
    return iter->second;
}

const std::string IpcServerStub::GetDmListenerPkgName(const wptr<IRemoteObject> &remote) const
{
    std::string pkgName = "";
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    for (const auto &iter : dmListener_) {
        if ((iter.second)->AsObject() == remote.promote()) {
            pkgName = iter.first;
            break;
        }
    }
    return pkgName;
}

int32_t IpcServerStub::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    LOGI("DistributedHardwareService Dump.");
    std::vector<std::string> argsStr {};
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    std::string result("");
    int ret = DeviceManagerService::GetInstance().DmHiDumper(argsStr, result);
    if (ret != DM_OK) {
        LOGE("Dump error, ret = %{public}d", ret);
    }

    ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        LOGE("HiDumper dprintf error");
        ret = ERR_DM_FAILED;
    }
    return ret;
}

void AppDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    std::string pkgName = IpcServerStub::GetInstance().GetDmListenerPkgName(remote);
    LOGI("AppDeathRecipient: OnRemoteDied for %{public}s", pkgName.c_str());
    IpcServerStub::GetInstance().UnRegisterDeviceManagerListener(pkgName);
    DeviceManagerService::GetInstance().ClearDiscoveryCache(pkgName);
}
} // namespace DistributedHardware
} // namespace OHOS
