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

#include "ipc_server_stub.h"
#include "ipc_cmd_register.h"
#include "ipc_skeleton.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "kv_adapter_manager.h"
#endif
#ifdef SUPPORT_MEMMGR
#include "mem_mgr_client.h"
#include "mem_mgr_proxy.h"
#endif // SUPPORT_MEMMGR
#include "system_ability_definition.h"
#include "device_manager_service.h"
#include "device_manager_service_notify.h"
#include "device_name_manager.h"
#include "dm_error_type.h"
#include "dm_device_info.h"
#include "ffrt.h"
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include "dm_log.h"
#include "multiple_user_connector.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(IpcServerStub);

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&IpcServerStub::GetInstance());
constexpr int32_t DM_IPC_THREAD_NUM = 32;
constexpr int32_t MAX_CALLBACK_NUM = 5000;
constexpr int32_t RECLAIM_DELAY_TIME = 5 * 60 * 1000 * 1000; // 5 minutes
constexpr int32_t ECHO_COUNT = 2;

IpcServerStub::IpcServerStub() : SystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, true)
{
    registerToService_ = false;
    state_ = ServiceRunningState::STATE_NOT_START;
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
    AddSystemAbilityListener(DEVICE_AUTH_SERVICE_ID);
    AddSystemAbilityListener(ACCESS_TOKEN_MANAGER_SERVICE_ID);
    DeviceManagerService::GetInstance().SubscribePackageCommonEvent();
}

void IpcServerStub::ReclaimMemmgrFileMemForDM()
{
    int32_t memmgrPid = getpid();
    int32_t echoCnt = ECHO_COUNT;
    for (int32_t i = 0; i < echoCnt; ++i) {
        if (memmgrPid <= 0) {
            LOGE("Get invalid pid : %{public}d.", memmgrPid);
            return;
        }
        std::string path = JoinPath("/proc/", std::to_string(memmgrPid), "reclaim");
        std::string contentStr = "1";
        LOGI("Start echo 1 to pid : %{public}d, path: %{public}s", memmgrPid, path.c_str());
        int32_t fd = open(path.c_str(), O_WRONLY);
        if (fd == -1) {
            LOGE("ReclaimMemmgrFileMemForDM open file failed.");
            return;
        }
        if (write(fd, contentStr.c_str(), strlen(contentStr.c_str())) < 0) {
            LOGE("ReclaimMemmgrFileMemForDM write file failed.");
            close(fd);
            return;
        }
        close(fd);
    }
    LOGI("ReclaimMemmgrFileMemForDM success.");
}

std::string IpcServerStub::AddDelimiter(const std::string &path)
{
    if (path.empty()) {
        return path;
    }
    if (path.rfind("/") != path.size() - 1) {
        return path + "/";
    }
    return path;
}

std::string IpcServerStub::JoinPath(const std::string &prefixPath, const std::string &subPath)
{
    return AddDelimiter(prefixPath) + subPath;
}

std::string IpcServerStub::JoinPath(const std::string &prefixPath, const std::string &midPath,
    const std::string &subPath)
{
    return JoinPath(JoinPath(prefixPath, midPath), subPath);
}

void IpcServerStub::HandleSoftBusServerAdd()
{
    DeviceManagerService::GetInstance().InitSoftbusListener();
    if (!Init()) {
        LOGE("failed to init IpcServerStub");
        state_ = ServiceRunningState::STATE_NOT_START;
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    DeviceNameManager::GetInstance().InitDeviceNameWhenSoftBusReady();
    ReclaimMemmgrFileMemForDM();
    std::function<void()> task = [this]() {
        LOGI("HandleSoftBusServerAdd After 5mins.");
        ReclaimMemmgrFileMemForDM();
    };
    ffrt::submit(task, ffrt::task_attr().delay(RECLAIM_DELAY_TIME));
    return;
}

void IpcServerStub::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        HandleSoftBusServerAdd();
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
        DMAccountInfo dmAccountInfo;
        dmAccountInfo.accountId = MultipleUserConnector::GetOhosAccountId();
        dmAccountInfo.accountName = MultipleUserConnector::GetOhosAccountName();
        MultipleUserConnector::SetAccountInfo(MultipleUserConnector::GetCurrentAccountUserID(), dmAccountInfo);
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
        return;
    }
#endif
    if (systemAbilityId == DEVICE_AUTH_SERVICE_ID) {
        DeviceManagerService::GetInstance().InitHichainListener();
        return;
    }
    if (systemAbilityId == ACCESS_TOKEN_MANAGER_SERVICE_ID) {
        DeviceManagerService::GetInstance().InitHichainListener();
        return;
    }
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

int32_t IpcServerStub::RegisterDeviceManagerListener(const ProcessInfo &processInfo, sptr<IpcRemoteBroker> listener)
{
    LOGI("RegisterDeviceManagerListener start");
    if (processInfo.pkgName.empty() || listener == nullptr) {
        LOGE("RegisterDeviceManagerListener error: input parameter invalid.");
        return ERR_DM_POINT_NULL;
    }

    LOGI("Register device manager listener for package name: %{public}s", processInfo.pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto iter = dmListener_.find(processInfo);
    if (iter != dmListener_.end()) {
        LOGI("Listener already exists");
        auto recipientIter = appRecipient_.find(processInfo);
        if (recipientIter == appRecipient_.end()) {
            LOGI("AppRecipient not exists");
            dmListener_.erase(processInfo);
        } else {
            auto listener = iter->second;
            auto appRecipient = recipientIter->second;
            listener->AsObject()->RemoveDeathRecipient(appRecipient);
            appRecipient_.erase(processInfo);
            dmListener_.erase(processInfo);
        }
    }
    sptr<AppDeathRecipient> appRecipient = sptr<AppDeathRecipient>(new AppDeathRecipient());
    LOGI("Add death recipient.");
    if (!listener->AsObject()->AddDeathRecipient(appRecipient)) {
        LOGE("AddDeathRecipient Failed");
    }
    LOGI("Checking the number of listeners.");
    if (dmListener_.size() > MAX_CALLBACK_NUM || appRecipient_.size() > MAX_CALLBACK_NUM) {
        LOGE("dmListener_ or appRecipient_ size exceed the limit!");
        return ERR_DM_FAILED;
    }
    dmListener_[processInfo] = listener;
    appRecipient_[processInfo] = appRecipient;
    LOGI("Add system sa.");
    AddSystemSA(processInfo.pkgName);
    LOGI("Register listener complete.");
    return DM_OK;
}

int32_t IpcServerStub::UnRegisterDeviceManagerListener(const ProcessInfo &processInfo)
{
    if (processInfo.pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("IpcServerStub::UnRegisterDeviceManagerListener In, pkgName: %{public}s", processInfo.pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto listenerIter = dmListener_.find(processInfo);
    if (listenerIter == dmListener_.end()) {
        LOGI("Listener not exists");
        return DM_OK;
    }
    auto recipientIter = appRecipient_.find(processInfo);
    if (recipientIter == appRecipient_.end()) {
        LOGI("AppRecipient not exists");
        dmListener_.erase(processInfo);
        return DM_OK;
    }
    auto listener = listenerIter->second;
    auto appRecipient = recipientIter->second;
    listener->AsObject()->RemoveDeathRecipient(appRecipient);
    appRecipient_.erase(processInfo);
    dmListener_.erase(processInfo);
    RemoveSystemSA(processInfo.pkgName);
    DeviceManagerService::GetInstance().RemoveNotifyRecord(processInfo);
    return DM_OK;
}

std::vector<ProcessInfo> IpcServerStub::GetAllProcessInfo()
{
    std::vector<ProcessInfo> processInfoVec;
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    for (const auto &iter : dmListener_) {
        processInfoVec.push_back(iter.first);
    }
    return processInfoVec;
}

const sptr<IpcRemoteBroker> IpcServerStub::GetDmListener(ProcessInfo processInfo) const
{
    if (processInfo.pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return nullptr;
    }
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto iter = dmListener_.find(processInfo);
    if (iter == dmListener_.end()) {
        return nullptr;
    }
    return iter->second;
}

const ProcessInfo IpcServerStub::GetDmListenerPkgName(const wptr<IRemoteObject> &remote) const
{
    ProcessInfo processInfo;
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    for (const auto &iter : dmListener_) {
        if ((iter.second)->AsObject() == remote.promote()) {
            processInfo = iter.first;
            break;
        }
    }
    return processInfo;
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
    ProcessInfo processInfo = IpcServerStub::GetInstance().GetDmListenerPkgName(remote);
    LOGI("AppDeathRecipient: OnRemoteDied for %{public}s", processInfo.pkgName.c_str());
    IpcServerStub::GetInstance().UnRegisterDeviceManagerListener(processInfo);
    DeviceManagerService::GetInstance().ClearDiscoveryCache(processInfo);
    DeviceManagerServiceNotify::GetInstance().ClearDiedProcessCallback(processInfo);
    DeviceManagerService::GetInstance().ClearPulishIdCache(processInfo.pkgName);
}

void IpcServerStub::AddSystemSA(const std::string &pkgName)
{
    if (PermissionManager::GetInstance().CheckSystemSA(pkgName)) {
        systemSA_.insert(pkgName);
    }
}

void IpcServerStub::RemoveSystemSA(const std::string &pkgName)
{
    if (PermissionManager::GetInstance().CheckSystemSA(pkgName) || systemSA_.find(pkgName) != systemSA_.end()) {
        systemSA_.erase(pkgName);
    }
}

std::set<std::string> IpcServerStub::GetSystemSA()
{
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    std::set<std::string> systemSA;
    for (const auto &item : systemSA_) {
        systemSA.insert(item);
    }
    return systemSA;
}
} // namespace DistributedHardware
} // namespace OHOS
