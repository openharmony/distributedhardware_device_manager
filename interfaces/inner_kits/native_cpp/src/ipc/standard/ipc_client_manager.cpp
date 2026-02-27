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

#include <set>

#include "ipc_client_manager.h"

#include "device_manager_ipc_interface_code.h"
#include "device_manager_notify.h"
#include "device_manager_impl.h"
#include "dm_constants.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "dm_service_load.h"
#include "ipc_client_server_proxy.h"
#include "ipc_client_stub.h"
#include "ipc_register_listener_req.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
void DmDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LOGW("DmDeathRecipient : OnRemoteDied");
    (void)remote;
}

int32_t IpcClientManager::ClientInit()
{
    LOGI("Start");
    if (dmInterface_ != nullptr) {
        LOGI("DeviceManagerService Already Init");
        return DM_OK;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get SystemAbilityManager Failed");
        return ERR_DM_INIT_FAILED;
    }

    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (object == nullptr) {
        LOGE("Get DeviceManager SystemAbility Failed");
        DmServiceLoad::GetInstance().LoadDMService();
        return ERR_DM_INIT_FAILED;
    }

    if (dmRecipient_ == nullptr) {
        dmRecipient_ = sptr<DmDeathRecipient>(new DmDeathRecipient());
    }
    if (!object->AddDeathRecipient(dmRecipient_)) {
        LOGE("InitDeviceManagerService: AddDeathRecipient Failed");
    }
    if (!object->IsProxyObject()) {
        LOGI("object is not proxy object.");
        dmInterface_ = sptr<IpcClientServerProxy>(new IpcClientServerProxy(object));
        return DM_OK;
    }
    dmInterface_ = iface_cast<IpcRemoteBroker>(object);
    LOGI("Completed");
    return DM_OK;
}

int32_t IpcClientManager::Init(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    SubscribeDMSAChangeListener();
    int32_t ret = ClientInit();
    if (ret != DM_OK) {
        LOGE("InitDeviceManager Failed with ret %{public}d", ret);
        return ret;
    }
    CHECK_SIZE_RETURN(dmListener_, ERR_DM_FAILED);
    sptr<IpcClientStub> listener = sptr<IpcClientStub>(new IpcClientStub());
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetListener(listener);
    CHECK_NULL_RETURN(dmInterface_, ERR_DM_POINT_NULL);
    ret = dmInterface_->SendCmd(REGISTER_DEVICE_MANAGER_LISTENER, req, rsp);
    if (ret != DM_OK) {
        LOGE("InitDeviceManager: RegisterDeviceManagerListener Failed with ret %{public}d", ret);
        return ret;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        return ret;
    }
    dmListener_[pkgName] = listener;
    LOGI("completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t IpcClientManager::UnInit(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("UnInit in, pkgName %{public}s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmInterface_ == nullptr) {
        LOGE("DeviceManager not Init");
        return ERR_DM_INIT_FAILED;
    }

    if (dmListener_.count(pkgName) > 0) {
        std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
        std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
        req->SetPkgName(pkgName);
        int32_t ret = dmInterface_->SendCmd(UNREGISTER_DEVICE_MANAGER_LISTENER, req, rsp);
        if (ret != DM_OK) {
            LOGE("UnRegisterDeviceManagerListener Failed with ret %{public}d", ret);
            return ret;
        }
        dmListener_.erase(pkgName);
    }
    if (dmListener_.empty()) {
        if (dmRecipient_ != nullptr) {
            dmInterface_->AsObject()->RemoveDeathRecipient(dmRecipient_);
            dmRecipient_ = nullptr;
        }
        dmInterface_ = nullptr;
    }
    LOGI("completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t IpcClientManager::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcClientManager::SendRequest cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    if (req == nullptr || rsp == nullptr) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmInterface_ != nullptr) {
        LOGD("IpcClientManager::SendRequest cmdCode: %{public}d", cmdCode);
        return dmInterface_->SendCmd(cmdCode, req, rsp);
    } else {
        LOGE("dmInterface_ is not init.");
        return ERR_DM_INIT_FAILED;
    }
}

int32_t IpcClientManager::OnDmServiceDied()
{
    LOGI("IpcClientManager::OnDmServiceDied begin");
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (dmInterface_ == nullptr) {
            LOGE("IpcClientManager::OnDmServiceDied, dmInterface_ null");
            return ERR_DM_POINT_NULL;
        }
        if (dmRecipient_ != nullptr) {
            dmInterface_->AsObject()->RemoveDeathRecipient(dmRecipient_);
            dmRecipient_ = nullptr;
        }
        dmInterface_ = nullptr;
    }
    LOGI("IpcClientManager::OnDmServiceDied complete");
    return DM_OK;
}

void IpcClientManager::SubscribeDMSAChangeListener()
{
    saListenerCallback = new (std::nothrow) SystemAbilityListener();
    if (saListenerCallback == nullptr) {
        LOGE("saListenerCallback is nullptr.");
        return;
    }
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();

    if (systemAbilityManager == nullptr) {
        LOGE("get system ability manager failed.");
        return;
    }

    if (!isSubscribeDMSAChangeListener.load()) {
        LOGI("try subscribe source sa change listener, sa id: %{public}d", DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        int32_t ret = systemAbilityManager->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID,
            saListenerCallback);
        if (ret != DM_OK) {
            LOGE("subscribe source sa change failed: %{public}d", ret);
            return;
        }
        isSubscribeDMSAChangeListener.store(true);
    }
}

void IpcClientManager::SystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    if (systemAbilityId == DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID) {
        DeviceManagerImpl::GetInstance().OnDmServiceDied();
        DeviceManagerNotify::GetInstance().OnRemoteDied();
    }
    LOGI("sa %{public}d is removed.", systemAbilityId);
}

void IpcClientManager::SystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("sa %{public}d is added.", systemAbilityId);
    if (systemAbilityId == DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID) {
        std::map<std::string, std::shared_ptr<DmInitCallback>> dmInitCallback =
            DeviceManagerNotify::GetInstance().GetDmInitCallback();
        if (dmInitCallback.size() == 0) {
            LOGI("dmInitCallback is empty when ReInit");
            return;
        }
        for (auto iter : dmInitCallback) {
            DeviceManagerImpl::GetInstance().InitDeviceManager(iter.first, iter.second);
        }
        std::map<DmCommonNotifyEvent, std::set<std::string>> callbackMap;
        DeviceManagerNotify::GetInstance().GetCallBack(callbackMap);
        if (callbackMap.size() == 0) {
            LOGE("callbackMap is empty when ReInit");
            return;
        }
        DeviceManagerImpl::GetInstance().SyncCallbacksToService(callbackMap);
        // service callback
        std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> serviceCallbackMap;
        DeviceManagerNotify::GetInstance().GetServiceCallBack(serviceCallbackMap);
        if (serviceCallbackMap.size() == 0) {
            LOGE("serviceCallbackMap is empty when ReInit");
            return;
        }
        DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(serviceCallbackMap);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
